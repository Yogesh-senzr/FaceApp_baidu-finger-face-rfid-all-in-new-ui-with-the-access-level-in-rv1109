#pragma once

// 射频卡类型
#define ESIDCARD_RFTYPE_A "A"
#define ESIDCARD_RFTYPE_B "B"
#define ESIDCARD_RFTYPE_V "V"
#define ESIDCARD_RFTYPE_F "F"

// 证件分类
#define ESIDCARD_CLASSIFY_IDCARD 0x01	// 真实证件
#define ESIDCARD_CLASSIFY_EID 0x02		// 电子证照

// 证件类型
#define ESIDCARD_IDTYPE_N 0x00			// 常规-身份证
#define ESIDCARD_IDTYPE_J 'J'			// 港澳台
#define ESIDCARD_IDTYPE_I 'I'			// 外国人

// 汉字要兼容 linux utf-8 编码情况
typedef struct _EsIdcardInfo
{
	char classify;				// 射频分类：1 真实证件，2 电子证照
	char idType;				// 证件类型
	char szName[64];		    // 姓名：15 * 3
	char szSex[4];			    // 性别 未知 0，男 1，女 2
	char szNation[8];		    // 常规-身份证有效，民族: 汉字
	char szBirthDate[16];		// 生日
	char szAddr[256];		    // 住址：70 * 3
	char szCert[32];		    // 身份证号
	char szDep[64];			    // 签发机关：15 * 3
	char szBegin[16];		    // 有效期-起始时间
	char szEnd[16];			    // 有效期-结束时间
	char szOtherIdNum[32];		// 港澳台有效，通行证号码
	char szSigningTimes[4];		// 签发次数
	char wlt[1024];				// WLT原数据
}EsIdcardInfo, *PEsIdcardInfo;

#define ESIDCARD_NOTIFY_ERROR_SOCKET		-4	// SOCKET 通讯失败
#define ESIDCARD_NOTIFY_ERROR_READ			-3	// 信息   读取失败
#define ESIDCARD_NOTIFY_ERROR_REQUEST		-2	// REQID  请求失败
#define ESIDCARD_NOTIFY_ERROR				-1
#define ESIDCARD_NOTIFY_NOFIND				0
#define ESIDCARD_NOTIFY_LEAVE				0	// 证件离开
#define ESIDCARD_NOTIFY_FIND				1	// 发现证件
#define ESIDCARD_NOTIFY_SUCCESS				2	// 证件信息提取成功
#define ESIDCARD_NOTIFY_REQID				3	// REQID 提取成功
#define ESIDCARD_NOTIFY_UUID				4	// UUID
#define ESIDCARD_NOTIFY_FIND_RF				5	// 发现射频卡
#define ESIDCARD_NOTIFY_RF_UID				6	// 射频卡ID
#define ESIDCARD_NOTIFY_RF_SUCCESS			7	// 射频卡信息提取完成

enum EsIdcardStatus {
	ES_IDCARD_ERROR_IO = -2,		// 读写失败 - 自动重新尝试
	ES_IDCARD_ERROR = -1,			// 连接失败 - 自动重新尝试
	ES_IDCARD_STOP = 0,				// 未启动
	ES_IDCARD_STOPPING,				// 停止中
	ES_IDCARD_STARTING,				// 启动中
	ES_IDCARD_RUNNING,				// 运行中
	ES_IDCARD_PAUSE					// 暂停
};

enum EsTempStatus {
	ES_TEMP_ERROR = -1,				// 出错
	ES_TEMP_SUCCESS = 0,			// 恢复
};

enum EsIrStatus {
	ES_IR_NOFIND = 0,
	ES_IR_LEAVE = 0,				// 离开
	ES_IR_FIND,						// 发现
};

typedef void(*PEsIdcardCB)(int notify, const char* data, int dataLen, LPVOID lpVoid);
typedef void(*PEsIdcardStatusCB)(EsIdcardStatus status, LPVOID lpVoid);

typedef void(*PEsTempCB)(float temp, LPVOID lpVoid);
typedef void(*PEsTempStatusCB)(EsTempStatus status, LPVOID lpVoid);

typedef void(*PEsIrCB)(int v, LPVOID lpVoid);
typedef void(*PEsIrStatusCB)(EsIrStatus status, LPVOID lpVoid);

typedef void(*PEsApduCB)(unsigned char cmd, unsigned char userParam, const unsigned char* inData, int inDataLen, LPVOID lpVoid);
