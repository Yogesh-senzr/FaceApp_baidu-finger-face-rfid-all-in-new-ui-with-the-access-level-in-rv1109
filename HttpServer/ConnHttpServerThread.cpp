#include "ConnHttpServerThread.h"
#include "PostPersonRecordThread.h"
#include "Version.h"
#include "USB/UsbObserver.h"
#include "Application/FaceApp.h"
#include "Config/ReadConfig.h"
#include "PCIcore/Watchdog.h"
#include "MessageHandler/Log.h"
#include "SharedInclude/GlobalDef.h"
#include "Helper/myhelper.h"
#include "PCIcore/RkUtils.h"
#include "PCIcore/Utils_Door.h"
#include "DB/RegisteredFacesDB.h"
#include "ManageEngines/PersonRecordToDB.h"
#include "RKCamera/Camera/cameramanager.h"
#include "base64.hpp"
#include "BaiduFace/BaiduFaceManager.h"
#include "RkNetWork/NetworkControlThread.h"
#include "json-cpp/json.h"

#include "PCIcore/GPIO.h"

#include <unistd.h>
#include <errno.h>
#include <sys/select.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <arpa/inet.h>
#include <openssl/md5.h>
#include <curl/curl.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <netserver.h>
#include <dbserver.h>
#include <QtCore/QTextStream>
#include <QtCore/QDir>
#include <QtCore/QFileInfoList>



#define ISC_NULL                 0L

#define MERR_ASF_FACEENGINE_BASE							0x30000
#define MERR_ASF_FACEENGINE_IMAGE							(MERR_ASF_FACEENGINE_BASE + 1)
#define MERR_ASF_FACEENGINE_FACEDETECT						(MERR_ASF_FACEENGINE_BASE + 2)
#define MERR_ASF_FACEENGINE_MULTIFACE						(MERR_ASF_FACEENGINE_BASE + 3)
#define MERR_ASF_FACEENGINE_FACEEXTRACT						(MERR_ASF_FACEENGINE_BASE + 4)
#define MERR_ASF_FACEENGINE_THUMBNAIL						(MERR_ASF_FACEENGINE_BASE + 5)
#define MERR_ASF_FACEENGINE_FACEQUALITY						(MERR_ASF_FACEENGINE_BASE + 6)

typedef struct _MESSAGE_HEADER_S
{
	std::string msg_type;
	std::string sn;
	std::string timestamp;
	std::string password;
	std::string cmd_id;
} MESSAGE_HEADER_S;

class ConnHttpServerThreadPrivate
{
	Q_DECLARE_PUBLIC(ConnHttpServerThread)
public:
	ConnHttpServerThreadPrivate(ConnHttpServerThread *dd);
	bool doDownloadFile(QString url, QString dist);
	QString sn;
	QString mHttpServerUrl;
	QString mHttpServerPassword;

private:
	QString mac;
	QString msg;
	mutable QMutex sync;
	QWaitCondition pauseCond;
	int threadDelay;

private:
	QString doPostJson(Json::Value json);
	void doHeartbeat();
	bool doNextMessage();

	void doAddPerson(Json::Value root, MESSAGE_HEADER_S httpMsgHeader);
	void doAddPersons(Json::Value root, MESSAGE_HEADER_S httpMsgHeader);
	void doAddPersonswithreason(Json::Value root, MESSAGE_HEADER_S httpMsgHeader);
	void doDeletePerson(Json::Value root, MESSAGE_HEADER_S httpMsgHeader);
	void doDeleteAllPerson(Json::Value root, MESSAGE_HEADER_S httpMsgHeader);
	void doUpdatePerson(Json::Value root, MESSAGE_HEADER_S httpMsgHeader);
	void doFindPerson(Json::Value root, MESSAGE_HEADER_S httpMsgHeader);
	void doGetAllPerson(Json::Value root, MESSAGE_HEADER_S httpMsgHeader);
	void doSetTimeOfAccess(Json::Value root, MESSAGE_HEADER_S httpMsgHeader);
	void doDeleteTimeOfAccess(Json::Value root, MESSAGE_HEADER_S httpMsgHeader);
	void doDeleteAllPersonRecord(Json::Value root, MESSAGE_HEADER_S httpMsgHeader);
	void doGetPersonRecord(Json::Value root, MESSAGE_HEADER_S httpMsgHeader);
	void doUpdatePersonRecordUploadFlag(Json::Value root, MESSAGE_HEADER_S httpMsgHeader);
	void doDeletePersonRecord(Json::Value root, MESSAGE_HEADER_S httpMsgHeader);
	void doOpenDoor(Json::Value root, MESSAGE_HEADER_S httpMsgHeader);
	void doReboot(Json::Value root, MESSAGE_HEADER_S httpMsgHeader);
	void doSetNetwork(Json::Value root, MESSAGE_HEADER_S httpMsgHeader);
	void doGetDeviceConfig(Json::Value root, MESSAGE_HEADER_S httpMsgHeader);
	void doSetDeviceConfig(Json::Value root, MESSAGE_HEADER_S httpMsgHeader);
	void doResetFactorySetting(Json::Value root, MESSAGE_HEADER_S httpMsgHeader);
	void doSetAlgoParam(Json::Value root, MESSAGE_HEADER_S httpMsgHeader);
	void doGetDeviceVersion(Json::Value root, MESSAGE_HEADER_S httpMsgHeader);
	void doTakePicture(Json::Value root, MESSAGE_HEADER_S httpMsgHeader);
	void doSystemUpdate(Json::Value root, MESSAGE_HEADER_S httpMsgHeader);
	void doSetStaticLanIP(Json::Value root, MESSAGE_HEADER_S httpMsgHeader);
	void doSetDeviceHttpPassword(Json::Value root, MESSAGE_HEADER_S httpMsgHeader);
	void doSyncPersonsList(Json::Value root, MESSAGE_HEADER_S httpMsgHeader);
	void doSaveFile(Json::Value root, MESSAGE_HEADER_S httpMsgHeader);

private:
	ConnHttpServerThread * const q_ptr;
};

static bool checkFileMd5Sum(std::string srcFile, std::string strDstMd5)
{
	std::string strSrcMd5;
	std::string strSrcMd5File = srcFile + "_md5";
	std::string cmd = "md5sum " + srcFile + " > " + strSrcMd5File;
	YNH_LJX::RkUtils::Utils_ExecCmd(cmd.c_str());

	std::ifstream in0(strSrcMd5File);
	if (in0)
	{
		getline(in0, strSrcMd5);
		in0.close();
	}
	unlink(strSrcMd5File.c_str());
	if (!strncasecmp(strDstMd5.c_str(), strSrcMd5.c_str(), 32))
	{
		LogD("%s %s[%d] match \n", __FILE__, __FUNCTION__, __LINE__);
		return true;
	}

	LogE("%s %s[%d] %s not match %s \n", __FILE__, __FUNCTION__, __LINE__, strDstMd5.c_str(), strSrcMd5.c_str());
	return false;
}

static std::string fileToBase64String(std::string strFilePath)
{
	std::string base_64;
	if (strFilePath.length() > 3)
	{
		std::string cmd = "/bin/chmod 777 " + strFilePath;
		YNH_LJX::RkUtils::Utils_ExecCmd(cmd.c_str());
	}

	struct stat _stat;
	stat(strFilePath.c_str(), &_stat);

	if (_stat.st_size > 0)
	{
		FILE *pFile = fopen(strFilePath.c_str(), "rb");
		if (pFile != ISC_NULL)
		{
			char *pTmpBuf = (char*) malloc(_stat.st_size);
			if (pTmpBuf != ISC_NULL)
			{
				long nTotalNum = 0;
				while (true)
				{
					int ret = fread(pTmpBuf + nTotalNum, 1, _stat.st_size, pFile);
					if (ret<=0)
					{
						break;
					}
					
					nTotalNum += ret;
					if (nTotalNum >= _stat.st_size)
					{
						break;
					}
				}
				base_64 = cereal::base64::encode((unsigned char const*) pTmpBuf, (size_t) _stat.st_size);
				free(pTmpBuf);
			}
			fclose(pFile);
		}
	}
	return base_64;
}

static std::vector<std::string> split(const std::string& s, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	while (std::getline(tokenStream, token, delimiter))
	{
		tokens.push_back(token);
	}
	return tokens;
}

static std::string& trim(std::string &s)
{
	if (s.empty())
	{
		return s;
	}

	s.erase(0, s.find_first_not_of(" "));
	s.erase(s.find_last_not_of(" ") + 1);
	return s;
}

static std::string getTime()
{
	time_t timep;
	time(&timep);
	char tmp[64];
	strftime(tmp, sizeof(tmp), "%Y%m%d%H%M%S", localtime(&timep));
	return tmp;
}

static std::string getTime2(std::string time)
{
	std::string newTime;
	if (time.length() > 0)
	{
		int year = 0, month = 0, day = 0, hour = 0, minues = 0, second = 0;
		char newStr[128] = { 0 };
		sscanf(time.c_str(), "%04d/%02d/%02d %02d:%02d", &year, &month, &day, &hour, &minues);
		sprintf(newStr, "%04d%02d%02d%02d%02d%02d", year, month, day, hour, minues, second);

		newTime = std::string(newStr, 0, strlen(newStr));
	}
	return newTime;
}

static std::string getTime3()
{
	time_t timep;
	time(&timep);
	char tmp[64];
	strftime(tmp, sizeof(tmp), "%Y/%m/%d %H:%M:%S", localtime(&timep));
	return tmp;
}

static std::string md5sum(std::string src)
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

static size_t download_write_data(void *ptr, size_t size, size_t nmemb, void* userdata)
{	
	int fd = (int) userdata;
	if (fd <= 0)
	{
		LogE("%s %s[%d] fd :%d\n", __FILE__, __FUNCTION__, __LINE__, fd);
		return 0;
	}
	int written = write(fd, ptr, size * nmemb);
	return written;
}

static int write_data(void *buffer, size_t sz, size_t nmemb, void *ResInfo)
{
	std::string* psResponse = (std::string*) ResInfo; //强制转换
	psResponse->append((char*) buffer, sz * nmemb); //sz*nmemb表示接受数据的多少
	return sz * nmemb;  //返回接受数据的多少
}

static char *mypem = ISC_NULL;
static int pemsize = 0;
static CURLcode sslctx_function(CURL *curl, void *sslctx, void *parm)
{
	CURLcode rv = CURLE_ABORTED_BY_CALLBACK;
	if (mypem == ISC_NULL)
	{
		pemsize = YNH_LJX::RkUtils::Utils_getFileSize("/isc/cacert.pem");
		mypem = (char*) malloc(pemsize);
		if (mypem == ISC_NULL)
		{
			return rv;
		}
		int fd = open("/isc/cacert.pem", O_RDONLY, 0666);
		if (fd <= 0)
		{
			return rv;
		}
		read(fd, (void*) mypem, pemsize);
		close(fd);
	}

	BIO *cbio = BIO_new_mem_buf(mypem, pemsize);
	X509_STORE *cts = SSL_CTX_get_cert_store((SSL_CTX *) sslctx);
	int i;
	STACK_OF(X509_INFO) * inf;
	(void) curl;
	(void) parm;

	if (!cts || !cbio)
	{
		return rv;
	}

	inf = PEM_X509_INFO_read_bio(cbio, ISC_NULL, ISC_NULL, ISC_NULL);

	if (!inf)
	{
		BIO_free(cbio);
		return rv;
	}

	for (i = 0; i < sk_X509_INFO_num(inf); i++)
	{
		X509_INFO *itmp = sk_X509_INFO_value(inf, i);
		if (itmp->x509)
		{
			X509_STORE_add_cert(cts, itmp->x509);
		}
		if (itmp->crl)
		{
			X509_STORE_add_crl(cts, itmp->crl);
		}
	}

	sk_X509_INFO_pop_free(inf, X509_INFO_free);
	BIO_free(cbio);
 
	rv = CURLE_OK;
	return rv;
}

bool ConnHttpServerThreadPrivate::doDownloadFile(QString url, QString dist)
{
	if (url.size() > 0 && dist.size() > 0)
	{
		unlink(dist.toStdString().c_str());

		url += "?sn=" + sn;
		int fd = open(dist.toStdString().c_str(), O_CREAT | O_RDWR, 0666);
		LogD("%s %s[%d] url %s dist %s fd :%d\n", __FILE__, __FUNCTION__, __LINE__, url.toStdString().c_str(), dist.toStdString().c_str(),
				fd);
		CURL *curl;
		CURLcode res;
		curl = curl_easy_init();
#if 0		
		if (curl)
		{
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
			curl_easy_setopt(curl, CURLOPT_URL, url.toStdString().c_str());
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
			curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
			curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, download_write_data); //数据请求到以后的回调函数
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, fd);
			if (mHttpServerUrl.startsWith("https://"))
			{
				curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION, *sslctx_function);
				curl_easy_setopt(curl, CURLOPT_TIMEOUT, 6);
			}
			res = curl_easy_perform(curl);
		}
#endif 		
#if 0		
		if (curl)
		{
  			char errbuf[CURL_ERROR_SIZE];			
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
			curl_easy_setopt(curl, CURLOPT_URL, url.toStdString().c_str());
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
			curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
			curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, download_write_data); //数据请求到以后的回调函数
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, fd);
			curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
			curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
			
			/* set the error buffer as empty before performing a request */
			errbuf[0] = 0;

			if (mHttpServerUrl.startsWith("https://"))
			{
				curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION, *sslctx_function);
				curl_easy_setopt(curl, CURLOPT_TIMEOUT, 6);
			}
			res = curl_easy_perform(curl);
			if(res != CURLE_OK) {
				size_t len = strlen(errbuf);
				fprintf(stderr, "\n%s,%s,%d,libcurl: (%d) ",__FILE__,__func__,__LINE__, res);
				if(len)
				fprintf(stderr, "%s%s", errbuf,
						((errbuf[len - 1] != '\n') ? "\n" : ""));
				else
				fprintf(stderr, "%s\n", curl_easy_strerror(res));
			}	
			else {
			printf(">>>>>>%s,%s,%d,res=%d \n",__FILE__,__func__,__LINE__,res);	
			}	
			printf(">>>>>>%s,%s,%d,res=%d \n",__FILE__,__func__,__LINE__,res);				
		}
#endif 	
#if 1 //ok 
	std::string cmd = "/usr/bin/curl --cacert /isc/cacert.pem ";
    cmd += "  ";
    cmd += url.toStdString().c_str();
    cmd += "  -o  ";
	cmd += dist.toStdString().c_str();
    std::string rets = "";    

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
                rets += std::string(buf, 0, readSize);
            }
        } while (readSize > 0);
        pclose(pFile);
        //qDebug()<<"buf : "<<buf;   
        if (rets.length()>10 )
        {
          // resultString = rets;
        }
    }
#endif 
		long res_code = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res_code);
		LogD("%s %s[%d] res_code %d \n", __FILE__, __FUNCTION__, __LINE__, res_code);
		curl_easy_cleanup(curl);
		if (CURLE_OK != res)
		{
			LogD("%s %s[%d] res %d %s \n", __FILE__, __FUNCTION__, __LINE__, res, curl_easy_strerror(res));
		}

		if (fd > 0)
		{
			close(fd);
		}
		int nFileSize = YNH_LJX::RkUtils::Utils_getFileSize(dist.toStdString().c_str());
		LogD("%s %s[%d] dist %s size :%d\n", __FILE__, __FUNCTION__, __LINE__, dist.toStdString().c_str(), nFileSize);
		return (nFileSize > 0);
	} else
	{
		LogE("%s %s[%d] param error ,%s %s \n", __FILE__, __FUNCTION__, __LINE__, url.toStdString().c_str(), dist.toStdString().c_str());
	}
	return false;
}

ConnHttpServerThreadPrivate::ConnHttpServerThreadPrivate(ConnHttpServerThread *dd) :
		q_ptr(dd), //
		threadDelay(30) //
{
	QFile file("/param/mac.txt");
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QTextStream in(&file);
		mac = in.readLine();
	}
	file.close();
}

QString ConnHttpServerThreadPrivate::doPostJson(Json::Value json)
{
	std::string jsonStr = "";
	std::string resultString = "";
	qDebug() << "POST_JSON_DEBUG: Starting doPostJson function";
	if(mHttpServerUrl.size() < 8)
	{
		qDebug() << "POST_JSON_DEBUG: URL too short: " << mHttpServerUrl;
		return "";
	}
    if (json.size() > 0)
    {
        Json::FastWriter fast_writer;
        jsonStr = fast_writer.write(json);
        jsonStr = jsonStr.substr(0, jsonStr.length() - 1);
        qDebug() << "FULL_JSON_DEBUG: Complete JSON payload:";
		qDebug() << QString::fromStdString(jsonStr);
    }
    
    qDebug() << "POST_JSON_DEBUG: Constructing curl command...";
#if 0	
	CURL *curl;
	CURLcode res;
	curl = curl_easy_init();
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
		curl_easy_setopt(curl, CURLOPT_URL, mHttpServerUrl.toStdString().c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 1);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
		struct curl_slist *headers = ISC_NULL;
		headers = curl_slist_append(headers, "Content-Type: application/json;charset=utf-8");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data); //数据请求到以后的回调函数
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resultString);
		if (mHttpServerUrl.startsWith("https://"))
		{
			curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION, *sslctx_function);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, 6);
		}
		res = curl_easy_perform(curl);
		printf(">>>>>>%s,%s,%d,res=%d \n",__FILE__,__func__,__LINE__,res);			
	}
	curl_easy_cleanup(curl);
#endif 
#if 1 //ok
	//std::string cmd = "/usr/bin/curl -H \"Content-Type:application/json;charset=UTF-8\" ";
    std::string cmd = "/usr/bin/curl --cacert /isc/cacert.pem -H \"Content-Type:application/json;charset=UTF-8\" ";
    cmd += "  ";
    cmd += mHttpServerUrl.toStdString().c_str();
    cmd += "  --connect-timeout 5 -v -X POST ";  // Added -v for verbose output
    cmd += "  -d '";
    cmd += jsonStr.c_str();
    cmd +="'";
    
	qDebug() << "CURL_DEBUG: Complete curl command:";
	qDebug() << QString::fromStdString(cmd);
	qDebug() << "POST_JSON_DEBUG: Full URL: " << mHttpServerUrl;
    
    std::string rets = "";    

    qDebug() << "POST_JSON_DEBUG: Executing curl command...";
    FILE *pFile = popen(cmd.c_str(), "r");
    if (pFile != ISC_NULL)
    {
        qDebug() << "POST_JSON_DEBUG: curl command executed, reading response...";
        char buf[4096] = { 0 };
        int readSize = 0;
        do
        {
            readSize = fread(buf, 1, sizeof(buf), pFile);
            if (readSize > 0)
            {
                rets += std::string(buf, 0, readSize);
                qDebug() << "POST_JSON_DEBUG: Read " << readSize << " bytes";
            }
        } while (readSize > 0);
        
        int pclose_result = pclose(pFile);
        qDebug() << "POST_JSON_DEBUG: pclose result: " << pclose_result;
        qDebug() << "POST_JSON_DEBUG: Response read, length: " << rets.length();
        
        if (rets.length() > 10)
        {
            resultString = rets;
            qDebug() << "POST_JSON_DEBUG: Valid response received (first 100 chars): " << QString::fromStdString(rets).left(100);
        }
        else {
            qDebug() << "POST_JSON_DEBUG: Response too short, may indicate error. Full response: " << QString::fromStdString(rets);
        }
    }
    else {
        qDebug() << "POST_JSON_DEBUG: Failed to execute curl command!";
    }
#endif 
#if 0	
	CURL *curl;
	CURLcode res;
	curl = curl_easy_init();
	if (curl)
	{
  		char errbuf[CURL_ERROR_SIZE];		
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
		curl_easy_setopt(curl, CURLOPT_URL, mHttpServerUrl.toStdString().c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 1);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
  		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
 
  		/* set the error buffer as empty before performing a request */
  		errbuf[0] = 0;

		struct curl_slist *headers = ISC_NULL;
		headers = curl_slist_append(headers, "Content-Type: application/json;charset=utf-8");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data); //数据请求到以后的回调函数
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resultString);
		if (mHttpServerUrl.startsWith("https://"))
		{
			curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION, *sslctx_function);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, 6);
		}
		res = curl_easy_perform(curl);
		if(res != CURLE_OK) {
			size_t len = strlen(errbuf);
			//fprintf(stderr, "\n%s,%s,%d,libcurl: (%d) ",__FILE__,__func__,__LINE__, res);
			if(len)
			fprintf(stderr, "%s%s", errbuf,	((errbuf[len - 1] != '\n') ? "\n" : ""));
			else
			fprintf(stderr, "%s\n", curl_easy_strerror(res));
		}		
	}
	LogD("%s %s[%d] mHttpServerUrl %s \n", __FILE__, __FUNCTION__, __LINE__, mHttpServerUrl.toStdString().c_str());
	curl_easy_cleanup(curl);
#endif 
	//if(resultString.size() > 3)
	{
		LogD("%s %s[%d] ret %s \n", __FILE__, __FUNCTION__, __LINE__, resultString.substr(0, resultString.size() > 128 ? 128 : resultString.size()).c_str());
	}
	std::string debug_cmd = cmd + " > /tmp/curl_debug.txt 2>&1";
    system(debug_cmd.c_str());
    qDebug() << "POST_JSON_DEBUG: Debug curl output saved to /tmp/curl_debug.txt";
    qDebug() << "POST_JSON_DEBUG: Function complete, returning string of length: " << resultString.length();
    
	return QString::fromStdString(resultString);
}

void ConnHttpServerThreadPrivate::doHeartbeat()
{
	Json::Value config;
	std::string timestamp;
	std::string password;

	if (sn.size() < 2)
	{
		return;
	}
	timestamp = getTime();
	password = mHttpServerPassword.toStdString() + timestamp;
	password = md5sum(password);
	password = md5sum(password);
	transform(password.begin(), password.end(), password.begin(), ::tolower);

	config["msg_type"] = "heartbeat";
	config["sn"] = sn.toStdString().c_str();
	config["timestamp"] = timestamp.c_str();
	config["password"] = password.c_str();
	config["device_version"] = ISC_VERSION;
	if (((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->getAlgoFaceInitState() == true)
	{
		config["active_info"] = "actived";
	}

	config["device_name"] = ReadConfig::GetInstance()->getHomeDisplay_DeviceName().toStdString().c_str();
	config["algo_name"] = "";

	if (((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->getAlgoFaceInitState() != true)
	{
		if (!access("/param/deviceinfo.txt", F_OK))
		{
			config["arcsoft_face_device_info"] = fileToBase64String("/param/deviceinfo.txt");

			if (mac.size() > 2)
			{
				config["arcsoft_face_mac"] = mac.toStdString().c_str();
			}
		}
	}
	
    QString ret = doPostJson(config);

    if (ret.size() > 0)
    {
        msg = ret;
        q_ptr->setLastHeartbeatResponse(ret); 
		q_ptr->checkAndSyncUsers(ret);
    }
}

void ConnHttpServerThreadPrivate::doAddPerson(Json::Value root, MESSAGE_HEADER_S stMsgHeader)
{	
	QString person_uuid = QString::fromStdString(root["person_uuid"].asString());
	QString card_no = QString::fromStdString(root["card_no"].asString());
	QString id_card_no = QString::fromStdString(root["id_card_no"].asString());
	QString person_name = QString::fromStdString(root["person_name"].asString());
	QString person_type = QString::fromStdString(root["person_type"].asString());
	QString group = QString::fromStdString(root["group"].asString());
	QString male = QString::fromStdString(root["male"].asString());
	QString face_img = QString::fromStdString(root["face_img"].asString());
	QString creat_time = QString::fromStdString(root["creat_time"].asString());
	QString auth_type = QString::fromStdString(root["auth_type"].asString());
	QString time_data = QString::fromStdString(root["data"].asString());
	QString time_range = QString::fromStdString(root["time_range"].asString());

	mkdir("/mnt/user/tmp/", 0777);
	if (doDownloadFile(face_img, "/mnt/user/facedb/RegImage.jpeg"))
	{
		if (person_uuid.length() <= 0 && person_name.length() <= 0)
		{
			return;
		}

		Json::Value json;
		std::string timestamp;
		std::string password;
		timestamp = getTime();
		password = mHttpServerPassword.toStdString() + timestamp;
		password = md5sum(password);
		password = md5sum(password);
		transform(password.begin(), password.end(), password.begin(), ::tolower);
		json["msg_type"] = stMsgHeader.msg_type.c_str();
		json["sn"] = stMsgHeader.sn.c_str();
		json["timestamp"] = timestamp.c_str();
		json["password"] = password.c_str();
		json["cmd_id"] = stMsgHeader.cmd_id.c_str();

		json["person_uuid"] = person_uuid.toStdString().c_str();
		json["result"] = "0";
		json["success"] = "0";

		int result = -1;
		int faceNum = 0;
		double threshold = 0;
		QByteArray faceFeature;
		result = ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->RegistPerson("/mnt/user/facedb/RegImage.jpeg", faceNum, threshold, faceFeature);
		LogD("%s %s[%d] RegistPerson result %d \n", __FILE__, __FUNCTION__, __LINE__, result);
		LogD("%s %s[%d] auth_type %s \n", __FILE__, __FUNCTION__, __LINE__, auth_type.toStdString().c_str());
		LogD("%s %s[%d] time_data %s \n", __FILE__, __FUNCTION__, __LINE__, time_data.toStdString().c_str());
		LogD("%s %s[%d] time_range %s \n", __FILE__, __FUNCTION__, __LINE__, time_range.toStdString().c_str());

		if (result == 0)
		{
			QString timeOfAccess = "";
			if (auth_type == "time_range")
			{
				timeOfAccess = time_range;
			} else if (auth_type == "week_cycle")
			{
				//data = 1,2,3,4,5,6,7
				//time_range = 00:00;21:00
				time_range.replace(QString(";"), QString(","));
				timeOfAccess = time_range;

				QStringList sections = time_data.split(",");
				for (int i = 1; i <= 7; i++)
				{
					bool isSet = false;
					timeOfAccess += ",";
					for (int j = 0; j < sections.size(); j++)
					{
						if (sections.at(j) == QString::number(i))
						{
							timeOfAccess += "1";
							isSet = true;
							break;
						}
					}
					if (isSet == false)
					{
						timeOfAccess += "0";
					}
				}
			}

			bool isSaveDBOk = RegisteredFacesDB::GetInstance()->RegPersonToDBAndRAM(person_uuid, person_name, id_card_no, card_no, male,
					group, timeOfAccess, faceFeature);
			LogD("%s %s[%d] RegPersonToDBAndRAM isSaveDBOk %d \n", __FILE__, __FUNCTION__, __LINE__, isSaveDBOk);

			result = isSaveDBOk ? ISC_OK : ISC_ERROR;
		}

		if (result == ISC_OK)
		{
			json["result"] = "1";
			json["success"] = "1";
			json["message"] = "person register success!";
		} else if (result == MERR_ASF_FACEENGINE_FACEDETECT)
		{
			json["result"] = "-1";
			json["success"] = "0";
			json["message"] = "picture do not have face";
		} else if (result == MERR_ASF_FACEENGINE_MULTIFACE)
		{
			json["result"] = "-2";
			json["success"] = "0";
			json["message"] = "picture have many face";
		} else if (result == MERR_ASF_FACEENGINE_FACEEXTRACT)
		{
			json["result"] = "-3";
			json["success"] = "0";
			json["message"] = "extract the feature failed!";
		} else if (result == ISC_ERROR_EXIST)
		{
			json["result"] = "-4";
			json["success"] = "0";
			json["message"] = "person already register";
		} else if (result == MERR_ASF_FACEENGINE_FACEQUALITY)
		{
			json["result"] = "-5";
			json["success"] = "0";
			json["message"] = "the picture is too low or the image is too bright or blurry";
		} else
		{
			json["result"] = "-6";
			json["success"] = "0";
			json["message"] = "register failed, unknown error, please try again";
		}

		QString ret = doPostJson(json);
		if (ret.length() > 0)
		{
			msg = ret;
		}
	}
}

void ConnHttpServerThreadPrivate::doAddPersons(Json::Value root, MESSAGE_HEADER_S stMsgHeader)
{
	Json::Value data = root["data"];
	QString okUUID = "";

	for (Json::Value::ArrayIndex i = 0; i != data.size(); i++)
	{
		QString person_uuid = QString::fromStdString(data[i]["person_uuid"].asString());
		QString card_no = QString::fromStdString(data[i]["card_no"].asString());
		QString id_card_no = QString::fromStdString(data[i]["id_card_no"].asString());
		QString person_name = QString::fromStdString(data[i]["person_name"].asString());
		QString person_type = QString::fromStdString(data[i]["person_type"].asString());
		QString group = QString::fromStdString(data[i]["group"].asString());
		QString male = QString::fromStdString(data[i]["male"].asString());
		QString face_img = QString::fromStdString(data[i]["face_img"].asString());
		QString creat_time = QString::fromStdString(data[i]["creat_time"].asString());
		QString auth_type = QString::fromStdString(data[i]["auth_type"].asString());
		QString time_data = QString::fromStdString(data[i]["data"].asString());
		QString time_range = QString::fromStdString(data[i]["time_range"].asString());

		LogD("%s %s[%d] ============================ \n", __FILE__, __FUNCTION__, __LINE__);
		LogD("%s %s[%d] person_uuid %s \n", __FILE__, __FUNCTION__, __LINE__, person_uuid.toStdString().c_str());
		LogD("%s %s[%d] card_no %s \n", __FILE__, __FUNCTION__, __LINE__, card_no.toStdString().c_str());
		LogD("%s %s[%d] id_card_no %s \n", __FILE__, __FUNCTION__, __LINE__, id_card_no.toStdString().c_str());
		LogD("%s %s[%d] person_name %s \n", __FILE__, __FUNCTION__, __LINE__, person_name.toStdString().c_str());
		LogD("%s %s[%d] person_type %s \n", __FILE__, __FUNCTION__, __LINE__, person_type.toStdString().c_str());
		LogD("%s %s[%d] group %s \n", __FILE__, __FUNCTION__, __LINE__, group.toStdString().c_str());
		LogD("%s %s[%d] male %s \n", __FILE__, __FUNCTION__, __LINE__, male.toStdString().c_str());
		LogD("%s %s[%d] face_img %s \n", __FILE__, __FUNCTION__, __LINE__, face_img.toStdString().c_str());
		LogD("%s %s[%d] creat_time %s \n", __FILE__, __FUNCTION__, __LINE__, creat_time.toStdString().c_str());
		LogD("%s %s[%d] auth_type %s \n", __FILE__, __FUNCTION__, __LINE__, auth_type.toStdString().c_str());
		LogD("%s %s[%d] time_data %s \n", __FILE__, __FUNCTION__, __LINE__, time_data.toStdString().c_str());
		LogD("%s %s[%d] time_range %s \n", __FILE__, __FUNCTION__, __LINE__, time_range.toStdString().c_str());

		mkdir("/mnt/user/tmp/", 0777);
		LogD("%s %s[%d] ============================ \n", __FILE__, __FUNCTION__, __LINE__);		
		if (doDownloadFile(face_img, "/mnt/user/facedb/RegImage.jpeg"))
		{
		LogD("%s %s[%d] ============================ \n", __FILE__, __FUNCTION__, __LINE__);			
			if (person_uuid.length() > 0 && person_name.length() > 0)
			{
		LogD("%s %s[%d] ============================ \n", __FILE__, __FUNCTION__, __LINE__);				
				int result = -1;
				int faceNum = 0;
				double threshold = 0;
				QByteArray faceFeature;
				result = ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->RegistPerson("/mnt/user/facedb/RegImage.jpeg", faceNum, threshold, faceFeature);
				LogD("%s %s[%d] RegistPerson result %d \n", __FILE__, __FUNCTION__, __LINE__, result);
				LogD("%s %s[%d] auth_type %s \n", __FILE__, __FUNCTION__, __LINE__, auth_type.toStdString().c_str());
				LogD("%s %s[%d] time_data %s \n", __FILE__, __FUNCTION__, __LINE__, time_data.toStdString().c_str());
				LogD("%s %s[%d] time_range %s \n", __FILE__, __FUNCTION__, __LINE__, time_range.toStdString().c_str());

				if (result == 0)
				{
					QString timeOfAccess = "";
					if (auth_type == "time_range")
					{
						timeOfAccess = time_range;
					} else if (auth_type == "week_cycle")
					{
						//data = 1,2,3,4,5,6,7
						//time_range = 00:00;21:00
						time_range.replace(QString(";"), QString(","));
						timeOfAccess = time_range;

						QStringList sections = time_data.split(",");
						for (int i = 1; i <= 7; i++)
						{
							bool isSet = false;
							timeOfAccess += ",";
							for (int j = 0; j < sections.size(); j++)
							{
								if (sections.at(j) == QString::number(i))
								{
									timeOfAccess += "1";
									isSet = true;
									break;
								}
							}
							if (isSet == false)
							{
								timeOfAccess += "0";
							}
						}
					}

					bool isSaveDBOk = RegisteredFacesDB::GetInstance()->RegPersonToDBAndRAM(person_uuid, person_name, id_card_no, card_no,
							male, group, timeOfAccess, faceFeature);
					LogD("%s %s[%d] RegPersonToDBAndRAM isSaveDBOk %d \n", __FILE__, __FUNCTION__, __LINE__, isSaveDBOk);

					if (isSaveDBOk)
					{
						okUUID += person_uuid+ ",";
					}
				} 
			}
		}
	}

	Json::Value json;
	std::string timestamp;
	std::string password;
	timestamp = getTime();
	password = mHttpServerPassword.toStdString() + timestamp;
	password = md5sum(password);
	password = md5sum(password);
	transform(password.begin(), password.end(), password.begin(), ::tolower);
	json["msg_type"] = stMsgHeader.msg_type.c_str();
	json["sn"] = stMsgHeader.sn.c_str();
	json["timestamp"] = timestamp.c_str();
	json["password"] = password.c_str();
	json["cmd_id"] = stMsgHeader.cmd_id.c_str();

	if (okUUID.length() > 1)
	{
		json["person_uuid"] = okUUID.toStdString().c_str();
		json["result"] = "1";
		json["success"] = "1";
	} else
	{
		json["person_uuid"] = "";
		json["result"] = "0";
		json["success"] = "0";
	}

	QString ret = doPostJson(json);
	if (ret.length() > 0)
	{
		msg = ret;
	}
}

#if 0
 uuid,必填,否则不知对应哪一记录
#endif 

void ConnHttpServerThreadPrivate::doAddPersonswithreason(Json::Value root, MESSAGE_HEADER_S stMsgHeader)
{
	Json::Value data = root["data"];
	QString okUUID = "";
//	QString failUUID = "";
	QString failReason = "";


	Json::Value faildata;	

	for (Json::Value::ArrayIndex i = 0; i != data.size(); i++)
	{
		int result = -1000;

		QString person_uuid = QString::fromStdString(data[i]["person_uuid"].asString());
		QString card_no = QString::fromStdString(data[i]["card_no"].asString());
		QString id_card_no = QString::fromStdString(data[i]["id_card_no"].asString());
		QString person_name = QString::fromStdString(data[i]["person_name"].asString());
		QString person_type = QString::fromStdString(data[i]["person_type"].asString());
		QString group = QString::fromStdString(data[i]["group"].asString());
		QString male = QString::fromStdString(data[i]["male"].asString());
		QString face_img = QString::fromStdString(data[i]["face_img"].asString());
		QString creat_time = QString::fromStdString(data[i]["creat_time"].asString());
		QString auth_type = QString::fromStdString(data[i]["auth_type"].asString());
		QString time_data = QString::fromStdString(data[i]["data"].asString());
		QString time_range = QString::fromStdString(data[i]["time_range"].asString());

		LogD("%s %s[%d] ============================ \n", __FILE__, __FUNCTION__, __LINE__);
		LogD("%s %s[%d] person_uuid %s \n", __FILE__, __FUNCTION__, __LINE__, person_uuid.toStdString().c_str());
		LogD("%s %s[%d] card_no %s \n", __FILE__, __FUNCTION__, __LINE__, card_no.toStdString().c_str());
		LogD("%s %s[%d] id_card_no %s \n", __FILE__, __FUNCTION__, __LINE__, id_card_no.toStdString().c_str());
		LogD("%s %s[%d] person_name %s \n", __FILE__, __FUNCTION__, __LINE__, person_name.toStdString().c_str());
		LogD("%s %s[%d] person_type %s \n", __FILE__, __FUNCTION__, __LINE__, person_type.toStdString().c_str());
		LogD("%s %s[%d] group %s \n", __FILE__, __FUNCTION__, __LINE__, group.toStdString().c_str());
		LogD("%s %s[%d] male %s \n", __FILE__, __FUNCTION__, __LINE__, male.toStdString().c_str());
		LogD("%s %s[%d] face_img %s \n", __FILE__, __FUNCTION__, __LINE__, face_img.toStdString().c_str());
		LogD("%s %s[%d] creat_time %s \n", __FILE__, __FUNCTION__, __LINE__, creat_time.toStdString().c_str());
		LogD("%s %s[%d] auth_type %s \n", __FILE__, __FUNCTION__, __LINE__, auth_type.toStdString().c_str());
		LogD("%s %s[%d] time_data %s \n", __FILE__, __FUNCTION__, __LINE__, time_data.toStdString().c_str());
		LogD("%s %s[%d] time_range %s \n", __FILE__, __FUNCTION__, __LINE__, time_range.toStdString().c_str());

		mkdir("/mnt/user/tmp/", 0777);
		LogD("%s %s[%d] ============================ \n", __FILE__, __FUNCTION__, __LINE__);		
		if (doDownloadFile(face_img, "/mnt/user/facedb/RegImage.jpeg"))
		{
		LogD("%s %s[%d] ============================ \n", __FILE__, __FUNCTION__, __LINE__);			
			if (person_uuid.length() > 0 && person_name.length() > 0)
			{
		LogD("%s %s[%d] ============================ \n", __FILE__, __FUNCTION__, __LINE__);				
				//int result = -1000;
				int faceNum = 0;
				double threshold = 0;
				QByteArray faceFeature;
				result = ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->RegistPerson("/mnt/user/facedb/RegImage.jpeg", faceNum, threshold, faceFeature);
				LogD("%s %s[%d] RegistPerson result %d \n", __FILE__, __FUNCTION__, __LINE__, result);
				LogD("%s %s[%d] auth_type %s \n", __FILE__, __FUNCTION__, __LINE__, auth_type.toStdString().c_str());
				LogD("%s %s[%d] time_data %s \n", __FILE__, __FUNCTION__, __LINE__, time_data.toStdString().c_str());
				LogD("%s %s[%d] time_range %s \n", __FILE__, __FUNCTION__, __LINE__, time_range.toStdString().c_str());

				if (result == 0)
				{
					QString timeOfAccess = "";
					if (auth_type == "time_range")
					{
						timeOfAccess = time_range;
					} else if (auth_type == "week_cycle")
					{
						//data = 1,2,3,4,5,6,7
						//time_range = 00:00;21:00
						time_range.replace(QString(";"), QString(","));
						timeOfAccess = time_range;

						QStringList sections = time_data.split(",");
						for (int i = 1; i <= 7; i++)
						{
							bool isSet = false;
							timeOfAccess += ",";
							for (int j = 0; j < sections.size(); j++)
							{
								if (sections.at(j) == QString::number(i))
								{
									timeOfAccess += "1";
									isSet = true;
									break;
								}
							}
							if (isSet == false)
							{
								timeOfAccess += "0";
							}
						}
					}

					bool isSaveDBOk = RegisteredFacesDB::GetInstance()->RegPersonToDBAndRAM(person_uuid, person_name, id_card_no, card_no,
							male, group, timeOfAccess, faceFeature);
					LogD("%s %s[%d] RegPersonToDBAndRAM isSaveDBOk %d \n", __FILE__, __FUNCTION__, __LINE__, isSaveDBOk);

					if (isSaveDBOk)
					{
						okUUID += person_uuid+ ",";
					}
				} else //注册失败
				{

					//"fail_uuid":"1,2,3,4,", //注册成功之后返回的PID 字符串类型
					//"failresult": "1", //成功标志 或 注明失败的原因, 如果ID值，请附一张对应的出错原因表。
					failReason="person_uuid,或 person_name 为空";
					switch (result)
					{
						case -1:
							failReason ="path (/mnt/user/facedb/RegImage.jpeg) not exit " ;
						case -2:
							failReason ="bface_detect_face failed error (检测人脸失败,照片质量较低)" ;
						case -3:
							failReason ="bface_detect_face no face(无人脸数据,照片质量较低)" ;
						case -4:
							failReason ="bface_alignment failed error (提取人脸关键点失败,照片质量较低)" ;
						case -5:
							failReason ="bface_quality_score failed error (计算人脸得分较低,照片质量较低)" ;
						case -6:		
							failReason ="bface_extract_feature failed error (提取人脸特征失败, 照片质量较低)" ;		
						case -7:		
							failReason ="人脸数>2" ;																
						default:
						   failReason ="错误!";
					}
					//failUUID = person_uuid+ ",";
				}
				person_uuid="";
			} else //if (person_uuid.length() > 0 && person_name.length() > 0)
			{
				//failresult  person_uuid,或 person_name 为空
				failReason="person_uuid,或 person_name 为空";
				//failUUID = person_uuid+ ",";
			}
		} else //doDownloadFile(face_img, "/mnt/user/facedb/RegImage.jpeg") 
		{
			failReason="下载 /mnt/user/facedb/RegImage.jpeg 失败";
			//failUUID = person_uuid+ ",";
			//fail_uuid
			//failresult  不能下载 /mnt/user/facedb/RegImage.jpeg
			//doDownloadFile(face_img, "/mnt/user/facedb/RegImage.jpeg") 
		}
		if (result<0) 
		{
			Json::Value faildataitem;		
			faildataitem["uuid"] = person_uuid.toStdString().c_str();
			faildataitem["failReason"] = failReason.toStdString().c_str();
			faildata["faildata"].append(faildataitem);
		}
	}

	Json::Value json;
	std::string timestamp;
	std::string password;
	timestamp = getTime();
	password = mHttpServerPassword.toStdString() + timestamp;
	password = md5sum(password);
	password = md5sum(password);
	transform(password.begin(), password.end(), password.begin(), ::tolower);
	json["msg_type"] = stMsgHeader.msg_type.c_str();
	json["sn"] = stMsgHeader.sn.c_str();
	json["timestamp"] = timestamp.c_str();
	json["password"] = password.c_str();
	json["cmd_id"] = stMsgHeader.cmd_id.c_str();

	if (okUUID.length() > 1)
	{
		json["person_uuid"] = okUUID.toStdString().c_str();
		json["result"] = "1";
		json["success"] = "1";
	} else
	{
		json["person_uuid"] = "";
		json["result"] = "0";
		json["success"] = "0";
	}

	//if (failUUID.length() > 1)
	//{
	//	json["fail_uuid"] = failUUID.toStdString().c_str();
	//}

	//失败原因
	json["faildata"] = faildata;

	QString ret = doPostJson(json);
	if (ret.length() > 0)
	{
		msg = ret;
	}
}


void ConnHttpServerThreadPrivate::doDeletePerson(Json::Value root, MESSAGE_HEADER_S stMsgHeader)
{
	
	QString delPersonUUID = QString::fromStdString(root["person_uuid"].asString());
	QString personType = QString::fromStdString(root["person_type"].asString());
	QString delPersonUUIDs = "";
	QString effective = "";
	QString invalid = "";
	QStringList sections = delPersonUUID.split(",");
	for (int i = 0; i < sections.size(); i++)
	{
		QString uuid = sections[i];
		bool isDelOk = RegisteredFacesDB::GetInstance()->DelPersonByPersionUUIDFromDBAndRAM(uuid);
		if (isDelOk)
		{
			LogD("%s %s[%d] uuid %s delete success \n", __FILE__, __FUNCTION__, __LINE__, uuid.toStdString().c_str());
			if (effective.length() > 0)
			{
				effective += ",";
			}
			effective += uuid;
			continue;
		}

		if (invalid.length() > 0)
		{
			invalid += ",";
		}
		invalid += uuid;
	}

	Json::Value json;
	std::string timestamp;
	std::string password;
	timestamp = getTime();
	password = mHttpServerPassword.toStdString() + timestamp;
	password = md5sum(password);
	password = md5sum(password);
	transform(password.begin(), password.end(), password.begin(), ::tolower);
	json["msg_type"] = stMsgHeader.msg_type.c_str();
	json["sn"] = stMsgHeader.sn.c_str();
	json["timestamp"] = timestamp.c_str();
	json["password"] = password.c_str();
	json["cmd_id"] = stMsgHeader.cmd_id.c_str();
	json["effective"] = effective.toStdString().c_str();
	json["invalid"] = invalid.toStdString().c_str();
	json["message"] = "";
	json["result"] = "0";
	json["success"] = "0";
	if (effective.length() > 0)
	{
		json["result"] = "1";
		json["success"] = "1";
	}
	QString ret = doPostJson(json);
	msg = "";
	if (ret.length() > 0)
	{
		msg = ret;
	}
}

void ConnHttpServerThreadPrivate::doDeleteAllPerson(Json::Value root, MESSAGE_HEADER_S stMsgHeader)
{

	Json::Value json;
	std::string timestamp;
	std::string password;
	timestamp = getTime();
	password = mHttpServerPassword.toStdString() + timestamp;
	password = md5sum(password);
	password = md5sum(password);
	transform(password.begin(), password.end(), password.begin(), ::tolower);
	json["msg_type"] = stMsgHeader.msg_type.c_str();
	json["sn"] = stMsgHeader.sn.c_str();
	json["timestamp"] = timestamp.c_str();
	json["password"] = password.c_str();
	json["cmd_id"] = stMsgHeader.cmd_id.c_str();
	json["result"] = "1";
	json["success"] = "1";
	json["message"] = "del all person OK.";

	doPostJson(json);
	myHelper::Utils_ExecCmd("rm -rf /mnt/user/facedb/isc.db");
	myHelper::Utils_ExecCmd("rm -rf /mnt/user/facedb/isc_arcsoft_face.db");
	myHelper::Utils_ExecCmd("sync");
	myHelper::Utils_Reboot();
	while (1)
	{
		sleep(1);
	}
}

void ConnHttpServerThreadPrivate::doUpdatePerson(Json::Value root, MESSAGE_HEADER_S stMsgHeader)
{

	QString person_uuid = QString::fromStdString(root["person_uuid"].asString());
	QString card_no = QString::fromStdString(root["card_no"].asString());
	QString id_card_no = QString::fromStdString(root["id_card_no"].asString());
	QString person_name = QString::fromStdString(root["person_name"].asString());
	QString person_type = QString::fromStdString(root["person_type"].asString());
	QString group = QString::fromStdString(root["group"].asString());
	QString male = QString::fromStdString(root["male"].asString());
	QString face_img = QString::fromStdString(root["face_img"].asString());
	QString auth_type = QString::fromStdString(root["auth_type"].asString());
	QString time_data = QString::fromStdString(root["data"].asString());
	QString time_range = QString::fromStdString(root["time_range"].asString());
	QString person_image = "";
	mkdir("/mnt/user/tmp/", 0777);
	if (person_uuid.length() > 0 && person_name.length() > 0)
	{
		unlink("/mnt/user/facedb/RegImage.jpeg");
		if (doDownloadFile(face_img, "/mnt/user/facedb/RegImage.jpeg"))
		{
			person_image = "/mnt/user/facedb/RegImage.jpeg";
		}

		Json::Value json;
		std::string timestamp;
		std::string password;
		timestamp = getTime();
		password = mHttpServerPassword.toStdString() + timestamp;
		password = md5sum(password);
		password = md5sum(password);
		transform(password.begin(), password.end(), password.begin(), ::tolower);
		json["msg_type"] = stMsgHeader.msg_type.c_str();
		json["sn"] = stMsgHeader.sn.c_str();
		json["timestamp"] = timestamp.c_str();
		json["password"] = password.c_str();
		json["cmd_id"] = stMsgHeader.cmd_id.c_str();

		json["person_uuid"] = person_uuid.toStdString().c_str();
		json["result"] = "0";
		json["success"] = "0";

		int result = -1;
		QString timeOfAccess = "";
		if (auth_type == "time_range")
		{
			timeOfAccess = time_range;
		} else if (auth_type == "week_cycle")
		{
			//data = 1,2,3,4,5,6,7
			//time_range = 00:00;21:00
			time_range.replace(QString(";"), QString(","));
			timeOfAccess = time_range;

			QStringList sections = time_data.split(",");
			for (int i = 1; i <= 7; i++)
			{
				bool isSet = false;
				timeOfAccess += ",";
				for (int j = 0; j < sections.size(); j++)
				{
					if (sections.at(j) == QString::number(i))
					{
						timeOfAccess += "1";
						isSet = true;
						break;
					}
				}
				if (isSet == false)
				{
					timeOfAccess += "0";
				}
			}
		}
		result = RegisteredFacesDB::GetInstance()->UpdatePersonToDBAndRAM(person_uuid, person_name, id_card_no, card_no, male, group,
				timeOfAccess, person_image);
		LogD("%s %s[%d] UpdatePersonToDBAndRAM result %d \n", __FILE__, __FUNCTION__, __LINE__, result);

		usleep(500 * 1000);
		if (result == ISC_OK)
		{
			json["result"] = "1";
			json["success"] = "1";
			json["message"] = "person register success!";
		} else if (result == MERR_ASF_FACEENGINE_FACEDETECT)
		{
			json["result"] = "-1";
			json["success"] = "0";
			json["message"] = "picture do not have face";
		} else if (result == MERR_ASF_FACEENGINE_MULTIFACE)
		{
			json["result"] = "-2";
			json["success"] = "0";
			json["message"] = "picture have many face";
		} else if (result == MERR_ASF_FACEENGINE_FACEEXTRACT)
		{
			json["result"] = "-3";
			json["success"] = "0";
			json["message"] = "extract the feature failed!";
		} else if (result == ISC_ERROR_EXIST)
		{
			json["result"] = "-4";
			json["success"] = "0";
			json["message"] = "person already register";
		} else if (result == MERR_ASF_FACEENGINE_FACEQUALITY)
		{
			json["result"] = "-5";
			json["success"] = "0";
			json["message"] = "the picture is too low or the image is too bright or blurry";
		} else
		{
			json["result"] = "-6";
			json["success"] = "0";
			json["message"] = "register failed, unknown error, please try again";
		}

		QString ret = doPostJson(json);
		msg = "";
		if (ret.length() > 0)
		{
			msg = ret;
		}
	}
}

void ConnHttpServerThreadPrivate::doFindPerson(Json::Value root, MESSAGE_HEADER_S stMsgHeader)
{
	
	QString uuid = QString::fromStdString(root["uuid"].asString());
	int nPersonType = 0;
	PERSONS_t stPerson = { 0 };
	if (uuid.length() > 0)
	{
		QList<PERSONS_t> persons = RegisteredFacesDB::GetInstance()->GetPersonDataByPersonUUIDFromRAM(uuid);
		if (persons.size() > 0)
		{
			stPerson = persons[0];
		}
	}
	Json::Value json;
	std::string timestamp;
	std::string password;
	timestamp = getTime();
	password = mHttpServerPassword.toStdString() + timestamp;
	password = md5sum(password);
	password = md5sum(password);
	transform(password.begin(), password.end(), password.begin(), ::tolower);
	json["msg_type"] = stMsgHeader.msg_type.c_str();
	json["sn"] = stMsgHeader.sn.c_str();
	json["timestamp"] = timestamp.c_str();
	json["password"] = password.c_str();
	json["cmd_id"] = stMsgHeader.cmd_id.c_str();

	if (stPerson.uuid.size() > 3)
	{
		json["count"] = "1";
		json["current_page"] = "1";
		json["message"] = "people data";
		json["page_count"] = "1";
		json["result"] = "1";
		json["success"] = "1";
		json["total_page"] = "1";

		Json::Value person;
		person["person_uuid"] = stPerson.uuid.toStdString().c_str();
		person["name"] = stPerson.name.toStdString().c_str();
		person["male"] = stPerson.sex.toStdString().c_str();
		person["id_card_no"] = stPerson.idcard.toStdString().c_str();
		person["card_no"] = stPerson.iccard.toStdString().c_str();
		person["add_time"] = stPerson.createtime.toStdString().c_str();
		json["data"].append(person);
	} else
	{
		json["count"] = "0";
		json["current_page"] = "0";
		json["message"] = "";
		json["page_count"] = "0";
		json["result"] = "0";
		json["success"] = "0";
		json["total_page"] = "0";
	}
	QString ret = doPostJson(json);
	msg = "";
	if (ret.length() > 0)
	{
//			msg = ret;
	}
}

void ConnHttpServerThreadPrivate::doGetAllPerson(Json::Value root, MESSAGE_HEADER_S stMsgHeader)
{
	
	Json::Value json;
	std::string timestamp;
	std::string password;
	timestamp = getTime();
	password = mHttpServerPassword.toStdString() + timestamp;
	password = md5sum(password);
	password = md5sum(password);
	transform(password.begin(), password.end(), password.begin(), ::tolower);
	json["msg_type"] = stMsgHeader.msg_type.c_str();
	json["sn"] = stMsgHeader.sn.c_str();
	json["timestamp"] = timestamp.c_str();
	json["password"] = password.c_str();
	json["cmd_id"] = stMsgHeader.cmd_id.c_str();

	QString page = QString::fromStdString(root["page"].asString());
	QString per_page = QString::fromStdString(root["per_page"].asString());
	QList<PERSONS_t> list = RegisteredFacesDB::GetInstance()->GetAllPersonFromRAM();
	int person_count = list.size();

	LogD("%s %s[%d] get all person :%d. page: %s, per_page: %s \n", __FILE__, __FUNCTION__, __LINE__, person_count,
			page.toStdString().c_str(), per_page.toStdString().c_str());
	int nPage = page.toInt();
	int nPerPage = per_page.toInt();
	int nRealPageCount = 0;

	if (nPage >= 0 && nPerPage > 0 && person_count > 0)
	{
		for (int i = 0; i < nPerPage; i++)
		{
			int index = i + nPage * nPerPage;
			if (index >= person_count)
			{
				break;
			}
			nRealPageCount++;
			Json::Value data;
			PERSONS_t stPerson = list[i];
			data["person_uuid"] = stPerson.uuid.toStdString().c_str();
			data["name"] = stPerson.name.toStdString().c_str();
			data["male"] = stPerson.sex.toStdString().c_str();
			data["id_card_no"] = stPerson.idcard.toStdString().c_str();
			data["card_no"] = stPerson.iccard.toStdString().c_str();
			data["add_time"] = stPerson.createtime.toStdString().c_str();
			data["department"] = stPerson.department.toStdString().c_str();

			json["data"].append(data);
		}
	}
	json["message"] = "";
	json["page_count"] = std::to_string(nRealPageCount).c_str();
	if (nRealPageCount > 0)
	{
		json["result"] = "1";
		json["success"] = "1";
		int nTotalPage = (person_count / nPerPage);
		float fTotalPage = ((float) person_count / (float) nPerPage);
		if (fTotalPage != nTotalPage)
		{
			nTotalPage++;
		}
		json["total_page"] = std::to_string(nTotalPage).c_str();
	} else
	{
		json["result"] = "0";
		json["success"] = "0";
		json["total_page"] = "0";
	}

	QString ret = doPostJson(json);
	msg = "";
	if (ret.length() > 0)
	{
//			msg = ret;
	}
}

void ConnHttpServerThreadPrivate::doSetTimeOfAccess(Json::Value root, MESSAGE_HEADER_S stMsgHeader)
{

	int nPersonType = 0;
	QString person_uuid = QString::fromStdString(root["person_uuid"].asString());
	QString auth_type = QString::fromStdString(root["auth_type"].asString());
	QString data = QString::fromStdString(root["data"].asString());
	QString time_range = QString::fromStdString(root["time_range"].asString());
	QString pass_flag = QString::fromStdString(root["pass_flag"].asString());
	QString timeOfAccess;

	LogD("%s %s[%d] person_uuid %s \n", __FILE__, __FUNCTION__, __LINE__, person_uuid.toStdString().c_str());
	LogD("%s %s[%d] auth_type %s \n", __FILE__, __FUNCTION__, __LINE__, auth_type.toStdString().c_str());
	LogD("%s %s[%d] data %s \n", __FILE__, __FUNCTION__, __LINE__, data.toStdString().c_str());
	LogD("%s %s[%d] time_range %s \n", __FILE__, __FUNCTION__, __LINE__, time_range.toStdString().c_str());
	LogD("%s %s[%d] pass_flag %s \n", __FILE__, __FUNCTION__, __LINE__, pass_flag.toStdString().c_str());

	bool isOk = false;
	if (auth_type == "time_range")
	{
		timeOfAccess = time_range;  //time_range = 2000/01/01 00:00;2000/01/01 00:00
	} else if (auth_type == "week_cycle")
	{
		//data = 1,2,3,4,5,6,7
		//time_range = 00:00;21:00
		time_range.replace(QString(";"), QString(","));
		timeOfAccess = time_range;

		QStringList sections = data.split(",");
		for (int i = 1; i <= 7; i++)
		{
			bool isSet = false;
			timeOfAccess += ",";
			for (int j = 0; j < sections.size(); j++)
			{
				if (sections.at(j) == QString::number(i))
				{
					timeOfAccess += "1";
					isSet = true;
					break;
				}
			}
			if (isSet == false)
			{
				timeOfAccess += "0";
			}
		}
	}

	LogV("%s %s[%d] timeOfAccess %s\n", __FILE__, __FUNCTION__, __LINE__, timeOfAccess.toStdString().c_str());
	if (timeOfAccess.size() > 3)
	{
		isOk = RegisteredFacesDB::GetInstance()->UpdatePersonToDBAndRAM(person_uuid, "", "", "", "", "", timeOfAccess, "");
	}

	Json::Value json;
	std::string timestamp;
	std::string password;
	timestamp = getTime();
	password = mHttpServerPassword.toStdString() + timestamp;
	password = md5sum(password);
	password = md5sum(password);
	transform(password.begin(), password.end(), password.begin(), ::tolower);
	json["msg_type"] = stMsgHeader.msg_type.c_str();
	json["sn"] = stMsgHeader.sn.c_str();
	json["timestamp"] = timestamp.c_str();
	json["password"] = password.c_str();
	json["cmd_id"] = stMsgHeader.cmd_id.c_str();

	printf(">>>%s,%s,%d,isOk=%d\n",__FILE__,__func__,__LINE__,isOk);
	if (isOk == 1)//if (isOk == true)
	{
		json["result"] = "1";
		json["success"] = "1";
	} else
	{		
		switch (isOk)
		{
		  case  -1 : 
				json["result"] = "-1; uuid 为空";
		  case  -5 : //ISC_UPDATE_PERSON_NOT_EXIST
		        json["result"] = "-5; ISC_UPDATE_PERSON_NOT_EXIST PERSON 不存在";  
		  case -6:
		  		json["result"] = "-6; ISC_UPDATE_PERSON_DB_ERROR 更新数据库出错"; 			
			case -11:
				json["result"] ="-11; path (/mnt/user/facedb/RegImage.jpeg) not exit " ;
			case -12:
				json["result"] ="-12;bface_detect_face failed error (检测人脸失败,照片质量较低)" ;
			case -13:
				json["result"] ="-13;bface_detect_face no face(无人脸数据,照片质量较低)" ;
			case -14:
				json["result"] ="-14;bface_alignment failed error (提取人脸关键点失败,照片质量较低)" ;
			case -15:
				json["result"] ="-15;bface_quality_score failed error (计算人脸得分较低,照片质量较低)" ;
			case -16:		
				json["result"] ="-16;bface_extract_feature failed error (提取人脸特征失败, 照片质量较低)" ;		
			case -17:		
				json["result"] ="-17;人脸数>2" ;																
			default:
				json["result"] ="错误!";
		}		  

		json["success"] = "0";
	}

	QString ret = doPostJson(json);
	msg = "";
	if (ret.length() > 0)
	{
//			msg = ret;
	}
}

void ConnHttpServerThreadPrivate::doDeleteTimeOfAccess(Json::Value root, MESSAGE_HEADER_S stMsgHeader)
{

	int nPersonType = 0;
	QString person_uuid = QString::fromStdString(root["person_uuid"].asString());
	LogD("%s %s[%d] person_uuid %s \n", __FILE__, __FUNCTION__, __LINE__, person_uuid.toStdString().c_str());

	bool isOk = false;
	QString timeOfAccess = "";
	isOk = RegisteredFacesDB::GetInstance()->UpdatePersonToDBAndRAM(person_uuid, "", "", "", "", "", timeOfAccess, "");

	Json::Value json;
	std::string timestamp;
	std::string password;
	timestamp = getTime();
	password = mHttpServerPassword.toStdString() + timestamp;
	password = md5sum(password);
	password = md5sum(password);
	transform(password.begin(), password.end(), password.begin(), ::tolower);
	json["msg_type"] = stMsgHeader.msg_type.c_str();
	json["sn"] = stMsgHeader.sn.c_str();
	json["timestamp"] = timestamp.c_str();
	json["password"] = password.c_str();
	json["cmd_id"] = stMsgHeader.cmd_id.c_str();

	if (isOk == true)
	{
		json["result"] = "1";
		json["success"] = "1";
	} else
	{
		json["result"] = "0";
		json["success"] = "0";
	}

	QString ret = doPostJson(json);
	msg = "";
	if (ret.length() > 0)
	{
//			msg = ret;
	}
}

void ConnHttpServerThreadPrivate::doDeleteAllPersonRecord(Json::Value root, MESSAGE_HEADER_S stMsgHeader)
{

	Json::Value json;
	std::string timestamp;
	std::string password;
	timestamp = getTime();
	password = mHttpServerPassword.toStdString() + timestamp;
	password = md5sum(password);
	password = md5sum(password);
	transform(password.begin(), password.end(), password.begin(), ::tolower);
	json["msg_type"] = stMsgHeader.msg_type.c_str();
	json["sn"] = stMsgHeader.sn.c_str();
	json["timestamp"] = timestamp.c_str();
	json["password"] = password.c_str();
	json["cmd_id"] = stMsgHeader.cmd_id.c_str();
	json["result"] = "1";
	json["success"] = "1";

	QString ret = doPostJson(json);
	myHelper::Utils_ExecCmd("rm -rf /mnt/user/facedb/isc_ir_arcsoft_face.db");
	myHelper::Utils_ExecCmd("sync");
	myHelper::Utils_Reboot();
	msg = "";
	if (ret.length() > 0)
	{
//			msg = ret;
	}
}

void ConnHttpServerThreadPrivate::doGetPersonRecord(Json::Value root, MESSAGE_HEADER_S stMsgHeader)
{

	QString person_uuid = QString::fromStdString(root["person_uuid"].asString());
	QString type = QString::fromStdString(root["type"].asString());
	QString page = QString::fromStdString(root["page"].asString());
	QString name = QString::fromStdString(root["name"].asString());
	QString per_page = QString::fromStdString(root["per_page"].asString());
	QString start_time = QString::fromStdString(root["start_time"].asString());
	QString end_time = QString::fromStdString(root["end_time"].asString());

//		LogD("%s %s[%d] type %s \n", __FILE__, __FUNCTION__, __LINE__, type.c_str());
//		LogD("%s %s[%d] page %s \n", __FILE__, __FUNCTION__, __LINE__, page.c_str());
//		LogD("%s %s[%d] name %s \n", __FILE__, __FUNCTION__, __LINE__, name.c_str());
//		LogD("%s %s[%d] per_page %s \n", __FILE__, __FUNCTION__, __LINE__, per_page.c_str());
//		LogD("%s %s[%d] start_time %s \n", __FILE__, __FUNCTION__, __LINE__, start_time.c_str());
//		LogD("%s %s[%d] end_time %s \n", __FILE__, __FUNCTION__, __LINE__, end_time.c_str());

	int nPage = page.toInt();
	int nPerPage = per_page.toInt();
	int nTotalCount = 0;
	int nDataCount = 0;

	QList<IdentifyFaceRecord_t> list;
	int select_type = type.toInt();
	switch (select_type)
	{
	case 1:
	{   //select by name
//		LogD("%s %s[%d] name %s \n", __FILE__, __FUNCTION__, __LINE__, name.toStdString().c_str());
		nTotalCount = PersonRecordToDB::GetInstance()->GetPersonRecordTotalNumByName(name, false);
		list = PersonRecordToDB::GetInstance()->GetPersonRecordDataByName(nPage, nPerPage, name, false);
		nDataCount = list.size();
		break;
	}
	case 2:
	{   //select by uid
//		LogD("%s %s[%d] person_uuid %s \n", __FILE__, __FUNCTION__, __LINE__, person_uuid.toStdString().c_str());
		nTotalCount = PersonRecordToDB::GetInstance()->GetPersonRecordTotalNumByPersonUUID(person_uuid, false);
		list = PersonRecordToDB::GetInstance()->GetPersonRecordDataByPersonUUID(nPage, nPerPage, person_uuid, false);
		nDataCount = list.size();
		break;
	}
	case 3:
	{   //select by time
//		LogD("%s %s[%d] start %s end %s \n", __FILE__, __FUNCTION__, __LINE__, start_time.toStdString().c_str(),
//				end_time.toStdString().c_str());
		QDateTime startDateTime = QDateTime::fromString(start_time, "yyyy/MM/dd hh:mm:ss");
		QDateTime endDateTime = QDateTime::fromString(end_time, "yyyy/MM/dd hh:mm:ss");
		if (startDateTime.isValid() == false)
		{
			startDateTime = QDateTime::fromString(start_time, "yyyy/MM/dd hh:mm");
		}
		if (endDateTime.isValid() == false)
		{
			endDateTime = QDateTime::fromString(end_time, "yyyy/MM/dd hh:mm");
		}
		nTotalCount = PersonRecordToDB::GetInstance()->GetPersonRecordTotalNumByDateTime(startDateTime, endDateTime, false);
		list = PersonRecordToDB::GetInstance()->GetPersonRecordDataByDateTime(nPage, nPerPage, startDateTime, endDateTime, false);
		nDataCount = list.size();
		break;
	}
	case 4:
	{   //select by time and not upload
//		LogD("%s %s[%d] start %s end %s \n", __FILE__, __FUNCTION__, __LINE__, start_time.toStdString().c_str(),
//				end_time.toStdString().c_str());
		QDateTime startDateTime = QDateTime::fromString(start_time, "yyyy/MM/dd hh:mm:ss");
		QDateTime endDateTime = QDateTime::fromString(end_time, "yyyy/MM/dd hh:mm:ss");
		if (startDateTime.isValid() == false)
		{
			startDateTime = QDateTime::fromString(start_time, "yyyy/MM/dd hh:mm");
		}
		if (endDateTime.isValid() == false)
		{
			endDateTime = QDateTime::fromString(end_time, "yyyy/MM/dd hh:mm");
		}
		nTotalCount = PersonRecordToDB::GetInstance()->GetPersonRecordTotalNumByDateTime(startDateTime, endDateTime, true);
		list = PersonRecordToDB::GetInstance()->GetPersonRecordDataByDateTime(nPage, nPerPage, startDateTime, endDateTime, true);
		nDataCount = list.size();
		break;
	}
	default:
		break;
	}

	Json::Value json;
	std::string timestamp;
	std::string password;
	timestamp = getTime();
	password = mHttpServerPassword.toStdString() + timestamp;
	password = md5sum(password);
	password = md5sum(password);
	transform(password.begin(), password.end(), password.begin(), ::tolower);
	json["msg_type"] = stMsgHeader.msg_type.c_str();
	json["sn"] = stMsgHeader.sn.c_str();
	json["timestamp"] = timestamp.c_str();
	json["password"] = password.c_str();
	json["cmd_id"] = stMsgHeader.cmd_id.c_str();
	json["current_page"] = page.toStdString().c_str();
	json["dev_name"] = "";
	json["location"] = "";
	json["message"] = "records select";

	json["per_page"] = "0";
	json["total_page"] = "0";
	json["result"] = "0";
	json["success"] = "0";

	if (nTotalCount > 0 && nDataCount > 0)
	{
		for (int i = 0; i < nDataCount; i++)
		{
			if (i >= list.size())
			{
				break;
			}
			auto &t = list[i];

			if(access(t.FaceImgPath.toStdString().c_str(), F_OK))
			{
				LogE("%s %s[%d] %s %s not exist , so ignore \n",__FILE__,__FUNCTION__,__LINE__,t.face_name.toStdString().c_str(),t.FaceImgPath.toStdString().c_str());
				continue;
			}
			Json::Value data;
			data["crop_data"] = fileToBase64String(t.FaceImgPath.toStdString().c_str());
			data["name"] = t.face_name.toStdString().c_str();
			data["time"] = t.createtime.toString("yyyy/MM/dd hh:mm:ss").toStdString().c_str();
			data["uuid"] = t.face_uuid.toStdString().c_str();
			data["rID"] = std::to_string(t.rid).c_str();
			data["group"] = t.face_gids.toStdString().c_str();
			data["person_type"] = std::to_string(t.face_persontype).c_str();
			data["face_mack"] = std::to_string(t.face_mack).c_str();
			data["temp_value"] = std::to_string(t.temp_value).c_str();
			json["data"].append(data);
		}
		json["per_page"] = std::to_string(nDataCount).c_str();
		json["count"] = std::to_string(nDataCount).c_str();
		json["total_page"] = std::to_string(nTotalCount / nPerPage);
		json["result"] = "1";
		json["success"] = "1";
	}

	QString ret = doPostJson(json);
	msg = "";
	if (ret.length() > 0)
	{
		Json::Reader reader;
		if (reader.parse(ret.toStdString(), root))
		{
			if (std::string("records_select_done") == root["msg_type"].asString())
			{
				msg = ret;
			}
		}
	}
}

void ConnHttpServerThreadPrivate::doUpdatePersonRecordUploadFlag(Json::Value root, MESSAGE_HEADER_S stMsgHeader)
{
	QString rIDs = QString::fromStdString(root["rID"].asString());
	QStringList sections = rIDs.split(",");
	LogD("%s %s[%d] rIDs %s \n", __FILE__, __FUNCTION__, __LINE__, rIDs.toStdString().c_str());
	for (int i = 0; i < sections.size(); i++)
	{
		auto &t = sections[i];
		if (t.size() <= 0)
		{
			continue;
		}
		int rid = sections[i].toInt();
		PersonRecordToDB::GetInstance()->UpdatePersonRecordUploadFlag(rid, true);
	}
	msg = "";
}

void ConnHttpServerThreadPrivate::doDeletePersonRecord(Json::Value root, MESSAGE_HEADER_S stMsgHeader)
{
	QString type = QString::fromStdString(root["type"].asString());
	QString name = QString::fromStdString(root["name"].asString());
	QString rIDs = QString::fromStdString(root["rID"].asString());
	QString start_time = QString::fromStdString(root["start_time"].asString());
	QString end_time = QString::fromStdString(root["end_time"].asString());

	LogD("%s %s[%d] type %s \n", __FILE__, __FUNCTION__, __LINE__, type.toStdString().c_str());
	LogD("%s %s[%d] name %s \n", __FILE__, __FUNCTION__, __LINE__, name.toStdString().c_str());
	LogD("%s %s[%d] rIDs %s \n", __FILE__, __FUNCTION__, __LINE__, rIDs.toStdString().c_str());
	LogD("%s %s[%d] start_time %s \n", __FILE__, __FUNCTION__, __LINE__, start_time.toStdString().c_str());
	LogD("%s %s[%d] end_time %s \n", __FILE__, __FUNCTION__, __LINE__, end_time.toStdString().c_str());

	int nRet = ISC_ERROR;
	int nSelectType = type.toInt();
	switch (nSelectType)
	{
	case 1:
	{   //delete by name
		if (PersonRecordToDB::GetInstance()->DeletePersonRecordByName(name) == true)
		{
			nRet = ISC_OK;
		}
		break;
	}
	case 2:
	{   //delete by rid
		nRet = ISC_OK;
		QStringList sections = rIDs.split(",");
		for (int i = 0; i < sections.size(); i++)
		{
			PersonRecordToDB::GetInstance()->DeletePersonRecordByRID(sections[i].toLong());
		}
		break;
	}
	case 3:
	{   //delete by time
		QDateTime startDateTime = QDateTime::fromString(start_time, "yyyy/MM/dd hh:mm:ss");
		QDateTime endDateTime = QDateTime::fromString(end_time, "yyyy/MM/dd hh:mm:ss");
		if (startDateTime.isValid() == false)
		{
			startDateTime = QDateTime::fromString(start_time, "yyyy/MM/dd hh:mm");
		}
		if (endDateTime.isValid() == false)
		{
			endDateTime = QDateTime::fromString(end_time, "yyyy/MM/dd hh:mm");
		}
		PersonRecordToDB::GetInstance()->DeletePersonRecordByTime(startDateTime, endDateTime);
		break;
	}
	default:
		break;
	}

	Json::Value json;
	std::string timestamp;
	std::string password;
	timestamp = getTime();
	password = mHttpServerPassword.toStdString() + timestamp;
	password = md5sum(password);
	password = md5sum(password);
	transform(password.begin(), password.end(), password.begin(), ::tolower);
	json["msg_type"] = stMsgHeader.msg_type.c_str();
	json["sn"] = stMsgHeader.sn.c_str();
	json["timestamp"] = timestamp.c_str();
	json["password"] = password.c_str();
	json["cmd_id"] = stMsgHeader.cmd_id.c_str();
	json["type"] = type.toStdString().c_str();

	if (nRet == ISC_OK)
	{
		json["result"] = "1";
		json["success"] = "1";
	} else
	{
		json["result"] = "0";
		json["success"] = "0";
	}

	QString ret = doPostJson(json);
	msg = "";
	if (ret.length() > 0)
	{
//			msg = ret;
	}
}

void ConnHttpServerThreadPrivate::doOpenDoor(Json::Value root, MESSAGE_HEADER_S stMsgHeader)
{


	Json::Value json;
	std::string timestamp;
	std::string password;
	timestamp = getTime();
	password = mHttpServerPassword.toStdString() + timestamp;
	password = md5sum(password);
	password = md5sum(password);
	transform(password.begin(), password.end(), password.begin(), ::tolower);
	json["msg_type"] = stMsgHeader.msg_type.c_str();
	json["sn"] = stMsgHeader.sn.c_str();
	json["timestamp"] = timestamp.c_str();
	json["password"] = password.c_str();
	json["cmd_id"] = stMsgHeader.cmd_id.c_str();
	json["result"] = "1";
	json["success"] = "1";
	//json["status"] = status.toStdString().c_str();
/*
状态 
-1:默认 延时时间
1:常开
2:常闭

//状态,默认 -1  延时 时间　　　常开 1，常闭　2
*/
	QString status = "-1";
	status = QString::fromStdString(root["status"].asString());
	//if (status.toInt() ==1 || status.toInt() ==2)
	{
		//保存　	
		
		ReadConfig::GetInstance()->setDoor_Relay(status.toInt());		
		LogD("%s %s[%d] status=%d \n", __FILE__, __FUNCTION__, __LINE__,status.toInt());		
	} 
	if (status.toInt() == -1)
		YNH_LJX::Utils_Door::GetInstance()->OpenDoor("");
	else if (status.toInt() == 1) //1:常开
	{
        YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Relay, 1);
	}
	else if (status.toInt() == 2) //2:常闭
	{        
        YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Relay, 0);
	}

	QString ret = doPostJson(json);
	msg = "";
	if (ret.length() > 0)
	{
//			msg = ret;
	}
}

void ConnHttpServerThreadPrivate::doReboot(Json::Value root, MESSAGE_HEADER_S stMsgHeader)
{
	Json::Value json;
	std::string timestamp;
	std::string password;
	timestamp = getTime();
	password = mHttpServerPassword.toStdString() + timestamp;
	password = md5sum(password);
	password = md5sum(password);
	transform(password.begin(), password.end(), password.begin(), ::tolower);
	json["msg_type"] = stMsgHeader.msg_type.c_str();
	json["sn"] = stMsgHeader.sn.c_str();
	json["timestamp"] = timestamp.c_str();
	json["password"] = password.c_str();
	json["cmd_id"] = stMsgHeader.cmd_id.c_str();
	json["result"] = "1";
	json["success"] = "1";

	QString ret = doPostJson(json);
	msg = "";
	if (ret.length() > 0)
	{
//			msg = ret;
	}
	myHelper::Utils_Reboot();
}

void ConnHttpServerThreadPrivate::doSetNetwork(Json::Value root, MESSAGE_HEADER_S stMsgHeader)
{
	QString ip_addr = QString::fromStdString(root["ip_addr"].asString());
	QString geteway = QString::fromStdString(root["geteway"].asString());
	QString subnet_mask = QString::fromStdString(root["subnet_mask"].asString());
	QString dns1 = QString::fromStdString(root["dns1"].asString());

	LogD("%s %s[%d] ip_addr %s \n", __FILE__, __FUNCTION__, __LINE__, ip_addr.toStdString().c_str());
	LogD("%s %s[%d] geteway %s \n", __FILE__, __FUNCTION__, __LINE__, geteway.toStdString().c_str());
	LogD("%s %s[%d] subnet_mask %s \n", __FILE__, __FUNCTION__, __LINE__, subnet_mask.toStdString().c_str());
	LogD("%s %s[%d] dns1 %s \n", __FILE__, __FUNCTION__, __LINE__, dns1.toStdString().c_str());

	int nRet = ISC_ERROR;

	NetworkControlThread::GetInstance()->setLinkLan(1, ip_addr, subnet_mask, geteway, dns1);

	ReadConfig::GetInstance()->setLAN_IP(ip_addr);
	ReadConfig::GetInstance()->setLAN_Maks(subnet_mask);
	ReadConfig::GetInstance()->setLAN_Gateway(geteway);
	ReadConfig::GetInstance()->setLAN_DNS(dns1);
	ReadConfig::GetInstance()->setSaveConfig();

	Json::Value json;
	std::string timestamp;
	std::string password;
	timestamp = getTime();
	password = mHttpServerPassword.toStdString() + timestamp;
	password = md5sum(password);
	password = md5sum(password);
	transform(password.begin(), password.end(), password.begin(), ::tolower);
	json["msg_type"] = stMsgHeader.msg_type.c_str();
	json["sn"] = stMsgHeader.sn.c_str();
	json["timestamp"] = timestamp.c_str();
	json["password"] = password.c_str();
	json["cmd_id"] = stMsgHeader.cmd_id.c_str();

	if (nRet == ISC_OK)
	{
		json["result"] = "1";
		json["success"] = "1";
	} else
	{
		json["result"] = "0";
		json["success"] = "0";
	}

	QString ret = doPostJson(json);
	msg = "";
	if (ret.length() > 0)
	{
//			msg = ret;
	}
	myHelper::Utils_Reboot();
}

void ConnHttpServerThreadPrivate::doGetDeviceConfig(Json::Value root, MESSAGE_HEADER_S stMsgHeader)
{
	Json::Value json;
	std::string timestamp;
	std::string password;
	timestamp = getTime();
	password = mHttpServerPassword.toStdString() + timestamp;
	password = md5sum(password);
	password = md5sum(password);
	transform(password.begin(), password.end(), password.begin(), ::tolower);
	json["msg_type"] = stMsgHeader.msg_type.c_str();
	json["sn"] = stMsgHeader.sn.c_str();
	json["timestamp"] = timestamp.c_str();
	json["password"] = password.c_str();
	json["cmd_id"] = stMsgHeader.cmd_id.c_str();
	json["result"] = "1";
	json["success"] = "1";

	QString dev_name_str = ReadConfig::GetInstance()->getHomeDisplay_DeviceName();
	json["dev_name"] = dev_name_str.toStdString().c_str();

	QString location_str = ReadConfig::GetInstance()->getHomeDisplay_Location();
	json["location"] = location_str.toStdString().c_str();

	QString support_lang_str;
	QDir dir("/isc/languages_baidu");
	if (dir.exists())
	{
		QFileInfoList fileList = dir.entryInfoList();
		for (int i = 0; i < fileList.size(); i++)
		{
			auto &t = fileList[i];
			if (t.fileName() == "." || t.fileName() == "..")
			{
				continue;
			}
			QString tmp = t.fileName().replace(".qm", "");
			tmp = tmp.replace("innohi_", "");
			support_lang_str += tmp + ",";
		}
	}
	json["support_lang"] = support_lang_str.toStdString().c_str();

	QString current_lang_str = ReadConfig::GetInstance()->getHomeDisplay_Language();
	json["current_lang"] = current_lang_str.toStdString().c_str();

	QString ip = ReadConfig::GetInstance()->getLAN_IP();
	QString netmask = ReadConfig::GetInstance()->getLAN_Maks();
	QString gateway = ReadConfig::GetInstance()->getLAN_Gateway();
	QString dns = ReadConfig::GetInstance()->getLAN_DNS();
	json["ip_address"] = ip.toStdString().c_str();
	json["ip_mask"] = netmask.toStdString().c_str();
	json["ip_gateway"] = gateway.toStdString().c_str();
	json["ip_dns"] = dns.toStdString().c_str();

	json["visit_record"] = std::to_string(ReadConfig::GetInstance()->getRecords_Manager_Stranger()).c_str();
	json["irliveness_threshold"] = std::to_string(ReadConfig::GetInstance()->getIdentity_Manager_Living_value()).c_str();
	json["recognition_compare_threshold"] = std::to_string(ReadConfig::GetInstance()->getIdentity_Manager_Thanthreshold()).c_str();
	json["identify_interval"] = std::to_string(ReadConfig::GetInstance()->getIdentity_Manager_IdentifyInterval()).c_str();

	int ipDhcpMode = ReadConfig::GetInstance()->getLan_DHCP();
	if (ipDhcpMode == 1)
	{
		json["ip_mode"] = "dynamic";
	} else
	{
		json["ip_mode"] = "static";
	}

	QString must_open_mode = ReadConfig::GetInstance()->getDoor_MustOpenMode();
	QString optional_open_mode = ReadConfig::GetInstance()->getDoor_OptionalOpenMode();

	must_open_mode.replace("&", ",");
	optional_open_mode.replace("|", ",");
	json["must_open_mode"] = must_open_mode.toStdString().c_str();
	json["optional_open_mode"] = optional_open_mode.toStdString().c_str();

	printf("%s %s[%d] strIpAddress %s \n", __FILE__, __FUNCTION__, __LINE__, ip.toStdString().c_str());
	printf("%s %s[%d] strNetMask %s \n", __FILE__, __FUNCTION__, __LINE__, netmask.toStdString().c_str());
	printf("%s %s[%d] strGateWay %s \n", __FILE__, __FUNCTION__, __LINE__, gateway.toStdString().c_str());
	printf("%s %s[%d] strDns %s \n", __FILE__, __FUNCTION__, __LINE__, dns.toStdString().c_str());
	QString ret = doPostJson(json);
	msg = "";
	if (ret.length() > 0)
	{
//			msg = ret;
	}
}

void ConnHttpServerThreadPrivate::doSetDeviceConfig(Json::Value root, MESSAGE_HEADER_S stMsgHeader)
{
	QString dev_name_str = QString::fromStdString(root["dev_name"].asString());
	QString location_str = QString::fromStdString(root["location"].asString());
	QString pass_startTime_str = QString::fromStdString(root["pass_startTime"].asString());
	QString pass_endTime_str = QString::fromStdString(root["pass_endTime"].asString());
	QString lang_str = QString::fromStdString(root["current_language"].asString());
	QString flash_range = QString::fromStdString(root["flash_range"].asString());
	QString temp_mode = QString::fromStdString(root["temp_mode"].asString());
	QString sleep_time = QString::fromStdString(root["sleep_time"].asString());
	QString visit_record = QString::fromStdString(root["visit_record"].asString());
	QString id_record = QString::fromStdString(root["id_record"].asString());
	QString must_open_mode = QString::fromStdString(root["must_open_mode"].asString());
	QString optional_open_mode = QString::fromStdString(root["optional_open_mode"].asString());
	QString save_crop = QString::fromStdString(root["save_crop"].asString());
	QString temp_detect = QString::fromStdString(root["temp_detect"].asString());
	QString irliveness_threshold = QString::fromStdString(root["irliveness_threshold"].asString());
	QString recognition_compare_threshold = QString::fromStdString(root["recognition_compare_threshold"].asString());
	QString identify_interval = QString::fromStdString(root["identify_interval"].asString());

	LogV("%s %s[%d] dev_name_str %s \n", __FILE__, __FUNCTION__, __LINE__, dev_name_str.toStdString().c_str());
	LogV("%s %s[%d] location_str %s \n", __FILE__, __FUNCTION__, __LINE__, location_str.toStdString().c_str());
	LogV("%s %s[%d] pass_startTime_str %s \n", __FILE__, __FUNCTION__, __LINE__, pass_startTime_str.toStdString().c_str());
	LogV("%s %s[%d] pass_endTime_str %s \n", __FILE__, __FUNCTION__, __LINE__, pass_endTime_str.toStdString().c_str());
	LogV("%s %s[%d] current_language %s \n", __FILE__, __FUNCTION__, __LINE__, lang_str.toStdString().c_str());
	LogV("%s %s[%d] flash_range %s \n", __FILE__, __FUNCTION__, __LINE__, flash_range.toStdString().c_str());
	LogV("%s %s[%d] temp_mode %s \n", __FILE__, __FUNCTION__, __LINE__, temp_mode.toStdString().c_str());
	LogV("%s %s[%d] sleep_time %s \n", __FILE__, __FUNCTION__, __LINE__, sleep_time.toStdString().c_str());
	LogV("%s %s[%d] visit_record %s \n", __FILE__, __FUNCTION__, __LINE__, visit_record.toStdString().c_str());
	LogV("%s %s[%d] id_record %s \n", __FILE__, __FUNCTION__, __LINE__, id_record.toStdString().c_str());
	LogV("%s %s[%d] must_open_mode %s \n", __FILE__, __FUNCTION__, __LINE__, must_open_mode.toStdString().c_str());
	LogV("%s %s[%d] optional_open_mode %s \n", __FILE__, __FUNCTION__, __LINE__, optional_open_mode.toStdString().c_str());
	LogV("%s %s[%d] save_crop %s \n", __FILE__, __FUNCTION__, __LINE__, save_crop.toStdString().c_str());
	LogV("%s %s[%d] temp_detect %s \n", __FILE__, __FUNCTION__, __LINE__, temp_detect.toStdString().c_str());
	LogV("%s %s[%d] irliveness_threshold %s \n", __FILE__, __FUNCTION__, __LINE__, irliveness_threshold.toStdString().c_str());
	LogV("%s %s[%d] recognition_compare_threshold %s \n", __FILE__, __FUNCTION__, __LINE__,
			recognition_compare_threshold.toStdString().c_str());
	LogV("%s %s[%d] identify_interval %s \n", __FILE__, __FUNCTION__, __LINE__, identify_interval.toStdString().c_str());

	Json::Value json;
	std::string timestamp;
	std::string password;
	timestamp = getTime();
	password = mHttpServerPassword.toStdString() + timestamp;
	password = md5sum(password);
	password = md5sum(password);
	transform(password.begin(), password.end(), password.begin(), ::tolower);
	json["msg_type"] = stMsgHeader.msg_type.c_str();
	json["sn"] = stMsgHeader.sn.c_str();
	json["timestamp"] = timestamp.c_str();
	json["password"] = password.c_str();
	json["cmd_id"] = stMsgHeader.cmd_id.c_str();
	json["result"] = "1";
	json["success"] = "1";

	ReadConfig::GetInstance()->setHomeDisplay_DeviceName(dev_name_str);
	ReadConfig::GetInstance()->setHomeDisplay_Location(location_str);

	must_open_mode.replace(",", "&");
	if (must_open_mode.endsWith("&"))
		must_open_mode = must_open_mode.left(must_open_mode.size() - 1);

	optional_open_mode.replace(",", "|");
	if (optional_open_mode.endsWith("|"))
		optional_open_mode = optional_open_mode.left(optional_open_mode.size() - 1);

	ReadConfig::GetInstance()->setDoor_MustOpenMode(must_open_mode);
	ReadConfig::GetInstance()->setDoor_OptionalOpenMode(optional_open_mode);

	if (visit_record.size() > 0)
	{
		ReadConfig::GetInstance()->setRecords_Manager_Stranger(visit_record.toInt());
	}
	if (irliveness_threshold.size() > 0)
	{
		ReadConfig::GetInstance()->setIdentity_Manager_Living_value(irliveness_threshold.toFloat());
	}
	if (recognition_compare_threshold.size() > 0)
	{
		ReadConfig::GetInstance()->setIdentity_Manager_Thanthreshold(recognition_compare_threshold.toFloat());
	}
	if(identify_interval.size() > 0)
	{
		ReadConfig::GetInstance()->setIdentity_Manager_IdentifyInterval(identify_interval.toInt());
	}

	QString ret = doPostJson(json);
	msg = "";
	if (ret.length() > 0)
	{
//			msg = ret;
	}
	ReadConfig::GetInstance()->setSaveConfig();
}

void ConnHttpServerThreadPrivate::doResetFactorySetting(Json::Value root, MESSAGE_HEADER_S stMsgHeader)
{
	Json::Value json;
	std::string timestamp;
	std::string password;
	timestamp = getTime();
	password = mHttpServerPassword.toStdString() + timestamp;
	password = md5sum(password);
	password = md5sum(password);
	transform(password.begin(), password.end(), password.begin(), ::tolower);
	json["msg_type"] = stMsgHeader.msg_type.c_str();
	json["sn"] = stMsgHeader.sn.c_str();
	json["timestamp"] = timestamp.c_str();
	json["password"] = password.c_str();
	json["cmd_id"] = stMsgHeader.cmd_id.c_str();
	json["result"] = "1";
	json["success"] = "1";

	QString ret = doPostJson(json);
	msg = "";
	if (ret.length() > 0)
	{
//			msg = ret;
	}

	YNH_LJX::RkUtils::Utils_ExecCmd("rm -rf /update/hi3516_update*");
	YNH_LJX::RkUtils::Utils_ExecCmd("rm -rf /mnt/user/*");
	YNH_LJX::RkUtils::Utils_ExecCmd("rm -rf /param/eth0-settings.txt");
	YNH_LJX::RkUtils::Utils_ExecCmd("rm -rf /userdata/");
	YNH_LJX::RkUtils::Utils_ExecCmd("sync");
	myHelper::Utils_Reboot();
}

void ConnHttpServerThreadPrivate::doSetAlgoParam(Json::Value root, MESSAGE_HEADER_S stMsgHeader)
{
	std::string irliveness_threshold = root["irliveness_threshold"].asString();
	std::string sim_threshold = root["sim_threshold"].asString();
	std::string id_threshold = root["id_threshold"].asString();
	std::string rgbliveness_threshold = root["rgbliveness_threshold"].asString();

	Json::Value json;
	std::string timestamp;
	std::string password;
	timestamp = getTime();
	password = mHttpServerPassword.toStdString() + timestamp;
	password = md5sum(password);
	password = md5sum(password);
	transform(password.begin(), password.end(), password.begin(), ::tolower);
	json["msg_type"] = stMsgHeader.msg_type.c_str();
	json["sn"] = stMsgHeader.sn.c_str();
	json["timestamp"] = timestamp.c_str();
	json["password"] = password.c_str();
	json["cmd_id"] = stMsgHeader.cmd_id.c_str();

	bool isUpdate = false;
	if (irliveness_threshold.length() > 0)
	{
		ReadConfig::GetInstance()->setIdentity_Manager_Living_value(atof(irliveness_threshold.c_str()));
		isUpdate = true;
	}
	if (rgbliveness_threshold.length() > 0)
	{
		ReadConfig::GetInstance()->setIdentity_Manager_FqThreshold(atof(rgbliveness_threshold.c_str()));
		isUpdate = true;
	}
	if (id_threshold.length() > 0)
	{
		ReadConfig::GetInstance()->setIdentity_Manager_Thanthreshold(atof(id_threshold.c_str()));
		isUpdate = true;
	}
	if (isUpdate)
	{
		json["result"] = "1";
		json["success"] = "1";
	} else
	{
		json["result"] = "0";
		json["success"] = "0";
	}
	QString ret = doPostJson(json);
	msg = "";
	if (ret.length() > 0)
	{
//			msg = ret;
	}
	myHelper::Utils_Reboot();
}

void ConnHttpServerThreadPrivate::doGetDeviceVersion(Json::Value root, MESSAGE_HEADER_S stMsgHeader)
{
	Json::Value json;
	std::string timestamp;
	std::string password;
	timestamp = getTime();
	password = mHttpServerPassword.toStdString() + timestamp;
	password = md5sum(password);
	password = md5sum(password);
	transform(password.begin(), password.end(), password.begin(), ::tolower);
	json["msg_type"] = stMsgHeader.msg_type.c_str();
	json["sn"] = stMsgHeader.sn.c_str();
	json["timestamp"] = timestamp.c_str();
	json["password"] = password.c_str();
	json["cmd_id"] = stMsgHeader.cmd_id.c_str();
	if (((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->getAlgoFaceInitState() == true)
	{
		json["active_info"] = "actived";
	} else
	{
		json["active_info"] = "not actived";
	}
	json["algo_ver"] = ISC_ALGO_VERSION;
	json["dev_sn"] = myHelper::getCpuSerial().toStdString().c_str();
	json["device_ver"] = ISC_VERSION;
	json["result"] = "1";
	json["success"] = "1";

	QString ret = doPostJson(json);
	msg = "";
	if (ret.length() > 0)
	{
//			msg = ret;
	}
}

void ConnHttpServerThreadPrivate::doTakePicture(Json::Value root, MESSAGE_HEADER_S stMsgHeader)
{
	Json::Value json;
	std::string timestamp;
	std::string password;
	timestamp = getTime();
	password = mHttpServerPassword.toStdString() + timestamp;
	password = md5sum(password);
	password = md5sum(password);
	transform(password.begin(), password.end(), password.begin(), ::tolower);
	json["msg_type"] = stMsgHeader.msg_type.c_str();
	json["sn"] = stMsgHeader.sn.c_str();
	json["timestamp"] = timestamp.c_str();
	json["password"] = password.c_str();
	json["cmd_id"] = stMsgHeader.cmd_id.c_str();

	const char *rgbPicture = "/mnt/user/catch_rgb_base64.raw";
	const char *irPicture = "/mnt/user/catch_ir_base64.raw";

	char *pRgbData = ISC_NULL;
	char *pIrData = ISC_NULL;
	int nRgbDataSize = 0;
	int nIrDataSize = 0;
	qXLApp->GetCameraManager()->takePhotos(&pRgbData, &nRgbDataSize, &pIrData, &nIrDataSize);

	if (nRgbDataSize > 0 && nIrDataSize > 0)
	{
		std::string rgbData_base64;
		rgbData_base64 = cereal::base64::encode((unsigned char*) pRgbData, nRgbDataSize);
		if (rgbData_base64.length() > 0)
		{
			unlink(rgbPicture);
			int fd = open(rgbPicture, O_WRONLY | O_CREAT, 0666);
			if (fd > 0)
			{
				write(fd, rgbData_base64.c_str(), rgbData_base64.length());
				close(fd);
			}
		}

		std::string irData_base64;
		irData_base64 = cereal::base64::encode((unsigned char*) pIrData, nIrDataSize);
		if (irData_base64.length() > 0)
		{
			unlink(irPicture);
			int fd = open(irPicture, O_WRONLY | O_CREAT, 0666);
			if (fd > 0)
			{
				write(fd, irData_base64.c_str(), irData_base64.length());
				close(fd);
			}
		}
	}

	json["result"] = "1";
	json["success"] = "1";
	json["message"] = "get Picture OK.";

	std::string ret = "";
	std::string jsonStr = "";
	std::string cmd = "/usr/bin/curl --connect-timeout 3 -s -X POST ";

	if (json.size() > 0)
	{
		Json::FastWriter fast_writer;
		jsonStr = fast_writer.write(json);
		jsonStr = jsonStr.substr(0, jsonStr.length() - 1);
		cmd += std::string("-F \'json=") + jsonStr + std::string("\' ");
	}

	if (!access(rgbPicture, F_OK))
	{
		cmd += " -F \"rgb_image=@" + std::string(rgbPicture) + "\" ";
	}
	if (!access(irPicture, F_OK))
	{
		cmd += " -F \"ir_image=@" + std::string(irPicture) + "\" ";
	}

	cmd += mHttpServerUrl.toStdString();
	LogD("%s %s[%d]  cmd : %s \n", __FILE__, __FUNCTION__, __LINE__, cmd.c_str());
	FILE *pFile = popen(cmd.c_str(), "r");
	if (pFile != ISC_NULL)
	{
		char buf[256] = { 0 };
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
	}
	LogD("%s %s[%d]  ret %s \n", __FILE__, __FUNCTION__, __LINE__, ret.c_str());

	if (!access(rgbPicture, F_OK))
	{
		unlink(rgbPicture);
	}
	if (!access(irPicture, F_OK))
	{
		unlink(irPicture);
	}

	doPostJson(json);
	msg = "";
}

void ConnHttpServerThreadPrivate::doSystemUpdate(Json::Value root, MESSAGE_HEADER_S stMsgHeader)
{
	bool isOK = false;
	std::string img = "";
	std::string path = root["path"].asString();
	std::string strMd5 = root["md5sum"].asString();
	std::string cmd = "rm -rf /update/*; rm -rf /mnt/user/update.zip; mkdir -p /update/lost+found;sync";
	YNH_LJX::RkUtils::Utils_ExecCmd(cmd.c_str());

	if (path.find("ftp://") != std::string::npos)
	{
		unsigned char TipMsg[128] = "正在下载固件...";
//		GUI::getInstance()->showTipsMsg(TipMsg);
		cmd = "cd /update; wget2 -c -nH -m --timeout=3 --ftp-user=admin --ftp-password=admin  " + path + " ; sync;";
		YNH_LJX::RkUtils::Utils_ExecCmd(cmd.c_str());

		isOK = checkFileMd5Sum("/update/update.zip", strMd5);
	} else if (path.find("http://") != std::string::npos)
	{
		unsigned char TipMsg[128] = "正在下载固件...";
//		GUI::getInstance()->showTipsMsg(TipMsg);
		cmd = "cd /update; /usr/bin/curl --connect-timeout 3  -o /mnt/user/update.zip  " + path + " ; sync;";
		YNH_LJX::RkUtils::Utils_ExecCmd(cmd.c_str());
		isOK = UsbObserver::GetInstance()->doCheckUpdateImage("/mnt/user/update.zip");
	}

	Json::Value json;
	std::string timestamp;
	std::string password;
	timestamp = getTime();
	password = mHttpServerPassword.toStdString() + timestamp;
	password = md5sum(password);
	password = md5sum(password);
	transform(password.begin(), password.end(), password.begin(), ::tolower);
	json["msg_type"] = stMsgHeader.msg_type.c_str();
	json["sn"] = stMsgHeader.sn.c_str();
	json["timestamp"] = timestamp.c_str();
	json["password"] = password.c_str();
	json["cmd_id"] = stMsgHeader.cmd_id.c_str();

	if (isOK == true)
	{
		json["result"] = "1";
		json["success"] = "1";
	} else
	{
		json["result"] = "0";
		json["success"] = "0";
		json["message"] = "md5 error";
	}

	QString ret = doPostJson(json);
	if (isOK == true)
	{
		UsbObserver::GetInstance()->doDeviceUpdate();
	}
	msg = "";
	if (ret.length() > 0)
	{
//			msg = ret;
	}
}

void ConnHttpServerThreadPrivate::doSetStaticLanIP(Json::Value root, MESSAGE_HEADER_S stMsgHeader)
{
	bool isOk = false;
	QString ip_address = QString::fromStdString(root["ip_address"].asString());
	QString ip_mask = QString::fromStdString(root["ip_mask"].asString());
	QString ip_gateway = QString::fromStdString(root["ip_gateway"].asString());
	QString ip_dns = QString::fromStdString(root["ip_dns"].asString());
	QString ip_mode = QString::fromStdString(root["ip_mode"].asString());

	LogD("%s %s[%d] ip_address %s \n", __FILE__, __FUNCTION__, __LINE__, ip_address.toStdString().c_str());
	LogD("%s %s[%d] ip_mask %s \n", __FILE__, __FUNCTION__, __LINE__, ip_mask.toStdString().c_str());
	LogD("%s %s[%d] ip_gateway %s \n", __FILE__, __FUNCTION__, __LINE__, ip_gateway.toStdString().c_str());
	LogD("%s %s[%d] ip_dns %s \n", __FILE__, __FUNCTION__, __LINE__, ip_dns.toStdString().c_str());
	LogD("%s %s[%d] ip_mode %s \n", __FILE__, __FUNCTION__, __LINE__, ip_mode.toStdString().c_str());
	if (ip_address.size() >= 7 && ip_mask.size() >= 7 && ip_gateway.size() >= 7)
	{
		isOk = true;
	}

	if (ip_mode != "static")
	{
		isOk = true;
	}

	Json::Value json;
	std::string timestamp;
	std::string password;
	timestamp = getTime();
	password = mHttpServerPassword.toStdString() + timestamp;
	password = md5sum(password);
	password = md5sum(password);
	transform(password.begin(), password.end(), password.begin(), ::tolower);
	json["msg_type"] = stMsgHeader.msg_type.c_str();
	json["sn"] = stMsgHeader.sn.c_str();
	json["timestamp"] = timestamp.c_str();
	json["password"] = password.c_str();
	json["cmd_id"] = stMsgHeader.cmd_id.c_str();

	if (isOk == true)
	{
		json["result"] = "1";
		json["success"] = "1";
	} else
	{
		json["result"] = "0";
		json["success"] = "0";
	}

	QString ret = doPostJson(json);
	msg = "";
	if (ret.length() > 0)
	{
//			msg = ret;
	}

	if (isOk == true)
	{
		if (ip_mode == "static")
		{
			NetworkControlThread::GetInstance()->setLinkLan(1, ip_address,ip_mask, ip_gateway,ip_dns);
			ReadConfig::GetInstance()->setLAN_IP(ip_address);
			ReadConfig::GetInstance()->setLAN_Maks(ip_mask);
			ReadConfig::GetInstance()->setLAN_Gateway(ip_gateway);
			ReadConfig::GetInstance()->setLAN_DNS(ip_dns);
			ReadConfig::GetInstance()->setSaveConfig();
		} else
		{
			char *json_str = ISC_NULL;
			json_str = dbserver_network_ipv4_set("eth0", "dhcp", ISC_NULL, ISC_NULL, ISC_NULL);
			LogD("%s %s[%d] dbserver_network_ipv4_set, %s\n", __FILE__, __FUNCTION__, __LINE__, json_str);
			if (json_str)
			{
				free(json_str);
			}
			YNH_LJX::RkUtils::Utils_ExecCmd("rm -rf /param/eth0-settings.txt; sync");
		}
		myHelper::Utils_Reboot();
	}
}

void ConnHttpServerThreadPrivate::doSetDeviceHttpPassword(Json::Value root, MESSAGE_HEADER_S stMsgHeader)
{
	bool isOk = false;
	std::string strNewpassword = root["new_password"].asString();

	LogD("%s %s[%d] strNewpassword %s \n", __FILE__, __FUNCTION__, __LINE__, strNewpassword.c_str());
	if (strNewpassword.length() > 1)
	{
		isOk = true;
	}

	Json::Value json;
	std::string timestamp;
	std::string password;
	timestamp = getTime();
	password = strNewpassword + timestamp;
	password = md5sum(password);
	password = md5sum(password);
	transform(password.begin(), password.end(), password.begin(), ::tolower);
	json["msg_type"] = stMsgHeader.msg_type.c_str();
	json["sn"] = stMsgHeader.sn.c_str();
	json["timestamp"] = timestamp.c_str();
	json["password"] = password.c_str();
	json["cmd_id"] = stMsgHeader.cmd_id.c_str();

	if (isOk == true)
	{
		json["result"] = "1";
		json["success"] = "1";
	} else
	{
		json["result"] = "0";
		json["success"] = "0";
	}

	QString ret = doPostJson(json);
	msg = "";
	if (ret.length() > 0)
	{
//			msg = ret;
	}

	if (isOk == true)
	{
		ReadConfig::GetInstance()->setSrv_Manager_Password(strNewpassword.c_str());
		ReadConfig::GetInstance()->setSaveConfig();
		myHelper::Utils_Reboot();
	}
}

void *SyncPersonsListThread(void* arg)
{
	ConnHttpServerThreadPrivate *thiz = (ConnHttpServerThreadPrivate*) arg;
	if (!access("/mnt/user/tmp/sync_person.list", F_OK))
	{
		std::ifstream fin("/mnt/user/tmp/sync_person.list");
		std::string str;
		std::string key;
		std::string value;
		bool person_info_end = false;
		Person_S stPerson = { 0 };

		std::string auth_type;
		std::string time_data;
		std::string time_range;
		std::string start_time;
		std::string end_time;

		unlink("/mnt/user/tmp/sync_person_result.list");
		std::ofstream fout("/mnt/user/tmp/sync_person_result.list");

		while (std::getline(fin, str))
		{
			person_info_end = false;
			key = str.substr(0, str.find_last_of("="));

			value = str.substr(str.find_last_of("=") + 1, str.size() - key.size() - 2);
//			printf("%s %s[%d] %s len %d %s len %d  \n", __FILE__, __FUNCTION__, __LINE__, key.c_str(), key.length(), value.c_str(), value.length());
			if (std::string("person_uuid") == key)
			{
				if (value.size() > 0)
				{
					int len = (sizeof(stPerson.szUUID) > value.length()) ? value.length() : sizeof(stPerson.szUUID);
					strncpy(stPerson.szUUID, value.c_str(), len);
				}
			} else if (std::string("card_no") == key)
			{
				if (value.size() > 0)
				{
					int len = (sizeof(stPerson.szICCardNum) > value.length()) ? value.length() : sizeof(stPerson.szICCardNum);
					strncpy(stPerson.szICCardNum, value.c_str(), len);
				}
			} else if (std::string("id_card_no") == key)
			{
				if (value.size() > 0)
				{
					int len = (sizeof(stPerson.szIDCardNum) > value.length()) ? value.length() : sizeof(stPerson.szIDCardNum);
					strncpy(stPerson.szIDCardNum, value.c_str(), len);
				}
			} else if (std::string("person_name") == key)
			{
				if (value.size() > 0)
				{
					int len = (sizeof(stPerson.szName) > value.length()) ? value.length() : sizeof(stPerson.szName);
					strncpy(stPerson.szName, value.c_str(), len);
				}

			} else if (std::string("person_type") == key)
			{
				if (value.size() > 0)
				{
					stPerson.nPersonType = atoi(value.c_str());
				} else
				{
					stPerson.nPersonType = 0;
				}

			} else if (std::string("group") == key)
			{
				if (value.size() > 0)
				{
					int len = (sizeof(stPerson.szGids) > value.length()) ? value.length() : sizeof(stPerson.szGids);
					strncpy(stPerson.szGids, value.c_str(), len);
				}

			} else if (std::string("male") == key)
			{
				if (value.size() > 0)
				{
					int len = (sizeof(stPerson.szSex) > value.length()) ? value.length() : sizeof(stPerson.szSex);
					strncpy(stPerson.szSex, value.c_str(), len);
				}

			} else if (std::string("face_img") == key)
			{
				if (value.size() > 0)
				{
					char person_img[256] = "/mnt/user/facedb/RegImage.jpeg";
					if (thiz->doDownloadFile(value.c_str(), person_img))
					{
						int len = (sizeof(stPerson.szImage) > strlen(person_img)) ? strlen(person_img) : sizeof(stPerson.szImage);
						strncpy(stPerson.szImage, person_img, len);
					} else
					{
						if (thiz->doDownloadFile(value.c_str(), person_img))
						{
							int len = (sizeof(stPerson.szImage) > strlen(person_img)) ? strlen(person_img) : sizeof(stPerson.szImage);
							strncpy(stPerson.szImage, person_img, len);
						} else
						{
							LogE("%s %s %d download the image secord filed,end the thread !", __FILE__, __FUNCTION__, __LINE__);
							break;
						}
					}
				}
			} else if (std::string("creat_time") == key)
			{
				if (value.size() > 0)
				{
					int len = (sizeof(stPerson.szCreateTime) > value.length()) ? value.length() : sizeof(stPerson.szCreateTime);
					strncpy(stPerson.szCreateTime, value.c_str(), len);
				}

			} else if (std::string("auth_type") == key)
			{
				if (value.size() > 0)
				{
					auth_type = value;
				}

			} else if (std::string("data") == key)
			{
				if (value.size() > 0)
				{
					time_data = value;
				}
			} else if (std::string("time_range") == key)
			{
				if (value.size() > 0)
				{
					time_range = value;
					std::vector<std::string> vect;
					char split_c = ';';
					vect = split(time_range, split_c);
					bool isOk = false;
					if (vect.size() == 2)
					{
						isOk = true;
					}
					if (isOk == true)
					{
						start_time = vect[0];
						end_time = vect[1];
					}
				}
			} else if (std::string("person_sync") == key)
			{
				person_info_end = true;
				if (std::string("add") == value)
				{
					if (RegisteredFacesDB::GetInstance()->GetPersonTotalNumByPersonUUIDFromRAM(stPerson.szUUID) != 0)
					{
						RegisteredFacesDB::GetInstance()->DelPersonByPersionUUIDFromDBAndRAM(stPerson.szUUID);
					}
					usleep(10 * 1000);
					int result = -1;
					int faceNum = 0;
					double threshold = 0;
					QByteArray faceFeature;
					result = ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->RegistPerson(stPerson.szImage, faceNum, threshold, faceFeature);
					if (result == 0)
					{
						QString timeOfAccess = "";
						QString timeRange = QString::fromStdString(time_range.c_str());
						QString timeData = QString::fromStdString(time_data.c_str());
						if (auth_type == "time_range")
						{
							timeOfAccess = timeRange;  //time_range = 2000/01/01 00:00;2000/01/01 00:00
						} else if (auth_type == "week_cycle")
						{
							//data = 1,2,3,4,5,6,7
							//time_range = 00:00;21:00
							timeRange.replace(QString(";"), QString(","));
							timeOfAccess = timeRange;

							QStringList sections = timeData.split(",");
							for (int i = 1; i <= 7; i++)
							{
								bool isSet = false;
								timeOfAccess += ",";
								for (int j = 0; j < sections.size(); j++)
								{
									if (sections.at(j) == QString::number(i))
									{
										timeOfAccess += "1";
										isSet = true;
										break;
									}
								}
								if (isSet == false)
								{
									timeOfAccess += "0";
								}
							}
						}

						bool isSaveDBOk = RegisteredFacesDB::GetInstance()->RegPersonToDBAndRAM(stPerson.szUUID, stPerson.szName,
								stPerson.szIDCardNum, stPerson.szICCardNum, stPerson.szSex, stPerson.szGids, timeOfAccess, faceFeature);
						LogD("%s %s[%d] RegPersonToDBAndRAM stPerson.szUUID %s isSaveDBOk %d \n", __FILE__, __FUNCTION__, __LINE__,
								stPerson.szUUID, isSaveDBOk);
						if (isSaveDBOk == true)
						{
							fout.write(stPerson.szUUID, strlen(stPerson.szUUID));
							fout.write("\n",1);
						}
					}
				} else if (std::string("remove") == value)
				{
					RegisteredFacesDB::GetInstance()->DelPersonByPersionUUIDFromDBAndRAM(stPerson.szUUID);
					fout.write(stPerson.szUUID, strlen(stPerson.szUUID));
					fout.write("\n",1);

				} else if (std::string("update") == value)
				{
					/**批量更新用 先删除，再录入的方式操作**/
					if (RegisteredFacesDB::GetInstance()->GetPersonTotalNumByPersonUUIDFromRAM(stPerson.szUUID) != 0)
					{
						RegisteredFacesDB::GetInstance()->DelPersonByPersionUUIDFromDBAndRAM(stPerson.szUUID);
					}

					usleep(500 * 1000);
					int result = -1;
					int faceNum = 0;
					double threshold = 0;
					QByteArray faceFeature;
					result = ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->RegistPerson(stPerson.szImage, faceNum, threshold, faceFeature);
					if (result == 0)
					{
						QString timeOfAccess = "";
						QString timeRange = QString::fromStdString(time_range.c_str());
						QString timeData = QString::fromStdString(time_data.c_str());
						if (auth_type == "time_range")
						{
							timeOfAccess = timeRange;  //time_range = 2000/01/01 00:00;2000/01/01 00:00
						} else if (auth_type == "week_cycle")
						{
							//data = 1,2,3,4,5,6,7
							//time_range = 00:00;21:00
							timeRange.replace(QString(";"), QString(","));
							timeOfAccess = timeRange;

							QStringList sections = timeData.split(",");
							for (int i = 1; i <= 7; i++)
							{
								bool isSet = false;
								timeOfAccess += ",";
								for (int j = 0; j < sections.size(); j++)
								{
									if (sections.at(j) == QString::number(i))
									{
										timeOfAccess += "1";
										isSet = true;
										break;
									}
								}
								if (isSet == false)
								{
									timeOfAccess += "0";
								}
							}
						}

						bool isSaveDBOk = RegisteredFacesDB::GetInstance()->RegPersonToDBAndRAM(stPerson.szUUID, stPerson.szName,
								stPerson.szIDCardNum, stPerson.szICCardNum, stPerson.szSex, stPerson.szGids, timeOfAccess, faceFeature);
						LogD("%s %s[%d] RegPersonToDBAndRAM isSaveDBOk %d \n", __FILE__, __FUNCTION__, __LINE__, isSaveDBOk);
						if (isSaveDBOk == true)
						{
							fout.write(stPerson.szUUID, strlen(stPerson.szUUID));
							fout.write("\n",1);
						}
					}
					/**批量更新用 先删除，再录入的方式操作 end**/

				}
				memset(&stPerson, 0, sizeof(Person_S));
			}
		}
		fin.close();
		fout.flush();
		fout.close();

		Json::Value json;
		std::string timestamp;
		std::string password;
		timestamp = getTime();
		password = thiz->mHttpServerPassword.toStdString() + timestamp;
		password = md5sum(password);
		password = md5sum(password);
		transform(password.begin(), password.end(), password.begin(), ::tolower);
		json["msg_type"] = "sync_persons_list_result";
		json["sn"] = thiz->sn.toStdString().c_str();
		json["timestamp"] = timestamp.c_str();
		json["password"] = password.c_str();

		json["result"] = "1";
		json["success"] = "1";
		json["message"] = "sync person result";

		std::string ret = "";
		std::string jsonStr = "";
		std::string cmd = "/usr/bin/curl --connect-timeout 3 -s -X POST ";

		if (json.size() > 0)
		{
			Json::FastWriter fast_writer;
			jsonStr = fast_writer.write(json);
			jsonStr = jsonStr.substr(0, jsonStr.length() - 1);
			cmd += std::string("-F \'json=") + jsonStr + std::string("\' ");
		}

		if (!access("/mnt/user/tmp/sync_person_result.list", F_OK))
		{
			cmd += " -F \"filename=@/mnt/user/tmp/sync_person_result.list\"  ";
		}
		cmd += thiz->mHttpServerUrl.toStdString();

		FILE *pFile = popen(cmd.c_str(), "r");
		if (pFile != ISC_NULL)
		{
			char buf[256] = { 0 };
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
		}
		//unlink("/mnt/user/tmp/sync_person_result.list");
	}
	return ISC_NULL;
}

void ConnHttpServerThreadPrivate::doSyncPersonsList(Json::Value root, MESSAGE_HEADER_S stMsgHeader)
{
	std::string person_list_url = root["person_list_url"].asString();

	LogD("%s %s[%d] person_list_url %s \n", __FILE__, __FUNCTION__, __LINE__, person_list_url.c_str());
	Json::Value json;
	std::string timestamp;
	std::string password;
	timestamp = getTime();
	password = mHttpServerPassword.toStdString() + timestamp;
	password = md5sum(password);
	password = md5sum(password);
	transform(password.begin(), password.end(), password.begin(), ::tolower);
	json["msg_type"] = stMsgHeader.msg_type.c_str();
	json["sn"] = stMsgHeader.sn.c_str();
	json["timestamp"] = timestamp.c_str();
	json["password"] = password.c_str();
	json["cmd_id"] = stMsgHeader.cmd_id.c_str();

	YNH_LJX::RkUtils::Utils_ExecCmd("mkdir -p /mnt/user/tmp/");
	YNH_LJX::RkUtils::Utils_ExecCmd("rm -rf /mnt/user/tmp/sync_person.list");
	if (doDownloadFile(person_list_url.c_str(), "/mnt/user/tmp/sync_person.list"))
	{
		json["result"] = "1";
		json["success"] = "1";
		pthread_t syncThreadId;
		pthread_create(&syncThreadId, ISC_NULL, SyncPersonsListThread, this);
	} else
	{
		json["result"] = "0";
		json["success"] = "0";
	}

	QString ret = doPostJson(json);
	if (ret.length() > 0)
	{
		msg = ret;
	}
}

void ConnHttpServerThreadPrivate::doSaveFile(Json::Value root, MESSAGE_HEADER_S stMsgHeader)
{
	bool isOk = false;
	std::string src_file = root["src_file"].asString();
	std::string dst_file = root["dst_file"].asString();

	LogD("%s %s[%d] src_file %s \n", __FILE__, __FUNCTION__, __LINE__, src_file.c_str());
	LogD("%s %s[%d] dst_file %s \n", __FILE__, __FUNCTION__, __LINE__, dst_file.c_str());
	if (src_file.length() > 1 && dst_file.length() > 1)
	{
		unlink(dst_file.c_str());
		doDownloadFile(src_file.c_str(), dst_file.c_str());
		if (!access(dst_file.c_str(), F_OK))
		{
			isOk = true;
		}
	}

	Json::Value json;
	std::string timestamp;
	std::string password;
	timestamp = getTime();
	password = mHttpServerPassword.toStdString() + timestamp;
	password = md5sum(password);
	password = md5sum(password);
	transform(password.begin(), password.end(), password.begin(), ::tolower);
	json["msg_type"] = stMsgHeader.msg_type.c_str();
	json["sn"] = stMsgHeader.sn.c_str();
	json["timestamp"] = timestamp.c_str();
	json["password"] = password.c_str();
	json["cmd_id"] = stMsgHeader.cmd_id.c_str();

	if (isOk == true)
	{
		json["result"] = "1";
		json["success"] = "1";
	} else
	{
		json["result"] = "0";
		json["success"] = "0";
	}

	QString ret = doPostJson(json);
	msg = "";
	if (ret.length() > 0)
	{
//			mstrMsg = ret;
	}
}

bool ConnHttpServerThreadPrivate::doNextMessage()
{

	if (msg.size() <= 10)
	{
		return false;
	}

	Json::Reader reader;
	Json::Value root;
	MESSAGE_HEADER_S stMsgHeader;
	stMsgHeader.cmd_id = "";
	stMsgHeader.msg_type = "";
	stMsgHeader.sn = "";
	stMsgHeader.timestamp = "";
	stMsgHeader.password = "";

	if (reader.parse(msg.toStdString(), root))
	{
		msg = "";
		stMsgHeader.msg_type = root["msg_type"].asString();
		stMsgHeader.sn = root["sn"].asString();
		stMsgHeader.timestamp = root["timestamp"].asString();
		stMsgHeader.password = root["password"].asString();
		stMsgHeader.cmd_id = root["cmd_id"].asString();

		std::string interval = root["interval"].asString();
		std::string time_difference = root["time_difference"].asString();
		if (interval.length() > 0)
		{
			int newDelay = atoi(interval.c_str());
			threadDelay = (newDelay < 30) ? 30 : newDelay;
		}
		if (time_difference.length() > 0)
		{
			int nTimeDifference = atoi(time_difference.c_str());
			if (nTimeDifference > 0)
			{
				tm stTime = { 0 };
				int year, month, day, hour, minute, second; //20200725182035  //YYYYMMDDHHMMSS

				const char *szTimestamp = stMsgHeader.timestamp.c_str();
				char szValue[4] = { 0 };
				memset(szValue, 0, sizeof(szValue));
				strncpy(szValue, (char*) szTimestamp, 4);
				year = atoi(szValue);
				stTime.tm_year = year - 1900;

				memset(szValue, 0, sizeof(szValue));
				strncpy(szValue, (char*) szTimestamp + 4, 2);
				month = atoi(szValue);
				stTime.tm_mon = month - 1;

				memset(szValue, 0, sizeof(szValue));
				strncpy(szValue, (char*) szTimestamp + 6, 2);
				day = atoi(szValue);
				stTime.tm_mday = day;

				memset(szValue, 0, sizeof(szValue));
				strncpy(szValue, (char*) szTimestamp + 8, 2);
				hour = atoi(szValue);
				stTime.tm_hour = hour;

				memset(szValue, 0, sizeof(szValue));
				strncpy(szValue, (char*) szTimestamp + 10, 2);
				minute = atoi(szValue);
				stTime.tm_min = minute;

				memset(szValue, 0, sizeof(szValue));
				strncpy(szValue, (char*) szTimestamp + 12, 2);
				second = atoi(szValue);
				stTime.tm_sec = second;

				stTime.tm_isdst = 0;
				time_t t1 = mktime(&stTime);
				time_t t2;
				time(&t2);
				double nDiff = difftime(t1, t2);
				if (nDiff < -nTimeDifference || nDiff > nTimeDifference)
				{
					char szNow[128] = { 0 };
					struct tm *p;
					p = localtime(&t2);
					sprintf(szNow, "%04d%02d%02d%02d%02d%02d", (1900 + p->tm_year), (1 + p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min,
							p->tm_sec);

					LogD("%s %s[%d]  stMsgHeader.timestamp %s \n", __FILE__, __FUNCTION__, __LINE__, stMsgHeader.timestamp.c_str());
					LogD("%s %s[%d]  t2 %s \n", __FILE__, __FUNCTION__, __LINE__, szNow);
					LogD("%s %s[%d]  t2(%lu) - t1(%lu) %f \n", __FILE__, __FUNCTION__, __LINE__, t2, t1, nDiff);

					stime(&t1);
					system("/sbin/hwclock -w -u");
				}
			}
		}
	}

	if (stMsgHeader.msg_type.length() < 2 || stMsgHeader.sn.length() < 2)
	{
		return false;
	}

	std::string password = mHttpServerPassword.toStdString() + stMsgHeader.timestamp;
	password = md5sum(password);
	password = md5sum(password);
	transform(password.begin(), password.end(), password.begin(), ::tolower);
	if (password != stMsgHeader.password && stMsgHeader.msg_type != "set_device_password")
	{
		LogE("%s %s[%d] error password , now password %s\n", __FILE__, __FUNCTION__, __LINE__, mHttpServerPassword.toStdString().c_str());
		Json::Value json;
		std::string timestamp;
		std::string password;
		timestamp = getTime();
		password = mHttpServerPassword.toStdString() + timestamp;
		password = md5sum(password);
		password = md5sum(password);
		transform(password.begin(), password.end(), password.begin(), ::tolower);
		json["msg_type"] = "ErrorPassword";
		json["sn"] = sn.toStdString().c_str();
		json["timestamp"] = timestamp.c_str();
		json["password"] = password.c_str();
		json["cmd_id"] = stMsgHeader.cmd_id.c_str();
		json["result"] = "0";
		json["success"] = "0";
		json["message"] = "error password";
		QString ret = doPostJson(json);
		if (ret.length() > 0)
		{
			msg = ret;
		}
		return false;
	}

	if (stMsgHeader.sn != sn.toStdString())
	{
		Json::Value json;
		std::string timestamp;
		std::string password;
		timestamp = getTime();
		password = mHttpServerPassword.toStdString() + timestamp;
		password = md5sum(password);
		password = md5sum(password);
		transform(password.begin(), password.end(), password.begin(), ::tolower);
		json["msg_type"] = "ErrorSN";
		json["sn"] = sn.toStdString().c_str();
		json["timestamp"] = timestamp.c_str();
		json["password"] = password.c_str();
		json["cmd_id"] = stMsgHeader.cmd_id.c_str();
		json["result"] = "0";
		json["success"] = "0";
		json["message"] = "error sn";
		QString ret = doPostJson(json);
		if (ret.length() > 0)
		{
			msg = ret;
		}
		return false;
	}

	if (stMsgHeader.msg_type == std::string("add") || stMsgHeader.msg_type == std::string("add_person")) //注册人员
	{
		doAddPerson(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("add_persons")) //批量注册人员
	{
		doAddPersons(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("add_personswithreason")) //批量注册人员 带原因
	{
		doAddPersonswithreason(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("delete") || stMsgHeader.msg_type == std::string("delete_person")) //删除人员
	{
		doDeletePerson(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("delete_all") || stMsgHeader.msg_type == std::string("delete_all_person")) //删除所有人员
	{
		doDeleteAllPerson(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("update") || stMsgHeader.msg_type == std::string("update_person")) //注册人员
	{
		doUpdatePerson(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("find") || stMsgHeader.msg_type == std::string("find_person")) //查询指定人员
	{
		doFindPerson(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("get_all") || stMsgHeader.msg_type == std::string("get_all_person")) //查询所有人员
	{
		doGetAllPerson(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("get_all_group")) //查询所有部门
	{

	} else if (stMsgHeader.msg_type == std::string("set_pass_permission")) //设置用户通行权限
	{
		doSetTimeOfAccess(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("delete_pass_permission")) //删除用户通行权限
	{
		doDeleteTimeOfAccess(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("device_delete_pass_permission")) //删除设备通行权限
	{

	} else if (stMsgHeader.msg_type == std::string("delete_all_record")) //删除所有识别记录
	{
		doDeleteAllPersonRecord(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("records_select")) //查询识别记录
	{
		doGetPersonRecord(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("records_select_done")) //facetools 定时查询记录后成功返回，需要修改数据库中记录状态
	{
		doUpdatePersonRecordUploadFlag(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("delete_record")) //删除指定识别记录
	{
		doDeletePersonRecord(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("open_door")) //开闸
	{
		doOpenDoor(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("restart")) //重启
	{
		doReboot(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("net_config")) //配置设备网络
	{
		doSetNetwork(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("get_dev_config")) //获取设备设置信息
	{
		doGetDeviceConfig(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("dev_config") || stMsgHeader.msg_type == std::string("set_dev_config")) //更改设备设置
	{
		doSetDeviceConfig(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("factory_setting")) //恢复默认设置
	{
		doResetFactorySetting(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("delete_all_user_data")) //删除所有用户数据，恢复到出厂状态
	{

	} else if (stMsgHeader.msg_type == std::string("algo_setting")) //设置算法参数
	{
		doSetAlgoParam(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("algo_reset")) //恢复默认算法
	{

	} else if (stMsgHeader.msg_type == std::string("dev_version")) //获取设备版本号
	{
		doGetDeviceVersion(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("get_picture")) //获取设备实时拍照图片
	{
		doTakePicture(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("system_update")) //ota 升级命令
	{
		doSystemUpdate(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("set_static_ip")) // 静态IP设置
	{
		doSetStaticLanIP(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("set_device_password")) //设置密码
	{
		doSetDeviceHttpPassword(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("save_file")) //文件保存到设备
	{
		doSaveFile(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("sync_persons_list")) //同步人员列表
	{
		doSyncPersonsList(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("set_device_httpserver"))
	{

	} else if (stMsgHeader.msg_type == std::string("set_timer")) // 时间设置
	{

	}

	if (msg.length() < 2)
	{
		return false;
	}

//假如没有返回 msg_type 为 idle，则服务器会有后续需要处理的消息
	if (stMsgHeader.msg_type != std::string("idle"))
	{
		return true;
	}
	return false;
}

ConnHttpServerThread::ConnHttpServerThread(QObject *parent) :
        QThread(parent), //
        d_ptr(new ConnHttpServerThreadPrivate(this))
{
    qDebug() << "THREAD_DEBUG: Initializing ConnHttpServerThread";
    
    // Check network configuration
    QString serverUrl = ReadConfig::GetInstance()->getPerson_Registration_Address();
    QString serverPassword = ReadConfig::GetInstance()->getPerson_Registration_Password();
    QString serverAddress = ReadConfig::GetInstance()->getSrv_Manager_Address();
    
    qDebug() << "THREAD_DEBUG: Registration Server URL: " << serverUrl;
    qDebug() << "THREAD_DEBUG: Registration Server Password length: " << serverPassword.length();
    qDebug() << "THREAD_DEBUG: Server Manager Address: " << serverAddress;
    
    // Check if network interfaces are up
    system("ifconfig > /tmp/ifconfig_debug.txt");
    system("ping -c 1 8.8.8.8 > /tmp/ping_debug.txt 2>&1");
    
    qDebug() << "THREAD_DEBUG: Network diagnostic info saved to /tmp/ifconfig_debug.txt and /tmp/ping_debug.txt";
    
    this->start();
    if(ReadConfig::GetInstance()->getPost_PersonRecord_Address().size() > 3)
    {
        qDebug() << "THREAD_DEBUG: Starting PostPersonRecordThread";
        PostPersonRecordThread::GetInstance();
    }
}

ConnHttpServerThread::~ConnHttpServerThread()
{
	Q_D(ConnHttpServerThread);
	this->requestInterruption();
	d->pauseCond.wakeOne();

	this->quit();
	this->wait();
}

void ConnHttpServerThread::run()
{
    Q_D(ConnHttpServerThread);
    qDebug() << "THREAD_DEBUG: ConnHttpServerThread starting...";
    sleep(5);
    
    while (true)
    {
        qDebug() << "THREAD_DEBUG: Thread loop iteration";
        d->sync.lock();
        
        QString oldUrl = d->mHttpServerUrl;
        QString oldPassword = d->mHttpServerPassword;
        QString oldSn = d->sn;
        
        d->mHttpServerUrl = ReadConfig::GetInstance()->getSrv_Manager_Address();
        d->mHttpServerPassword = ReadConfig::GetInstance()->getSrv_Manager_Password();
        d->sn = myHelper::getCpuSerial();
        
        // Log if values have changed
        if (oldUrl != d->mHttpServerUrl) {
            qDebug() << "THREAD_DEBUG: Server URL updated: " << d->mHttpServerUrl;
        }
        if (oldPassword != d->mHttpServerPassword) {
            qDebug() << "THREAD_DEBUG: Server password updated (length): " << d->mHttpServerPassword.length();
        }
        if (oldSn != d->sn) {
            qDebug() << "THREAD_DEBUG: SN updated: " << d->sn;
        }
        
        qDebug() << "THREAD_DEBUG: Calling doHeartbeat...";
        d->doHeartbeat();

        qDebug() << "THREAD_DEBUG: Processing messages...";
        while (d->doNextMessage())
        {
            qDebug() << "THREAD_DEBUG: Message processed, continuing...";
        }
        
        qDebug() << "THREAD_DEBUG: Waiting for " << d->threadDelay << " seconds...";
        d->pauseCond.wait(&d->sync, d->threadDelay * 1000);
        d->sync.unlock();
    }
}


bool ConnHttpServerThread::reportDeviceInfo()
{
	std::string result = "";
    char buf[64] = { 0 };

    std::string cmd = "/usr/bin/curl --connect-timeout 3 -s -X POST  '47.118.51.71:8211/device_report' ";
    std::string SN = myHelper::getCpuSerial().toStdString();
    std::string MAC = myHelper::GetNetworkMac().toStdString();
    std::string BAIDU_DEVICE_ID = myHelper::GetBaiduDeviceID().toStdString();
    std::string BAIDU_LICENSE_KEY = myHelper::GetBaiduLicenseKey().toStdString();
    std::string BAIDU_LICENSE_INI = myHelper::GetBaiduLicenseIni().toStdString();
    std::string VERSION = ISC_VERSION;
    std::string IPADDRESS = ISC_VERSION;

    if(SN.length() < 2 || MAC.length() < 2  || BAIDU_DEVICE_ID.length() < 2 )
    {
    	return false;
    }

    cmd += "--form 'SN=\""+SN+"\"' ";
    cmd += "--form 'MAC=\""+MAC.substr(0, MAC.length()-1)+"\"' ";
    cmd += "--form 'BAIDU_DEVICE_ID=\""+BAIDU_DEVICE_ID.substr(0, BAIDU_DEVICE_ID.length()-1)+"\"' ";
    cmd += "--form 'VERSION=\""+VERSION+"\"' ";
    if(BAIDU_LICENSE_KEY.length() > 5)
    {
    	cmd += "--form 'BAIDU_LICENSE_KEY=\""+BAIDU_LICENSE_KEY.substr(0, BAIDU_LICENSE_KEY.length()-1)+"\"' ";
    }
    if(BAIDU_LICENSE_INI.length() > 5)
    {
    	cmd += "--form 'BAIDU_LICENSE_INI=\""+BAIDU_LICENSE_INI.substr(0, BAIDU_LICENSE_INI.length()-1)+"\"' ";
    }
//    LogD("%s %s[%d] %s \n",__FILE__,__FUNCTION__,__LINE__,cmd.c_str());

	FILE *pFile = popen(cmd.c_str(), "r");
	if (pFile != ISC_NULL)
	{
		char buf[256] = { 0 };
		int readSize = 0;
		do
		{
			readSize = fread(buf, 1, sizeof(buf), pFile);
			if (readSize > 0)
			{
				result += std::string(buf, 0, readSize);
			}
		} while (readSize > 0);
		pclose(pFile);
	}
//	LogD("%s %s[%d] %s \n",__FILE__,__FUNCTION__,__LINE__,result.c_str());
	Json::Reader reader;
	Json::Value root;
	if (reader.parse(result, root))
	{
		if(root.isObject() && root.isMember("result"))
		{
			if(root["result"] == "1")
			{
				return true;
			}
		}
	}
	return false;
}



bool ConnHttpServerThread::sendUserToServer(const QString &person_uuid, const QString &name, 
    const QString &idCard, const QString &icCard, 
    const QString &sex, const QString ,
    const QString &timeOfAccess, 
    const QByteArray &faceFeature)
{
    Q_D(ConnHttpServerThread);
    qDebug() << "SEND_USER_DEBUG: Starting sendUserToServer function";
    
    QString regServerUrl = ReadConfig::GetInstance()->getPerson_Registration_Address();
    qDebug() << "SEND_USER_DEBUG: Server URL: " << regServerUrl;
    
    if (regServerUrl.isEmpty()) {
        qDebug() << "SEND_USER_DEBUG: Registration server URL is empty, canceling operation";
        return false;
    }

    Json::Value json;
    std::string timestamp = getTime();
    qDebug() << "SEND_USER_DEBUG: Generated timestamp: " << QString::fromStdString(timestamp);

    QString regPassword = ReadConfig::GetInstance()->getPerson_Registration_Password();
    qDebug() << "SEND_USER_DEBUG: Registration password length: " << regPassword.length();
    QString originalUrl = d->mHttpServerUrl;
    d->mHttpServerUrl = regServerUrl;
    std::string password = regPassword.toStdString() + timestamp;
    password = md5sum(password);
    password = md5sum(password);
    transform(password.begin(), password.end(), password.begin(), ::tolower);
    qDebug() << "SEND_USER_DEBUG: Generated password hash: " << QString::fromStdString(password);

    json["msg_type"] = "register_person";
    json["sn"] = d->sn.toStdString().c_str();
    json["timestamp"] = timestamp.c_str();
    json["password"] = password.c_str();

    json["person_uuid"] = person_uuid.toStdString().c_str();
    json["person_name"] = name.toStdString().c_str();
    json["id_card_no"] = idCard.toStdString().c_str();
    json["card_no"] = icCard.toStdString().c_str();
    json["male"] = sex.toStdString().c_str();
	json["time_of_access"] = timeOfAccess.toStdString().c_str();

    std::string base64Feature = cereal::base64::encode(
        (unsigned char*)faceFeature.data(), 
        faceFeature.size()
    );
    json["face_feature"] = base64Feature.c_str();
    
    qDebug() << "SEND_USER_DEBUG: Prepared JSON with user data for " << name;
    qDebug() << "SEND_USER_DEBUG: About to call doPostJson...";

    QString ret = d->doPostJson(json);
    
    qDebug() << "SEND_USER_DEBUG: doPostJson returned response length: " << ret.length();
    qDebug() << "SEND_USER_DEBUG: Response (first 100 chars): " << ret.left(100);
    d->mHttpServerUrl = originalUrl;
    return ret.length() > 0;
}


void ConnHttpServerThread::checkAndSyncUsers(const QString& heartbeatResponse)
{
    qDebug() << "DEBUG: checkAndSyncUsers - Starting with heartbeat response length: " << heartbeatResponse.length();
    
	ReadConfig readConfig;
    if (!readConfig.getSyncEnabled()) {
        LogD("%s %s[%d] Sync is disabled. Skipping user sync.\n", __FILE__, __FUNCTION__, __LINE__);
        return; // Do nothing if sync is disabled
    }

	LogD("%s %s[%d] Sync is enabled. Running user sync.\n", __FILE__, __FUNCTION__, __LINE__);
	
    // Parse the heartbeat response
    Json::Reader reader;
    Json::Value responseJson;
    
    if (!reader.parse(heartbeatResponse.toStdString(), responseJson)) {
        qDebug() << "ERROR: checkAndSyncUsers - Failed to parse heartbeat response JSON";
        return;
    }
    qDebug() << "DEBUG: checkAndSyncUsers - Successfully parsed heartbeat response JSON";
    
    // Check if the response has the person count in TotalUserCount
    int serverUserCount = 0;
    bool hasCount = false;
    
    if (responseJson.isMember("TotalUserCount")) {
        serverUserCount = atoi(responseJson["TotalUserCount"].asString().c_str());
        hasCount = true;
        qDebug() << "DEBUG: checkAndSyncUsers - Found TotalUserCount: " << serverUserCount;
    } else {
        qDebug() << "DEBUG: checkAndSyncUsers - No TotalUserCount found in response";
        
        // Dump response keys for debugging
        qDebug() << "DEBUG: Available keys in heartbeat response:";
        for (const auto& key : responseJson.getMemberNames()) {
            qDebug() << "  Key: " << QString::fromStdString(key);
        }
    }
    
    if (!hasCount) {
        qDebug() << "ERROR: checkAndSyncUsers - No user count found in heartbeat response, skipping sync";
        return;
    }
    
    // Get local user count
    QList<PERSONS_t> localUsers = RegisteredFacesDB::GetInstance()->GetAllPersonFromRAM();
    int localUserCount = localUsers.size();
    
    qDebug() << "DEBUG: checkAndSyncUsers - User counts - Server: " << serverUserCount << ", Local: " << localUserCount;
    
    // Check if we need to sync
    if (serverUserCount > localUserCount) {
        qDebug() << "DEBUG: checkAndSyncUsers - Detected more users on server, starting sync";
        syncMissingUsers();
    } else {
        qDebug() << "DEBUG: checkAndSyncUsers - User counts match or local has more, no sync needed";
    }
}
void ConnHttpServerThread::syncMissingUsers()
{
    Q_D(ConnHttpServerThread);
    qDebug() << "DEBUG: syncMissingUsers - Starting user synchronization process";
    
    // Save original URL
    QString originalUrl = d->mHttpServerUrl;
    qDebug() << "DEBUG: syncMissingUsers - Original URL: " << originalUrl;
    
    // Get the sync URL from config
    QString syncUrl = ReadConfig::GetInstance()->getSyncUsersAddress();
    if (syncUrl.isEmpty()) {
        qDebug() << "ERROR: syncMissingUsers - Sync URL not configured";
        return;
    }
    qDebug() << "DEBUG: syncMissingUsers - Using sync URL: " << syncUrl;
    
    // Set URL for this request
    d->mHttpServerUrl = syncUrl;
    
    // Prepare request
    Json::Value json;
    std::string timestamp = getTime();
    std::string password = ReadConfig::GetInstance()->getSyncUsersPassword().toStdString() + timestamp;
    password = md5sum(password);
    password = md5sum(password);
    transform(password.begin(), password.end(), password.begin(), ::tolower);
    
    json["msg_type"] = "get_all_person_ids";
    json["sn"] = d->sn.toStdString().c_str();
    json["timestamp"] = timestamp.c_str();
    json["password"] = password.c_str();
    
    qDebug() << "DEBUG: syncMissingUsers - Sending request for all user IDs";
    
    // Send request
    QString response = d->doPostJson(json);
    qDebug() << "DEBUG: syncMissingUsers - Received response, length: " << response.length();
    
    // Restore original URL
    d->mHttpServerUrl = originalUrl;
    qDebug() << "DEBUG: syncMissingUsers - Restored original URL";
    
    // Process response
    Json::Reader reader;
    Json::Value responseJson;
    if (!reader.parse(response.toStdString(), responseJson)) {
        qDebug() << "ERROR: syncMissingUsers - Failed to parse user IDs response";
        return;
    }
    qDebug() << "DEBUG: syncMissingUsers - Successfully parsed JSON response";
    
    // Extract user ID card numbers from the All_UserID array
    QStringList serverUserIdCards;
    if (responseJson.isMember("All_UserID") && responseJson["All_UserID"].isArray()) {
        Json::Value idCards = responseJson["All_UserID"];
        qDebug() << "DEBUG: syncMissingUsers - Processing All_UserID array with " << idCards.size() << " entries";
        for (Json::Value::ArrayIndex i = 0; i < idCards.size(); i++) {
            QString idCard = QString::fromStdString(idCards[i].asString());
            serverUserIdCards.append(idCard);
            qDebug() << "DEBUG: syncMissingUsers - Added ID card: " << idCard;
        }
    } else {
        qDebug() << "ERROR: syncMissingUsers - No All_UserID array found in response";
        // Dump response keys for debugging
        qDebug() << "DEBUG: Available keys in response:";
        for (const auto& key : responseJson.getMemberNames()) {
            qDebug() << "  Key: " << QString::fromStdString(key);
        }
        return;
    }
    
    // Get local user ID cards
    QList<PERSONS_t> localUsers = RegisteredFacesDB::GetInstance()->GetAllPersonFromRAM();
    QStringList localUserIdCards;
    
    qDebug() << "DEBUG: syncMissingUsers - Processing " << localUsers.size() << " local users";
    for (const PERSONS_t& person : localUsers) {
        localUserIdCards.append(person.idcard);
        qDebug() << "DEBUG: syncMissingUsers - Local user: " << person.name << ", ID: " << person.idcard;
    }
    
    // Find missing users
    QStringList missingUserIdCards;
    for (const QString& idCard : serverUserIdCards) {
        if (!localUserIdCards.contains(idCard)) {
            missingUserIdCards.append(idCard);
            qDebug() << "DEBUG: syncMissingUsers - Found missing user with ID: " << idCard;
        }
    }
    
    qDebug() << "DEBUG: syncMissingUsers - Found " << missingUserIdCards.size() << " users missing from local database";
    
    // Fetch and add each missing user
    for (const QString& idCard : missingUserIdCards) {
        qDebug() << "DEBUG: syncMissingUsers - Fetching details for user with ID: " << idCard;
        fetchAndAddUser(idCard);
    }
    
    qDebug() << "DEBUG: syncMissingUsers - User synchronization completed";
}

void ConnHttpServerThread::fetchAndAddUser(const QString& idCard)
{
    Q_D(ConnHttpServerThread);
    qDebug() << "DEBUG: fetchAndAddUser - Starting to fetch user with ID: " << idCard;
    
    // Save original URL
    QString originalUrl = d->mHttpServerUrl;
    qDebug() << "DEBUG: fetchAndAddUser - Original URL: " << originalUrl;
    
    // Get the user detail URL from config
    QString userDetailUrl = ReadConfig::GetInstance()->getUserDetailAddress();
    if (userDetailUrl.isEmpty()) {
        qDebug() << "ERROR: fetchAndAddUser - User detail URL not configured";
        return;
    }
    qDebug() << "DEBUG: fetchAndAddUser - Using user detail URL: " << userDetailUrl;
    
    // Set URL for this request
    d->mHttpServerUrl = userDetailUrl;
    
    // Prepare request
    Json::Value json;
    std::string timestamp = getTime();
    std::string password = ReadConfig::GetInstance()->getUserDetailPassword().toStdString() + timestamp;
    password = md5sum(password);
    password = md5sum(password);
    transform(password.begin(), password.end(), password.begin(), ::tolower);
    
    json["msg_type"] = "get_person_detail";
    json["sn"] = d->sn.toStdString().c_str();
    json["timestamp"] = timestamp.c_str();
    json["password"] = password.c_str();
    json["id_card_no"] = idCard.toStdString().c_str(); // Using id_card_no as identifier
    
    qDebug() << "DEBUG: fetchAndAddUser - Sending request for user details";
    
    // Send request
    QString response = d->doPostJson(json);
    qDebug() << "DEBUG: fetchAndAddUser - Received response, length: " << response.length();
    
    // Restore original URL
    d->mHttpServerUrl = originalUrl;
    qDebug() << "DEBUG: fetchAndAddUser - Restored original URL";
    
    // Process response
    Json::Reader reader;
    Json::Value responseJson;
    if (!reader.parse(response.toStdString(), responseJson)) {
        qDebug() << "ERROR: fetchAndAddUser - Failed to parse user detail response for ID Card: " << idCard;
        return;
    }
    qDebug() << "DEBUG: fetchAndAddUser - Successfully parsed JSON response";
    
    // Check for proper response structure
    if (!responseJson.isMember("allEmployeeData")) {
        qDebug() << "ERROR: fetchAndAddUser - Response doesn't contain allEmployeeData field";
        // Dump response keys for debugging
        qDebug() << "DEBUG: Available keys in response:";
        for (const auto& key : responseJson.getMemberNames()) {
            qDebug() << "  Key: " << QString::fromStdString(key);
        }
        return;
    }
    
    // Extract user data
    Json::Value userData = responseJson["allEmployeeData"];
    qDebug() << "DEBUG: fetchAndAddUser - Processing allEmployeeData";
    
    // Initialize variables with default values per database requirements
    QString person_uuid = ""; // Will be created by database if empty
    QString person_name = "";
    QString id_card_no = idCard; // We already have this as input parameter
    QString card_no = "000000"; // Database default
    QString male = "Unknown"; // Database default
    QString time_of_access = "00:00,23:59,1,1,1,1,1,1,1"; // 24/7 access
    QByteArray faceFeature;
    
    // Extract employeeId as person_uuid if available
    if (userData.isMember("assignedTo") && userData["assignedTo"].isMember("employeeId")) {
        person_uuid = QString::fromStdString(userData["assignedTo"]["employeeId"].asString());
        qDebug() << "DEBUG: fetchAndAddUser - Found employee ID as UUID: " << person_uuid;
    }
    
    // Extract first_name as person_name if available
    if (userData.isMember("assignedTo") && userData["assignedTo"].isMember("assignedUser") && 
        userData["assignedTo"]["assignedUser"].isMember("first_name")) {
        person_name = QString::fromStdString(userData["assignedTo"]["assignedUser"]["first_name"].asString());
        qDebug() << "DEBUG: fetchAndAddUser - Found name: " << person_name;
    }
    
    // Extract gender/sex if available
    if (userData.isMember("assignedTo") && userData["assignedTo"].isMember("assignedUser") && 
        userData["assignedTo"]["assignedUser"].isMember("gender") && 
        !userData["assignedTo"]["assignedUser"]["gender"].isNull()) {
        male = QString::fromStdString(userData["assignedTo"]["assignedUser"]["gender"].asString());
        qDebug() << "DEBUG: fetchAndAddUser - Found gender: " << male;
    }
    
    // Extract card_no from response if available
    // This would depend on your server structure - adjust as needed
    if (userData.isMember("assignedTo") && userData["assignedTo"].isMember("cardNo") && 
        !userData["assignedTo"]["assignedUser"]["cardNo"].isNull()) {
        card_no = QString::fromStdString(userData["assignedTo"]["assignedUser"]["cardNo"].asString());
        qDebug() << "DEBUG: fetchAndAddUser - Found card number: " << card_no;
    }
    
 
	
    
    // Extract time_of_access from response if available
    // This would depend on your server structure - adjust as needed
    if (userData.isMember("timeOfAccess") && !userData["timeOfAccess"].isNull()) {
        time_of_access = QString::fromStdString(userData["timeOfAccess"].asString());
        qDebug() << "DEBUG: fetchAndAddUser - Found time of access: " << time_of_access;
    }
    
    // Extract face feature from base64Data
    if (userData.isMember("base64Data") && !userData["base64Data"].isNull()) {
        std::string base64Feature = userData["base64Data"].asString();
        qDebug() << "DEBUG: fetchAndAddUser - Found base64 face data, length: " << base64Feature.length();
        
        try {
            // For face images in JPEG format
            if (base64Feature.substr(0, 10).find("JFIF") != std::string::npos || 
                base64Feature.substr(0, 10).find("/9j/") != std::string::npos) {
                // Save base64 as image and extract features
                QString tempPath = "/mnt/user/facedb/RegImage.jpeg";
                QFile file(tempPath);
                if (file.open(QIODevice::WriteOnly)) {
                    QByteArray base64Data = QByteArray::fromStdString(base64Feature);
                    QByteArray imageData = QByteArray::fromBase64(base64Data);
                    file.write(imageData);
                    file.close();
                    
                    qDebug() << "DEBUG: fetchAndAddUser - Saved base64 data as image, extracting features";
                    
                    // Extract face feature from the saved image
                    int faceNum = 0;
                    double threshold = 0;
                    int ret = ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->RegistPerson(
                        tempPath, faceNum, threshold, faceFeature);
                        
                    if (ret != 0) {
                        qDebug() << "ERROR: fetchAndAddUser - Failed to extract face feature from image: " << ret;
                        return;
                    }
                    qDebug() << "DEBUG: fetchAndAddUser - Successfully extracted face feature from image";
                } else {
                    qDebug() << "ERROR: fetchAndAddUser - Failed to save face image";
                    return;
                }
            } else {
                // Assume it's already a binary feature
                std::string binaryFeature = cereal::base64::decode(base64Feature);
                faceFeature = QByteArray(binaryFeature.data(), binaryFeature.size());
                qDebug() << "DEBUG: fetchAndAddUser - Decoded face feature, size: " << faceFeature.size();
            }
        } catch (const std::exception& e) {
            qDebug() << "ERROR: fetchAndAddUser - Failed to decode base64 data: " << e.what();
            return;
        }
    } else {
        qDebug() << "ERROR: fetchAndAddUser - No face data found";
        return;
    }
    
    // Set default name if none was found
    if (person_name.isEmpty()) {
        person_name = "User_" + id_card_no;
        qDebug() << "DEBUG: fetchAndAddUser - Setting default name: " << person_name;
    }
    
    // Add the user to local database
    qDebug() << "DEBUG: fetchAndAddUser - Adding user to database with the following details:";
    qDebug() << "  UUID: " << person_uuid;
    qDebug() << "  Name: " << person_name;
    qDebug() << "  ID Card: " << id_card_no;
    qDebug() << "  Card No: " << card_no;
    qDebug() << "  Gender: " << male;
    qDebug() << "  Time Access: " << time_of_access;
    qDebug() << "  Face Feature Size: " << faceFeature.size();
    
    bool success = RegisteredFacesDB::GetInstance()->RegPersonToDBAndRAM(
        person_uuid, 
        person_name, 
        id_card_no, 
        card_no, 
        male, 
        "", 
        time_of_access, 
        faceFeature);
        
    if (success) {
        qDebug() << "DEBUG: fetchAndAddUser - Successfully added user to local database: " << person_name << " (ID Card: " << id_card_no << ")";
    } else {
        qDebug() << "ERROR: fetchAndAddUser - Failed to add user to local database: " << id_card_no;
    }
}

void ConnHttpServerThread::setLastHeartbeatResponse(const QString& response)
{
    m_lastHeartbeatResponse = response;
}

QString ConnHttpServerThread::getLastHeartbeatResponse() const
{
    return m_lastHeartbeatResponse;
}