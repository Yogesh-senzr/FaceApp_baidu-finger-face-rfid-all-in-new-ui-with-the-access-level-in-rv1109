// PersonRegistrationThread.cpp
#include "PersonRegistrationThread.h"
#include "Config/ReadConfig.h"
#include "Helper/myhelper.h"
#include "MessageHandler/Log.h"
#include "BaiduFace/BaiduFaceManager.h"
#include "DB/RegisteredFacesDB.h"
#include "Application/FaceApp.h"
#include "base64.hpp"

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <curl/curl.h>
#include <openssl/md5.h>

PersonRegistrationThread* PersonRegistrationThread::m_pInstance = nullptr;
QMutex PersonRegistrationThread::m_mutex;

class PersonRegistrationThreadPrivate
{
    Q_DECLARE_PUBLIC(PersonRegistrationThread)
public:
    PersonRegistrationThreadPrivate(PersonRegistrationThread* dd);
    
    QString mSN;
    QString mRegistrationServerUrl;
    QString mRegistrationServerPassword;
    
    // Queue of person registration requests
    QList<Json::Value> mPendingRegistrations;
    QList<QString> mSuccessfulRegistrations;
    
private:
    mutable QMutex sync;
    QWaitCondition pauseCond;
    int threadDelay;
    
    // Helper methods
    bool doDownloadFile(const QString& url, const QString& dist);
    QString doPostJson(const Json::Value& json);
    bool processRegistration(const Json::Value& personData, const std::string& msgType, const std::string& cmdId);
    void sendBatchReport();
    
    // Utility methods
    std::string md5sum(const std::string& src);
    std::string getTime();
    std::string fileToBase64String(const std::string& strFilePath);
    
    PersonRegistrationThread* const q_ptr;
};

// Implementation of private class methods
PersonRegistrationThreadPrivate::PersonRegistrationThreadPrivate(PersonRegistrationThread* dd)
    : q_ptr(dd)
    , threadDelay(60)
{
    // Initialize
}

std::string PersonRegistrationThreadPrivate::getTime()
{
    time_t timep;
    time(&timep);
    char tmp[64];
    strftime(tmp, sizeof(tmp), "%Y%m%d%H%M%S", localtime(&timep));
    return tmp;
}

std::string PersonRegistrationThreadPrivate::md5sum(const std::string& src)
{
    std::string dst = "";
    unsigned char outmd[16] = { 0 };
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, src.c_str(), src.length());
    MD5_Final(outmd, &ctx);
    for (int i = 0; i < 16; i++)
    {
        char tmp[4] = { 0 };
        sprintf(tmp, "%02x", (unsigned char) outmd[i]);
        dst += tmp;
    }
    return dst;
}

std::string PersonRegistrationThreadPrivate::fileToBase64String(const std::string& strFilePath)
{
    std::string base_64;
    struct stat _stat;
    
    if (stat(strFilePath.c_str(), &_stat) != 0 || _stat.st_size <= 0)
    {
        LogE("%s %s[%d] Failed to get file stats: %s\n", __FILE__, __FUNCTION__, __LINE__, strFilePath.c_str());
        return base_64;
    }
    
    FILE *pFile = fopen(strFilePath.c_str(), "rb");
    if (pFile != nullptr)
    {
        char *pTmpBuf = (char*) malloc(_stat.st_size);
        if (pTmpBuf != nullptr)
        {
            long nTotalNum = 0;
            while (true)
            {
                int ret = fread(pTmpBuf + nTotalNum, 1, _stat.st_size - nTotalNum, pFile);
                if (ret <= 0) break;
                
                nTotalNum += ret;
                if (nTotalNum >= _stat.st_size) break;
            }
            base_64 = cereal::base64::encode((unsigned char const*) pTmpBuf, (size_t) _stat.st_size);
            free(pTmpBuf);
        }
        fclose(pFile);
    }
    return base_64;
}

bool PersonRegistrationThreadPrivate::doDownloadFile(const QString& url, const QString& dist)
{
    LogD("doDownloadFile() called with URL: %s", url.toStdString().c_str());
    
    if (url.isEmpty() || dist.isEmpty()) {
        LogE("Invalid parameters for doDownloadFile - URL or Destination is empty.");
        return false;
    }

    if (!url.startsWith("http")) {
        LogE("Invalid URL detected in doDownloadFile: %s", url.toStdString().c_str());
        return false;
    }

    unlink(dist.toStdString().c_str());

    std::string cmd = "/usr/bin/curl --cacert /isc/cacert.pem ";
    cmd += "  \"" + url.toStdString() + "\"";
    cmd += "  -o " + dist.toStdString();

    LogD("Download command: %s", cmd.c_str());

    int result = system(cmd.c_str());

    if (result != 0) {
        LogE("Download command failed with code: %d", result);
        return false;
    }

    LogD("Download successful: %s", dist.toStdString().c_str());

    return true;
}


QString PersonRegistrationThreadPrivate::doPostJson(const Json::Value& json)
{
    std::string jsonStr = "";
    std::string resultString = "";
    
    if (mRegistrationServerUrl.isEmpty())
    {
        LogE("%s %s[%d] Registration server URL not configured\n", __FILE__, __FUNCTION__, __LINE__);
        return "";
    }
    
    if (json.size() > 0)
    {
        Json::FastWriter fast_writer;
        jsonStr = fast_writer.write(json);
        jsonStr = jsonStr.substr(0, jsonStr.length() - 1);
    }
    
    std::string cmd = "/usr/bin/curl --cacert /isc/cacert.pem -H \"Content-Type:application/json;charset=UTF-8\" ";
    cmd += "  \"";
    cmd += mRegistrationServerUrl.toStdString().c_str();
    cmd += "\"";
    cmd += "  --connect-timeout 3 -s -X POST ";
    cmd += "  -d '";
    cmd += jsonStr.c_str();
    cmd += "'";
    
    LogD("%s %s[%d] POST command: %s\n", __FILE__, __FUNCTION__, __LINE__, cmd.c_str());
    
    FILE *pFile = popen(cmd.c_str(), "r");
    if (pFile != nullptr)
    {
        char buf[4096] = { 0 };
        int readSize = 0;
        do
        {
            readSize = fread(buf, 1, sizeof(buf), pFile);
            if (readSize > 0)
            {
                resultString += std::string(buf, 0, readSize);
            }
        } while (readSize > 0);
        pclose(pFile);
    }
    
    LogD("%s %s[%d] Response: %s\n", __FILE__, __FUNCTION__, __LINE__, resultString.c_str());
    return QString::fromStdString(resultString);
}

bool PersonRegistrationThreadPrivate::processRegistration(
    const Json::Value& personData,
    const std::string& msgType,
    const std::string& cmdId)
{
    LogD("Starting processRegistration for person: %s", 
         personData["person_name"].asString().c_str());

    // Extract data
    QString person_uuid = QString::fromStdString(personData["person_uuid"].asString());
    QString face_img = QString::fromStdString(personData["face_img"].asString());

    LogD("Received face image URL: %s", face_img.toStdString().c_str());

    // Check if face_img is a valid URL
    if (!face_img.startsWith("http")) {
        LogE("Invalid face image URL: %s", face_img.toStdString().c_str());
        return false;
    }

    // Download face image
    mkdir("/mnt/user/tmp/", 0777);
    LogD("Attempting to download face image from URL: %s", face_img.toStdString().c_str());

    if (!doDownloadFile(face_img, "/mnt/user/facedb/RegImage.jpeg")) {
        LogE("Failed to download face image from URL: %s", face_img.toStdString().c_str());
        return false;
    }

    LogD("Face image downloaded successfully to /mnt/user/facedb/RegImage.jpeg");

    // Register person and extract features
    int result = -1;
    int faceNum = 0;
    double threshold = 0;
    QByteArray faceFeature;

    LogD("Starting person registration for UUID: %s", person_uuid.toStdString().c_str());

    result = ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->RegistPerson(
        "/mnt/user/facedb/RegImage.jpeg", faceNum, threshold, faceFeature
    );

    if (result != 0) {
        LogE("Face registration failed with error code: %d", result);
        return false;
    }

    LogD("Person registration successful for UUID: %s", person_uuid.toStdString().c_str());

    // Save to database
    bool dbResult = RegisteredFacesDB::GetInstance()->RegPersonToDBAndRAM(
        person_uuid, 
        personData["person_name"].asString().c_str(),
        personData["id_card_no"].asString().c_str(),
        personData["card_no"].asString().c_str(),
        personData["sex"].asString().c_str(),
        "default_group",
        "08:00;18:00",
        faceFeature
    );

    if (!dbResult) {
        LogE("Failed to save registered person data to the database.");
        return false;
    }

    LogD("Person data successfully saved to database for UUID: %s", person_uuid.toStdString().c_str());

    // Prepare JSON response
    Json::Value responseJson;
    responseJson["msg_type"] = msgType.empty() ? "add_person" : msgType;
    responseJson["person_uuid"] = person_uuid.toStdString().c_str();
    responseJson["result"] = "1";

    // Send response to server
    QString response = doPostJson(responseJson);

    if (response.isEmpty()) {
        LogE("Failed to send registration response to the server.");
        return false;
    }

    LogD("Successfully sent registration response for UUID: %s", person_uuid.toStdString().c_str());

    return true;
}

void PersonRegistrationThreadPrivate::sendBatchReport()
{
    if (mSuccessfulRegistrations.isEmpty()) {
        return;
    }
    
    LogD("%s %s[%d] Sending batch registration report, %d entries\n", 
         __FILE__, __FUNCTION__, __LINE__, mSuccessfulRegistrations.size());
    
    // Prepare batch report
    Json::Value reportJson;
    std::string timestamp = getTime();
    std::string password = mRegistrationServerPassword.toStdString() + timestamp;
    password = md5sum(password);
    password = md5sum(password);
    transform(password.begin(), password.end(), password.begin(), ::tolower);
    
    reportJson["msg_type"] = "registration_batch_report";
    reportJson["sn"] = mSN.toStdString().c_str();
    reportJson["timestamp"] = timestamp.c_str();
    reportJson["password"] = password.c_str();
    reportJson["total_count"] = std::to_string(mSuccessfulRegistrations.size()).c_str();
    
    // Add all registered person UUIDs
    Json::Value uuidsArray(Json::arrayValue);
    for (const QString& uuid : mSuccessfulRegistrations) {
        uuidsArray.append(uuid.toStdString());
    }
    reportJson["person_uuids"] = uuidsArray;
    
    // Send the report
    QString response = doPostJson(reportJson);
    
    // Reset the list after reporting
    mSuccessfulRegistrations.clear();
}

// Implement the main thread class methods
PersonRegistrationThread* PersonRegistrationThread::GetInstance()
{
    if (m_pInstance == nullptr)
    {
        QMutexLocker locker(&m_mutex);
        if (m_pInstance == nullptr)
        {
            m_pInstance = new PersonRegistrationThread();
        }
    }
    return m_pInstance;
}

PersonRegistrationThread::PersonRegistrationThread(QObject *parent)
    : QThread(parent)
    , d_ptr(new PersonRegistrationThreadPrivate(this))
{
    this->start();
}

PersonRegistrationThread::~PersonRegistrationThread()
{
    this->requestInterruption();
    QMutexLocker locker(&d_ptr->sync);
    d_ptr->pauseCond.wakeOne();
    this->quit();
    this->wait();
}

bool PersonRegistrationThread::registerPerson(const Json::Value& personData, 
                                             const std::string& msgType,
                                             const std::string& sn,
                                             const std::string& cmdId)
{
    Q_D(PersonRegistrationThread);
    QMutexLocker locker(&d->sync);
    
    LogD("%s %s[%d] Adding person registration request to queue\n", __FILE__, __FUNCTION__, __LINE__);
    
    // Store the original data
    Json::Value registrationData = personData;
    
    // Add any missing fields from the request
    if (msgType.length() > 0 && !registrationData.isMember("msg_type")) {
        registrationData["msg_type"] = msgType;
    }
    if (sn.length() > 0 && !registrationData.isMember("sn")) {
        registrationData["sn"] = sn;
    }
    if (cmdId.length() > 0 && !registrationData.isMember("cmd_id")) {
        registrationData["cmd_id"] = cmdId;
    }
    
    // Add the registration request to the queue
    d->mPendingRegistrations.append(registrationData);
    
    // Wake up the thread if it's waiting
    d->pauseCond.wakeOne();
    
    return true;
}

void PersonRegistrationThread::run()
{
    Q_D(PersonRegistrationThread);
    sleep(10); // Initial delay
    
    LogD("%s %s[%d] Person Registration Thread started\n", __FILE__, __FUNCTION__, __LINE__);
    
    int batchReportInterval = 0;
    
    while (!isInterruptionRequested())
    {
        d->sync.lock();
        
        // Update configuration
        d->mSN = myHelper::getCpuSerial();
        d->mRegistrationServerUrl = ReadConfig::GetInstance()->getPerson_Registration_Address();
        d->mRegistrationServerPassword = ReadConfig::GetInstance()->getPerson_Registration_Password();


        LogD("Registration Server URL: %s", d->mRegistrationServerUrl.toStdString().c_str());

if (d->mRegistrationServerUrl.isEmpty()) {
    LogE("Error: Registration Server URL is EMPTY or not configured!");
} else {
    LogD("Registration Server URL: %s", d->mRegistrationServerUrl.toStdString().c_str());
}


        
        LogD("%s %s[%d] Person Registration Thread using URL: %s\n", __FILE__, __FUNCTION__, __LINE__, 
             d->mRegistrationServerUrl.toStdString().c_str());
        
        // Process any pending registrations
        if (!d->mPendingRegistrations.isEmpty())
        {
            Json::Value personData = d->mPendingRegistrations.takeFirst();
            std::string msgType = personData.isMember("msg_type") ? 
                                 personData["msg_type"].asString() : "";
            std::string cmdId = personData.isMember("cmd_id") ? 
                               personData["cmd_id"].asString() : "";
            d->processRegistration(personData, msgType, cmdId);
        }
        
        // Send batch reports periodically
        batchReportInterval++;
        if (batchReportInterval >= 60) { // Every ~60 cycles (or adjust as needed)
            batchReportInterval = 0;
            if (!d->mSuccessfulRegistrations.isEmpty()) {
                d->sendBatchReport();
            }
        }
        
        // Wait for more registrations or the next cycle
        if (d->mPendingRegistrations.isEmpty())
        {
            d->pauseCond.wait(&d->sync, 60 * 1000); // 60 seconds if no pending registrations
        }
        else
        {
            d->pauseCond.wait(&d->sync, 1000); // 1 second if we have more to process
        }
        
        d->sync.unlock();
    }
}
