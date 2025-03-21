#include "ZJKTest.h"
#include "AES/aesencrypt.h"
#include "MD5/md5.h"
#include "MD5/hashmd5.h"
#include "algo_hmac.h"

#include <QDateTime>

#ifdef Q_OS_LINUX
#include <curl/curl.h>
#include <curl/easy.h>
#endif
#include <QDebug>
#include <QMessageAuthenticationCode>
#include <QLibrary>

static QString JsonCrypt(const QString &AppSecret, const QString &iv, const QString &idcard, const QString &name)
{
    QByteArray inputStr = QByteArray().append(QString("{\"idCard\":\"%1\",\"name\":\"%2\"}").arg(idcard).arg(name));
    std::string str = AesEncrypt::AesCbcEncrypt(inputStr.toStdString(),
                                                AppSecret.toStdString(),
                                                iv.toStdString(),
                                                AesEncrypt::PKCS7Padding);
    return QString::fromStdString(str);
}

static QString DataToMD5(const QString &idcard, const QString &name)
{
    QString data= QString("{\"idCard\":\"%1\",\"name\":\"%2\"}").arg(idcard).arg(name);
#if 1
    std::string m =  md5(data.toStdString());
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

static inline std::string PosDKIdCardCode(std::string url, std::string appid, std::string time, std::string sign, std::string requestData)
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
    // printf("%s %s [%d] requestUrl=%s \n",__FILE__,__FUNCTION__,__LINE__,requestData.c_str());
    if (curl)
    {
#if 1
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
        printf("%s %d %s", __FUNCTION__, __LINE__, requestData.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestData.c_str());
        curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION, *sslctx_function);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
        //		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        res = curl_easy_perform(curl);
#endif
    }
    if (res != CURLE_OK)
    {
        printf("%s %s[%d] curl_easy_perform() failed: %s\n", __FILE__, __FUNCTION__, __LINE__, curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return "";
    }
    curl_easy_cleanup(curl);
    return ResString;
#else
    return std::string("{\"status\":0,\"errorMsg\":\"\",\"result\":{\"codeLevel\":0,\"message\":\"绿码\",\"warning\":false,\"tempWarn\":true,\"warningTemp\":37.3}}");
#endif
}

static QString Sort(const QString &srcappid, const QString &srcappsecret, const QString &srctimestamp, const QString &srcdata, const QString &srcsign1)
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

ZJKTest::ZJKTest(QObject *parent) : QObject(parent)
{
    QString appid = QString("a4185be6ef3c4080924276de9280505a");
    QString appsecret = QString("266f776FeEA54a52a960F66D7Ca9a181");
    int timestamp = QDateTime::currentDateTime().toTime_t();
    QString dataMD5 =  DataToMD5("440982198911235890", "林坚雄");
    QString paramStr1;
    QString paramStr2;
    QString sign1;
    QString sign;

    {
        //将参与签名的参数(appid、timestamp 必须参与签名)升序排序，并使用 url 键值
        //对（appId=valueA&key1 =valueB& … key2=valued&timestamp=valueC）格式
        //拼接为 paramStr1。
        paramStr1 = QString("appid=%1&appsecret=%2&data=%3&timestamp=%4").arg(appid).arg(appsecret).arg(dataMD5).arg(timestamp);
    }
    {
#if 1
        //利用 HmacSHA256 算法，appsecret 做为私钥，将 paramStr1 进行 HmacSHA256加密处理转 base64 得到待加密签名，
        //即 sign1 = HmacSHA256(appsecret, paramStr1)
        sign1 = QMessageAuthenticationCode::hash(paramStr1.toLatin1(), appsecret.toLatin1(), QCryptographicHash::Sha256).toBase64();
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
        paramStr2 = Sort(appid, appsecret, QString::number(timestamp), dataMD5, sign1);
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

    QString data = QString("{\"data\":\"%1\"}").arg(JsonCrypt(appsecret, iv, "440982198911235890", "林坚雄"));

    qDebug()<<"iv "<<iv;
    qDebug()<<"paramStr1 "<<paramStr1;
    qDebug()<<"sign1 "<<sign1;
    qDebug()<<"paramStr2 "<<paramStr2;
    qDebug()<<"sign "<<sign;
    qDebug()<<"DataMD5 "<<dataMD5;
    qDebug()<<"postData "<<data;
    std::string rstr =    PosDKIdCardCode("http://60.8.117.87:8091/open-api/v1/rabbit-person/health-code", QString("x-appid:%1").arg(appid).toStdString(), QString("x-timestamp:%1").arg(timestamp).toStdString(), QString("x-sign:%1").arg(sign).toStdString(), data.toStdString());
    qDebug()<<rstr.c_str();
}
