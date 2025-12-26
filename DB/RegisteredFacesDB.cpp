#include "RegisteredFacesDB.h"

#ifdef Q_OS_LINUX
//#include "ArcsoftFaceManager.h"
#include <unistd.h>
#endif

#include "BaiduFaceManager.h"
#include "SharedInclude/GlobalDef.h"
#include "Application/FaceApp.h"
#include "MessageHandler/Log.h"
#include "HttpServer/ConnHttpServerThread.h"
#include "FaceMainFrm.h"
#include <fcntl.h>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDatabase>
#include <QtCore/QWaitCondition>
#include <QtCore/QMutex>
#include <QtCore/QList>
#include <QtCore/QUuid>
#include <QtCore/QDateTime>
#include <QtSql/QSqlError>
#include <QtCore/QDebug>
#include <QtCore/QTime>
#include <QDir>
#include <QPixmap>
#include <QBuffer>
#include <QImageWriter>
#include <QImageReader>


 double gAlogStateFaceSimilar = 0; //å…¨å±€äººè„¸å¯¹æ¯”é˜ˆå€¼

static QMutex g_personIdMutex;

class RegisteredFacesDBPrivate
{
    Q_DECLARE_PUBLIC(RegisteredFacesDB)
public:
    RegisteredFacesDBPrivate(RegisteredFacesDB *dd);
private:
    mutable QMutex sync;
    QWaitCondition pauseCond;
    volatile bool is_pause;
private:
    float mThanthreshold;
    QList<PERSONS_t>mPersons;
private:
    RegisteredFacesDB *const q_ptr;
};

RegisteredFacesDBPrivate::RegisteredFacesDBPrivate(RegisteredFacesDB *dd)
    : q_ptr(dd)
    , is_pause(false)
    , mThanthreshold(0.8)
{
}


RegisteredFacesDB::RegisteredFacesDB(QObject *parent)
    : QThread(parent)
    , d_ptr(new RegisteredFacesDBPrivate(this))
{
    this->start();
}

RegisteredFacesDB::~RegisteredFacesDB()
{
    Q_D(RegisteredFacesDB);
    this->requestInterruption();
    d->is_pause = false;
    d->pauseCond.wakeOne();
    this->quit();
    this->wait();
}

bool RegisteredFacesDB::ensureFaceImageDirectory()
{
    QString dirPath = "/mnt/user/reg_face_image";
    QDir dir;
    
    // Check if directory exists
    if (!dir.exists(dirPath)) {
        LogD("%s %s[%d] Directory does not exist, creating: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, dirPath.toStdString().c_str());
             
        // Create the directory with all parent directories
        if (!dir.mkpath(dirPath)) {
            LogE("%s %s[%d] Failed to create directory: %s\n", 
                 __FILE__, __FUNCTION__, __LINE__, dirPath.toStdString().c_str());
            return false;
        }
        
        // Set proper permissions (read/write for owner, group, others)
        QFile::setPermissions(dirPath, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                                     QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup |
                                     QFile::ReadOther | QFile::WriteOther | QFile::ExeOther);
        
        LogD("%s %s[%d] Successfully created directory: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, dirPath.toStdString().c_str());
    } else {
        LogD("%s %s[%d] Directory already exists: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, dirPath.toStdString().c_str());
    }
    
    // Double check that directory is writable
    QFileInfo dirInfo(dirPath);
    if (!dirInfo.isWritable()) {
        LogE("%s %s[%d] Directory is not writable: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, dirPath.toStdString().c_str());
        return false;
    }
    
    return true;
}

bool RegisteredFacesDB::saveFaceImage(const QString &employeeId, const QByteArray &imageData, const QString &format)
{
    LogD("%s %s[%d] Starting to save face image for employee: %s, data size: %d bytes\n", 
         __FILE__, __FUNCTION__, __LINE__, employeeId.toStdString().c_str(), imageData.size());
    
    if (imageData.isEmpty()) {
        LogE("%s %s[%d] Image data is empty for employee: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, employeeId.toStdString().c_str());
        return false;
    }
    
    if (employeeId.isEmpty()) {
        LogE("%s %s[%d] Employee ID is empty\n", __FILE__, __FUNCTION__, __LINE__);
        return false;
    }
    
    if (!ensureFaceImageDirectory()) {
        LogE("%s %s[%d] Failed to ensure directory exists\n", __FILE__, __FUNCTION__, __LINE__);
        return false;
    }
    
    QString fileName = QString("/mnt/user/reg_face_image/%1.%2").arg(employeeId).arg(format.toLower());
    LogD("%s %s[%d] Attempting to save image to: %s\n", 
         __FILE__, __FUNCTION__, __LINE__, fileName.toStdString().c_str());
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        LogE("%s %s[%d] Failed to open file for writing: %s, Error: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, fileName.toStdString().c_str(), 
             file.errorString().toStdString().c_str());
        return false;
    }
    
    qint64 bytesWritten = file.write(imageData);
    file.close();
    
    if (bytesWritten == imageData.size()) {
        LogD("%s %s[%d] Successfully saved face image: %s (size: %lld bytes)\n", 
             __FILE__, __FUNCTION__, __LINE__, fileName.toStdString().c_str(), bytesWritten);
        
        // Sync to ensure data is written to disk
#ifdef Q_OS_LINUX
        system("sync");
#endif
        
        // Verify file was actually created and has correct size
        QFileInfo fileInfo(fileName);
        if (fileInfo.exists() && fileInfo.size() == imageData.size()) {
            LogD("%s %s[%d] File verification successful: %s, size: %lld\n", 
                 __FILE__, __FUNCTION__, __LINE__, fileName.toStdString().c_str(), fileInfo.size());
            return true;
        } else {
            LogE("%s %s[%d] File verification failed: %s, exists: %s, expected size: %d, actual size: %lld\n", 
                 __FILE__, __FUNCTION__, __LINE__, fileName.toStdString().c_str(),
                 fileInfo.exists() ? "true" : "false", imageData.size(), fileInfo.size());
            return false;
        }
    } else {
        LogE("%s %s[%d] Failed to write complete image data to: %s, wrote %lld of %d bytes\n", 
             __FILE__, __FUNCTION__, __LINE__, fileName.toStdString().c_str(), 
             bytesWritten, imageData.size());
        return false;
    }
}

bool RegisteredFacesDB::saveCroppedFaceImage(const QString &employeeId, const QString &originalImagePath, 
                                           const QRect &faceRect, const QString &format)
{
    if (!ensureFaceImageDirectory()) {
        return false;
    }
    
    // Load original image
    QImageReader reader(originalImagePath);
    if (!reader.canRead()) {
        LogE("%s %s[%d] Cannot read image file: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, originalImagePath.toStdString().c_str());
        return false;
    }
    
    QImage originalImage = reader.read();
    if (originalImage.isNull()) {
        LogE("%s %s[%d] Failed to load image: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, originalImagePath.toStdString().c_str());
        return false;
    }
    
    // Crop the face region
    QImage croppedImage = originalImage.copy(faceRect);
    if (croppedImage.isNull()) {
        LogE("%s %s[%d] Failed to crop image with rect (%d,%d,%d,%d)\n", 
             __FILE__, __FUNCTION__, __LINE__, faceRect.x(), faceRect.y(), 
             faceRect.width(), faceRect.height());
        return false;
    }
    
    // Save cropped image
    QString fileName = QString("/mnt/user/reg_face_image/%1.%2").arg(employeeId).arg(format.toLower());
    
    QImageWriter writer(fileName, format.toUpper().toUtf8());
    writer.setQuality(95); // High quality
    
    if (writer.write(croppedImage)) {
        LogD("%s %s[%d] Successfully saved cropped face image: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, fileName.toStdString().c_str());
        return true;
    } else {
        LogE("%s %s[%d] Failed to save cropped image: %s, Error: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, fileName.toStdString().c_str(), 
             writer.errorString().toStdString().c_str());
        return false;
    }
}


QString RegisteredFacesDB::getFaceImagePath(const QString &employeeId, const QString &format)
{
    QString fileName = QString("/mnt/user/reg_face_image/%1.%2").arg(employeeId).arg(format.toLower());
    QFile file(fileName);
    if (file.exists()) {
        return fileName;
    }
    return QString(); // Return empty string if file doesn't exist
}

bool RegisteredFacesDB::deleteFaceImage(const QString &employeeId, const QString &format)
{
    QString fileName = QString("/mnt/user/reg_face_image/%1.%2").arg(employeeId).arg(format.toLower());
    QFile file(fileName);
    if (file.exists()) {
        bool result = file.remove();
        if (result) {
            LogD("%s %s[%d] Successfully deleted face image: %s\n", 
                 __FILE__, __FUNCTION__, __LINE__, fileName.toStdString().c_str());
        } else {
            LogE("%s %s[%d] Failed to delete face image: %s\n", 
                 __FILE__, __FUNCTION__, __LINE__, fileName.toStdString().c_str());
        }
        return result;
    }
    return true; // Consider it successful if file doesn't exist
}

bool RegisteredFacesDB::extractAndSaveFaceImage(const QString &employeeId, const QString &sourceImagePath)
{
    LogD("%s %s[%d] === FACE CROPPING EXTRACTION (like AddUserFrm) ===\n", __FILE__, __FUNCTION__, __LINE__);
    LogD("%s %s[%d] Employee ID: %s, Source: %s\n", __FILE__, __FUNCTION__, __LINE__, 
         employeeId.toStdString().c_str(), sourceImagePath.toStdString().c_str());
    
    if (employeeId.isEmpty() || sourceImagePath.isEmpty()) {
        LogE("%s %s[%d] Invalid parameters\n", __FILE__, __FUNCTION__, __LINE__);
        return false;
    }
    
    // Check if source file exists
    QFile sourceFile(sourceImagePath);
    if (!sourceFile.exists()) {
        LogE("%s %s[%d] Source image does not exist: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, sourceImagePath.toStdString().c_str());
        return false;
    }
    
    // âœ… USE SAME LOGIC AS AddUserFrm - Call BaiduFaceManager for cropping
    BaiduFaceManager* faceManager = (BaiduFaceManager*)qXLApp->GetAlgoFaceManager();
    if (!faceManager) {
        LogE("%s %s[%d] BaiduFaceManager not available\n", __FILE__, __FUNCTION__, __LINE__);
        return false;
    }
    
    bool imageSaved = false;
    
    // Method 1: Try cropping from the captured image file (SAME AS AddUserFrm)
    LogD("%s %s[%d] Attempting to crop from file...\n", __FILE__, __FUNCTION__, __LINE__);
    imageSaved = faceManager->cropAndSaveFaceImage(employeeId, sourceImagePath);
    
    if (imageSaved) {
        LogD("%s %s[%d] SUCCESS: Cropped face image saved from file for employee: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, employeeId.toStdString().c_str());
    } else {
        LogD("%s %s[%d] File-based cropping failed, trying current face data...\n", __FILE__, __FUNCTION__, __LINE__);
        
        // Method 2: Try cropping from current face data in memory (SAME AS AddUserFrm)
        imageSaved = faceManager->cropCurrentFaceAndSave(employeeId);
        
        if (imageSaved) {
            LogD("%s %s[%d] SUCCESS: Cropped face image saved from current data for employee: %s\n", 
                 __FILE__, __FUNCTION__, __LINE__, employeeId.toStdString().c_str());
        }
    }
    
    if (imageSaved) {
        QString savedPath = QString("/mnt/user/reg_face_image/%1.jpg").arg(employeeId);
        LogD("%s %s[%d] Cropped face image saved at: %s\n", __FILE__, __FUNCTION__, __LINE__, savedPath.toStdString().c_str());
        
        // Verify the file exists and has reasonable size (SAME AS AddUserFrm)
        QFileInfo fileInfo(savedPath);
        if (fileInfo.exists() && fileInfo.size() > 1000) { // At least 1KB
            LogD("%s %s[%d] File verification passed: size = %lld bytes\n", __FILE__, __FUNCTION__, __LINE__, fileInfo.size());
        } else {
            LogE("%s %s[%d] File verification failed or file too small\n", __FILE__, __FUNCTION__, __LINE__);
            return false;
        }
    } else {
        LogE("%s %s[%d] Failed to save cropped face image for employee: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, employeeId.toStdString().c_str());
        return false;
    }
    
    return imageSaved;
}

bool RegisteredFacesDB::selectPersonByFingerId(uint16_t fingerId, PERSONS_s &info)
{
    QString sql = QString(
        "SELECT * FROM persons WHERE finger_id = %1").arg(fingerId);
    QSqlQuery query;
    if (!query.exec(sql)) {
        LogE("DB query failed: %s\n", 
             query.lastError().text().toStdString().c_str());
        return false;
    }
    if (query.next()) {
        info.personid = query.value("personid").toInt();  // âœ… toInt(), not toString()
        info.name = query.value("name").toString();
        info.idcard = query.value("idcard").toString();
        info.iccard = query.value("iccard").toString();
        info.sex = query.value("sex").toString();
        info.persontype = query.value("persontype").toInt();
        return true;
    }
    return false;
}

bool RegisteredFacesDB::selectPersonByPersonId(int personId, PERSONS_s &info)
{
    QString sql = QString(
        "SELECT * FROM persons WHERE personid = %1").arg(personId);  // âœ… %1, not '%1'

    QSqlQuery query;
    if (!query.exec(sql)) {
        LogE("DB query failed: %s\n", 
             query.lastError().text().toStdString().c_str());
        return false;
    }
    if (query.next()) {
        info.personid = query.value("personid").toInt();  // âœ… toInt()
        info.name = query.value("name").toString();
        info.idcard = query.value("idcard").toString();
        info.iccard = query.value("iccard").toString();
        info.sex = query.value("sex").toString();
        info.persontype = query.value("persontype").toInt();
        return true;
    }
    return false;
}

// Add this function to RegisteredFacesDB.cpp
// (Helper function to find UUID by employee ID)
QString RegisteredFacesDB::findUuidByEmployeeId(const QString &employeeId)
{
    Q_D(RegisteredFacesDB);
    QMutexLocker lock(&d->sync);
    
    LogD("%s %s[%d] Looking for UUID for employee ID: %s\n", 
         __FILE__, __FUNCTION__, __LINE__, employeeId.toStdString().c_str());
    
    // First check in-memory data
    for (int i = 0; i < d->mPersons.size(); i++) {
        if (d->mPersons[i].idcard == employeeId) {
            LogD("%s %s[%d] Found UUID in memory: %s for employee ID: %s\n", 
                 __FILE__, __FUNCTION__, __LINE__, d->mPersons[i].uuid.toStdString().c_str(), employeeId.toStdString().c_str());
            return d->mPersons[i].uuid;
        }
    }
    
    // If not found in memory, check database
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    query.prepare("SELECT uuid FROM person WHERE idcardnum = ?");
    query.bindValue(0, employeeId);
    
    if (query.exec() && query.next()) {
        QString uuid = query.value("uuid").toString();
        LogD("%s %s[%d] Found UUID in database: %s for employee ID: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, uuid.toStdString().c_str(), employeeId.toStdString().c_str());
        return uuid;
    }
    
    LogE("%s %s[%d] UUID not found for employee ID: %s\n", 
         __FILE__, __FUNCTION__, __LINE__, employeeId.toStdString().c_str());
    return QString();
}

void RegisteredFacesDB::startEventDrivenSync()
{
    if (m_syncTimerActive) {
        LogD("%s %s[%d] Event-driven sync already active\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    
    LogD("%s %s[%d] === STARTING EVENT-DRIVEN SYNC TIMER ===\n", __FILE__, __FUNCTION__, __LINE__);
    
    // Create timer only when needed
    if (!m_offlineSyncTimer) {
        m_offlineSyncTimer = new QTimer(this);
        connect(m_offlineSyncTimer, &QTimer::timeout, this, &RegisteredFacesDB::onEventDrivenSyncTimeout);
    }
    
    m_syncTimerActive = true;
    m_offlineSyncTimer->start(30000); // 30 seconds
    
    LogD("%s %s[%d] Event-driven sync timer started (30s interval)\n", __FILE__, __FUNCTION__, __LINE__);
}

// Stop event-driven sync timer (when all records are synced)
void RegisteredFacesDB::stopEventDrivenSync()
{
    if (!m_syncTimerActive || !m_offlineSyncTimer) {
        return;
    }
    
    LogD("%s %s[%d] === STOPPING EVENT-DRIVEN SYNC TIMER (no more offline records) ===\n", 
         __FILE__, __FUNCTION__, __LINE__);
    
    m_syncTimerActive = false;
    m_offlineSyncTimer->stop();
    
    LogD("%s %s[%d] Event-driven sync timer stopped\n", __FILE__, __FUNCTION__, __LINE__);
}

// Timer timeout handler - only runs when offline records exist
void RegisteredFacesDB::onEventDrivenSyncTimeout()
{
    LogD("%s %s[%d] === EVENT-DRIVEN SYNC TIMEOUT TRIGGERED ===\n", __FILE__, __FUNCTION__, __LINE__);
    
    // Check if we still have pending records
    QList<PERSONS_t> pendingRecords = getPendingUploadRecords();
    if (pendingRecords.isEmpty()) {
        LogD("%s %s[%d] No more pending records, stopping sync timer\n", __FILE__, __FUNCTION__, __LINE__);
        stopEventDrivenSync();
        return;
    }
    
    LogD("%s %s[%d] Found %d pending records to sync\n", __FILE__, __FUNCTION__, __LINE__, pendingRecords.size());
    
    // Check network availability
    if (!checkNetworkConnectivity()) {
        LogD("%s %s[%d] Network unavailable, keeping sync timer active\n", __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    
    // Attempt sync
    LogD("%s %s[%d] Network available, attempting sync...\n", __FILE__, __FUNCTION__, __LINE__);
    bool syncSuccess = syncPendingRecordsToServer();
    
    if (syncSuccess) {
        LogD("%s %s[%d] Sync successful! Checking if more records remain...\n", __FILE__, __FUNCTION__, __LINE__);
        
        // Check if all records are now uploaded
        QList<PERSONS_t> remainingRecords = getPendingUploadRecords();
        if (remainingRecords.isEmpty()) {
            LogD("%s %s[%d] All records synced successfully, stopping timer\n", __FILE__, __FUNCTION__, __LINE__);
            stopEventDrivenSync();
        } else {
            LogD("%s %s[%d] %d records still pending, keeping timer active\n", 
                 __FILE__, __FUNCTION__, __LINE__, remainingRecords.size());
        }
    } else {
        LogE("%s %s[%d] Sync failed, keeping timer active for retry\n", __FILE__, __FUNCTION__, __LINE__);
    }
}

bool RegisteredFacesDB::sendCroppedImageToServer(const QString &employeeId, const QString &userName, 
                                               const QString &icCard, const QString &userSex, 
                                               const QString &department, const QString &timeOfAccess, 
                                               const QByteArray &faceFeature)
{
    LogD("%s %s[%d] ============= sendCroppedImageToServer (ONLY NEW OFFLINE USERS) =============\n", __FILE__, __FUNCTION__, __LINE__);
    
    QString uuid = findUuidByEmployeeId(employeeId);
    if (uuid.isEmpty()) {
        LogE("%s %s[%d] ERROR: Could not find UUID for employee: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, employeeId.toStdString().c_str());
        return false;
    }
    
    // Validate image and convert to base64
    if (!validateCroppedImageExists(employeeId)) {
        LogE("%s %s[%d] ERROR: Cropped image validation failed\n", __FILE__, __FUNCTION__, __LINE__);
        return false;
    }
    
    QByteArray base64ImageData = convertCroppedImageToBase64(employeeId);
    if (base64ImageData.isEmpty()) {
        LogE("%s %s[%d] ERROR: Base64 conversion failed\n", __FILE__, __FUNCTION__, __LINE__);
        return false;
    }
    
    // Check network and server availability
    bool isNetworkAvailable = checkNetworkConnectivity();
    if (!isNetworkAvailable) {
        LogD("%s %s[%d] NETWORK OFFLINE: Marking THIS SPECIFIC USER for offline sync\n", __FILE__, __FUNCTION__, __LINE__);
        bool stored = markUserAsPendingOfflineUpload(uuid, employeeId);
        if (stored) {
            startEventDrivenSync();
        }
        return stored;
    }
    
    ConnHttpServerThread* serverThread = ConnHttpServerThread::GetInstance();
    if (!serverThread) {
        LogE("%s %s[%d] SERVER UNAVAILABLE: Marking THIS SPECIFIC USER for offline sync\n", __FILE__, __FUNCTION__, __LINE__);
        bool stored = markUserAsPendingOfflineUpload(uuid, employeeId);
        if (stored) {
            startEventDrivenSync();
        }
        return stored;
    }
    
    // *** PRESERVE ORIGINAL ONLINE UPLOAD LOGIC ***
    bool serverSendResult = false;
    try {
        LogD("%s %s[%d] Attempting online upload for: %s\n", __FILE__, __FUNCTION__, __LINE__, employeeId.toStdString().c_str());
        
        serverSendResult = serverThread->sendUserToServer(
            employeeId, userName, employeeId, icCard, userSex, department, timeOfAccess, base64ImageData
        );
        
        if (serverSendResult) {
            LogD("%s %s[%d] *** SUCCESS: Online upload successful for %s ***\n", 
                 __FILE__, __FUNCTION__, __LINE__, employeeId.toStdString().c_str());
            // Mark as successfully uploaded
            markUserAsSuccessfullyUploaded(uuid);
            return true;
        } else {
            LogE("%s %s[%d] Online upload failed for %s\n", __FILE__, __FUNCTION__, __LINE__, employeeId.toStdString().c_str());
        }
        
    } catch (...) {
        LogE("%s %s[%d] Exception during online upload for %s\n", __FILE__, __FUNCTION__, __LINE__, employeeId.toStdString().c_str());
    }
    
    // Store for offline sync if server fails
    LogE("%s %s[%d] Marking %s for offline sync due to server failure\n", __FILE__, __FUNCTION__, __LINE__, employeeId.toStdString().c_str());
    
    bool stored = markUserAsPendingOfflineUpload(uuid, employeeId);
    if (stored) {
        startEventDrivenSync();
    }
    return stored;
}


bool RegisteredFacesDB::checkNetworkConnectivity()
{
    LogD("%s %s[%d] === Enhanced Network Connectivity Check ===\n", __FILE__, __FUNCTION__, __LINE__);
    
    // Method 1: Check server thread availability first
    ConnHttpServerThread* serverThread = ConnHttpServerThread::GetInstance();
    if (!serverThread) {
        LogD("%s %s[%d] Server thread not available\n", __FILE__, __FUNCTION__, __LINE__);
        return false;
    }
    
    // Method 2: Try a simple server health check if available
    // (You could add a ping/health endpoint to your server)
    if (serverThread->isServerReachable()) { // You may need to implement this
        LogD("%s %s[%d] Server is reachable\n", __FILE__, __FUNCTION__, __LINE__);
        return true;
    }
    
    // Method 3: Basic network interface check (Linux)
#ifdef Q_OS_LINUX
    // Check for any active network interface with IP
    QStringList interfaces = {"eth0", "eth1", "wlan0", "wlan1", "enp0s3", "ens33"};
    
    for (const QString& iface : interfaces) {
        QString cmd = QString("ip addr show %1 | grep 'inet ' >/dev/null 2>&1").arg(iface);
        if (system(cmd.toStdString().c_str()) == 0) {
            LogD("%s %s[%d] Network interface %s has IP address\n", 
                 __FILE__, __FUNCTION__, __LINE__, iface.toStdString().c_str());
            return true;
        }
    }
    
    LogD("%s %s[%d] No network interfaces with IP found\n", __FILE__, __FUNCTION__, __LINE__);
#endif
    
    LogD("%s %s[%d] Network connectivity: NOT AVAILABLE\n", __FILE__, __FUNCTION__, __LINE__);
    return false;
}

// Helper function to update only uploadStatus column
bool RegisteredFacesDB::updatePersonUploadStatus(const QString &uuid, int uploadStatus)
{
    LogD("%s %s[%d] === Updating uploadStatus for UUID: %s to: %d ===\n", 
         __FILE__, __FUNCTION__, __LINE__, uuid.toStdString().c_str(), uploadStatus);
    
    Q_D(RegisteredFacesDB);
    QMutexLocker lock(&d->sync);
    
    // Ensure database schema is up to date
    createOrUpdateDatabaseSchema();
    
    // Update database
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    query.prepare("UPDATE person SET uploadStatus = ? WHERE uuid = ?");
    query.bindValue(0, uploadStatus);
    query.bindValue(1, uuid);
    
    bool success = query.exec();
    
    if (success) {
        // Update in-memory list
        for (int i = 0; i < d->mPersons.size(); i++) {
            if (d->mPersons[i].uuid == uuid) {
                d->mPersons[i].uploadStatus = uploadStatus;
                break;
            }
        }
        LogD("%s %s[%d] Upload status updated successfully\n", __FILE__, __FUNCTION__, __LINE__);
    } else {
        QSqlError error = query.lastError();
        LogE("%s %s[%d] Failed to update uploadStatus: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, error.text().toStdString().c_str());
    }
    
    return success;
}

bool RegisteredFacesDB::markUserAsPendingOfflineUpload(const QString &uuid, const QString &employeeId)
{
    LogD("%s %s[%d] === MARKING SPECIFIC USER AS PENDING: %s (%s) ===\n", 
         __FILE__, __FUNCTION__, __LINE__, employeeId.toStdString().c_str(), uuid.toStdString().c_str());
    
    Q_D(RegisteredFacesDB);
    QMutexLocker lock(&d->sync);
    
    // Update database with explicit pending status
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    query.prepare("UPDATE person SET uploadStatus = 0 WHERE uuid = ?");
    query.bindValue(0, uuid);
    
    if (!query.exec()) {
        QSqlError error = query.lastError();
        LogE("%s %s[%d] Failed to mark user as pending: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, error.text().toStdString().c_str());
        return false;
    }
    
    // Update in-memory
    for (int i = 0; i < d->mPersons.size(); i++) {
        if (d->mPersons[i].uuid == uuid) {
            d->mPersons[i].uploadStatus = 0;
            break;
        }
    }
    
    // Log this specific offline user
    logOfflineUser(employeeId, uuid);
    
    LogD("%s %s[%d] Successfully marked user %s as pending offline upload\n", 
         __FILE__, __FUNCTION__, __LINE__, employeeId.toStdString().c_str());
    
    return true;
}

bool RegisteredFacesDB::markUserAsSuccessfullyUploaded(const QString &uuid)
{
    LogD("%s %s[%d] === MARKING USER AS SUCCESSFULLY UPLOADED: %s ===\n", 
         __FILE__, __FUNCTION__, __LINE__, uuid.toStdString().c_str());
    
    Q_D(RegisteredFacesDB);
    QMutexLocker lock(&d->sync);
    
    // Update database
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    query.prepare("UPDATE person SET uploadStatus = 1 WHERE uuid = ?");
    query.bindValue(0, uuid);
    
    if (!query.exec()) {
        QSqlError error = query.lastError();
        LogE("%s %s[%d] Failed to mark user as uploaded: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, error.text().toStdString().c_str());
        return false;
    }
    
    // Update in-memory
    for (int i = 0; i < d->mPersons.size(); i++) {
        if (d->mPersons[i].uuid == uuid) {
            d->mPersons[i].uploadStatus = 1;
            break;
        }
    }
    
    return true;
}

void RegisteredFacesDB::logOfflineUser(const QString &employeeId, const QString &uuid)
{
    QString offlineLogFile = "/mnt/user/sync_data/offline_users.log";
    
    // Ensure directory exists
    QDir dir;
    dir.mkpath("/mnt/user/sync_data/");
    
    QFile file(offlineLogFile);
    if (file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream stream(&file);
        QString timestamp = QDateTime::currentDateTime().toString("yyyy/MM/dd HH:mm:ss");
        stream << QString("%1|%2|%3|MARKED_OFFLINE\n")
                  .arg(timestamp)
                  .arg(employeeId)
                  .arg(uuid);
        file.close();
        
        LogD("%s %s[%d] Logged offline user: %s\n", __FILE__, __FUNCTION__, __LINE__, employeeId.toStdString().c_str());
    }
}

bool RegisteredFacesDB::ensureExistingUsersAreMarkedAsUploaded()
{
    LogD("%s %s[%d] === ENSURING EXISTING USERS ARE MARKED AS UPLOADED ===\n", __FILE__, __FUNCTION__, __LINE__);
    
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    
    // Mark ALL NULL uploadStatus as uploaded (these are existing users)
    QString updateSql = "UPDATE person SET uploadStatus = 1 WHERE uploadStatus IS NULL";
    
    if (query.exec(updateSql)) {
        int updatedCount = query.numRowsAffected();
        LogD("%s %s[%d] Marked %d existing users as uploaded\n", 
             __FILE__, __FUNCTION__, __LINE__, updatedCount);
        
        if (updatedCount > 0) {
            // Force sync to disk
#ifdef Q_OS_LINUX
            system("sync");
#endif
        }
        
        return true;
    } else {
        QSqlError error = query.lastError();
        LogE("%s %s[%d] Failed to mark existing users as uploaded: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, error.text().toStdString().c_str());
        return false;
    }
}

// Function to get all records pending upload (uploadStatus = "0") - PersonRecordToDB pattern
QList<PERSONS_t> RegisteredFacesDB::getPendingUploadRecords()
{
    LogD("%s %s[%d] === Getting ONLY pending upload records (uploadStatus=0) ===\n", __FILE__, __FUNCTION__, __LINE__);
    
    Q_D(RegisteredFacesDB);
    QMutexLocker lock(&d->sync);
    
    QList<PERSONS_t> pendingRecords;
    
    // Query only records with uploadStatus = 0 (pending)
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    QString strSql = "SELECT * FROM person WHERE uploadStatus = 0";
    
    if (query.exec(strSql)) {
        while (query.next()) {
            PERSONS_t person;
            person.uuid = query.value("uuid").toString();
            person.name = query.value("name").toString();
            person.idcard = query.value("idcardnum").toString();
            person.iccard = query.value("iccardnum").toString();
            person.sex = query.value("sex").toString();
            person.department = query.value("department").toString();
            person.timeOfAccess = query.value("time_of_access").toString();
            person.uploadStatus = query.value("uploadStatus").toInt();
            person.feature = query.value("feature").toByteArray();
            person.attendanceMode = query.value("attendanceMode").toString();
            person.tenantId = query.value("tenantId").toString();
            person.id = query.value("id").toString();
            person.status = query.value("status").toString();
            
            // Only include if status is exactly 0 (pending)
            if (person.uploadStatus == 0) {
                pendingRecords.append(person);
                LogD("%s %s[%d] Found pending record: %s (ID: %s)\n", 
                     __FILE__, __FUNCTION__, __LINE__, 
                     person.name.toStdString().c_str(), 
                     person.idcard.toStdString().c_str());
            }
        }
    } else {
        QSqlError error = query.lastError();
        LogE("%s %s[%d] Database query failed: %s\n", __FILE__, __FUNCTION__, __LINE__, 
             error.text().toStdString().c_str());
    }
    
    LogD("%s %s[%d] Total pending upload records: %d\n", __FILE__, __FUNCTION__, __LINE__, pendingRecords.size());
    return pendingRecords;
}

// Function to sync all pending records when network becomes available - PersonRecordToDB pattern
bool RegisteredFacesDB::syncPendingRecordsToServer()
{
    LogD("%s %s[%d] === IMPROVED: Starting sync of ONLY pending records ===\n", __FILE__, __FUNCTION__, __LINE__);
    
    if (!checkNetworkConnectivity()) {
        LogD("%s %s[%d] Network not available, skipping sync\n", __FILE__, __FUNCTION__, __LINE__);
        return false;
    }
    
    ConnHttpServerThread* serverThread = ConnHttpServerThread::GetInstance();
    if (!serverThread) {
        LogE("%s %s[%d] Server thread not available for sync\n", __FILE__, __FUNCTION__, __LINE__);
        return false;
    }
    
    QList<PERSONS_t> pendingRecords = getPendingUploadRecords();
    if (pendingRecords.isEmpty()) {
        LogD("%s %s[%d] No pending records to sync\n", __FILE__, __FUNCTION__, __LINE__);
        return true;
    }
    
    LogD("%s %s[%d] Found %d pending records to sync\n", __FILE__, __FUNCTION__, __LINE__, pendingRecords.size());
    
    int successCount = 0;
    int failureCount = 0;
    
    for (const PERSONS_t& record : pendingRecords) {
        LogD("%s %s[%d] Processing: %s (%s)\n", __FILE__, __FUNCTION__, __LINE__, 
             record.name.toStdString().c_str(), record.idcard.toStdString().c_str());
        
        // Double-check this record is still pending (prevent race conditions)
        int currentStatus = getPersonUploadStatus(record.uuid);
        if (currentStatus != 0) {
            LogD("%s %s[%d] Record no longer pending, skipping: %s (status: %d)\n", 
                 __FILE__, __FUNCTION__, __LINE__, record.idcard.toStdString().c_str(), currentStatus);
            continue;
        }
        
        // Mark as uploading to prevent duplicate processing
        bool markResult = updatePersonUploadStatus(record.uuid, 2); // 2 = uploading
        if (!markResult) {
            LogE("%s %s[%d] Failed to mark record as uploading: %s\n", 
                 __FILE__, __FUNCTION__, __LINE__, record.idcard.toStdString().c_str());
            failureCount++;
            continue;
        }
        
        // Convert image to base64
        QByteArray base64ImageData = convertCroppedImageToBase64(record.idcard);
        if (base64ImageData.isEmpty()) {
            LogE("%s %s[%d] Failed to convert image for %s, reverting to pending\n", 
                 __FILE__, __FUNCTION__, __LINE__, record.idcard.toStdString().c_str());
            updatePersonUploadStatus(record.uuid, 0); // Back to pending
            failureCount++;
            continue;
        }
        
        // Attempt server upload
        bool syncResult = false;
        try {
            LogD("%s %s[%d] Uploading to server: %s\n", __FILE__, __FUNCTION__, __LINE__, 
                 record.name.toStdString().c_str());
            
            syncResult = serverThread->sendUserToServer(
                record.idcard, record.name, record.idcard, record.iccard, 
                record.sex, record.department, record.timeOfAccess, base64ImageData
            );
            
            if (syncResult) {
                // SUCCESS: Mark as uploaded (clear status)
                bool clearResult = markRecordAsUploaded(record.uuid);
                if (clearResult) {
                    successCount++;
                    LogD("%s %s[%d] âœ“ SUCCESS: Uploaded and marked as complete: %s\n", 
                         __FILE__, __FUNCTION__, __LINE__, record.name.toStdString().c_str());
                } else {
                    LogE("%s %s[%d] Upload succeeded but failed to clear status: %s\n", 
                         __FILE__, __FUNCTION__, __LINE__, record.name.toStdString().c_str());
                    failureCount++;
                }
            } else {
                // FAILURE: Revert to pending for retry
                LogE("%s %s[%d] Server rejected upload, reverting to pending: %s\n", 
                     __FILE__, __FUNCTION__, __LINE__, record.name.toStdString().c_str());
                updatePersonUploadStatus(record.uuid, 0); // Back to pending
                failureCount++;
            }
            
        } catch (...) {
            LogE("%s %s[%d] Exception during upload, reverting to pending: %s\n", 
                 __FILE__, __FUNCTION__, __LINE__, record.name.toStdString().c_str());
            updatePersonUploadStatus(record.uuid, 0); // Back to pending
            failureCount++;
        }
        
        // Small delay to prevent server overload
        QThread::msleep(100);
    }
    
    LogD("%s %s[%d] === SYNC COMPLETED: %d success, %d failures ===\n", 
         __FILE__, __FUNCTION__, __LINE__, successCount, failureCount);
    
    return (successCount > 0);
}

bool RegisteredFacesDB::markRecordAsUploaded(const QString &uuid)
{
    LogD("%s %s[%d] === MARKING RECORD AS UPLOADED: %s ===\n", __FILE__, __FUNCTION__, __LINE__, uuid.toStdString().c_str());
    
    Q_D(RegisteredFacesDB);
    QMutexLocker lock(&d->sync);
    
    // Clear upload status in database (set to NULL or 1)
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    query.prepare("UPDATE person SET uploadStatus = 1 WHERE uuid = ?"); // 1 = uploaded
    query.bindValue(0, uuid);
    
    if (!query.exec()) {
        QSqlError error = query.lastError();
        LogE("%s %s[%d] Failed to mark record as uploaded in database: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, error.text().toStdString().c_str());
        return false;
    }
    
    // Update in-memory data
    for (int i = 0; i < d->mPersons.size(); i++) {
        if (d->mPersons[i].uuid == uuid) {
            d->mPersons[i].uploadStatus = 1; // 1 = uploaded
            break;
        }
    }
    
    LogD("%s %s[%d] âœ“ Record marked as uploaded: %s\n", 
         __FILE__, __FUNCTION__, __LINE__, uuid.toStdString().c_str());
    
    return true;
}


// Function to get records with filter for upload flag - PersonRecordToDB pattern
QList<PERSONS_t> RegisteredFacesDB::GetPersonDataByNameWithUploadFilter(int nCurrPage, int nPerPage, QString name, bool filterUploadFlag)
{
    Q_D(RegisteredFacesDB);
    QMutexLocker lock(&d->sync);
    
    LogD("%s %s[%d] === Getting person data by name with upload filter - PersonRecordToDB pattern ===\n", __FILE__, __FUNCTION__, __LINE__);
    
    QList<PERSONS_t> result;
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    QString strSql;
    
    if (filterUploadFlag == true) {
        strSql = QString("SELECT * FROM person WHERE name='%1' AND uploadStatus='0' ORDER BY createtime DESC LIMIT %2 OFFSET %3");
    } else {
        strSql = QString("SELECT * FROM person WHERE name='%1' ORDER BY createtime DESC LIMIT %2 OFFSET %3");
    }
    
    strSql = strSql.arg(name);
    strSql = strSql.arg(nPerPage);
    strSql = strSql.arg((nCurrPage - 1) * nPerPage);
    
    LogD("%s %s[%d] SQL: %s\n", __FILE__, __FUNCTION__, __LINE__, strSql.toStdString().c_str());
    
    if (query.exec(strSql) == false) {
        QSqlError error = query.lastError();
        LogE("%s %s[%d] SQL failed: %s\n", __FILE__, __FUNCTION__, __LINE__, strSql.toStdString().c_str());
        LogE("%s %s[%d] Error: %s\n", __FILE__, __FUNCTION__, __LINE__, error.text().toStdString().c_str());
        return result;
    }
    
    while (query.next()) {
        PERSONS_t person;
        person.uuid = query.value("uuid").toString();
        person.name = query.value("name").toString();
        person.idcard = query.value("idcardnum").toString();
        person.iccard = query.value("iccardnum").toString();
        person.sex = query.value("sex").toString();
        person.department = query.value("department").toString();
        person.timeOfAccess = query.value("time_of_access").toString();
        person.createtime = query.value("createtime").toString();
        person.uploadStatus = query.value("uploadStatus").toInt();
        person.feature = query.value("feature").toByteArray();
        
        result.append(person);
    }
    
    LogD("%s %s[%d] Retrieved %d records\n", __FILE__, __FUNCTION__, __LINE__, result.size());
    return result;
}

int RegisteredFacesDB::getPersonUploadStatus(const QString &uuid)
{
    Q_D(RegisteredFacesDB);
    QMutexLocker lock(&d->sync);
    
    // Check in-memory first (faster)
    for (int i = 0; i < d->mPersons.size(); i++) {
        if (d->mPersons[i].uuid == uuid) {
            return d->mPersons[i].uploadStatus;
        }
    }
    
    // Check database if not found in memory
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    query.prepare("SELECT uploadStatus FROM person WHERE uuid = ?");
    query.bindValue(0, uuid);
    
    if (query.exec() && query.next()) {
        QVariant statusValue = query.value("uploadStatus");
        if (statusValue.isNull()) {
            return 1; // NULL means uploaded
        }
        return statusValue.toInt();
    }
    
    return -1; // Not found
}


QByteArray RegisteredFacesDB::convertCroppedImageToBase64(const QString &employeeId)
{
    LogD("%s %s[%d] ========== convertCroppedImageToBase64 DEBUG START ==========\n", __FILE__, __FUNCTION__, __LINE__);
    LogD("%s %s[%d] Converting image to base64 for employeeId: '%s'\n", __FILE__, __FUNCTION__, __LINE__, 
         employeeId.toStdString().c_str());
    
    QString imagePath = QString("/mnt/user/reg_face_image/%1.jpg").arg(employeeId);
    LogD("%s %s[%d] Image path: %s\n", __FILE__, __FUNCTION__, __LINE__, imagePath.toStdString().c_str());
    
    // Check file exists
    QFileInfo fileInfo(imagePath);
    if (!fileInfo.exists()) {
        LogE("%s %s[%d] ERROR: Image file does not exist: %s\n", __FILE__, __FUNCTION__, __LINE__, 
             imagePath.toStdString().c_str());
        return QByteArray();
    }
    
    LogD("%s %s[%d] Image file exists, size: %lld bytes\n", __FILE__, __FUNCTION__, __LINE__, fileInfo.size());
    
    // Single file operation - no repeated opens
    QFile imageFile(imagePath);
    if (!imageFile.open(QIODevice::ReadOnly)) {
        LogE("%s %s[%d] ERROR: Cannot open image file for reading\n", __FILE__, __FUNCTION__, __LINE__);
        LogE("%s %s[%d] Error details: %s\n", __FILE__, __FUNCTION__, __LINE__, 
             imageFile.errorString().toStdString().c_str());
        return QByteArray();
    }
    
    LogD("%s %s[%d] Image file opened successfully\n", __FILE__, __FUNCTION__, __LINE__);
    
    QByteArray imageData = imageFile.readAll();
    imageFile.close();
    
    if (imageData.isEmpty()) {
        LogE("%s %s[%d] ERROR: Read image data is empty\n", __FILE__, __FUNCTION__, __LINE__);
        return QByteArray();
    }
    
    LogD("%s %s[%d] Image data read: %d bytes\n", __FILE__, __FUNCTION__, __LINE__, imageData.size());
    
    // Convert to base64 in single operation
    QByteArray base64Data = imageData.toBase64();
    
    LogD("%s %s[%d] Base64 conversion completed\n", __FILE__, __FUNCTION__, __LINE__);
    LogD("%s %s[%d] Original size: %d bytes -> Base64 size: %d bytes\n", __FILE__, __FUNCTION__, __LINE__, 
         imageData.size(), base64Data.size());
    
    if (!base64Data.isEmpty()) {
        LogD("%s %s[%d] Base64 preview (first 30 chars): %.30s\n", __FILE__, __FUNCTION__, __LINE__, 
             base64Data.constData());
    }
    
    LogD("%s %s[%d] ========== convertCroppedImageToBase64 DEBUG END ==========\n", __FILE__, __FUNCTION__, __LINE__);
    
    return base64Data;
}



bool RegisteredFacesDB::validateBase64PersistenceAfterCreation(const QString &employeeId)
{
    LogD("%s %s[%d] === CHECKING BASE64 PERSISTENCE ===\n", __FILE__, __FUNCTION__, __LINE__);
    
    // Wait a moment to ensure file system operations complete
    QThread::msleep(100);
    
    QString croppedImagePath = QString("/mnt/user/reg_face_image/%1.jpg").arg(employeeId);
    QFileInfo fileInfo(croppedImagePath);
    
    if (!fileInfo.exists()) {
        LogE("%s %s[%d] ERROR: Cropped image disappeared after creation!\n", __FILE__, __FUNCTION__, __LINE__);
        return false;
    }
    
    if (fileInfo.size() < 1000) {
        LogE("%s %s[%d] ERROR: Cropped image size too small: %lld bytes\n", __FILE__, __FUNCTION__, __LINE__, 
             fileInfo.size());
        return false;
    }
    
    // Test base64 conversion
    QByteArray testBase64 = convertCroppedImageToBase64(employeeId);
    if (testBase64.isEmpty()) {
        LogE("%s %s[%d] ERROR: Base64 conversion fails after file creation\n", __FILE__, __FUNCTION__, __LINE__);
        return false;
    }
    
    LogD("%s %s[%d] SUCCESS: Base64 persistence validated\n", __FILE__, __FUNCTION__, __LINE__);
    return true;
}

// Helper function to validate cropped image exists
bool RegisteredFacesDB::validateCroppedImageExists(const QString &employeeId)
{
    LogD("%s %s[%d] Validating cropped image exists for: %s\n", __FILE__, __FUNCTION__, __LINE__, 
         employeeId.toStdString().c_str());
    
    QString croppedImagePath = QString("/mnt/user/reg_face_image/%1.jpg").arg(employeeId);
    QFileInfo imageInfo(croppedImagePath);
    
    if (!imageInfo.exists()) {
        LogE("%s %s[%d] Cropped image does not exist: %s\n", __FILE__, __FUNCTION__, __LINE__, 
             croppedImagePath.toStdString().c_str());
        return false;
    }
    
    if (imageInfo.size() < 1000) {
        LogE("%s %s[%d] Cropped image too small: %lld bytes\n", __FILE__, __FUNCTION__, __LINE__, 
             imageInfo.size());
        return false;
    }
    
    LogD("%s %s[%d] Cropped image validation passed: %lld bytes\n", __FILE__, __FUNCTION__, __LINE__, 
         imageInfo.size());
    return true;
}

void RegisteredFacesDB::setThanthreshold(const float &value)
{
    Q_D(RegisteredFacesDB);
    d->mThanthreshold = value;
}

bool RegisteredFacesDB::selectICcardPerson(const QString &iccard, PERSONS_s &s)
{
    s.reader = true;
    s.iccard = QString();
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    query.exec(QString("select *from person ORDER BY iccardnum='%1' DESC LIMIT 1").arg(iccard));
    while(query.next())
    {
        s.feature = query.value("feature").toByteArray();
        s.name = query.value("name").toString();
        s.sex = query.value("sex").toString();
        s.idcard = query.value("idcardnum").toString();
        s.iccard = query.value("iccardnum").toString();
        s.uuid= query.value("uuid").toString();
        s.persontype = query.value("persontype").toInt();
        s.personid = query.value("personid").toInt();
        s.gids = query.value("gids").toString();
        s.pids = query.value("aids").toString();
        s.createtime = query.value("createtime").toString();
        s.access_level = query.value("access_level").toInt();
        return true;
    }
    return false;
}

bool RegisteredFacesDB::selectIDcardPerson(const QString &idCard, PERSONS_s &s)
{
    s.reader = true;
    s.idcard = QString();
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    query.exec(QString("select *from person ORDER BY idcardnum='%1' DESC LIMIT 1").arg(idCard));
    while(query.next())
    {
        s.feature = query.value("feature").toByteArray();
        s.name = query.value("name").toString();
        s.sex = query.value("sex").toString();
        s.idcard = query.value("idcardnum").toString();
        s.iccard = query.value("iccardnum").toString();
        s.uuid= query.value("uuid").toString();
        s.persontype = query.value("persontype").toInt();
        s.personid = query.value("personid").toInt();
        s.gids = query.value("gids").toString();
        s.pids = query.value("aids").toString();
        s.createtime = query.value("createtime").toString();
        s.access_level = query.value("access_level").toInt();
        return true;
    }
    return false;
}

bool RegisteredFacesDB::CheckPassageOfTime(QString person_uuid)
{
	/**å…ˆæŸ¥è¯¢ç”¨æˆ·è¡¨é‡Œé¢ï¼Œç”¨æˆ·è‡ªå·±çš„é€šè¡Œæƒé™*/
    QString strStartTime;
    QString strCurTime;
    QString strEndTime;

#if 0
case 1: person é€šè¡Œ  PassageTime é€šè¡Œ  é¢„æœŸ:é€šè¡Œ 
       1.1 person ä¸ºç©º
	   1.2 person é€šè¡Œ      00:10,10:30,1,1,1,1,1,1,1
	   1.3 PassageTime  ä¸ºç©º
	   1.4 person é€šè¡Œ 	   
case 2  person ç¦è¡Œ  PassageTime é€šè¡Œ  é¢„æœŸ:ç¦è¡Œ
       2.1 person ç¦è¡Œ
	   2.2  PassageTime  ä¸ºç©º
	   2.3  PassageTime é€šè¡Œ  00:00|23:59,1,1,1,1,1,1,1
case 3  person é€šè¡Œ  PassageTime ç¦è¡Œ  é¢„æœŸ:ç¦è¡Œ
       3.1 person ä¸ºç©º
	   3.2 person é€šè¡Œ 
	   3.3  PassageTime ç¦è¡Œ  00:00|23:59,0,0,0,0,0,0,0
case 4  person ç¦è¡Œ  PassageTime ç¦è¡Œ  é¢„æœŸ:ç¦è¡Œ
       4.1 person ç¦è¡Œ
	   4.2  PassageTime ç¦è¡Œ 

#endif 

   //  bool bAccessPerson = false;
	if(person_uuid.size() > 3)
	{
		Q_D(RegisteredFacesDB);

        //step 1:
		for(int i = 0; i < d->mPersons.size(); i++)
		{
			auto &t = d->mPersons.at(i);
			if(t.uuid == person_uuid)
			{
				QStringList sections = t.timeOfAccess.split(",");
				LogD("%s %s[%d] uuid %s timeOfAccess %s  sections=%d\n",__FILE__,__FUNCTION__,__LINE__,t.uuid.toStdString().c_str(),t.timeOfAccess.toStdString().c_str(),sections.size());
                if (sections.size() <= 1) 
                {
                   //bAccessPerson =true;
                }
				else if (sections.size() == 9)  //å‘¨å¾ªçŽ¯
				{
					strStartTime = sections.at(0);
					strEndTime = sections.at(1);

					QTime curTime = QTime::currentTime();
					QTime stateTime = QTime::fromString(strStartTime, "hh:mm");
					QTime endTime = QTime::fromString(strEndTime, "hh:mm");

					if (curTime <= endTime && curTime >= stateTime)
					{
						switch (QDate::currentDate().dayOfWeek())
						{
						case 1:
							return (sections[2] == "1");
						case 2:
							return (sections[3] == "1");
						case 3:
							return (sections[4] == "1");
						case 4:
							return (sections[5] == "1");
						case 5:
							return (sections[6] == "1");
						case 6:
							return (sections[7] == "1");
						case 7:
							return (sections[8] == "1");
						}
					} 
                    
					//LogD("%s %s[%d] uuid %s time_of_access %s \n", __FILE__, __FUNCTION__, __LINE__, t.uuid.toStdString().c_str(),t.timeOfAccess.toStdString().c_str());
                    return false;
				} else  //æ—¶é—´èŒƒå›´
				{
					//t.timeOfAccess 2000/01/01 00:00;2000/01/01 00:00
					QStringList sections = t.timeOfAccess.split(";");
					if (sections.size() == 2)
					{
						QDateTime curTimer = QDateTime::currentDateTime();
						QDateTime startTimer = QDateTime::fromString(sections[0], "yyyy/MM/dd hh:mm");
						QDateTime endTimer = QDateTime::fromString(sections[1], "yyyy/MM/dd hh:mm");
						if (curTimer <= endTimer && curTimer >= startTimer)
						{
							return true;
						} else
						{
					        strCurTime = curTimer.toString(QString::fromLatin1("yyyy/MM/dd hh:mm"));
					        strStartTime = startTimer.toString(QString::fromLatin1("yyyy/MM/dd hh:mm"));
					        strEndTime = endTimer.toString(QString::fromLatin1("yyyy/MM/dd hh:mm"));

					        LogD("%s %s[%d] start %s cur %s end %s \n",__FILE__,__FUNCTION__,__LINE__,strStartTime.toStdString().c_str(),
					        		strCurTime.toStdString().c_str(),strEndTime.toStdString().c_str());
							return false;
						}
					}
				}
			}
		}
	}
  
    //step 2:
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    query.exec("select *from PassageTime");

    strCurTime = "";
    strStartTime = "";
    strEndTime = "";

    while(query.next())
    {
    	QTime curTime = QTime::currentTime();
    	QTime stateTime = QTime::fromString(query.value("stateTimer").toString(), "hh:mm");
    	QTime endTime =  QTime::fromString(query.value("endTimer").toString(), "hh:mm");
        strCurTime = curTime.toString(QString::fromLatin1("hh:mm"));
        strStartTime = stateTime.toString(QString::fromLatin1("hh:mm"));
        strEndTime = endTime.toString(QString::fromLatin1("hh:mm"));

        if(curTime<=endTime && curTime>=stateTime)
        {
            switch(QDate::currentDate().dayOfWeek())
            {
            case 1: return query.value("Monday").toBool();
            case 2: return query.value("Tuesday").toBool();
            case 3: return query.value("Wednesday").toBool();
            case 4: return query.value("Thursday").toBool();
            case 5: return query.value("Friday").toBool();
            case 6: return query.value("Saturday").toBool();
            case 7: return query.value("Sunday").toBool();
            }
        }
    }

    //ä¸¤æ­¥æ ¡éªŒ ç»“æŸåŽ
    //if (!bAccessPerson) 
    //  return false; //æœ‰ä¸€é¡¹ç¦æ­¢
        

    //é»˜è®¤æ²¡æœ‰é€šè¡Œæƒé™ï¼Œåˆ™ä»»æ„æ—¶é—´æ®µå‡å¯é€šè¡Œ
    if(strStartTime.size() <= 0 || strEndTime.size() <= 0)
    {
    	return true;
    }


    LogD("%s %s[%d] start %s cur %s end %s ,day %d \n",__FILE__,__FUNCTION__,__LINE__,strStartTime.toStdString().c_str(),
    		strCurTime.toStdString().c_str(),strEndTime.toStdString().c_str(),QDate::currentDate().dayOfWeek());
    return false;
}


static inline int getPersonMaxid()
{
    QMutexLocker lock(&g_personIdMutex);
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    query.exec("select personid from person ORDER BY personid DESC LIMIT 1");
    while(query.next())
    {
        int maxId = query.value("personid").toInt();
        return maxId >= 1 ? maxId + 1 : 1; // Return 1 if maxId is 0 or negative
    }
    return 1; // Default to 1 if no records exist
}

bool RegisteredFacesDB::ProcessFaceRegistrationFromCroppedImage(const QString &employeeId)
{
    LogD("%s %s[%d] === Starting ProcessFaceRegistrationFromCroppedImage ===\n", 
         __FILE__, __FUNCTION__, __LINE__);
    LogD("%s %s[%d] Employee ID: %s\n", __FILE__, __FUNCTION__, __LINE__, employeeId.toStdString().c_str());
    
    if (employeeId.isEmpty()) {
        LogE("%s %s[%d] Employee ID is empty\n", __FILE__, __FUNCTION__, __LINE__);
        return false;
    }
    
    // Check if cropped image exists
    QString croppedImagePath = QString("/mnt/user/reg_face_image/%1.jpg").arg(employeeId);
    QFile imageFile(croppedImagePath);
    if (!imageFile.exists()) {
        LogE("%s %s[%d] Cropped image not found: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, croppedImagePath.toStdString().c_str());
        return false;
    }
    
    LogD("%s %s[%d] Cropped image found: %s (size: %lld bytes)\n", 
         __FILE__, __FUNCTION__, __LINE__, croppedImagePath.toStdString().c_str(), imageFile.size());
    
    // Extract features from the cropped image
    return ExtractAndStoreFaceFeatures(employeeId, croppedImagePath);  // Correct
}

bool RegisteredFacesDB::ExtractAndStoreFaceFeatures(const QString &employeeId, const QString &croppedImagePath)
{
    LogD("%s %s[%d] === Starting ExtractAndStoreFaceFeatures ===\n", __FILE__, __FUNCTION__, __LINE__);
    LogD("%s %s[%d] Employee ID: %s, Image: %s\n", 
         __FILE__, __FUNCTION__, __LINE__, employeeId.toStdString().c_str(), croppedImagePath.toStdString().c_str());
    
    // Get BaiduFaceManager instance
    BaiduFaceManager* faceManager = (BaiduFaceManager*)qXLApp->GetAlgoFaceManager();
    if (!faceManager) {
        LogE("%s %s[%d] BaiduFaceManager not available\n", __FILE__, __FUNCTION__, __LINE__);
        return false;
    }
    
    // Extract face features from cropped image
    QByteArray faceFeature;
    double quality = 0.0;
    
    LogD("%s %s[%d] Extracting features from cropped image...\n", __FILE__, __FUNCTION__, __LINE__);
    bool extractResult = faceManager->extractFeaturesFromImagePath(croppedImagePath, faceFeature, quality);
    
    if (!extractResult) {
        LogE("%s %s[%d] Failed to extract features from cropped image\n", __FILE__, __FUNCTION__, __LINE__);
        return false;
    }
    
    if (faceFeature.isEmpty()) {
        LogE("%s %s[%d] Extracted feature is empty\n", __FILE__, __FUNCTION__, __LINE__);
        return false;
    }
    
    LogD("%s %s[%d] Features extracted successfully: %d bytes, quality: %f\n", 
         __FILE__, __FUNCTION__, __LINE__, faceFeature.size(), quality);
    
    // Read the cropped image data for database storage
    QFile imageFile(croppedImagePath);
    QByteArray imageData;
    if (imageFile.open(QIODevice::ReadOnly)) {
        imageData = imageFile.readAll();
        imageFile.close();
        LogD("%s %s[%d] Image data read: %d bytes\n", 
             __FILE__, __FUNCTION__, __LINE__, imageData.size());
    } else {
        LogE("%s %s[%d] Failed to read cropped image data\n", __FILE__, __FUNCTION__, __LINE__);
        return false;
    }
    
    // Find the user in database by employee ID
    QString uuid = findUuidByEmployeeId(employeeId);
    if (uuid.isEmpty()) {
        LogE("%s %s[%d] User not found in database for employee ID: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, employeeId.toStdString().c_str());
        return false;
    }
    
    LogD("%s %s[%d] Found user UUID: %s for employee ID: %s\n", 
         __FILE__, __FUNCTION__, __LINE__, uuid.toStdString().c_str(), employeeId.toStdString().c_str());
    
    // Update the user with face features and image data
    bool updateResult = UpdatePersonFaceFeature(uuid, faceFeature, imageData, "jpg");
    
    if (updateResult) {
        LogD("%s %s[%d] === Face features successfully stored in database ===\n", __FILE__, __FUNCTION__, __LINE__);
        LogD("%s %s[%d] Employee: %s, UUID: %s, Feature size: %d bytes\n", 
             __FILE__, __FUNCTION__, __LINE__, employeeId.toStdString().c_str(), 
             uuid.toStdString().c_str(), faceFeature.size());
    } else {
        LogE("%s %s[%d] Failed to update user with face features\n", __FILE__, __FUNCTION__, __LINE__);
    }
    
    return updateResult;
}

bool RegisteredFacesDB::UpdateUserWithCroppedImageFeatures(const QString &employeeId)
{
    LogD("%s %s[%d] === Starting UpdateUserWithCroppedImageFeatures ===\n", __FILE__, __FUNCTION__, __LINE__);
    LogD("%s %s[%d] Employee ID: %s\n", __FILE__, __FUNCTION__, __LINE__, employeeId.toStdString().c_str());
    
    // This is a convenience function that combines image checking and feature extraction
    QString croppedImagePath = QString("/mnt/user/reg_face_image/%1.jpg").arg(employeeId);
    
    // Verify image exists and is valid
    QFileInfo imageInfo(croppedImagePath);
    if (!imageInfo.exists()) {
        LogE("%s %s[%d] Cropped image does not exist: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, croppedImagePath.toStdString().c_str());
        return false;
    }
    
    if (imageInfo.size() < 1000) { // Less than 1KB probably indicates a problem
        LogE("%s %s[%d] Cropped image too small (likely corrupted): %lld bytes\n", 
             __FILE__, __FUNCTION__, __LINE__, imageInfo.size());
        return false;
    }
    
    LogD("%s %s[%d] Cropped image validated: %s (%lld bytes)\n", 
         __FILE__, __FUNCTION__, __LINE__, croppedImagePath.toStdString().c_str(), imageInfo.size());
    
    // Extract and store features
    return ExtractAndStoreFaceFeatures(employeeId, croppedImagePath);
}

bool RegisteredFacesDB::ProcessFaceRegistrationComplete(const QString &employeeId, const QByteArray &liveFeature)
{
    LogD("%s %s[%d] === Starting ProcessFaceRegistrationComplete ===\n", __FILE__, __FUNCTION__, __LINE__);
    LogD("%s %s[%d] Employee ID: %s, Live feature size: %d\n", 
         __FILE__, __FUNCTION__, __LINE__, employeeId.toStdString().c_str(), liveFeature.size());
    
    bool preferCroppedFeatures = true; // Flag to prefer cropped image features over live features
    
    // Method 1: Try to use features from cropped image (higher quality)
    if (preferCroppedFeatures) {
        LogD("%s %s[%d] Attempting to use cropped image features...\n", __FILE__, __FUNCTION__, __LINE__);
        
        bool croppedResult = ProcessFaceRegistrationFromCroppedImage(employeeId);
        if (croppedResult) {
            LogD("%s %s[%d] Successfully used cropped image features\n", __FILE__, __FUNCTION__, __LINE__);
            return true;
        } else {
            LogD("%s %s[%d] Cropped image features failed, falling back to live features\n", 
                 __FILE__, __FUNCTION__, __LINE__);
        }
    }
    
    // Method 2: Fallback to live features if cropped image method failed
    if (!liveFeature.isEmpty()) {
        LogD("%s %s[%d] Using live capture features as fallback\n", __FILE__, __FUNCTION__, __LINE__);
        
        QString uuid = findUuidByEmployeeId(employeeId);
        if (!uuid.isEmpty()) {
            // Try to get cropped image data for storage
            QString croppedImagePath = QString("/mnt/user/reg_face_image/%1.jpg").arg(employeeId);
            QByteArray imageData;
            
            QFile imageFile(croppedImagePath);
            if (imageFile.open(QIODevice::ReadOnly)) {
                imageData = imageFile.readAll();
                imageFile.close();
                LogD("%s %s[%d] Using cropped image data with live features\n", __FILE__, __FUNCTION__, __LINE__);
            } else {
                LogD("%s %s[%d] No cropped image data available for live features\n", __FILE__, __FUNCTION__, __LINE__);
            }
            
            bool result = UpdatePersonFaceFeature(uuid, liveFeature, imageData, "jpg");
            if (result) {
                LogD("%s %s[%d] Successfully used live features as fallback\n", __FILE__, __FUNCTION__, __LINE__);
            }
            return result;
        } else {
            LogE("%s %s[%d] Could not find UUID for employee ID: %s\n", 
                 __FILE__, __FUNCTION__, __LINE__, employeeId.toStdString().c_str());
        }
    }
    
    LogE("%s %s[%d] Both cropped and live feature methods failed\n", __FILE__, __FUNCTION__, __LINE__);
    return false;
}

bool RegisteredFacesDB::RegPersonToDBAndRAM(const QString &uuid, const QString &Name, const QString &idCard, 
                                          const QString &icCard, const QString &Sex, const QString &department, 
                                          const QString &timeOfAccess, const QByteArray &FaceFeature, 
                                          const QByteArray &faceImageData, const QString &imageFormat,
                                          const QString &attendanceMode, const QString &tenantId, 
                                          const QString &id, const QString &status, int uploadStatus, int accessLevel) 
{
    qDebug() << "=== DB_INDIVIDUAL_TIME: RegPersonToDBAndRAM START WITH NEW COLUMNS ===";
    qDebug() << "DB_INDIVIDUAL_TIME: employeeId:" << idCard;
    qDebug() << "DB_INDIVIDUAL_TIME: name:" << Name;
    qDebug() << "DB_NEW_COLUMNS: attendanceMode:" << attendanceMode;
    qDebug() << "DB_NEW_COLUMNS: tenantId:" << tenantId;
    qDebug() << "DB_NEW_COLUMNS: id:" << id;
    qDebug() << "DB_NEW_COLUMNS: status:" << status;
    
    Q_D(RegisteredFacesDB);
    QMutexLocker lock(&d->sync);

    LogD("%s %s[%d] Starting RegPersonToDBAndRAM for idCard: %s, Name: %s\n", 
         __FILE__, __FUNCTION__, __LINE__, idCard.toStdString().c_str(), Name.toStdString().c_str());
    
    // Ensure database schema is up to date
    createOrUpdateDatabaseSchema();
    
    // Save face image first (existing logic)
    bool imageSaved = false;
    QString imagePath = QString("/mnt/user/");
    
    if (!faceImageData.isEmpty() && !imageFormat.isEmpty()) {
        qDebug() << "DB_INDIVIDUAL_TIME: Saving face image...";
        imageSaved = saveFaceImage(idCard, faceImageData, imageFormat);
        if (imageSaved) {
            imagePath = getFaceImagePath(idCard, imageFormat);
            qDebug() << "DB_INDIVIDUAL_TIME: Face image saved to:" << imagePath;
        }
    }

    // Check if user already exists (existing logic)
    QSqlQuery checkQuery(QSqlDatabase::database("isc_arcsoft_face"));
    checkQuery.prepare("SELECT personid, uuid, feature FROM person WHERE idcardnum = ?");
    checkQuery.bindValue(0, idCard);
    
    bool userExists = false;
    int existingPersonId = 1;
    QString existingUuid = "";
    
    if (checkQuery.exec() && checkQuery.next()) {
        userExists = true;
        existingPersonId = checkQuery.value("personid").toInt();
        existingUuid = checkQuery.value("uuid").toString();
        qDebug() << "DB_INDIVIDUAL_TIME: User exists in database";
    }
    
    PERSONS_t t{};
    t.feature = FaceFeature;
    t.name = Name;
    t.sex = Sex.isEmpty() ? "Unknown" : Sex;
    t.idcard = idCard;
    t.iccard = icCard.isEmpty() ? "000000" : icCard;
    
    // Set new column values (no defaults applied)
    t.attendanceMode = attendanceMode;
    t.tenantId = tenantId;
    t.id = id;
    t.status = status;
    t.uploadStatus = uploadStatus;  // NEW COLUMN VALUE
    t.access_level = accessLevel; 
    
    // Handle UUID assignment (existing logic)
    if (userExists) {
        t.uuid = existingUuid;
        t.personid = existingPersonId;
    } else {
        if (uuid.size() > 3) {
            t.uuid = uuid;
        } else {
            t.uuid = QUuid::createUuid().toString();
        }
        t.personid = getPersonMaxid();
    }

    t.persontype = 0;
    t.gids = QString("0");
    t.pids = QString("2");
    t.timeOfAccess = timeOfAccess;
    t.department = department;
    
    // Use individual server time via getServerDateUpdatedTime()
    QString serverTime = getServerDateUpdatedTime();
    if (!serverTime.isEmpty()) {
        t.createtime = serverTime;
        qDebug() << "DB_INDIVIDUAL_TIME: Using INDIVIDUAL server time:" << t.createtime;
    } else {
        // Fallback to current device time
        t.createtime = QDateTime::currentDateTime().toString("yyyy/MM/dd HH:mm:ss");
        qDebug() << "DB_INDIVIDUAL_TIME: Using device time as fallback:" << t.createtime;
    }

    qDebug() << "DB_INDIVIDUAL_TIME: Final createtime set to:" << t.createtime;

     // Database insertion with NEW COLUMNS
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
     QString strSql;
    strSql.append("INSERT OR REPLACE INTO person");
    strSql.append("(personid,");
    strSql.append("uuid,");
    strSql.append("persontype,");
    strSql.append("name,");
    strSql.append("sex,");
    strSql.append("image,");
    strSql.append("createtime,");
    strSql.append("idcardnum,");
    strSql.append("iccardnum,");
    strSql.append("feature,");
    strSql.append("featuresize,");
    strSql.append("gids,");
    strSql.append("aids,");
    strSql.append("time_of_access,");
    strSql.append("department,");
    strSql.append("attendanceMode,");
    strSql.append("tenantId,");
    strSql.append("id,");
    strSql.append("status,");
    strSql.append("access_level)");  // NEW
    strSql.append("VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");  // 20 placeholders now
    
    query.prepare(strSql);
    query.bindValue(0, t.personid);
    query.bindValue(1, t.uuid);
    query.bindValue(2, t.persontype);
    query.bindValue(3, t.name);
    query.bindValue(4, t.sex);
    query.bindValue(5, imagePath);
    query.bindValue(6, t.createtime);
    query.bindValue(7, t.idcard);
    query.bindValue(8, t.iccard);
    query.bindValue(9, t.feature);
    query.bindValue(10, FaceFeature.size());
    query.bindValue(11, t.gids);
    query.bindValue(12, t.pids);
    query.bindValue(13, t.timeOfAccess);
    query.bindValue(14, t.department);
    query.bindValue(15, t.attendanceMode);
    query.bindValue(16, t.tenantId);
    query.bindValue(17, t.id);
    query.bindValue(18, t.status);
    query.bindValue(19, t.access_level);

    qDebug() << "DB_NEW_COLUMNS: Executing with new columns:";
    qDebug() << "  attendanceMode:" << t.attendanceMode;
    qDebug() << "  tenantId:" << t.tenantId;
    qDebug() << "  id:" << t.id;
    qDebug() << "  status:" << t.status;

    bool ret = query.exec();
    
    if (!ret) {
        QSqlError error = query.lastError();
        qDebug() << "DB_NEW_COLUMNS: Database operation failed:" << error.text();
        LogE("%s %s[%d] ERROR: Database operation failed: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, error.text().toStdString().c_str());
        
        if (imageSaved) {
            deleteFaceImage(idCard, imageFormat);
        }
        return false;
    } else {
        qDebug() << "DB_NEW_COLUMNS: SUCCESS with new columns!";
    }

    // Update in-memory list
    if (userExists) {
        bool found = false;
        for (int i = 0; i < d->mPersons.size(); i++) {
            if (d->mPersons[i].idcard == idCard) {
                d->mPersons[i] = t;
                found = true;
                qDebug() << "DB_NEW_COLUMNS: Updated memory with new columns";
                break;
            }
        }
        if (!found) {
            d->mPersons.append(t);
        }
    } else {
        d->mPersons.append(t);
        qDebug() << "DB_NEW_COLUMNS: Added to memory with new columns";
    }

    // Update UI
    qXLApp->GetFaceMainFrm()->updateHome_PersonNum();

#ifdef Q_OS_LINUX
    system("sync");
#endif
    
    qDebug() << "=== DB_NEW_COLUMNS: RegPersonToDBAndRAM SUCCESS ===";
    return ret;
}

// Updated StoreUserWithoutFaceData function with new columns
bool RegisteredFacesDB::StoreUserWithoutFaceData(const QString &uuid, const QString &Name, const QString &idCard, 
                                                const QString &icCard, const QString &Sex, const QString &department, 
                                                const QString &timeOfAccess, const QString &attendanceMode, 
                                                const QString &tenantId, const QString &id, const QString &status, int uploadStatus)
{
    Q_D(RegisteredFacesDB);
    QMutexLocker lock(&d->sync);

    // Ensure database schema is up to date
    createOrUpdateDatabaseSchema();

    PERSONS_t t{};
    // Create an empty feature - this user won't have face recognition initially
    t.feature = QByteArray();
    
    // Store all the user data
    t.name = Name;
    t.sex = Sex.isEmpty() ? "Unknown" : Sex;
    t.idcard = idCard;
    t.iccard = icCard.isEmpty() ? "000000" : icCard;
    
    // Set new column values (no defaults applied)
    t.attendanceMode = attendanceMode;
    t.tenantId = tenantId;
    t.id = id;
    t.status = status;
    t.uploadStatus = uploadStatus;  // NEW COLUMN VALUE
    
    // Ensure we have a valid UUID
    if(uuid.size() > 3)
    {
        t.uuid = uuid;
    }
    else
    {
        t.uuid = QUuid::createUuid().toString();
    }

    t.persontype = 0;
    t.personid = getPersonMaxid();
    t.gids = QString("0");
    t.pids = QString("2");
    t.timeOfAccess = timeOfAccess;
    t.createtime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    t.department = department;

    // Insert the user into the database with new columns
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    QString strSql;
    strSql.append("INSERT OR REPLACE INTO person");
    strSql.append("(personid,");
    strSql.append("uuid,");
    strSql.append("persontype,");
    strSql.append("name,");
    strSql.append("sex,");
    strSql.append("image,");
    strSql.append("createtime,");
    strSql.append("idcardnum,");
    strSql.append("iccardnum,");
    strSql.append("feature,");
    strSql.append("featuresize,");
    strSql.append("gids,");
    strSql.append("aids,");
    strSql.append("time_of_access,");
    strSql.append("department,");
    // NEW COLUMNS
    strSql.append("attendanceMode,");
    strSql.append("tenantId,");
    strSql.append("id,");
    strSql.append("status)");
    strSql.append("uploadStatus)");
    strSql.append("VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
    
    query.prepare(strSql);
    query.bindValue(0, t.personid);
    query.bindValue(1, t.uuid);
    query.bindValue(2, t.persontype);
    query.bindValue(3, t.name);
    query.bindValue(4, t.sex);
    query.bindValue(5, QString("/mnt/user/")); // Default image path
    query.bindValue(6, t.createtime);
    query.bindValue(7, t.idcard);
    query.bindValue(8, t.iccard);
    query.bindValue(9, t.feature); // Empty feature
    query.bindValue(10, 0); // Feature size is 0
    query.bindValue(11, t.gids);
    query.bindValue(12, t.pids);
    query.bindValue(13, t.timeOfAccess);
    query.bindValue(14, t.department);
    // NEW COLUMN VALUES
    query.bindValue(15, t.attendanceMode);
    query.bindValue(16, t.tenantId);
    query.bindValue(17, t.id);
    query.bindValue(19, t.uploadStatus);
    query.bindValue(18, t.status);
    
    LogD("%s %s[%d] Adding user without face data: %s (ID: %s) with new columns\n", 
         __FILE__, __FUNCTION__, __LINE__, 
         t.name.toStdString().c_str(), t.idcard.toStdString().c_str());
    
    // Add to in-memory list
    d->mPersons.append(t);

    bool ret = query.exec();
    if(ret == false)
    {
        QSqlError error = query.lastError();
        LogE("%s %s[%d] strSql %s \n", __FILE__, __FUNCTION__, __LINE__, strSql.toStdString().c_str());
        LogE("%s %s[%d] error %s \n", __FILE__, __FUNCTION__, __LINE__, error.text().toStdString().c_str());
        return false;
    }
    else
    {
        qXLApp->GetFaceMainFrm()->updateHome_PersonNum();
    }

#ifdef Q_OS_LINUX
    system("sync");
#endif
    return ret;
}

bool RegisteredFacesDB::triggerManualSync()
{
    LogD("%s %s[%d] === MANUAL SYNC TRIGGER ===\n", __FILE__, __FUNCTION__, __LINE__);
    
    QList<PERSONS_t> pendingRecords = getPendingUploadRecords();
    if (pendingRecords.isEmpty()) {
        LogD("%s %s[%d] No offline records to sync\n", __FILE__, __FUNCTION__, __LINE__);
        return true;
    }
    
    LogD("%s %s[%d] Found %d offline records, starting event-driven sync\n", 
         __FILE__, __FUNCTION__, __LINE__, pendingRecords.size());
    
    startEventDrivenSync();
    return true;
}


QString RegisteredFacesDB::getServerDateUpdatedTime()
{
    qDebug() << "DEBUG: getServerDateUpdatedTime - Checking for individual user server time";
    
    // *** PRIORITY 1: Use individual user's server time if set ***
    QString individualTime = getCurrentUserServerTime();
    if (!individualTime.isEmpty()) {
        qDebug() << "SUCCESS: getServerDateUpdatedTime - Using INDIVIDUAL user server time:" << individualTime;
        return individualTime;
    }
    
    // *** PRIORITY 2: Fallback to global server time from heartbeat ***
    QString serverTimeFile = "/mnt/user/sync_data/server_lastmodified.txt";
    QFile file(serverTimeFile);
    
    if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString serverTime = in.readLine().trimmed();
        QString source = in.readLine().trimmed();
        
        if (!serverTime.isEmpty() && source.contains("SERVER_DATE_UPDATED")) {
            file.close();
            qDebug() << "SUCCESS: getServerDateUpdatedTime - Using GLOBAL server time as fallback:" << serverTime;
            return serverTime;
        }
        file.close();
    }
    
    qDebug() << "DEBUG: getServerDateUpdatedTime - No server time found, will return empty string";
    return QString(); // Return empty string if no server time available
}

bool RegisteredFacesDB::createOrUpdateDatabaseSchema()
{
    LogD("%s %s[%d] === Checking and updating database schema ===\n", 
         __FILE__, __FUNCTION__, __LINE__);
    
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    
    // Check existing columns
    query.exec("PRAGMA table_info(person)");
    bool hasAttendanceMode = false;
    bool hasTenantId = false;
    bool hasId = false;
    bool hasStatus = false;
    bool hasFingerprint = false;
     bool hasFingerId = false;
    bool hasAccessLevel = false;  // NEW
    
    while (query.next()) {
        QString columnName = query.value("name").toString();
        if (columnName == "attendanceMode") hasAttendanceMode = true;
        if (columnName == "tenantId") hasTenantId = true;
        if (columnName == "id") hasId = true;
        if (columnName == "status") hasStatus = true;
        if (columnName == "fingerprint") hasFingerprint = true;
        if (columnName == "finger_id") hasFingerId = true;
        if (columnName == "access_level") hasAccessLevel = true;  // NEW
    }
    
    bool needsUpdate = false;
    
    // Add missing columns
    if (!hasAttendanceMode) {
        query.exec("ALTER TABLE person ADD COLUMN attendanceMode TEXT");
        LogD("%s %s[%d] Added attendanceMode column\n", __FILE__, __FUNCTION__, __LINE__);
        needsUpdate = true;
    }
    
    if (!hasTenantId) {
        query.exec("ALTER TABLE person ADD COLUMN tenantId TEXT");
        LogD("%s %s[%d] Added tenantId column\n", __FILE__, __FUNCTION__, __LINE__);
        needsUpdate = true;
    }
    
    if (!hasId) {
        query.exec("ALTER TABLE person ADD COLUMN id TEXT");
        LogD("%s %s[%d] Added id column\n", __FILE__, __FUNCTION__, __LINE__);
        needsUpdate = true;
    }
    
    if (!hasStatus) {
        query.exec("ALTER TABLE person ADD COLUMN status TEXT");
        LogD("%s %s[%d] Added status column\n", __FILE__, __FUNCTION__, __LINE__);
        needsUpdate = true;
    }
    
    // NEW: Add fingerprint column
    if (!hasFingerprint) {
        query.exec("ALTER TABLE person ADD COLUMN fingerprint BLOB");
        LogD("%s %s[%d] Added fingerprint column\n", __FILE__, __FUNCTION__, __LINE__);
        needsUpdate = true;
    }

     if (!hasFingerId) {
        query.exec("ALTER TABLE person ADD COLUMN finger_id INTEGER DEFAULT -1");
        LogD("%s %s[%d] Added finger_id column\n", __FILE__, __FUNCTION__, __LINE__);
        needsUpdate = true;

    }
    
     if (!hasAccessLevel) {
        query.exec("ALTER TABLE person ADD COLUMN access_level INTEGER");
        LogD("%s %s[%d] Added access_level column\n", __FILE__, __FUNCTION__, __LINE__);
        needsUpdate = true;
    }
    
    if (needsUpdate) {
        LogD("%s %s[%d] Database schema updated successfully\n", 
             __FILE__, __FUNCTION__, __LINE__);
    } else {
        LogD("%s %s[%d] Database schema is up to date\n", 
             __FILE__, __FUNCTION__, __LINE__);
    }
    
    return true;
}


void RegisteredFacesDB::setCurrentUserServerTime(const QString& serverTime)
{
    m_currentUserServerTime = serverTime;
    qDebug() << "DEBUG: setCurrentUserServerTime - Set to:" << serverTime;
}

QString RegisteredFacesDB::getCurrentUserServerTime()
{
    qDebug() << "DEBUG: getCurrentUserServerTime - Returning:" << m_currentUserServerTime;
    return m_currentUserServerTime;
}


QString RegisteredFacesDB::getGlobalServerDateUpdatedTime()
{
    // Read global server date_updated time from heartbeat (existing file)
    QString serverTimeFile = "/mnt/user/sync_data/server_lastmodified.txt";
    QFile file(serverTimeFile);
    
    if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString serverTime = in.readLine().trimmed();
        QString source = in.readLine().trimmed();
        
        if (!serverTime.isEmpty() && source.contains("SERVER_DATE_UPDATED")) {
            file.close();
            qDebug() << "SUCCESS: getGlobalServerDateUpdatedTime - Found global server time:" << serverTime;
            return serverTime;
        }
        file.close();
    }
    
    return QString();
}

QString RegisteredFacesDB::getMostRecentUserServerTime()
{
    QString userTimeDir = "/mnt/user/sync_data/user_times/";
    QDir dir(userTimeDir);
    
    if (!dir.exists()) {
        qDebug() << "DEBUG: getMostRecentUserServerTime - User times directory does not exist";
        return QString();
    }
    
    QFileInfoList files = dir.entryInfoList(QStringList("*_server_time.txt"), QDir::Files);
    
    QDateTime mostRecentDateTime;
    QString mostRecentTimeString;
    
    qDebug() << "DEBUG: getMostRecentUserServerTime - Found" << files.size() << "user time files";
    
    for (const QFileInfo& fileInfo : files) {
        QFile file(fileInfo.absoluteFilePath());
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString timeString = in.readLine().trimmed();
            
            if (!timeString.isEmpty()) {
                QDateTime dateTime = QDateTime::fromString(timeString, "yyyy/MM/dd HH:mm:ss");
                if (dateTime.isValid() && (!mostRecentDateTime.isValid() || dateTime > mostRecentDateTime)) {
                    mostRecentDateTime = dateTime;
                    mostRecentTimeString = timeString;
                    
                    qDebug() << "DEBUG: getMostRecentUserServerTime - Updated most recent time:" << timeString;
                }
            }
            file.close();
        }
    }
    
    if (!mostRecentTimeString.isEmpty()) {
        qDebug() << "SUCCESS: getMostRecentUserServerTime - Most recent server time:" << mostRecentTimeString;
    }
    
    return mostRecentTimeString;
}

int RegisteredFacesDB::UpdatePersonToDBAndRAM(const QString &uuid, const QString &name, const QString &idCard, 
                                              const QString &icCard, const QString &sex, const QString &department, 
                                              const QString &timeOfAccess, const QString &jpeg, 
                                              const QByteArray &faceEmbedding, const QString &attendanceMode, 
                                              const QString &tenantId, const QString &id, const QString &status,
                                              const QString &uploadStatus,int accessLevel)  
{
    Q_D(RegisteredFacesDB);
    QMutexLocker lock(&d->sync);
    int result = 0;
    PERSONS_t stPerson = {0};
    QByteArray faceFeatureFinal;

    qDebug() << "=== DB_UPDATE_WITH_NEW_COLUMNS: UpdatePersonToDBAndRAM START ===";
    qDebug() << "DB_UPDATE_WITH_NEW_COLUMNS: uuid:" << uuid;
    qDebug() << "DB_UPDATE_WITH_NEW_COLUMNS: attendanceMode:" << attendanceMode;
    qDebug() << "DB_UPDATE_WITH_NEW_COLUMNS: tenantId:" << tenantId;
    qDebug() << "DB_UPDATE_WITH_NEW_COLUMNS: id:" << id;
    qDebug() << "DB_UPDATE_WITH_NEW_COLUMNS: status:" << status;

    // Ensure database schema is up to date
    createOrUpdateDatabaseSchema();

    LogV("%s %s[%d] uuid %s \n", __FILE__, __FUNCTION__, __LINE__, uuid.toStdString().c_str());
    if(uuid.size() <= 0)
    {
        return -1;
    }

    int index = -1;
    for(int i = 0; i < d->mPersons.size(); i++)
    {
        auto &t = d->mPersons.at(i);
        if(t.uuid == uuid)
        {
            index = i;
            stPerson = t;
            break;
        }
    }

    if(index < 0)
    {
        return ISC_UPDATE_PERSON_NOT_EXIST;
    }

    // Update person data including new columns
    stPerson.uuid = uuid;
    if(name.size() > 0)
    {
        stPerson.name = name;
    }
    if(idCard.size() > 0)
    {
        stPerson.idcard = idCard;
    }
    if(icCard.size() > 0)
    {
        stPerson.iccard = icCard;
    }
    if(sex.size() > 0)
    {
        stPerson.sex = sex;
    }
    if(department.size() > 0)
    {
        stPerson.department = department;
    }
    if(timeOfAccess.size() > 0)
    {
        stPerson.timeOfAccess = timeOfAccess;
    }
    
    // Update new columns (no defaults applied)
    if(attendanceMode.size() > 0)
    {
        stPerson.attendanceMode = attendanceMode;
    }
    if(tenantId.size() > 0)
    {
        stPerson.tenantId = tenantId;
    }
    if(id.size() > 0)
    {
        stPerson.id = id;
    }
    if(status.size() > 0)
    {
        stPerson.status = status;
    }

      // NEW UPLOAD STATUS COLUMN UPDATE
    if(uploadStatus.size() > 0)
    {
        stPerson.uploadStatus = uploadStatus.toInt();
    }

    stPerson.access_level = accessLevel;

    // Face feature processing (existing logic)
    bool hasProvidedEmbedding = !faceEmbedding.isEmpty();
    bool hasImagePath = (jpeg.size() > 0 && !access(jpeg.toStdString().c_str(), F_OK));
    
    if (hasProvidedEmbedding) {
        faceFeatureFinal = faceEmbedding;
        qDebug() << "DB_UPDATE_WITH_NEW_COLUMNS: Using PROVIDED face embedding";
        result = 0;
    } else if (hasImagePath) {
        int faceNum = 0;
        double threshold = 0;
        QByteArray extractedFeature;
        
        result = ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->RegistPerson(jpeg, faceNum, threshold, extractedFeature);
        
        if(result == 0 && !extractedFeature.isEmpty())
        {
            faceFeatureFinal = extractedFeature;
            qDebug() << "DB_UPDATE_WITH_NEW_COLUMNS: Successfully EXTRACTED face embedding from image";
        } else {
            qDebug() << "ERROR: DB_UPDATE_WITH_NEW_COLUMNS - Failed to extract face embedding";
            return result - 10;
        }
    } else {
        faceFeatureFinal = QByteArray();
        qDebug() << "DB_UPDATE_WITH_NEW_COLUMNS: CLEARING face embedding";
        result = 0;
    }

    stPerson.feature = faceFeatureFinal;

    // Use individual server time
    QString serverTime = getServerDateUpdatedTime();
    if (!serverTime.isEmpty()) {
        stPerson.createtime = serverTime;
        qDebug() << "DB_UPDATE_WITH_NEW_COLUMNS: Using INDIVIDUAL server time:" << stPerson.createtime;
    } else {
        stPerson.createtime = QDateTime::currentDateTime().toString("yyyy/MM/dd HH:mm:ss");
        qDebug() << "DB_UPDATE_WITH_NEW_COLUMNS: Using device time as fallback:" << stPerson.createtime;
    }

     // Database update with new columns
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    QString strSql;
    strSql.append("UPDATE person SET ");
    strSql.append("name=?,");
    strSql.append("sex=?,");
    strSql.append("idcardnum=?,");
    strSql.append("iccardnum=?,");
    strSql.append("feature=?,");
    strSql.append("featuresize=?,");
    strSql.append("time_of_access=?,");
    strSql.append("department=?,");
    strSql.append("createtime=?,");
    strSql.append("attendanceMode=?,");
    strSql.append("tenantId=?,");
    strSql.append("id=?,");
    strSql.append("status=?,");
    strSql.append("access_level=?");  // NEW
    strSql.append(" where uuid=?");
    
    query.prepare(strSql);
    query.bindValue(0, stPerson.name);
    query.bindValue(1, stPerson.sex);
    query.bindValue(2, stPerson.idcard);
    query.bindValue(3, stPerson.iccard);
    query.bindValue(4, stPerson.feature);
    query.bindValue(5, stPerson.feature.size());
    query.bindValue(6, stPerson.timeOfAccess);
    query.bindValue(7, stPerson.department);
    query.bindValue(8, stPerson.createtime);
    query.bindValue(9, stPerson.attendanceMode);
    query.bindValue(10, stPerson.tenantId);
    query.bindValue(11, stPerson.id);
    query.bindValue(12, stPerson.status);
    query.bindValue(13, stPerson.access_level);  // NEW
    query.bindValue(14, stPerson.uuid);
    
    qDebug() << "DB_UPDATE_WITH_NEW_COLUMNS: Executing update with new columns";
    
    if(query.exec() == false)
    {
        QSqlError error = query.lastError();
        qDebug() << "DB_UPDATE_WITH_NEW_COLUMNS: Database update FAILED:" << error.text();
        LogE("%s %s[%d] strSql %s \n",__FILE__,__FUNCTION__,__LINE__,strSql.toStdString().c_str());
        LogE("%s %s[%d] error %s \n",__FILE__,__FUNCTION__,__LINE__,error.text().toStdString().c_str());
        return ISC_UPDATE_PERSON_DB_ERROR;
    } else {
        qDebug() << "DB_UPDATE_WITH_NEW_COLUMNS: Database update SUCCESS!";
    }
    
    // Update in-memory list
    d->mPersons.replace(index, stPerson);
    
    qDebug() << "DB_UPDATE_WITH_NEW_COLUMNS: Updated in-memory person with new columns";

#ifdef Q_OS_LINUX
    system("sync");
#endif
    
    qDebug() << "=== DB_UPDATE_WITH_NEW_COLUMNS: UpdatePersonToDBAndRAM SUCCESS ===";
    
    if (result == 0)
        return 1;
    else 
        return result;
}


bool RegisteredFacesDB::UpdatePersonDirectly(const QString &uuid, const QString &name, const QString &idCard, 
                                           const QString &sex, const QByteArray &faceFeature)
{
    qDebug() << "=== DB_FACE_DEBUG: UpdatePersonDirectly START (with server time) ===";
    qDebug() << "DB_FACE_DEBUG: Update parameters:";
    qDebug() << "  uuid:" << uuid;
    qDebug() << "  name:" << name;
    qDebug() << "  idCard:" << idCard;
    qDebug() << "  sex:" << sex;
    qDebug() << "  faceFeature.size():" << faceFeature.size();
    
    Q_D(RegisteredFacesDB);
    QMutexLocker lock(&d->sync);
    
    // Check current state (existing logic)
    QSqlQuery preCheckQuery(QSqlDatabase::database("isc_arcsoft_face"));
    preCheckQuery.prepare("SELECT name, idcardnum, sex, feature, featuresize FROM person WHERE uuid = ?");
    preCheckQuery.bindValue(0, uuid);
    
    QByteArray currentFeature;
    bool hasCurrentFeature = false;
    
    if (preCheckQuery.exec() && preCheckQuery.next()) {
        currentFeature = preCheckQuery.value("feature").toByteArray();
        hasCurrentFeature = !currentFeature.isEmpty();
        qDebug() << "DB_FACE_DEBUG: Current feature size:" << currentFeature.size();
    }
    
    // Decide which face feature to use (existing logic)
    QByteArray featureToStore;
    QString updateStrategy;
    
    if (!faceFeature.isEmpty()) {
        featureToStore = faceFeature;
        updateStrategy = "USE_NEW_FEATURE";
        qDebug() << "DB_FACE_DEBUG: Using new face feature";
    } else if (hasCurrentFeature) {
        featureToStore = currentFeature;
        updateStrategy = "PRESERVE_EXISTING_FEATURE";
        qDebug() << "DB_FACE_DEBUG: Preserving existing face feature";
    } else {
        featureToStore = QByteArray();
        updateStrategy = "NO_FEATURE_DATA";
        qDebug() << "DB_FACE_DEBUG: No face feature data available";
    }
    
    // *** MODIFIED: Use server date_updated time instead of current device time ***
    QString updateTime = getServerDateUpdatedTime();
    if (updateTime.isEmpty()) {
        updateTime = QDateTime::currentDateTime().toString("yyyy/MM/dd HH:mm:ss");
        qDebug() << "DB_FACE_DEBUG: Using DEVICE time as fallback:" << updateTime;
    } else {
        qDebug() << "DB_FACE_SUCCESS: Using SERVER date_updated time:" << updateTime;
    }
    
    // Execute update with server time
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    QString updateSql = "UPDATE person SET name = ?, idcardnum = ?, sex = ?, feature = ?, featuresize = ?, createtime = ? WHERE uuid = ?";
    query.prepare(updateSql);
    query.bindValue(0, name);
    query.bindValue(1, idCard);
    query.bindValue(2, sex);
    query.bindValue(3, featureToStore);
    query.bindValue(4, featureToStore.size());
    query.bindValue(5, updateTime);  // *** SERVER TIME STORED HERE ***
    query.bindValue(6, uuid);
    
    qDebug() << "DB_FACE_DEBUG: UPDATE query with SERVER time:" << updateTime;
    
    bool success = query.exec();
    
    if (success) {
        qDebug() << "DB_FACE_SUCCESS: Update query executed with SERVER time!";
    } else {
        QSqlError error = query.lastError();
        qDebug() << "DB_FACE_ERROR: Update query failed:" << error.text();
        return false;
    }
    
    // Update in-memory list (existing logic but with server time)
    for (int i = 0; i < d->mPersons.size(); i++) {
        if (d->mPersons[i].uuid == uuid) {
            d->mPersons[i].name = name;
            d->mPersons[i].idcard = idCard;
            d->mPersons[i].sex = sex;
            d->mPersons[i].feature = featureToStore;
            d->mPersons[i].createtime = updateTime;  // *** SERVER TIME STORED HERE ***
            
            qDebug() << "DB_FACE_DEBUG: Updated person in memory with SERVER time:" << updateTime;
            break;
        }
    }
    
    qDebug() << "DB_FACE_SUCCESS: UpdatePersonDirectly completed with SERVER date_updated time!";
    qDebug() << "=== DB_FACE_DEBUG: UpdatePersonDirectly END ===";
    return success;
}


bool RegisteredFacesDB::RegPersonToDBAndRAM(const QString &name, const QString &sex, const QString &nation, const QString &idcardnum, const QString &Marital, const QString &education, const QString &location, const QString &phone, const QString &politics_status, const QString &date_birth, const QString &number, const QString &branch, const QString &hiredate, const QString &extension_phone, const QString &mailbox, const QString &status, const QString &persontype, const QString &iccardnum, const QByteArray &FaceFeature)
{
    return this->RegPersonToDBAndRAM("",name, idcardnum, iccardnum, sex, "","",FaceFeature);
}

bool RegisteredFacesDB::ComparisonPersonFaceFeature_baidu(QString &name, QString &sex, QString &idcard, QString &iccard, QString &uuid, int &persontype, 
   int &personid, QString &gids, QString &pids, QString &id, unsigned char *FaceFeature, int FaceFeatureSize)
{
    Q_D(RegisteredFacesDB);
    QMutexLocker lock(&d->sync);

    

    gAlogStateFaceSimilar = 0;
    
    
    for(int i = 0; i < d->mPersons.count(); i++)
    {
        
        auto &t = d->mPersons.at(i);
        
        // Validate person data
        if (t.feature.isEmpty() || t.feature.size() <= 0) {
            continue;
        }
        
        double similar = ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->getFaceFeatureCompare_baidu(
            (uchar *)t.feature.data(), t.feature.size(), FaceFeature, FaceFeatureSize);
        
        if (similar > gAlogStateFaceSimilar) {
            gAlogStateFaceSimilar = similar;
        }

        if (similar >= d->mThanthreshold) // 0.8
        {
            
            name = t.name;
            sex = t.sex;
            idcard = t.idcard;
            iccard = t.iccard;
            uuid = t.uuid;
            persontype = t.persontype;
            personid = t.personid;
            gids = t.gids;
            pids = t.pids;
            id = t.id;
            
            return true;
        }
        
    }

    return false;
}   

bool RegisteredFacesDB::ComparisonPersonFaceFeature(QString &name, QString &sex, QString &idcard, QString &iccard, QString &uuid, int &persontype, int &personid, QString &gids, QString &pids, const QByteArray &FaceFeature)
{
    Q_D(RegisteredFacesDB);
    QMutexLocker lock(&d->sync);

    gAlogStateFaceSimilar = 0;
    for(int i = 0; i<d->mPersons.count(); i++)
    {
        auto &t = d->mPersons.at(i);
     

        double similar = ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->getFaceFeatureCompare((uchar *)t.feature.data(), t.feature.size(), FaceFeature);
        if(similar > gAlogStateFaceSimilar)
        {
        	gAlogStateFaceSimilar = similar;
        }
        if(similar>=d->mThanthreshold)
        {
            name = t.name;
            sex = t.sex;
            idcard = t.idcard;
            iccard = t.iccard;
            uuid = t.uuid;
            persontype = t.persontype;
            personid = t.personid;
            gids = t.gids;
            pids = t.pids;
            return true;
        }
    }
    return false;
}

bool RegisteredFacesDB::DelPersonDBInfo(const QString &name, const QString &createtime)
{/*åˆ é™¤æ•°æ®åº“ä¿¡æ¯*/
    Q_D(RegisteredFacesDB);
    QMutexLocker lock(&d->sync);
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    query.prepare(QString("DELETE FROM person WHERE name='%1' AND createtime='%2'")
                  .arg(name).arg(createtime));
    bool ret = query.exec();

    for(int i = 0; i<d->mPersons.count(); i++)
    {
        auto &t = d->mPersons.at(i);

        if(t.name == name && t.createtime == createtime)
        {
            d->mPersons.removeAt(i);

            qXLApp->GetFaceMainFrm()->updateHome_PersonNum();
            return ret;
        }
    }

    return ret;
}

bool RegisteredFacesDB::QueryNameRepetition(const QString &name) const
{
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    query.exec(QString("select name from person where name='%1'").arg(name));
    while(query.next())
    {
        return true;
    }
    return false;
}

void RegisteredFacesDB::run()
{
    Q_D(RegisteredFacesDB);
    while (!isInterruptionRequested())
    {
        d->sync.lock();
        if (d->is_pause)d->pauseCond.wait(&d->sync);
        
        // Ensure database schema is up to date when loading
        createOrUpdateDatabaseSchema();
        
        QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
        
        // First check if we need to migrate any 0 IDs to 1
        QSqlQuery migrateQuery(QSqlDatabase::database("isc_arcsoft_face"));
        migrateQuery.exec("SELECT COUNT(*) FROM person WHERE personid = 0");
        if (migrateQuery.next() && migrateQuery.value(0).toInt() > 0) {
            LogD("%s %s[%d] Migrating person IDs from 0 to 1 for %d records\n",
                 __FILE__, __FUNCTION__, __LINE__, migrateQuery.value(0).toInt());
            migrateQuery.exec("UPDATE person SET personid = 1 WHERE personid = 0");
        }
        
        query.exec("select *from person");
        while(query.next())
        {
            PERSONS_t t{};
            t.feature = query.value("feature").toByteArray();
            t.name = query.value("name").toString();
            t.sex = query.value("sex").toString();
            if(t.sex == "") {
                t.sex = "unknow";
            }
            t.idcard = query.value("idcardnum").toString();
            t.iccard = query.value("iccardnum").toString();
            t.uuid = query.value("uuid").toString();
            t.persontype = query.value("persontype").toInt();
            t.personid = query.value("personid").toInt();
            if (t.personid < 1) {
                t.personid = 1;
            }
            t.gids = query.value("gids").toString();
            t.pids = query.value("aids").toString();
            t.createtime = query.value("createtime").toString();
            t.timeOfAccess = query.value("time_of_access").toString();
            t.department = query.value("department").toString();
            
            // Load existing columns
            t.attendanceMode = query.value("attendanceMode").toString();
            t.tenantId = query.value("tenantId").toString();
            t.id = query.value("id").toString();
            t.status = query.value("status").toString();
            
            // NEW: Load fingerprint data
            t.fingerprint = query.value("fingerprint").toByteArray();
            t.finger_id = query.value("finger_id").toInt();  // âœ… ADD THIS LINE
            t.access_level = query.value("access_level").toInt();
            
            d->mPersons.append(t);
        }
        
        d->is_pause = true;
        d->sync.unlock();
    }
}

PERSONS_t RegisteredFacesDB::getPersonByUuid(const QString &uuid)
{
    Q_D(RegisteredFacesDB);
    QMutexLocker lock(&d->sync);
    
    for (int i = 0; i < d->mPersons.size(); i++) {
        if (d->mPersons[i].uuid == uuid) {
            return d->mPersons[i];
        }
    }
    
    return PERSONS_t{}; // Return empty struct if not found
}

// Helper function to update only new columns
bool RegisteredFacesDB::updatePersonNewColumns(const QString &uuid, const QString &attendanceMode, 
                                             const QString &tenantId, const QString &id, const QString &status)
{
    Q_D(RegisteredFacesDB);
    QMutexLocker lock(&d->sync);
    
    // Ensure database schema is up to date
    createOrUpdateDatabaseSchema();
    
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    QString updateSql = "UPDATE person SET attendanceMode = ?, tenantId = ?, id = ?, status = ? WHERE uuid = ?";
    query.prepare(updateSql);
    query.bindValue(0, attendanceMode);
    query.bindValue(1, tenantId);
    query.bindValue(2, id);
    query.bindValue(3, status);
    query.bindValue(4, uuid);
    
    bool success = query.exec();
    
    if (success) {
        // Update in-memory list (no default values)
        for (int i = 0; i < d->mPersons.size(); i++) {
            if (d->mPersons[i].uuid == uuid) {
                d->mPersons[i].attendanceMode = attendanceMode;
                d->mPersons[i].tenantId = tenantId;
                d->mPersons[i].id = id;
                d->mPersons[i].status = status;
                break;
            }
        }
        
        LogD("%s %s[%d] Updated new columns for person with UUID: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, uuid.toStdString().c_str());
    } else {
        QSqlError error = query.lastError();
        LogE("%s %s[%d] Failed to update new columns: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, error.text().toStdString().c_str());
    }
    
    return success;
}

QList<PERSONS_t> RegisteredFacesDB::GetAllPersonFromRAM()
{
	QList<PERSONS_t> result;
	Q_D(RegisteredFacesDB);
	d->sync.lock();
	result = d->mPersons;

	d->sync.unlock();
	return result;
}

QList<PERSONS_t> RegisteredFacesDB::GetPersonDataByNameFromRAM(int nCurrPage,int nPerPage,QString name)
{
	QList<PERSONS_t> result;
	Q_D(RegisteredFacesDB);
	d->sync.lock();
	for (int i = nCurrPage; i < nCurrPage + nPerPage; i++)
	{
		if(i < 0 || d->mPersons.size() <= i)
		{
			break;
		}
		auto &t = d->mPersons.at(i);
		if (t.name == name)
		{
			result.append(t);
		}
	}
	d->sync.unlock();
	return result;
}

int RegisteredFacesDB::GetPersonTotalNumByNameFromRAM(QString name)
{
	Q_D(RegisteredFacesDB);
	int count = 0;
	d->sync.lock();
    for(int i = 0; i<d->mPersons.count(); i++)
    {
        auto &t = d->mPersons.at(i);
        if(t.name == name)
        {
        	count++;
        }
    }

	d->sync.unlock();
	return count;
}

QList<PERSONS_t> RegisteredFacesDB::GetPersonDataByPersonUUIDFromRAM(QString uuid)
{
	Q_D(RegisteredFacesDB);
	QList<PERSONS_t> result;
	d->sync.lock();
	for (int i = 0; i < d->mPersons.size(); i++)
    {
		if(i < 0 || d->mPersons.size() <= i)
		{
			break;
		}
        auto &t = d->mPersons.at(i);
        if(t.uuid == uuid)
        {
        	result.append(t);
        }
    }

	d->sync.unlock();
	return result;
}

int RegisteredFacesDB::GetPersonTotalNumByPersonUUIDFromRAM(QString uuid)
{
	Q_D(RegisteredFacesDB);
	int count = 0;
	d->sync.lock();
    for(int i = 0; i<d->mPersons.count(); i++)
    {
        auto &t = d->mPersons.at(i);
        if(t.uuid == uuid)
        {
        	count++;
        }
    }

	d->sync.unlock();
	return count;
}

QList<PERSONS_t> RegisteredFacesDB::GetPersonDataByTimeFromRAM(int nCurrPage,int nPerPage,QTime startTime,QTime endTime)
{
	Q_D(RegisteredFacesDB);
	QList<PERSONS_t> result;
	d->sync.lock();
	for (int i = nCurrPage; i < nCurrPage + nPerPage; i++)
    {
		if(i < 0 || d->mPersons.size() <= i)
		{
			break;
		}
        auto &t = d->mPersons.at(i);
        QTime tTime = QTime::fromString(t.createtime,"yyyy/MM/dd hh:mm:ss");
        if(tTime >= startTime && tTime <=  endTime)
        {
        	result.append(t);
        }
    }

	d->sync.unlock();
	return result;
}

int RegisteredFacesDB::GetPersonTotalNumByTimeFromRAM(QTime startTime,QTime endTime)
{
	Q_D(RegisteredFacesDB);
	int count = 0;
	d->sync.lock();
	for (int i = 0; i < d->mPersons.size(); i++)
    {
        auto &t = d->mPersons.at(i);
        QTime tTime = QTime::fromString(t.createtime,"yyyy/MM/dd hh:mm:ss");
        if(tTime >= startTime && tTime <=  endTime)
        {
        	count++;
        }
    }

	d->sync.unlock();
	return count;
}

bool RegisteredFacesDB::DelPersonByPersionUUIDFromDBAndRAM(QString uuid)
{/*åˆ é™¤æ•°æ®åº“ä¿¡æ¯*/
    Q_D(RegisteredFacesDB);
    QMutexLocker lock(&d->sync);
    LogD("%s %s[%d] delete %s \n", __FILE__, __FUNCTION__, __LINE__, uuid.toStdString().c_str());
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    query.prepare(QString("DELETE FROM person WHERE uuid='%1'").arg(uuid));
    bool ret = query.exec();

    for(int i = 0; i<d->mPersons.count(); i++)
    {
        auto &t = d->mPersons.at(i);

        if(t.uuid == uuid)
        {
            d->mPersons.removeAt(i);
            qXLApp->GetFaceMainFrm()->updateHome_PersonNum();
            return ret;
        }
    }
    return ret;
}

bool RegisteredFacesDB::UpdatePersonFaceFeature(const QString &uuid, const QByteArray &faceFeature, 
                                               const QByteArray &faceImageData, const QString &imageFormat)
{
    Q_D(RegisteredFacesDB);
    QMutexLocker lock(&d->sync);

    LogD("%s %s[%d] === Starting UpdatePersonFaceFeature ===\n", __FILE__, __FUNCTION__, __LINE__);
    LogD("%s %s[%d] UUID: %s\n", __FILE__, __FUNCTION__, __LINE__, uuid.toStdString().c_str());
    LogD("%s %s[%d] Face feature size: %d bytes\n", __FILE__, __FUNCTION__, __LINE__, faceFeature.size());
    LogD("%s %s[%d] Face image data size: %d bytes\n", __FILE__, __FUNCTION__, __LINE__, faceImageData.size());
    LogD("%s %s[%d] Image format: %s\n", __FILE__, __FUNCTION__, __LINE__, imageFormat.toStdString().c_str());

    // Find the person to get their employee ID
    QString employeeId;
    QString personName;
    for (int i = 0; i < d->mPersons.size(); i++) {
        if (d->mPersons[i].uuid == uuid) {
            employeeId = d->mPersons[i].idcard;
            personName = d->mPersons[i].name;
            break;
        }
    }

    if (employeeId.isEmpty()) {
        LogE("%s %s[%d] Person with UUID %s not found in memory\n", 
             __FILE__, __FUNCTION__, __LINE__, uuid.toStdString().c_str());
        return false;
    }
    
    LogD("%s %s[%d] Found person - Name: %s, Employee ID: %s\n", 
         __FILE__, __FUNCTION__, __LINE__, personName.toStdString().c_str(), employeeId.toStdString().c_str());

    // Save face image if provided
    bool imageSaved = false;
    QString imagePath = QString("/mnt/user/");
    
    if (!faceImageData.isEmpty() && !imageFormat.isEmpty()) {
        LogD("%s %s[%d] Attempting to save face image...\n", __FILE__, __FUNCTION__, __LINE__);
        imageSaved = saveFaceImage(employeeId, faceImageData, imageFormat);
        
        if (imageSaved) {
            imagePath = getFaceImagePath(employeeId, imageFormat);
            LogD("%s %s[%d] Face image saved successfully, path: %s\n", 
                 __FILE__, __FUNCTION__, __LINE__, imagePath.toStdString().c_str());
        } else {
            LogE("%s %s[%d] Failed to save face image for employee: %s\n", 
                 __FILE__, __FUNCTION__, __LINE__, employeeId.toStdString().c_str());
        }
    } else {
        LogD("%s %s[%d] No image data provided (imageData: %d bytes, format: %s)\n", 
             __FILE__, __FUNCTION__, __LINE__, faceImageData.size(), imageFormat.toStdString().c_str());
    }

    // Update database
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    if (imageSaved) {
        LogD("%s %s[%d] Updating database with feature and image path\n", __FILE__, __FUNCTION__, __LINE__);
        query.prepare("UPDATE person SET feature = ?, featuresize = ?, image = ? WHERE uuid = ?");
        query.bindValue(0, faceFeature);
        query.bindValue(1, faceFeature.size());
        query.bindValue(2, imagePath);
        query.bindValue(3, uuid);
    } else {
        LogD("%s %s[%d] Updating database with feature only\n", __FILE__, __FUNCTION__, __LINE__);
        query.prepare("UPDATE person SET feature = ?, featuresize = ? WHERE uuid = ?");
        query.bindValue(0, faceFeature);
        query.bindValue(1, faceFeature.size());
        query.bindValue(2, uuid);
    }
    
    if (!query.exec()) {
        QSqlError error = query.lastError();
        LogE("%s %s[%d] Database update error: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, error.text().toStdString().c_str());
        return false;
    } else {
        LogD("%s %s[%d] Database updated successfully\n", __FILE__, __FUNCTION__, __LINE__);
    }

    // Update in-memory list
    for (int i = 0; i < d->mPersons.size(); i++) {
        if (d->mPersons[i].uuid == uuid) {
            d->mPersons[i].feature = faceFeature;
            LogD("%s %s[%d] Successfully updated face feature in memory for person: %s\n", 
                 __FILE__, __FUNCTION__, __LINE__, d->mPersons[i].name.toStdString().c_str());
            
            LogD("%s %s[%d] === UpdatePersonFaceFeature completed successfully ===\n", __FILE__, __FUNCTION__, __LINE__);
            return true;
        }
    }
    
    LogE("%s %s[%d] Failed to find person in memory for final update\n", __FILE__, __FUNCTION__, __LINE__);
    return false;
}

bool RegisteredFacesDB::ensureFingerprintColumn()
{
    LogD("%s %s[%d] === Checking fingerprint column ===\n", __FILE__, __FUNCTION__, __LINE__);
    
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    
    // Check if fingerprint column exists
    query.exec("PRAGMA table_info(person)");
    bool hasFingerprint = false;
    
    while (query.next()) {
        QString columnName = query.value("name").toString();
        if (columnName == "fingerprint") {
            hasFingerprint = true;
            break;
        }
    }
    
    // Add fingerprint column if it doesn't exist
    if (!hasFingerprint) {
        LogD("%s %s[%d] Adding fingerprint column to database\n", __FILE__, __FUNCTION__, __LINE__);
        
        if (!query.exec("ALTER TABLE person ADD COLUMN fingerprint BLOB")) {
            LogE("%s %s[%d] Failed to add fingerprint column: %s\n", 
                 __FILE__, __FUNCTION__, __LINE__, 
                 query.lastError().text().toStdString().c_str());
            return false;
        }
        
        LogD("%s %s[%d] Fingerprint column added successfully\n", __FILE__, __FUNCTION__, __LINE__);
    } else {
        LogD("%s %s[%d] Fingerprint column already exists\n", __FILE__, __FUNCTION__, __LINE__);
    }
    
    return true;
}

// Function to update person's fingerprint data
bool RegisteredFacesDB::UpdatePersonFingerprint(const QString &uuid, const QByteArray &fingerprintData)
{
    Q_D(RegisteredFacesDB);
    QMutexLocker lock(&d->sync);
    
    LogD("%s %s[%d] === Updating fingerprint for UUID: %s ===\n", 
         __FILE__, __FUNCTION__, __LINE__, uuid.toStdString().c_str());
    LogD("%s %s[%d] Fingerprint data size: %d bytes\n", 
         __FILE__, __FUNCTION__, __LINE__, fingerprintData.size());
    
    if (uuid.isEmpty()) {
        LogE("%s %s[%d] UUID is empty\n", __FILE__, __FUNCTION__, __LINE__);
        return false;
    }
    
    if (fingerprintData.isEmpty()) {
        LogE("%s %s[%d] Fingerprint data is empty\n", __FILE__, __FUNCTION__, __LINE__);
        return false;
    }
    
    // Ensure fingerprint column exists
    if (!ensureFingerprintColumn()) {
        LogE("%s %s[%d] Failed to ensure fingerprint column exists\n", __FILE__, __FUNCTION__, __LINE__);
        return false;
    }
    
    // Update database
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    query.prepare("UPDATE person SET fingerprint = ? WHERE uuid = ?");
    query.bindValue(0, fingerprintData);
    query.bindValue(1, uuid);
    
    if (!query.exec()) {
        QSqlError error = query.lastError();
        LogE("%s %s[%d] Database update failed: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, error.text().toStdString().c_str());
        return false;
    }
    
    if (query.numRowsAffected() == 0) {
        LogE("%s %s[%d] No rows affected - UUID not found: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, uuid.toStdString().c_str());
        return false;
    }
    
    // Update in-memory list
    for (int i = 0; i < d->mPersons.size(); i++) {
        if (d->mPersons[i].uuid == uuid) {
            d->mPersons[i].fingerprint = fingerprintData;
            LogD("%s %s[%d] Updated fingerprint in memory for person: %s\n", 
                 __FILE__, __FUNCTION__, __LINE__, d->mPersons[i].name.toStdString().c_str());
            break;
        }
    }
    
    LogD("%s %s[%d] === Fingerprint update successful ===\n", __FILE__, __FUNCTION__, __LINE__);
    
#ifdef Q_OS_LINUX
    system("sync");
#endif
    
    return true;
}

// Function to retrieve person's fingerprint data
bool RegisteredFacesDB::GetPersonFingerprint(const QString &uuid, QByteArray &fingerprintData)
{
    Q_D(RegisteredFacesDB);
    QMutexLocker lock(&d->sync);
    
    LogD("%s %s[%d] Getting fingerprint for UUID: %s\n", 
         __FILE__, __FUNCTION__, __LINE__, uuid.toStdString().c_str());
    
    if (uuid.isEmpty()) {
        return false;
    }
    
    // First check in-memory list (faster)
    for (int i = 0; i < d->mPersons.size(); i++) {
        if (d->mPersons[i].uuid == uuid) {
            fingerprintData = d->mPersons[i].fingerprint;
            LogD("%s %s[%d] Found fingerprint in memory: %d bytes\n", 
                 __FILE__, __FUNCTION__, __LINE__, fingerprintData.size());
            return !fingerprintData.isEmpty();
        }
    }
    
    // Fallback to database query
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    query.prepare("SELECT fingerprint FROM person WHERE uuid = ?");
    query.bindValue(0, uuid);
    
    if (!query.exec()) {
        LogE("%s %s[%d] Query failed: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, 
             query.lastError().text().toStdString().c_str());
        return false;
    }
    
    if (query.next()) {
        fingerprintData = query.value("fingerprint").toByteArray();
        LogD("%s %s[%d] Found fingerprint in database: %d bytes\n", 
             __FILE__, __FUNCTION__, __LINE__, fingerprintData.size());
        return !fingerprintData.isEmpty();
    }
    
    LogD("%s %s[%d] No fingerprint found for UUID: %s\n", 
         __FILE__, __FUNCTION__, __LINE__, uuid.toStdString().c_str());
    return false;
}

// Function to delete person's fingerprint data
bool RegisteredFacesDB::DeletePersonFingerprint(const QString &uuid)
{
    Q_D(RegisteredFacesDB);
    QMutexLocker lock(&d->sync);
    
    LogD("%s %s[%d] Deleting fingerprint for UUID: %s\n", 
         __FILE__, __FUNCTION__, __LINE__, uuid.toStdString().c_str());
    
    if (uuid.isEmpty()) {
        return false;
    }
    
    // Update database - set fingerprint to NULL
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    query.prepare("UPDATE person SET fingerprint = NULL WHERE uuid = ?");
    query.bindValue(0, uuid);
    
    if (!query.exec()) {
        LogE("%s %s[%d] Failed to delete fingerprint: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, 
             query.lastError().text().toStdString().c_str());
        return false;
    }
    
    // Update in-memory list
    for (int i = 0; i < d->mPersons.size(); i++) {
        if (d->mPersons[i].uuid == uuid) {
            d->mPersons[i].fingerprint.clear();
            LogD("%s %s[%d] Cleared fingerprint in memory\n", __FILE__, __FUNCTION__, __LINE__);
            break;
        }
    }
    
    LogD("%s %s[%d] Fingerprint deleted successfully\n", __FILE__, __FUNCTION__, __LINE__);
    return true;
}

bool RegisteredFacesDB::UpdatePersonFingerId(const QString &uuid, int fingerId)
{
    Q_D(RegisteredFacesDB);
    QMutexLocker lock(&d->sync);
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    query.prepare("UPDATE person SET finger_id = ? WHERE uuid = ?");
    query.bindValue(0, fingerId);
    query.bindValue(1, uuid);
    if (!query.exec()) {
        return false;
    }
    for (int i = 0; i < d->mPersons.size(); i++) {
        if (d->mPersons[i].uuid == uuid) {
            d->mPersons[i].finger_id = fingerId;
            break;
        }
    }
    return true;
}

int RegisteredFacesDB::getNextAvailableFingerId()
{
    QMutexLocker lock(&g_personIdMutex);
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    query.exec("SELECT MAX(finger_id) FROM person WHERE finger_id > 0");
    if (query.next()) {
        int maxId = query.value(0).toInt();
        return maxId >= 1 ? maxId + 1 : 1;
    }
    return 1;
}

bool RegisteredFacesDB::GetPersonByFingerId(int fingerId, PERSONS_t &person)
{
    Q_D(RegisteredFacesDB);
    QMutexLocker lock(&d->sync);
    for (int i = 0; i < d->mPersons.size(); i++) {
        if (d->mPersons[i].finger_id == fingerId) {
            person = d->mPersons[i];
            return true;
       }
    }
    return false;
}

bool RegisteredFacesDB::clearFingerprint(const QString& uuid)
{
    LogD("%s %s[%d] Clearing fingerprint for UUID: %s\n", 
         __FILE__, __FUNCTION__, __LINE__, uuid.toStdString().c_str());
    // Clear both fingerprint data and finger_id
    bool deleteResult = DeletePersonFingerprint(uuid);
    bool idResult = UpdatePersonFingerId(uuid, 0); // Set to 0 (no fingerprint)
    return deleteResult && idResult;
}

uint16_t RegisteredFacesDB::getFingerId(const QString& uuid)
{
    Q_D(RegisteredFacesDB);
    QMutexLocker lock(&d->sync);
    // Check in memory first
    for (int i = 0; i < d->mPersons.size(); i++) {
        if (d->mPersons[i].uuid == uuid) {
        return d->mPersons[i].finger_id;
     }
    }
    // Fallback to database
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    query.prepare("SELECT finger_id FROM person WHERE uuid = ?");
    query.bindValue(0, uuid);
    if (query.exec() && query.next()) {
        return query.value(0).toUInt();
    }
    return 0; // No fingerprint enrolled
}

bool RegisteredFacesDB::clearAllFingerIds()
{
    Q_D(RegisteredFacesDB);
    QMutexLocker lock(&d->sync);
    LogD("%s %s[%d] === Clearing all finger IDs from database ===\n", 
         __FILE__, __FUNCTION__, __LINE__);
    // Clear all finger_id values in database
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    if (!query.exec("UPDATE person SET finger_id = 0")) {
        LogE("%s %s[%d] Failed to clear finger IDs: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, 
 query.lastError().text().toStdString().c_str());
        return false;
    }
    // Clear in-memory list
    for (int i = 0; i < d->mPersons.size(); i++) {
        d->mPersons[i].finger_id = 0;
    }
    LogD("%s %s[%d] All finger IDs cleared successfully\n", 
         __FILE__, __FUNCTION__, __LINE__);
    return true;
}