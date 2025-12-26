#include "FingerprintManager.h"
#include "PCIcore/UARTUart.h"
#include "DB/RegisteredFacesDB.h"
#include "MessageHandler/Log.h"
#include <QThread>
#include <QMutexLocker>
#include <QTime>
#include <QCoreApplication>
#include <QTimer>
#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDebug>
#include <cstdint>
#include <cstring>
#include <unistd.h>
#include <time.h>
#include <termios.h>
#include <fcntl.h>      // ✅ ADD THIS LINE - Required for fcntl(), F_GETFL, O_NONBLOCK
#include <errno.h>      // ✅ ADD THIS LINE - Required for errno


using namespace YNH_LJX;

// ============================================================================
// CONSTANTS (Matching ESP32)
// ============================================================================
#define FP_UART_INDEX 1
#define FP_HEADER1 0xEF
#define FP_HEADER2 0x01
#define FP_BAUD_RATE 115200

// Command codes
#define CMD_GET_IMAGE 0x01
#define CMD_GEN_CHAR 0x02
#define CMD_MATCH 0x03
#define CMD_SEARCH 0x04
#define CMD_REG_MODEL 0x05
#define CMD_STORE_CHAR 0x06
#define CMD_LOAD_CHAR 0x07
#define CMD_UP_CHAR 0x08
#define CMD_DOWN_CHAR 0x09
#define CMD_DELETE_CHAR 0x0C
#define CMD_EMPTY_DB 0x0D
#define CMD_TEMPLATE_NUM 0x1D

// Response codes
#define RESP_OK 0x00
#define RESP_ERROR 0x01
#define RESP_NO_FINGER 0x02
#define RESP_IMAGE_FAIL 0x03
#define RESP_NOT_MATCH 0x08
#define RESP_NOT_FOUND 0x09
#define RESP_ENROLL_MISMATCH 0x0A

// Timing constants (MATCH ESP32 EXACTLY!)
#define DELAY_DETECT_FINGER_MS 50        // ESP32: vTaskDelay(pdMS_TO_TICKS(50))
#define DELAY_AFTER_CAPTURE_MS 50        // ESP32: vTaskDelay(pdMS_TO_TICKS(50))
#define DELAY_BETWEEN_STAGES_MS 100      // ESP32: vTaskDelay(pdMS_TO_TICKS(100))
#define DELAY_VENDOR_CHECK_MS 20         // ESP32: vTaskDelay(pdMS_TO_TICKS(20))
#define DELAY_BEFORE_SAVE_MS 50          // ESP32: vTaskDelay(pdMS_TO_TICKS(50))
#define DELAY_AFTER_COMMAND_MS 50        // Default delay after sending command

#define MAX_FINGER_DETECT_ATTEMPTS 100   // ESP32: const int max_attempts = 100

// ============================================================================
// PRIVATE CLASS
// ============================================================================
class FingerprintManagerPrivate
{
    Q_DECLARE_PUBLIC(FingerprintManager)
    
public:
    FingerprintManagerPrivate(FingerprintManager *ptr);
    ~FingerprintManagerPrivate();
    
    // Initialization
    bool initUART();
    void cleanupUART();
    
    // Low-level UART operations (ESP32-style - SIMPLE!)
    bool sendCommand(uint8_t cmd, const uint8_t* data, uint16_t dataLen);
    bool receiveResponse(uint8_t* response, int maxLen, int &bytesReceived, int timeoutMs);
    
    // Helper functions
    void printHex(const char* label, const uint8_t* data, int length);
    uint16_t calculateChecksum(const uint8_t* data, int length);
    
    // Core fingerprint operations (matching ESP32 exactly)
    bool detectFinger(const char* stageName);
    bool captureAndGenerateChar(uint8_t bufferID, const char* stageName);
    bool checkDuplicates(uint8_t bufferID, const char* stageName);
    bool vendorCheck();
    bool storeToFlash(uint16_t fingerId);
    bool searchDatabase(uint16_t &fingerId, uint16_t &confidence);
    bool captureAndGenerateCharWithRetry(uint8_t bufferID, 
                                         const char* stageName, 
                                         int maxRetries,
                                         FingerprintManager* qPtr = nullptr);
    bool captureFinger(const char* stageName);
    void prepareForOperation();
     bool validateAndRecoverUART(const char* location);
    bool attemptUARTRecovery();

private:
    FingerprintManager *const q_ptr;
    QMutex sync;
    bool sensorReady;
    int uartHandle;
    bool isUARTOpen();
    void debugUARTStatus(const char* location);
    int consecutiveFailures;
    QDateTime lastSuccessfulCommand;

};

// ============================================================================
// PRIVATE IMPLEMENTATION
// ============================================================================

FingerprintManagerPrivate::FingerprintManagerPrivate(FingerprintManager *ptr)
    : q_ptr(ptr)
    , sensorReady(false)
    , uartHandle(-1)
    , consecutiveFailures(0)              // 🆕 Initialize
    , lastSuccessfulCommand(QDateTime())  // 🆕 Initialize
{
}

FingerprintManagerPrivate::~FingerprintManagerPrivate()
{
    cleanupUART();
}



bool FingerprintManagerPrivate::initUART()
{
    LogD("=== INITIALIZING FINGERPRINT UART ===\n");
    
    try {
        // Close existing connection if any
        if (uartHandle >= 0) {
            UARTUart::Uart_CloseUartDev(FP_UART_INDEX);
            usleep(100000);
        }
        
        // ✅ CRITICAL: UART configuration for select() usage
        UART_ATTR_S stUartAttr;
        stUartAttr.nBaudRate = 115200;
        stUartAttr.RDBlock = 1;           // ✅ BLOCKING mode (not 0!)
        stUartAttr.mBlockData = 0;        // ✅ VMIN = 0 (return immediately)
        stUartAttr.mBlockTime = 0;        // ✅ VTIME = 0 (no timeout, we use select)
        
        LogD("Opening UART: Baud=115200, BLOCKING mode, VMIN=0, VTIME=0\n");
        LogD("  (Timeout handled by select() in receiveResponse)\n");
        
        int result = UARTUart::Uart_OpenUartDev(FP_UART_INDEX, stUartAttr);
        
        if (result != ISC_OK) {
            LogE("❌ Failed to open UART\n");
            return false;
        }
        
        LogD("✅ UART opened successfully\n");
        uartHandle = FP_UART_INDEX;
        
        // ✅ Get actual file descriptor and verify/configure
        int actual_fd = UARTUart::Uart_GetFileDescriptor(FP_UART_INDEX);
        LogD("File descriptor: %d\n", actual_fd);
        
        if (actual_fd <= 0) {
            LogE("❌ Invalid file descriptor: %d\n", actual_fd);
            return false;
        }
        
        // ✅ Configure terminal for raw mode with select() compatibility
        struct termios tty;
        if (tcgetattr(actual_fd, &tty) != 0) {
            LogE("❌ Failed to get terminal attributes: %s\n", strerror(errno));
            return false;
        }
        
        LogD("Before configuration:\n");
        LogD("  VMIN: %d, VTIME: %d\n", tty.c_cc[VMIN], tty.c_cc[VTIME]);
        
        // ✅ Raw mode settings
        tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);  // No canonical, echo, signals
        tty.c_iflag &= ~(IXON | IXOFF | IXANY | ICRNL | INLCR);  // No flow control
        tty.c_oflag &= ~OPOST;  // No output processing
        
        // ✅ CRITICAL: VMIN=0, VTIME=0 for select() usage
        tty.c_cc[VMIN] = 0;   // Return immediately if data available
        tty.c_cc[VTIME] = 0;  // No inter-byte timeout (we use select)
        
        // ✅ Ensure blocking mode (remove O_NONBLOCK if present)
        int flags = fcntl(actual_fd, F_GETFL);
        if (flags & O_NONBLOCK) {
            LogD("Removing O_NONBLOCK flag...\n");
            if (fcntl(actual_fd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
                LogE("❌ Failed to remove O_NONBLOCK: %s\n", strerror(errno));
            }
        }
        
        // ✅ Apply terminal settings
        if (tcsetattr(actual_fd, TCSANOW, &tty) != 0) {
            LogE("❌ Failed to set terminal attributes: %s\n", strerror(errno));
            return false;
        }
        
        // ✅ Verify final settings
        if (tcgetattr(actual_fd, &tty) == 0) {
            LogD("✅ Final configuration:\n");
            LogD("  VMIN: %d (should be 0)\n", tty.c_cc[VMIN]);
            LogD("  VTIME: %d (should be 0)\n", tty.c_cc[VTIME]);
            
            flags = fcntl(actual_fd, F_GETFL);
            LogD("  File flags: 0x%X (%s)\n", 
                 flags, (flags & O_NONBLOCK) ? "NON-BLOCKING" : "BLOCKING");
            
            // Verify correctness
            if (tty.c_cc[VMIN] != 0 || tty.c_cc[VTIME] != 0) {
                LogE("⚠️  WARNING: VMIN/VTIME not set correctly!\n");
            }
            if (flags & O_NONBLOCK) {
                LogE("⚠️  WARNING: Still in non-blocking mode!\n");
            }
        }
        
        // Clear any stale data
        UARTUart::Uart_ReadFlush(FP_UART_INDEX);
        usleep(50000); // 50ms settle time
        
        // Test communication
        LogD("\n🧪 Testing sensor communication...\n");
        
        uint8_t testCmd[] = {
            0xEF, 0x01, 0x00, 0x00, 0x00, 0x00,
            0x01, 0x00, 0x03, 0x0F, 0x00, 0x13
        };
        
        int sent = UARTUart::Uart_WriteUart(FP_UART_INDEX, testCmd, sizeof(testCmd));
        
        if (sent != sizeof(testCmd)) {
            LogE("❌ Failed to send test command\n");
            UARTUart::Uart_CloseUartDev(FP_UART_INDEX);
            uartHandle = -1;
            return false;
        }
        
        LogD("✅ Test command sent\n");
        
        // Test receive with new function
        uint8_t response[32];
        int bytes;
        
        if (!receiveResponse(response, sizeof(response), bytes, 2000)) {
            LogE("❌ No response from sensor\n");
            UARTUart::Uart_CloseUartDev(FP_UART_INDEX);
            uartHandle = -1;
            return false;
        }
        
        if (bytes >= 1 && response[0] == 0x00) {
            LogD("✅ Sensor communication verified\n");
            UARTUart::Uart_ReadFlush(FP_UART_INDEX);
            sensorReady = true;
            return true;
        }
        
        LogE("❌ Invalid sensor response\n");
        UARTUart::Uart_CloseUartDev(FP_UART_INDEX);
        uartHandle = -1;
        return false;
        
    } catch (const std::exception& e) {
        LogE("❌ Exception: %s\n", e.what());
        if (uartHandle >= 0) {
            UARTUart::Uart_CloseUartDev(FP_UART_INDEX);
            uartHandle = -1;
        }
        return false;
    }
}

bool FingerprintManagerPrivate::isUARTOpen()
{
    extern int fd[6];
    int actual_fd = fd[FP_UART_INDEX];
    
    // ✅ Auto-sync if mismatch detected
    if (actual_fd > 0 && uartHandle < 0) {
        LogD("⚠️  SYNC: Fixing uartHandle from -1 to 1\n");
        uartHandle = FP_UART_INDEX;
    }
    
    if (uartHandle >= 0 && actual_fd <= 0) {
        uartHandle = -1;
        return false;
    }
    
    return (uartHandle >= 0 && actual_fd > 0);
}

void FingerprintManagerPrivate::debugUARTStatus(const char* location)
{
    LogD("\n");
    LogD("╔════════════════════════════════════════╗\n");
    LogD("║  🔍 UART STATUS CHECK: %-16s║\n", location);
    LogD("╚════════════════════════════════════════╝\n");
    
    LogD("uartHandle: %d\n", uartHandle);
    
    extern int fd[6];
    int actual_fd = fd[FP_UART_INDEX];
    LogD("Actual FD: %d\n", actual_fd);
    
    if (actual_fd > 0) {
        int flags = fcntl(actual_fd, F_GETFL);
        if (flags == -1) {
            LogE("❌ UART CLOSED! (fcntl failed: %s)\n", strerror(errno));
        } else {
            LogD("✅ UART OPEN (flags: 0x%X)\n", flags);
            
            // Check if non-blocking
            if (flags & O_NONBLOCK) {
                LogD("   Mode: NON-BLOCKING\n");
            } else {
                LogD("   Mode: BLOCKING\n");
            }
        }
        
        // Try to get terminal attributes
        struct termios options;
        if (tcgetattr(actual_fd, &options) == 0) {
            LogD("✅ Terminal attributes accessible\n");
            LogD("   VMIN: %d\n", options.c_cc[VMIN]);
            LogD("   VTIME: %d\n", options.c_cc[VTIME]);
        } else {
            LogE("❌ Cannot get terminal attributes: %s\n", strerror(errno));
        }
    } else {
        LogE("❌ Invalid file descriptor!\n");
    }
    
    LogD("\n");
}

void FingerprintManagerPrivate::cleanupUART()
{
    if (uartHandle >= 0) {
        UARTUart::Uart_CloseUartDev(FP_UART_INDEX);
        uartHandle = -1;
        sensorReady = false;
    }
}

void FingerprintManagerPrivate::printHex(const char* label, const uint8_t* data, int length)
{
    QString hexString = QString("DEBUG: %1 (%2 bytes): ").arg(label).arg(length);
    for (int i = 0; i < length; i++) {
        hexString += QString("%1 ").arg(data[i], 2, 16, QChar('0')).toUpper();
    }
    LogD("%s\n", hexString.toStdString().c_str());
}

uint16_t FingerprintManagerPrivate::calculateChecksum(const uint8_t* data, int length)
{
    uint16_t checksum = 0;
    for (int i = 0; i < length; i++) {
        checksum += data[i];
    }
    return checksum;
}

bool FingerprintManagerPrivate::validateAndRecoverUART(const char* location)
{
    LogD("🔍 Validating UART at: %s\n", location);
    
    // Check if UART is open
    if (!isUARTOpen()) {
        LogE("❌ UART is CLOSED at %s!\n", location);
        debugUARTStatus(location);
        
        // Attempt recovery
        if (attemptUARTRecovery()) {
            LogD("✅ UART recovered successfully at %s\n", location);
            consecutiveFailures = 0;
            return true;
        }
        
        consecutiveFailures++;
        LogE("❌ UART recovery failed at %s (attempt %d)\n", location, consecutiveFailures);
        
        // If too many failures, mark sensor as not ready
        if (consecutiveFailures >= 3) {
            LogE("❌❌❌ UART PERMANENTLY FAILED - marking sensor as not ready\n");
            sensorReady = false;
        }
        
        return false;
    }
    
    // Verify file descriptor is actually writable
    extern int fd[6];
    int actual_fd = fd[FP_UART_INDEX];
    
    if (actual_fd <= 0) {
        LogE("❌ Invalid FD: %d at %s\n", actual_fd, location);
        return attemptUARTRecovery();
    }
    
    // Test with fcntl
    int flags = fcntl(actual_fd, F_GETFL);
    if (flags == -1) {
        LogE("❌ FD %d is invalid (fcntl failed: %s) at %s\n", 
             actual_fd, strerror(errno), location);
        return attemptUARTRecovery();
    }
    
    // Update success tracking
    consecutiveFailures = 0;
    lastSuccessfulCommand = QDateTime::currentDateTime();
    
    LogD("✅ UART validated at %s (FD: %d, flags: 0x%X)\n", location, actual_fd, flags);
    return true;
}

// ============================================================================
// 🆕 ADD: ATTEMPT TO RECOVER UART
// ============================================================================

bool FingerprintManagerPrivate::attemptUARTRecovery()
{
    LogD("╔═══════════════════════════════════════╗\n");
    LogD("║  🔧 ATTEMPTING UART RECOVERY          ║\n");
    LogD("╚═══════════════════════════════════════╝\n");
    
    // Close existing connection
    if (uartHandle >= 0) {
        LogD("Closing existing UART connection...\n");
        UARTUart::Uart_CloseUartDev(FP_UART_INDEX);
        uartHandle = -1;
        usleep(200000);  // 200ms to let system clean up
    }
    
    // Re-initialize
    LogD("Re-initializing UART...\n");
    if (initUART()) {
        LogD("╔═══════════════════════════════════════╗\n");
        LogD("║  ✅ UART RECOVERY SUCCESSFUL!         ║\n");
        LogD("╚═══════════════════════════════════════╝\n");
        sensorReady = true;
        consecutiveFailures = 0;
        lastSuccessfulCommand = QDateTime::currentDateTime();
        return true;
    }
    
    LogE("╔═══════════════════════════════════════╗\n");
    LogE("║  ❌ UART RECOVERY FAILED               ║\n");
    LogE("╚═══════════════════════════════════════╝\n");
    sensorReady = false;
    return false;
}

// ============================================================================
// ✅ SIMPLIFIED SEND COMMAND (ESP32-Style - NO RETRIES, NO EXCESSIVE DELAYS)
// ============================================================================
bool FingerprintManagerPrivate::sendCommand(uint8_t cmd, const uint8_t* data, uint16_t dataLen)
{
    LogD("──────────────────────────────────────\n");
    LogD("📤 SENDING COMMAND: 0x%02X\n", cmd);
    
    // 🆕 CRITICAL: Validate UART before send
    if (!validateAndRecoverUART("sendCommand")) {
        LogE("❌ UART validation failed - cannot send command 0x%02X\n", cmd);
        return false;
    }
    
    // Build packet (existing code)
    uint8_t packet[256];
    int idx = 0;
    
    packet[idx++] = 0xEF;
    packet[idx++] = 0x01;
    packet[idx++] = 0x00; packet[idx++] = 0x00;
    packet[idx++] = 0x00; packet[idx++] = 0x00;
    packet[idx++] = 0x01;  // Command packet type
    
    uint16_t length = 3 + dataLen;
    packet[idx++] = (length >> 8) & 0xFF;
    packet[idx++] = length & 0xFF;
    packet[idx++] = cmd;
    
    if (data && dataLen > 0) {
        memcpy(&packet[idx], data, dataLen);
        idx += dataLen;
    }
    
    // Calculate checksum
    uint16_t checksum = 0;
    for (int i = 6; i < idx; i++) {
        checksum += packet[i];
    }
    packet[idx++] = (checksum >> 8) & 0xFF;
    packet[idx++] = checksum & 0xFF;
    
    printHex("TX", packet, idx);
    
    // 🆕 IMPROVED: Send with error handling and retry
    int bytesSent = UARTUart::Uart_WriteUart(FP_UART_INDEX, packet, idx);
    
    if (bytesSent != idx) {
        LogE("❌ Send failed: %d/%d bytes (errno: %s)\n", 
             bytesSent, idx, strerror(errno));
        
        // 🆕 Check if UART became invalid during send
        extern int fd[6];
        int actual_fd = fd[FP_UART_INDEX];
        LogE("   Current FD: %d\n", actual_fd);
        
        // 🆕 Try immediate recovery and retry
        LogD("🔄 Attempting immediate recovery and retry...\n");
        if (attemptUARTRecovery()) {
            LogD("✅ Recovery successful, retrying send...\n");
            bytesSent = UARTUart::Uart_WriteUart(FP_UART_INDEX, packet, idx);
            
            if (bytesSent == idx) {
                LogD("✅ Send succeeded after recovery!\n");
                consecutiveFailures = 0;
                lastSuccessfulCommand = QDateTime::currentDateTime();
                return true;
            } else {
                LogE("❌ Retry also failed: %d/%d bytes\n", bytesSent, idx);
            }
        }
        
        consecutiveFailures++;
        return false;
    }
    
    LogD("✅ Command sent (%d bytes)\n", bytesSent);
    consecutiveFailures = 0;
    lastSuccessfulCommand = QDateTime::currentDateTime();
    
    return true;
}

void FingerprintManagerPrivate::prepareForOperation()
{
    LogD("🧹 Clearing UART buffers before operation...\n");
    UARTUart::Uart_ReadFlush(FP_UART_INDEX);
    usleep(50000);  // 50ms settle time
    LogD("✅ Buffers cleared\n");
}

// ============================================================================
// ✅ SIMPLIFIED RECEIVE RESPONSE (ESP32-Style - SINGLE BLOCKING READ)
// ============================================================================
bool FingerprintManagerPrivate::receiveResponse(uint8_t* response, 
                                                int maxLen, 
                                                int &bytesReceived, 
                                                int timeoutMs)
{
    // 🆕 Check UART status before receive
    if (!isUARTOpen()) {
        LogE("❌ UART is CLOSED before receive!\n");
        debugUARTStatus("receiveResponse");
        
        // Try to recover
        if (attemptUARTRecovery()) {
            LogD("✅ UART recovered before receive\n");
        } else {
            return false;
        }
    }
    
    LogD("📥 Waiting for response (timeout: %dms)...\n", timeoutMs);
    
    // Get file descriptor
    int actual_fd = UARTUart::Uart_GetFileDescriptor(FP_UART_INDEX);
    if (actual_fd <= 0) {
        LogE("❌ Invalid file descriptor: %d\n", actual_fd);
        return false;
    }
    
    // Use select() for timeout control
    fd_set readfds;
    struct timeval tv;
    
    FD_ZERO(&readfds);
    FD_SET(actual_fd, &readfds);
    
    tv.tv_sec = timeoutMs / 1000;
    tv.tv_usec = (timeoutMs % 1000) * 1000;
    
    LogD("⏱️  select() waiting (max %d.%03d seconds)...\n", 
         tv.tv_sec, tv.tv_usec / 1000);
    
    auto start = std::chrono::steady_clock::now();
    
    // Wait for data with timeout
    int ret = select(actual_fd + 1, &readfds, NULL, NULL, &tv);
    
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();
    
    if (ret == -1) {
        LogE("❌ select() error: %s\n", strerror(errno));
        return false;
    }
    
    if (ret == 0) {
        // Timeout - this is often normal (no finger present)
        LogD("⏱️  Timeout (no data after %lld ms)\n", elapsed_ms);
        return false;
    }
    
    // Data is available, read it
    LogD("✅ Data available after %lld ms, reading...\n", elapsed_ms);
    
    bytesReceived = read(actual_fd, response, maxLen);
    
    if (bytesReceived <= 0) {
        if (bytesReceived == 0) {
            LogE("❌ read() returned 0 (EOF?)\n");
        } else {
            LogE("❌ read() error: %s\n", strerror(errno));
        }
        debugUARTStatus("after read error");
        return false;
    }
    
    LogD("✅ Received %d bytes after %lld ms total\n", bytesReceived, elapsed_ms);
    printHex("RX", response, bytesReceived);
    
    // Validate response
    if (bytesReceived < 12) {
        LogE("❌ Response too short: %d bytes\n", bytesReceived);
        printHex("Incomplete data", response, bytesReceived);
        return false;
    }
    
    // Find header
    int headerIdx = -1;
    for (int i = 0; i <= bytesReceived - 12; i++) {
        if (response[i] == 0xEF && response[i+1] == 0x01) {
            headerIdx = i;
            break;
        }
    }
    
    if (headerIdx < 0) {
        LogE("❌ No valid header found\n");
        printHex("Invalid packet", response, bytesReceived);
        return false;
    }
    
    // Extract confirmation code
    if (headerIdx + 9 < bytesReceived) {
        uint8_t confirmCode = response[headerIdx + 9];
        LogD("Confirmation code: 0x%02X", confirmCode);
        
        switch(confirmCode) {
            case 0x00: LogD(" -> ✅ SUCCESS\n"); break;
            case 0x01: LogD(" -> ❌ ERROR: Command execution error\n"); break;
            case 0x02: LogD(" -> ⏸️  NO FINGER: No finger on sensor\n"); break;
            case 0x09: LogD(" -> ℹ️  NOT FOUND (no duplicate)\n"); break;
            case 0x0A: LogD(" -> ❌ ERROR: Templates don't match\n"); break;
            default: LogD(" -> ℹ️  Code: 0x%02X\n", confirmCode); break;
        }
        
        // Move response data to start of buffer
        memmove(response, &response[headerIdx + 9], bytesReceived - headerIdx - 9);
        bytesReceived = bytesReceived - headerIdx - 9;
    }
    
    // 🆕 Update success tracking
    lastSuccessfulCommand = QDateTime::currentDateTime();
    consecutiveFailures = 0;
    
    return true;
}

// ============================================================================
// ✅ DETECT FINGER (Matching ESP32's detect_finger function)
// ============================================================================
bool FingerprintManagerPrivate::detectFinger(const char* stageName)
{
    LogD("╔════════════════════════════════════════╗\n");
    LogD("║  🔍 DETECTING FINGER: %-16s║\n", stageName);
    LogD("╚════════════════════════════════════════╝\n");
    
    int attempt = 0;
    
    while (attempt < MAX_FINGER_DETECT_ATTEMPTS) {
        // Send GetImage command (0x01)

        UARTUart::Uart_ReadFlush(FP_UART_INDEX);
        usleep(10000);  // 10ms settle time
        if (!sendCommand(CMD_GET_IMAGE, NULL, 0)) {
            LogE("Failed to send GetImage command\n");
            return false;
        }
        
        uint8_t response[16];
        int bytesReceived;
        
        if (receiveResponse(response, sizeof(response), bytesReceived, 1000)) {
            if (bytesReceived >= 1 && response[0] == 0x00) {  // RESP_OK
                LogD("✅ Finger detected on attempt %d!\n", attempt + 1);
                return true;
            } else if (response[0] == 0x02) {
                // No finger yet, continue polling
                if (attempt % 10 == 0) {
                    LogD("⏳ No finger yet (attempt %d/%d)\n", attempt + 1, MAX_FINGER_DETECT_ATTEMPTS);
                }
            }
        }
        
        attempt++;
        QThread::msleep(DELAY_DETECT_FINGER_MS);  // ✅ Match ESP32: 50ms
        
        // Keep UI responsive
        if (attempt % 10 == 0) {
            QApplication::processEvents();
        }
    }
    
    LogE("❌ Finger detection timeout after %d attempts\n", MAX_FINGER_DETECT_ATTEMPTS);
    return false;
}

bool FingerprintManagerPrivate::captureFinger(const char* stageName)
{
    LogD("╔═══════════════════════════════════════╗\n");
    LogD("║  📸 CAPTURING FINGER: %-16s║\n", stageName);
    LogD("╚═══════════════════════════════════════╝\n");
    
    // Send CaptureFinger command (0x29)
    if (!sendCommand(0x29, NULL, 0)) {
        LogE("❌ Failed to send CaptureFinger command\n");
        return false;
    }
    
    uint8_t response[16];
    int bytesReceived;
    
    if (!receiveResponse(response, sizeof(response), bytesReceived,  2000)) {
        LogE("❌ Failed to receive CaptureFinger response\n");
        return false;
    }
    
    if (bytesReceived >= 1 && response[0] == 0x00) {
        LogD("✅ Finger captured successfully!\n");
        return true;
    }
    
    LogE("❌ CaptureFinger failed with code: 0x%02X\n", response[0]);
    return false;
}

// ============================================================================
// ✅ UPDATED: CAPTURE AND GENERATE CHARACTER (ESP32-style with 0x29)
// ============================================================================
bool FingerprintManagerPrivate::captureAndGenerateChar(uint8_t bufferID, const char* stageName)
{
    LogD("╔═══════════════════════════════════════╗\n");
    LogD("║  🔸 CAPTURE & TEMPLATE: %-14s║\n", stageName);
    LogD("║  Buffer: %d                            ║\n", bufferID);
    LogD("╚═══════════════════════════════════════╝\n");
    
    // ✅ STEP 1: Capture finger image (0x29) - THIS WAS MISSING!
    if (!captureFinger(stageName)) {
        return false;
    }
    
    // ✅ Small delay after capture (match ESP32: 50ms)
    QThread::msleep(50);
    
    // ✅ STEP 2: Generate character file from captured image (0x02)
    uint8_t data[1] = { bufferID };
    
    if (!sendCommand(CMD_GEN_CHAR, data, sizeof(data))) {
        LogE("❌ Failed to send GenChar command\n");
        return false;
    }
    
    uint8_t response[16];
    int bytesReceived;
    
    if (!receiveResponse(response, sizeof(response), bytesReceived, 2000)) {
        LogE("❌ Failed to receive GenChar response\n");
        return false;
    }
    
    if (bytesReceived >= 1 && response[0] == 0x00) {
        LogD("✅ Character file generated in Buffer %d\n", bufferID);
        return true;
    }
    
    LogE("❌ GenChar failed with code: 0x%02X\n", response[0]);
    return false;
}

// ============================================================================
// ✅ UPDATED: CAPTURE AND GENERATE WITH RETRY (includes 0x29)
// ============================================================================
bool FingerprintManagerPrivate::captureAndGenerateCharWithRetry(
    uint8_t bufferID, 
    const char* stageName, 
    int maxRetries,
    FingerprintManager* qPtr)
{
    int attempt = 0;
    
    while (attempt < maxRetries) {
        LogD("╔═══════════════════════════════════════╗\n");
        LogD("║  🔸 CAPTURE & TEMPLATE: %-14s║\n", stageName);
        LogD("║  Buffer: %d | Attempt: %d/%d            ║\n", 
             bufferID, attempt + 1, maxRetries);
        LogD("╚═══════════════════════════════════════╝\n");
        
        // Emit progress update with attempt info
        if (qPtr && attempt > 0) {
            QString msg = QString("Retry %1/%2\n\nAdjust finger placement\nand press firmly")
                         .arg(attempt)
                         .arg(maxRetries - 1);
            qPtr->sigEnrollmentProgress(bufferID + 4, msg);
        }
        
        // ✅ STEP 1: Capture finger image (0x29)
        if (!captureFinger(stageName)) {
            LogE("❌ Capture failed on attempt %d\n", attempt + 1);
            attempt++;
            if (attempt < maxRetries) {
                LogD("🔄 Retrying in 1 second...\n");
                QThread::msleep(1000);
            }
            continue;
        }
        
        // ✅ Small delay after capture (match ESP32)
        QThread::msleep(50);
        
        // ✅ STEP 2: Generate character file (0x02)
        uint8_t data[1] = { bufferID };
        
        if (!sendCommand(CMD_GEN_CHAR, data, sizeof(data))) {
            LogE("❌ Failed to send GenChar command\n");
            attempt++;
            continue;
        }
        
        uint8_t response[16];
        int bytesReceived;
        
        if (!receiveResponse(response, sizeof(response), bytesReceived, 200)) {
            LogE("❌ Failed to receive GenChar response\n");
            attempt++;
            continue;
        }
        
        // Check response
        if (bytesReceived >= 1 && response[0] == 0x00) {
            LogD("✅ Character file generated in Buffer %d\n", bufferID);
            return true;
        } else if (response[0] == 0x01) {
            // Image quality error - can retry
            LogE("❌ GenChar failed (0x01): Poor image quality\n");
            LogD("💡 Tips:\n");
            LogD("   - Clean the sensor surface\n");
            LogD("   - Adjust finger placement\n");
            LogD("   - Press firmly and hold steady\n");
            
            attempt++;
            
            if (attempt < maxRetries) {
                LogD("🔄 Retrying in 2 seconds...\n");
                QThread::msleep(2000);
            }
        } else {
            // Other errors - don't retry
            LogE("❌ GenChar failed with code: 0x%02X (not retrying)\n", response[0]);
            return false;
        }
    }
    
    LogE("❌ Failed after %d attempts\n", maxRetries);
    return false;
}

// ============================================================================
// ✅ CHECK DUPLICATES (Matching ESP32's check_duplicates)
// ============================================================================
bool FingerprintManagerPrivate::checkDuplicates(uint8_t bufferID, const char* stageName)
{
    LogD("╔════════════════════════════════════════╗\n");
    LogD("║  🔎 DUPLICATE CHECK: %-17s║\n", stageName);
    LogD("╚════════════════════════════════════════╝\n");
    
    // ✅ CRITICAL FIX #1: Check if database is empty first
    LogD("📊 Checking database size...\n");
    
    if (!sendCommand(CMD_TEMPLATE_NUM, NULL, 0)) {
        LogE("❌ Failed to send template count command\n");
        // Continue anyway - don't fail just because count failed
    } else {
        uint8_t countResponse[16];
        int countBytes;
        
        if (receiveResponse(countResponse, sizeof(countResponse), countBytes, 5000)) {
            if (countBytes >= 3 && countResponse[0] == 0x00) {
                int templateCount = (countResponse[1] << 8) | countResponse[2];
                LogD("📊 Database has %d templates\n", templateCount);
                
                // ✅ IF EMPTY, SKIP SEARCH - no duplicates possible!
                if (templateCount == 0) {
                    LogD("✅ Database is empty - no duplicates possible\n");
                    return true;
                }
            }
        }
        
        // Small delay after count command
        usleep(50000);
    }
    
    // ✅ Prepare for search
    LogD("🧹 Preparing for search command...\n");
    UARTUart::Uart_ReadFlush(FP_UART_INDEX);
    usleep(100000);  // 100ms delay
    
    // Verify UART is open
    if (!isUARTOpen()) {
        LogE("❌ UART is CLOSED before search!\n");
        debugUARTStatus("before search");
        return false;
    }
    LogD("✅ UART verified open before search\n");
    
    // ✅ CRITICAL FIX #2: Use smaller page count (100 instead of 3000)
    // This matches what you use in searchDatabase() which WORKS!
    uint8_t data[5] = {
        bufferID,
        0x00, 0x00,     // Start page: 0
        0x00, 0x64      // Page count: 100 (NOT 3000!)
    };
    
    if (!sendCommand(CMD_SEARCH, data, sizeof(data))) {
        LogE("❌ Failed to send search command\n");
        return false;
    }
    
    uint8_t response[16];
    int bytesReceived;
    
    // ✅ CRITICAL FIX #3: Increase timeout to 1000ms for search
    if (!receiveResponse(response, sizeof(response), bytesReceived, 5000)) {
        LogE("❌ Failed to receive search response\n");
        
        // Check UART status after failure
        if (!isUARTOpen()) {
            LogE("❌❌❌ UART CLOSED after search timeout!\n");
            debugUARTStatus("after search timeout");
        } else {
            LogD("ℹ️  UART still open after timeout\n");
        }
        
        return false;
    }
    
    if (bytesReceived >= 1) {
        if (response[0] == 0x09) {  // NOT_FOUND - No duplicate
            LogD("✅ No duplicate found for %s\n", stageName);
            return true;
        } else if (response[0] == 0x00) {  // FOUND - Duplicate exists!
            if (bytesReceived >= 3) {
                uint16_t matchedId = (response[1] << 8) | response[2];
                LogE("❌ DUPLICATE FOUND! Matched ID: %d\n", matchedId);
            } else {
                LogE("❌ Duplicate found\n");
            }
            return false;
        }
    }
    
    LogE("❌ Unexpected response: 0x%02X\n", response[0]);
    return false;
}

// ============================================================================
// ✅ VENDOR CHECK (Matching ESP32 - used before save)
// ============================================================================
bool FingerprintManagerPrivate::vendorCheck()
{
    LogD("═══ VENDOR CHECK (0x05) ═══\n");
    
    // ✅ Just send command, ESP32 doesn't wait for response!
    if (!sendCommand(CMD_REG_MODEL, NULL, 0)) {
        LogE("❌ Failed to send vendor check\n");
        return false;
    }
    
    // ✅ CRITICAL: ESP32 DOES NOT READ RESPONSE!
    // It just waits 20ms then continues
    usleep(DELAY_VENDOR_CHECK_MS * 1000);  // 20ms
    LogD("⏱️ 20ms delay (ESP32 style - no response wait)\n");
    
    return true;
}

// ============================================================================
// ✅ STORE TO FLASH (Matching ESP32's save_finger)
// ============================================================================
bool FingerprintManagerPrivate::storeToFlash(uint16_t fingerId)
{
    LogD("╔════════════════════════════════════════╗\n");
    LogD("║  💾 STORING TO FLASH (ID: %d)         ║\n", fingerId);
    LogD("╚════════════════════════════════════════╝\n");
    
    // ✅ Match ESP32: 50ms delay before save
    QThread::msleep(DELAY_BEFORE_SAVE_MS);
    
    // ✅ Step 1: Vendor check (don't wait for response)
    if (!vendorCheck()) {
        return false;
    }
    
    // ✅ Step 2: Store command immediately after vendor check
    uint8_t data[3] = {
        0x01,  // Buffer ID (Buffer 1 contains merged template)
        (uint8_t)((fingerId >> 8) & 0xFF),
        (uint8_t)(fingerId & 0xFF)
    };
    
    if (!sendCommand(CMD_STORE_CHAR, data, sizeof(data))) {
        LogE("❌ Failed to send store command\n");
        return false;
    }
    
    // ✅ NOW wait for store response
    uint8_t response[16];
    int bytesReceived;
    
    if (!receiveResponse(response, sizeof(response), bytesReceived, 3000)) {
        LogE("❌ Failed to receive store response\n");
        return false;
    }
    
    if (bytesReceived >= 1 && response[0] == 0x00) {
        LogD("╔════════════════════════════════════════╗\n");
        LogD("║  ✅ FINGERPRINT STORED SUCCESSFULLY!   ║\n");
        LogD("╚════════════════════════════════════════╝\n");
        LogD("Finger ID: %d\n", fingerId);
        return true;
    }
    
    LogE("❌ Store failed with code: 0x%02X\n", response[0]);
    return false;
}

// ============================================================================
// ✅ SEARCH DATABASE (Matching ESP32's identification)
// ============================================================================
bool FingerprintManagerPrivate::searchDatabase(uint16_t &fingerId, uint16_t &confidence)
{
    LogD("╔════════════════════════════════════════╗\n");
    LogD("║  🔍 SEARCHING DATABASE                 ║\n");
    LogD("╚════════════════════════════════════════╝\n");
    
    uint8_t data[5] = {
        0x01,        // Buffer 1
        0x00, 0x00,  // Start page: 0
        0x00, 0x64   // Page count: 100
    };
    
    if (!sendCommand(CMD_SEARCH, data, sizeof(data))) {
        LogE("❌ Failed to send search command\n");
        return false;
    }
    
    // ✅ For search, ESP32 waits longer (sensor needs time to search)
    uint8_t response[32];
    int bytesReceived;
    
    if (!receiveResponse(response, sizeof(response), bytesReceived, 5000)) {
        LogE("❌ No search response\n");
        return false;
    }
    
    if (bytesReceived >= 1) {
        if (response[0] == 0x00 && bytesReceived >= 5) {  // Match found
            fingerId = (response[1] << 8) | response[2];
            confidence = (response[3] << 8) | response[4];
            
            LogD("╔════════════════════════════════════════╗\n");
            LogD("║  ✅ MATCH FOUND!                       ║\n");
            LogD("╚════════════════════════════════════════╝\n");
            LogD("Finger ID: %d\n", fingerId);
            LogD("Confidence: %d\n", confidence);
            
            return true;
        } else if (response[0] == 0x09) {  // Not found
            LogD("❌ No match found in database\n");
            return false;
        }
    }
    
    LogE("❌ Search failed with code: 0x%02X\n", response[0]);
    return false;
}

// ============================================================================
// PUBLIC API IMPLEMENTATION
// ============================================================================

FingerprintManager::FingerprintManager(QObject *parent)
    : QObject(parent)
    , d_ptr(new FingerprintManagerPrivate(this))
{
}

FingerprintManager::~FingerprintManager()
{
}

bool FingerprintManager::initFingerprintSensor()
{
    Q_D(FingerprintManager);
    QMutexLocker locker(&d->sync);
    
    LogD("╔═══════════════════════════════════════╗\n");
    LogD("║  INITIALIZING FINGERPRINT SENSOR      ║\n");
    LogD("╚═══════════════════════════════════════╝\n");
    
    // Single UART initialization - no test command
    return d->initUART();
}

bool FingerprintManager::isSensorReady() const
{
    Q_D(const FingerprintManager);
    return d->sensorReady;
}

void FingerprintManager::startContinuousMonitoring()
 {
     Q_D(FingerprintManager);
    
     if (m_isMonitoring) {
         LogD("%s %s[%d] Already monitoring\n", 
              __FILE__, __FUNCTION__, __LINE__);
        return;
     }
    
     LogD("═══════════════════════════════════════\n");
     LogD("  STARTING FINGERPRINT MONITORING     \n");
     LogD("═══════════════════════════════════════\n");
    
    if (!d->sensorReady) {
        LogD("Marking sensor as ready...\n");
        d->sensorReady = true;
    }
    
    m_monitoringTimer = new QTimer(this);
    
    // ✅ Just call directly in main thread
    connect(m_monitoringTimer, &QTimer::timeout, 
            this, &FingerprintManager::checkForFingerprint);
    
   m_monitoringTimer->start(500);
   m_isMonitoring = true;
    
     LogD("✅ Monitoring started (checking every 500ms)\n");
}

void FingerprintManager::stopContinuousMonitoring()
{
    if (!m_isMonitoring) {
        return;
    }
    
    LogD("Stopping monitoring...\n");
    
    if (m_monitoringTimer) {
        m_monitoringTimer->stop();
        m_monitoringTimer->deleteLater();
        m_monitoringTimer = nullptr;
    }
    
    m_isMonitoring = false;
    LogD("✅ Monitoring stopped\n");
}

bool FingerprintManager::isMonitoring() const
{
    return m_isMonitoring;
}

void FingerprintManager::checkForFingerprint()
{
    Q_D(FingerprintManager);
    
    // CRITICAL: Add mutex lock for thread safety
    QMutexLocker locker(&d->sync);
    
    // Check sensor readiness
    if (!d->sensorReady) {
        // Try to recover every 10 seconds
        static QDateTime lastRecoveryAttempt;
        if (lastRecoveryAttempt.isNull() || 
            lastRecoveryAttempt.secsTo(QDateTime::currentDateTime()) > 10) {
            
            LogD("🔧 Sensor not ready, attempting recovery...\n");
            lastRecoveryAttempt = QDateTime::currentDateTime();
            
            if (d->attemptUARTRecovery()) {
                LogD("✅ Sensor recovered in monitoring\n");
            }
        }
        return;
    }
    
    // Validate UART before checking
    if (!d->validateAndRecoverUART("checkForFingerprint")) {
        LogD("⚠️ UART validation failed in monitoring (will retry next cycle)\n");
        return;
    }
    
    // Clear any stale data before checking
    UARTUart::Uart_ReadFlush(FP_UART_INDEX);
    usleep(10000);  // 10ms settle
    
    // Check if finger on sensor
    if (!d->sendCommand(CMD_GET_IMAGE, nullptr, 0)) {
        return;
    }
    
    uint8_t response[16];
    int bytesReceived;
    
    if (!d->receiveResponse(response, sizeof(response), bytesReceived, 1000)) {
        return;
    }
    
    if (bytesReceived >= 1 && response[0] == RESP_OK) {
        LogD("\n👆 FINGER DETECTED (Monitoring Mode)\n");
        
        // Stop monitoring during identification
        if (m_monitoringTimer) {
            m_monitoringTimer->stop();
        }
        
        uint16_t fingerId = 0;
        uint16_t confidence = 0;
        
        // Step 1: Capture and generate characteristics
        if (d->captureAndGenerateCharWithRetry(0x01, "Monitor", 2, this)) {
            
            // Step 2: Search sensor database
            if (d->searchDatabase(fingerId, confidence)) {
                
                LogD("╔═══════════════════════════════════════╗\n");
                LogD("║  ✅ MATCH FOUND!                       ║\n");
                LogD("║  Finger ID: %-27d║\n", fingerId);
                LogD("║  Confidence: %-25d║\n", confidence);
                LogD("╚═══════════════════════════════════════╝\n");
                
                // Step 3: Query database to get person information
                PERSONS_t person;
                bool personFound = RegisteredFacesDB::GetInstance()->GetPersonByFingerId(fingerId, person);
                
                if (personFound) {
                    LogD("\n╔════════════════════════════════════════════════════════════╗\n");
                    LogD("║                    👤 USER IDENTIFIED                      ║\n");
                    LogD("╠════════════════════════════════════════════════════════════╣\n");
                    LogD("║  Employee ID:  %-43s ║\n", person.idcard.toStdString().c_str());
                    LogD("║  Name:         %-43s ║\n", person.name.toStdString().c_str());
                    LogD("║  Department:   %-43s ║\n", person.department.toStdString().c_str());
                    LogD("║  Finger ID:    %-43d ║\n", fingerId);
                    LogD("║  Confidence:   %-43d ║\n", confidence);
                    LogD("╚════════════════════════════════════════════════════════════╝\n");
                    
                    // ✅ CRITICAL DEBUG: Check signal connections BEFORE emitting
                    QString fingerIdStr = QString::number(fingerId);
                    
                    LogD("\n╔════════════════════════════════════════════════════════════╗\n");
                    LogD("║  🔍 SIGNAL CONNECTION CHECK                                ║\n");
                    LogD("╚════════════════════════════════════════════════════════════╝\n");
                    
                    // Check how many slots are connected to this signal
                    int connCount = this->receivers(
                        SIGNAL(sigFingerprintMatched(QString,QString,int)));
                    
                    LogD("Signal: sigFingerprintMatched(QString, QString, int)\n");
                    LogD("Connected receivers: %d\n", connCount);
                    
                    if (connCount == 0) {
                        LogE("\n");
                        LogE("❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌\n");
                        LogE("❌  CRITICAL ERROR: NO SIGNAL CONNECTIONS!  ❌\n");
                        LogE("❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌❌\n");
                        LogE("\n");
                        LogE("Signal 'sigFingerprintMatched' has ZERO receivers!\n");
                        LogE("This means the connection in FaceApp::InitConnect() FAILED!\n");
                        LogE("\n");
                        LogE("Check these things:\n");
                        LogE("1. Is Q_OBJECT macro in FingerprintManager.h?\n");
                        LogE("2. Did you run 'qmake' after adding the signal?\n");
                        LogE("3. Did you rebuild completely?\n");
                        LogE("4. Is the connection code in FaceApp::InitConnect()?\n");
                        LogE("\n");
                        LogE("Signal will be emitted but NO ONE is listening!\n");
                        LogE("\n");
                    } else {
                        LogD("✅ Signal has %d receiver(s) - connection OK!\n", connCount);
                    }
                    
                    LogD("\n╔════════════════════════════════════════════════════════════╗\n");
                    LogD("║  🚀 EMITTING SIGNAL NOW                                    ║\n");
                    LogD("╚════════════════════════════════════════════════════════════╝\n");
                    
                    LogD("Parameters to be sent:\n");
                    LogD("  fingerIdStr = \"%s\" (type: QString)\n", fingerIdStr.toStdString().c_str());
                    LogD("  name = \"%s\" (type: QString)\n", person.name.toStdString().c_str());
                    LogD("  confidence = %d (type: int)\n", confidence);
                    
                    LogD("\nEmitting signal...\n");
                    
                    // Emit the signal
                    emit sigFingerprintMatched(fingerIdStr, person.name, confidence);
                    
                    LogD("Signal emitted!\n");
                    
                    // Force immediate processing to ensure signal is delivered
                    QCoreApplication::processEvents();
                    
                    LogD("\n╔════════════════════════════════════════════════════════════╗\n");
                    LogD("║  ✅ SIGNAL PROCESSING COMPLETE                             ║\n");
                    LogD("╚════════════════════════════════════════════════════════════╝\n");
                    
                    LogD("If you see this but slotFingerprintMatched() is NOT called,\n");
                    LogD("then the signal-slot connection is broken!\n");
                    
                } else {
                    // ❌ ERROR - Finger ID found but person not in database
                    LogE("\n╔═══════════════════════════════════════╗\n");
                    LogE("║  ❌ DATABASE SYNC ERROR!               ║\n");
                    LogE("╚═══════════════════════════════════════╝\n");
                    LogE("Finger ID %d matched but person NOT FOUND in database!\n", fingerId);
                    
                    QString fingerIdStr = QString::number(fingerId);
                    emit sigFingerprintMatched(fingerIdStr, "Unknown User", confidence);
                }
                
            } else {
                // No match in sensor database
                LogD("👤 STRANGER (no match)\n");
                emit sigFingerprintNotRecognized();
            }
            
        } else {
            // Failed to capture fingerprint
            LogE("❌ Failed to capture fingerprint\n");
            emit sigFingerprintNotRecognized();
        }
        
        // Resume monitoring after 3 seconds
        QTimer::singleShot(3000, this, [this, d]() {
            if (m_monitoringTimer && d->sensorReady) {
                LogD("▶️  Resuming fingerprint monitoring...\n");
                m_monitoringTimer->start(500);
            }
        });
    }
}

bool FingerprintManager::checkUARTHealth()
{
    Q_D(FingerprintManager);
    QMutexLocker locker(&d->sync);
    
    LogD("\n");
    LogD("╔═══════════════════════════════════════╗\n");
    LogD("║  🏥 UART HEALTH CHECK                 ║\n");
    LogD("╚═══════════════════════════════════════╝\n");
    
    if (!d->isUARTOpen()) {
        LogE("❌ UART is closed\n");
        d->debugUARTStatus("health check");
        
        if (d->attemptUARTRecovery()) {
            LogD("✅ UART recovered during health check\n");
            return true;
        }
        
        return false;
    }
    
    LogD("✅ UART is healthy\n");
    LogD("   Consecutive failures: %d\n", d->consecutiveFailures);
    
    if (d->lastSuccessfulCommand.isValid()) {
        qint64 secondsSinceSuccess = d->lastSuccessfulCommand.secsTo(QDateTime::currentDateTime());
        LogD("   Last successful command: %lld seconds ago\n", secondsSinceSuccess);
        
        // If no successful commands in 5 minutes, something might be wrong
        if (secondsSinceSuccess > 300) {
            LogE("⚠️ WARNING: No successful commands in %lld seconds\n", secondsSinceSuccess);
            LogE("   This may indicate a problem with the sensor\n");
        }
    } else {
        LogD("   Last successful command: Never\n");
    }
    
    return true;
}

// ============================================================================
// ✅ ENROLLMENT (3-STAGE PROCESS - MATCHING ESP32 EXACTLY!)
// ============================================================================
bool FingerprintManager::startEnrollment(uint16_t fingerId)
{
    Q_D(FingerprintManager);
    QMutexLocker locker(&d->sync);
    d->prepareForOperation();
    
    LogD("╔═══════════════════════════════════════╗\n");
    LogD("║  FINGERPRINT ENROLLMENT (3 Stages)    ║\n");
    LogD("║  Finger ID: %-27d║\n", fingerId);
    LogD("╚═══════════════════════════════════════╝\n");
    
    if (!d->sensorReady) {
        emit sigEnrollmentFailed("Sensor not ready");
        return false;
    }
    
    // ========================================================================
    // STAGE 1: First Sample → Buffer 1
    // ========================================================================
    LogD("\n");
    LogD("╔═══════════════════════════════════════╗\n");
    LogD("║  STAGE 1/3: First Capture             ║\n");
    LogD("╚═══════════════════════════════════════╝\n");
    
    emit sigEnrollmentProgress(1, "Place finger firmly\n(Sample 1/3)");
    
    // ✅ 1. Detect finger (0x01)
    if (!d->detectFinger("first sample")) {
        emit sigEnrollmentFailed("No finger detected (Stage 1)");
        return false;
    }
    
    // ✅ 2. Capture (0x29) + Generate (0x02) → Buffer 1
    if (!d->captureAndGenerateChar(0x01, "First")) {
        emit sigEnrollmentFailed("Failed to generate first template");
        return false;
    }
    
    // ✅ 3. Check duplicates (0x04)
    if (!d->checkDuplicates(0x01, "first")) {
        emit sigEnrollmentFailed("Duplicate fingerprint found!");
        return false;
    }
    
    LogD("✅ Stage 1 complete\n");
    
    // ✅ Match ESP32: 100ms delay between stages
    QThread::msleep(100);
    
    emit sigEnrollmentProgress(2, "✅ First sample captured!\n\nRemove finger...");
    QThread::msleep(500);
    
    // ========================================================================
    // STAGE 2: Second Sample → Buffer 2
    // ========================================================================
    LogD("\n");
    LogD("╔═══════════════════════════════════════╗\n");
    LogD("║  STAGE 2/3: Second Capture            ║\n");
    LogD("╚═══════════════════════════════════════╝\n");
    
    emit sigEnrollmentProgress(3, "Place SAME finger again\n(Sample 2/3)");
    
    // ✅ 1. Detect finger (0x01)
    if (!d->detectFinger("second sample")) {
        emit sigEnrollmentFailed("No finger detected (Stage 2)");
        return false;
    }
    
    // ✅ 2. Capture (0x29) + Generate (0x02) → Buffer 2
    if (!d->captureAndGenerateChar(0x02, "Second")) {
        emit sigEnrollmentFailed("Failed to generate second template");
        return false;
    }
    
    // ✅ 3. Check duplicates (0x04)
    if (!d->checkDuplicates(0x02, "second")) {
        emit sigEnrollmentFailed("Duplicate fingerprint found!");
        return false;
    }
    
    LogD("✅ Stage 2 complete\n");
    
    // ✅ Match ESP32: 100ms delay
    QThread::msleep(100);
    
    emit sigEnrollmentProgress(4, "✅ Second sample captured!\n\nRemove finger...");
    QThread::msleep(500);
    
    // ========================================================================
    // STAGE 3: Third Sample → Buffer 3
    // ========================================================================
    LogD("\n");
    LogD("╔═══════════════════════════════════════╗\n");
    LogD("║  STAGE 3/3: Final Capture             ║\n");
    LogD("╚═══════════════════════════════════════╝\n");
    
    emit sigEnrollmentProgress(5, "Place SAME finger one more time\n(Sample 3/3)");
    
    // ✅ 1. Detect finger (0x01)
    if (!d->detectFinger("final sample")) {
        emit sigEnrollmentFailed("No finger detected (Stage 3)");
        return false;
    }
    
    // ✅ 2. Capture (0x29) + Generate (0x02) → Buffer 3 (with retry)
    if (!d->captureAndGenerateCharWithRetry(0x03, "Third", 3, this)) {
        emit sigEnrollmentFailed("Failed to generate third template");
        return false;
    }
    
    // ✅ 3. Check duplicates (0x04)
    if (!d->checkDuplicates(0x03, "third")) {
        emit sigEnrollmentFailed("Duplicate fingerprint found!");
        return false;
    }
    
    LogD("✅ Stage 3 complete\n");
    
    // ========================================================================
    // FINAL STAGE: Vendor Check (0x05) + Store (0x06)
    // ========================================================================
    LogD("\n");
    LogD("╔═══════════════════════════════════════╗\n");
    LogD("║  FINAL: Merging and Saving            ║\n");
    LogD("╚═══════════════════════════════════════╝\n");
    
    emit sigEnrollmentProgress(6, "Saving fingerprint...");
    
    // ✅ Store to flash (includes vendor check)
    if (!d->storeToFlash(fingerId)) {
        emit sigEnrollmentFailed("Failed to save fingerprint");
        return false;
    }
    
    LogD("\n");
    LogD("╔═══════════════════════════════════════╗\n");
    LogD("║  ✅ ENROLLMENT SUCCESSFUL!             ║\n");
    LogD("╚═══════════════════════════════════════╝\n");
    
    emit sigEnrollmentComplete(fingerId);
    return true;
}

bool FingerprintManager::compareFingerprintWithDatabase()
{
    Q_D(FingerprintManager);
    QMutexLocker locker(&d->sync);
    
    LogD("╔════════════════════════════════════════╗\n");
    LogD("║  COMPARE WITH DATABASE                ║\n");
    LogD("╚════════════════════════════════════════╝\n");
    
    if (!d->sensorReady) {
        LogE("❌ Sensor not ready\n");
        return false;
    }
    
    // Step 1: Detect finger
    LogD("\n[STEP 1/3] Detecting finger...\n");
    if (!d->detectFinger("Compare")) {
        return false;
    }
    
    // Step 2: Capture and generate characteristics
    LogD("\n[STEP 2/3] Capturing fingerprint...\n");
    if (!d->captureAndGenerateCharWithRetry(0x01, "Compare", 3, this)) {
        return false;
    }
    
    // Step 3: Search database for match
    LogD("\n[STEP 3/3] Searching database...\n");
    uint16_t fingerId = 0;
    uint16_t confidence = 0;
    
    if (!d->searchDatabase(fingerId, confidence)) {
        LogD("❌ No match found in database\n");
        return false;
    }
    
     LogD("╔════════════════════════════════════════╗\n");
    LogD("║  ✅ MATCH FOUND!                       ║\n");
    LogD("║  Finger ID: %-27d║\n", fingerId);
    LogD("║  Confidence: %-25d║\n", confidence);
    LogD("╚════════════════════════════════════════╝\n");
    
    // ✅ SIMPLIFIED: Just emit finger ID and confidence
    // Database query will happen in IdentityManagement where DB is open
    LogD("✅ Match found - Finger ID: %d, Confidence: %d\n", fingerId, confidence);

    // Convert fingerId to QString for signal
    QString fingerIdStr = QString::number(fingerId);

    // Emit signal with finger ID (not person ID!)
    // IdentityManagement will query the database to get person info
    emit sigFingerprintMatched(fingerIdStr, "", confidence);

    LogD("🚀 Signal emitted: sigFingerprintMatched(fingerID=\"%s\", confidence=%d)\n",
         fingerIdStr.toStdString().c_str(),
         confidence);

    return true;

}

// ============================================================================
// ✅ IDENTIFICATION (Matching ESP32's capture_and_identify_finger)
// ============================================================================
bool FingerprintManager::identifyFingerprint(uint16_t &matchedFingerId, float &confidence)
{
    Q_D(FingerprintManager);
    QMutexLocker locker(&d->sync);
    
    LogD("╔═══════════════════════════════════════╗\n");
    LogD("║  FINGERPRINT IDENTIFICATION           ║\n");
    LogD("╚═══════════════════════════════════════╝\n");
    
    if (!d->sensorReady) {
        LogE("❌ Sensor not ready\n");
        return false;
    }
    
    // ✅ Step 1: Detect finger (0x01)
    if (!d->detectFinger("identification")) {
        LogE("❌ No finger detected\n");
        return false;
    }
    
    // ✅ Step 2: Capture finger (0x29)
    if (!d->captureFinger("identification")) {
        LogE("❌ Failed to capture finger\n");
        return false;
    }
    
    // ✅ Small delay (match ESP32)
    QThread::msleep(50);
    
    // ✅ Step 3: Generate character file (0x02)
    uint8_t data[1] = { 0x01 };  // Buffer 1
    
    if (!d->sendCommand(CMD_GEN_CHAR, data, sizeof(data))) {
        LogE("❌ Failed to send GenChar command\n");
        return false;
    }
    
    uint8_t response[16];
    int bytesReceived;
    
    if (!d->receiveResponse(response, sizeof(response), bytesReceived, 200)) {
        LogE("❌ Failed to receive GenChar response\n");
        return false;
    }
    
    if (bytesReceived < 1 || response[0] != 0x00) {
        LogE("❌ GenChar failed with code: 0x%02X\n", response[0]);
        return false;
    }
    
    LogD("✅ Template generated\n");
    
    // ✅ Step 4: Search database (0x04)
    uint16_t conf;
    if (d->searchDatabase(matchedFingerId, conf)) {
        confidence = (float)conf / 100.0f;
        emit sigFingerprintMatched(matchedFingerId, confidence);
        return true;
    }
    
    emit sigFingerprintNotMatched();
    return false;
}

// ============================================================================
// ✅ IDENTIFICATION WITH UI (Complete flow)
// ============================================================================
bool FingerprintManager::identifyAndShowResult(QWidget* parent)
{
    Q_D(FingerprintManager);
    QMutexLocker locker(&d->sync);
    
    LogD("╔════════════════════════════════════════╗\n");
    LogD("║  IDENTIFICATION WITH RESULT DISPLAY   ║\n");
    LogD("╚════════════════════════════════════════╝\n");
    
    if (!d->sensorReady) {
        QMessageBox::warning(parent, "Sensor Error", "Fingerprint sensor not ready!");
        return false;
    }
    
    // Step 1: Detect finger
    LogD("\n[STEP 1] Waiting for finger...\n");
    
    if (!d->detectFinger("identification")) {
        QMessageBox::warning(parent, "No Finger", 
                           "No finger detected.\nPlease place your finger on the sensor.");
        return false;
    }
    
    // Step 2: Generate character
    LogD("\n[STEP 2] Generating template...\n");
    
    if (!d->captureAndGenerateChar(0x01, "Identification")) {
        QMessageBox::warning(parent, "Error", "Failed to generate fingerprint template.");
        return false;
    }
    
    // Step 3: Search database
    LogD("\n[STEP 3] Searching database...\n");
    
    uint16_t fingerId;
    uint16_t confidence;
    
    if (!d->searchDatabase(fingerId, confidence)) {
        LogD("❌ No match found\n");
        QMessageBox::warning(parent, "Not Recognized", 
                           "Fingerprint not recognized!\n\nNo match found in database.");
        return false;
    }
    
    // Step 4: Look up user
    LogD("\n[STEP 4] Looking up user...\n");
    
    PERSONS_t person;
    if (!RegisteredFacesDB::GetInstance()->GetPersonByFingerId(fingerId, person)) {
        QMessageBox::warning(parent, "User Not Found", 
                           QString("Fingerprint recognized (ID: %1)\nbut user not found in database!")
                           .arg(fingerId));
        return false;
    }
    
    // Step 5: Show result
    float confidencePercent = (float)confidence / 100.0f;
    
    QString message = QString("✅ FINGERPRINT RECOGNIZED!\n\n"
                             "Employee ID: %1\n"
                             "Name: %2\n"
                             "Department: %3\n"
                             "Confidence: %4%\n"
                             "Finger ID: %5")
                        .arg(person.idcard)
                        .arg(person.name)
                        .arg(person.department)
                        .arg(confidencePercent, 0, 'f', 1)
                        .arg(fingerId);
    
    QMessageBox::information(parent, "Access Granted", message);
    
    LogD("╔════════════════════════════════════════╗\n");
    LogD("║  ✅ IDENTIFICATION SUCCESS!            ║\n");
    LogD("╚════════════════════════════════════════╝\n");
    
    return true;
}

// ============================================================================
// ✅ DELETE FINGERPRINT
// ============================================================================
bool FingerprintManager::deleteFingerprintTemplate(uint16_t fingerId)
{
    Q_D(FingerprintManager);
    QMutexLocker locker(&d->sync);
    
    LogD("╔════════════════════════════════════════╗\n");
    LogD("║  DELETE FINGERPRINT (ID: %d)          ║\n", fingerId);
    LogD("╚════════════════════════════════════════╝\n");
    
    uint8_t data[4] = {
        (uint8_t)((fingerId >> 8) & 0xFF),
        (uint8_t)(fingerId & 0xFF),
        0x00, 0x01  // Delete count: 1
    };
    
    if (!d->sendCommand(CMD_DELETE_CHAR, data, sizeof(data))) {
        LogE("❌ Failed to send delete command\n");
        return false;
    }
    
    uint8_t response[16];
    int bytesReceived;
    
    if (!d->receiveResponse(response, sizeof(response), bytesReceived, 200)) {
        LogE("❌ Failed to receive delete response\n");
        return false;
    }
    
    if (bytesReceived >= 1 && response[0] == 0x00) {
        LogD("✅ Fingerprint %d deleted successfully\n", fingerId);
        return true;
    }
    
    LogE("❌ Delete failed with code: 0x%02X\n", response[0]);
    return false;
}

// ============================================================================
// ✅ DELETE ALL FINGERPRINTS
// ============================================================================
bool FingerprintManager::deleteAllFingerprints()
{
    Q_D(FingerprintManager);
    QMutexLocker locker(&d->sync);
    
    LogD("╔════════════════════════════════════════╗\n");
    LogD("║  DELETE ALL FINGERPRINTS              ║\n");
    LogD("╚════════════════════════════════════════╝\n");
    
    if (!d->sendCommand(CMD_EMPTY_DB, NULL, 0)) {
        LogE("❌ Failed to send delete all command\n");
        return false;
    }
    
    uint8_t response[16];
    int bytesReceived;
    
    if (!d->receiveResponse(response, sizeof(response), bytesReceived, 200)) {
        LogE("❌ Failed to receive delete all response\n");
        return false;
    }
    
    if (bytesReceived >= 1 && response[0] == 0x00) {
        LogD("✅ All fingerprints deleted successfully\n");
        
        // Clear finger_id values from database
        RegisteredFacesDB::GetInstance()->clearAllFingerIds();
        
        return true;
    }
    
    LogE("❌ Delete all failed with code: 0x%02X\n", response[0]);
    return false;
}

// ============================================================================
// ✅ GET TEMPLATE COUNT
// ============================================================================
int FingerprintManager::getTemplateCount()
{
    Q_D(FingerprintManager);
    QMutexLocker locker(&d->sync);
    
    LogD("╔════════════════════════════════════════╗\n");
    LogD("║  GET TEMPLATE COUNT                   ║\n");
    LogD("╚════════════════════════════════════════╝\n");
    
    if (!d->sendCommand(CMD_TEMPLATE_NUM, NULL, 0)) {
        LogE("❌ Failed to send template count command\n");
        return -1;
    }
    
    uint8_t response[16];
    int bytesReceived;
    
    if (!d->receiveResponse(response, sizeof(response), bytesReceived, 200)) {
        LogE("❌ Failed to receive template count response\n");
        return -1;
    }
    
    if (bytesReceived >= 3 && response[0] == 0x00) {
        int count = (response[1] << 8) | response[2];
        LogD("✅ Template count: %d\n", count);
        return count;
    }
    
    LogE("❌ Failed to get template count, code: 0x%02X\n", response[0]);
    return -1;
}

// ============================================================================
// ✅ DOWNLOAD FINGERPRINT TEMPLATE
// ============================================================================
bool FingerprintManager::downloadFingerprintTemplate(uint16_t fingerId, QByteArray &templateData)
{
    Q_D(FingerprintManager);
    QMutexLocker locker(&d->sync);
    
    LogD("╔════════════════════════════════════════╗\n");
    LogD("║  DOWNLOAD TEMPLATE (ID: %d)           ║\n", fingerId);
    LogD("╚════════════════════════════════════════╝\n");
    
    templateData.clear();
    
    // Step 1: Load template from flash to Buffer 1
    LogD("\n[STEP 1] Loading from flash to Buffer 1...\n");
    
    uint8_t loadData[3] = {
        0x01,  // Buffer 1
        (uint8_t)((fingerId >> 8) & 0xFF),
        (uint8_t)(fingerId & 0xFF)
    };
    
    if (!d->sendCommand(CMD_LOAD_CHAR, loadData, sizeof(loadData))) {
        LogE("❌ Failed to send LoadChar command\n");
        return false;
    }
    
    uint8_t response[16];
    int bytesReceived;
    
    if (!d->receiveResponse(response, sizeof(response), bytesReceived, 200)) {
        LogE("❌ Failed to receive LoadChar response\n");
        return false;
    }
    
    if (bytesReceived < 1 || response[0] != 0x00) {
        LogE("❌ LoadChar failed with code: 0x%02X\n", response[0]);
        return false;
    }
    
    LogD("✅ Template loaded to Buffer 1\n");
    
    // Step 2: Upload template from Buffer 1
    LogD("\n[STEP 2] Uploading from Buffer 1...\n");
    
    uint8_t uploadData[1] = { 0x01 };  // Buffer 1
    
    if (!d->sendCommand(CMD_UP_CHAR, uploadData, sizeof(uploadData))) {
        LogE("❌ Failed to send UpChar command\n");
        return false;
    }
    
    if (!d->receiveResponse(response, sizeof(response), bytesReceived, 200)) {
        LogE("❌ Failed to receive UpChar acknowledgment\n");
        return false;
    }
    
    if (bytesReceived < 1 || response[0] != 0x00) {
        LogE("❌ UpChar failed with code: 0x%02X\n", response[0]);
        return false;
    }
    
    LogD("✅ UpChar acknowledged, receiving data packets...\n");
    
    // Step 3: Receive template data packets
    LogD("\n[STEP 3] Receiving template data packets...\n");
    
    const int MAX_PACKETS = 20;
    int packetCount = 0;
    bool endPacketReceived = false;
    
    while (packetCount < MAX_PACKETS && !endPacketReceived) {
        uint8_t packet[256];
        int bytes = UARTUart::Uart_ReadUart(FP_UART_INDEX, packet, sizeof(packet));
        
        if (bytes <= 0) {
            LogD("No more data\n");
            usleep(50000);
            continue;
        }
        
        LogD("Packet %d: %d bytes\n", packetCount + 1, bytes);
        
        // Find header
        int headerIdx = -1;
        for (int i = 0; i < bytes - 1; i++) {
            if (packet[i] == 0xEF && packet[i+1] == 0x01) {
                headerIdx = i;
                break;
            }
        }
        
        if (headerIdx < 0 || headerIdx + 9 > bytes) {
            LogE("❌ Invalid packet\n");
            break;
        }
        
        uint8_t packetType = packet[headerIdx + 6];
        uint16_t packetLen = (packet[headerIdx + 7] << 8) | packet[headerIdx + 8];
        
        LogD("Packet type: 0x%02X, Length: %d\n", packetType, packetLen);
        
        // Extract data (skip header + checksum)
        int dataStart = headerIdx + 9;
        int dataLen = packetLen - 2;
        
        if (dataStart + dataLen <= bytes && dataLen > 0) {
            templateData.append((char*)&packet[dataStart], dataLen);
            LogD("Extracted %d bytes (total: %d)\n", dataLen, templateData.size());
        }
        
        // Check for end packet
        if (packetType == 0x08) {
            LogD("✅ End packet received\n");
            endPacketReceived = true;
            break;
        }
        
        packetCount++;
    }
    
    if (!endPacketReceived) {
        LogE("❌ Did not receive end packet\n");
        return false;
    }
    
    LogD("╔════════════════════════════════════════╗\n");
    LogD("║  ✅ TEMPLATE DOWNLOADED!               ║\n");
    LogD("╚════════════════════════════════════════╝\n");
    LogD("Total size: %d bytes\n", templateData.size());
    
    return !templateData.isEmpty();
}

// ============================================================================
// ✅ UPLOAD TEMPLATE TO SENSOR (For restoring from database)
// ============================================================================
bool FingerprintManager::uploadTemplateToSensor(uint16_t fingerId, const QByteArray &templateData)
{
    Q_D(FingerprintManager);
    QMutexLocker locker(&d->sync);
    
    LogD("╔════════════════════════════════════════╗\n");
    LogD("║  UPLOAD TEMPLATE TO SENSOR            ║\n");
    LogD("║  ID: %-33d║\n", fingerId);
    LogD("║  Size: %-30d║\n", templateData.size());
    LogD("╚════════════════════════════════════════╝\n");
    
    if (templateData.isEmpty()) {
        LogE("❌ Empty template data\n");
        return false;
    }
    
    // Step 1: Send DownChar command
    LogD("\n[STEP 1] Sending DownChar command...\n");
    
    uint8_t downCharData[1] = { 0x01 };  // Buffer 1
    
    if (!d->sendCommand(CMD_DOWN_CHAR, downCharData, sizeof(downCharData))) {
        LogE("❌ Failed to send DownChar command\n");
        return false;
    }
    
    uint8_t response[16];
    int bytesReceived;
    
    if (!d->receiveResponse(response, sizeof(response), bytesReceived, 200)) {
        LogE("❌ Failed to receive DownChar response\n");
        return false;
    }
    
    if (bytesReceived < 1 || response[0] != 0x00) {
        LogE("❌ DownChar failed with code: 0x%02X\n", response[0]);
        return false;
    }
    
    LogD("✅ DownChar acknowledged\n");
    
    // Step 2: Send template data as single packet (0x08 - end packet)
    LogD("\n[STEP 2] Sending template data...\n");
    
    int totalSize = 9 + templateData.size() + 2;  // header + data + checksum
    uint8_t *packet = new uint8_t[totalSize];
    
    packet[0] = 0xEF;
    packet[1] = 0x01;
    packet[2] = 0x00; packet[3] = 0x00; packet[4] = 0x00; packet[5] = 0x00;
    packet[6] = 0x08;  // End packet type
    
    uint16_t payloadLen = templateData.size() + 2;
    packet[7] = (payloadLen >> 8) & 0xFF;
    packet[8] = payloadLen & 0xFF;
    
    memcpy(&packet[9], templateData.constData(), templateData.size());
    
    // Calculate checksum
    uint16_t checksum = 0;
    for (int i = 6; i < 9 + templateData.size(); i++) {
        checksum += packet[i];
    }
    packet[9 + templateData.size()] = (checksum >> 8) & 0xFF;
    packet[9 + templateData.size() + 1] = checksum & 0xFF;
    
    d->printHex("Template packet", packet, totalSize);
    
    int sent = UARTUart::Uart_WriteUart(FP_UART_INDEX, packet, totalSize);
    delete[] packet;
    
    if (sent != totalSize) {
        LogE("❌ Failed to send template data\n");
        return false;
    }
    
    LogD("✅ Template data sent (%d bytes)\n", sent);
    
    // Wait for upload response
    usleep(100000);  // 100ms
    
    if (!d->receiveResponse(response, sizeof(response), bytesReceived, 200)) {
        LogE("❌ Failed to receive upload response\n");
        return false;
    }
    
    if (bytesReceived < 1 || response[0] != 0x00) {
        LogE("❌ Upload failed with code: 0x%02X\n", response[0]);
        return false;
    }
    
    LogD("✅ Template uploaded to Buffer 1\n");
    
    // Step 3: Store template to flash
    LogD("\n[STEP 3] Storing to flash...\n");
    
    uint8_t storeData[3] = {
        0x01,  // Buffer 1
        (uint8_t)((fingerId >> 8) & 0xFF),
        (uint8_t)(fingerId & 0xFF)
    };
    
    if (!d->sendCommand(CMD_STORE_CHAR, storeData, sizeof(storeData))) {
        LogE("❌ Failed to send store command\n");
        return false;
    }
    
    if (!d->receiveResponse(response, sizeof(response), bytesReceived, 200)) {
        LogE("❌ Failed to receive store response\n");
        return false;
    }
    
    if (bytesReceived >= 1 && response[0] == 0x00) {
        LogD("╔════════════════════════════════════════╗\n");
        LogD("║  ✅ TEMPLATE UPLOADED & STORED!        ║\n");
        LogD("╚════════════════════════════════════════╝\n");
        return true;
    }
    
    LogE("❌ Store failed with code: 0x%02X\n", response[0]);
    return false;
}

// ============================================================================
// STUB FUNCTIONS (For compatibility)
// ============================================================================
bool FingerprintManager::captureFingerprint(QByteArray &fingerprintTemplate)
{
    // Not used in ESP32 style
    return false;
}

bool FingerprintManager::storeFingerprintTemplate(uint16_t fingerId, const QByteArray &templateData)
{
    return uploadTemplateToSensor(fingerId, templateData);
}

bool FingerprintManager::downloadTemplateFromSensor(uint16_t fingerId, QByteArray &templateData)
{
    return downloadFingerprintTemplate(fingerId, templateData);
}