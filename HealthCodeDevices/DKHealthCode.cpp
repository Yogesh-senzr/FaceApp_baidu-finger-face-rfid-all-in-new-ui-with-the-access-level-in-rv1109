#include "DKHealthCode.h"

#include "PCIcore/Audio.h"
#include "SharedInclude/GlobalDef.h"
#include "PCIcore/RkUtils.h"

#include "json-cpp/json.h"
#include <openssl/ssl.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include "MessageHandler/Log.h"

#include "Helper/myhelper.h"

#include <dlfcn.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <QWaitCondition>
#include <QMutex>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>

#define STATUS_10000   10000
#define STATUS_11003   11003
#define STATUS_40000   40000
#define STATUS_40001   40001
#define STATUS_40006   40006
#define STATUS_40007   40007
#define STATUS_40008   40008
#define STATUS_40009   40009
#define STATUS_40011   40011
#define UART_PORT "/dev/ttyACM0"
//#define UART_PORT "/dev/ttyS2"

typedef int (*IF_InitDevice)(unsigned char *serialPort);
typedef void (*IF_GetCtidQRCode)(unsigned char *outData, int *outLen);
typedef void (*IF_SetSignData)(char *key, char *value);
typedef int (*IF_GetCheckData)(char *jsonData);
typedef int (*IF_GetIdentityCheckData)(unsigned char *name, unsigned char *id, unsigned char *publicKey, unsigned int publicKeyLen,
                                       unsigned char *publicVer, unsigned char *randomNum, char *jsonData);
typedef void (*IF_ClearCtidQRCode)();
typedef void (*IF_CloseDevice)();
typedef int (*IF_GetVersion)(unsigned char *version);

class DKHealthCodePrivate
{
    Q_DECLARE_PUBLIC(DKHealthCode)
public:
    DKHealthCodePrivate(DKHealthCode *dd);
private:
    void InitDKHealthCodeLib();
private:
    void findHandleDKHealthCode();
    void DealDKHealthCode(const int &type, const QString &);
private:
    bool mInitLib;
    QString DK_IDCardSN;
    IF_InitDevice InitDevice;
    IF_GetCheckData GetCheckData;
    IF_CloseDevice CloseDevice;
    IF_ClearCtidQRCode ClearCtidQRCode;
    IF_SetSignData SetSignData;
    IF_GetCtidQRCode GetCtidQRCode;
    IF_GetIdentityCheckData GetIdentityCheckData = NULL;
private:
    mutable QMutex sync;
    QWaitCondition pauseCond;
private:
    DKHealthCode *const q_ptr;
};

DKHealthCodePrivate::DKHealthCodePrivate(DKHealthCode *dd)
    : q_ptr(dd)
    , mInitLib(false)
{
#if 1 //移到后面启动    
    QSettings sysIniFile("/isc/DK_Device.ini", QSettings::IniFormat);
    this->DK_IDCardSN = sysIniFile.value("SN",  "").toString();
    this->InitDKHealthCodeLib();
#endif     
    LogV("%s %s[%d] \n", __FILE__, __FUNCTION__, __LINE__ );
}

DKHealthCode::DKHealthCode(QObject *parent)
    : QThread(parent)
    , d_ptr(new DKHealthCodePrivate(this))
{
    this->start();
}

DKHealthCode::~DKHealthCode()
{
    Q_D(DKHealthCode);
    Q_UNUSED(d);
    this->requestInterruption();
    d->pauseCond.wakeOne();

    this->quit();
    this->wait();
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

static inline std::string PosDKHealthCode(std::string url, std::string requestData)
{
    std::string ResString;
   
#if 1
    std::string cmd = "/usr/bin/curl -H \"Content-Type:application/json;charset=UTF-8\" ";
    cmd += "  ";
    cmd += url.c_str();
    cmd += "  --cacert /isc/cacert.pem ";
    cmd += "--connect-timeout 3 -s -X POST ";
    //cmd += "--data-raw '";
   // cmd += "-d 'json=";
    cmd += "-d '";
    cmd += requestData.c_str();
    cmd +="'";
    std::string ret = "";    
#define ISC_NULL                 0L

    //qDebug()<<"cmd : "<<cmd.c_str();
    FILE *pFile = popen(cmd.c_str(), "r");
    if (pFile != ISC_NULL)
    {
        char buf[4096] = { 0 };
        int readSize = 0;
        do
        {
            readSize = fread(buf, 1, sizeof(buf), pFile);
            if (readSize > 0)
            {
                ret += std::string(buf, 0, readSize);
            }
        } while (readSize > 0);
        pclose(pFile);
        //qDebug()<<"buf : "<<buf;   
        if (ret.length()>10 )
        {
          // res = CURLE_OK;
           ResString = ret;
        }
    }

#endif 
    return ResString;
}

void DKHealthCodePrivate::InitDKHealthCodeLib()
{
    void *pHandle = NULL;
    pHandle = dlopen("/usr/lib/libhealthcode_module_4800.so", RTLD_NOW | RTLD_LOCAL | RTLD_DEEPBIND);
    LogV("%s %s[%d] handle %x %s\n", __FILE__, __FUNCTION__, __LINE__, pHandle, dlerror() );
    if (pHandle != NULL)
    {
        InitDevice = (IF_InitDevice) dlsym(pHandle, "initDevice");
        CloseDevice = (IF_CloseDevice) dlsym(pHandle, "closeDevice");
        GetCtidQRCode = (IF_GetCtidQRCode) dlsym(pHandle, "getCtidQRCode");
        GetCheckData = (IF_GetCheckData) dlsym(pHandle, "getCheckData");
        ClearCtidQRCode = (IF_ClearCtidQRCode) dlsym(pHandle, "clearCtidQRCode");
        SetSignData = (IF_SetSignData) dlsym(pHandle, "setSignData");
        GetIdentityCheckData = (IF_GetIdentityCheckData) dlsym(pHandle, "getIdentityCheckData");

        //system("chmod 666 /dev/ttyS1");
        system("chmod 666 /dev/ttyS2");
        LogV("%s %s[%d] \n", __FILE__, __FUNCTION__, __LINE__ );
        if (InitDevice((unsigned char*) UART_PORT) == 1)
        {
            LogV("DKHealthCode init success \n");
        } else
        {
            LogV("DKHealthCode init fail,end! \n");
            return;
        }
    }else
    {
        return;
    }
    mInitLib = true;
}

void DKHealthCodePrivate::findHandleDKHealthCode()
{
    char szCodeData[4096] = { 0 };
    unsigned char recvBuf[1024] = { 0 };
    int recvLen = 0;
    Json::Reader reader;
    Json::Value root;
    Json::Value jsonData;
    std::string strRetJson;
    int ret = 0;
    memset(szCodeData, 0, sizeof(szCodeData));
    memset(recvBuf, 0, sizeof(recvBuf));

    this->GetCtidQRCode(recvBuf, &recvLen);
    if (recvLen <= 0)return;
    ret = this->GetCheckData(szCodeData);
    this->ClearCtidQRCode();
    LogV("%s %s[%d] GetCheckData ret :%d Data=%s\n", __FILE__, __FUNCTION__, __LINE__, ret, szCodeData);
    if(ret != 0)return;
    else if (reader.parse(szCodeData, root))
    {//提示正在查询
        YNH_LJX::Audio::Audio_PlayCustomerPcm("zh", "be_inquiring.wav", true);

        jsonData["devValue"] = root["qrCode"]["value"].asString();
        jsonData["devSign"] = root["qrCode"]["sign"].asString();
        jsonData["sn"] = root["qrCode"]["sn"].asString();
        jsonData["currentBodyTemp"] = 0;
        // jsonData["authorizationCode"] = isc_setting::SStting::instance().getDeviceSN();
        jsonData["authorizationCode"] = "13512250204191231238";
        jsonData["strCompanyAddress"] = "";//isc_setting::SStting::instance().getLocation();
        jsonData["nonce"] = "c66bb4065c47a476";
        jsonData["sign"] = "3a4f615b0cd36a4";
        jsonData["source"] = "1";
        if (DK_IDCardSN.toStdString() != root["qrCode"]["sn"].asString())
        {
            DK_IDCardSN = QString::fromStdString(root["qrCode"]["sn"].asString());
            //写入ini
            QSettings sysIniFile("/isc/DK_Device.ini", QSettings::IniFormat);
            sysIniFile.setValue("SN",  DK_IDCardSN);
        }

        strRetJson = PosDKHealthCode("https://fk.rakinda.cn:7443/healthCode/verification", jsonData.toStyledString());
        this->DealDKHealthCode(1, QString::fromStdString(strRetJson));
    }
}

void DKHealthCodePrivate::DealDKHealthCode(const int &type, const QString &strRetJson)
{
    //qDebug()<<strRetJson;
    Json::Reader reader;
    Json::Value root;
    if (strRetJson.size())
    {
        if (reader.parse(strRetJson.toStdString(), root))
        {
            int code = root["code"].asInt();
            int qrCodeType = -1;
            std::string msg = root["msg"].asString();
            std::string name;
            std::string idcardNum;
            switch(code)
            {
            case 10000:
            {
                Json::Value data_root;
                std::string data = root["data"].asString();
                //printf("%s %s[%d] data : %s \n", __FILE__, __FUNCTION__, __LINE__, data.c_str());
                if (reader.parse(data, data_root))
                {
                    std:: string type = data_root["qrCodeType"].asString();
                    qrCodeType = atoi(type.c_str());
                    name = data_root["name"].asString();
                    idcardNum = data_root["idCard"].asString();
                }
                if(qrCodeType == 0)
                    YNH_LJX::Audio::Audio_PlayCustomerPcm("zh", "green_code.wav", true);
                else if(qrCodeType == 1 || qrCodeType == 10)
                    YNH_LJX::Audio::Audio_PlayCustomerPcm("zh", "red_code.wav", true);
                //上传UI
                printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
                emit q_func()->sigDKHealthCodeMsg(type, QString::fromStdString(name), QString::fromStdString(idcardNum), qrCodeType, 37.3, QString::fromStdString(msg));
            }break;
            case 10006:
            	printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
                emit q_func()->sigDKHealthCodeMsg(type, QString(), QString(), 10006, 37.3, QObject::tr("SignatureError"));//签名错误
                break;
            case 40000:
            	printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
                emit q_func()->sigDKHealthCodeMsg(type, QString(), QString(), 10006, 37.3, QObject::tr("DeviceNotAuthorized."));//设备未授权,Device is not authorized.
                break;
            case 40001:
            	printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
                emit q_func()->sigDKHealthCodeMsg(type, QString(), QString(), 10006, 37.3, QObject::tr("WrongAuthorizationCode"));//Wrong authorization code,授权码错误
                break;
            case 40006:
            	printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
                emit q_func()->sigDKHealthCodeMsg(type, QString(), QString(), 10006, 37.3, QObject::tr("3partyServicesBusy"));//第三方服务繁忙,Third-party services are busy
                break;
            case 40007:
            	printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
                YNH_LJX::Audio::Audio_PlayCustomerPcm("zh", "qrcode_over_date.wav", true);
                emit q_func()->sigDKHealthCodeMsg(type, QString(), QString(), 10006, 37.3, QObject::tr("QRcodeExpired"));//二维码已失效或已过期,The QR code has expired or expired
                break;
            case 40008:
            	printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
                emit q_func()->sigDKHealthCodeMsg(type, QString(), QString(), 10006, 37.3, QObject::tr("RequestFailed"));//请求结果解析失败，请重试 Request resolution failed, please try again.
                break;
            case 40009:
            	printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
                emit q_func()->sigDKHealthCodeMsg(type, QString(), QString(), 10006, 37.3, QObject::tr("WrongRequestResult"));//Wrong request result 请求结果错误
                break;
            case 40017:
            	printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
                emit q_func()->sigDKHealthCodeMsg(type, QString(), QString(), 10006, 37.3, QObject::tr("PlsTryAgain"));//请重试 Please try again
                break;
            case 40019:
            	printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
                emit q_func()->sigDKHealthCodeMsg(type, QString(), QString(), 10006, 37.3, QObject::tr("SystemMaintenancing"));//平台维护中，请稍后重试,The platform is under maintenance, please try again later
                break;
            default:break;
            }
        }
    }else
    {//未读到正常数据，即表示网络异常，或服务器异常
    	printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
        YNH_LJX::Audio::Audio_PlayCustomerPcm("zh", "net_error.wav", true);
        emit q_func()->sigDKHealthCodeMsg(type, QString(), QString(), -1, 37.3, QObject::tr("NetworkException"));//网络异常
    }
}

void DKHealthCode::setQueryHealthCode(const QString &name, const QString &idCard, const QString &sex)
{//身份证解析完成，开始和 远景达的后台接口开始交互 ,此流程中设备必须接有远景达的设备
    Q_UNUSED(name);
    Q_UNUSED(idCard);
    Q_UNUSED(sex);
    Q_D(DKHealthCode);

    //printf("%s %s[%d] ApplyWater start \n", __FILE__, __FUNCTION__, __LINE__);
    std::string strRetJson;
    Json::Value root;
    Json::Reader reader;

    //TODO STEP 1 申请流水号
    QJsonObject jsonObject;
    jsonObject.insert("authorizationCode", "13512250204191231238");
    jsonObject.insert("idCardSn", d->DK_IDCardSN);
    jsonObject.insert("nonce", "c66bb4065c47a476");
    jsonObject.insert("sign", "3a4f615b0cd36a4");

    //使用QJsonDocument设置该json对象
    QJsonDocument jsonDoc;
    jsonDoc.setObject(jsonObject);

    strRetJson = PosDKHealthCode("https://fk.rakinda.cn:7443/healthCode/apply",  jsonDoc.toJson().toStdString());
    LogV("%s %s[%d] strRetJson %s \n", __FILE__, __FUNCTION__, __LINE__, strRetJson.c_str());
    if (strRetJson.length() > 10)
    {
        if (reader.parse(strRetJson, root))
        {
            int code = root["code"].asInt();
            if (code == 10000) // 申请流水正常
            {
                Json::Value data_root;
                std::string data = root["data"].asString();
                if (reader.parse(data, data_root))
                {
                    QString randomNumber = QString::fromStdString(data_root["randomNumber"].asString());
                    QString bsn = QString::fromStdString(data_root["bsn"].asString());
                    LogV("%s %s[%d]\n", __FILE__, __FUNCTION__, __LINE__);
                    //提示正在查询
                    YNH_LJX::Audio::Audio_PlayCustomerPcm("zh", "be_inquiring.wav", false);

                    if (randomNumber.length() > 0 && bsn.length() > 0)
                    {
                        QJsonObject jsonObject;
                        jsonObject.insert("authorizationCode", d->DK_IDCardSN);
                        jsonObject.insert("randomNumber", randomNumber);
                        jsonObject.insert("bsn", bsn);
                        jsonObject.insert("nonce", "c66bb4065c47a476");
                        jsonObject.insert("sign", "3a4f615b0cd36a4");

                        unsigned char publicKey[64] = { 0x31, 0x49, 0xb5, 0x57, 0xd6, 0xe1, 0x01, 0xd0, 0x2d, 0x4f, 0xb3, 0xd6, 0x7d,
                                                        0xed, 0xb3, 0x3d, 0xde, 0x32, 0x48, 0x4e, 0xc8, 0x7e, 0xa4, 0xbd, 0xe5, 0xc2, 0x50, 0xe7, 0xaf, 0xe1,
                                                        0x64, 0xfe, 0x10, 0x68, 0xf7, 0xda, 0x5c, 0x35, 0xd9, 0x93, 0x17, 0xd9, 0x2f, 0x6e, 0xea, 0xe6, 0x3c,
                                                        0xb1, 0x53, 0xb4, 0x42, 0xf6, 0x92, 0x64, 0xd3, 0x8f, 0xd4, 0x43, 0x9e, 0xf2, 0xfc, 0xc2, 0x8c, 0x62 };
                        unsigned char *randomNum = (unsigned char *) randomNumber.toLatin1().data();
                        unsigned char *version = (unsigned char *) "V1.0";
                        char szData[4096] = { 0 };
                        int ret = 0;
                        ret = this->IdentityCheckData((uchar *)name.toLatin1().data(), (uchar *)idCard.toLatin1().data(), publicKey, 64, version, randomNum, szData);
                        LogV("%s %s[%d] ret %d \n", __FILE__, __FUNCTION__, __LINE__, ret);
                        LogV("%s %s[%d] szData %s \n", __FILE__, __FUNCTION__, __LINE__, szData);
                        //jsonData["identityCheckData"] = szData;
                        jsonObject.insert("identityCheckData", szData);

                        //使用QJsonDocument设置该json对象
                        QJsonDocument jsonDoc;
                        jsonDoc.setObject(jsonObject);

                        LogV("%s %s[%d] %s\n", __FILE__, __FUNCTION__, __LINE__, jsonDoc.toJson().toStdString().c_str());

                        std::string strRetJson = PosDKHealthCode("https://fk.rakinda.cn:7443/healthCode/verificationIdCard", jsonDoc.toJson().toStdString());
                        //printf("%s %s[%d] strRetJson %s \n", __FILE__, __FUNCTION__, __LINE__, strRetJson.c_str());
                        d->DealDKHealthCode(2, QString::fromStdString(strRetJson));
                    }
                }
            }else
            {
            	printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
                emit sigDKHealthCodeMsg(2, name, idCard, -1, 37.3, QObject::tr("ServerException"));//服务器异常
            }
        }
    }else
    {//未读到正常数据，即表示网络异常，或服务器异常
    	printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
#ifdef Q_OS_LINUX//直接报网络异常
        YNH_LJX::Audio::Audio_PlayCustomerPcm("zh", "net_error.wav", true);
#endif
        emit sigDKHealthCodeMsg(2, name, idCard, -1, 37.3, QObject::tr("NetworkException"));//网络异常
    }
}

int DKHealthCode::IdentityCheckData(unsigned char *name, unsigned char *id, unsigned char *publicKey, unsigned int publicKeyLen, unsigned char *publicVer, unsigned char *randomNum, char *jsonData)
{
    Q_D(DKHealthCode);
    if(!d->GetIdentityCheckData)return -1;
    return d->GetIdentityCheckData(name, id, publicKey, publicKeyLen, publicVer, randomNum, jsonData);
}

void DKHealthCode::run()
{
    Q_D(DKHealthCode);
#if 0 //移到此处启动    
    QSettings sysIniFile("/isc/DK_Device.ini", QSettings::IniFormat);
    d->DK_IDCardSN = sysIniFile.value("SN",  "").toString();
    d->InitDKHealthCodeLib();
#endif   

    while (!isInterruptionRequested())
    {
        d->sync.lock();
        if (!d->mInitLib)d->pauseCond.wait(&d->sync);
        else d->findHandleDKHealthCode();
        d->pauseCond.wait(&d->sync, 200);
        d->sync.unlock();
    }
}
