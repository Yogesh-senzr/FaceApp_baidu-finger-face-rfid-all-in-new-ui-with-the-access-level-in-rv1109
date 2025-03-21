#include "HaoyuIdentificationCheck.h"

#include "json-cpp/json.h"
#include <openssl/ssl.h>
#ifdef Q_OS_LINUX
#include <curl/curl.h>
#include <curl/easy.h>
#include "PCIcore/Audio.h"
#endif
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "Zhangjiakou/AES/common/base64.h"
#include "BaseFace/BaseFaceManager.h"

#include "MessageHandler/Log.h"
#include "ManageEngines/PersonRecordToDB.h"
#include "Helper/myhelper.h"
#include "Application/FaceApp.h"

#include <QThread>
#include <sys/time.h>
#include <QBuffer>
#include <QDebug>
#include <QDateTime>

HaoyuIdentificationCheck::HaoyuIdentificationCheck(QObject *parent) : QObject(parent)
{
    QThread *thread = new QThread;
    this->moveToThread(thread);
    thread->start();
}

static inline int write_data(void *buffer, size_t sz, size_t nmemb, void *ResInfo)
{
    std::string* psResponse = (std::string*) ResInfo; //强制转换
    psResponse->append((char*) buffer, sz * nmemb); //sz*nmemb表示接受数据的多少
    return sz * nmemb;  //返回接受数据的多少
}

static inline void getTimeStr(char* str)
{
    if (str == NULL)
        return;
    time_t timep;
    struct tm *p;
    time(&timep);
    p = localtime(&timep);
    sprintf(str, "%04d/%02d/%02d %02d:%02d:%02d", (1900 + p->tm_year), (1 + p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
}

static inline std::string PosDeviceUserAuthResult(std::string url, std::string requestData)
{
    Q_UNUSED(url);
    Q_UNUSED(requestData);

    std::string ResString;
#ifdef Q_OS_LINUX
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    //	LogV("%s %s [%d] requestUrl=%s \n",__FILE__,__FUNCTION__,__LINE__,requestUrl);
    if (curl)
    {
#if 1
        struct curl_slist *headers = 0;
        headers = curl_slist_append(headers, "Content-Type:application/json;charset=utf-8");

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl,CURLOPT_SSLVERSION,1);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data); //数据请求到以后的回调函数
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ResString);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestData.c_str());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
        res = curl_easy_perform(curl);
#else
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json;charset=utf-8");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data); //数据请求到以后的回调函数
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ResString);
        LogV("%s %d %s", __FUNCTION__, __LINE__, requestData.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestData.c_str());
        curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION, *sslctx_function);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
        //		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        res = curl_easy_perform(curl);
#endif
    }
    if (res != CURLE_OK)
    {
        LogV("%s %s[%d] curl_easy_perform() failed: %s\n", __FILE__, __FUNCTION__, __LINE__, curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return "";
    }
    curl_easy_cleanup(curl);
#endif

    return ResString;
}

static inline std::string fileToBase64StringFace(const char * strFilePath)
{
    std::string base_64;
    struct stat _stat;
    stat(strFilePath, &_stat);
    if (_stat.st_size > 0)
    {
        FILE *pFile = fopen(strFilePath, "rb");
        if (pFile != 0)
        {
            char *pTmpBuf = (char*) malloc(_stat.st_size);
            if (pTmpBuf != 0)
            {
                long nTotalNum = 0;
                while (true)
                {
                    int ret = fread(pTmpBuf + nTotalNum, 1, _stat.st_size, pFile);
                    nTotalNum += ret;
                    if (nTotalNum >= _stat.st_size)
                    {
                        break;
                    }
                }
                base_64 = base64_encode((unsigned char const*) pTmpBuf, (size_t) _stat.st_size);
                free(pTmpBuf);
            }
            fclose(pFile);
        }else LogV("%s %s[%d] fopen fail \n", __FILE__, __FUNCTION__, __LINE__);
    }
    return base_64;
}

void HaoyuIdentificationCheck::slotPostFaceInfo(const float TemperatureValue, const int mask)
{//{"result":0,"message":"绿码通行"}
#ifdef Q_OS_LINUX
    YNH_LJX::Audio::Audio_PlayCustomerPcm("zh", "be_inquiring.wav", true);
    qXLApp->GetAlgoFaceManager()->saveCurFaceImg("/mnt/user/temphaiyu.jpeg", 75);
#endif
    QString base64 = QString::fromStdString(fileToBase64StringFace("/mnt/user/temphaiyu.jpeg"));
    QString postData =  QString("{\"uid\":\"%1\",\"sn\":\"%2\",\"data\":{\"snapFaceBase64\":\"%3\","
                                "\"temperature\" :\"%4\",\"isMask\":\"%5\""
                                "}}")
            .arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"))
            .arg(myHelper::getCpuSerial())
            .arg(base64)
            .arg(TemperatureValue)
            .arg(mask);
    QString message;
    std::string ResString = PosDeviceUserAuthResult("https://36.159.72.12/hapi/api/h/iot/face/getDeviceUserAuthResult", postData.toStdString());
    printf("%s %s[%d] ResString  %s\n", __FILE__, __FUNCTION__, __LINE__, ResString.c_str());
    int result = 1;
    Json::Value root;
    Json::Reader reader;
    if(!reader.parse(ResString.c_str(), root))
    {
        if(ResString.length())
        {
            message = QObject::tr("ServerException");//服务器异常
        }else
        {
#ifdef Q_OS_LINUX//直接报网络异常
            YNH_LJX::Audio::Audio_PlayCustomerPcm("zh", "net_error.wav", true);
#endif
            message = QObject::tr("NetworkException");//网络异常
        }
        LogV("%s %s[%d] ResString parse fai %s\n", __FILE__, __FUNCTION__, __LINE__, ResString.c_str());
    }else
    {
        result = root["result"].asInt();
        message = QString::fromStdString(root["message"].asString());
    }
    printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
    emit sigUserAuthResult(result, message);
}
