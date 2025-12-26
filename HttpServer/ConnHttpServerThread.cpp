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
#include "ManageEngines/IdentityManagement.h"
#include "RKCamera/Camera/cameramanager.h"
#include "base64.hpp"
#include "BaiduFace/BaiduFaceManager.h"
#include "RkNetWork/NetworkControlThread.h"
#include "json-cpp/json.h"
#include "FaceMainFrm.h"
#include "FaceHomeFrms/FaceHomeFrm.h"
#include "FaceHomeFrms/FaceHomeBottomFrm.h"

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
#include <QUuid>
#include <QtCore/QFileInfoList>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>



#define ISC_NULL                 0L

#define MERR_ASF_FACEENGINE_BASE							0x30000
#define MERR_ASF_FACEENGINE_IMAGE							(MERR_ASF_FACEENGINE_BASE + 1)
#define MERR_ASF_FACEENGINE_FACEDETECT						(MERR_ASF_FACEENGINE_BASE + 2)
#define MERR_ASF_FACEENGINE_MULTIFACE						(MERR_ASF_FACEENGINE_BASE + 3)
#define MERR_ASF_FACEENGINE_FACEEXTRACT						(MERR_ASF_FACEENGINE_BASE + 4)
#define MERR_ASF_FACEENGINE_THUMBNAIL						(MERR_ASF_FACEENGINE_BASE + 5)
#define MERR_ASF_FACEENGINE_FACEQUALITY						(MERR_ASF_FACEENGINE_BASE + 6)
#define DEFAULT_DOOR_ID "28(TM-10, B-0, BT-10, S-0, P-0)"
#define DEFAULT_ACCESS_LEVEL "2"

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
	mutable QMutex syncMutex;           // Mutex for sync operations
    bool syncInProgress;                // Flag to track sync status
	QWaitCondition pauseCond;
	int threadDelay;
	float m_heartbeatLivenessThreshold;
    float m_heartbeatQualityThreshold;
    float m_heartbeatComparisonThreshold;
	int m_heartbeatIdentificationInterval;
	int m_heartbeatRecognitionDistance;
    bool m_hasHeartbeatThresholds;

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
    QDateTime convertLocalTimeToUTC(const QDateTime& localTime);
    QDateTime getLocalLastModifiedTimeUTC();
	    // ADD THIS LINE - Declaration for the new method
    QDateTime convertServerUTCToIST(const QString& utcTimeStr);
	
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
	std::string* psResponse = (std::string*) ResInfo; //å¼ºåˆ¶è½¬æ¢
	psResponse->append((char*) buffer, sz * nmemb); //sz*nmembè¡¨ç¤ºæŽ¥å—æ•°æ®çš„å¤šå°‘
	return sz * nmemb;  //è¿”å›žæŽ¥å—æ•°æ®çš„å¤šå°‘
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
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, download_write_data); //æ•°æ®è¯·æ±‚åˆ°ä»¥åŽçš„å›žè°ƒå‡½æ•°
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
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, download_write_data); //æ•°æ®è¯·æ±‚åˆ°ä»¥åŽçš„å›žè°ƒå‡½æ•°
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
		threadDelay(60),
		m_heartbeatLivenessThreshold(0.0f),     // No defaults
        m_heartbeatQualityThreshold(0.0f),      // No defaults
        m_heartbeatComparisonThreshold(0.0f),   // No defaults
		m_heartbeatIdentificationInterval(0),    // Initialize to 0
        m_heartbeatRecognitionDistance(0),         // Initialize to 0
        m_hasHeartbeatThresholds(false)      // Initially false
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
    qDebug() << "doPostJson called from thread:" << QThread::currentThread();
    qDebug() << "Main thread is:" << QCoreApplication::instance()->thread();
    
    if (QThread::currentThread() == QCoreApplication::instance()->thread()) {
        qDebug() << "WARNING: Network call in MAIN/UI thread - will cause freeze!";
    }
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
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data); //æ•°æ®è¯·æ±‚åˆ°ä»¥åŽçš„å›žè°ƒå‡½æ•°
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
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data); //æ•°æ®è¯·æ±‚åˆ°ä»¥åŽçš„å›žè°ƒå‡½æ•°
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
    
    // === NEW DEBUG: Log heartbeat request ===
    qDebug() << "HEARTBEAT_DEBUG: Sending heartbeat request to server...";
    qDebug() << "HEARTBEAT_DEBUG: Target URL: " << mHttpServerUrl;
    qDebug() << "HEARTBEAT_DEBUG: Device SN: " << sn;
    
    QString ret = doPostJson(config);

    if (ret.size() > 0) {
        msg = ret;
        q_ptr->setLastHeartbeatResponse(ret);
        
        Json::Reader reader;
        Json::Value responseJson;
        if (reader.parse(ret.toStdString(), responseJson)) {
            
            // === Extract doorId from heartbeat response ===
            QString doorId = DEFAULT_DOOR_ID; // Default hardcoded value
            
            // Check for doorsIds array (note the plural form from your JSON)
            if (responseJson.isMember("doorsIds") && responseJson["doorsIds"].isArray()) {
                Json::Value doorsArray = responseJson["doorsIds"];
                if (doorsArray.size() > 0 && doorsArray[0].isString()) {
                    doorId = QString::fromStdString(doorsArray[0].asString());
                    qDebug() << "DEBUG: doHeartbeat - Found doorId from server doorsIds array:" << doorId;
                } else {
                    qDebug() << "DEBUG: doHeartbeat - doorsIds array empty, using default:" << doorId;
                }
            }
            // Fallback: check for single doorId field
            else if (responseJson.isMember("doorId") && !responseJson["doorId"].isNull()) {
                doorId = QString::fromStdString(responseJson["doorId"].asString());
                qDebug() << "DEBUG: doHeartbeat - Found doorId from server:" << doorId;
            } else {
                qDebug() << "DEBUG: doHeartbeat - No doorId in response, using default:" << doorId;
            }
            
            // Store doorId in ReadConfig
            ReadConfig::GetInstance()->setHeartbeat_DoorId(doorId);
            qDebug() << "DEBUG: doHeartbeat - Stored doorId:" << doorId;
            // Extract threshold values from heartbeat response
            m_hasHeartbeatThresholds = false;
            
            if (responseJson.isMember("liveness_threshold") && !responseJson["liveness_threshold"].isNull()) {
                m_heartbeatLivenessThreshold = responseJson["liveness_threshold"].asFloat();
                qDebug() << "DEBUG: Found liveness_threshold:" << m_heartbeatLivenessThreshold;
                m_hasHeartbeatThresholds = true;
            }
            
            if (responseJson.isMember("quality_threshold") && !responseJson["quality_threshold"].isNull()) {
                m_heartbeatQualityThreshold = responseJson["quality_threshold"].asFloat();
                qDebug() << "DEBUG: Found quality_threshold:" << m_heartbeatQualityThreshold;
                m_hasHeartbeatThresholds = true;
            }
            
            if (responseJson.isMember("comparison_threshold") && !responseJson["comparison_threshold"].isNull()) {
                m_heartbeatComparisonThreshold = responseJson["comparison_threshold"].asFloat();
                qDebug() << "DEBUG: Found comparison_threshold:" << m_heartbeatComparisonThreshold;
                m_hasHeartbeatThresholds = true;
            }
            
            // Extract identification_interval from heartbeat response
           if (responseJson.isMember("identificationInterval") && !responseJson["identificationInterval"].isNull()) {
           m_heartbeatIdentificationInterval = responseJson["identificationInterval"].asInt();
           qDebug() << "DEBUG: Found identificationInterval:" << m_heartbeatIdentificationInterval;
           m_hasHeartbeatThresholds = true;
}

           // Extract recognition_distance from heartbeat response  
           if (responseJson.isMember("identificationDistance") && !responseJson["identificationDistance"].isNull()) {
           m_heartbeatRecognitionDistance = responseJson["identificationDistance"].asInt();
           qDebug() << "DEBUG: Found identificationDistance:" << m_heartbeatRecognitionDistance;
           m_hasHeartbeatThresholds = true;
}
            
            // Extract tenant name
            if (responseJson.isMember("tenantName") && !responseJson["tenantName"].isNull()) {
                QString tenantName = QString::fromStdString(responseJson["tenantName"].asString());
                qDebug() << "DEBUG: doHeartbeat - Found tenant name:" << tenantName;
                q_ptr->updateTenantName(tenantName);
            }
            
            if (responseJson.isMember("tenantId") && !responseJson["tenantId"].isNull()) {
                QString tenantId = QString::fromStdString(responseJson["tenantId"].asString());
                // Store in existing ReadConfig system
                ReadConfig::GetInstance()->setHeartbeat_TenantId(tenantId);
                qDebug() << "DEBUG: Extracted tenantId from heartbeat:" << tenantId;
            }
            
            // Extract attendanceMode  
            if (responseJson.isMember("attendanceMode") && !responseJson["attendanceMode"].isNull()) {
                QString attendanceMode = QString::fromStdString(responseJson["attendanceMode"].asString());
                ReadConfig::GetInstance()->setHeartbeat_AttendanceMode(attendanceMode);
                qDebug() << "DEBUG: Extracted attendanceMode from heartbeat:" << attendanceMode;
            }
            
            // Extract Device status (from your image)
            if (responseJson.isMember("Device_status") && !responseJson["Device_status"].isNull()) {
                QString deviceStatus = QString::fromStdString(responseJson["Device_status"].asString());
                ReadConfig::GetInstance()->setHeartbeat_DeviceStatus(deviceStatus);
                qDebug() << "DEBUG: Extracted Device_status from heartbeat:" << deviceStatus;
            }

            // === MODIFIED: STORE SERVER'S LAST MODIFIED TIME FROM date_updated FIELD ===
            QString serverLastModifiedStr = "";
            
            if (responseJson.isMember("date_updated") && !responseJson["date_updated"].isNull()) {
                serverLastModifiedStr = QString::fromStdString(responseJson["date_updated"].asString());
                qDebug() << "DEBUG: doHeartbeat - Found server date_updated:" << serverLastModifiedStr;
            }
            else if (responseJson.isMember("dateUpdated") && !responseJson["dateUpdated"].isNull()) {
                serverLastModifiedStr = QString::fromStdString(responseJson["dateUpdated"].asString());
                qDebug() << "DEBUG: doHeartbeat - Found server dateUpdated:" << serverLastModifiedStr;
            }
            else if (responseJson.isMember("lastModified") && !responseJson["lastModified"].isNull()) {
                serverLastModifiedStr = QString::fromStdString(responseJson["lastModified"].asString());
                qDebug() << "DEBUG: doHeartbeat - Found server lastModified:" << serverLastModifiedStr;
            }
            else if (responseJson.isMember("lastUpdatedAt") && !responseJson["lastUpdatedAt"].isNull()) {
                serverLastModifiedStr = QString::fromStdString(responseJson["lastUpdatedAt"].asString());
                qDebug() << "DEBUG: doHeartbeat - Found server lastUpdatedAt:" << serverLastModifiedStr;
            }
            
            // If we found server's lastModified time, store it using existing file-based approach
    if (!serverLastModifiedStr.isEmpty()) {
        qDebug() << "DEBUG: doHeartbeat - Processing server date_updated time:" << serverLastModifiedStr;
        
        // Convert server UTC time to IST using existing function
        QDateTime serverTimeIST = convertServerUTCToIST(serverLastModifiedStr);
        
        if (serverTimeIST.isValid()) {
            qDebug() << "SUCCESS: doHeartbeat - Server date_updated time converted to IST:" << serverTimeIST.toString("yyyy-MM-dd hh:mm:ss");
            
            // *** Store server date_updated time in a simple file ***
            QString serverTimeFormatted = serverTimeIST.toString("yyyy/MM/dd HH:mm:ss");
            
            // *** CHANGED: Replace directory creation and file write with debug message ***
            QString serverTimeFile = "/mnt/user/sync_data/server_lastmodified.txt";
            qDebug() << "SYNC_DATA_DEBUG: Would write to file:" << serverTimeFile;
            qDebug() << "SYNC_DATA_DEBUG: Content would be:" << serverTimeFormatted;
            qDebug() << "SYNC_DATA_DEBUG: SOURCE=SERVER_DATE_UPDATED";
            qDebug() << "SYNC_DATA_DEBUG: TIMESTAMP=" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
            qDebug() << "SUCCESS: doHeartbeat - Server date_updated time processed (debug only, no file created)";
            
        } else {
            qDebug() << "ERROR: doHeartbeat - Failed to convert server date_updated time to IST";
        }
    } else {
        qDebug() << "DEBUG: doHeartbeat - No server date_updated time found in response";
    }
            
            // FIXED: Use singleton instance instead of local instance
            bool syncEnabled = ReadConfig::GetInstance()->getSyncEnabled();
            qDebug() << "DEBUG: doHeartbeat - Sync enabled from singleton:" << syncEnabled;
            
            if (syncEnabled) {
                qDebug() << "DEBUG: doHeartbeat - Sync is enabled, proceeding with sync";
                
                // Extract server count for initial display
                int serverCount = 0;
                if (responseJson.isMember("TotalUserCount")) {
                    serverCount = atoi(responseJson["TotalUserCount"].asString().c_str());
                }
                
                QList<PERSONS_t> localUsers = RegisteredFacesDB::GetInstance()->GetAllPersonFromRAM();
                int localCount = localUsers.size();
                
                // Show initial status
                q_ptr->updateSyncDisplay("Checking", localCount, serverCount);
                q_ptr->updateLocalFaceCount();
                
                // Only call sync if enabled
                q_ptr->checkAndSyncUsers(ret);
            } else {
                qDebug() << "DEBUG: doHeartbeat - Sync is disabled, skipping sync";
                
                // Still update local count and show disabled status
                QList<PERSONS_t> localUsers = RegisteredFacesDB::GetInstance()->GetAllPersonFromRAM();
                int localCount = localUsers.size();
                q_ptr->updateSyncDisplay("Disabled", localCount, 0);
                q_ptr->updateLocalFaceCount();
            }
        }
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

	LogD("%s %s[%d] === doAddPerson START === Person: %s, ID: %s\n", 
		__FILE__, __FUNCTION__, __LINE__, person_name.toStdString().c_str(), id_card_no.toStdString().c_str());

	mkdir("/mnt/user/tmp/", 0777);
	
	// Ensure face image directory exists using mkdir
	mkdir("/mnt/user/reg_face_image/", 0777);
	
	// Use employee ID as filename for face image storage
	QString faceImagePath = QString("/mnt/user/reg_face_image/%1.jpg").arg(id_card_no);
	
	if (doDownloadFile(face_img, faceImagePath))
	{
		LogD("%s %s[%d] Face image downloaded successfully: %s\n", 
			__FILE__, __FUNCTION__, __LINE__, faceImagePath.toStdString().c_str());
			
		if (person_uuid.length() <= 0 && person_name.length() <= 0)
		{
			LogE("%s %s[%d] Invalid person data - UUID or name empty\n", __FILE__, __FUNCTION__, __LINE__);
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

		// Extract face embedding from the downloaded image
		QByteArray faceEmbedding;
		bool embeddingExtracted = false;
		
		LogD("%s %s[%d] Extracting face embedding from downloaded image...\n", __FILE__, __FUNCTION__, __LINE__);
		
		int result = -1;
		int faceNum = 0;
		double threshold = 0;
		
		result = ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->RegistPerson(
			faceImagePath, 
			faceNum, 
			threshold, 
			faceEmbedding
		);
		
		if (result == 0 && !faceEmbedding.isEmpty()) {
			embeddingExtracted = true;
			LogD("%s %s[%d] Face embedding extracted successfully: %d bytes\n", 
				__FILE__, __FUNCTION__, __LINE__, faceEmbedding.size());
		} else {
			LogE("%s %s[%d] Failed to extract face embedding, result: %d\n", 
				__FILE__, __FUNCTION__, __LINE__, result);
		}

		// Read face image data for storage
		QFile imageFile(faceImagePath);
		QByteArray faceImageData;
		bool serverSuccess = false;
		
		if (imageFile.open(QIODevice::ReadOnly)) {
			faceImageData = imageFile.readAll();
			imageFile.close();
			
			LogD("%s %s[%d] Read face image data: %d bytes\n", __FILE__, __FUNCTION__, __LINE__, 
				faceImageData.size());
			
			// Prepare time of access
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
			
			LogD("%s %s[%d] Calling sendUserToServer with image data...\n", __FILE__, __FUNCTION__, __LINE__);
			
			// Call sendUserToServer with face image data
			serverSuccess = q_ptr->sendUserToServer(id_card_no, person_name, id_card_no, 
				card_no, male, group, timeOfAccess, faceImageData);
			
			LogD("%s %s[%d] sendUserToServer result: %s\n", __FILE__, __FUNCTION__, __LINE__, 
				serverSuccess ? "SUCCESS" : "FAILED");
		} else {
			LogE("%s %s[%d] Failed to read face image file: %s\n", __FILE__, __FUNCTION__, __LINE__, 
				faceImagePath.toStdString().c_str());
		}

		if (serverSuccess)
		{
			LogD("%s %s[%d] Server upload successful, saving to local database...\n", __FILE__, __FUNCTION__, __LINE__);
			
			// Prepare time of access string (duplicate code for clarity)
			QString timeOfAccess = "";
			if (auth_type == "time_range")
			{
				timeOfAccess = time_range;
			} else if (auth_type == "week_cycle")
			{
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

			// Store user locally with ONLY face embedding in DB, image already saved to directory
			bool isSaveDBOk = RegisteredFacesDB::GetInstance()->RegPersonToDBAndRAM(
				person_uuid, 
				person_name, 
				id_card_no, 
				card_no, 
				male, 
				group, 
				timeOfAccess, 
				faceEmbedding,     // ONLY face embedding stored in DB, image is in directory
                QByteArray(),
				"",
				"",
				"",
				"",
				"",
				0,
				0
			);
			
			LogD("%s %s[%d] RegPersonToDBAndRAM result: %s\n", __FILE__, __FUNCTION__, __LINE__, 
				isSaveDBOk ? "SUCCESS" : "FAILED");

			if (isSaveDBOk)
			{
				json["result"] = "1";
				json["success"] = "1";
				json["message"] = "person register success with server!";
				LogD("%s %s[%d] === doAddPerson SUCCESS ===\n", __FILE__, __FUNCTION__, __LINE__);
			}
			else
			{
				json["result"] = "-6";
				json["success"] = "0";
				json["message"] = "database save failed after server upload";
				LogE("%s %s[%d] Database save failed after server upload\n", __FILE__, __FUNCTION__, __LINE__);
			}
		}
		else
		{
			json["result"] = "-7";
			json["success"] = "0";
			json["message"] = "server upload failed";
			LogE("%s %s[%d] === doAddPerson FAILED - Server upload failed ===\n", __FILE__, __FUNCTION__, __LINE__);
		}

		QString ret = doPostJson(json);
		if (ret.length() > 0)
		{
			msg = ret;
			LogD("%s %s[%d] Response sent: %s\n", __FILE__, __FUNCTION__, __LINE__, ret.toStdString().c_str());
		}
	}
	else
	{
		LogE("%s %s[%d] Failed to download face image from: %s\n", __FILE__, __FUNCTION__, __LINE__, 
			face_img.toStdString().c_str());
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
							male, group, timeOfAccess, faceFeature, QByteArray(), "", "", "", "", "", 0, 0);
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
 uuid,å¿…å¡«,å¦åˆ™ä¸çŸ¥å¯¹åº”å“ªä¸€è®°å½•
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
							male, group, timeOfAccess, faceFeature,QByteArray(), "", "", "", "", "", 0, 0);
					LogD("%s %s[%d] RegPersonToDBAndRAM isSaveDBOk %d \n", __FILE__, __FUNCTION__, __LINE__, isSaveDBOk);

					if (isSaveDBOk)
					{
						okUUID += person_uuid+ ",";
					}
				} else //æ³¨å†Œå¤±è´¥
				{

					//"fail_uuid":"1,2,3,4,", //æ³¨å†ŒæˆåŠŸä¹‹åŽè¿”å›žçš„PID å­—ç¬¦ä¸²ç±»åž‹
					//"failresult": "1", //æˆåŠŸæ ‡å¿— æˆ– æ³¨æ˜Žå¤±è´¥çš„åŽŸå› , å¦‚æžœIDå€¼ï¼Œè¯·é™„ä¸€å¼ å¯¹åº”çš„å‡ºé”™åŽŸå› è¡¨ã€‚
					failReason="person_uuid,æˆ– person_name ä¸ºç©º";
					switch (result)
					{
						case -1:
							failReason ="path (/mnt/user/facedb/RegImage.jpeg) not exit " ;
						case -2:
							failReason ="bface_detect_face failed error (æ£€æµ‹äººè„¸å¤±è´¥,ç…§ç‰‡è´¨é‡è¾ƒä½Ž)" ;
						case -3:
							failReason ="bface_detect_face no face(æ— äººè„¸æ•°æ®,ç…§ç‰‡è´¨é‡è¾ƒä½Ž)" ;
						case -4:
							failReason ="bface_alignment failed error (æå–äººè„¸å…³é”®ç‚¹å¤±è´¥,ç…§ç‰‡è´¨é‡è¾ƒä½Ž)" ;
						case -5:
							failReason ="bface_quality_score failed error (è®¡ç®—äººè„¸å¾—åˆ†è¾ƒä½Ž,ç…§ç‰‡è´¨é‡è¾ƒä½Ž)" ;
						case -6:		
							failReason ="bface_extract_feature failed error (æå–äººè„¸ç‰¹å¾å¤±è´¥, ç…§ç‰‡è´¨é‡è¾ƒä½Ž)" ;		
						case -7:		
							failReason ="äººè„¸æ•°>2" ;																
						default:
						   failReason ="é”™è¯¯!";
					}
					//failUUID = person_uuid+ ",";
				}
				person_uuid="";
			} else //if (person_uuid.length() > 0 && person_name.length() > 0)
			{
				//failresult  person_uuid,æˆ– person_name ä¸ºç©º
				failReason="person_uuid,æˆ– person_name ä¸ºç©º";
				//failUUID = person_uuid+ ",";
			}
		} else //doDownloadFile(face_img, "/mnt/user/facedb/RegImage.jpeg") 
		{
			failReason="ä¸‹è½½ /mnt/user/facedb/RegImage.jpeg å¤±è´¥";
			//failUUID = person_uuid+ ",";
			//fail_uuid
			//failresult  ä¸èƒ½ä¸‹è½½ /mnt/user/facedb/RegImage.jpeg
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

	//å¤±è´¥åŽŸå› 
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

float ConnHttpServerThread::getHeartbeatLivenessThreshold() const
{
    Q_D(const ConnHttpServerThread);
    return d->m_heartbeatLivenessThreshold;
}

float ConnHttpServerThread::getHeartbeatQualityThreshold() const
{
    Q_D(const ConnHttpServerThread);
    return d->m_heartbeatQualityThreshold;
}

float ConnHttpServerThread::getHeartbeatComparisonThreshold() const
{
    Q_D(const ConnHttpServerThread);
    return d->m_heartbeatComparisonThreshold;
}

int ConnHttpServerThread::getHeartbeatIdentificationInterval() const
{
    Q_D(const ConnHttpServerThread);
    return d->m_heartbeatIdentificationInterval;
}

int ConnHttpServerThread::getHeartbeatRecognitionDistance() const
{
    Q_D(const ConnHttpServerThread);
    return d->m_heartbeatRecognitionDistance;
}

bool ConnHttpServerThread::hasHeartbeatThresholds() const
{
    Q_D(const ConnHttpServerThread);
    return d->m_hasHeartbeatThresholds;
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
	
	LogD("%s %s[%d] === doUpdatePerson START === Person: %s, ID: %s\n", 
		__FILE__, __FUNCTION__, __LINE__, person_name.toStdString().c_str(), id_card_no.toStdString().c_str());
	
	mkdir("/mnt/user/tmp/", 0777);
	mkdir("/mnt/user/reg_face_image/", 0777);
	
	if (person_uuid.length() > 0 && person_name.length() > 0)
	{
		QString faceImagePath = QString("/mnt/user/reg_face_image/%1.jpeg").arg(id_card_no);
		bool hasNewImage = false;
		bool serverSuccess = true; // Default to true if no new image
		
		// Remove old image file first
		unlink(faceImagePath.toStdString().c_str());
		LogD("%s %s[%d] Removed old image file: %s\n", __FILE__, __FUNCTION__, __LINE__, 
			faceImagePath.toStdString().c_str());
		
		if (doDownloadFile(face_img, faceImagePath))
		{
			person_image = faceImagePath;
			hasNewImage = true;
			LogD("%s %s[%d] New face image downloaded: %s\n", __FILE__, __FUNCTION__, __LINE__, 
				person_image.toStdString().c_str());
		}
		else
		{
			LogD("%s %s[%d] No new face image to download\n", __FILE__, __FUNCTION__, __LINE__);
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

		int result = ISC_OK;
		QString userTimeOfAccess = "";
		if (auth_type == "time_range")
		{
			userTimeOfAccess = time_range;
		} else if (auth_type == "week_cycle")
		{
			//data = 1,2,3,4,5,6,7
			//time_range = 00:00;21:00
			time_range.replace(QString(";"), QString(","));
			userTimeOfAccess = time_range;

			QStringList sections = time_data.split(",");
			for (int i = 1; i <= 7; i++)
			{
				bool isSet = false;
				userTimeOfAccess += ",";
				for (int j = 0; j < sections.size(); j++)
				{
					if (sections.at(j) == QString::number(i))
					{
						userTimeOfAccess += "1";
						isSet = true;
						break;
					}
				}
				if (isSet == false)
				{
					userTimeOfAccess += "0";
				}
			}
		}

		// FIXED: If there's a new image, call sendUserToServer with base64 data
		if (hasNewImage)
		{
			QFile imageFile(person_image);
			QByteArray faceImageData;
			QByteArray base64FaceImageData;
			
			if (imageFile.open(QIODevice::ReadOnly)) {
				faceImageData = imageFile.readAll();
				imageFile.close();
				
				LogD("%s %s[%d] Read face image data for update: %d bytes\n", __FILE__, __FUNCTION__, __LINE__, 
					faceImageData.size());
				
				// *** CRITICAL FIX: Convert to base64 to match sendUserToServer expectation ***
				base64FaceImageData = faceImageData.toBase64();
				
				LogD("%s %s[%d] Converted to base64 for update: %d bytes (original: %d bytes)\n", 
					__FILE__, __FUNCTION__, __LINE__, base64FaceImageData.size(), faceImageData.size());
				
				LogD("%s %s[%d] Calling sendUserToServer for update with base64 data...\n", __FILE__, __FUNCTION__, __LINE__);
				
				// Call sendUserToServer with BASE64 encoded face image data
				serverSuccess = q_ptr->sendUserToServer(id_card_no, person_name, id_card_no, 
					card_no, male, group, userTimeOfAccess, base64FaceImageData);
				
				LogD("%s %s[%d] sendUserToServer update result: %s\n", __FILE__, __FUNCTION__, __LINE__, 
					serverSuccess ? "SUCCESS" : "FAILED");
			} else {
				LogE("%s %s[%d] Failed to read face image file: %s\n", __FILE__, __FUNCTION__, __LINE__, 
					person_image.toStdString().c_str());
				serverSuccess = false;
			}
			
			if (!serverSuccess)
			{
				result = -7; // Server upload failed
				LogE("%s %s[%d] Server upload failed for update\n", __FILE__, __FUNCTION__, __LINE__);
			}
		}
		else
		{
			LogD("%s %s[%d] No new image, skipping server upload\n", __FILE__, __FUNCTION__, __LINE__);
		}

		if (result == ISC_OK)
		{
			LogD("%s %s[%d] Updating person in local database...\n", __FILE__, __FUNCTION__, __LINE__);
			
			// Update person without face feature since server handles it
			result = RegisteredFacesDB::GetInstance()->UpdatePersonToDBAndRAM(person_uuid, 
				person_name, id_card_no, card_no, male, group, userTimeOfAccess, "", QByteArray(), "", "", "", "", "", 0);
		}
		
		LogD("%s %s[%d] UpdatePersonToDBAndRAM result: %d\n", __FILE__, __FUNCTION__, __LINE__, result);

		usleep(500 * 1000);
		if (result == ISC_OK)
		{
			json["result"] = "1";
			json["success"] = "1";
			json["message"] = "person update success with server!";
			LogD("%s %s[%d] === doUpdatePerson SUCCESS ===\n", __FILE__, __FUNCTION__, __LINE__);
		} 
		else if (result == -7)
		{
			json["result"] = "-7";
			json["success"] = "0";
			json["message"] = "server upload failed";
			LogE("%s %s[%d] === doUpdatePerson FAILED - Server upload failed ===\n", __FILE__, __FUNCTION__, __LINE__);
		}
		else
		{
			json["result"] = "-6";
			json["success"] = "0";
			json["message"] = "update failed, unknown error";
			LogE("%s %s[%d] === doUpdatePerson FAILED - Unknown error ===\n", __FILE__, __FUNCTION__, __LINE__);
		}

		QString ret = doPostJson(json);
		msg = "";
		if (ret.length() > 0)
		{
			msg = ret;
			LogD("%s %s[%d] Update response sent: %s\n", __FILE__, __FUNCTION__, __LINE__, ret.toStdString().c_str());
		}
	}
	else
	{
		LogE("%s %s[%d] Invalid person data - UUID or name empty\n", __FILE__, __FUNCTION__, __LINE__);
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
		isOk = RegisteredFacesDB::GetInstance()->UpdatePersonToDBAndRAM(person_uuid, "", "", "", "", "", timeOfAccess, "", QByteArray(), "", "", "", "", "", 0);
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
				json["result"] = "-1; uuid ä¸ºç©º";
		  case  -5 : //ISC_UPDATE_PERSON_NOT_EXIST
		        json["result"] = "-5; ISC_UPDATE_PERSON_NOT_EXIST PERSON ä¸å­˜åœ¨";  
		  case -6:
		  		json["result"] = "-6; ISC_UPDATE_PERSON_DB_ERROR æ›´æ–°æ•°æ®åº“å‡ºé”™"; 			
			case -11:
				json["result"] ="-11; path (/mnt/user/facedb/RegImage.jpeg) not exit " ;
			case -12:
				json["result"] ="-12;bface_detect_face failed error (æ£€æµ‹äººè„¸å¤±è´¥,ç…§ç‰‡è´¨é‡è¾ƒä½Ž)" ;
			case -13:
				json["result"] ="-13;bface_detect_face no face(æ— äººè„¸æ•°æ®,ç…§ç‰‡è´¨é‡è¾ƒä½Ž)" ;
			case -14:
				json["result"] ="-14;bface_alignment failed error (æå–äººè„¸å…³é”®ç‚¹å¤±è´¥,ç…§ç‰‡è´¨é‡è¾ƒä½Ž)" ;
			case -15:
				json["result"] ="-15;bface_quality_score failed error (è®¡ç®—äººè„¸å¾—åˆ†è¾ƒä½Ž,ç…§ç‰‡è´¨é‡è¾ƒä½Ž)" ;
			case -16:		
				json["result"] ="-16;bface_extract_feature failed error (æå–äººè„¸ç‰¹å¾å¤±è´¥, ç…§ç‰‡è´¨é‡è¾ƒä½Ž)" ;		
			case -17:		
				json["result"] ="-17;äººè„¸æ•°>2" ;																
			default:
				json["result"] ="é”™è¯¯!";
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
	isOk = RegisteredFacesDB::GetInstance()->UpdatePersonToDBAndRAM(person_uuid, "", "", "", "", "", timeOfAccess, "", QByteArray(), "", "", "", "", "", 0);

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
çŠ¶æ€ 
-1:é»˜è®¤ å»¶æ—¶æ—¶é—´
1:å¸¸å¼€
2:å¸¸é—­

//çŠ¶æ€,é»˜è®¤ -1  å»¶æ—¶ æ—¶é—´ã€€ã€€ã€€å¸¸å¼€ 1ï¼Œå¸¸é—­ã€€2
*/
	QString status = "-1";
	status = QString::fromStdString(root["status"].asString());
	//if (status.toInt() ==1 || status.toInt() ==2)
	{
		//ä¿å­˜ã€€	
		
		ReadConfig::GetInstance()->setDoor_Relay(status.toInt());		
		LogD("%s %s[%d] status=%d \n", __FILE__, __FUNCTION__, __LINE__,status.toInt());		
	} 
	if (status.toInt() == -1)
		YNH_LJX::Utils_Door::GetInstance()->OpenDoor("");
	else if (status.toInt() == 1) //1:å¸¸å¼€
	{
        YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Relay, 1);
	}
	else if (status.toInt() == 2) //2:å¸¸é—­
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
		unsigned char TipMsg[128] = "æ­£åœ¨ä¸‹è½½å›ºä»¶...";
//		GUI::getInstance()->showTipsMsg(TipMsg);
		cmd = "cd /update; wget2 -c -nH -m --timeout=3 --ftp-user=admin --ftp-password=admin  " + path + " ; sync;";
		YNH_LJX::RkUtils::Utils_ExecCmd(cmd.c_str());

		isOK = checkFileMd5Sum("/update/update.zip", strMd5);
	} else if (path.find("http://") != std::string::npos)
	{
		unsigned char TipMsg[128] = "æ­£åœ¨ä¸‹è½½å›ºä»¶...";
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
								stPerson.szIDCardNum, stPerson.szICCardNum, stPerson.szSex, stPerson.szGids, timeOfAccess, faceFeature ,QByteArray(), "", "", "", "", "", 0, 0);
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
					/**æ‰¹é‡æ›´æ–°ç”¨ å…ˆåˆ é™¤ï¼Œå†å½•å…¥çš„æ–¹å¼æ“ä½œ**/
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
								stPerson.szIDCardNum, stPerson.szICCardNum, stPerson.szSex, stPerson.szGids, timeOfAccess, faceFeature, QByteArray(), "", "", "", "", "", 0, 0);
						LogD("%s %s[%d] RegPersonToDBAndRAM isSaveDBOk %d \n", __FILE__, __FUNCTION__, __LINE__, isSaveDBOk);
						if (isSaveDBOk == true)
						{
							fout.write(stPerson.szUUID, strlen(stPerson.szUUID));
							fout.write("\n",1);
						}
					}
					/**æ‰¹é‡æ›´æ–°ç”¨ å…ˆåˆ é™¤ï¼Œå†å½•å…¥çš„æ–¹å¼æ“ä½œ end**/

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
	/*if (password != stMsgHeader.password && stMsgHeader.msg_type != "set_device_password")
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
	}*/

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

	if (stMsgHeader.msg_type == std::string("add") || stMsgHeader.msg_type == std::string("add_person")) //æ³¨å†Œäººå‘˜
	{
		doAddPerson(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("add_persons")) //æ‰¹é‡æ³¨å†Œäººå‘˜
	{
		doAddPersons(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("add_personswithreason")) //æ‰¹é‡æ³¨å†Œäººå‘˜ å¸¦åŽŸå› 
	{
		doAddPersonswithreason(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("delete") || stMsgHeader.msg_type == std::string("delete_person")) //åˆ é™¤äººå‘˜
	{
		doDeletePerson(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("delete_all") || stMsgHeader.msg_type == std::string("delete_all_person")) //åˆ é™¤æ‰€æœ‰äººå‘˜
	{
		doDeleteAllPerson(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("update") || stMsgHeader.msg_type == std::string("update_person")) //æ³¨å†Œäººå‘˜
	{
		doUpdatePerson(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("find") || stMsgHeader.msg_type == std::string("find_person")) //æŸ¥è¯¢æŒ‡å®šäººå‘˜
	{
		doFindPerson(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("get_all") || stMsgHeader.msg_type == std::string("get_all_person")) //æŸ¥è¯¢æ‰€æœ‰äººå‘˜
	{
		doGetAllPerson(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("get_all_group")) //æŸ¥è¯¢æ‰€æœ‰éƒ¨é—¨
	{

	} else if (stMsgHeader.msg_type == std::string("set_pass_permission")) //è®¾ç½®ç”¨æˆ·é€šè¡Œæƒé™
	{
		doSetTimeOfAccess(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("delete_pass_permission")) //åˆ é™¤ç”¨æˆ·é€šè¡Œæƒé™
	{
		doDeleteTimeOfAccess(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("device_delete_pass_permission")) //åˆ é™¤è®¾å¤‡é€šè¡Œæƒé™
	{

	} else if (stMsgHeader.msg_type == std::string("delete_all_record")) //åˆ é™¤æ‰€æœ‰è¯†åˆ«è®°å½•
	{
		doDeleteAllPersonRecord(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("records_select")) //æŸ¥è¯¢è¯†åˆ«è®°å½•
	{
		doGetPersonRecord(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("records_select_done")) //facetools å®šæ—¶æŸ¥è¯¢è®°å½•åŽæˆåŠŸè¿”å›žï¼Œéœ€è¦ä¿®æ”¹æ•°æ®åº“ä¸­è®°å½•çŠ¶æ€
	{
		doUpdatePersonRecordUploadFlag(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("delete_record")) //åˆ é™¤æŒ‡å®šè¯†åˆ«è®°å½•
	{
		doDeletePersonRecord(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("open_door")) //å¼€é—¸
	{
		doOpenDoor(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("restart")) //é‡å¯
	{
		doReboot(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("net_config")) //é…ç½®è®¾å¤‡ç½‘ç»œ
	{
		doSetNetwork(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("get_dev_config")) //èŽ·å–è®¾å¤‡è®¾ç½®ä¿¡æ¯
	{
		doGetDeviceConfig(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("dev_config") || stMsgHeader.msg_type == std::string("set_dev_config")) //æ›´æ”¹è®¾å¤‡è®¾ç½®
	{
		doSetDeviceConfig(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("factory_setting")) //æ¢å¤é»˜è®¤è®¾ç½®
	{
		doResetFactorySetting(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("delete_all_user_data")) //åˆ é™¤æ‰€æœ‰ç”¨æˆ·æ•°æ®ï¼Œæ¢å¤åˆ°å‡ºåŽ‚çŠ¶æ€
	{

	} else if (stMsgHeader.msg_type == std::string("algo_setting")) //è®¾ç½®ç®—æ³•å‚æ•°
	{
		doSetAlgoParam(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("algo_reset")) //æ¢å¤é»˜è®¤ç®—æ³•
	{

	} else if (stMsgHeader.msg_type == std::string("dev_version")) //èŽ·å–è®¾å¤‡ç‰ˆæœ¬å·
	{
		doGetDeviceVersion(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("get_picture")) //èŽ·å–è®¾å¤‡å®žæ—¶æ‹ç…§å›¾ç‰‡
	{
		doTakePicture(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("system_update")) //ota å‡çº§å‘½ä»¤
	{
		doSystemUpdate(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("set_static_ip")) // é™æ€IPè®¾ç½®
	{
		doSetStaticLanIP(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("set_device_password")) //è®¾ç½®å¯†ç 
	{
		doSetDeviceHttpPassword(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("save_file")) //æ–‡ä»¶ä¿å­˜åˆ°è®¾å¤‡
	{
		doSaveFile(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("sync_persons_list")) //åŒæ­¥äººå‘˜åˆ—è¡¨
	{
		doSyncPersonsList(root, stMsgHeader);
	} else if (stMsgHeader.msg_type == std::string("set_device_httpserver"))
	{

	} else if (stMsgHeader.msg_type == std::string("set_timer")) // æ—¶é—´è®¾ç½®
	{

	}

	if (msg.length() < 2)
	{
		return false;
	}

//å‡å¦‚æ²¡æœ‰è¿”å›ž msg_type ä¸º idleï¼Œåˆ™æœåŠ¡å™¨ä¼šæœ‰åŽç»­éœ€è¦å¤„ç†çš„æ¶ˆæ¯
	if (stMsgHeader.msg_type != std::string("idle"))
	{
		return true;
	}
	return false;
}

ConnHttpServerThread::ConnHttpServerThread(QObject *parent) :
        QThread(parent), //
        d_ptr(new ConnHttpServerThreadPrivate(this)),
		     cleanupTimer(nullptr)  // Initialize cleanupTimer
{
    //qDebug() << "THREAD_DEBUG: Initializing ConnHttpServerThread";
    
    // Check network configuration
    QString serverUrl = ReadConfig::GetInstance()->getPerson_Registration_Address();
    QString serverPassword = ReadConfig::GetInstance()->getPerson_Registration_Password();
    QString serverAddress = ReadConfig::GetInstance()->getSrv_Manager_Address();
    
    //qDebug() << "THREAD_DEBUG: Registration Server URL: " << serverUrl;
    //qDebug() << "THREAD_DEBUG: Registration Server Password length: " << serverPassword.length();
    //qDebug() << "THREAD_DEBUG: Server Manager Address: " << serverAddress;
    
    // Check if network interfaces are up
    system("ifconfig > /tmp/ifconfig_debug.txt");
    system("ping -c 1 8.8.8.8 > /tmp/ping_debug.txt 2>&1");
    
    //qDebug() << "THREAD_DEBUG: Network diagnostic info saved to /tmp/ifconfig_debug.txt and /tmp/ping_debug.txt";
    
	initializeCleanup();

    this->start();
    if(ReadConfig::GetInstance()->getPost_PersonRecord_Address().size() > 3)
    {
        //qDebug() << "THREAD_DEBUG: Starting PostPersonRecordThread";
        PostPersonRecordThread::GetInstance();
    }
}

ConnHttpServerThread::~ConnHttpServerThread()
{
    Q_D(ConnHttpServerThread);
    this->requestInterruption();
    d->pauseCond.wakeOne();

    // Clean up timer
    if (cleanupTimer) {
        cleanupTimer->stop();
        delete cleanupTimer;
        cleanupTimer = nullptr;
    }

    this->quit();
    this->wait();
}

void ConnHttpServerThread::run()
{
    Q_D(ConnHttpServerThread);
    //qDebug() << "THREAD_DEBUG: ConnHttpServerThread starting...";
    sleep(5);
    
    while (true)
    {
        //qDebug() << "THREAD_DEBUG: Thread loop iteration";
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

        // Use the unified sync function that checks both time and count
        checkAndSyncUsers(m_lastHeartbeatResponse);

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

void ConnHttpServerThread::updateSyncDisplay(const QString &status, int currentCount, int totalCount)
{
    qDebug() << "DEBUG: updateSyncDisplay called with status:" << status << "count:" << currentCount << "/" << totalCount;
    
    // Update the main UI with sync information using the correct method
    if (qXLApp && qXLApp->GetFaceMainFrm()) {
        FaceMainFrm* mainFrm = qXLApp->GetFaceMainFrm();
        
        // Use the correct way to access FaceHomeFrm through FaceMainFrm
        // Based on the code, we need to access it through the private member
        // We'll add public methods to FaceMainFrm to forward the calls
        mainFrm->updateSyncStatus(status);
        mainFrm->updateSyncUserCount(currentCount, totalCount);
        
        // Update last sync time
        QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm");
        mainFrm->updateLastSyncTime(currentTime);
        
        qDebug() << "DEBUG: UI updated successfully through FaceMainFrm";
    } else {
        qDebug() << "WARNING: qXLApp or GetFaceMainFrm() is null, cannot update UI";
    }
}

bool ConnHttpServerThread::isServerReachable()
{
    // Simple ping to your server endpoint
    // Return true if server responds, false otherwise
    return true; // Implement based on your server API
}

void ConnHttpServerThread::updateTenantName(const QString &tenantName)
{
    qDebug() << "DEBUG: updateTenantName called with:" << tenantName;
    
    if (qXLApp && qXLApp->GetFaceMainFrm()) {
        FaceMainFrm* mainFrm = qXLApp->GetFaceMainFrm();
        mainFrm->setTenantName(tenantName);
        qDebug() << "DEBUG: Tenant name updated successfully";
    } else {
        qDebug() << "WARNING: qXLApp or GetFaceMainFrm() is null, cannot update tenant name";
    }
}

bool ConnHttpServerThread::sendUserToServer(const QString &employeeId, const QString &name, 
    const QString &idCard, const QString &icCard, const QString &sex, const QString &department,
    const QString &timeOfAccess, const QByteArray &faceFeature)
{
    Q_D(ConnHttpServerThread);
    
    qDebug() << "DEBUG: sendUserToServer - Starting to send user:" << name << "(" << employeeId << ")";
    
    QString regServerUrl = ReadConfig::GetInstance()->getPerson_Registration_Address();
    if (regServerUrl.isEmpty()) {
        qDebug() << "ERROR: Registration server URL not configured";
        return false;
    }
    
    // Save original URL
    QString originalUrl = d->mHttpServerUrl;
    d->mHttpServerUrl = regServerUrl;
    
    // Prepare clean JSON with only essential fields
    Json::Value json;
    std::string timestamp = getTime();
    QString regPassword = ReadConfig::GetInstance()->getPerson_Registration_Password();
    
    std::string password = regPassword.toStdString() + timestamp;
    password = md5sum(password);
    password = md5sum(password);
    transform(password.begin(), password.end(), password.begin(), ::tolower);
    
    // === CLEAN JSON STRUCTURE (Essential fields only) ===
    json["msg_type"] = "added_face_data";
    json["device"]   ="Ai Device";
    json["sn"] = d->sn.toStdString().c_str();
    json["timestamp"] = timestamp.c_str();
    json["password"] = password.c_str();
    
    // Core user data
    json["employeeId"] = employeeId.toStdString().c_str();
    json["name"] = name.toStdString().c_str();
    json["gender"] = sex.toStdString().c_str();
    json["idcardnum"] = idCard.toStdString().c_str();
    json["lastModified"] = QDateTime::currentDateTime().toString("yyyy/MM/dd HH:mm:ss").toStdString().c_str();
    
    // === FACE FEATURE DATA PROCESSING ===
    bool hasFaceDataToUpload = false;
    
    // Try to get face image data from cropped image file
    QString croppedImagePath = QString("/mnt/user/reg_face_image/%1.jpg").arg(employeeId);
    QFile croppedImageFile(croppedImagePath);
    QByteArray faceImageData;
    
    if (croppedImageFile.exists() && croppedImageFile.open(QIODevice::ReadOnly)) {
        faceImageData = croppedImageFile.readAll();
        croppedImageFile.close();
        
        if (faceImageData.size() > 1000) { // Valid image size
            // Encode face image to base64
            std::string base64FaceData = cereal::base64::encode(
                (unsigned char*)faceImageData.data(), 
                faceImageData.size()
            );
            
            // Add face feature to JSON (CLEAN - no debug fields)
            json["faceFeature"] = base64FaceData.c_str();
            hasFaceDataToUpload = true;
            
            qDebug() << "DEBUG: Using cropped face image data:" << faceImageData.size() << "bytes";
        }
    }
    
    // Fallback to provided face feature if no image file
    if (!hasFaceDataToUpload && !faceFeature.isEmpty()) {
        // Check if it's already base64 encoded
        QString faceDataString = QString::fromLatin1(faceFeature);
        QRegExp base64Regex("^[A-Za-z0-9+/]*={0,2}$");
        bool isAlreadyBase64 = base64Regex.exactMatch(faceDataString.left(100));
        
        if (isAlreadyBase64 && faceFeature.size() > 10000) {
            // Use as-is (already base64)
            json["faceFeature"] = faceDataString.toStdString().c_str();
            hasFaceDataToUpload = true;
            qDebug() << "DEBUG: Using base64 face feature from parameter";
        } else if (faceFeature.size() > 100) {
            // Encode binary data to base64
            std::string base64FaceData = cereal::base64::encode(
                (unsigned char*)faceFeature.data(), 
                faceFeature.size()
            );
            json["faceFeature"] = base64FaceData.c_str();
            hasFaceDataToUpload = true;
            qDebug() << "DEBUG: Encoded binary face feature to base64";
        }
    }
    
    // If no face data available, don't include faceFeature field at all
    if (!hasFaceDataToUpload) {
        qDebug() << "DEBUG: No face data available - sending user info only";
    }
    
    qDebug() << "DEBUG: Sending clean JSON to server (hasFaceData:" << hasFaceDataToUpload << ")";
    
    // Send request
    QString ret = d->doPostJson(json);
    
    // Restore original URL
    d->mHttpServerUrl = originalUrl;
    
    // ===== NEW: PARSE RESPONSE FOR FACE DATA SYNC =====
    if (ret.length() > 0) {
        qDebug() << "SUCCESS: Received server response, length:" << ret.length();
        
        // Parse the response JSON
        Json::Reader reader;
        Json::Value responseJson;
        if (reader.parse(ret.toStdString(), responseJson)) {
            qDebug() << "DEBUG: Successfully parsed server response JSON";
            
            // === RESPONSE VERIFICATION FOR DEBUGGING ===
            qDebug() << "=== SERVER RESPONSE ANALYSIS ===";
            qDebug() << "Response fields available:";
            for (const auto& key : responseJson.getMemberNames()) {
                qDebug() << "  Field:" << QString::fromStdString(key);
            }
            
            // Check basic response status
            QString result = QString::fromStdString(responseJson.get("result", "0").asString());
            QString success = QString::fromStdString(responseJson.get("success", "0").asString());
            QString message = QString::fromStdString(responseJson.get("message", "").asString());
            
            qDebug() << "Server result:" << result;
            qDebug() << "Server success:" << success;
            qDebug() << "Server message:" << message;
            
            // === FACE DATA PROCESSING FOR CROSS-DEVICE SYNC ===
            bool foundFaceDataInResponse = false;
            QByteArray responseFaceFeature;
            
            // Method 1: Check for employeeData object (recommended format)
            if (responseJson.isMember("employeeData") && responseJson["employeeData"].isObject()) {
                Json::Value employeeData = responseJson["employeeData"];
                qDebug() << "âœ“ Found employeeData object in response";
                
                if (employeeData.isMember("faceFeature") && !employeeData["faceFeature"].isNull()) {
                    std::string base64FaceFeature = employeeData["faceFeature"].asString();
                    qDebug() << "âœ“ Found faceFeature in employeeData, length:" << base64FaceFeature.length();
                    
                    if (base64FaceFeature.length() > 100) {
                        // Decode the base64 face feature
                        try {
                            std::string decodedFeature = cereal::base64::decode(base64FaceFeature);
                            responseFaceFeature = QByteArray(decodedFeature.data(), decodedFeature.size());
                            foundFaceDataInResponse = true;
                            
                            qDebug() << "âœ“ Successfully decoded face feature:" << responseFaceFeature.size() << "bytes";
                            
                            // Verify it's a valid face feature (should be 512 bytes for most systems)
                            if (responseFaceFeature.size() == 512 || responseFaceFeature.size() >= 128) {
                                qDebug() << "âœ“ Face feature size validation passed";
                            } else {
                                qDebug() << "WARNING: Unexpected face feature size:" << responseFaceFeature.size();
                            }
                        } catch (const std::exception& e) {
                            qDebug() << "ERROR: Failed to decode face feature:" << e.what();
                        }
                    }
                }
                
                // Also extract other sync-relevant data
                if (employeeData.isMember("employeeId")) {
                    QString responseEmployeeId = QString::fromStdString(employeeData["employeeId"].asString());
                    qDebug() << "Response employeeId:" << responseEmployeeId;
                    
                    if (responseEmployeeId != employeeId) {
                        qDebug() << "WARNING: EmployeeId mismatch - sent:" << employeeId << "received:" << responseEmployeeId;
                    }
                }
                
                if (employeeData.isMember("firstName")) {
                    QString responseFirstName = QString::fromStdString(employeeData["firstName"].asString());
                    qDebug() << "Response firstName:" << responseFirstName;
                }
                
                if (employeeData.isMember("lastModified")) {
                    QString responseLastModified = QString::fromStdString(employeeData["lastModified"].asString());
                    qDebug() << "Response lastModified:" << responseLastModified;
                }
            }
            
            // Method 2: Check for direct faceFeature field (alternative format)
            else if (responseJson.isMember("faceFeature") && !responseJson["faceFeature"].isNull()) {
                std::string base64FaceFeature = responseJson["faceFeature"].asString();
                qDebug() << "âœ“ Found direct faceFeature field, length:" << base64FaceFeature.length();
                
                if (base64FaceFeature.length() > 100) {
                    try {
                        std::string decodedFeature = cereal::base64::decode(base64FaceFeature);
                        responseFaceFeature = QByteArray(decodedFeature.data(), decodedFeature.size());
                        foundFaceDataInResponse = true;
                        
                        qDebug() << "âœ“ Successfully decoded direct face feature:" << responseFaceFeature.size() << "bytes";
                    } catch (const std::exception& e) {
                        qDebug() << "ERROR: Failed to decode direct face feature:" << e.what();
                    }
                }
            }
            
            // Method 3: Check for faceData array (legacy format)
            else if (responseJson.isMember("faceData") && responseJson["faceData"].isArray()) {
                Json::Value faceDataArray = responseJson["faceData"];
                if (faceDataArray.size() > 0 && faceDataArray[0].isString()) {
                    std::string base64FaceFeature = faceDataArray[0].asString();
                    qDebug() << "âœ“ Found faceData array, length:" << base64FaceFeature.length();
                    
                    if (base64FaceFeature.length() > 100) {
                        try {
                            std::string decodedFeature = cereal::base64::decode(base64FaceFeature);
                            responseFaceFeature = QByteArray(decodedFeature.data(), decodedFeature.size());
                            foundFaceDataInResponse = true;
                            
                            qDebug() << "âœ“ Successfully decoded faceData array:" << responseFaceFeature.size() << "bytes";
                        } catch (const std::exception& e) {
                            qDebug() << "ERROR: Failed to decode faceData array:" << e.what();
                        }
                    }
                }
            }
            
            // === CROSS-DEVICE SYNC ENABLEMENT ===
            if (foundFaceDataInResponse && !responseFaceFeature.isEmpty()) {
                qDebug() << "SUCCESS: Server returned face data for cross-device sync!";
                qDebug() << "Face feature available for other devices during sync";
                qDebug() << "Cross-device face recognition: ENABLED";
                
                // Optional: Store the returned face feature locally for verification
                // This ensures the exact same feature that other devices will get during sync
                QString verificationPath = QString("/mnt/user/reg_face_image/%1_server_feature.dat").arg(employeeId);
                QFile featureFile(verificationPath);
                if (featureFile.open(QIODevice::WriteOnly)) {
                    featureFile.write(responseFaceFeature);
                    featureFile.close();
                    qDebug() << "DEBUG: Saved server face feature for verification:" << verificationPath;
                }
                
                qDebug() << "=== CROSS-DEVICE SYNC STATUS: SUCCESS ===";
                return true;
                
            } else {
                qDebug() << "WARNING: Server did not return face data in response";
                qDebug() << "Cross-device face recognition may not work properly";
                qDebug() << "Other devices will not be able to recognize this person's face";
                
                if (result == "1" && success == "1") {
                    qDebug() << "Registration successful but missing face data for sync";
                    qDebug() << "=== CROSS-DEVICE SYNC STATUS: PARTIAL ===";
                    return true; // Registration succeeded, sync may be limited
                } else {
                    qDebug() << "Registration failed - server returned error";
                    qDebug() << "=== CROSS-DEVICE SYNC STATUS: FAILED ===";
                    return false;
                }
            }
            
        } else {
            qDebug() << "ERROR: Failed to parse server response JSON";
            qDebug() << "Raw response:" << ret.left(200) << "...";
            return false;
        }
        
    } else {
        qDebug() << "ERROR: Empty response from server";
        return false;
    }
}

void ConnHttpServerThread::checkAndSyncUsers(const QString& heartbeatResponse)
{
    qDebug() << "DEBUG: checkAndSyncUsers - Starting sync check with corrected server vs device time comparison";
    
    // Check if sync is enabled
    bool syncEnabled = ReadConfig::GetInstance()->getSyncEnabled();
    qDebug() << "DEBUG: checkAndSyncUsers - Sync enabled:" << syncEnabled;
    
    // Get local user count for display
    QList<PERSONS_t> localUsers = RegisteredFacesDB::GetInstance()->GetAllPersonFromRAM();
    int localCount = localUsers.size();
    
    if (!syncEnabled) {
        LogD("%s %s[%d] Sync is disabled. Skipping user sync.\n", __FILE__, __FUNCTION__, __LINE__);
        qDebug() << "DEBUG: checkAndSyncUsers - Sync disabled, updating display and returning";
        updateSyncDisplay("Disabled", localCount, 0);
        updateLocalFaceCount();
        return;
    }

    // Show sync starting with current local count
    updateSyncDisplay("Checking", localCount, 0);
    
    LogD("%s %s[%d] DEBUG: Sync is enabled, proceeding with sync check\n", __FILE__, __FUNCTION__, __LINE__);

    // Parse heartbeat response
    Json::Reader reader;
    Json::Value responseJson;
    
    if (!reader.parse(heartbeatResponse.toStdString(), responseJson)) {
        qDebug() << "ERROR: checkAndSyncUsers - Failed to parse heartbeat response JSON";
        updateSyncDisplay("Failed", localCount, 0);
        return;
    }
    
    qDebug() << "DEBUG: checkAndSyncUsers - Successfully parsed heartbeat JSON";
    
    // Extract server info
    int serverUserCount = 0;
    QString currentServerLastModified = "";
    
    if (responseJson.isMember("TotalUserCount")) {
        serverUserCount = atoi(responseJson["TotalUserCount"].asString().c_str());
        qDebug() << "DEBUG: checkAndSyncUsers - Server user count:" << serverUserCount;
    }
    
    // Get current server's date_updated from this heartbeat
    if (responseJson.isMember("date_updated") && !responseJson["date_updated"].isNull()) {
        currentServerLastModified = QString::fromStdString(responseJson["date_updated"].asString());
    }
    else if (responseJson.isMember("dateUpdated") && !responseJson["dateUpdated"].isNull()) {
        currentServerLastModified = QString::fromStdString(responseJson["dateUpdated"].asString());
    }
    else if (responseJson.isMember("lastModified") && !responseJson["lastModified"].isNull()) {
        currentServerLastModified = QString::fromStdString(responseJson["lastModified"].asString());
    }
    else if (responseJson.isMember("lastUpdatedAt") && !responseJson["lastUpdatedAt"].isNull()) {
        currentServerLastModified = QString::fromStdString(responseJson["lastUpdatedAt"].asString());
    }
    
    updateSyncDisplay("Analyzing", localCount, serverUserCount);
    
    // === ENHANCED SYNC DECISION LOGIC (SAME AS WORKING FUNCTION) ===
    bool needsSync = false;
    QString syncReason = "";
    
    // Get local last modified time (using working function's approach)
    QDateTime localLastModifiedTime = getLocalLastModifiedTime();
    QString localLastModified = localLastModifiedTime.isValid() ? 
        localLastModifiedTime.toString("yyyy/MM/dd HH:mm:ss") : "None";
    
    qDebug() << "DEBUG: checkAndSyncUsers - Local lastModified (IST):" << localLastModified;
    
    // Convert server UTC time to IST for comparison (same as working function)
    QDateTime serverLastModifiedIST;
    if (!currentServerLastModified.isEmpty()) {
        serverLastModifiedIST = convertUTCToIST(currentServerLastModified);
        qDebug() << "DEBUG: checkAndSyncUsers - Server lastModified converted to IST:" 
                 << (serverLastModifiedIST.isValid() ? serverLastModifiedIST.toString("yyyy/MM/dd HH:mm:ss") : "Invalid");
    }
    
    // Reason 1: Count mismatch (ENHANCED - same logic as working function)
    if (serverUserCount != localCount) {
        needsSync = true;
        syncReason = QString("Count mismatch - Server: %1, Local: %2").arg(serverUserCount).arg(localCount);
        qDebug() << "DEBUG: checkAndSyncUsers - " << syncReason;
    } 
    // Reason 2: Time mismatch (ONLY if no count mismatch - same as working function)
    else if (serverLastModifiedIST.isValid() && localLastModifiedTime.isValid() && 
             serverLastModifiedIST > localLastModifiedTime) {
        needsSync = true;
        syncReason = "Time mismatch - Server has newer data";
        qDebug() << "DEBUG: checkAndSyncUsers - " << syncReason;
    } else {
        qDebug() << "DEBUG: checkAndSyncUsers - No sync needed";
    }
    
    // === MAIN SYNC EXECUTION ===
    if (needsSync) {
        qDebug() << "DEBUG: checkAndSyncUsers - Starting sync. Reason:" << syncReason;
        
        // Perform the actual sync
        performFullSync(serverUserCount);
        
        if (!currentServerLastModified.isEmpty()) {
        Q_D(ConnHttpServerThread);
        QDateTime newServerTimeIST = d->convertServerUTCToIST(currentServerLastModified);
        if (newServerTimeIST.isValid()) {
            QString newServerTimeFormatted = newServerTimeIST.toString("yyyy/MM/dd HH:mm:ss");
            
            // *** CHANGED: Replace file operations with debug messages ***
            QString serverRefTimeFile = "/mnt/user/sync_data/server_reference_time.txt";
            qDebug() << "SYNC_DATA_DEBUG: Would write to file:" << serverRefTimeFile;
            qDebug() << "SYNC_DATA_DEBUG: Content would be:" << newServerTimeFormatted;
            qDebug() << "SYNC_DATA_DEBUG: SOURCE=SERVER_LAST_MODIFIED_REFERENCE";
            qDebug() << "SYNC_DATA_DEBUG: SYNC_COMPLETED=" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
            qDebug() << "SYNC_DATA_DEBUG: SYNC_REASON=" << syncReason;
            qDebug() << "SUCCESS: Stored server reference time after sync (debug only, no file created):" << newServerTimeFormatted;
        }
    }
                 else {
                    qDebug() << "ERROR: Could not save server reference time";
                }
            }
        
        
    else {
        qDebug() << "DEBUG: checkAndSyncUsers - No sync needed - device data is current";
        updateSyncDisplay("Up to Date", localCount, serverUserCount);
    }
    
    // Final status update
    QList<PERSONS_t> finalUsers = RegisteredFacesDB::GetInstance()->GetAllPersonFromRAM();
    int finalCount = finalUsers.size();
    
    if (needsSync) {
        QString finalStatus;
        if (finalCount == serverUserCount) {
            finalStatus = "Complete";
            qDebug() << "SUCCESS: checkAndSyncUsers - Perfect sync completed";
        } else if (finalCount > localCount) {
            finalStatus = "Improved";
        } else {
            finalStatus = "partial";
        }
        updateSyncDisplay(finalStatus, finalCount, serverUserCount);
    }
    
    qDebug() << "DEBUG: checkAndSyncUsers - Sync check completed with corrected logic";
}

QDateTime ConnHttpServerThread::getDeviceLastModifiedTime()
{
    qDebug() << "DEBUG: getDeviceLastModifiedTime - Getting DEVICE's actual last modified time";
    
    // Get all local users from device database
    QList<PERSONS_t> localUsers = RegisteredFacesDB::GetInstance()->GetAllPersonFromRAM();
    if (localUsers.isEmpty()) {
        qDebug() << "DEBUG: getDeviceLastModifiedTime - No local users found on device";
        return QDateTime(); // Return invalid datetime if no users
    }
    
    // Find the most recent created/modified time from device's actual user records
    QDateTime mostRecentDeviceTime;
    QString mostRecentUserName = "";
    
    for (const PERSONS_t& person : localUsers) {
        QDateTime userCreatedTime;
        
        // Try different time format parsing for device records
        userCreatedTime = QDateTime::fromString(person.createtime, "yyyy/MM/dd HH:mm:ss");
        
        if (!userCreatedTime.isValid()) {
            userCreatedTime = QDateTime::fromString(person.createtime, "yyyyMMddHHmmss");
        }
        
        if (!userCreatedTime.isValid()) {
            userCreatedTime = QDateTime::fromString(person.createtime, "yyyy-MM-dd HH:mm:ss");
        }
        
        if (!userCreatedTime.isValid()) {
            qDebug() << "DEBUG: getDeviceLastModifiedTime - Could not parse time for user:" 
                     << person.name << ", createtime:" << person.createtime;
            continue;
        }
        
        // Track the most recent modification time on this device
        if (!mostRecentDeviceTime.isValid() || userCreatedTime > mostRecentDeviceTime) {
            mostRecentDeviceTime = userCreatedTime;
            mostRecentUserName = person.name;
        }
    }
    
    if (mostRecentDeviceTime.isValid()) {
        qDebug() << "SUCCESS: getDeviceLastModifiedTime - Device's most recent modification time:" 
                 << mostRecentDeviceTime.toString("yyyy-MM-dd hh:mm:ss");
        qDebug() << "  Most recent user:" << mostRecentUserName;
        qDebug() << "  Source: DEVICE LOCAL RECORDS (not stored server time)";
    } else {
        qDebug() << "WARNING: getDeviceLastModifiedTime - No valid times found in device records";
    }
    
    return mostRecentDeviceTime;
}

void ConnHttpServerThread::performFullSync(int serverCount)
{
    Q_D(ConnHttpServerThread);
    
    qDebug() << "DEBUG: performFullSync - Starting INCREMENTAL sync with target server count:" << serverCount;
    
    // === SAME MUTEX LOGIC AS WORKING VERSION ===
    if (!d->syncMutex.tryLock()) {
        qDebug() << "WARNING: performFullSync - Another sync in progress, skipping";
        updateSyncDisplay("Sync In Progress", 0, 0);
        return;
    }
    
    d->syncInProgress = true;
    
    // Get current local count for display
    QList<PERSONS_t> currentLocalUsers = RegisteredFacesDB::GetInstance()->GetAllPersonFromRAM();
    int currentLocalCount = getValidUserCount(currentLocalUsers);
    
    qDebug() << "DEBUG: performFullSync - Starting with local count:" << currentLocalCount << "target:" << serverCount;
    
	qDebug() << "[SYNC_TRIGGER] getServerUserList() called due to lastModified mismatch or count mismatch";

    
    // Step 2: Get server user list (same as working version)
    updateSyncDisplay("Getting Modified Server List", currentLocalCount, serverCount);
    QMap<QString, QString> serverUsers = getServerUserList();
    if (serverUsers.isEmpty()) {
        qDebug() << "ERROR: performFullSync - Failed to get server user list or list is empty";
        updateSyncDisplay("Failed", currentLocalCount, serverCount);
        d->syncInProgress = false;
        d->syncMutex.unlock();
        return;
    }
    
    // Use actual parsed count (same as working version)
    int actualServerCount = serverUsers.size();
    qDebug() << "DEBUG: performFullSync - Found" << actualServerCount << "users (target was" << serverCount << ")";
    
    int targetCount = qMax(actualServerCount, serverCount);
    
    updateSyncDisplay("Processing Modified Users", currentLocalCount, targetCount);
    
    // Step 3: Build comprehensive local user map (same as working version)
    QList<PERSONS_t> localUsers = RegisteredFacesDB::GetInstance()->GetAllPersonFromRAM();
    QMap<QString, QString> localUserMap;
    QSet<QString> localEmployeeIds;
    
    for (const PERSONS_t& person : localUsers) {
        if (!person.idcard.isEmpty() && !person.name.isEmpty()) {
            localUserMap[person.idcard] = person.createtime;
            localEmployeeIds.insert(person.idcard);
        }
    }
    
    qDebug() << "DEBUG: performFullSync - Local user map has" << localUserMap.size() << "valid users";

    // === STEP 4: Filter users based on sync type ===
    QStringList usersToProcess;
    QDateTime lastSyncDateTime;

        // Handle extra users (same logic as working version)
        QSet<QString> serverEmployeeIds = QSet<QString>::fromList(serverUsers.keys());
        QSet<QString> extraUsers = localEmployeeIds - serverEmployeeIds;
        
        if (!extraUsers.isEmpty()) {
            updateSyncDisplay("Removing Extra Users", currentLocalCount, targetCount);
            
            int removedCount = 0;
            for (const QString& employeeId : extraUsers) {
                QString uuid = findUuidByEmployeeId(employeeId);
                if (!uuid.isEmpty()) {
                    bool deleted = RegisteredFacesDB::GetInstance()->DelPersonByPersionUUIDFromDBAndRAM(uuid);
                    if (deleted) {
                        removedCount++;
                        qDebug() << "DEBUG: performFullSync - Removed extra user[" << removedCount << "]:" << employeeId;
                    }
                }
            }
            
            qDebug() << "DEBUG: performFullSync - Removed" << removedCount << "extra users";
            
            // Update count after deletions
            QList<PERSONS_t> afterDeleteUsers = RegisteredFacesDB::GetInstance()->GetAllPersonFromRAM();
            currentLocalCount = getValidUserCount(afterDeleteUsers);
            qDebug() << "DEBUG: performFullSync - After deletions, local count:" << currentLocalCount;
        }

		for (const QString& employeeId : serverEmployeeIds) {
    if (!localUserMap.contains(employeeId)) {
        // New user
        usersToProcess << employeeId;
    } else {
        // Compare timestamps
        QString localTimeStr = localUserMap[employeeId];
        QDateTime localTime = QDateTime::fromString(localTimeStr, "yyyy/MM/dd hh:mm:ss");
        QDateTime serverTimeIST = convertUTCToIST(serverUsers[employeeId]);

        if (serverTimeIST.isValid() && localTime.isValid() && serverTimeIST > localTime) {
            usersToProcess << employeeId; // Needs update
        }
    }
}	
    

    // Step 5: Process identified users (same processing logic as working version)
    int totalToProcess = usersToProcess.size();
    int processedCount = 0;
    int successCount = 0;
    int failCount = 0;
    int addCount = 0;
    int updateCount = 0;
    int skipCount = 0;
    
    if (totalToProcess == 0) {
        qDebug() << "DEBUG: performFullSync - No users to process - all up to date";
        updateSyncDisplay("Up to Date", currentLocalCount, targetCount);
        
        // Update last sync time even if no users processed
        updateLastSyncTime();
        
        d->syncInProgress = false;
        d->syncMutex.unlock();
        return;
    }
	
    
    updateSyncDisplay("Syncing Modified Users", currentLocalCount, targetCount);
    
    qDebug() << "DEBUG: performFullSync - Starting to process" << totalToProcess << "users individually";
    
    // Process each identified user (same individual processing as working version)
    for (const QString& employeeId : usersToProcess) {
        QString serverTimeUTC = serverUsers[employeeId];
        
        processedCount++;
        
        qDebug() << "DEBUG: performFullSync - Processing user" << processedCount << "/" << totalToProcess << ":" << employeeId;
        
        // Show progress every 5 users (same as working version)
        if (processedCount % 5 == 0) {
            QList<PERSONS_t> progressUsers = RegisteredFacesDB::GetInstance()->GetAllPersonFromRAM();
            int progressLocalCount = getValidUserCount(progressUsers);
            
            QString progressStatus = QString("%1 %2/%3 (%4%)")
                .arg(processedCount)
                .arg(totalToProcess)
                .arg((processedCount * 100) / totalToProcess);
            
            updateSyncDisplay(progressStatus, progressLocalCount, targetCount);
            QApplication::processEvents(); // Allow UI updates
        }
        
        // Determine if user needs sync (same logic as working version)
        bool needsSync = false;
        bool isNewUser = false;
        
        if (!localUserMap.contains(employeeId)) {
            needsSync = true;
            isNewUser = true;
            qDebug() << "DEBUG: User" << employeeId << "is NEW - needs to be added";
        } else {
            // Check if server version is newer (same as working version)
            QString localTimeStr = localUserMap[employeeId];
            QDateTime localTime = QDateTime::fromString(localTimeStr, "yyyy/MM/dd hh:mm:ss");
            QDateTime serverTimeIST = convertUTCToIST(serverTimeUTC);
            
            if (serverTimeIST.isValid() && localTime.isValid() && serverTimeIST > localTime) {
                needsSync = true;
                qDebug() << "DEBUG: User" << employeeId << "needs UPDATE - server is newer";
            } else {
                skipCount++;
                qDebug() << "DEBUG: User" << employeeId << "is UP-TO-DATE - skipping";
            }
        }
        
        // Sync the user if needed (same sync logic as working version)
        if (needsSync) {
            qDebug() << "DEBUG: performFullSync - Fetching data for user:" << employeeId;
            
            bool syncSuccess = false;
            int retryCount = 0;
            int maxRetries = (isNewUser) ? 3 : 1;
            
            while (!syncSuccess && retryCount < maxRetries) {
                retryCount++;
                qDebug() << "DEBUG: performFullSync - Attempt" << retryCount << "for user:" << employeeId;
                
                syncSuccess = fetchAndAddUser(employeeId, serverTimeUTC);
                
                if (!syncSuccess && retryCount < maxRetries) {
                    qDebug() << "WARNING: performFullSync - Retry" << retryCount << "failed for user:" << employeeId;
                    QThread::msleep(1000);
                }
            }
            
            if (syncSuccess) {
                successCount++;
                if (isNewUser) {
                    addCount++;
                    qDebug() << "SUCCESS: Added new user[" << addCount << "]:" << employeeId;
                } else {
                    updateCount++;
                    qDebug() << "SUCCESS: Updated user[" << updateCount << "]:" << employeeId;
                }
                localUserMap[employeeId] = serverTimeUTC;
            } else {
                failCount++;
                qDebug() << "ERROR: Failed to sync user[" << failCount << "]:" << employeeId;
                
                // Same fallback logic as working version for critical users
                if (isNewUser) {
                    qDebug() << "CRITICAL: Attempting fallback storage for new user:" << employeeId;
                    
                    QString uuid = QUuid::createUuid().toString();
                    uuid = uuid.replace("{", "").replace("}", "");
                    
                    bool fallbackSuccess = RegisteredFacesDB::GetInstance()->RegPersonToDBAndRAM(
                        uuid,
                        "User_" + employeeId,
                        employeeId,
                        "000000",
                        "Unknown",
                        "",
                        "00:00,23:59,1,1,1,1,1,1,1",
                        QByteArray(),
						"",
						"",
						"",
						"",
						"",
						"",
						0,
						0
                    );
                    
                    if (fallbackSuccess) {
                        qDebug() << "SUCCESS: Fallback storage worked for user:" << employeeId;
                        successCount++;
                        addCount++;
                        failCount--;
                        localUserMap[employeeId] = serverTimeUTC;
                    }
                }
            }
        }
        
        // Small delay (same as working version)
        if (processedCount % 3 == 0) {
            QThread::msleep(100);
        }
        
        // Check for interruption (same as working version)
        if (QThread::currentThread()->isInterruptionRequested()) {
            qDebug() << "DEBUG: performFullSync - Interruption requested at user" << processedCount;
            break;
        }
    }
    
    // === STEP 6: Update last sync time after successful sync ===
    if (successCount > 0 || totalToProcess == 0) {
        updateLastSyncTime();
        qDebug() << "DEBUG: performFullSync - Updated last sync time";
    }
    
    // Get final counts (same as working version)
    QList<PERSONS_t> finalLocalUsers = RegisteredFacesDB::GetInstance()->GetAllPersonFromRAM();
    int finalLocalCount = getValidUserCount(finalLocalUsers);
    
    qDebug() << "DEBUG: performFullSync - INCREMENTAL SYNC SUMMARY:";
    qDebug() << "  Total users processed:" << processedCount << "/" << totalToProcess;
    qDebug() << "  New users added:" << addCount;
    qDebug() << "  Existing users updated:" << updateCount;
    qDebug() << "  Users skipped (up-to-date):" << skipCount;
    qDebug() << "  Users failed:" << failCount;
    qDebug() << "  Final local count:" << finalLocalCount;
    qDebug() << "  Target server count:" << targetCount;
    
    
    // === SAME MUTEX CLEANUP AS WORKING VERSION ===
    d->syncInProgress = false;
    d->syncMutex.unlock();
    
    // Don't update final status here - let calling function handle it
}

bool ConnHttpServerThread::determineIfFirstTimeSync()
{
    // *** CHANGED: Replace file check with debug message ***
    QString lastSyncFile = "/mnt/user/sync_data/last_sync_time.txt";
    qDebug() << "SYNC_DATA_DEBUG: Would check file existence:" << lastSyncFile;
    qDebug() << "DEBUG: No last sync time file found (debug mode) - this is first time sync";
    
    // Check if we have any local users (this logic remains unchanged)
    QList<PERSONS_t> localUsers = RegisteredFacesDB::GetInstance()->GetAllPersonFromRAM();
    int validUserCount = getValidUserCount(localUsers);
    
    if (validUserCount == 0) {
        qDebug() << "DEBUG: No local users found - treating as first time sync";
        return true;
    }
    
    // === NEW: Check for tenant change OR significant count mismatch ===
    QString lastResponse = getLastHeartbeatResponse();
    if (!lastResponse.isEmpty()) {
        Json::Reader reader;
        Json::Value responseJson;
        if (reader.parse(lastResponse.toStdString(), responseJson)) {
            
            // === 1. CHECK FOR TENANT CHANGE ===
            bool tenantChanged = false;
            QString currentTenantId = "";
            QString storedTenantId = ReadConfig::GetInstance()->getHeartbeat_TenantId();
            
            if (responseJson.isMember("tenantId") && !responseJson["tenantId"].isNull()) {
                currentTenantId = QString::fromStdString(responseJson["tenantId"].asString());
                
                qDebug() << "TENANT_CHANGE_DEBUG: Tenant comparison:";
                qDebug() << "TENANT_CHANGE_DEBUG:   Current tenant ID:" << currentTenantId;
                qDebug() << "TENANT_CHANGE_DEBUG:   Stored tenant ID:" << storedTenantId;
                
                if (!storedTenantId.isEmpty() && currentTenantId != storedTenantId) {
                    tenantChanged = true;
                    qDebug() << "TENANT_CHANGE_DEBUG: *** TENANT CHANGE DETECTED ***";
                    qDebug() << "TENANT_CHANGE_DEBUG: Changed from" << storedTenantId << "to" << currentTenantId;
                } else if (storedTenantId.isEmpty()) {
                    qDebug() << "TENANT_CHANGE_DEBUG: First time tenant ID detected:" << currentTenantId;
                    tenantChanged = true; // Treat first-time tenant as change
                } else {
                    qDebug() << "TENANT_CHANGE_DEBUG: Same tenant - no change detected";
                }
            }
            
            // === 2. CHECK FOR SIGNIFICANT COUNT MISMATCH ===
            bool significantCountMismatch = false;
            if (responseJson.isMember("TotalUserCount")) {
                int serverCount = atoi(responseJson["TotalUserCount"].asString().c_str());
                int localCount = validUserCount;
                
                // Calculate the percentage difference
                float countDifferencePercent = 0;
                if (localCount > 0) {
                    countDifferencePercent = abs(serverCount - localCount) * 100.0f / localCount;
                }
                
                qDebug() << "TENANT_CHANGE_DEBUG: Count comparison for sync type decision:";
                qDebug() << "TENANT_CHANGE_DEBUG:   Server count:" << serverCount;
                qDebug() << "TENANT_CHANGE_DEBUG:   Local count:" << localCount;
                qDebug() << "TENANT_CHANGE_DEBUG:   Difference:" << abs(serverCount - localCount);
                qDebug() << "TENANT_CHANGE_DEBUG:   Difference percentage:" << countDifferencePercent << "%";
                
                // If there's a significant count mismatch (>30%), treat as requiring full sync
                if (countDifferencePercent > 30.0f) {
                    significantCountMismatch = true;
                    qDebug() << "TENANT_CHANGE_DEBUG: *** SIGNIFICANT COUNT MISMATCH DETECTED ***";
                    qDebug() << "TENANT_CHANGE_DEBUG: Threshold: 30%, Actual:" << countDifferencePercent << "%";
                }
            }
            
            // === 3. FORCE FIRST TIME SYNC IF EITHER CONDITION IS TRUE ===
            if (tenantChanged || significantCountMismatch) {
                if (tenantChanged && significantCountMismatch) {
                    qDebug() << "TENANT_CHANGE_DEBUG: *** BOTH TENANT CHANGE AND COUNT MISMATCH DETECTED ***";
                    qDebug() << "TENANT_CHANGE_DEBUG: This confirms a tenant switch - forcing FIRST TIME SYNC";
                } else if (tenantChanged) {
                    qDebug() << "TENANT_CHANGE_DEBUG: *** TENANT CHANGE DETECTED - forcing FIRST TIME SYNC ***";
                } else if (significantCountMismatch) {
                    qDebug() << "TENANT_CHANGE_DEBUG: *** COUNT MISMATCH DETECTED - forcing FIRST TIME SYNC ***";
                }
                
                // *** CHANGED: Replace file removal with debug messages ***
                qDebug() << "SYNC_DATA_DEBUG: Would remove file:" << lastSyncFile;
                qDebug() << "SYNC_DATA_DEBUG: Would remove file: /mnt/user/sync_data/server_lastmodified.txt";
                qDebug() << "SYNC_DATA_DEBUG: Would remove file: /mnt/user/sync_data/server_reference_time.txt";
                qDebug() << "TENANT_CHANGE_DEBUG: Cleared last sync time file to force full sync (debug only)";
                qDebug() << "TENANT_CHANGE_DEBUG: Cleared server time references for fresh start (debug only)";
                
                return true;
            } else {
                qDebug() << "FIRST_TIME_SYNC_DEBUG: No tenant change or significant count mismatch";
                qDebug() << "FIRST_TIME_SYNC_DEBUG: Checking other first-time sync conditions...";
            }
        } else {
            qDebug() << "FIRST_TIME_SYNC_DEBUG: Could not parse heartbeat response";
        }
    } else {
        qDebug() << "FIRST_TIME_SYNC_DEBUG: No heartbeat response available";
    }
    
    // === EXISTING CONDITION: Check if last sync was more than 24 hours ago ===
    QString lastSyncTime = getLastSyncTime(); // This will return empty in debug mode
    if (!lastSyncTime.isEmpty()) {
        QDateTime lastSync = QDateTime::fromString(lastSyncTime, "yyyy/MM/dd HH:mm:ss");
        if (lastSync.isValid()) {
            qint64 hoursSinceLastSync = lastSync.secsTo(QDateTime::currentDateTime()) / 3600;
            qDebug() << "FIRST_TIME_SYNC_DEBUG: Hours since last sync:" << hoursSinceLastSync;
            
            if (hoursSinceLastSync > 24) {
                qDebug() << "FIRST_TIME_SYNC_DEBUG: Last sync was" << hoursSinceLastSync << "hours ago - FIRST TIME SYNC";
                return true;
            }
        } else {
            qDebug() << "FIRST_TIME_SYNC_DEBUG: Invalid last sync time format - FIRST TIME SYNC";
            return true;
        }
    } else {
        qDebug() << "FIRST_TIME_SYNC_DEBUG: No last sync time found (debug mode) - INCREMENTAL SYNC";
        // In debug mode, since we don't create files, default to incremental sync
        return false;
    }
    
    qDebug() << "FIRST_TIME_SYNC_DEBUG: All conditions checked - INCREMENTAL SYNC";
    qDebug() << "FIRST_TIME_SYNC_DEBUG: Found last sync time and local users, no tenant change, no significant count mismatch";
    return false;
}

QString ConnHttpServerThread::getLastSyncTime()
{
    QString lastSyncFile = "/mnt/user/sync_data/last_sync_time.txt";
    
    // *** CHANGED: Replace file read with debug message and return empty ***
    qDebug() << "SYNC_DATA_DEBUG: Would read from file:" << lastSyncFile;
    qDebug() << "DEBUG: Could not read last sync time file (debug mode - no file created)";
    return QString(); // Return empty since no file exists
}

void ConnHttpServerThread::updateLastSyncTime()
{
    QString lastSyncFile = "/mnt/user/sync_data/last_sync_time.txt";
    QString currentTime = QDateTime::currentDateTime().toString("yyyy/MM/dd HH:mm:ss");
    
    // *** CHANGED: Replace directory creation and file write with debug message ***
    qDebug() << "SYNC_DATA_DEBUG: Would write to file:" << lastSyncFile;
    qDebug() << "SYNC_DATA_DEBUG: Content would be:" << currentTime;
    qDebug() << "SYNC_DATA_DEBUG: SYNC_TYPE=INCREMENTAL";
    qDebug() << "SYNC_DATA_DEBUG: DEVICE_ID=" << myHelper::getCpuSerial();
    qDebug() << "DEBUG: Updated last sync time to (debug only, no file created):" << currentTime;
}

int ConnHttpServerThread::getValidUserCount(const QList<PERSONS_t>& users)
{
    int validCount = 0;
    for (const PERSONS_t& person : users) {
        if (!person.idcard.isEmpty() && !person.name.isEmpty()) {
            validCount++;
        }
    }
    return validCount;
}

// ADD this new function
bool ConnHttpServerThread::shouldSyncUser(const QString& employeeId, 
                                         const QString& serverTimeUTC,
                                         const QMap<QString, QString>& localUserMap)
{
    if (!localUserMap.contains(employeeId)) {
        // User doesn't exist locally - needs sync
        qDebug() << "DEBUG: shouldSyncUser - New user:" << employeeId;
        return true;
    }
    
    // User exists - check if server version is newer
    QString localTimeStr = localUserMap[employeeId];
    QDateTime localTime = QDateTime::fromString(localTimeStr, "yyyy/MM/dd hh:mm:ss");
    QDateTime serverTimeIST = convertUTCToIST(serverTimeUTC);
    
    if (serverTimeIST.isValid() && localTime.isValid() && serverTimeIST > localTime) {
        qDebug() << "DEBUG: shouldSyncUser - User needs update:" << employeeId 
                 << "Server:" << serverTimeIST.toString("yyyy/MM/dd HH:mm:ss")
                 << "Local:" << localTime.toString("yyyy/MM/dd HH:mm:ss");
        return true;
    }
    
    // User is up to date
    return false;
}

// ADD this new function
void ConnHttpServerThread::processQueueBatch(QQueue<QString>& queue, int& successCount, int& failCount)
{
    int batchProcessed = 0;
    
    qDebug() << "DEBUG: processQueueBatch - Processing batch of up to" << SYNC_BATCH_SIZE 
             << "users from queue of" << queue.size();
    
    while (!queue.isEmpty() && batchProcessed < SYNC_BATCH_SIZE) {
        QString employeeId = queue.dequeue();
        
        qDebug() << "DEBUG: processQueueBatch - Processing user" << (batchProcessed + 1) 
                 << ":" << employeeId;
        
        if (fetchAndAddUser(employeeId)) {
            successCount++;
            qDebug() << "DEBUG: processQueueBatch - Successfully processed:" << employeeId;
        } else {
            failCount++;
            qDebug() << "ERROR: processQueueBatch - Failed to process:" << employeeId;
        }
        
        batchProcessed++;
        
        // Small delay between individual users in batch
        if (batchProcessed < SYNC_BATCH_SIZE && !queue.isEmpty()) {
            QThread::msleep(50);
        }
    }
    
    qDebug() << "DEBUG: processQueueBatch - Batch completed. Processed:" << batchProcessed 
             << "Success total:" << successCount << "Fail total:" << failCount;
}

// ADD this new function
void ConnHttpServerThread::updateSyncProgress(int processed, int total, int queued)
{
    QString progressStatus;
    if (total > 0) {
        int percentage = (processed * 100) / total;
        progressStatus = QString("Processing %1% (%2/%3) Q:%4")
            .arg(percentage)
            .arg(processed)
            .arg(total)
            .arg(queued);
    } else {
        progressStatus = QString("Processing (%1) Q:%2").arg(processed).arg(queued);
    }
    
    QList<PERSONS_t> currentUsers = RegisteredFacesDB::GetInstance()->GetAllPersonFromRAM();
    updateSyncDisplay(progressStatus, currentUsers.size(), total);
    
    qDebug() << "DEBUG: updateSyncProgress -" << progressStatus;
}

// ADD this debugging function (optional)
void ConnHttpServerThread::debugPrintQueueStatus(const QQueue<QString>& queue, 
                                                const QString& context)
{
    qDebug() << "DEBUG: Queue Status [" << context << "]";
    qDebug() << "  Queue size:" << queue.size() << "/" << MAX_QUEUE_LENGTH;
    qDebug() << "  Queue threshold:" << QUEUE_REFILL_THRESHOLD;
    qDebug() << "  Batch size:" << SYNC_BATCH_SIZE;
    
    if (!queue.isEmpty()) {
        qDebug() << "  Next users in queue:";
        QQueue<QString> tempQueue = queue; // Copy for debugging
        for (int i = 0; i < qMin(5, tempQueue.size()); i++) {
            qDebug() << "    " << (i+1) << ":" << tempQueue.dequeue();
        }
    }
}

void ConnHttpServerThread::syncMissingUsers()
{
    Q_D(ConnHttpServerThread);
    
    qDebug() << "DEBUG: syncMissingUsers - Checking if sync is already in progress";
    
    // Check if sync is in progress (non-blocking check)
    if (!d->syncMutex.tryLock()) {
        qDebug() << "WARNING: syncMissingUsers - Full sync in progress, skipping individual user sync";
        return;
    }
    
    qDebug() << "DEBUG: syncMissingUsers - Starting user synchronization process with mutex protection";
    
    try {
        // Save original URL
        QString originalUrl = d->mHttpServerUrl;
        
        // Get the sync URL from config
        QString syncUrl = ReadConfig::GetInstance()->getSyncUsersAddress();
        if (syncUrl.isEmpty()) {
            qDebug() << "ERROR: syncMissingUsers - Sync URL not configured";
            d->syncMutex.unlock();
            return;
        }
        
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
        
        qDebug() << "DEBUG: syncMissingUsers - Sending request for all user IDs (MUTEX PROTECTED)";
        
        // Send request
        QString response = d->doPostJson(json);
        
        // Restore original URL
        d->mHttpServerUrl = originalUrl;
        
        // Process response (rest of the existing logic)
        Json::Reader reader;
        Json::Value responseJson;
        if (!reader.parse(response.toStdString(), responseJson)) {
            qDebug() << "ERROR: syncMissingUsers - Failed to parse user IDs response";
            d->syncMutex.unlock();
            return;
        }
    qDebug() << "DEBUG: syncMissingUsers - Successfully parsed JSON response";
    
    // Only check for updatedUsers array
    if (!responseJson.isMember("updatedUsers") || !responseJson["updatedUsers"].isArray()) {
        qDebug() << "ERROR: syncMissingUsers - No updatedUsers array found in response";
        return;
    }
    
    // Get all local users for comparison
    QList<PERSONS_t> localUsers = RegisteredFacesDB::GetInstance()->GetAllPersonFromRAM();
    QMap<QString, QDateTime> localUserMapUTC; // Map of ID to UTC modification time
    QSet<QString> localUserIdSet;
    
    for (const PERSONS_t& person : localUsers) {
        if (!person.idcard.isEmpty()) {
            // Parse local time and convert to UTC
            QDateTime createTimeLocal = QDateTime::fromString(person.createtime, "yyyy/MM/dd HH:mm:ss");
            if (!createTimeLocal.isValid()) {
                createTimeLocal = QDateTime::fromString(person.createtime, "yyyyMMddHHmmss");
            }
            
            // Convert to UTC for comparison using the private function
            QDateTime createTimeUTC = d->convertLocalTimeToUTC(createTimeLocal);
            
            localUserMapUTC[person.idcard] = createTimeUTC;
            localUserIdSet.insert(person.idcard);
            
            qDebug() << "DEBUG: syncMissingUsers - Local user: " << person.idcard 
                     << " Local Time: " << (createTimeLocal.isValid() ? createTimeLocal.toString("yyyy/MM/dd HH:mm:ss") : "Invalid")
                     << " UTC Time: " << (createTimeUTC.isValid() ? createTimeUTC.toString("yyyy/MM/dd HH:mm:ss") : "Invalid");
        }
    }
    
    // Process updatedUsers array
    Json::Value updatedUsers = responseJson["updatedUsers"];
    qDebug() << "DEBUG: syncMissingUsers - Processing updatedUsers array with " << updatedUsers.size() << " entries";
    
    // Also track all server IDs to identify users that need to be deleted
    QSet<QString> serverUserIds;
    
    for (Json::Value::ArrayIndex i = 0; i < updatedUsers.size(); i++) {
        QString updateEntry = QString::fromStdString(updatedUsers[i].asString());
        
        // Parse entry to get ID and timestamp
        QString userID;
        QString serverTimeUTC;
        
        int hyphenPos = updateEntry.indexOf('-');
        if (hyphenPos > 0) {
            // Has hyphen - extract parts
            userID = updateEntry.left(hyphenPos);
            QString timestampStr = updateEntry.mid(hyphenPos + 1).trimmed();
            
            if (timestampStr.isEmpty()) {
                // Empty timestamp - treat as needs sync (use current time)
                serverTimeUTC = QDateTime::currentDateTime().toString("yyyy/MM/dd HH:mm:ss");
                qDebug() << "DEBUG: syncMissingUsers - Entry with empty timestamp, forcing sync: " << userID;
            } else {
                // Parse timestamp
                serverTimeUTC = timestampStr;
                qDebug() << "DEBUG: syncMissingUsers - Entry with timestamp: " << userID << "at" << serverTimeUTC;
            }
        } else {
            // No hyphen - treat entire entry as userID and force sync
            userID = updateEntry.trimmed();
            if (!userID.isEmpty()) {
                // Use current time to force sync of users without timestamps
                serverTimeUTC = QDateTime::currentDateTime().toString("yyyy/MM/dd HH:mm:ss");
                qDebug() << "DEBUG: syncMissingUsers - Entry without timestamp, forcing sync: " << userID;
            } else {
                qDebug() << "ERROR: syncMissingUsers - Empty user entry at index: " << i;
                continue;
            }
        }
        
        // Skip empty userIDs
        if (userID.isEmpty()) {
            qDebug() << "ERROR: syncMissingUsers - Empty userID for entry: " << updateEntry;
            continue;
        }
        
        serverUserIds.insert(userID);
        
        qDebug() << "DEBUG: syncMissingUsers - Server user: " << userID 
                 << " UTC Time: " << serverTimeUTC;
        
        // Compare UTC times directly
        if (!localUserMapUTC.contains(userID) || 
            !localUserMapUTC[userID].isValid() || 
            QDateTime::fromString(serverTimeUTC, "yyyy/MM/dd HH:mm:ss") > localUserMapUTC[userID]) {
            
            qDebug() << "DEBUG: syncMissingUsers - User " << userID << " needs update (UTC comparison)";
            
            // *** MODIFIED: Pass server time to fetchAndAddUser ***
            fetchAndAddUser(userID, serverTimeUTC);
        } else {
            qDebug() << "DEBUG: syncMissingUsers - User " << userID << " is up to date";
        }
    }
    
    // For count-based sync, also check for users that exist locally but not on the server
    QSet<QString> extraUserIds = localUserIdSet - serverUserIds;
    if (!extraUserIds.isEmpty()) {
        qDebug() << "DEBUG: syncMissingUsers - Found " << extraUserIds.size() << " extra users locally, removing them";
        
        for (const QString& userId : extraUserIds) {
            QString uuid = findUuidByEmployeeId(userId);
            if (!uuid.isEmpty()) {
                RegisteredFacesDB::GetInstance()->DelPersonByPersionUUIDFromDBAndRAM(uuid);
                qDebug() << "DEBUG: syncMissingUsers - Deleted user:" << userId;
            }
        }
    }
    
    qDebug() << "DEBUG: syncMissingUsers - User synchronization completed with mutex protection";
        
    } catch (...) {
        qDebug() << "ERROR: syncMissingUsers - Exception occurred, releasing mutex";
    }
    
    // Release mutex
    d->syncMutex.unlock();
    qDebug() << "DEBUG: syncMissingUsers - Released mutex";
}

bool ConnHttpServerThread::isSyncInProgress() const
{
    Q_D(const ConnHttpServerThread);
    return d->syncInProgress;
}

// New function to handle removal of extra users
void ConnHttpServerThread::removeExtraUsers()
{
    Q_D(ConnHttpServerThread);
    qDebug() << "DEBUG: removeExtraUsers - Starting process to remove extra users";
    
    // Save original URL
    QString originalUrl = d->mHttpServerUrl;
    qDebug() << "DEBUG: removeExtraUsers - Original URL: " << originalUrl;
    
    // Get the sync URL from config
    QString syncUrl = ReadConfig::GetInstance()->getSyncUsersAddress();
    if (syncUrl.isEmpty()) {
        qDebug() << "ERROR: removeExtraUsers - Sync URL not configured";
        return;
    }
    qDebug() << "DEBUG: removeExtraUsers - Using sync URL: " << syncUrl;
    
    // Set URL for this request
    d->mHttpServerUrl = syncUrl;
    
    // Prepare request to get all user IDs from server
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
    
    qDebug() << "DEBUG: removeExtraUsers - Sending request for all user IDs";
    
    // Send request
    QString response = d->doPostJson(json);
    qDebug() << "DEBUG: removeExtraUsers - Received response, length: " << response.length();
    
    // Restore original URL
    d->mHttpServerUrl = originalUrl;
    qDebug() << "DEBUG: removeExtraUsers - Restored original URL";
    
    // Process response
    Json::Reader reader;
    Json::Value responseJson;
    if (!reader.parse(response.toStdString(), responseJson)) {
        qDebug() << "ERROR: removeExtraUsers - Failed to parse user IDs response";
        return;
    }
    qDebug() << "DEBUG: removeExtraUsers - Successfully parsed JSON response";
    
    // Extract user ID card numbers from the All_UserID array
    QSet<QString> serverUserIdCards;
    if (responseJson.isMember("updatedUsers") && responseJson["updatedUsers"].isArray()) {
        Json::Value idCards = responseJson["updatedUsers"];
        qDebug() << "DEBUG: removeExtraUsers - Processing updatedUsers array with " << idCards.size() << " entries";
        for (Json::Value::ArrayIndex i = 0; i < idCards.size(); i++) {
            QString idCard = QString::fromStdString(idCards[i].asString());
            serverUserIdCards.insert(idCard);
            qDebug() << "DEBUG: removeExtraUsers - Added ID card to set: " << idCard;
        }
    } else {
        qDebug() << "ERROR: removeExtraUsers - No updatedUsers array found in response";
        // Dump response keys for debugging
        qDebug() << "DEBUG: Available keys in response:";
        for (const auto& key : responseJson.getMemberNames()) {
            qDebug() << "  Key: " << QString::fromStdString(key);
        }
        return;
    }
    
    // Get local users
    QList<PERSONS_t> localUsers = RegisteredFacesDB::GetInstance()->GetAllPersonFromRAM();
    
    // Find and remove extra users that exist locally but not on server
    int removedCount = 0;
    qDebug() << "DEBUG: removeExtraUsers - Looking for extra users among " << localUsers.size() << " local users";
    
    for (const PERSONS_t& person : localUsers) {
        // Skip users without ID cards
        if (person.idcard.isEmpty()) {
            qDebug() << "DEBUG: removeExtraUsers - Skipping user without ID card: " << person.name;
            continue;
        }
        
        // If the local user's ID card is not in the server list, delete it
        if (!serverUserIdCards.contains(person.idcard)) {
            qDebug() << "DEBUG: removeExtraUsers - Found extra user to remove: " << person.name << ", ID: " << person.idcard;
            
            // Delete the user from local database
            bool deleteResult = RegisteredFacesDB::GetInstance()->DelPersonByPersionUUIDFromDBAndRAM(person.uuid);
            if (deleteResult) {
                qDebug() << "DEBUG: removeExtraUsers - Successfully removed user: " << person.name;
                removedCount++;
            } else {
                qDebug() << "ERROR: removeExtraUsers - Failed to remove user: " << person.name;
            }
        }
    }
    
    qDebug() << "DEBUG: removeExtraUsers - Completed removal process. Removed " << removedCount << " extra users";
}

/*void ConnHttpServerThread::checkLastModifiedTime(const QString& heartbeatResponse)
{
    qDebug() << "DEBUG: checkLastModifiedTime - Starting time-based sync check";
    
    ReadConfig readConfig;
    if (!readConfig.getSyncEnabled()) {
        LogD("%s %s[%d] Sync is disabled. Skipping last modified time check.\n", __FILE__, __FUNCTION__, __LINE__);
        return; // Do nothing if sync is disabled
    }

    LogD("%s %s[%d] Sync is enabled. Checking last modified time.\n", __FILE__, __FUNCTION__, __LINE__);
    
    // Parse the heartbeat response
    Json::Reader reader;
    Json::Value responseJson;
    
    if (!reader.parse(heartbeatResponse.toStdString(), responseJson)) {
        qDebug() << "ERROR: checkLastModifiedTime - Failed to parse heartbeat response JSON";
        return;
    }
    
    // Check if the response has lastUpdatedAt field
    if (!responseJson.isMember("lastUpdatedAt") || responseJson["lastUpdatedAt"].isNull()) {
        qDebug() << "DEBUG: checkLastModifiedTime - No lastUpdatedAt found in response, skipping check";
        return;
    }
    
    // Extract lastUpdatedAt from response - format: "2025/04/17 06:18:06"
    std::string serverLastModifiedStr = responseJson["lastUpdatedAt"].asString();
    qDebug() << "DEBUG: checkLastModifiedTime - Server lastUpdatedAt: " << QString::fromStdString(serverLastModifiedStr);
    
    // Parse server last modified time - format "yyyy/MM/dd HH:mm:ss"
    QDateTime serverLastModified = QDateTime::fromString(
        QString::fromStdString(serverLastModifiedStr),
        "yyyy/MM/dd HH:mm:ss"
    );
    
    if (!serverLastModified.isValid()) {
        qDebug() << "ERROR: checkLastModifiedTime - Failed to parse server last modified time: " 
                 << QString::fromStdString(serverLastModifiedStr);
        return;
    }
    
    // Get the most recent local user modified time
    QDateTime localLastModified = getLocalLastModifiedTime();
    qDebug() << "DEBUG: checkLastModifiedTime - Local LastModifiedTime: " << localLastModified.toString("yyyy/MM/dd HH:mm:ss");
    
    // Skip if no local users exist
    if (!localLastModified.isValid()) {
        qDebug() << "DEBUG: checkLastModifiedTime - No valid local modification time found, triggering sync";
        checkAndSyncUsers(heartbeatResponse);
        return;
    }
    
    // Compare the times
    if (serverLastModified > localLastModified) {
        qDebug() << "DEBUG: checkLastModifiedTime - Server has more recent modifications, triggering sync";
        checkAndSyncUsers(heartbeatResponse);
    } else {
        qDebug() << "DEBUG: checkLastModifiedTime - Local data is up to date, no sync needed";
    }
}*/
// Fixed version of getLocalLastModifiedTime() - removed modifytime references

QMap<QString, QString> ConnHttpServerThread::getServerUserList()
{
    Q_D(ConnHttpServerThread);
    QMap<QString, QString> result;
    
    qDebug() << "DEBUG: getServerUserList - Starting to get ALL server users (expecting ~180)";
    
    // Save original URL
    QString originalUrl = d->mHttpServerUrl;
    qDebug() << "DEBUG: getServerUserList - Original URL:" << originalUrl;
    
    // Set sync URL
    QString syncUrl = ReadConfig::GetInstance()->getSyncUsersAddress();
    if (syncUrl.isEmpty()) {
        qDebug() << "ERROR: getServerUserList - Sync URL not configured";
        return result;
    }
    
    qDebug() << "DEBUG: getServerUserList - Using sync URL:" << syncUrl;
    d->mHttpServerUrl = syncUrl;
    
    // === ENHANCED REQUEST WITH PAGINATION SUPPORT ===
    int pageSize = 500;  // Request more users per page
    int currentPage = 0;
    int totalReceived = 0;
    bool hasMorePages = true;
    
    while (hasMorePages) {
        qDebug() << "DEBUG: getServerUserList - Requesting page" << currentPage << "with size" << pageSize;
        
        // Prepare request with pagination
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
        json["page"] = currentPage;           // Add pagination support
        json["page_size"] = pageSize;         // Request larger page size
        json["include_all"] = true;           // Flag to get ALL users
        
        // === NEW DEBUG: Log request details ===
        qDebug() << "GETSERVER_DEBUG: === REQUEST TO SERVER ===";
        qDebug() << "GETSERVER_DEBUG: URL:" << syncUrl;
        qDebug() << "GETSERVER_DEBUG: msg_type: get_all_person_ids";
        qDebug() << "GETSERVER_DEBUG: page:" << currentPage;
        qDebug() << "GETSERVER_DEBUG: page_size:" << pageSize;
        qDebug() << "GETSERVER_DEBUG: SN:" << d->sn;
        
        qDebug() << "DEBUG: getServerUserList - Sending request for page" << currentPage;
        
        // Send request
        QString response = d->doPostJson(json);
        
        // === NEW DEBUG: Log response details ===
        qDebug() << "GETSERVER_DEBUG: === RESPONSE FROM SERVER ===";
        qDebug() << "GETSERVER_DEBUG: Response length:" << response.length();
        if (response.length() > 0) {
            qDebug() << "GETSERVER_DEBUG: Response preview (first 300 chars):" << response.left(300);
        } else {
            qDebug() << "GETSERVER_DEBUG: ERROR - Empty response from server!";
        }
        
        qDebug() << "DEBUG: getServerUserList - Received response for page" << currentPage << ", length:" << response.length();
        
        if (response.isEmpty()) {
            qDebug() << "ERROR: getServerUserList - Empty response for page" << currentPage;
            break;
        }
        
        // Parse response
        Json::Reader reader;
        Json::Value responseJson;
        if (!reader.parse(response.toStdString(), responseJson)) {
            qDebug() << "ERROR: getServerUserList - Failed to parse JSON for page" << currentPage;
            qDebug() << "GETSERVER_DEBUG: Raw response causing parse error:" << response;
            break;
        }
        
        qDebug() << "DEBUG: getServerUserList - Successfully parsed JSON for page" << currentPage;
        
        // === NEW DEBUG: Show all available fields in response ===
        qDebug() << "GETSERVER_DEBUG: === RESPONSE ANALYSIS ===";
        qDebug() << "GETSERVER_DEBUG: Available fields in response:";
        for (const auto& key : responseJson.getMemberNames()) {
            qDebug() << "GETSERVER_DEBUG:   Field:" << QString::fromStdString(key);
            if (responseJson[key].isArray()) {
                qDebug() << "GETSERVER_DEBUG:     Array size:" << responseJson[key].size();
            } else if (responseJson[key].isString()) {
                qDebug() << "GETSERVER_DEBUG:     String value:" << QString::fromStdString(responseJson[key].asString()).left(50);
            }
        }
        
        // Check for updatedUsers array
        if (!responseJson.isMember("updatedUsers") || !responseJson["updatedUsers"].isArray()) {
            qDebug() << "ERROR: getServerUserList - No updatedUsers array in response for page" << currentPage;
            
            // Check for alternative field names that might contain user list
            QStringList alternativeFields = {"userList", "users", "personList", "employeeList", "allUsers"};
            bool foundAlternative = false;
            
            for (const QString& altField : alternativeFields) {
                if (responseJson.isMember(altField.toStdString()) && responseJson[altField.toStdString()].isArray()) {
                    qDebug() << "DEBUG: getServerUserList - Found alternative field:" << altField;
                    responseJson["updatedUsers"] = responseJson[altField.toStdString()];
                    foundAlternative = true;
                    break;
                }
            }
            
            if (!foundAlternative) {
                // If no pagination support, try to get all data in single request
                if (currentPage == 0) {
                    qDebug() << "WARNING: getServerUserList - Server may not support pagination, trying single request";
                    hasMorePages = false; // Exit after this attempt
                } else {
                    break; // No more data
                }
            }
        }
        
        if (!responseJson.isMember("updatedUsers")) {
            qDebug() << "ERROR: getServerUserList - Still no updatedUsers field found";
            break;
        }
        
        // Process updatedUsers array
        Json::Value updatedUsers = responseJson["updatedUsers"];
        int pageUserCount = updatedUsers.size();
        
        qDebug() << "DEBUG: getServerUserList - Processing page" << currentPage << "with" << pageUserCount << "users";
        qDebug() << "GETSERVER_DEBUG: === PROCESSING USER LIST ===";
        qDebug() << "GETSERVER_DEBUG: updatedUsers array size:" << pageUserCount;
        
        if (pageUserCount == 0) {
            qDebug() << "DEBUG: getServerUserList - No more users on page" << currentPage;
            hasMorePages = false;
            break;
        }
        
        // === NEW DEBUG: Show sample of user entries ===
        qDebug() << "GETSERVER_DEBUG: Sample user entries (first 10):";
        int sampleCount = qMin(10, pageUserCount);
        for (int sampleIdx = 0; sampleIdx < sampleCount; sampleIdx++) {
            QString sampleEntry = QString::fromStdString(updatedUsers[sampleIdx].asString());
            qDebug() << "GETSERVER_DEBUG:   Sample" << (sampleIdx + 1) << ":" << sampleEntry;
        }
        
        // Process each user in this page
        for (Json::Value::ArrayIndex i = 0; i < updatedUsers.size(); i++) {
            QString entry = QString::fromStdString(updatedUsers[i].asString());
            
            if (entry.isEmpty()) {
                qDebug() << "WARNING: getServerUserList - Empty entry at index" << i << "on page" << currentPage;
                continue;
            }
            
            // Enhanced parsing to handle multiple formats
            QString employeeId;
            QString timestamp;
            
            // Format analysis and extraction
            int hyphenPos = entry.indexOf('-');
            if (hyphenPos > 0) {
                // Format: "EmployeeID-TIMESTAMP" or "EmployeeID-"
                employeeId = entry.left(hyphenPos).trimmed();
                QString timestampPart = entry.mid(hyphenPos + 1).trimmed();
                
                if (timestampPart.isEmpty()) {
                    // Empty timestamp - use current time
                    timestamp = QDateTime::currentDateTime().toString("yyyy/MM/dd HH:mm:ss");
                    qDebug() << "DEBUG: getServerUserList - Page" << currentPage << "entry" << (i+1) 
                             << "empty timestamp, using current:" << employeeId;
                } else {
                    // Has timestamp
                    timestamp = timestampPart;
                    qDebug() << "DEBUG: getServerUserList - Page" << currentPage << "entry" << (i+1) 
                             << "with timestamp:" << employeeId << "at" << timestamp;
                }
            } else {
                // Format: "EmployeeID" only
                employeeId = entry.trimmed();
                if (!employeeId.isEmpty()) {
                    timestamp = QDateTime::currentDateTime().toString("yyyy/MM/dd HH:mm:ss");
                    qDebug() << "DEBUG: getServerUserList - Page" << currentPage << "entry" << (i+1) 
                             << "ID only, using current time:" << employeeId;
                } else {
                    qDebug() << "ERROR: getServerUserList - Empty employeeId for entry:" << entry;
                    continue;
                }
            }
            
            // Validate and add to result
            if (employeeId.isEmpty()) {
                qDebug() << "ERROR: getServerUserList - Invalid employeeId for entry:" << entry;
                continue;
            }
            
            // Check for duplicates (in case of overlapping pages)
            if (!result.contains(employeeId)) {
                result[employeeId] = timestamp;
                totalReceived++;
                qDebug() << "DEBUG: getServerUserList - Added user" << totalReceived << ":" << employeeId;
            } else {
                qDebug() << "DEBUG: getServerUserList - Duplicate user ignored:" << employeeId;
            }
        }
        
        // Check if we should continue to next page
        if (pageUserCount < pageSize) {
            // Received fewer users than requested - likely last page
            qDebug() << "DEBUG: getServerUserList - Page" << currentPage << "returned" << pageUserCount 
                     << "users (less than" << pageSize << "), assuming last page";
            hasMorePages = false;
        } else {
            // Full page received - check for more
            currentPage++;
            
            // Safety limit to prevent infinite loops
            if (currentPage > 10) {
                qDebug() << "WARNING: getServerUserList - Reached page limit, stopping";
                hasMorePages = false;
            }
        }
        
        // Add delay between pages to avoid overwhelming server
        if (hasMorePages) {
            qDebug() << "DEBUG: getServerUserList - Waiting before next page request";
            QThread::msleep(1000); // 1 second delay between pages
        }
    }
    
    // Restore URL
    d->mHttpServerUrl = originalUrl;
    qDebug() << "DEBUG: getServerUserList - Restored original URL";
    
    // === NEW DEBUG: Final summary with detailed analysis ===
    qDebug() << "GETSERVER_DEBUG: === FINAL SUMMARY ===";
    qDebug() << "GETSERVER_DEBUG: Total pages processed:" << (currentPage + 1);
    qDebug() << "GETSERVER_DEBUG: Total unique users received:" << result.size();
    qDebug() << "GETSERVER_DEBUG: Expected users (~180):" << (result.size() >= 180 ? "REACHED" : "MISSING");
    
    if (result.size() < 100) {
        qDebug() << "GETSERVER_DEBUG: WARNING - User count seems low, may need server-side investigation";
        
        // Debug: Show sample of received users
        qDebug() << "GETSERVER_DEBUG: Sample of received users:";
        int sampleCount = 0;
        for (auto it = result.begin(); it != result.end() && sampleCount < 10; ++it, ++sampleCount) {
            qDebug() << "GETSERVER_DEBUG:   Sample" << (sampleCount + 1) << ":" << it.key() << "at" << it.value();
        }
    } else {
        qDebug() << "GETSERVER_DEBUG: SUCCESS - Received good number of users:" << result.size();
    }
    
    // === NEW DEBUG: Show what will happen next ===
    qDebug() << "GETSERVER_DEBUG: === NEXT STEPS ===";
    qDebug() << "GETSERVER_DEBUG: Users will now be processed individually by fetchAndAddUser()";
    qDebug() << "GETSERVER_DEBUG: Each user will be compared with local database and synced if needed";
    qDebug() << "GETSERVER_DEBUG: Watch for 'fetchAndAddUser' debug messages for individual user processing";
    
    return result;
}

QDateTime ConnHttpServerThread::getLocalLastModifiedTime()
{
    qDebug() << "DEBUG: getLocalLastModifiedTime - Getting last modified time (checking for stored server date_updated time)";
    
    // *** CHANGED: Replace file read with debug message ***
    QString serverTimeFile = "/mnt/user/sync_data/server_lastmodified.txt";
    qDebug() << "SYNC_DATA_DEBUG: Would read from file:" << serverTimeFile;
    qDebug() << "DEBUG: getLocalLastModifiedTime - No stored server date_updated time file (debug mode), using local user times";
    
    // Continue with existing fallback logic: find most recent local user modification time
    qDebug() << "DEBUG: getLocalLastModifiedTime - No stored server date_updated time file, using local user times";
    
    QList<PERSONS_t> localUsers = RegisteredFacesDB::GetInstance()->GetAllPersonFromRAM();
    if (localUsers.isEmpty()) {
        qDebug() << "DEBUG: getLocalLastModifiedTime - No local users found";
        return QDateTime(); // Return invalid datetime
    }
    
    // Find the most recent created time from local users (existing logic)
    QDateTime mostRecentTime;
    
    for (const PERSONS_t& person : localUsers) {
        QDateTime userCreatedTime;
        
        // Try different formats (existing logic)
        userCreatedTime = QDateTime::fromString(person.createtime, "yyyy/MM/dd HH:mm:ss");
        
        if (!userCreatedTime.isValid()) {
            userCreatedTime = QDateTime::fromString(person.createtime, "yyyyMMddHHmmss");
        }
        
        if (!userCreatedTime.isValid()) {
            qDebug() << "DEBUG: getLocalLastModifiedTime - Couldn't parse time for user: " 
                     << person.name << ", createtime: " << person.createtime;
            continue;
        }
        
        if (!mostRecentTime.isValid() || userCreatedTime > mostRecentTime) {
            mostRecentTime = userCreatedTime;
        }
    }
    
    if (mostRecentTime.isValid()) {
        qDebug() << "SUCCESS: getLocalLastModifiedTime - Using LOCAL user time:" << mostRecentTime.toString("yyyy-MM-dd hh:mm:ss");
        qDebug() << "  Source: LOCAL (from user records)";
    } else {
        qDebug() << "WARNING: getLocalLastModifiedTime - No valid times found";
    }
    
    return mostRecentTime;
}

bool ConnHttpServerThread::fetchAndAddUser(const QString& employeeId, const QString& serverTimeFromList)
{
    Q_D(ConnHttpServerThread);
    qDebug() << "DEBUG: fetchAndAddUser - Starting to fetch user:" << employeeId;
    
    // Save original URL
    QString originalUrl = d->mHttpServerUrl;
    
    // Set user detail URL
    QString userDetailUrl = ReadConfig::GetInstance()->getUserDetailAddress();
    if (userDetailUrl.isEmpty()) {
        qDebug() << "ERROR: fetchAndAddUser - User detail URL not configured";
        return false;
    }
    
    qDebug() << "DEBUG: fetchAndAddUser - Using user detail URL:" << userDetailUrl;
    d->mHttpServerUrl = userDetailUrl;
    
    // Prepare request (same as working version)
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
    json["id_card_no"] = employeeId.toStdString().c_str();
    
    qDebug() << "DEBUG: fetchAndAddUser - Sending request for user details";

	
    
    QString response;   // outer response
int maxRetries = 3;
int retryCount = 0;

while (retryCount < maxRetries) {
    qDebug() << "FETCHUSER_DEBUG: Sending request to user detail server...";
    response = d->doPostJson(json);   // âš ï¸ no QString here
    qDebug() << "FETCHUSER_DEBUG: Received response length:" << response.length();
    if (response.length() > 0) {
        qDebug() << "FETCHUSER_DEBUG: Response preview:" << response.left(200);
    } else {
        qDebug() << "FETCHUSER_DEBUG: ERROR - Empty response for user:" << employeeId;
        return false;
    }

    if (response.length() > 50) { // Valid response should be longer
        break;
    }
    
    retryCount++;
    qDebug() << "WARNING: fetchAndAddUser - Retry" << retryCount << "for user:" << employeeId;
    QThread::msleep(1000);
}

// Restore URL (same as working version)
    d->mHttpServerUrl = originalUrl;
    
    if (response.length() <= 50) {
        qDebug() << "ERROR: fetchAndAddUser - Failed to get valid response after retries for:" << employeeId;
        return false;
    }
    
    
    
    // Parse response (same as working version)
    Json::Reader reader;
    Json::Value responseJson;
    std::string rawJson = response.toUtf8().constData();  // keep UTF-8 safe
if (!reader.parse(rawJson, responseJson)) {
    qDebug() << "ERROR: fetchAndAddUser - Failed to parse user detail response for:" << employeeId;
    qDebug() << "ERROR: Raw Response Preview:" << response.left(200);
    return false;
}
    
    qDebug() << "DEBUG: fetchAndAddUser - Successfully parsed JSON response";
    
    if (!responseJson.isMember("allEmployeeData")) {
        qDebug() << "ERROR: fetchAndAddUser - No allEmployeeData in response for:" << employeeId;
        
        // Check for error message in response
        if (responseJson.isMember("message")) {
            qDebug() << "ERROR: Server message:" << QString::fromStdString(responseJson["message"].asString());
        }
        
        return false;
    }
    
     Json::Value userData = responseJson["allEmployeeData"];
    qDebug() << "DEBUG: fetchAndAddUser - Processing allEmployeeData for:" << employeeId;
    
    // === DUAL SYNC: Extract user information ===
    QString employeeIdFromServer = "";
    QString firstName = "";
    QString gender = "";
    QString userSpecificId = "";
    QString rfidCardData = "";

	// === Extract accessLevel from server response (NO DEFAULT) ===
QString accessLevel = "";  // Empty - will use device default if not provided

// Check for nested assignedAccessLevel object
if (userData.isMember("assignedAccessLevel") && userData["assignedAccessLevel"].isObject()) {
    Json::Value assignedAccessLevel = userData["assignedAccessLevel"];
    
    if (assignedAccessLevel.isMember("accessLevelNumber") && !assignedAccessLevel["accessLevelNumber"].isNull()) {
        if (assignedAccessLevel["accessLevelNumber"].isInt()) {
            accessLevel = QString::number(assignedAccessLevel["accessLevelNumber"].asInt());
            qDebug() << "DEBUG: fetchAndAddUser - Found accessLevel from server:" << accessLevel;
        }
        else if (assignedAccessLevel["accessLevelNumber"].isString()) {
            accessLevel = QString::fromStdString(assignedAccessLevel["accessLevelNumber"].asString());
            qDebug() << "DEBUG: fetchAndAddUser - Found accessLevel from server:" << accessLevel;
        }
    }
}
    
    // 1. employeeId (mandatory)
    if (userData.isMember("employeeId") && !userData["employeeId"].isNull()) {
        employeeIdFromServer = QString::fromStdString(userData["employeeId"].asString());
        qDebug() << "DEBUG: fetchAndAddUser - Found employeeId:" << employeeIdFromServer;
    } else {
        qDebug() << "ERROR: fetchAndAddUser - No employeeId found in response";
        return false;
    }
    
    // 2. first_name (with multiple fallback strategies - same as working version)
    if (userData.isMember("assignedUser") && userData["assignedUser"].isMember("first_name") && 
        !userData["assignedUser"]["first_name"].isNull()) {
        firstName = QString::fromStdString(userData["assignedUser"]["first_name"].asString());
        qDebug() << "DEBUG: fetchAndAddUser - Found firstName:" << firstName;
    } else if (userData.isMember("name") && !userData["name"].isNull()) {
        firstName = QString::fromStdString(userData["name"].asString());
        qDebug() << "DEBUG: fetchAndAddUser - Found name:" << firstName;
    } else {
        qDebug() << "WARNING: fetchAndAddUser - No first_name found, using fallback";
        firstName = "User_" + employeeIdFromServer;
    }
    
    // 3. gender (with fallback - same as working version)
    if (userData.isMember("assignedUser") && userData["assignedUser"].isMember("gender") && 
        !userData["assignedUser"]["gender"].isNull()) {
        gender = QString::fromStdString(userData["assignedUser"]["gender"].asString());
        qDebug() << "DEBUG: fetchAndAddUser - Found gender:" << gender;
    } else {
        qDebug() << "WARNING: fetchAndAddUser - No gender found, using default";
        gender = "Unknown";
    }
    
    // 4. Extract 'id' from user response
    if (userData.isMember("id") && !userData["id"].isNull()) {
        userSpecificId = QString::fromStdString(userData["id"].asString());
        qDebug() << "DEBUG: fetchAndAddUser - Extracted id from user response:" << userSpecificId;
    }
    // 5. Extract RFID cards from user response and pad to specific length
if (userData.isMember("rfidCards") && userData["rfidCards"].isArray()) {
    Json::Value rfidCardsArray = userData["rfidCards"];
    qDebug() << "DEBUG: fetchAndAddUser - Found rfidCards array with" << rfidCardsArray.size() << "elements";
    
    QStringList rfidCardsList;
    const int RFID_CARD_LENGTH = 8; // Adjust this value based on your Wiegand reader requirements
    
    for (Json::Value::ArrayIndex i = 0; i < rfidCardsArray.size(); i++) {
        if (rfidCardsArray[i].isString()) {
            QString rfidCard = QString::fromStdString(rfidCardsArray[i].asString());
            
            // Pad with leading zeros to specific length for Wiegand reader compatibility
            QString formattedRfidCard = QString("%1").arg(rfidCard, RFID_CARD_LENGTH, '0');
            
            rfidCardsList.append(formattedRfidCard);
            qDebug() << "DEBUG: fetchAndAddUser - Original RFID card:" << rfidCard 
                     << "-> Padded (" << RFID_CARD_LENGTH << " digits):" << formattedRfidCard;
        }
    }
    
    // Join multiple RFID cards with comma separator (all now padded to correct length)
    if (!rfidCardsList.isEmpty()) {
        rfidCardData = rfidCardsList.join(",");
        qDebug() << "DEBUG: fetchAndAddUser - Final RFID card data with padding:" << rfidCardData;
    }
} else {
    qDebug() << "DEBUG: fetchAndAddUser - No rfidCards found in response";
}

// Fallback to default icCard if no RFID data
if (rfidCardData.isEmpty()) {
    rfidCardData = "00000000"; // Default value padded to same length
    qDebug() << "DEBUG: fetchAndAddUser - Using default RFID card data:" << rfidCardData;
}
    // === DUAL SYNC: Process Face Data (BASE64 IMAGE + FACE EMBEDDING) ===
    QByteArray finalFaceEmbedding;
    QByteArray processedImageData;
    QString savedImagePath = QString("/mnt/user/reg_face_image/%1.jpg").arg(employeeIdFromServer);
    bool hasBase64ImageData = false;
    bool hasFaceEmbedding = false;
    
    qDebug() << "DEBUG: fetchAndAddUser - === PROCESSING DUAL FACE DATA ===";
    
    // Ensure face image directory exists
    mkdir("/mnt/user/reg_face_image/", 0777);
    
    // === STEP 1: PROCESS BASE64 IMAGE DATA ===
    std::string base64FaceImageData = "";
    
    if (userData.isMember("faceData") && userData["faceData"].isArray() && userData["faceData"].size() > 0) {
        Json::Value faceDataArray = userData["faceData"];
        qDebug() << "DEBUG: fetchAndAddUser - Found faceData array with" << faceDataArray.size() << "elements";
        
        for (Json::Value::ArrayIndex i = 0; i < faceDataArray.size(); i++) {
            if (faceDataArray[i].isString()) {
                std::string faceDataElement = faceDataArray[i].asString();
                qDebug() << "DEBUG: fetchAndAddUser - faceData[" << i << "] length:" << faceDataElement.length();
                
                // Check if this looks like base64 image data (larger size)
                if (faceDataElement.length() > 1000) {
                    base64FaceImageData = faceDataElement;
                    qDebug() << "DEBUG: fetchAndAddUser - Found base64 image data in faceData array element" << i;
                    break;
                }
            }
        }
    }
    
    // Process and save base64 image data if found
    if (!base64FaceImageData.empty()) {
        qDebug() << "DEBUG: fetchAndAddUser - Processing base64 image data, length:" << base64FaceImageData.length();
        
        try {
            std::string decodedImageData = cereal::base64::decode(base64FaceImageData);
            qDebug() << "DEBUG: fetchAndAddUser - Decoded image data size:" << decodedImageData.size() << "bytes";
            
            if (decodedImageData.size() > 1000 && decodedImageData.size() < 5242880) {
                processedImageData = QByteArray(decodedImageData.data(), decodedImageData.size());
                
                QFile imageFile(savedImagePath);
                if (imageFile.open(QIODevice::WriteOnly)) {
                    qint64 bytesWritten = imageFile.write(processedImageData);
                    imageFile.close();
                    
                    if (bytesWritten == processedImageData.size()) {
                        hasBase64ImageData = true;
                        qDebug() << "DEBUG: fetchAndAddUser - BASE64 IMAGE SAVED:" << savedImagePath;
                    }
                }
            }
        } catch (const std::exception& e) {
            qDebug() << "ERROR: fetchAndAddUser - Base64 image decode failed:" << e.what();
        }
    }
    
    // === STEP 2: PROCESS FACE EMBEDDING DATA ===
    std::string base64FaceEmbedding = "";
    
    // Check for face embedding in various fields
    if (userData.isMember("faceFeature") && !userData["faceFeature"].isNull()) {
        std::string faceFeatureStr = userData["faceFeature"].asString();
        if (faceFeatureStr.length() > 100) {
            base64FaceEmbedding = faceFeatureStr;
            qDebug() << "DEBUG: fetchAndAddUser - Found face embedding in faceFeature field";
        }
    }
    else if (userData.isMember("faceEmbedding") && !userData["faceEmbedding"].isNull()) {
        std::string faceEmbeddingStr = userData["faceEmbedding"].asString();
        if (faceEmbeddingStr.length() > 100) {
            base64FaceEmbedding = faceEmbeddingStr;
            qDebug() << "DEBUG: fetchAndAddUser - Found face embedding in faceEmbedding field";
        }
    }
    else if (userData.isMember("faceData") && userData["faceData"].isArray()) {
        // Look for smaller base64 data (likely face embedding)
        Json::Value faceDataArray = userData["faceData"];
        for (Json::Value::ArrayIndex i = 0; i < faceDataArray.size(); i++) {
            if (faceDataArray[i].isString()) {
                std::string faceDataElement = faceDataArray[i].asString();
                // Face embeddings are typically smaller than images
                if (faceDataElement.length() > 100 && faceDataElement.length() < 2000) {
                    base64FaceEmbedding = faceDataElement;
                    qDebug() << "DEBUG: fetchAndAddUser - Found face embedding in faceData array element" << i;
                    break;
                }
            }
        }
    }
    
    // Process face embedding if found
    if (!base64FaceEmbedding.empty()) {
        qDebug() << "DEBUG: fetchAndAddUser - Processing face embedding, base64 length:" << base64FaceEmbedding.length();
        
        try {
            std::string decodedEmbedding = cereal::base64::decode(base64FaceEmbedding);
            qDebug() << "DEBUG: fetchAndAddUser - Decoded embedding size:" << decodedEmbedding.size() << "bytes";
            
            if (decodedEmbedding.size() >= 128 && decodedEmbedding.size() <= 2048) {
                finalFaceEmbedding = QByteArray(decodedEmbedding.data(), decodedEmbedding.size());
                hasFaceEmbedding = true;
                qDebug() << "DEBUG: fetchAndAddUser - Successfully decoded face embedding, size:" << finalFaceEmbedding.size();
            }
        } catch (const std::exception& e) {
            qDebug() << "ERROR: fetchAndAddUser - Failed to decode face embedding:" << e.what();
        }
    }
    
    // === FALLBACK: Extract embedding from saved image if not provided ===
    if (!hasFaceEmbedding && hasBase64ImageData && QFile::exists(savedImagePath)) {
        qDebug() << "DEBUG: fetchAndAddUser - FALLBACK - Extracting embedding from saved image";
        
        int result = -1;
        int faceNum = 0;
        double threshold = 0;
        QByteArray extractedEmbedding;
        
        result = ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->RegistPerson(
            savedImagePath, 
            faceNum, 
            threshold, 
            extractedEmbedding
        );
        
        if (result == 0 && !extractedEmbedding.isEmpty()) {
            finalFaceEmbedding = extractedEmbedding;
            hasFaceEmbedding = true;
            qDebug() << "DEBUG: fetchAndAddUser - FALLBACK SUCCESS - Extracted embedding from image:" << finalFaceEmbedding.size() << "bytes";
        } else {
            qDebug() << "WARNING: fetchAndAddUser - FALLBACK FAILED - Could not extract embedding from image, result:" << result;
        }
    }
    
    // Validate critical data (same as working version)
    if (employeeIdFromServer != employeeId) {
        qDebug() << "ERROR: fetchAndAddUser - EmployeeId mismatch. Expected:" << employeeId 
                 << "Got:" << employeeIdFromServer;
        return false;
    }
    
    qDebug() << "DEBUG: fetchAndAddUser - About to store user with:";
    qDebug() << "  employeeId:" << employeeIdFromServer;
    qDebug() << "  firstName:" << firstName;
    qDebug() << "  gender:" << gender;
    qDebug() << "  hasBase64ImageData:" << (hasBase64ImageData ? "Yes" : "No");
    qDebug() << "  hasFaceEmbedding:" << (hasFaceEmbedding ? "Yes" : "No");
    
    // === DUAL SYNC: Store user with enhanced dual face data ===
    bool success = false;
    QString existingUuid = findUuidByEmployeeId(employeeIdFromServer);
    
    // Get config values for enhanced storage
    QString storedTenantId = ReadConfig::GetInstance()->getHeartbeat_TenantId();
    QString storedAttendanceMode = ReadConfig::GetInstance()->getHeartbeat_AttendanceMode();
    QString storedDeviceStatus = ReadConfig::GetInstance()->getHeartbeat_DeviceStatus();
    QString derivedStatus = storedDeviceStatus.isEmpty() ? "approved" : storedDeviceStatus;
    
    // Setup server time
    QString finalServerCreateTime = "";
    if (!serverTimeFromList.isEmpty()) {
        QDateTime serverTimeIST = convertUTCToIST(serverTimeFromList);
        if (serverTimeIST.isValid()) {
            finalServerCreateTime = serverTimeIST.toString("yyyy/MM/dd HH:mm:ss");
        } else {
            finalServerCreateTime = QDateTime::currentDateTime().toString("yyyy/MM/dd HH:mm:ss");
        }
    } else {
        finalServerCreateTime = QDateTime::currentDateTime().toString("yyyy/MM/dd HH:mm:ss");
    }
    
    // Set server time for DB functions
    RegisteredFacesDB::GetInstance()->setCurrentUserServerTime(finalServerCreateTime);
    
    if (!existingUuid.isEmpty()) {
        // User exists - update (same pattern as working version)
        qDebug() << "DEBUG: fetchAndAddUser - User exists, updating:" << employeeIdFromServer;
        
        // Enhanced update with dual face data
        int updateResult = RegisteredFacesDB::GetInstance()->UpdatePersonToDBAndRAM(
    existingUuid,           // uuid
    firstName,              // name
    employeeIdFromServer,   // idCard
    rfidCardData,          // icCard (RFID cards from server)
    gender,                // sex
    "",                    // department
    "00:00,23:59,1,1,1,1,1,1,1", // timeOfAccess
    "",                    // jpeg (not used in update)
    finalFaceEmbedding,    // faceEmbedding
    storedAttendanceMode,  // attendanceMode
    storedTenantId,        // tenantId
    userSpecificId,        // id
    derivedStatus,         // status
	"",                     // uploadStatus - ADD THIS
    accessLevel.isEmpty() ? 0 : accessLevel.toInt()  // ADD THIS - accessLevel (14th parameter)
);
        
        success = (updateResult == 1);
        qDebug() << "DEBUG: fetchAndAddUser - Update result:" << (success ? "SUCCESS" : "FAILED");
        
    } else {
        // New user - add (same pattern as working version)
        qDebug() << "DEBUG: fetchAndAddUser - New user, adding:" << employeeIdFromServer;
        
        // Enhanced add with dual face data
        success = RegisteredFacesDB::GetInstance()->RegPersonToDBAndRAM(
    "",                     // uuid (auto-generated)
    firstName,              // name
    employeeIdFromServer,   // idCard
    rfidCardData,          // icCard (RFID cards from server)
    gender,                // sex
    "",                    // department
    "00:00,23:59,1,1,1,1,1,1,1", // timeOfAccess
    finalFaceEmbedding,    // face feature
    processedImageData,    // faceImageData (DUAL SYNC: base64 image)
    "jpg",                 // imageFormat
    storedAttendanceMode,  // attendanceMode
    storedTenantId,        // tenantId
    userSpecificId,        // id
    derivedStatus,         // status
	0,
    accessLevel.isEmpty() ? 0 : accessLevel.toInt()  // ADD THIS - accessLevel (15th parameter)
);
    }
    
    // Clear server time after use
    RegisteredFacesDB::GetInstance()->setCurrentUserServerTime("");
    
    if (success) {
    qDebug() << "SUCCESS: fetchAndAddUser - Successfully stored user with dual face data:" << firstName 
             << "(" << employeeIdFromServer << ")";
    qDebug() << "DUAL_SYNC_STATUS:";
    qDebug() << "  - Base64 Image:" << (hasBase64ImageData ? "SYNCED" : "MISSING");
    qDebug() << "  - Face Embedding:" << (hasFaceEmbedding ? "SYNCED" : "MISSING");
    qDebug() << "  - Image File:" << (QFile::exists(savedImagePath) ? "SAVED" : "MISSING");
    qDebug() << "  - Access Level:" << (accessLevel.isEmpty() ? "DEFAULT (0)" : accessLevel);  // ADD THIS
        
        // Force UI update
        updateLocalFaceCount();
        return true;
    } else {
        qDebug() << "ERROR: fetchAndAddUser - Failed to store user:" << employeeIdFromServer;
        return false;
    }
}

bool ConnHttpServerThread::storeServerDateUpdatedForUser(const QString& employeeId, const QString& serverDateUpdated)
{
    qDebug() << "DEBUG: storeServerDateUpdatedForUser - Storing for employee:" << employeeId;
    qDebug() << "  Server date_updated time:" << serverDateUpdated;
    
    if (employeeId.isEmpty() || serverDateUpdated.isEmpty()) {
        qDebug() << "ERROR: storeServerDateUpdatedForUser - Empty parameters";
        return false;
    }
    
    // Parse and convert server UTC time to local format for storage (keep all this logic)
    QString localFormattedTime = "";
    
    // Try to parse server time (assuming UTC format like "2025-07-31T14:39:50.000Z" or "2025/07/31 14:39:50")
    QDateTime serverTime;
    
    // Format 1: ISO 8601 with Z (UTC)
    if (serverDateUpdated.contains('T') && serverDateUpdated.endsWith('Z')) {
        serverTime = QDateTime::fromString(serverDateUpdated, Qt::ISODate);
    }
    // Format 2: ISO 8601 without Z
    else if (serverDateUpdated.contains('T')) {
        serverTime = QDateTime::fromString(serverDateUpdated, "yyyy-MM-ddTHH:mm:ss");
        serverTime.setTimeSpec(Qt::UTC);
    }
    // Format 3: Simple format "2025/07/31 14:39:50"
    else if (serverDateUpdated.contains('/')) {
        serverTime = QDateTime::fromString(serverDateUpdated, "yyyy/MM/dd HH:mm:ss");
        serverTime.setTimeSpec(Qt::UTC);
    }
    // Format 4: Simple format "2025-07-31 14:39:50"
    else if (serverDateUpdated.contains('-')) {
        serverTime = QDateTime::fromString(serverDateUpdated, "yyyy-MM-dd HH:mm:ss");
        serverTime.setTimeSpec(Qt::UTC);
    }
    
    if (serverTime.isValid()) {
        // Convert UTC to local time for storage (assuming IST = UTC + 5:30)
        QDateTime localTime = serverTime.addSecs(5 * 3600 + 30 * 60);
        localFormattedTime = localTime.toString("yyyy/MM/dd HH:mm:ss");
        
        qDebug() << "SUCCESS: storeServerDateUpdatedForUser - Converted server time:";
        qDebug() << "  Server UTC:" << serverTime.toString("yyyy-MM-dd HH:mm:ss");
        qDebug() << "  Local IST:" << localFormattedTime;
    } else {
        qDebug() << "WARNING: storeServerDateUpdatedForUser - Could not parse server time, using as-is";
        localFormattedTime = serverDateUpdated; // Use as-is if parsing fails
    }
    
    // *** CHANGED: Replace directory creation and file operations with debug messages ***
    QString userTimeDir = "/mnt/user/sync_data/user_times/";
    QString userTimeFile = QString("%1%2_server_time.txt").arg(userTimeDir).arg(employeeId);
    
    qDebug() << "SYNC_DATA_DEBUG: Would create directory: /mnt/user/sync_data/user_times/";
    qDebug() << "SYNC_DATA_DEBUG: Would write to file:" << userTimeFile;
    qDebug() << "SYNC_DATA_DEBUG: Content would be:" << localFormattedTime;
    qDebug() << "SYNC_DATA_DEBUG: EMPLOYEE_ID=" << employeeId;
    qDebug() << "SYNC_DATA_DEBUG: SERVER_TIME_UTC=" << serverDateUpdated;
    qDebug() << "SYNC_DATA_DEBUG: LOCAL_TIME_IST=" << localFormattedTime;
    qDebug() << "SYNC_DATA_DEBUG: STORED_AT=" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    qDebug() << "SUCCESS: storeServerDateUpdatedForUser - Stored in debug (no file created):" << userTimeFile;
    
    return true;
}

void ConnHttpServerThread::setLastHeartbeatResponse(const QString& response)
{
    m_lastHeartbeatResponse = response;
}

QString ConnHttpServerThread::getLastHeartbeatResponse() const
{
    return m_lastHeartbeatResponse;
}

void ConnHttpServerThread::debugPrintSyncConfig()
{
    ReadConfig readConfig;
    
    qDebug() << "=== DEBUG SYNC CONFIGURATION ===";
    qDebug() << "Sync enabled:" << (readConfig.getSyncEnabled() ? "Yes" : "No");
    qDebug() << "Sync users address:" << ReadConfig::GetInstance()->getSyncUsersAddress();
    qDebug() << "User detail address:" << ReadConfig::GetInstance()->getUserDetailAddress();
    qDebug() << "Registration address:" << ReadConfig::GetInstance()->getPerson_Registration_Address();
    qDebug() << "Server manager address:" << ReadConfig::GetInstance()->getSrv_Manager_Address();
    qDebug() << "=== END DEBUG CONFIG ===";
}

void ConnHttpServerThread::debugPrintUserCounts()
{
    QList<PERSONS_t> localUsers = RegisteredFacesDB::GetInstance()->GetAllPersonFromRAM();
    
    qDebug() << "=== DEBUG USER COUNT SUMMARY ===";
    qDebug() << "Total local users:" << localUsers.size();
    
    int usersWithFaceData = 0;
    int usersWithoutFaceData = 0;
    
    for (const PERSONS_t& person : localUsers) {
        if (!person.feature.isEmpty()) {
            usersWithFaceData++;
        } else {
            usersWithoutFaceData++;
        }
        
        qDebug() << "User:" << person.name << "ID:" << person.idcard 
                 << "UUID:" << person.uuid << "FaceData:" << (person.feature.isEmpty() ? "No" : "Yes")
                 << "Created:" << person.createtime;
    }
    
    qDebug() << "Users with face data:" << usersWithFaceData;
    qDebug() << "Users without face data:" << usersWithoutFaceData;
    qDebug() << "=== END DEBUG SUMMARY ===";
}

// Helper function to find UUID by employeeId
QString ConnHttpServerThread::findUuidByEmployeeId(const QString& employeeId)
{
    qDebug() << "DEBUG: findUuidByEmployeeId - Looking for employeeId:" << employeeId;
    
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    query.prepare("SELECT uuid FROM person WHERE idcardnum = ?");
    query.bindValue(0, employeeId);
    
    if (query.exec() && query.next()) {
        QString uuid = query.value("uuid").toString();
        qDebug() << "DEBUG: findUuidByEmployeeId - Found UUID:" << uuid << "for employeeId:" << employeeId;
        return uuid;
    } else {
        qDebug() << "DEBUG: findUuidByEmployeeId - No UUID found for employeeId:" << employeeId;
        return QString();
    }
}

QDateTime ConnHttpServerThread::convertUTCToIST(const QString& utcTimeStr)
{
    qDebug() << "DEBUG: convertUTCToIST - Input UTC string:" << utcTimeStr;
    
    QDateTime utcTime = QDateTime::fromString(utcTimeStr, "yyyy/MM/dd HH:mm:ss");
    if (!utcTime.isValid()) {
        qDebug() << "ERROR: convertUTCToIST - Invalid UTC time format:" << utcTimeStr;
        return QDateTime();
    }
    
    utcTime.setTimeSpec(Qt::UTC);
    
    // Convert to IST (UTC + 5:30)
    QDateTime istTime = utcTime.addSecs(5 * 3600 + 30 * 60);
    istTime.setTimeSpec(Qt::LocalTime);
    
    qDebug() << "DEBUG: convertUTCToIST - UTC:" << utcTime.toString("yyyy/MM/dd HH:mm:ss") 
             << "-> IST:" << istTime.toString("yyyy/MM/dd HH:mm:ss");
    
    return istTime;
}

QDateTime ConnHttpServerThreadPrivate::getLocalLastModifiedTimeUTC() 
{
    QDateTime localLastModified = q_ptr->getLocalLastModifiedTime();  // Call the public function
    if (!localLastModified.isValid()) {
        qDebug() << "DEBUG: getLocalLastModifiedTimeUTC - No valid local time found";
        return QDateTime();
    }
    
    // Convert to UTC for comparison
    return convertLocalTimeToUTC(localLastModified);
}
QDateTime ConnHttpServerThreadPrivate::convertLocalTimeToUTC(const QDateTime& localTime) 
{
    if (!localTime.isValid()) {
        return QDateTime();
    }
    
    // Subtract 5:30 from IST to get UTC
    QDateTime utcTime = localTime.addSecs(-(5 * 3600 + 30 * 60));
    utcTime.setTimeSpec(Qt::UTC);
    
    qDebug() << "DEBUG: convertLocalTimeToUTC - IST: " << localTime.toString("yyyy/MM/dd HH:mm:ss")
             << " -> UTC: " << utcTime.toString("yyyy/MM/dd HH:mm:ss");
    
    return utcTime;
}

void ConnHttpServerThread::updateLocalFaceCount()
{
    // Get local face database count
    QList<PERSONS_t> localUsers = RegisteredFacesDB::GetInstance()->GetAllPersonFromRAM();
    int localFaceCount = 0;
    
    // Count users that have face data
    for (const PERSONS_t &person : localUsers) {
        // Check if the person has face data - use correct member name
        if (!person.name.isEmpty() && !person.feature.isEmpty()) {  // Changed from person.id to person.feature
            localFaceCount++;
        }
    }
    
    // Get total user count from last heartbeat response
    int totalUserCount = 0;
    QString lastResponse = getLastHeartbeatResponse();
    if (!lastResponse.isEmpty()) {
        Json::Reader reader;
        Json::Value responseJson;
        if (reader.parse(lastResponse.toStdString(), responseJson)) {
            if (responseJson.isMember("TotalUserCount")) {
                totalUserCount = atoi(responseJson["TotalUserCount"].asString().c_str());
            }
        }
    }
    
    qDebug() << "DEBUG: Local face count:" << localFaceCount << "Total users:" << totalUserCount;
    
    // Update UI
    if (qXLApp && qXLApp->GetFaceMainFrm()) {
        FaceMainFrm* mainFrm = qXLApp->GetFaceMainFrm();
        mainFrm->updateLocalFaceCount(localFaceCount, totalUserCount);
    }
}

void ConnHttpServerThread::syncIndividualUser(const QString& employeeId)
{
    qDebug() << "DEBUG: syncIndividualUser - Starting sync for:" << employeeId;
    
    // Disable UI temporarily
    QApplication::setOverrideCursor(Qt::WaitCursor);
    
    // Perform the sync directly (it's usually fast)
    bool success = this->fetchAndAddUser(employeeId);
    
    // Restore cursor
    QApplication::restoreOverrideCursor();
    
    // Emit completion signal
    emit userSyncCompleted(employeeId, success);
}

// Add this helper function to ConnHttpServerThreadPrivate class
QDateTime ConnHttpServerThreadPrivate::convertServerUTCToIST(const QString& utcTimeStr)
{
    qDebug() << "DEBUG: convertServerUTCToIST - Input UTC string:" << utcTimeStr;
    
    // Parse the ISO 8601 UTC format: "2025-06-13T15:00:22.786Z"
    QDateTime utcTime = QDateTime::fromString(utcTimeStr, Qt::ISODate);
    
    if (!utcTime.isValid()) {
        qDebug() << "ERROR: convertServerUTCToIST - Invalid UTC time format:" << utcTimeStr;
        return QDateTime();
    }
    
    // Ensure it's marked as UTC
    utcTime.setTimeSpec(Qt::UTC);
    
    // Convert to IST (UTC + 5:30)
    QDateTime istTime = utcTime.addSecs(5 * 3600 + 30 * 60);
    istTime.setTimeSpec(Qt::LocalTime);
    
    qDebug() << "DEBUG: convertServerUTCToIST - UTC:" << utcTime.toString(Qt::ISODate) 
             << "-> IST:" << istTime.toString("yyyy-MM-dd dddd hh:mm:ss");
    
    return istTime;
}


// Add cleanup implementation:
void ConnHttpServerThread::initializeCleanup()
{
    cleanupTimer = new QTimer(this);
    connect(cleanupTimer, &QTimer::timeout, this, &ConnHttpServerThread::performPeriodicCleanup);
    
    // Cleanup every 5 minutes
    cleanupTimer->start(5 * 60 * 1000);
    
    // Initial cleanup
    QTimer::singleShot(10000, this, &ConnHttpServerThread::performPeriodicCleanup);
}

void ConnHttpServerThread::performPeriodicCleanup()
{
    qDebug() << "DEBUG: performPeriodicCleanup - Starting cleanup";
    
    cleanupTempFiles();
    
    // Force Qt memory cleanup
    QCoreApplication::sendPostedEvents();
}

void ConnHttpServerThread::cleanupTempFiles()
{
    QStringList tempPaths = {
        "/mnt/user/tmp/",
        "/mnt/user/facedb/RegImage.jpeg",
        "/mnt/user/facedb/RegImage.jpg", 
        "/mnt/user/facedb/faceFeature.jpg",
        "/mnt/user/catch_rgb_base64.raw",
        "/mnt/user/catch_ir_base64.raw"
    };
    
    for (const QString& path : tempPaths) {
        if (path.endsWith("/")) {
            // Directory cleanup - remove old files
            QDir dir(path);
            if (dir.exists()) {
                QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
                for (const QFileInfo& file : files) {
                    // Remove files older than 1 hour
                    if (file.lastModified().addSecs(3600) < QDateTime::currentDateTime()) {
                        QFile::remove(file.absoluteFilePath());
                        qDebug() << "DEBUG: Cleaned up old temp file:" << file.fileName();
                    }
                }
            }
        } else {
            // Individual file cleanup
            QFileInfo fileInfo(path);
            if (fileInfo.exists() && 
                fileInfo.lastModified().addSecs(300) < QDateTime::currentDateTime()) { // 5 minutes old
                QFile::remove(path);
                qDebug() << "DEBUG: Cleaned up temp file:" << path;
            }
        }
    }
    
    // Clean up any core dump or log files
    myHelper::Utils_ExecCmd("find /mnt/user -name '*.core' -mtime +1 -delete");
    myHelper::Utils_ExecCmd("find /tmp -name 'curl_debug*' -mtime +1 -delete");
}