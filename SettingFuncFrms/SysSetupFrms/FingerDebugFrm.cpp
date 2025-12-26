#include "FingerDebugFrm.h"
#include "PCIcore/UARTUart.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDebug>
#include <QTimer>
#include <QThread>
#include <QCoreApplication>
#include <vector>
#include <cstring>
#include <unistd.h>

using namespace YNH_LJX;

// Define constants to avoid linker issues
#define FP_UART_INDEX 1
#define FP_HEADER1 0xEF
#define FP_HEADER2 0x01

// Worker class to handle fingerprint operations in separate thread
class FingerprintWorker : public QObject {
    Q_OBJECT
    
public slots:
    void doFingerprintTest() {
        qDebug() << "FingerprintWorker: Starting test in separate thread...";
        
        // Initialize UART with non-blocking settings
        if (!initUART()) {
            emit testCompleted(false, "UART initialization failed");
            return;
        }
        
        // Run tests
        bool success = testBasicCommunication();
        
        // Cleanup
        cleanup();
        
        QString result = success ? "Communication test PASSED" : "Communication test FAILED";
        emit testCompleted(success, result);
    }
    
signals:
    void testCompleted(bool success, const QString& message);
    void statusUpdate(const QString& status);
    
private:
    void printHexDebug(const char* label, unsigned char* data, int length) {
        QString hexString = QString("DEBUG: %1 (%2 bytes): ").arg(label).arg(length);
        for (int i = 0; i < length; i++) {
            hexString += QString("%1 ").arg(data[i], 2, 16, QChar('0')).toUpper();
        }
        qDebug() << hexString;
    }
    
    bool initUART() {
        qDebug() << "DEBUG: Initializing UART for fingerprint sensor on ttyS1...";
        
        // Try multiple baud rates commonly used by fingerprint sensors
        int baud_rates[] = {57600, 9600, 19200, 38400, 115200};
        int num_rates = sizeof(baud_rates) / sizeof(int);
        
        for (int i = 0; i < num_rates; i++) {
            qDebug() << QString("DEBUG: Trying baud rate %1...").arg(baud_rates[i]);
            
            // Close previous connection if any
            UARTUart::Uart_CloseUartDev(FP_UART_INDEX);
            usleep(100000); // 100ms delay
            
            UART_ATTR_S stUartAttr;
            stUartAttr.nBaudRate = baud_rates[i];
            stUartAttr.RDBlock = 0;          // NON-BLOCKING READ
            stUartAttr.mBlockData = 0;       // Don't wait for data
            stUartAttr.mBlockTime = 10;      // Short timeout (1 second)
            
            int result = UARTUart::Uart_OpenUartDev(FP_UART_INDEX, stUartAttr);
            
            if (result == ISC_OK) {
                qDebug() << QString("DEBUG: UART opened with baud rate %1").arg(baud_rates[i]);
                
                // Test communication with a simple command
                if (testQuickCommunication()) {
                    qDebug() << QString("SUCCESS: Communication confirmed at %1 baud").arg(baud_rates[i]);
                    return true;
                } else {
                    qDebug() << QString("No response at %1 baud, trying next...").arg(baud_rates[i]);
                }
            } else {
                qDebug() << QString("Failed to open UART at %1 baud").arg(baud_rates[i]);
            }
        }
        
        qDebug() << "ERROR: Failed to establish communication at any baud rate";
        return false;
    }
    
    bool testQuickCommunication() {
        // Send a simple command and check for any response
        auto cmd = buildSimpleCommand(0x0F); // Read System Parameters
        
        unsigned char* cmd_data = (unsigned char*)cmd.data();
        int bytes_sent = UARTUart::Uart_WriteUart(FP_UART_INDEX, cmd_data, cmd.size());
        
        if (bytes_sent != (int)cmd.size()) {
            return false;
        }
        
        // Wait a bit for response
        usleep(200000); // 200ms
        
        // Try to read any response
        unsigned char response_buffer[32];
        int bytes_received = UARTUart::Uart_ReadUart(FP_UART_INDEX, response_buffer, sizeof(response_buffer));
        
        return (bytes_received > 0);
    }
    
    std::vector<uint8_t> buildSimpleCommand(uint8_t cmd) {
        std::vector<uint8_t> packet;
        
        // Header
        packet.push_back(FP_HEADER1);
        packet.push_back(FP_HEADER2);
        
        // Device Address (4 bytes, all zeros)
        packet.push_back(0x00);
        packet.push_back(0x00);
        packet.push_back(0x00);
        packet.push_back(0x00);
        
        // Packet Type (Command = 0x01)
        packet.push_back(0x01);
        
        // Length (command + checksum = 3 bytes)
        packet.push_back(0x00);
        packet.push_back(0x03);
        
        // Command
        packet.push_back(cmd);
        
        // Calculate checksum
        uint16_t checksum = 0x01 + 0x00 + 0x03 + cmd;
        
        // Add checksum bytes
        packet.push_back((checksum >> 8) & 0xFF);
        packet.push_back(checksum & 0xFF);
        
        return packet;
    }
    
    bool sendCommand(const std::vector<uint8_t>& command) {
        qDebug() << "DEBUG: Sending command...";
        
        unsigned char* cmd_data = (unsigned char*)command.data();
        printHexDebug("TX Command", cmd_data, command.size());
        
        int bytes_sent = UARTUart::Uart_WriteUart(FP_UART_INDEX, cmd_data, command.size());
        
        if (bytes_sent == (int)command.size()) {
            qDebug() << QString("DEBUG: Command sent successfully (%1 bytes)").arg(bytes_sent);
            return true;
        } else {
            qDebug() << QString("ERROR: Failed to send command. Sent %1 bytes, expected %2")
                        .arg(bytes_sent).arg(command.size());
            return false;
        }
    }
    
    bool receiveResponseNonBlocking() {
        qDebug() << "DEBUG: Waiting for response (non-blocking)...";
        
        unsigned char response_buffer[32];
        memset(response_buffer, 0, sizeof(response_buffer));
        
        // Try multiple times with small delays (non-blocking approach)
        int total_attempts = 10;  // 1 second total timeout
        int bytes_received = 0;
        
        for (int attempt = 0; attempt < total_attempts; attempt++) {
            bytes_received = UARTUart::Uart_ReadUart(FP_UART_INDEX, response_buffer, sizeof(response_buffer));
            
            if (bytes_received > 0) {
                qDebug() << QString("DEBUG: Received response on attempt %1 (%2 bytes)").arg(attempt + 1).arg(bytes_received);
                break;
            }
            
            // Small delay between attempts (non-blocking sleep)
            usleep(100000); // 100ms
        }
        
        if (bytes_received > 0) {
            printHexDebug("RX Response", response_buffer, bytes_received);
            
            // Check if we got a valid header
            if (bytes_received >= 2 && response_buffer[0] == FP_HEADER1 && response_buffer[1] == FP_HEADER2) {
                qDebug() << "DEBUG: Valid response header detected!";
                
                if (bytes_received >= 12) {
                    uint8_t confirmation_code = response_buffer[9];
                    qDebug() << QString("DEBUG: Confirmation code: 0x%1").arg(confirmation_code, 2, 16, QChar('0')).toUpper();
                    
                    if (confirmation_code == 0x00) {
                        qDebug() << "SUCCESS: Command executed successfully!";
                        return true;
                    } else {
                        qDebug() << QString("WARNING: Command returned error code: 0x%1")
                                    .arg(confirmation_code, 2, 16, QChar('0')).toUpper();
                        return false;
                    }
                } else {
                    qDebug() << QString("WARNING: Response too short (%1 bytes)").arg(bytes_received);
                    return false;
                }
            } else {
                qDebug() << "WARNING: Invalid response header";
                return false;
            }
        } else {
            qDebug() << "WARNING: No response received after all attempts";
            return false;
        }
    }
    
    bool testBasicCommunication() {
        qDebug() << "\n=== FINGERPRINT SENSOR COMMUNICATION TEST ===";
        
        // Test 1: Read System Parameters - This should work now
        emit statusUpdate("Testing System Parameters...");
        qDebug() << "\nTEST 1: Reading System Parameters...";
        auto cmd1 = buildSimpleCommand(0x0F);
        bool test1_result = false;
        if (sendCommand(cmd1)) {
            test1_result = receiveResponseNonBlocking();
        }
        
        usleep(500000); // 500ms delay
        
        // Test 2: Get Template Count - This worked before
        emit statusUpdate("Testing Template Count...");
        qDebug() << "\nTEST 2: Getting Template Count...";
        auto cmd2 = buildSimpleCommand(0x1D);
        bool test2_result = false;
        if (sendCommand(cmd2)) {
            test2_result = receiveResponseNonBlocking();
        }
        
        usleep(500000); // 500ms delay
        
        // Test 3: Try to detect finger (will fail if no finger present, but shows communication)
        emit statusUpdate("Testing Finger Detection...");
        qDebug() << "\nTEST 3: Finger Detection Test...";
        auto cmd3 = buildSimpleCommand(0x01); // Get Image
        bool test3_result = false;
        if (sendCommand(cmd3)) {
            test3_result = receiveResponseNonBlocking();
            if (!test3_result) {
                qDebug() << "INFO: No finger detected (this is normal if no finger on sensor)";
            }
        }
        
        usleep(500000); // 500ms delay
        
        // Test 4: LED Control Test (if supported)
        emit statusUpdate("Testing LED Control...");
        qDebug() << "\nTEST 4: LED Control Test...";
        std::vector<uint8_t> led_cmd;
        led_cmd.push_back(FP_HEADER1); led_cmd.push_back(FP_HEADER2);
        led_cmd.push_back(0x00); led_cmd.push_back(0x00); led_cmd.push_back(0x00); led_cmd.push_back(0x00);
        led_cmd.push_back(0x01); led_cmd.push_back(0x00); led_cmd.push_back(0x05);
        led_cmd.push_back(0x50); // LED control command
        led_cmd.push_back(0x01); // LED on
        led_cmd.push_back(0x64); // Speed/brightness
        uint16_t checksum = 0x01 + 0x00 + 0x05 + 0x50 + 0x01 + 0x64;
        led_cmd.push_back((checksum >> 8) & 0xFF); 
        led_cmd.push_back(checksum & 0xFF);
        
        bool test4_result = false;
        if (sendCommand(led_cmd)) {
            test4_result = receiveResponseNonBlocking();
        }
        
        // Summary
        qDebug() << "\n=== ENHANCED TEST RESULTS SUMMARY ===";
        qDebug() << QString("Test 1 (System Parameters): %1").arg(test1_result ? "PASSED" : "FAILED");
        qDebug() << QString("Test 2 (Template Count): %1").arg(test2_result ? "PASSED" : "FAILED");
        qDebug() << QString("Test 3 (Finger Detection): %1").arg(test3_result ? "FINGER DETECTED" : "NO FINGER");
        qDebug() << QString("Test 4 (LED Control): %1").arg(test4_result ? "PASSED" : "NOT SUPPORTED");
        
        bool overall_success = test1_result || test2_result;
        qDebug() << QString("Overall Communication: %1").arg(overall_success ? "SUCCESS" : "FAILED");
        
        if (overall_success) {
            qDebug() << "\n=== SENSOR READY FOR FINGERPRINT OPERATIONS ===";
            qDebug() << "The sensor is communicating properly and ready for:";
            qDebug() << "- Fingerprint enrollment";
            qDebug() << "- Fingerprint matching";
            qDebug() << "- Template management";
        }
        
        return overall_success;
    }
    
    void cleanup() {
        qDebug() << "DEBUG: Cleaning up UART connection...";
        UARTUart::Uart_CloseUartDev(FP_UART_INDEX);
        qDebug() << "DEBUG: UART closed";
    }
};

FingerDebugFrm::FingerDebugFrm(QWidget *parent)
    : QDialog(parent)
    , m_pDebugButton(nullptr)
    , m_pStatusLabel(nullptr)
    , m_pWorkerThread(nullptr)
    , m_pWorker(nullptr)
{
    setWindowTitle(tr("Finger Debug"));
    setFixedSize(300, 200);
    
    initUI();
    initConnect();
    setupWorkerThread();
}

FingerDebugFrm::~FingerDebugFrm()
{
    // Clean up worker thread
    if (m_pWorkerThread) {
        m_pWorkerThread->quit();
        m_pWorkerThread->wait();
        delete m_pWorkerThread;
    }
}

void FingerDebugFrm::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    m_pStatusLabel = new QLabel(tr("Finger Debug Tool\nReady to test"), this);
    m_pStatusLabel->setAlignment(Qt::AlignCenter);
    
    m_pDebugButton = new QPushButton(tr("Start Finger Debug"), this);
    m_pDebugButton->setFixedHeight(40);
    
    mainLayout->addWidget(m_pStatusLabel);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(m_pDebugButton);
    mainLayout->addStretch();
}

void FingerDebugFrm::initConnect()
{
    connect(m_pDebugButton, &QPushButton::clicked, this, &FingerDebugFrm::onDebugButtonClicked);
}

void FingerDebugFrm::setupWorkerThread()
{
    // Create worker thread
    m_pWorkerThread = new QThread(this);
    m_pWorker = new FingerprintWorker();
    
    // Move worker to thread
    m_pWorker->moveToThread(m_pWorkerThread);
    
    // Connect signals
    connect(m_pWorkerThread, &QThread::started, m_pWorker, &FingerprintWorker::doFingerprintTest);
    connect(m_pWorker, &FingerprintWorker::testCompleted, this, &FingerDebugFrm::onTestCompleted);
    connect(m_pWorker, &FingerprintWorker::statusUpdate, this, &FingerDebugFrm::onStatusUpdate);
    connect(m_pWorker, &FingerprintWorker::testCompleted, m_pWorkerThread, &QThread::quit);
    
    // Start the thread (but worker won't start until we call start() again)
    m_pWorkerThread->start();
}

void FingerDebugFrm::onDebugButtonClicked()
{
    qDebug() << "Finger Debug button clicked - starting debug process in separate thread...";
    
    // Disable button during test
    m_pDebugButton->setEnabled(false);
    m_pStatusLabel->setText(tr("Initializing test...\nCheck qDebug output"));
    
    // Start the fingerprint test in worker thread
    if (m_pWorkerThread->isRunning()) {
        // If thread is already running, restart it
        m_pWorkerThread->quit();
        m_pWorkerThread->wait();
    }
    
    m_pWorkerThread->start();
}

void FingerDebugFrm::onTestCompleted(bool success, const QString& message)
{
    qDebug() << "FingerDebugFrm: Test completed with result:" << success << message;
    
    // Update UI with results
    if (success) {
        m_pStatusLabel->setText(tr("Communication test PASSED!\nCheck qDebug for details"));
    } else {
        m_pStatusLabel->setText(tr("Communication test FAILED!\nCheck qDebug for details"));
    }
    
    // Re-enable button after a delay
    QTimer::singleShot(3000, this, [this]() {
        m_pDebugButton->setEnabled(true);
        m_pStatusLabel->setText(tr("Finger Debug Tool\nReady to test"));
    });
}

void FingerDebugFrm::onStatusUpdate(const QString& status)
{
    m_pStatusLabel->setText(status + "\nCheck qDebug output");
}

#include "FingerDebugFrm.moc"