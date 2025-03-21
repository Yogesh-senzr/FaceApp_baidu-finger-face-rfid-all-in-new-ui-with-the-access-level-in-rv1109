#include "ZhangjiakouHealthCode.h"
#include "AES/aesencrypt.h"
#include "MD5/md5.h"
#include "MD5/hashmd5.h"
#include "algo_hmac.h"

#ifdef Q_OS_LINUX
#include "PCIcore/Audio.h"
#include <curl/curl.h>
#include <curl/easy.h>

#endif
#include "MessageHandler/Log.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "AES/common/base64.h"

#include <QDebug>
#include <QMessageAuthenticationCode>
#include <QDateTime>
#include <QSettings>
#include <QLibrary>
#include <QJsonDocument>
#include <QJsonObject>
#include <QBuffer>
#include <QImage>
#include <QFile>
#include <QTextCodec>
#include <QPixmap>
#include <QThread>

#ifdef Q_OS_WIN
typedef  int(__stdcall *PostZJKHttps)(const char *url, const char *postData, const char *appid, const char *timestamp, const char *sign, char *strResponse, int* strResponse_len);
PostZJKHttps zjkPost = Q_NULLPTR;
#endif

class ZhangjiakouHealthCodePrivate
{
    Q_DECLARE_PUBLIC(ZhangjiakouHealthCode)
public:
    ZhangjiakouHealthCodePrivate(ZhangjiakouHealthCode *dd);
private:
    QString HeaderPackage(const QString &name, const QString &idCard, QString &sign, int &timestamp);
    QString HeaderPackage(const QString &json, QString &sign, int &timestamp);
    bool DealZhangjiakouHealthCode(const QString &name, const QString &idCard, const QString &json);
    void postTempRecognition(const QString &json);
private:
    QString mHealthCodeUrl;//健康码
    QString mRecognitionUrl;//上传记录
    QString mDeviceId;
    QString mDeviceName;
    QString mAppId;
    QString mAppSecret;
    QString mAreaId;
    QString mAreaName;
    QString mCounty;
private:
    ZhangjiakouHealthCode *const q_ptr;
};

ZhangjiakouHealthCodePrivate::ZhangjiakouHealthCodePrivate(ZhangjiakouHealthCode *dd)
    : q_ptr(dd)
{//读取配置表路径
#ifdef Q_OS_WIN
    QSettings IniFile("./HealthCodeSrv.ini", QSettings::IniFormat);
#else
    QSettings IniFile("/mnt/user/HealthCodeSrv.ini", QSettings::IniFormat);
#endif
    IniFile.setIniCodec(QTextCodec::codecForName("UTF8"));
    IniFile.beginGroup("Zhangjiakou");
    mHealthCodeUrl = IniFile.value("HealthCodeUrl", "http://60.8.117.87:8091/open-api/v1/rabbit-person/health-code").toString();
    mRecognitionUrl = IniFile.value("RecognitionUrl", "http://60.8.117.87:8091/open-api/v1/rabbit-record/recognition").toString();

    mDeviceId = IniFile.value("DeviceId",  "T12D6000351Y").toString();
    mDeviceName = IniFile.value("DeviceName",  "下花园区住房和城乡建设局").toString();
    mAppId = IniFile.value("AppId",  "a4185be6ef3c4080924276de9280505a").toString();
    mAppSecret = IniFile.value("AppSecret",  "266f776FeEA54a52a960F66D7Ca9a181").toString();
    mAreaId = IniFile.value("AreaId",  "63462").toString();
    mAreaName = IniFile.value("AreaName",  "下花园区住房和城乡建设局").toString();
    mCounty = IniFile.value("County",  "下花园区").toString();
    IniFile.endGroup();
}

ZhangjiakouHealthCode::ZhangjiakouHealthCode(QObject *parent)
    : QObject(parent)
    , d_ptr(new ZhangjiakouHealthCodePrivate(this))
{
#ifdef Q_OS_WIN
    QLibrary lib("./ZhangjiakouSrv.dll");
    if (lib.load())zjkPost = (PostZJKHttps)lib.resolve("PostZhangjiakouHttps");
#endif

    QThread *thread = new QThread;
    this->moveToThread(thread);
    thread->start();
}

ZhangjiakouHealthCode::~ZhangjiakouHealthCode()
{

}

static inline QImage pixmapScale(const QImage& image, const int &width, const int &height)
{
    QImage r_image;
    r_image = image.scaled(width, height, Qt::KeepAspectRatio);
    return r_image;
}

static inline QString Sort(const QString &srcappid, const QString &srcappsecret, const QString &srctimestamp, const QString &srcdata, const QString &srcsign1)
{
    QString appid = srcappid.toLower();
    QString appsecret = srcappsecret.toLower();
    QString timestamp = srctimestamp.toLower();
    QString data = srcdata.toLower();
    QString sign1 = srcsign1.toLower();

    QByteArray b_data = data.toUtf8();
    if((b_data.at(0) == 47) || (b_data.at(0) == 43) || (b_data.at(0) == 61))
        data = "000000";

    QByteArray b_sign1 = sign1.toUtf8();
    if((b_sign1.at(0) == 47) || (b_sign1.at(0) == 43) || (b_sign1.at(0) == 61))
        sign1 = "000000";

    //TODO yaosen 这是有问题，因为会有几率出现key相同
    QHash<QString, QString>params;
    params.insert(appid, srcappid);
    params.insert(appsecret, srcappsecret);
    params.insert(timestamp, srctimestamp);
    params.insert(data, srcdata);
    params.insert(sign1, srcsign1);

    auto list = params.keys();
    std::sort(list.begin(), list.end());

    QString msb;
    for(int i = 0; i<list.count(); i++)
    {
        msb.append(params.value(list.at(i)));
    }
    return msb;
}

static inline QString JsonCrypt(const QString &AppSecret, const QString &iv, const QString &idcard, const QString &name)
{
    QByteArray inputStr = QByteArray().append(QString("{\"idCard\":\"%1\",\"name\":\"%2\"}").arg(idcard).arg(name));
    std::string str = AesEncrypt::AesCbcEncrypt(inputStr.toStdString(),
                                                AppSecret.toStdString(),
                                                iv.toStdString(),
                                                AesEncrypt::PKCS7Padding);
    return QString::fromStdString(str);
}

static inline QString JsonCrypt(const QString &AppSecret, const QString &iv, const QString &json)
{
    std::string str = AesEncrypt::AesCbcEncrypt(json.toStdString(),
                                                AppSecret.toStdString(),
                                                iv.toStdString(),
                                                AesEncrypt::PKCS7Padding);
    return QString::fromStdString(str);
}

static inline QString DataToMD5(const QString &idcard, const QString &name)
{
    QString data= QString("{\"idCard\":\"%1\",\"name\":\"%2\"}").arg(idcard).arg(name);
#if 1
    std::string m =  md5(data.toStdString());
    return QString::fromStdString(m);
#else
    return QCryptographicHash::hash(data.toLatin1(), QCryptographicHash::Md5).toHex().toLower();
#endif
}

static inline QString DataToMD5(const QString &json)
{
#if 1
    std::string m =  md5(json.toStdString());
    return QString::fromStdString(m);
#else
    return QCryptographicHash::hash(data.toLatin1(), QCryptographicHash::Md5).toHex().toLower();
#endif
}

static inline int write_data(void *buffer, size_t sz, size_t nmemb, void *ResInfo)
{
    std::string* psResponse = (std::string*) ResInfo; //强制转换
    psResponse->append((char*) buffer, sz * nmemb); //sz*nmemb表示接受数据的多少
    return sz * nmemb;  //返回接受数据的多少
}

static inline std::string fileToBase64StringFace(const char * strFilePath)
{
    std::string base_64;
    struct stat _stat;
    stat(strFilePath, &_stat);
    //printf("%s %s[%d] fileSize %d \n", __FILE__, __FUNCTION__, __LINE__, _stat.st_size);

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

static inline std::string PosQueryingHealthCodes(const std::string &url, const std::string &appid, const std::string &time, const std::string &sign, const std::string &requestData)
{
    Q_UNUSED(url);
    Q_UNUSED(appid);
    Q_UNUSED(time);
    Q_UNUSED(sign);
    Q_UNUSED(requestData);
#ifdef Q_OS_LINUX
    std::string ResString;
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    //LogV("%s %s [%d] requestUrl=%s \n",__FILE__,__FUNCTION__,__LINE__,requestData.c_str());
    if (curl)
    {
        struct curl_slist *headers = 0;
        headers = curl_slist_append(headers, "Content-Type:application/json;charset=utf-8");
        headers = curl_slist_append(headers, appid.c_str());
        headers = curl_slist_append(headers, time.c_str());
        headers = curl_slist_append(headers, sign.c_str());

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
    }
    if (res != CURLE_OK)
    {
#ifdef Q_OS_LINUX//直接报网络异常
        YNH_LJX::Audio::Audio_PlayCustomerPcm("zh", "net_error.wav", true);
#endif
        LogV("%s %s[%d] curl_easy_perform() failed: %s\n", __FILE__, __FUNCTION__, __LINE__, curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return "";
    }
    curl_easy_cleanup(curl);
    return ResString;
#else
    char out[1024] = {0};
    int outlen = 0;
    zjkPost(url.c_str(), requestData.c_str(), appid.c_str(), time.c_str(), sign.c_str(), out, &outlen);
    return std::string(out, outlen);
    //return std::string("{\"status\":0,\"errorMsg\":\"\",\"result\":{\"codeLevel\":0,\"message\":\"绿码\",\"warning\":false,\"tempWarn\":true,\"warningTemp\":37.3}}");
#endif
}

QString ZhangjiakouHealthCodePrivate::HeaderPackage(const QString &json, QString &sign, int &timestamp)
{
    timestamp = QDateTime::currentDateTime().toTime_t();
    QString dataMD5 =  DataToMD5(json);
    QString paramStr1;
    QString paramStr2;
    QString sign1;
    {
        //将参与签名的参数(appid、timestamp 必须参与签名)升序排序，并使用 url 键值
        //对（appId=valueA&key1 =valueB& … key2=valued&timestamp=valueC）格式
        //拼接为 paramStr1。
        paramStr1 = QString("appid=%1&appsecret=%2&data=%3&timestamp=%4").arg(this->mAppId).arg(this->mAppSecret).arg(dataMD5).arg(timestamp);
    }
    {
#if 1
        //利用 HmacSHA256 算法，appsecret 做为私钥，将 paramStr1 进行 HmacSHA256加密处理转 base64 得到待加密签名，
        //即 sign1 = HmacSHA256(appsecret, paramStr1)
        sign1 = QMessageAuthenticationCode::hash(paramStr1.toLatin1(), this->mAppSecret.toLatin1(), QCryptographicHash::Sha256).toBase64();
#else
        const std::string str_data = paramStr1.toStdString();
        const std::string str_key = appsecret.toStdString();
        unsigned char * mac = NULL;
        unsigned int mac_length = 0;
        HmacEncode("sha256", str_key.c_str(), str_key.length(), str_data.c_str(), str_data.length(), mac, mac_length);
        //qDebug()<<"HmacEncode "<<ret <<" lenth: "<<mac_length;
        QByteArray ba;
        ba.append((char *)mac, mac_length);
        sign1.append(ba.toBase64());
#endif
    }

    {//将待加密签名（sign1）及签名参数（不含 sign）的值，升序排序拼接为paranmStr2。
        paramStr2 = Sort(this->mAppId, this->mAppSecret, QString::number(timestamp), dataMD5, sign1);
    }

    {
#if 1
        //利用哈希散列算法 HashAlgorithm MD5 方式加密待加密字符串 paranmStr2，转字符串得到全大写签名即 sign。
        //计算md5值，传入需要被计算的数据，传入计算的类型
        sign = QCryptographicHash::hash(paramStr2.toLatin1(), QCryptographicHash::Md5).toHex().toUpper();
#else
        std::string encodedStr;
        std::string encodedHexStr;
        Hashmd5(paramStr2.toStdString(), encodedStr, encodedHexStr);
        sign = QString::fromStdString(encodedHexStr).toUpper();
#endif
    }
    QString iv = "";
    for (int i = 1; i < sign.length(); i = i + 2) {iv += sign.mid(i, 1);}
    QString data = QString("{\"data\":\"%1\"}").arg(JsonCrypt(this->mAppSecret, iv, json));
#if 0
    qDebug()<<"iv "<<iv;
    qDebug()<<"paramStr1 "<<paramStr1;
    qDebug()<<"sign1 "<<sign1;
    qDebug()<<"paramStr2 "<<paramStr2;
    qDebug()<<"sign "<<sign;
    qDebug()<<"DataMD5 "<<dataMD5;
    qDebug()<<"postData "<<data;
#endif
    return data;
}

QString ZhangjiakouHealthCodePrivate::HeaderPackage(const QString &name, const QString &idCard, QString &sign, int &timestamp)
{
    timestamp = QDateTime::currentDateTime().toTime_t();
    QString dataMD5 =  DataToMD5(idCard, name);
    QString paramStr1;
    QString paramStr2;
    QString sign1;
    {
        //将参与签名的参数(appid、timestamp 必须参与签名)升序排序，并使用 url 键值
        //对（appId=valueA&key1 =valueB& … key2=valued&timestamp=valueC）格式
        //拼接为 paramStr1。
        paramStr1 = QString("appid=%1&appsecret=%2&data=%3&timestamp=%4").arg(this->mAppId).arg(this->mAppSecret).arg(dataMD5).arg(timestamp);
    }
    {
#if 1
        //利用 HmacSHA256 算法，appsecret 做为私钥，将 paramStr1 进行 HmacSHA256加密处理转 base64 得到待加密签名，
        //即 sign1 = HmacSHA256(appsecret, paramStr1)
        sign1 = QMessageAuthenticationCode::hash(paramStr1.toLatin1(), this->mAppSecret.toLatin1(), QCryptographicHash::Sha256).toBase64();
#else
        const std::string str_data = paramStr1.toStdString();
        const std::string str_key = appsecret.toStdString();
        unsigned char * mac = NULL;
        unsigned int mac_length = 0;
        HmacEncode("sha256", str_key.c_str(), str_key.length(), str_data.c_str(), str_data.length(), mac, mac_length);
        //qDebug()<<"HmacEncode "<<ret <<" lenth: "<<mac_length;
        QByteArray ba;
        ba.append((char *)mac, mac_length);
        sign1.append(ba.toBase64());
#endif
    }

    {//将待加密签名（sign1）及签名参数（不含 sign）的值，升序排序拼接为paranmStr2。
        paramStr2 = Sort(this->mAppId, this->mAppSecret, QString::number(timestamp), dataMD5, sign1);
    }

    {
#if 1
        //利用哈希散列算法 HashAlgorithm MD5 方式加密待加密字符串 paranmStr2，转字符串得到全大写签名即 sign。
        //计算md5值，传入需要被计算的数据，传入计算的类型
        sign = QCryptographicHash::hash(paramStr2.toLatin1(), QCryptographicHash::Md5).toHex().toUpper();
#else
        std::string encodedStr;
        std::string encodedHexStr;
        Hashmd5(paramStr2.toStdString(), encodedStr, encodedHexStr);
        sign = QString::fromStdString(encodedHexStr).toUpper();
#endif
    }
    QString iv = "";
    for (int i = 1; i < sign.length(); i = i + 2) {iv += sign.mid(i, 1);}

    QString data = QString("{\"data\":\"%1\"}").arg(JsonCrypt(this->mAppSecret, iv, idCard, name));
#if 0
    qDebug()<<"iv "<<iv;
    qDebug()<<"paramStr1 "<<paramStr1;
    qDebug()<<"sign1 "<<sign1;
    qDebug()<<"paramStr2 "<<paramStr2;
    qDebug()<<"sign "<<sign;
    qDebug()<<"DataMD5 "<<dataMD5;
    qDebug()<<"postData "<<data;
#endif
    return data;
}

bool ZhangjiakouHealthCodePrivate::DealZhangjiakouHealthCode(const QString &name, const QString &idCard, const QString &json)
{//{\"status\":0,\"errorMsg\":\"\",\"result\":{\"codeLevel\":0,\"message\":\"绿码\",\"warning\":false,\"tempWarn\":true,\"warningTemp\":37.3}}
    //qDebug()<<json;
    QJsonParseError err_rpt;
    QJsonDocument  root_Doc = QJsonDocument::fromJson(json.toUtf8(), &err_rpt);
    if(err_rpt.error != QJsonParseError::NoError)
    {
        emit q_func()->sigHealthCodeInfo(2, name, idCard, -1, 37.3, QObject::tr("ServerException"));//服务器异常
    }else
    {
        QJsonObject root_Obj = root_Doc.object();
        int status = root_Obj.value("status").toInt();
        QString errorMsg = root_Obj.value("errorMsg").toString();
        QJsonValue result = root_Obj.value("result");
        int codeLevel =  -2;
        QString message;
        int warning =  0;
        int tempWarn =  0;
        float warningTemp =   0.0;
        if(result.isObject())
        {
            auto resultObj =  result.toObject();
            codeLevel =  resultObj.value("codeLevel").toInt();
            message =  resultObj.value("message").toString();
            warning =  resultObj.value("warning").toInt();
            tempWarn =  resultObj.value("tempWarn").toInt();
            warningTemp =   resultObj.value("warningTemp").toDouble();
#ifdef Q_OS_LINUX
            switch(codeLevel)
            {
            case 0:
                YNH_LJX::Audio::Audio_PlayCustomerPcm("zh", "green_code.wav", true);
                break;
            case 1:
            case 2:
                YNH_LJX::Audio::Audio_PlayCustomerPcm("zh", "red_code.wav", true);
                break;
            }
#endif
        }
        //-2 健康码调用失败，请重试、-1 未申请/创建健康码请申请、0 绿、 1 黄、2 红
        if(status == 0)
            emit q_func()->sigHealthCodeInfo(2, name, idCard, codeLevel, warningTemp, message);
        else emit q_func()->sigHealthCodeInfo(2, name, idCard, codeLevel, 37.3, errorMsg);
    }

    return true;
}

void ZhangjiakouHealthCodePrivate::postTempRecognition(const QString &json)
{
    int timestamp;
    QString sign;
    QString data = this->HeaderPackage(json, sign, timestamp);
    //qDebug()<<data;
    QString rstr =  QString::fromStdString(PosQueryingHealthCodes(this->mRecognitionUrl.toStdString(), QString("x-appid:%1").arg(this->mAppId).toStdString(), QString("x-timestamp:%1").arg(timestamp).toStdString(), QString("x-sign:%1").arg(sign).toStdString(), data.toStdString()));
    if(rstr.isEmpty())
    {
        LogV("%s %s[%d] 上传温度记录至张家口失败\n", __FILE__, __FUNCTION__);
        emit q_func()->sigHealthCodeInfo(2, QString(), QString(), -1, 37.3, QObject::tr("NetworkException"));//网络异常
    }else
    {
        qDebug()<<"张家口温度记录上传 "<<rstr;
#ifdef Q_OS_LINUX
        system("rm -rf /isc/TempPersonBlast.jpeg");
#endif
    }
}

void ZhangjiakouHealthCode::slotQueryHealthCode(const QString name, const QString idCard, const QString sex)
{
    Q_UNUSED(sex);
    Q_D(ZhangjiakouHealthCode);
    int timestamp;
    QString sign;
    QString data = d->HeaderPackage(name,idCard, sign, timestamp);
    QString rstr =  QString::fromStdString(PosQueryingHealthCodes(d->mHealthCodeUrl.toStdString(), QString("x-appid:%1").arg(d->mAppId).toStdString(), QString("x-timestamp:%1").arg(timestamp).toStdString(), QString("x-sign:%1").arg(sign).toStdString(), data.toStdString()));
    if(rstr.isEmpty())
    {
        emit sigHealthCodeInfo(2, QString(), QString(), -1, 37.3, QObject::tr("NetworkException"));//网络异常
        LogV("%s %s[%d] 获取张家口健康码平台失败\n", __FILE__, __FUNCTION__);
    }else
    {
        d->DealZhangjiakouHealthCode(name, idCard, rstr);
    }
}

void ZhangjiakouHealthCode::slotPostTempRecognition(const QString path, const QString personName, const QString idCard, const QString sex, const int deviceType, const int checkType, const int codeLevel, const float temperature, const float translate, const bool warning)
{
    Q_D(ZhangjiakouHealthCode);
    //    QPixmap image(path);
    //    QByteArray ba;
    //    QBuffer buf(&ba);
    //    image.save(&buf, "png");

    QString base64 = QString::fromStdString(fileToBase64StringFace(path.toLatin1().data()));
    QJsonObject obj;
    obj.insert("image", base64);
    obj.insert("personName", personName);
    obj.insert("idCard", idCard);
    obj.insert("sex", sex);
    obj.insert("createdTime", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    obj.insert("deviceType", deviceType);
    obj.insert("deviceId", d->mDeviceId);
    obj.insert("deviceName", d->mDeviceName);
    obj.insert("areaId", d->mAreaId);
    obj.insert("areaName", d->mAreaName);
    obj.insert("checkType", checkType);
    obj.insert("codeLevel", codeLevel);
    obj.insert("temperature", temperature);
    obj.insert("translate", translate);
    obj.insert("warning", warning);

    QJsonDocument doc(obj);
    d->postTempRecognition(QString().append(doc.toJson()));
}
