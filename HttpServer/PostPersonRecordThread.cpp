#include "PostPersonRecordThread.h"
#include "Version.h"
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
#include "RkNetWork/NetworkControlThread.h"
#include "json-cpp/json.h"
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
#include <QtCore/QDebug>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDatabase>
#include <QTimer>
#include <QDateTime>
#include <QSqlError>

class PostPersonRecordThreadPrivate
{
	Q_DECLARE_PUBLIC(PostPersonRecordThread)
public:
	PostPersonRecordThreadPrivate(PostPersonRecordThread *dd);
	bool doDownloadFile(QString url, QString dist);
	int doPostPersonRecord();

	QString mSN;
	QString mPostPersonRecordServerUrl;
	QString mPostPersonRecordServerPassword;

private:
	mutable QMutex sync;
	QWaitCondition pauseCond;
	int threadDelay;
	QList<QString> picList;
	 bool isManualPushInProgress;
    QMutex manualPushMutex;
    QDateTime manualPushStartDateTime;
    QDateTime manualPushEndDateTime;


private:
	QString doPostJson(Json::Value json);
	int doManualPushRecords(const QDateTime &startDateTime, const QDateTime &endDateTime);
	

private:
	PostPersonRecordThread * const q_ptr;
};

typedef struct _MESSAGE_HEADER_S
{
	std::string msg_type;
	std::string sn;
	std::string timestamp;
	std::string password;
	std::string cmd_id;
} MESSAGE_HEADER_S;

static std::string getTime()
{
	time_t timep;
	time(&timep);
	char tmp[64];
	strftime(tmp, sizeof(tmp), "%Y%m%d%H%M%S", localtime(&timep));
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

static size_t download_write_data(void *ptr, size_t size, size_t nmemb, void* userdata)
{
	int fd = (int) userdata;
	if (fd <= 0)
	{
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
		pemsize = YNH_LJX::RkUtils::Utils_getFileSize("/etc/ssl/cacert.pem");
		//pemsize = YNH_LJX::RkUtils::Utils_getFileSize("/isc/cacert.pem");
		mypem = (char*) malloc(pemsize);
		if (mypem == ISC_NULL)
		{
			return rv;
		}
		int fd = open("/etc/ssl/cacert.pem", O_RDONLY, 0666);
		//int fd = open("/isc/cacert.pem", O_RDONLY, 0666);
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

PostPersonRecordThreadPrivate::PostPersonRecordThreadPrivate(PostPersonRecordThread *dd) :
		q_ptr(dd), //
		threadDelay(1) //
{
	mPostPersonRecordServerUrl = "";
	mPostPersonRecordServerPassword = "";

	qRegisterMetaType<IdentifyFaceRecord_t>("IdentifyFaceRecord_t");
	QObject::connect(q_func(), &PostPersonRecordThread::sigAppRecordData, q_func(), &PostPersonRecordThread::slotAppRecordData);
}

QString PostPersonRecordThreadPrivate::doPostJson(Json::Value json)
{
	std::string jsonStr = "";
	std::string resultString = "";
	if (json.size() > 0)
	{
		Json::FastWriter fast_writer;
		jsonStr = fast_writer.write(json);
		jsonStr = jsonStr.substr(0, jsonStr.length() - 1);
		//jsonStr = std::string("json=") + jsonStr;
	}
#if 0	
	CURL *curl;
	CURLcode res;
	curl = curl_easy_init();
	if (curl)
	{
		//printf("%s %s[%d] jsonStr %s \n", __FILE__, __FUNCTION__, __LINE__, jsonStr.c_str());
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
		curl_easy_setopt(curl, CURLOPT_URL, mPostPersonRecordServerUrl.toStdString().c_str());
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
		if (mPostPersonRecordServerUrl.startsWith("https://"))
		{
			curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION, *sslctx_function);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, 6);
		}
		res = curl_easy_perform(curl);
	}
	curl_easy_cleanup(curl);
#endif 
#if 1 //ok
	//std::string cmd = "/usr/bin/curl -H \"Content-Type:application/json;charset=UTF-8\" ";
	std::string cmd = "/usr/bin/curl --cacert /isc/cacert.pem -H \"Content-Type:application/json;charset=UTF-8\" ";
	cmd += "  ";
	cmd += mPostPersonRecordServerUrl.toStdString().c_str();
	cmd += "  --connect-timeout 1 -s -X POST ";
	cmd += "  -d '";
	cmd += jsonStr.c_str();
	cmd += "'";
	std::string rets = "";
	FILE* pFile = popen(cmd.c_str(), "r");
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
		if (rets.length() > 10)
		{
			resultString = rets;
		}
		if (resultString.size() > 3)
	{
		
	} else {
		
	}

	}
#endif 
		
#if 0	
	CURL *curl;
	CURLcode res;
	curl = curl_easy_init();
	if (curl)
	{
		char errbuf[CURL_ERROR_SIZE];
		//printf("%s %s[%d] jsonStr %s \n", __FILE__, __FUNCTION__, __LINE__, jsonStr.c_str());
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
		curl_easy_setopt(curl, CURLOPT_URL, mPostPersonRecordServerUrl.toStdString().c_str());
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
		if (mPostPersonRecordServerUrl.startsWith("https://"))
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
	}
	curl_easy_cleanup(curl);
#endif 		
	//if (resultString.size() > 3)
	{

	}
	return QString::fromStdString(resultString);
}

int PostPersonRecordThreadPrivate::doPostPersonRecord()
{
	int delay = 5;
	Json::Reader reader;
	Json::Value root;
	MESSAGE_HEADER_S stMsg;

	if (mPostPersonRecordServerUrl.size() < 3)
	{
		
		return delay;
	}

	std::string start_time = "1979/01/01 00:00:01";
	std::string end_time = getTime3();
	QString resultMsg = "";

	int nPerPage = 1;
	int nTotalCount = 0;
	int nDataCount = 0;
	QList<IdentifyFaceRecord_t> list;
	QDateTime startDateTime = QDateTime::fromString(start_time.c_str(), "yyyy/MM/dd hh:mm:ss");
	QDateTime endDateTime = QDateTime::fromString(end_time.c_str(), "yyyy/MM/dd hh:mm:ss");

	nTotalCount = PersonRecordToDB::GetInstance()->GetPersonRecordTotalNumByDateTime(startDateTime, endDateTime, true);
	list = PersonRecordToDB::GetInstance()->GetPersonRecordDataByDateTime(1, nPerPage, startDateTime, endDateTime, true);
	nDataCount = list.size();
	Json::Value json;
	std::string timestamp;
	std::string password;
	timestamp = getTime();
	password = mPostPersonRecordServerPassword.toStdString() + timestamp;
	password = md5sum(password);
	password = md5sum(password);
	transform(password.begin(), password.end(), password.begin(), ::tolower);
	json["msg_type"] = "post_offine_record";
	json["sn"] = mSN.toStdString().c_str();
	json["timestamp"] = timestamp.c_str();
	json["password"] = password.c_str();
	json["cmd_id"] = "";
	json["dev_name"] = "";
	json["location"] = "";
	json["message"] = "post_offine_record";

	json["per_page"] = "0";
	json["total_page"] = "0";
	json["result"] = "0";
	json["success"] = "0";
	long rid = 0;
	if (nTotalCount > 0 && nDataCount > 0)
	{
		for (int i = 0; i < nDataCount; i++)
		{
			if (i >= list.size())
			{
				break;
			}
			auto &t = list[i];

			int nArleadyPostPersonRecordIndex = -1;
			for (int i = 0; i < picList.size(); i++)
			{
				{
					nArleadyPostPersonRecordIndex = i;
					break;
				}
			}
			if (nArleadyPostPersonRecordIndex > -1)
			{
				picList.removeAt(nArleadyPostPersonRecordIndex);
				PersonRecordToDB::GetInstance()->UpdatePersonRecordUploadFlag(t.rid, true);
				return delay;
			}

			Json::Value data;
			rid = t.rid;

			if (t.FaceImgPath != "/mnt/user/face_crop_image") {
				std::string base64Data = fileToBase64String(t.FaceImgPath.toStdString().c_str());
				data["crop_data"] = base64Data;
				if (base64Data.length() > 50) {
				}
			}

			data["card_no"] = t.face_iccardnum.toStdString().c_str();
			data["male"] = t.face_sex.toStdString().c_str();
			data["id_card_no"] = t.face_idcardnum.toStdString().c_str();
			data["person_name"] = t.face_name.toStdString().c_str();
			data["time"] = t.createtime.toString("yyyy/MM/dd hh:mm:ss").toStdString().c_str();
			data["person_uuid"] = t.face_uuid.toStdString().c_str();
			data["Identifyed"] = std::to_string(t.Identifyed).c_str();
			data["rID"] = std::to_string(rid).c_str();
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

	bool isNeedSleep = false;
	bool isPostData = false;

	if (nTotalCount > 0 && nDataCount > 0)
	{
		isPostData = true;
	} else
	{
		isNeedSleep = true;
	}
	if (isPostData)
	{
		
		QString ret = doPostJson(json);
		if (ret.size() > 0)
		{
			if (reader.parse(ret.toStdString(), root))
			{
				stMsg.msg_type = root["msg_type"].asString();
				stMsg.sn = root["sn"].asString();
				stMsg.timestamp = root["timestamp"].asString();
				stMsg.password = root["password"].asString();
				stMsg.cmd_id = root["cmd_id"].asString();
			}
		} else
		{
			isNeedSleep = true;
		}
		if (stMsg.msg_type == std::string("post_offine_record_done"))
		{

			QString rIDs = QString::fromStdString(root["rID"].asString());
			QStringList sections = rIDs.split(",");
			if (sections.size() > 0)
			{
				for (int i = 0; i < sections.size(); i++)
				{
					int rid = sections[i].toInt();
					PersonRecordToDB::GetInstance()->UpdatePersonRecordUploadFlag(rid, true);
				}
			} else
			{
				PersonRecordToDB::GetInstance()->UpdatePersonRecordUploadFlag(rIDs.toInt(), true);
			}
#if 0			
			for (int i2 = 0; i2< list.size(); i2++)
			{
				if (i2 >= list.size())
				{
					break;
				}
				auto &t2 = list[i2];				
			}
#endif 			

		}

	}
	if (isNeedSleep)
	{
		delay = 60;
	}
	return delay;
}

PostPersonRecordThread::PostPersonRecordThread(QObject *parent) :
		QThread(parent), //
		d_ptr(new PostPersonRecordThreadPrivate(this))
{
	this->start();
}

PostPersonRecordThread::~PostPersonRecordThread()
{
	Q_D(PostPersonRecordThread);
	this->requestInterruption();
	d->pauseCond.wakeOne();

	this->quit();
	this->wait();
}

void PostPersonRecordThread::run()
{
    Q_D(PostPersonRecordThread);
    sleep(20);
    
    // Set thread to lowest priority for manual push operations
    setPriority(QThread::LowestPriority);
    
    int storageCheckCounter = 0;
    const int STORAGE_CHECK_INTERVAL = 10;
    
    while (true)
    {
        d->sync.lock();
        int delay = 10;
        d->mSN = myHelper::getCpuSerial();
        d->mPostPersonRecordServerUrl = ReadConfig::GetInstance()->getPost_PersonRecord_Address();
        d->mPostPersonRecordServerPassword = ReadConfig::GetInstance()->getPost_PersonRecord_Password();

        // Check if manual push is requested
        bool manualPushRequested = false;
        QDateTime pushStart, pushEnd;
        
        {
            QMutexLocker locker(&d->manualPushMutex);
            if (d->isManualPushInProgress)
            {
                manualPushRequested = true;
                pushStart = d->manualPushStartDateTime;
                pushEnd = d->manualPushEndDateTime;
            }
        }

        // Process manual push if requested (HIGHEST PRIORITY)
        if (manualPushRequested)
        {
            LogD("%s %s[%d] Processing manual push request...", __FILE__, __FUNCTION__, __LINE__);
            delay = d->doManualPushRecords(pushStart, pushEnd);
            
            // Reset manual push state
            QMutexLocker locker(&d->manualPushMutex);
            d->isManualPushInProgress = false;
            
            // After manual push, wait longer before normal operations
            delay = 30;
        }
        else
        {
            // Normal operations - periodic storage check
            //storageCheckCounter++;
            //if (storageCheckCounter >= STORAGE_CHECK_INTERVAL) {
               // LogD("%s %s[%d] Performing periodic storage check...", __FILE__, __FUNCTION__, __LINE__);
                //d->checkAndCleanupStorage();
                //storageCheckCounter = 0;
            //}

            // Regular automatic push
            if (d->mPostPersonRecordServerUrl.size() > 3)
            {
                delay = d->doPostPersonRecord();
            }
        }
        
        d->pauseCond.wait(&d->sync, delay * 1000);
        d->sync.unlock();
    }
}

int PostPersonRecordThreadPrivate::doManualPushRecords(const QDateTime &startDateTime, const QDateTime &endDateTime)
{
    LogD("=== MANUAL PUSH STARTED ===");
    LogD("%s %s[%d] Manual push time range: %s to %s", 
         __FILE__, __FUNCTION__, __LINE__,
         startDateTime.toString("yyyy/MM/dd hh:mm:ss").toStdString().c_str(),
         endDateTime.toString("yyyy/MM/dd hh:mm:ss").toStdString().c_str());

    if (mPostPersonRecordServerUrl.size() < 3)
    {
        LogE("%s %s[%d] No server URL configured for manual push", __FILE__, __FUNCTION__, __LINE__);
        emit q_func()->sigManualPushComplete(false, "Server URL not configured");
        return 5;
    }

    // Get total count of unpushed records in the date range
    int nTotalCount = PersonRecordToDB::GetInstance()->GetPersonRecordTotalNumByDateTime(
    startDateTime, endDateTime, false);  // ← false = ALL records
    
    LogD("%s %s[%d] Found %d unpushed records in date range", 
         __FILE__, __FUNCTION__, __LINE__, nTotalCount);

    if (nTotalCount == 0)
    {
        LogD("%s %s[%d] No unpushed records found in the selected date range", 
             __FILE__, __FUNCTION__, __LINE__);
        emit q_func()->sigManualPushComplete(true, "No unpushed records found");
        return 5;
    }

    int nProcessedCount = 0;
    int nSuccessCount = 0;
    int nFailedCount = 0;
    const int BATCH_SIZE = 5; // Process 5 records per batch

    // Process records in batches
    while (nProcessedCount < nTotalCount)
    {
        // Check if manual push was cancelled
        QMutexLocker locker(&manualPushMutex);
        if (!isManualPushInProgress)
        {
            LogD("%s %s[%d] Manual push cancelled by user", __FILE__, __FUNCTION__, __LINE__);
            emit q_func()->sigManualPushComplete(false, "Push cancelled");
            return 5;
        }
        locker.unlock();

        // Get next batch of records
        int currentPage = (nProcessedCount / BATCH_SIZE) + 1;
        QList<IdentifyFaceRecord_t> list = PersonRecordToDB::GetInstance()->GetPersonRecordDataByDateTime(
    currentPage, BATCH_SIZE, startDateTime, endDateTime, false);  // ← false = ALL records

        if (list.isEmpty())
        {
            break;
        }

        LogD("%s %s[%d] Processing batch %d: %d records", 
             __FILE__, __FUNCTION__, __LINE__, currentPage, list.size());

        // Process each record in the batch
        for (int i = 0; i < list.size(); i++)
        {
            auto &t = list[i];
            
            Json::Value json;
            std::string timestamp = getTime();
            std::string password = mPostPersonRecordServerPassword.toStdString() + timestamp;
            password = md5sum(password);
            password = md5sum(password);
            transform(password.begin(), password.end(), password.begin(), ::tolower);

            json["msg_type"] = "post_offine_record";
            json["sn"] = mSN.toStdString().c_str();
            json["tenantId"] = ReadConfig::GetInstance()->getHeartbeat_TenantId().toStdString().c_str();
            json["attendanceMode"] = ReadConfig::GetInstance()->getHeartbeat_AttendanceMode().toStdString().c_str();
            json["deviceStatus"] = ReadConfig::GetInstance()->getHeartbeat_DeviceStatus().toStdString().c_str();
            json["entry"] = "authorized";
            json["timestamp"] = timestamp.c_str();
            json["password"] = password.c_str();
            json["cmd_id"] = "";
            json["dev_name"] = "";
            json["location"] = "";
            json["message"] = "post_offine_record_manual";

            Json::Value data;
            data["card_no"] = t.face_iccardnum.toStdString().c_str();
            data["male"] = t.face_sex.toStdString().c_str();
            data["id_card_no"] = t.face_idcardnum.toStdString().c_str();
            data["person_name"] = t.face_name.toStdString().c_str();
            data["time"] = t.createtime.toString("yyyy/MM/dd hh:mm:ss").toStdString().c_str();
            data["person_uuid"] = t.face_uuid.toStdString().c_str();
            data["id"] = t.face_id.toStdString().c_str();
            data["Identifyed"] = std::to_string(t.Identifyed).c_str();
            data["rID"] = std::to_string(t.rid).c_str();
            data["group"] = t.face_gids.toStdString().c_str();
            data["person_type"] = std::to_string(t.face_persontype).c_str();
            data["face_mack"] = std::to_string(t.face_mack).c_str();
            data["temp_value"] = std::to_string(t.temp_value).c_str();

            json["data"].append(data);
            json["per_page"] = "1";
            json["count"] = "1";
            json["result"] = "1";
            json["success"] = "1";

            // Post the record
            QString ret = doPostJson(json);
            
            bool postSuccess = false;
            if (ret.size() > 0)
            {
                Json::Reader reader;
                Json::Value root;
                if (reader.parse(ret.toStdString(), root))
                {
                    std::string msg_type = root["msg_type"].asString();
                    if (msg_type == "post_offine_record_done")
                    {
                        PersonRecordToDB::GetInstance()->UpdatePersonRecordUploadFlag(t.rid, true);
                        nSuccessCount++;
                        postSuccess = true;
                        LogD("%s %s[%d] Manual push success for record %ld - %s", 
                             __FILE__, __FUNCTION__, __LINE__, t.rid, 
                             t.face_name.toStdString().c_str());
                    }
                }
            }
            
            if (!postSuccess)
            {
                nFailedCount++;
                LogE("%s %s[%d] Manual push failed for record %ld - %s", 
                     __FILE__, __FUNCTION__, __LINE__, t.rid, 
                     t.face_name.toStdString().c_str());
            }

            nProcessedCount++;
            
            // Emit progress update
            emit q_func()->sigManualPushProgress(nProcessedCount, nTotalCount, postSuccess);
            
            // Small delay between records to avoid overwhelming server
            QThread::msleep(100);
        }

        // Small delay between batches
        QThread::msleep(500);
    }

    LogD("=== MANUAL PUSH COMPLETED ===");
    LogD("%s %s[%d] Total: %d, Success: %d, Failed: %d", 
         __FILE__, __FUNCTION__, __LINE__, nTotalCount, nSuccessCount, nFailedCount);

    QString resultMsg = QString("Pushed %1/%2 records successfully").arg(nSuccessCount).arg(nTotalCount);
    emit q_func()->sigManualPushComplete(true, resultMsg);

    return 5;
}

void PostPersonRecordThread::slotManualPushRecords(const QDateTime &startDateTime, const QDateTime &endDateTime)
{
    Q_D(PostPersonRecordThread);
    
    QMutexLocker locker(&d->manualPushMutex);
    
    // Check if manual push is already in progress
    if (d->isManualPushInProgress)
    {
        LogD("%s %s[%d] Manual push already in progress, ignoring request", 
             __FILE__, __FUNCTION__, __LINE__);
        emit sigManualPushComplete(false, "Push already in progress");
        return;
    }

    // Validate date range
    if (!startDateTime.isValid() || !endDateTime.isValid())
    {
        LogE("%s %s[%d] Invalid date/time range for manual push", __FILE__, __FUNCTION__, __LINE__);
        emit sigManualPushComplete(false, "Invalid date/time range");
        return;
    }

    if (startDateTime > endDateTime)
    {
        LogE("%s %s[%d] Start date/time is after end date/time", __FILE__, __FUNCTION__, __LINE__);
        emit sigManualPushComplete(false, "Invalid date range: start is after end");
        return;
    }

    // Set manual push state
    d->isManualPushInProgress = true;
    d->manualPushStartDateTime = startDateTime;
    d->manualPushEndDateTime = endDateTime;
    
    LogD("%s %s[%d] Manual push initiated for range: %s to %s", 
         __FILE__, __FUNCTION__, __LINE__,
         startDateTime.toString("yyyy/MM/dd hh:mm:ss").toStdString().c_str(),
         endDateTime.toString("yyyy/MM/dd hh:mm:ss").toStdString().c_str());

    // Wake up the thread to process manual push
    d->pauseCond.wakeOne();
}

bool PostPersonRecordThread::pushRecordToServer(const QString &name, const QString &time, 
                                                 const QString &imgPath, int rid)
{
    // TODO: Implement your actual HTTP POST logic here
    // This is a placeholder - replace with your actual server communication
    
    // Example using QNetworkAccessManager:
    // QNetworkRequest request(QUrl("http://your-server.com/api/attendance"));
    // QJsonObject json;
    // json["name"] = name;
    // json["time"] = time;
    // json["rid"] = rid;
    // ... send request and wait for response
    
    qDebug() << "Pushing record:" << rid << name << time;
    
    // Return true if successful, false if failed
    return true; // Placeholder
}

void PostPersonRecordThread::slotAppRecordData(const IdentifyFaceRecord_t t)
{
	Q_D(PostPersonRecordThread);
	//d->sync.unlock();
	//QMutexLocker lock(&d->sync); //不用加锁
	Json::Value root;
	std::string timestamp;
	std::string password;

	timestamp = getTime();
	std::string currentTimeFormatted = getTime3(); 

	d->mPostPersonRecordServerUrl = ReadConfig::GetInstance()->getPost_PersonRecord_Address();
	d->mPostPersonRecordServerPassword = ReadConfig::GetInstance()->getPost_PersonRecord_Password();
	if (d->mPostPersonRecordServerUrl.size() < 3)
	{
		return;
	}
	QString mMustmode = ReadConfig::GetInstance()->getDoor_MustOpenMode();
	QString mOptionmode = ReadConfig::GetInstance()->getDoor_OptionalOpenMode();

	//if( (mMustmode == "1" || mOptionmode.contains("1")) )
	if (t.FaceImgPath=="/mnt/user/face_crop_image")
	{
		
		timestamp = getTime();
		password = d->mPostPersonRecordServerPassword.toStdString() + timestamp;
		password = md5sum(password);
		password = md5sum(password);
		transform(password.begin(), password.end(), password.begin(), ::tolower);
		root["msg_type"] = "post_record";
		root["sn"] = d->mSN.toStdString().c_str();
		root["timestamp"] = timestamp.c_str();
		root["time"] = currentTimeFormatted.c_str();
		root["password"] = password.c_str();
		root["type"] = std::to_string(t.passType).c_str();

		Json::Value data;


		data["person_uuid"] = t.face_uuid.toStdString().c_str();
		data["card_no"] = t.face_iccardnum.toStdString().c_str();
		data["id_card_no"] = t.face_idcardnum.toStdString().c_str();
		data["person_name"] = t.face_name.toStdString().c_str();
		data["person_type"] = std::to_string(t.face_persontype).c_str();
		data["male"] = t.face_sex.toStdString().c_str();
		data["face_mack"] = std::to_string(t.face_mack).c_str();
		data["temp_value"] = std::to_string(t.temp_value).c_str();
		data["time"] = currentTimeFormatted.c_str();


		root["data"].append(data);

		QString ret = d->doPostJson(root);

		Json::Reader reader;
		Json::Value result;
		std::string msg_type = "";
		if (ret.length() > 0)
		{
			if (reader.parse(ret.toStdString(), result))
			{
				msg_type = result["msg_type"].asString();
			}
		}
       return ;
	}

	//if( (mMustmode.contains("2") || mOptionmode.contains("2"))  && access(t.FaceImgPath.toStdString().c_str(), F_OK)) //wipeface //mMustmode.contains("2") &&
	if( access(t.FaceImgPath.toStdString().c_str(), F_OK)) //wipeface //mMustmode.contains("2") &&
	{

		
		return;

	}
	
	timestamp = getTime();
	password = d->mPostPersonRecordServerPassword.toStdString() + timestamp;
	password = md5sum(password);
	password = md5sum(password);
	transform(password.begin(), password.end(), password.begin(), ::tolower);
	root["msg_type"] = "post_record";
	root["sn"] = d->mSN.toStdString().c_str();
	root["timestamp"] = timestamp.c_str();
	root["password"] = password.c_str();
	root["time"] = currentTimeFormatted.c_str();
	root["type"] = std::to_string(t.passType).c_str();

	Json::Value data;

	QString faceImgPath ="";
	if(ReadConfig::GetInstance()->getRecords_Manager_FaceImg() == 1)
	{
		faceImgPath = t.FaceImgPath;
		
	}else if(ReadConfig::GetInstance()->getRecords_Manager_PanoramaImg() == 1)
	{
		faceImgPath = t.FaceFullImgPath;
		
	}
	if ( !access(faceImgPath.toStdString().c_str(), F_OK)) //mMustmode.contains("2") &&
	{
       std::string base64Data = fileToBase64String(faceImgPath.toStdString());
		data["crop_data"] = base64Data;
		
		if (base64Data.length() > 50) {
			
		}
	}

	data["person_uuid"] = t.face_uuid.toStdString().c_str();
	data["card_no"] = t.face_iccardnum.toStdString().c_str();
	data["id_card_no"] = t.face_idcardnum.toStdString().c_str();
	data["person_name"] = t.face_name.toStdString().c_str();
	data["person_type"] = std::to_string(t.face_persontype).c_str();
	data["male"] = t.face_sex.toStdString().c_str();
	data["face_mack"] = std::to_string(t.face_mack).c_str();
	data["temp_value"] = std::to_string(t.temp_value).c_str();
	data["time"] = currentTimeFormatted.c_str();

	root["data"].append(data);
	QString ret = d->doPostJson(root);

	Json::Reader reader;
	Json::Value result;
	std::string msg_type = "";
	if (ret.length() > 0)
	{
		if (reader.parse(ret.toStdString(), result))
		{
			msg_type = result["msg_type"].asString();
			
		}
	}
	if (msg_type == std::string("post_record_done")) {
    bool updateResult = PersonRecordToDB::GetInstance()->UpdatePersonRecordUploadFlag(t.rid, true);
    
    d->picList.append(faceImgPath);
}
}
