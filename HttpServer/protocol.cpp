#include "papi.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <openssl/md5.h>
#include <string>
#include "MessageHandler/Log.h"
#include "SharedInclude/GlobalDef.h"
#include "Config/ReadConfig.h"
#include "Application/FaceApp.h"
#include "USB/UsbObserver.h"
#include "RKCamera/Camera/cameramanager.h"
#include "RkNetWork/NetworkControlThread.h"
#include "BaiduFace/BaiduFaceManager.h"
#include "DB/RegisteredFacesDB.h"
#include "ManageEngines/PersonRecordToDB.h"
#include "PostPersonRecordThread.h"
#include "Version.h"
#include "json-cpp/json.h"
#include "Helper/myhelper.h"
#include "PCIcore/Utils_Door.h"
#include "PCIcore/RkUtils.h"
#include <sstream>
#include "protocol.h"
#include <unistd.h>
#include <dirent.h>
#include <netserver.h>
#include <dbserver.h>
#include <map>
#include <QtCore/QString>
#include <QtCore/QTime>

using namespace std;

pthread_t ntid;
PAPIConfig conf;

using std::string;

#define MERR_ASF_FACEENGINE_BASE							0x30000
#define MERR_ASF_FACEENGINE_IMAGE							(MERR_ASF_FACEENGINE_BASE + 1)
#define MERR_ASF_FACEENGINE_FACEDETECT						(MERR_ASF_FACEENGINE_BASE + 2)
#define MERR_ASF_FACEENGINE_MULTIFACE						(MERR_ASF_FACEENGINE_BASE + 3)
#define MERR_ASF_FACEENGINE_FACEEXTRACT						(MERR_ASF_FACEENGINE_BASE + 4)
#define MERR_ASF_FACEENGINE_THUMBNAIL						(MERR_ASF_FACEENGINE_BASE + 5)
#define MERR_ASF_FACEENGINE_FACEQUALITY						(MERR_ASF_FACEENGINE_BASE + 6)

//老版本兼容的内容
typedef enum
{
	CARD_FACE_EITHER = 0,
	CARD_FACE_TWO_FACTOR,
	CARD_ONLY,
	FACE_ONLY,
	TEMP_ONLY,
	TEMP_BTMASK_ONLY,
	TEMP_BT_FACE,
	TEMP_BT_MASK_FACE,
	IDCARD_FACE,
} id_method;
//老版本兼容的内容 end

struct http_parser
{
	/** PRIVATE **/
	unsigned int type :2; /* enum http_parser_type */
	unsigned int flags :7; /* F_* values from 'flags' enum; semi-public */
	unsigned int state :7; /* enum state from http_parser.c */
	unsigned int header_state :8; /* enum header_state from http_parser.c */
	unsigned int index :8; /* index into current matcher */

	uint32_t nread; /* # bytes read in various scenarios */
	uint64_t content_length; /* # bytes in body (0 if no Content-Length header) */

	/** READ-ONLY **/
	unsigned short http_major;
	unsigned short http_minor;
	unsigned int status_code :16; /* responses only */
	unsigned int method :8; /* requests only */
	unsigned int http_errno :7;

	/* 1 = Upgrade header was present and the parser has exited because of that.
	 * 0 = No upgrade header present.
	 * Should be checked when http_parser_execute() returns in addition to
	 * error checking.
	 */
	unsigned int upgrade :1;

	/** PUBLIC **/
	void *data; /* A pointer to get hook to the "connection" or "socket" object */
};

typedef struct http_parser http_parser;
typedef struct http_parser_settings http_parser_settings;

/* Callbacks should return non-zero to indicate an error. The parser will
 * then halt execution.
 *
 * The one exception is on_headers_complete. In a HTTP_RESPONSE parser
 * returning '1' from on_headers_complete will tell the parser that it
 * should not expect a body. This is used when receiving a response to a
 * HEAD request which may contain 'Content-Length' or 'Transfer-Encoding:
 * chunked' headers that indicate the presence of a body.
 *
 * http_data_cb does not return data chunks. It will be called arbitrarily
 * many times for each string. E.G. you might get 10 callbacks for "on_url"
 * each providing just a few characters more data.
 */
typedef int (*http_data_cb)(http_parser*, const char *at, size_t length);
typedef int (*http_cb)(http_parser*);
struct http_parser_settings
{
	http_cb on_message_begin;
	http_data_cb on_url;
	http_data_cb on_status;
	http_data_cb on_header_field;
	http_data_cb on_header_value;
	http_cb on_headers_complete;
	http_data_cb on_body;
	http_cb on_message_complete;
	/* When on_chunk_header is called, the current chunk length is stored
	 * in parser->content_length.
	 */
	http_cb on_chunk_header;
	http_cb on_chunk_complete;
};
class FileItem
{
public:
	FileItem();
	/**
	 * the item is file field or normal field
	 **/
	bool is_file();

	std::string *get_fieldname();
	// get upload file name
	std::string *get_filename();
	// get upload file content type
	std::string *get_content_type();
	// get file data or field value
	std::string *get_data();

	bool get_parse_state();
	void set_is_file();
	void set_name(const std::string &name);
	void set_filename(const std::string &filename);
	void append_data(const char *c, size_t len);
	void set_content_type(const char *c, int len);
	void set_parse_state(int state);
private:
	bool _is_file;
	bool _parse_state;
	std::string _content_type;
	std::string _name;
	std::string _filename;
	std::string _data;
};

class RequestParam
{
public:

	/**
	 * get param by name
	 * when not found , return empty string
	 */
	std::string get_param(std::string &name);

	/**
	 * get params by name
	 * when params in url like age=1&age=2, it will return [1, 2]
	 */
	void get_params(std::string &name, std::vector<std::string> &params);

	/**
	 * query_url : name=tom&age=3
	 */
	int parse_query_url(std::string &query_url);

	int add_param_pair(const std::string &key, const std::string &value);
private:
	std::multimap<std::string, std::string> _params;
};
class RequestBody
{
public:

	/**
	 * when Content-Type is "application/x-www-form-urlencoded"
	 * we will parse the request body , it will excepted like below
	 *
	 *     "name=tom&age=1"
	 *
	 */
	std::string get_param(std::string name);

	void get_params(std::string &name, std::vector<std::string> &params);

	/**
	 * get request body bytes
	 */
	std::string *get_raw_string();

	RequestParam *get_req_params();

	int parse_multi_params();

	std::vector<FileItem> *get_file_items();

private:
	std::string _raw_string;
	RequestParam _req_params;
	std::vector<FileItem> _file_items;
};
class RequestLine
{
public:
	/**
	 * return "GET" or "POST"
	 */
	std::string get_method();
	/**
	 * like /login
	 */
	std::string get_request_uri();
	/**
	 * like /login?name=tom&age=1
	 */
	std::string get_request_url();
	/**
	 * return "HTTP/1.0" or "HTTP/1.1"
	 */
	std::string get_http_version();

	RequestParam &get_request_param();

	std::string to_string();
	/**
	 * request_url : /sayhello?name=tom&age=3
	 */
	int parse_request_url_params();

	void set_method(std::string m);

	void set_request_url(std::string url);

	void append_request_url(std::string p_url);

	void set_http_version(const std::string &v);
private:
	RequestParam _param;
	std::string _method;       // like GET/POST
	std::string _request_url;  // like /hello?name=aaa
	std::string _http_version; // like HTTP/1.1
};
struct g_map_cmp_key
{
	bool operator()(const std::string & a, const std::string & b)
	{
		return strcasecmp(a.c_str(), b.c_str()) > 0;
	}
};
class Request
{
public:
	Request();

	~Request();

	std::string get_param(std::string name);

	/**
	 * Now, it's the same as get_param(std::string name);
	 */
	std::string get_unescape_param(std::string name);

	void get_params(std::string &name, std::vector<std::string> &params);

	std::string get_header(std::string name);

	/**
	 * return like /login
	 */
	std::string get_request_uri();

	/**
	 * return like /login?name=tom&age=1
	 */
	std::string get_request_url();

	std::string *get_client_ip();

	void set_client_ip(std::string *ip);

	void add_header(std::string &name, std::string &value);

	int parse_request(const char *read_buffer, int read_size);

	int clear();

	RequestBody *get_body();

	std::string get_method();

	bool _last_was_value;
	std::vector<std::string> _header_fields;
	std::vector<std::string> _header_values;
	int _parse_part;
	int _parse_err;
	RequestLine _line;
private:
	std::map<std::string, std::string, g_map_cmp_key> _headers;
	int _total_req_size;
	RequestBody _body;
	http_parser_settings _settings;
	http_parser _parser;
	std::string *_client_ip;
};

int get_file_md5(const std::string &file_name, std::string &md5_value)
{
	md5_value.clear();

	std::ifstream file(file_name.c_str(), std::ifstream::binary);
	if (!file)
	{
		return -1;
	}

	MD5_CTX md5Context;
	MD5_Init(&md5Context);

	char buf[1024 * 16];
	while (file.good())
	{
		file.read(buf, sizeof(buf));
		MD5_Update(&md5Context, buf, file.gcount());
	}

	unsigned char result[MD5_DIGEST_LENGTH];
	MD5_Final(result, &md5Context);

	char hex[35];
	memset(hex, 0, sizeof(hex));
	for (int i = 0; i < MD5_DIGEST_LENGTH; ++i)
	{
		sprintf(hex + i * 2, "%02x", result[i]);
	}
	hex[32] = '\0';
	md5_value = string(hex);

	return 0;
}

std::vector<std::string> split(const std::string& s, char delimiter)
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

void on_password_update(void *pReq, void *pResp)
{
	papi_request_password *prp = (papi_request_password*) pReq;
	papi_response_with_msg *resp = (papi_response_with_msg*) pResp;
	std::string new_pass(prp->new_password);
	LogV("update password.new:%s,old:%s.\n", new_pass.c_str(), prp->old_password);
	ReadConfig::GetInstance()->setSrv_Manager_Password(QString(new_pass.c_str()));

	resp->status_code = PAPI_ERR_OK;
	resp->success = PAPI_SUCCESS;
	// delay 1s and send restart signal
	resp->message[0] = '\0';

	ReadConfig::GetInstance()->setSaveConfig();
}

void on_get_versioninfo(void *pReq, void *pResp)
{
	string SN_str;
	//std::string new_pass("18682087");
	papi_request_password *prp = (papi_request_password*) pReq;
	papi_response_with_versioninfo *resp = (papi_response_with_versioninfo*) pResp;
	if (resp != ISC_NULL)
	{
		bool bAlogInit = ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->getAlgoFaceInitState();
		SN_str = myHelper::getCpuSerial().toStdString();
		//SStting::instance().setPassword(new_pass);
		LogV("info ,get SN:%s\n", SN_str.c_str());
		LogV("info ,activte:%d\n", bAlogInit);
		if (bAlogInit == true)
		{
			memcpy(resp->active_state, "actived", strlen("actived"));
		} else
		{
			memcpy(resp->active_state, "not actived", strlen("not actived"));
		}
		memcpy(resp->algo_version, ISC_ALGO_VERSION, strlen(ISC_ALGO_VERSION));
		memcpy(resp->device_version, ISC_VERSION, strlen(ISC_VERSION));
		memcpy(resp->device_sn, SN_str.c_str(), 32);
		resp->status_code = PAPI_ERR_OK;
		resp->success = PAPI_SUCCESS;
	}
}

void on_device_getPicture(void *pReq, void *pResp)
{
	LogV("%s,%d.", __FUNCTION__, __LINE__);

	papi_response_with_photodata *resp = (papi_response_with_photodata*) pResp;

	char *pRgbData = ISC_NULL;
	char *pIrData = ISC_NULL;
	int nRgbDataSize = 0;
	int nIrDataSize = 0;
	memcpy(resp->message, "get Picture fail.", strlen("get Picture fail."));
	resp->status_code = PAPI_ERR_UNKNOW;
	resp->success = PAPI_FAIL;

	qXLApp->GetCameraManager()->takePhotos(&pRgbData, &nRgbDataSize, &pIrData, &nIrDataSize);
	if (nRgbDataSize > 0 && nIrDataSize > 0)
	{
		resp->rgb_size = nRgbDataSize;
		resp->rgb_data = pRgbData;

		resp->ir_size = nIrDataSize;
		resp->ir_data = pIrData;
		memset(resp->message, 0, sizeof(resp->message));
		memcpy(resp->message, "get Picture OK.", strlen("get Picture OK."));
		resp->status_code = PAPI_ERR_OK;
		resp->success = PAPI_SUCCESS;
	}
}

void on_device_doActive(void *pReq, void *pResp)
{
	LogV("%s,%d.\n", __FUNCTION__, __LINE__);
	papi_request_filedata *prp = (papi_request_filedata *) pReq;
	papi_response *resp = (papi_response*) pResp;
	LogV("recive activefile md5:%s\n", prp->file_md5);

	if (prp != ISC_NULL && resp != ISC_NULL)
	{
		FILE *file = fopen("/mnt/user/license_upload.txt", "wb");
		if (!file)
		{
			LogV("Could not write file\n");
			resp->status_code = PAPI_FAIL;
			resp->success = PAPI_FAIL;
			return;
		}

		fwrite(prp->file_data, prp->file_size, 1, file);
		fflush(file);
		fclose(file);
		string file_name("/mnt/user/license_upload.txt");
		string file_md5;

		get_file_md5(file_name, file_md5);
		cout << "active file md5:" << file_md5 << endl;
		if (memcmp(prp->file_md5, file_md5.c_str(), 32) == 0)
		{
			resp->status_code = PAPI_SUCCESS;
			resp->success = PAPI_SUCCESS;
			system("mv /mnt/user/license_upload.txt /param/license");
			LogV("%s %s[%d] \n", __FILE__, __FUNCTION__, __LINE__);
			system("sync");
		} else
		{
			resp->status_code = PAPI_FAIL;
			resp->success = PAPI_FAIL;
		}
	}
}
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

typedef struct _device_update
{
	string md5;
	string path;
} device_update;

device_update stmdevice_update;
static void *doDeviceUpdateThread(void *arg)
{
	device_update *mdevice_update = &stmdevice_update;
	std::string path = mdevice_update->path;
	std::string strMd5 = mdevice_update->md5;
	std::string img = "";
	bool isCheckMd5OK = false;
	std::string cmd = "rm -rf /update/*; rm -rf /mnt/user/update.zip; mkdir -p /update/lost+found;sync";
	YNH_LJX::RkUtils::Utils_ExecCmd(cmd.c_str());

	if (path.find("ftp://") != std::string::npos)
	{
		unsigned char TipMsg[128] = "正在下载固件...";
#ifdef _old_version_
		GUI::getInstance()->showTipsMsg(TipMsg);
#endif
		cmd = "cd /update; wget2 -c -nH -m --timeout=3 --ftp-user=admin --ftp-password=admin  " + path + " ; sync;";
		YNH_LJX::RkUtils::Utils_ExecCmd(cmd.c_str());

		isCheckMd5OK = checkFileMd5Sum("/update/update.zip", strMd5);
	} else if (path.find("http://") != std::string::npos)
	{
		cmd = "cd /update; /usr/bin/curl --connect-timeout 3  -o /mnt/user/update.zip  " + path + " ; sync;";
		YNH_LJX::RkUtils::Utils_ExecCmd(cmd.c_str());

		if(UsbObserver::GetInstance()->doCheckUpdateImage("/mnt/user/update.zip") == true)
		{
			UsbObserver::GetInstance()->doDeviceUpdate();
		}
	}
}

void on_device_doUpdate(void *pReq, void *pResp)
{
	Request &request = *((Request *) pReq);
	Json::Value &root = *((Json::Value *) pResp);
	std::string pass = request.get_param("password");
	std::string strMd5 = request.get_param("md5sum");
	std::string path = request.get_param("path");
	cout << "get password:" << pass << endl;
	cout << "get fileMD5:" << strMd5 << endl;
	cout << "get file_info:" << path << endl;

	if (pass != ReadConfig::GetInstance()->getSrv_Manager_Password().toStdString())
	{
		root["result"] = PAPI_ERR_INVALID_PASSWORD;
		root["success"] = PAPI_FAIL;
		root["message"] = "Invalid password";
		return;
	}

	if (strMd5.length() != 32)
	{
		root["result"] = PAPI_FAIL;
		root["success"] = PAPI_FAIL;
		root["message"] = "invalid md5";
		return;
	}

	memset(&stmdevice_update, 0, sizeof(stmdevice_update));
	stmdevice_update.md5 = strMd5;
	stmdevice_update.path = path;

	static pthread_t mstUpdateThread = 0;
	pthread_create(&mstUpdateThread, ISC_NULL, doDeviceUpdateThread, ISC_NULL);
	root["result"] = "1";
	root["success"] = "1";
	root["message"] = "receive cmd";
}

void on_device_restart(void *pReq, void *pResp)
{
	papi_response_with_msg *resp = (papi_response_with_msg*) pResp;
	resp->status_code = PAPI_ERR_OK;
	resp->success = PAPI_SUCCESS;
	LogV("%s %s[%d] \n", __FILE__, __FUNCTION__, __LINE__);
	system("sync");
	myHelper::Utils_Reboot();
	memset(resp->message, 0, sizeof(char) * sizeof(resp->message));
	memcpy(resp->message, "Restarting...", sizeof(char) * 20);

}

void on_device_heartbeat(void *pReq, void *pResp)
{
	papi_request_heartbeat_url *prp = (papi_request_heartbeat_url*) pReq;
	papi_response_with_msg *resp = (papi_response_with_msg*) pResp;
	resp->status_code = PAPI_ERR_OK;
	resp->success = PAPI_SUCCESS;

	// save heartbeat url
	LogV("heartbeat url %s\n", prp->callback_url);

	resp->message[0] = '\0';
}

void on_people_add(void *pReq, void *pResp)
{
	int person_id = 0;
	papi_request_people_add *prp = (papi_request_people_add*) pReq;
	Json::Value &root = *((Json::Value *) pResp);

	char person_imgName[128] = { 0 };
	if (prp->image_data_size > 0)
	{
		person_id = 186;
		// save person image data
		LogV("image file size: %d\n", prp->image_data_size);
		//char person_imgName[128];
		memset(person_imgName, 0, sizeof(person_imgName));
		sprintf(person_imgName, "/mnt/user/facedb/img/%s_pid%d.jpg", "face_img", person_id);
		LogV("save person name:%s,group:%s \n", prp->name, prp->group);
		FILE *fp = fopen(person_imgName, "wb");
		fwrite(prp->image_data, 1, prp->image_data_size, fp);
		fflush(fp);
		fclose(fp);
		LogV("save person jpg:%s ok \n", person_imgName);
	}
	string auth_type = string(prp->auth_type);
	string time_data = string(prp->time_data);
	string time_range = string(prp->time_range);

	bool isOk = false;
	std::string start_time;
	std::string end_time;
	std::vector<std::string> vect;
	char split_c = ';';
	vect = split(time_range, split_c);
	if (vect.size() == 2)
	{
		isOk = true;
	}
	if (isOk == true)
	{
		start_time = vect[0];
		end_time = vect[1];
	}
	int result = -1;
	int faceNum = 0;
	double threshold = 0;
	QByteArray faceFeature;
	result = ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->RegistPerson(person_imgName, faceNum, threshold, faceFeature);
	LogD("%s %s[%d] RegistPerson result %d \n", __FILE__, __FUNCTION__, __LINE__, result);
	LogD("%s %s[%d] auth_type %s start_time %s end_time %s\n", __FILE__, __FUNCTION__, __LINE__, auth_type.c_str(), start_time.c_str(),
			end_time.c_str());
	LogD("%s %s[%d] auth_type %s time_data %s time_range %s\n", __FILE__, __FUNCTION__, __LINE__, auth_type.c_str(), time_data.c_str(),
			time_range.c_str());

	if (auth_type == std::string("time_range"))
	{
	} else if (auth_type == std::string("week_cycle"))
	{
	}

	if (result == 0)
	{
		bool isSaveDBOk = RegisteredFacesDB::GetInstance()->RegPersonToDBAndRAM(prp->person_uuid, prp->name, prp->id_card_no,
				prp->card_no, prp->male, prp->group, "", faceFeature);
		LogD("%s %s[%d] RegPersonToDBAndRAM isSaveDBOk %d \n", __FILE__, __FUNCTION__, __LINE__, isSaveDBOk);

		root["result"] = "1";
		root["success"] = "1";
		root["message"] = "person register success!";
	} else if (result == MERR_ASF_FACEENGINE_FACEDETECT)
	{
		root["result"] = "-1";
		root["success"] = "0";
		root["message"] = "picture do not have face";
	} else if (result == MERR_ASF_FACEENGINE_MULTIFACE)
	{
		root["result"] = "-2";
		root["success"] = "0";
		root["message"] = "picture have many face";
	} else if (result == MERR_ASF_FACEENGINE_FACEEXTRACT)
	{
		root["result"] = "-3";
		root["success"] = "0";
		root["message"] = "extract the feature failed!";
	} else if (result == ISC_ERROR_EXIST)
	{
		root["result"] = "-4";
		root["success"] = "0";
		root["message"] = "person already register";
	} else if (result == MERR_ASF_FACEENGINE_FACEQUALITY)
	{
		root["result"] = "-5";
		root["success"] = "0";
		root["message"] = "the picture is too low or the image is too bright or blurry";
	} else
	{
		root["result"] = "-6";
		root["success"] = "0";
		root["message"] = "register failed, unknown error, please try again";
	}

	if (strlen(person_imgName) > 10 && !access(person_imgName, F_OK))
	{
		unlink(person_imgName);
	}

	free(prp->image_data);
	prp->image_data = ISC_NULL;
	prp->image_data_size = 0;
	prp->image_data_size = 0;
}

void on_people_update(void *pReq, void *pResp)
{
	int person_id = 0;
	int ret = -1;
	papi_request_people_add *prp = (papi_request_people_add*) pReq;
	Json::Value &root = *((Json::Value *) pResp);

	person_id = 186;

	// save person image data
	LogV("on_people_update \n");
	LogV("image file size: %d\n", prp->image_data_size);
	char person_imgName[128];
	memset(person_imgName, 0, sizeof(person_imgName));
	if (prp->image_data_size > 0)
	{
		sprintf(person_imgName, "/mnt/user/facedb/img/%s_pid%d.jpg", prp->person_uuid, person_id);
		LogV("save person name:%s,group:%s \n", prp->person_uuid, prp->group);
		FILE *fp = fopen(person_imgName, "wb");
		fwrite(prp->image_data, 1, prp->image_data_size, fp);
		fflush(fp);
		fclose(fp);
		LogV("save person jpg:%s ok \n", person_imgName);
	}

	string auth_type = string(prp->auth_type);
	string time_data = string(prp->time_data);
	string time_range = string(prp->time_range);

	RegisteredFacesDB::GetInstance()->DelPersonByPersionUUIDFromDBAndRAM(prp->person_uuid);

	int faceNum = 0;
	double threshold = 0;
	QByteArray faceFeature;
	QString timeOfAccess = "";
	ret = ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->RegistPerson(person_imgName, faceNum, threshold, faceFeature);
	LogD("%s %s[%d] RegistPerson result %d \n", __FILE__, __FUNCTION__, __LINE__, ret);
	LogD("%s %s[%d] auth_type %s time_data %s time_range %s\n", __FILE__, __FUNCTION__, __LINE__, auth_type.c_str(), time_data.c_str(),
			time_range.c_str());

	if (auth_type == std::string("time_range"))
	{
	} else if (auth_type == std::string("week_cycle"))
	{
	}

	if (ret == 0)
	{
		bool isSaveDBOk = RegisteredFacesDB::GetInstance()->RegPersonToDBAndRAM(prp->person_uuid, prp->name, prp->id_card_no,
				prp->card_no, prp->male, prp->group, timeOfAccess, faceFeature);
		LogD("%s %s[%d] RegPersonToDBAndRAM result %d \n", __FILE__, __FUNCTION__, __LINE__, isSaveDBOk);
		if(isSaveDBOk != true)
		{
			ret = -1;
		}
	}

	if (ret == 0)
	{
		root["result"] = "1";
		root["success"] = "1";
		root["message"] = "person register success!";
	} else if (ret == MERR_ASF_FACEENGINE_FACEDETECT)
	{
		root["result"] = "-1";
		root["success"] = "0";
		root["message"] = "picture do not have face";
	} else if (ret == MERR_ASF_FACEENGINE_MULTIFACE)
	{
		root["result"] = "-2";
		root["success"] = "0";
		root["message"] = "picture have many face";
	} else if (ret == MERR_ASF_FACEENGINE_FACEEXTRACT)
	{
		root["result"] = "-3";
		root["success"] = "0";
		root["message"] = "extract the feature failed!";
	} else if (ret == ISC_ERROR_EXIST)
	{
		root["result"] = "-4";
		root["success"] = "0";
		root["message"] = "person already register";
	} else if (ret == MERR_ASF_FACEENGINE_FACEQUALITY)
	{
		root["result"] = "-5";
		root["success"] = "0";
		root["message"] = "the picture is too low or the image is too bright or blurry";
	} else
	{
		root["result"] = "-6";
		root["success"] = "0";
		root["message"] = "register failed, unknown error, please try again";
	}

	if (strlen(person_imgName) > 10 && !access(person_imgName, F_OK))
	{
		unlink(person_imgName);
	}

	free(prp->image_data);
	prp->image_data = ISC_NULL;
	prp->image_data_size = 0;
}

void on_people_get_all_group(void *pReq, void *pResp)
{
	LogD("%s %s %d \n", __FILE__, __FUNCTION__, __LINE__);
	papi_response_get_group *resp = (papi_response_get_group*) pResp;
#ifdef _old_version_
	if (MAX_GROUP_NUM >= PAPI_MAX_GROUP_NUM)
	resp->group_size = PAPI_MAX_GROUP_NUM;
	else
	resp->group_size = MAX_GROUP_NUM;
	Group_S *pAllGroup = (Group_S*) malloc(sizeof(Group_S) * resp->group_size);
	if (pAllGroup != ISC_NULL)
	{
		Ai_GetAllGroup(pAllGroup, resp->group_size);
		for (int i = 0; i < resp->group_size; i++)
		{
			memcpy(resp->grop_data[i], pAllGroup[i].szName, strlen(pAllGroup[i].szName));
		}
		resp->status_code = PAPI_ERR_OK;
		resp->success = PAPI_SUCCESS;
	} else
	{
		resp->group_size = 0;
		resp->status_code = PAPI_FAIL;
		resp->success = PAPI_FAIL;
	}
	free(pAllGroup);
#endif

}

void on_people_get_all(void *pReq, void *pResp)
{
	int person_count = 0;

	papi_request_people_get_all *prp = (papi_request_people_get_all*) pReq;
	papi_response_people_get_all *resp = (papi_response_people_get_all*) pResp;

	QList<PERSONS_t> list = RegisteredFacesDB::GetInstance()->GetAllPersonFromRAM();

	person_count = list.size();
	LogV("%s %s[%d] get all person :%d.",__FILE__,__FUNCTION__,__LINE__, person_count);
	LogV("page: %d, per_page: %d\n", prp->page, prp->per_page);
	if (person_count > 0)
	{
		if (person_count < prp->per_page)
		{
			resp->total_page = 1;
			resp->current_page = 1;
			resp->count = person_count;
			resp->per_page = person_count;
			for (int i = 0; i < person_count; i++)
			{
				auto &t = list.at(i);
				memcpy(resp->people_records[i].person_uuid, t.uuid.toStdString().c_str(), t.uuid.size());
				resp->people_records[i].person_type = t.persontype;
				memcpy(resp->people_records[i].name, t.name.toStdString().c_str(), t.name.size());
				memcpy(resp->people_records[i].group, t.gids.toStdString().c_str(), t.gids.size());
				memcpy(resp->people_records[i].id_card_no, t.idcard.toStdString().c_str(), t.idcard.size());
				memcpy(resp->people_records[i].card_no, t.iccard.toStdString().c_str(), t.iccard.size());
				memcpy(resp->people_records[i].male, t.sex.toStdString().c_str(), t.sex.size());
				memcpy(resp->people_records[i].add_time, t.createtime.toStdString().c_str(), t.createtime.size());
			}
		} else
		{
			if (person_count % prp->per_page == 0)
			{
				resp->total_page = person_count / prp->per_page;
			} else
			{
				resp->total_page = person_count / prp->per_page + 1;
				resp->current_page = prp->page;
				if (prp->page == resp->total_page)
				{
					int offset = (prp->page - 1) * prp->per_page;
					resp->per_page = person_count % prp->per_page;
					resp->count = person_count;
					for (int i = 0; i < resp->per_page; i++)
					{
						auto &t = list.at(i + offset);
						memcpy(resp->people_records[i].person_uuid, t.uuid.toStdString().c_str(), t.uuid.size());
						resp->people_records[i].person_type = t.persontype;
						memcpy(resp->people_records[i].name, t.name.toStdString().c_str(), t.name.size());
						memcpy(resp->people_records[i].group, t.gids.toStdString().c_str(), t.gids.size());
						memcpy(resp->people_records[i].id_card_no, t.idcard.toStdString().c_str(), t.idcard.size());
						memcpy(resp->people_records[i].card_no, t.iccard.toStdString().c_str(), t.iccard.size());
						memcpy(resp->people_records[i].male, t.sex.toStdString().c_str(), t.sex.size());
						memcpy(resp->people_records[i].add_time, t.createtime.toStdString().c_str(), t.createtime.size());
					}
				}
			}
			if (prp->page <= (person_count / prp->per_page))
			{
				int offset = (prp->page - 1) * prp->per_page;
				resp->current_page = prp->page;
				resp->count = person_count;
				resp->per_page = prp->per_page;
				for (int i = 0; i < prp->per_page; i++)
				{
					auto &t = list.at(i + offset);
					memcpy(resp->people_records[i].person_uuid, t.uuid.toStdString().c_str(), t.uuid.size());
					resp->people_records[i].person_type = t.persontype;
					memcpy(resp->people_records[i].name, t.name.toStdString().c_str(), t.name.size());
					memcpy(resp->people_records[i].group, t.gids.toStdString().c_str(), t.gids.size());
					memcpy(resp->people_records[i].id_card_no, t.idcard.toStdString().c_str(), t.idcard.size());
					memcpy(resp->people_records[i].card_no, t.iccard.toStdString().c_str(), t.iccard.size());
					memcpy(resp->people_records[i].male, t.sex.toStdString().c_str(), t.sex.size());
					memcpy(resp->people_records[i].add_time, t.createtime.toStdString().c_str(), t.createtime.size());
				}
			}
		}
		resp->status_code = PAPI_ERR_OK;
		resp->success = PAPI_SUCCESS;
	} else
	{
		resp->count = 0;
		resp->current_page = 0;
		resp->total_page = 0;
		resp->per_page = 0;
		resp->status_code = PAPI_ERR_OK;
		resp->success = PAPI_SUCCESS;
	}
}

void on_people_find(void *pReq, void *pResp)
{
	papi_request_people_find *prp = (papi_request_people_find*) pReq;
	papi_response_people_get_all *resp = (papi_response_people_get_all*) pResp;
	int find_type = prp->type;
	int current_page = prp->page;
	int per_page = prp->per_page;

	int data_count = 0;
	if (per_page <= 0 || find_type <= 0)
	{
		resp->count = 0;
		resp->status_code = PAPI_FAIL;
		resp->success = PAPI_FAIL;
		return;
	}

	LogD("%s %s[%d] find_type %d current_page %d per_page %d \n", __FILE__, __FUNCTION__, __LINE__, find_type, current_page - 1,
			per_page);
	QList<PERSONS_t> persons;
	switch (find_type)
	{
	case 1:
	{   //find by name
		resp->count = RegisteredFacesDB::GetInstance()->GetPersonTotalNumByNameFromRAM(QString(prp->name));
		LogD("%s %s[%d] name %s count %lld \n", __FILE__, __FUNCTION__, __LINE__, prp->name, resp->count);

		persons = RegisteredFacesDB::GetInstance()->GetPersonDataByNameFromRAM(current_page - 1, per_page, QString(prp->name));
		data_count = persons.size();
		break;
	}
	case 2:
	{   //find by group
		break;
	}
	case 3:
	{   //find by personid
		resp->count = RegisteredFacesDB::GetInstance()->GetPersonTotalNumByPersonUUIDFromRAM(QString(prp->uid));
		LogD("%s %s[%d] uid %s count %lld \n", __FILE__, __FUNCTION__, __LINE__, prp->uid, resp->count);

		persons = RegisteredFacesDB::GetInstance()->GetPersonDataByPersonUUIDFromRAM(QString(prp->uid));
		data_count = persons.size();
		break;
	}
	case 4:
	{   //find by time
		QTime startTime = QTime::fromString(prp->start_time, "yyyy/MM/dd hh:mm:ss");
		QTime endTime = QTime::fromString(prp->end_time, "yyyy/MM/dd hh:mm:ss");
		resp->count = RegisteredFacesDB::GetInstance()->GetPersonTotalNumByTimeFromRAM(startTime, endTime);
		LogD("%s %s[%d] start_time %s end_time %s count %lld \n", __FILE__, __FUNCTION__, __LINE__, prp->start_time, prp->end_time,
				resp->count);

		persons = RegisteredFacesDB::GetInstance()->GetPersonDataByTimeFromRAM(current_page - 1, per_page, startTime, endTime);
		data_count = persons.size();
		break;
	}
	default:
		break;
	}
	resp->current_page = current_page;
	if (resp->count == 0)
	{
		memcpy(resp->message, "find none people.", strlen("find none people."));
		resp->status_code = PAPI_ERR_OK;
		resp->success = PAPI_ERR_OK;
		return;
	}
	if (data_count > 0 && data_count <= per_page)
		resp->per_page = data_count;
	else
	{
		memcpy(resp->message, "get people err.", strlen("get people err."));
		resp->status_code = PAPI_FAIL;
		resp->success = PAPI_FAIL;
		return;
	}
	resp->total_page = 0;
	if (resp->count > 0)
	{
		if (resp->count < prp->per_page)
		{
			resp->total_page = 1;
		} else
		{
			if (resp->count % prp->per_page == 0)
			{
				resp->total_page = resp->count / prp->per_page;
			} else
			{
				resp->total_page = resp->count / prp->per_page + 1;
			}
		}
	}

	for (int i = 0; i < resp->per_page; i++)
	{
		auto &t = persons.at(i);
		memcpy(resp->people_records[i].person_uuid, t.uuid.toStdString().c_str(), t.uuid.size());
		resp->people_records[i].person_type = t.persontype;
		memcpy(resp->people_records[i].name, t.name.toStdString().c_str(), t.name.size());
		memcpy(resp->people_records[i].group, t.gids.toStdString().c_str(), t.gids.size());
		memcpy(resp->people_records[i].id_card_no, t.idcard.toStdString().c_str(), t.idcard.size());
		memcpy(resp->people_records[i].card_no, t.iccard.toStdString().c_str(), t.iccard.size());
		memcpy(resp->people_records[i].male, t.sex.toStdString().c_str(), t.sex.size());
		memcpy(resp->people_records[i].add_time, t.createtime.toStdString().c_str(), t.createtime.size());
	}

	memcpy(resp->message, "people data", strlen("people data"));
	resp->status_code = PAPI_ERR_OK;
	resp->success = PAPI_SUCCESS;
	return;
}

void on_people_delete(void *pReq, void *pResp)
{
	papi_request_people_delete *prp = (papi_request_people_delete*) pReq;
	papi_response_people_delete *resp = (papi_response_people_delete*) pResp;

	LogD("%s %s[%d] person_uuids: %s person_type %d \n", __FILE__, __FUNCTION__, __LINE__, prp->person_uuids, prp->person_type);

	stringstream ss_effective;
	stringstream ss_invalid;

	std::string ids2delete = std::string(prp->person_uuids);
	int nPersonType = prp->person_type;
	std::string value;
	std::replace(ids2delete.begin(), ids2delete.end(), ',', ' ');
	std::stringstream ss(ids2delete);
	while (ss >> value)
	{
		if (value.size() <= 0)
		{
			continue;
		}
		bool isDelOk = RegisteredFacesDB::GetInstance()->DelPersonByPersionUUIDFromDBAndRAM(QString(value.c_str()));
		LogD("%s %s[%d] uuid: %s isDelOk %d \n", __FILE__, __FUNCTION__, __LINE__, value.c_str(), isDelOk);
		if (isDelOk == true)
		{
			ss_effective << value << ",";
			resp->effective_count++;
		} else
		{
			ss_invalid << value << ",";
			resp->invalid_count++;
		}
	}
	resp->message[0] = '\0';
	memcpy(resp->effective_uuids, ss_effective.str().c_str(), sizeof(char) * (ss_effective.str().length() - 1));
	memcpy(resp->invalid_uuids, ss_invalid.str().c_str(), sizeof(char) * (ss_invalid.str().length() - 1));
	resp->status_code = PAPI_ERR_OK;
	resp->success = PAPI_SUCCESS;
}

void on_people_delete_all(void *pReq, void *pResp)
{
	papi_request_people_delete *prp = (papi_request_people_delete*) pReq;
	papi_response_people_delete *resp = (papi_response_people_delete*) pResp;

	LogV("delete all person.\n");

	LogV("%s %s[%d] \n", __FILE__, __FUNCTION__, __LINE__);
	resp->status_code = PAPI_ERR_OK;
	resp->success = PAPI_SUCCESS;
	system("rm -rf /mnt/user/facedb/isc.db");
	system("rm -rf /mnt/user/facedb/isc_baidu.db");
	system("rm -rf /mnt/user/facedb/isc_arcsoft_face.db");
	system("rm -rf /mnt/user/facedb/szht.db");
	system("rm /mnt/user/facedb/img -Rf");
	system("sync");
	myHelper::Utils_Reboot();
}

void on_device_delete_all_records(void *pReq, void *pResp)
{
	// NOTE: pReq is ISC_NULL
	papi_response_with_msg *resp = (papi_response_with_msg*) pResp;

	// delete all records
	system("rm -rf /mnt/user/facedb/isc_ir_arcsoft_face.db");
	system("sync");
	myHelper::Utils_Reboot();
}

std::map<int, std::string> g_HttpCallbackUrlMap;
void on_device_set_callback(void *pReq, void *pResp)
{
	papi_request_set_callback *prp = (papi_request_set_callback*) pReq;
	papi_response_with_msg *resp = (papi_response_with_msg*) pResp;

	// callbak url 为空则清掉保存的URL
	map<int, std::string>::iterator iter;
	iter = g_HttpCallbackUrlMap.find(prp->type);
	if (iter != g_HttpCallbackUrlMap.end())
	{
		g_HttpCallbackUrlMap.erase(iter);
	}
	g_HttpCallbackUrlMap.insert(pair<int, std::string>(prp->type, prp->callback_url));

	iter = g_HttpCallbackUrlMap.find(prp->type);
//	LogV("%s %s[%d] type %d callback_url %s\n", __FILE__, __FUNCTION__, __LINE__, iter->first, iter->second.c_str());

	switch (prp->type)
	{
	case DEVICE_CALLBACK_TYPE_STRANGER: // 陌生人回调
	{
		break;
	}
	case DEVICE_CALLBACK_TYPE_CARD_NUM: // 刷卡回调
	{
		break;
	}
	case DEVICE_CALLBACK_TYPE_FACE_MATCH: // 识别人脸回调
	{
		break;
	}
	case DEVICE_CALLBACK_TYPE_FACE_NOT_PERMISSION:
	{
		break;
	}
	case DEVICE_CALLBACK_TYPE_FACETOOLS_CALLBACK:
	{
		map<int, std::string>::iterator iter = g_HttpCallbackUrlMap.find(DEVICE_CALLBACK_TYPE_FACETOOLS_CALLBACK);
		if (iter->second.length() > 0)
		{
			std::string url = iter->second;
			QString httpServerUrl = ReadConfig::GetInstance()->getSrv_Manager_Address();
			if (httpServerUrl != url.c_str())
			{
				LogV("%s %s[%d] type %d callback_url %s set to config \n", __FILE__, __FUNCTION__, __LINE__, iter->first,
						iter->second.c_str());
				ReadConfig::GetInstance()->setSrv_Manager_Address(url.c_str());
				ReadConfig::GetInstance()->setSaveConfig();
			}
		}
		break;
	}
	}
	memcpy(resp->message, "set callback OK.", strlen("set callback OK."));
	resp->status_code = PAPI_ERR_OK;
	resp->success = PAPI_SUCCESS;
}

void on_device_open_door(void *pReq, void *pResp)
{
	// NOTE: pReq is ISC_NULL
	papi_response_with_msg *resp = (papi_response_with_msg*) pResp;
	papi_request_opendoord *prp = (papi_request_opendoord*) pReq;
	LogD("%s %s[%d] delay time=%d \n", __FILE__, __FUNCTION__, __LINE__, prp->delay_relay_time);

	if (prp->delay_relay_time > 0)
	{
		ReadConfig::GetInstance()->setDoor_Timer(prp->delay_relay_time);
	}

	// open door
	YNH_LJX::Utils_Door::GetInstance()->OpenDoor("");

	resp->status_code = PAPI_ERR_OK;
	resp->success = PAPI_SUCCESS;
}

static unsigned char szIpAddress[18] = { 0 };
static unsigned char szNetMask[18] = { 0 };
static unsigned char szGateWay[18] = { 0 };
static unsigned char szDns[6][18] = { 0 };

static void* netThread(void *arg)
{
	sleep(1);
	int *netmode = (int *) arg;

	printf("netmode =%d \n", *netmode);
	LogV("%s %s[%d] szIpAddress %s \n", __FILE__, __FUNCTION__, __LINE__, szIpAddress);
	LogV("%s %s[%d] szNetMask %s \n", __FILE__, __FUNCTION__, __LINE__, szNetMask);
	LogV("%s %s[%d] szGateWay %s \n", __FILE__, __FUNCTION__, __LINE__, szGateWay);

	if (*netmode == 0)
	{
		NetworkControlThread::GetInstance()->setLinkLan(1, QString((char*) szIpAddress), QString((char*) szNetMask),
				QString((char*) szGateWay), QString((char*) szDns[0]));
	} else
	{
		NetworkControlThread::GetInstance()->setLinkLan(0, "", "", "", "");
	}
	myHelper::Utils_Reboot();

}

void on_device_net_config(void *pReq, void *pResp)
{
	papi_request_net_config *prp = (papi_request_net_config*) pReq;
	papi_response_net_config *resp = (papi_response_net_config*) pResp;

	// config network 

	resp->status_code = PAPI_ERR_OK;
	resp->success = PAPI_FAIL;
	printf("%s %s[%d] prp->net_mode %d \n", __FILE__, __FUNCTION__, __LINE__, prp->net_mode);
	printf("%s %s[%d] prp->dhcp %d \n", __FILE__, __FUNCTION__, __LINE__, prp->dhcp);
	printf("%s %s[%d] prp->ip_addr %s \n", __FILE__, __FUNCTION__, __LINE__, prp->ip_addr);
	printf("%s %s[%d] prp->gateway %s \n", __FILE__, __FUNCTION__, __LINE__, prp->gateway);
	printf("%s %s[%d] prp->subnet_mask %s \n", __FILE__, __FUNCTION__, __LINE__, prp->subnet_mask);
	printf("%s %s[%d] prp->dns1 %s \n", __FILE__, __FUNCTION__, __LINE__, prp->dns1);
	printf("%s %s[%d] prp->dns2 %s \n", __FILE__, __FUNCTION__, __LINE__, prp->dns2);

	memset(szIpAddress, 0, sizeof(szIpAddress));
	memset(szNetMask, 0, sizeof(szNetMask));
	memset(szGateWay, 0, sizeof(szGateWay));
	memset(szDns[0], 0, sizeof(szDns[0]));

	bool isOk = false;
	if (strlen(prp->ip_addr) >= 7 && strlen(prp->subnet_mask) >= 7 && strlen(prp->gateway) >= 7)
	{
		strncpy((char*) szIpAddress, prp->ip_addr, strlen(prp->ip_addr));
		strncpy((char*) szNetMask, prp->subnet_mask, strlen(prp->subnet_mask));
		strncpy((char*) szGateWay, prp->gateway, strlen(prp->gateway));
		if (strlen(prp->dns1) >= 7)
		{
			strncpy((char*) szDns[0], prp->dns1, 18);
		}
		isOk = true;
	}

	if (isOk == true)
	{
		if (prp->dhcp == 0)
		{
			resp->status_code = PAPI_ERR_OK;
			resp->success = PAPI_SUCCESS;
			resp->dhcp = 0;
		} else
		{

			resp->status_code = PAPI_ERR_OK;
			resp->success = PAPI_SUCCESS;
			resp->dhcp = 1;
		}
		static pthread_t mstNetThread = 0;
		pthread_create(&mstNetThread, ISC_NULL, netThread, &prp->dhcp);
	}
}

void on_people_get_all_by_time(void *pReq, void *pResp)
{
	papi_request_people_find *prp = (papi_request_people_find*) pReq;
	papi_response_people_get_by_condition *resp = (papi_response_people_get_by_condition*) pResp;
	LogV("fun:test_on_people_get_all_by_time \n ");
	LogV("page: %d, per_page: %d\n", prp->page, prp->per_page);
	resp->status_code = PAPI_ERR_OK;
	resp->success = PAPI_SUCCESS;
	resp->count = 3;
}

void on_people_get_all_by_uncertain_value(void *pReq, void *pResp)
{
	papi_request_people_get_by_uncertain_value *prp = (papi_request_people_get_by_uncertain_value*) pReq;
	papi_response_people_get_by_condition *resp = (papi_response_people_get_by_condition*) pResp;
	LogV("fun:papi_request_people_get_by_uncertain_value \n ");
	LogV("page: %d, per_page: %d\n", prp->page, prp->per_page);
	resp->status_code = PAPI_ERR_OK;
	resp->success = PAPI_SUCCESS;
	resp->count = 3;
}

void on_people_get_pass_permision(void *pReq, void *pResp)
{
	papi_request_people_get_pass_permission *prp = (papi_request_people_get_pass_permission*) pReq;
	papi_response_people_get_pass_permission *resp = (papi_response_people_get_pass_permission*) pResp;
	int uid = atoi(prp->person_uuid);
	permission_Info info;
	memset(&info, 0, sizeof(permission_Info));

	QList<PERSONS_t> persons = RegisteredFacesDB::GetInstance()->GetPersonDataByPersonUUIDFromRAM(prp->person_uuid);
	if (persons.size() > 0)
	{
		PERSONS_t stPerson = persons[0];
		if (stPerson.timeOfAccess.contains(";"))
		{
			strncpy(info.auty_type, "time_range", sizeof(info.auty_type));
			QStringList sections = stPerson.timeOfAccess.split(";");
			strncpy(info.startTime, sections[0].toStdString().c_str(), sizeof(info.startTime));
			strncpy(info.endTime, sections[1].toStdString().c_str(), sizeof(info.endTime));
		} else
		{
			QStringList sections = stPerson.timeOfAccess.split(",");
			QString weekCycle = "";
			for (int i = 0; i < 7; i++)
			{
				if (sections[2 + i] == "1")
				{
					weekCycle += QString::number(i + 1) + ",";
				}
			}
			strncpy(info.startTime, sections[0].toStdString().c_str(), sizeof(info.startTime));
			strncpy(info.endTime, sections[1].toStdString().c_str(), sizeof(info.endTime));
			strncpy(info.auth_info, weekCycle.toStdString().c_str(), sizeof(info.endTime));
			strncpy(info.auty_type, "week_cycle", sizeof(info.auty_type));
		}
	}
	memcpy(resp->auth_type, info.auty_type, strlen(info.auty_type));
	memcpy(resp->data, info.auth_info, strlen(info.auth_info));
	std::string s_time = info.startTime;
	std::string e_time = info.endTime;
	std::string time_range = s_time + ";" + e_time;
	memcpy(resp->time_range, time_range.c_str(), time_range.length());
	resp->status_code = PAPI_ERR_OK;
	resp->success = PAPI_SUCCESS;
	memcpy(resp->message, "get people permision ok.", strlen("get people permision ok."));

}

void on_people_set_pass_permision(void *pReq, void *pResp)
{
	LogV("fun:on_people_set_pass_permision \n ");
	papi_request_people_set_pass_permission *prp = (papi_request_people_set_pass_permission*) pReq;
	papi_response_with_msg_t *resp = (papi_response_with_msg_t*) pResp;
	if (prp == ISC_NULL || resp == ISC_NULL)
	{
		resp->status_code = PAPI_FAIL;
		resp->success = PAPI_FAIL;
		return;
	}
	int ret = 0;
	string auth_type = prp->auth_type;
	string auth_info = prp->data;     //月循环，周循环时填写
	string time_range = prp->time_range;  //时间范围

	LogV("person_uuid = %s \n", prp->person_uuid);
	LogV("person_type = %d \n", prp->person_type);
	LogV("auth_type = %s \n", prp->auth_type);
	LogV("data = %s \n", prp->data);
	LogV("time_range = %s \n", prp->time_range);

	QString authType = prp->auth_type;
	QString timeRange = prp->time_range;
	QString data = prp->data;
	QString timeOfAccess;

	if (authType == "time_range")
	{
		timeOfAccess = timeRange;  //time_range = 2000/01/01 00:00;2000/01/01 00:00
	} else if (authType == "week_cycle")
	{
		//data = 1,2,3,4,5,6,7
		//time_range = 00:00;21:00
		timeRange.replace(QString(";"), QString(","));
		timeOfAccess = timeRange;

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
	ret = RegisteredFacesDB::GetInstance()->UpdatePersonToDBAndRAM(prp->person_uuid, "", "", "", "", "", timeOfAccess, "");
	LogV("%s %s[%d] UpdatePersonToDBAndRAM ret %d\n", __FILE__, __FUNCTION__, __LINE__, ret);
	if (ret == 0)
	{
		memcpy(resp->message, "set people permision ok.", strlen("set people permision ok."));
		resp->status_code = PAPI_ERR_OK;
		resp->success = PAPI_SUCCESS;
	} else
	{
		memcpy(resp->message, "set people permision fail.", strlen("set people permision fail."));
		resp->status_code = PAPI_ERR_OK;
		resp->success = PAPI_FAIL;
	}
	LogV("%s %s[%d] %s\n", __FILE__, __FUNCTION__, __LINE__, resp->message);
}

void on_device_set_pass_permision(void *pReq, void *pResp)
{
	papi_request_device_set_pass_permission *prp = (papi_request_device_set_pass_permission*) pReq;
	papi_response_with_msg_t *resp = (papi_response_with_msg_t*) pResp;
	LogV("fun:test_on_device_set_pass_permision \n ");
	LogV("auth_type = %s \n", prp->auth_type);
	LogV("data = %s \n", prp->data);
	LogV("time_range = %s \n", prp->time_range);
	resp->status_code = PAPI_ERR_OK;
	resp->success = PAPI_SUCCESS;
}

void on_people_delete_pass_permision(void *pReq, void *pResp)
{
	papi_request_people_delete_pass_permission *prp = (papi_request_people_delete_pass_permission*) pReq;
	papi_response_with_msg_t *resp = (papi_response_with_msg_t*) pResp;
	LogV("people uuid = %s \n", prp->person_uuid);
	LogV("fun:test_on_people_delete_pass_permision \n ");
	int result = -1;

	result = RegisteredFacesDB::GetInstance()->UpdatePersonToDBAndRAM(prp->person_uuid, "", "", "", "", "", "", "");
	if (result == 0)
	{
		memcpy(resp->message, "delete people permision ok.", strlen("delete people permision ok."));
		resp->status_code = PAPI_ERR_OK;
		resp->success = PAPI_SUCCESS;
	} else
	{
		resp->status_code = PAPI_ERR_OK;
		resp->success = PAPI_FAIL;
	}
}

void on_device_delete_pass_permision(void *pReq, void *pResp)
{
	papi_response_with_msg_t *resp = (papi_response_with_msg_t*) pResp;
	LogV("fun:test_on_device_delete_pass_permision \n ");
	resp->status_code = PAPI_ERR_OK;
	resp->success = PAPI_SUCCESS;
}

void on_device_config(void *pReq, void *pResp)
{

	papi_request_dev_config *prp = (papi_request_dev_config*) pReq;
	papi_response_dev_config *resp = (papi_response_dev_config*) pResp;
	std::string dev_name_str = prp->dev_name;
	std::string location_str = prp->location;
	std::string pass_startTime_str = prp->pass_start_time;
	std::string pass_endTime_str = prp->pass_end_time;
	std::string lang_str = prp->current_lang;
	std::string flash_range = prp->flash_range;
	std::string temp_mode = prp->temp_mode;
	std::string settings_password = prp->settings_password;

	LogD("%s %s[%d] dev_name_str = %s \n", __FILE__, __FUNCTION__, __LINE__, dev_name_str.c_str());
	LogD("%s %s[%d] location_str = %s \n", __FILE__, __FUNCTION__, __LINE__, location_str.c_str());
	LogD("%s %s[%d] pass_startTime_str = %s \n", __FILE__, __FUNCTION__, __LINE__, pass_startTime_str.c_str());
	LogD("%s %s[%d] pass_endTime_str = %s \n", __FILE__, __FUNCTION__, __LINE__, pass_endTime_str.c_str());
	LogD("%s %s[%d] lang_str = %s \n", __FILE__, __FUNCTION__, __LINE__, lang_str.c_str());
	LogD("%s %s[%d] flash_range = %s \n", __FILE__, __FUNCTION__, __LINE__, flash_range.c_str());
	LogD("%s %s[%d] temp_mode = %s \n", __FILE__, __FUNCTION__, __LINE__, temp_mode.c_str());
	LogD("%s %s[%d] settings_password = %s \n", __FILE__, __FUNCTION__, __LINE__, settings_password.c_str());
	LogD("%s %s[%d] sleep_time = %s \n", __FILE__, __FUNCTION__, __LINE__, prp->sleep_time);
	LogD("%s %s[%d] visit_record = %s \n", __FILE__, __FUNCTION__, __LINE__, prp->visit_record);
	LogD("%s %s[%d] id_record = %s \n", __FILE__, __FUNCTION__, __LINE__, prp->id_record);
	LogD("%s %s[%d] id_method = %s \n", __FILE__, __FUNCTION__, __LINE__, prp->id_method);
	LogD("%s %s[%d] save_crop = %s \n", __FILE__, __FUNCTION__, __LINE__, prp->save_crop);
	LogD("%s %s[%d] delay_relay_time = %s \n", __FILE__, __FUNCTION__, __LINE__, prp->delay_relay_time);
	LogD("%s %s[%d] show_ic_name = %s \n", __FILE__, __FUNCTION__, __LINE__, prp->show_ic_name);
	LogD("%s %s[%d] enable_idcard = %s \n", __FILE__, __FUNCTION__, __LINE__, prp->enable_idcard);
	LogD("%s %s[%d] enable_have_face = %s \n", __FILE__, __FUNCTION__, __LINE__, prp->enable_have_face);

	if (dev_name_str.size() > 0)
		ReadConfig::GetInstance()->setHomeDisplay_DeviceName(dev_name_str.c_str());

	if (location_str.length() > 0)
		ReadConfig::GetInstance()->setHomeDisplay_Location(location_str.c_str());

	if (temp_mode.length() > 0 && (temp_mode == "indoor" || temp_mode == "outdoor"))
		ReadConfig::GetInstance()->setIdentity_Manager_TemperatureMode((temp_mode == "outdoor") ? 1 : 0);

	id_method mode = id_method(atoi(prp->id_method));
	QString mustOpenMode = "";
	QString optOpenMode = "";
	switch (mode)
	{
	case CARD_FACE_EITHER:
		optOpenMode = QString::number(DOOR_OPEN_MODE::ICCARD) + "|" + QString::number(DOOR_OPEN_MODE::SWIPING_FACE);
		break;
	case CARD_FACE_TWO_FACTOR:
		mustOpenMode = QString::number(DOOR_OPEN_MODE::ICCARD) + "&" + QString::number(DOOR_OPEN_MODE::SWIPING_FACE);
		break;
	case CARD_ONLY:
		mustOpenMode = QString::number(DOOR_OPEN_MODE::ICCARD);
		break;
	case FACE_ONLY:
		mustOpenMode = QString::number(DOOR_OPEN_MODE::SWIPING_FACE);
		break;
	case TEMP_ONLY:
		mustOpenMode = QString::number(DOOR_OPEN_MODE::THERMOMETRY);
		break;
	case TEMP_BTMASK_ONLY:
		mustOpenMode = QString::number(DOOR_OPEN_MODE::THERMOMETRY) +"&"+QString::number(DOOR_OPEN_MODE::MASK);
		break;
	case TEMP_BT_FACE:
		mustOpenMode = QString::number(DOOR_OPEN_MODE::THERMOMETRY) +"&"+QString::number(DOOR_OPEN_MODE::SWIPING_FACE);
		break;
	case TEMP_BT_MASK_FACE:
		mustOpenMode = QString::number(DOOR_OPEN_MODE::THERMOMETRY) +"&"+QString::number(DOOR_OPEN_MODE::MASK)+"&"+QString::number(DOOR_OPEN_MODE::SWIPING_FACE);
		break;
	case IDCARD_FACE:
		mustOpenMode = QString::number(DOOR_OPEN_MODE::IDCARD) +"&"+QString::number(DOOR_OPEN_MODE::SWIPING_FACE);
		break;
	}

	ReadConfig::GetInstance()->setDoor_MustOpenMode(mustOpenMode);
	ReadConfig::GetInstance()->setDoor_OptionalOpenMode(optOpenMode);
	ReadConfig::GetInstance()->setSaveConfig();

	memcpy(resp->message, "device config success", strlen("device config success"));
	resp->status_code = PAPI_ERR_OK;
	resp->success = PAPI_SUCCESS;

//	myHelper::Utils_Reboot();
}

void on_getdevice_config(void *pReq, void *pResp)
{
	LogD("%s %s %d \n", __FILE__, __FUNCTION__, __LINE__);
	papi_response_dev_config *resp = (papi_response_dev_config*) pResp;
#ifdef _old_version_
	std::string dev_name_str = SStting::instance().getDevName();
	std::string location_str = SStting::instance().getLocation();
	std::string support_lang_str = SStting::instance().getSupportLanguage();
	std::string current_lang_str = SStting::instance().getLanguage();
	std::string flash_range = SStting::instance().getflashrangestr();
	std::string temp_mode = SStting::instance().getTempMode();
	std::string settings_password = SStting::instance().getSettingsPassword();

	memcpy(resp->dev_name, dev_name_str.c_str(), dev_name_str.length());
	memcpy(resp->location, location_str.c_str(), location_str.length());
	memcpy(resp->current_lang, current_lang_str.c_str(), current_lang_str.length());
	memcpy(resp->support_language, support_lang_str.c_str(), support_lang_str.length());
	memcpy(resp->pass_start_time, SStting::instance().getPassStartTime().c_str(), SStting::instance().getPassStartTime().length());
	memcpy(resp->pass_end_time, SStting::instance().getPassEndTime().c_str(), SStting::instance().getPassEndTime().length());
	memcpy(resp->flash_range, flash_range.c_str(), flash_range.length());
	memcpy(resp->temp_mode, temp_mode.c_str(), temp_mode.length());
	memcpy(resp->settings_password, settings_password.c_str(), settings_password.length());

	sprintf(resp->id_record, "%d", SStting::instance().getIdRecord());
	sprintf(resp->save_crop, "%d", SStting::instance().getFaceCrop());
	sprintf(resp->visit_record, "%d", SStting::instance().getVisitRecord());
	sprintf(resp->temp_detect, "%d", SStting::instance().getTempdetect());
	sprintf(resp->sleep_time, "%d", SStting::instance().getPowerOffTime());
	sprintf(resp->id_method, "%d", SStting::instance().getIdMethod());
	sprintf(resp->delay_relay_time, "%d", SStting::instance().getDelayRelayTime());
	sprintf(resp->show_ic_name, "%d", SStting::instance().getShowIcName());
	sprintf(resp->enable_idcard, "%d", SStting::instance().getEnalbeIDCard());
	sprintf(resp->enable_have_face, "%d", SStting::instance().getEnableHaveFace());
#endif
	memcpy(resp->message, "get device config", strlen("get device config"));
	resp->status_code = PAPI_ERR_OK;
	resp->success = PAPI_SUCCESS;
}

void on_device_update(void *pReq, void *pResp)
{
	papi_request_dev_update *prp = (papi_request_dev_update*) pReq;
	papi_response_dev_update *resp = (papi_response_dev_update*) pResp;

	memcpy(resp->fw_version, prp->fw_version, strlen(prp->fw_version));
	memcpy(resp->message, "device update", strlen("device update"));
	resp->status_code = PAPI_ERR_OK;
	resp->success = PAPI_SUCCESS;
}

void on_records_delete(void *pReq, void *pResp)
{
	papi_request_records_delete *prp = (papi_request_records_delete*) pReq;
	papi_response_with_msg_t *resp = (papi_response_with_msg_t*) pResp;
	int select_type = prp->type;
	int ret = -1;
	LogD("%s %s %d \n", __FILE__, __FUNCTION__, __LINE__);

	sprintf(resp->message, "delete record of name:%s uid:%s %s~%s success", prp->name, prp->uuid, prp->start_time, prp->end_time);
#ifdef _old_version_
	switch (select_type)
	{
		case 1:
		{   //delete by name
			ret = AI_DeleteRecordByName(prp->name);
			break;
		}
		case 2:
		{   //delete by rid
			ret = AI_DeleteRecordByRId(prp->rID);
			break;
			break;
		}
		case 3:
		{   //delete by time
			ret = AI_DeleteRecordByTime(prp->start_time, prp->end_time);
			break;
		}
		case 4:
		{ //change uploadflag by jserver
			bool upload_flag = true;
			LogV("%s %s[%d] rID %ld \n", __FILE__, __FUNCTION__, __LINE__, prp->rID);
			ret = AI_UpdateRecordUploadFlagByRId(prp->rID, upload_flag);
			break;
		}
		default:
		break;
	}
#endif
	if (ret == 0)
	{
		resp->status_code = PAPI_ERR_OK;
		resp->success = PAPI_SUCCESS;
		sprintf(resp->message, "delete ok.");
	} else
	{
		resp->status_code = PAPI_FAIL;
		resp->success = PAPI_FAIL;
		sprintf(resp->message, "delete fail.");

	}
}

void on_records_select(void *pReq, void *pResp)
{
	papi_request_records_select *prp = (papi_request_records_select*) pReq;
	papi_response_records_select *resp = (papi_response_records_select*) pResp;
	//char *start_time = "1970/01/01 08:00:00";
	//char *end_time = "2020/02/15 08:00:00";
	int select_type = prp->type;
	int current_page = prp->page;
	int per_page = prp->per_page;
	int data_count = 0;
	if (per_page <= 0 || select_type <= 0)
	{
		resp->count = 0;
		resp->status_code = PAPI_FAIL;
		resp->success = PAPI_FAIL;
		return;
	}

	QList<IdentifyFaceRecord_t> list;
	switch (select_type)
	{
	case 1:
	{   //select by name
		LogD("%s %s[%d] name %s \n", __FILE__, __FUNCTION__, __LINE__, prp->name);
		resp->count = PersonRecordToDB::GetInstance()->GetPersonRecordTotalNumByName(prp->name, false);
		list = PersonRecordToDB::GetInstance()->GetPersonRecordDataByName(current_page, per_page, prp->name, false);
		data_count = list.size();
		break;
	}
	case 2:
	{   //select by uid
		LogD("%s %s[%d] uuid %s \n", __FILE__, __FUNCTION__, __LINE__, prp->uuid);
		resp->count = PersonRecordToDB::GetInstance()->GetPersonRecordTotalNumByPersonUUID(prp->uuid, false);
		list = PersonRecordToDB::GetInstance()->GetPersonRecordDataByPersonUUID(current_page, per_page, prp->uuid, false);
		data_count = list.size();
		break;
	}
	case 3:
	{   //select by time
		LogD("%s %s[%d] start %s end %s \n", __FILE__, __FUNCTION__, __LINE__, prp->start, prp->end);
		QDateTime startDateTime = QDateTime::fromString(prp->start, "yyyy/MM/dd hh:mm:ss");
		QDateTime endDateTime = QDateTime::fromString(prp->end, "yyyy/MM/dd hh:mm:ss");
		if (startDateTime.isValid() == false)
		{
			startDateTime = QDateTime::fromString(prp->start, "yyyy/MM/dd hh:mm");
		}
		if (endDateTime.isValid() == false)
		{
			endDateTime = QDateTime::fromString(prp->end, "yyyy/MM/dd hh:mm");
		}
		resp->count = PersonRecordToDB::GetInstance()->GetPersonRecordTotalNumByDateTime(startDateTime, endDateTime, false);
		list = PersonRecordToDB::GetInstance()->GetPersonRecordDataByDateTime(current_page, per_page, startDateTime, endDateTime, false);
		data_count = list.size();
		break;
	}
	case 4:
	{   //select by time and not upload
		LogD("%s %s[%d] start %s end %s \n", __FILE__, __FUNCTION__, __LINE__, prp->start, prp->end);
		QDateTime startDateTime = QDateTime::fromString(prp->start, "yyyy/MM/dd hh:mm:ss");
		QDateTime endDateTime = QDateTime::fromString(prp->end, "yyyy/MM/dd hh:mm:ss");
		if (startDateTime.isValid() == false)
		{
			startDateTime = QDateTime::fromString(prp->start, "yyyy/MM/dd hh:mm");
		}
		if (endDateTime.isValid() == false)
		{
			endDateTime = QDateTime::fromString(prp->end, "yyyy/MM/dd hh:mm");
		}
		resp->count = PersonRecordToDB::GetInstance()->GetPersonRecordTotalNumByDateTime(startDateTime, endDateTime, true);
		list = PersonRecordToDB::GetInstance()->GetPersonRecordDataByDateTime(current_page, per_page, startDateTime, endDateTime, true);
		data_count = list.size();
		break;
	}
	default:
		break;
	}

	if (data_count <= 0)
	{
		resp->count = 0;
		resp->status_code = PAPI_FAIL;
		resp->success = PAPI_FAIL;
		return;
	}

	resp->current_page = current_page;
	resp->per_page = data_count;
	resp->total_page = 0;
	per_page = data_count > per_page ? per_page : data_count;

	if (resp->count > 0)
	{
		if (resp->count < prp->per_page)
		{
			resp->total_page = 1;
		} else
		{
			if (resp->count % prp->per_page == 0)
			{
				resp->total_page = resp->count / prp->per_page;
			} else
			{
				resp->total_page = resp->count / prp->per_page + 1;
			}
		}

	}

	for (int i = 0; i < per_page; i++)
	{
		auto &t = list.at(i);
		resp->people_records[i].rID = t.rid;
		memcpy(resp->people_records[i].name, t.face_name.toStdString().c_str(), sizeof(resp->people_records[i].name));
		memcpy(resp->people_records[i].group, t.face_gids.toStdString().c_str(), sizeof(resp->people_records[i].group));
		memcpy(resp->people_records[i].time, t.createtime.toString("yyyy/MM/dd hh:mm:ss").toStdString().c_str(),
				sizeof(resp->people_records[i].time));
		memcpy(resp->people_records[i].image, t.FaceImgPath.toStdString().c_str(), sizeof(resp->people_records[i].image));
		memcpy(resp->people_records[i].uuid, t.face_uuid.toStdString().c_str(), sizeof(resp->people_records[i].uuid));
		memcpy(resp->people_records[i].gender, t.face_sex.toStdString().c_str(), sizeof(resp->people_records[i].gender));
		memcpy(resp->people_records[i].card_no, t.face_iccardnum.toStdString().c_str(), sizeof(resp->people_records[i].card_no));
		memcpy(resp->people_records[i].id_card_no, t.face_idcardnum.toStdString().c_str(), sizeof(resp->people_records[i].id_card_no));
		resp->people_records[i].person_type = t.face_persontype;
		sprintf(resp->people_records[i].temp_value, "%.1f", t.temp_value);
	}

	memcpy(resp->message, "records select", strlen("records select"));
	resp->status_code = PAPI_ERR_OK;
	resp->success = PAPI_SUCCESS;
	return;
}

void on_device_threshold(void *pReq, void *pResp)
{
	papi_request_device_threshold *prp = (papi_request_device_threshold*) pReq;
	papi_response_device_threshold *resp = (papi_response_device_threshold*) pResp;
	memcpy(resp->threshold, prp->threshold, strlen(prp->threshold));
	memcpy(resp->message, "set threshold", strlen("set threshold"));
	resp->status_code = PAPI_ERR_OK;
	resp->success = PAPI_SUCCESS;
}

void on_device_alive_level(void *pReq, void *pResp)
{
	papi_request_device_alive_level *prp = (papi_request_device_alive_level*) pReq;
	papi_response_device_alive_level *resp = (papi_response_device_alive_level*) pResp;
	memcpy(resp->alive_level, prp->alive_level, strlen(prp->alive_level));
	memcpy(resp->message, "set alive level", strlen("set alive level"));
	resp->status_code = PAPI_ERR_OK;
	resp->success = PAPI_SUCCESS;
}

void on_device_timesync(void *pReq, void *pResp)
{
	papi_request_device_timesync *prp = (papi_request_device_timesync*) pReq;
	papi_response_device_timesync *resp = (papi_response_device_timesync*) pResp;
	memcpy(resp->time, prp->time, strlen(prp->time));
	memcpy(resp->message, "set threshold", strlen("set threshold"));
	resp->status_code = PAPI_ERR_OK;
	resp->success = PAPI_SUCCESS;
}

void on_device_mode_config(void *pReq, void *pResp)
{
	papi_request_device_mode_config *prp = (papi_request_device_mode_config*) pReq;
	papi_response_with_msg_t *resp = (papi_response_with_msg_t*) pResp;
	LogV("workding_made = %s \n", prp->working_mode);
	LogV("app_key = %s \n", prp->app_key);
	LogV("app_secret = %s \n", prp->app_secret);
	LogV("orion_server = %s \n", prp->orion_server);
	LogV("orion_port = %d \n", prp->orion_port);
	resp->status_code = PAPI_ERR_OK;
	resp->success = PAPI_SUCCESS;
}

void on_device_reset_factory_settings(void *pReq, void *pResp)
{
	LogV("test_on_device_reset_factory_settings \n");
	papi_response_with_msg_t *resp = (papi_response_with_msg_t*) pResp;
	system("cp /isc/setting.json /mnt/user/setting.json && sync");
	myHelper::Utils_Reboot();
	resp->status_code = PAPI_ERR_OK;
	resp->success = PAPI_SUCCESS;
}

void on_device_del_userData(void *pReq, void *pResp)
{
	LogV("del all userdata. \n");
	papi_response_with_msg_t *resp = (papi_response_with_msg_t*) pResp;
	system("rm /mnt/user/* -Rf");
	system("sync");
	myHelper::Utils_Reboot();
	memcpy(resp->message, "del userdata ok.", strlen("del userdata ok."));
	resp->status_code = PAPI_ERR_OK;
	resp->success = PAPI_SUCCESS;
}

void on_algo_setting(void *pReq, void *pResp)
{
	LogV("reset algo setting \n");
	papi_response_with_msg_t *resp = (papi_response_with_msg_t*) pResp;
	papi_request_algo_setting *prp = (papi_request_algo_setting*) pReq;

	bool isUpdate = false;
	if (strlen(prp->ir_threshold) > 0)
	{
		ReadConfig::GetInstance()->setIdentity_Manager_Living_value(atof(prp->ir_threshold));
		isUpdate = true;
	}
	if (strlen(prp->rgb_threshold) > 0)
	{
		ReadConfig::GetInstance()->setIdentity_Manager_FqThreshold(atof(prp->rgb_threshold));
		isUpdate = true;
	}
	if (strlen(prp->sim_threshold) > 0)
	{
		isUpdate = true;
	}
	if (strlen(prp->id_threshold) > 0)
	{
		ReadConfig::GetInstance()->setIdentity_Manager_Thanthreshold(atof(prp->id_threshold));
		isUpdate = true;
	}

	if (isUpdate)
	{
		ReadConfig::GetInstance()->setSaveConfig();
		memcpy(resp->message, "update algo setting ok.", strlen("update algo setting ok."));
		resp->status_code = PAPI_ERR_OK;
		resp->success = PAPI_SUCCESS;

		myHelper::Utils_Reboot();
	} else
	{
		memcpy(resp->message, "update algo setting fail.", strlen("update algo setting fail."));
		resp->status_code = PAPI_FAIL;
		resp->success = PAPI_FAIL;
	}
}

void on_algo_reset(void *pReq, void *pResp)
{
	LogD("%s %s %d \n", __FILE__, __FUNCTION__, __LINE__);
	papi_response_with_msg_t *resp = (papi_response_with_msg_t*) pResp;

#ifdef _old_version_
	SStting::instance().setAlgoIr_threshold(0.7);
	SStting::instance().setAlgoRgb_threshold(0.5);
	SStting::instance().setAlgoSim_threshold(0.8);
	SStting::instance().setAlgoId_threshold(0.7);
#endif
	myHelper::Utils_Reboot();
	memcpy(resp->message, "reset algo setting ok.", strlen("reset algo setting ok."));
	resp->status_code = PAPI_ERR_OK;
	resp->success = PAPI_SUCCESS;
}

void on_device_get_error_code(void *pReq, void *pResp)
{
	LogV("test_on_device_get_error_code \n");
	papi_response_device_get_error_code *resp = (papi_response_device_get_error_code*) pResp;
	resp->state_msg.status_code = PAPI_ERR_OK;
	resp->state_msg.success = PAPI_SUCCESS;
	strncpy(resp->error_code, "1", sizeof(resp->error_code));
}

void on_device_set_httpserver_settings(void *pReq, void *pResp)
{
	LogD("%s %s %d \n", __FILE__, __FUNCTION__, __LINE__);
	papi_request_httpserver_info * prp = (papi_request_httpserver_info *) pReq;
	Json::Value &root = *((Json::Value *) pResp);

	LogD("%s %s %d prp->http_server_url %s  \n", __FILE__, __FUNCTION__, __LINE__,prp->http_server_url);
	LogD("%s %s %d prp->http_passwd %s  \n", __FILE__, __FUNCTION__, __LINE__,prp->http_passwd);
	if (strlen(prp->http_server_url) > 0)
	{
		ReadConfig::GetInstance()->setPost_PersonRecord_Address(prp->http_server_url);
		ReadConfig::GetInstance()->setPost_PersonRecord_Password(prp->http_passwd);
		ReadConfig::GetInstance()->setSaveConfig();
		PostPersonRecordThread::GetInstance();

		root["message"] = "ok";
		root["result"] = PAPI_SUCCESS;
		root["success"] = PAPI_SUCCESS;
	} else
	{
		root["message"] = "request param is null";
		root["result"] = PAPI_FAIL;
		root["success"] = PAPI_FAIL;
	}
}

void on_device_set_time_settings(void *pReq, void *pResp)
{
	Request &request = *((Request *) pReq);
	Json::Value &root = *((Json::Value *) pResp);

	std::string pass = request.get_param("password");
	if (pass != ReadConfig::GetInstance()->getSrv_Manager_Password().toStdString())
	{
		root["result"] = PAPI_ERR_INVALID_PASSWORD;
		root["success"] = PAPI_FAIL;
		root["message"] = "Invalid password";
		return;
	}

	std::string server_timestamp = request.get_param("time");

	if (server_timestamp.size() == 14)
	{
		char date_cmd[128];
		memset(date_cmd, 0, sizeof(date_cmd));   //date -s "2014-07-12 18:30:50"

		std::string year = server_timestamp.substr(0, 4);
		std::string mouth = server_timestamp.substr(4, 2);
		std::string day = server_timestamp.substr(6, 2);
		std::string hour = server_timestamp.substr(8, 2);
		std::string minute = server_timestamp.substr(10, 2);
		std::string secord = server_timestamp.substr(12, 2);
		sprintf(date_cmd, "/bin/date -s \"%s-%s-%s %s:%s:%s\"", year.c_str(), mouth.c_str(), day.c_str(), hour.c_str(), minute.c_str(),
				secord.c_str());
		printf("data_cmd %s \n", date_cmd);
		system(date_cmd);
		system("/sbin/hwclock -w -u");
		root["result"] = PAPI_SUCCESS;
		root["success"] = PAPI_SUCCESS;
		root["message"] = "ok";
	} else
	{
		root["message"] = "request param is error";
		root["result"] = PAPI_FAIL;
		root["success"] = PAPI_FAIL;
	}
}

void on_device_set_record_url(void *pReq, void *pResp)
{
	Request &request = *((Request *) pReq);
	Json::Value &root = *((Json::Value *) pResp);
	LogD("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
	std::string pass = request.get_param("password");
	if (QString::fromStdString(pass) != ReadConfig::GetInstance()->getSrv_Manager_Password())
	{
		root["result"] = PAPI_ERR_INVALID_PASSWORD;
		root["success"] = PAPI_FAIL;
		root["message"] = "Invalid password";
		return;
	}
	std::string record_url = request.get_param("url");
	std::string record_password = request.get_param("password");
	if (record_url.length() > 0)
	{
		ReadConfig::GetInstance()->setPost_PersonRecord_Address(QString::fromStdString(record_url));
		ReadConfig::GetInstance()->setPost_PersonRecord_Password(QString::fromStdString(record_password));
		PostPersonRecordThread::GetInstance();
		system("sync");
		root["result"] = PAPI_SUCCESS;
		root["success"] = PAPI_SUCCESS;
		root["message"] = "ok";
	} else
	{
		root["message"] = "request param is error";
		root["result"] = PAPI_FAIL;
		root["success"] = PAPI_FAIL;
	}
}

void *thread_start_server(void *arg)
{
	PAPIConfig *pConf = (PAPIConfig*) arg;

	if (ISC_NULL == pConf)
	{
		LogV("Error start server params\n");
		exit(1);
	}

	papi_v1_start_server(pConf->port, pConf->ip_addr.c_str(), pConf->password.c_str(), pConf->heartbeat_url.c_str(), pConf->sn.c_str(),
			pConf->mac.c_str(), pConf->firmware_version.c_str(), pConf->log_conf_path.c_str());
}

void start_papi_protocol(char *ip_addr)
{

	papi_v1_on_device_update_password(on_password_update);
	papi_v1_on_device_get_versioninfo(on_get_versioninfo);
	papi_v1_on_device_get_picture(on_device_getPicture);
	papi_v1_on_device_do_active(on_device_doActive);
	papi_v1_on_device_do_update(on_device_doUpdate);
	papi_v1_on_people_add(on_people_add);
	papi_v1_on_people_update(on_people_update);
	papi_v1_on_people_get_all(on_people_get_all);
	papi_v1_on_people_get_all_group(on_people_get_all_group);
	papi_v1_on_people_find(on_people_find);
	papi_v1_on_people_delete(on_people_delete);
	papi_v1_on_people_delete_all(on_people_delete_all);
	papi_v1_on_device_delete_all_records(on_device_delete_all_records);
	papi_v1_on_device_record_callback(on_device_set_callback);

	papi_v1_on_device_open_door(on_device_open_door);
	papi_v1_on_device_restart(on_device_restart);
	papi_v1_on_device_heart_beat_config(on_device_heartbeat);
	papi_v1_on_device_net_config(on_device_net_config);
	papi_v1_on_people_find_by_time(on_people_get_all_by_time);
	papi_v1_on_people_find_by_uncertain_value(on_people_get_all_by_uncertain_value);
	papi_v1_on_people_set_pass_permission(on_people_set_pass_permision);
	papi_v1_on_people_get_pass_permission(on_people_get_pass_permision);
	papi_v1_on_device_set_pass_permission(on_device_set_pass_permision);
	papi_v1_on_people_delete_pass_permission(on_people_delete_pass_permision);
	papi_v1_on_device_delete_pass_permission(on_device_delete_pass_permision);

	papi_v1_on_device_config(on_device_config);
	papi_v1_on_getdevice_config(on_getdevice_config);
	papi_v1_on_device_update(on_device_update);
	papi_v1_on_record_delete(on_records_delete);
	papi_v1_on_record_select(on_records_select);
	papi_v1_on_device_threshold(on_device_threshold);
	papi_v1_on_device_alive_level(on_device_alive_level);
	papi_v1_on_device_timesync(on_device_timesync);
	papi_v1_on_device_mode_config(on_device_mode_config);
	papi_v1_on_device_reset_factory_settings(on_device_reset_factory_settings);
	papi_v1_on_device_del_userdata(on_device_del_userData);
	papi_v1_on_algo_setting(on_algo_setting);
	papi_v1_on_algo_reset(on_algo_reset);
	papi_v1_on_device_get_error_code(on_device_get_error_code);

	papi_v1_on_device_httpserver_setting(on_device_set_httpserver_settings);
	papi_v1_on_device_time_setting(on_device_set_time_settings);
	papi_v1_on_device_record_url_setting(on_device_set_record_url);

	conf.port = 8090;
	conf.ip_addr = ip_addr;
	conf.password = ReadConfig::GetInstance()->getSrv_Manager_Password().toStdString();
	conf.heartbeat_url = "http://localhost:8090/heartbeat";
	conf.sn = "real_sn";
	conf.mac = "00:15:18:18:e6:8e";
	conf.firmware_version = "12.01.12";
	conf.log_conf_path = "/mnt/user/papi_log.conf";

	int err = pthread_create(&ntid, ISC_NULL, thread_start_server, &conf);

}

void stop_papi_protocol()
{
	printf("%s,%s\n", __FILE__, __FUNCTION__);
	papi_v1_stop_server();
}

