#pragma once

// 射频卡类型
#define ESIDCARD_RFTYPE_SA "A"
#define ESIDCARD_RFTYPE_SB "B"
#define ESIDCARD_RFTYPE_SV "V"
#define ESIDCARD_RFTYPE_SF "F"

#define ESIDCARD_RFTYPE_CA 'A'
#define ESIDCARD_RFTYPE_CB 'B'
#define ESIDCARD_RFTYPE_CV 'V'
#define ESIDCARD_RFTYPE_CF 'F'

// 证件分类
#define ESIDCARD_CLASSIFY_IDCARD 0x01	// 真实证件
#define ESIDCARD_CLASSIFY_EID 0x02		// 电子证照

// 证件类型
#define ESIDCARD_IDTYPE_N 0x00			// 大陆-身份证
#define ESIDCARD_IDTYPE_J 'J'			// 港澳台
#define ESIDCARD_IDTYPE_I 'I'			// 外国人

// 汉字要兼容 linux utf-8 编码情况
typedef struct _EsIdcardInfo
{
	char classify;				// 射频分类：1 真实证件，2 电子证照
	char idType;				// 证件类型: ESIDCARD_IDTYPE_N | ESIDCARD_IDTYPE_J | ESIDCARD_IDTYPE_I
	char szName[64];		    // 姓名：15 * 3	| 中文姓名(I证)
	char szSex[4];			    // 性别 未知 0，男 1，女 2
	char szNationCode[4];		// 民族编码(N证) | 无效(J证) | 国籍编码(I证)
	char szNation[128];		    // 民族(N证) | 无效(J证) | 国籍(I证)
	char szBirthDate[16];		// 生日
	char szAddr[256];		    // 住址：70 * 3	| 无效(I证)
	char szCert[32];		    // 证件号
	char szDep[64];			    // 签发机关：15 * 3		| 无效(I证)
	char szBegin[16];		    // 有效期-起始时间
	char szEnd[16];			    // 有效期-结束时间
	char wlt[1024];				// WLT原数据
	// J证扩展
	char szOtherIdNum[32];		// 港澳台有效，通行证号码
	char szSigningTimes[4];		// 港澳台有效，签发次数
	// I证扩展
	char szEnName[64];		    // 英文姓名
	char szVersion[4];			// 证件版本号
	char szDepCode[8];			// 受理机关代码 1500 -> 公安部/Ministry of Public Security
} EsIdcardInfo, * PEsIdcardInfo;

#define ESIDCARD_NOTIFY_ERROR_SOCKET		-4	// SOCKET 通讯失败
#define ESIDCARD_NOTIFY_ERROR_READ			-3	// 信息   读取失败
#define ESIDCARD_NOTIFY_ERROR_REQUEST		-2	// REQID  请求失败
#define ESIDCARD_NOTIFY_ERROR				-1	// IO 错误 或 认证错误
#define ESIDCARD_NOTIFY_NOFIND				0
#define ESIDCARD_NOTIFY_LEAVE				0	// 证件离开
#define ESIDCARD_NOTIFY_FIND				1	// 发现证件
#define ESIDCARD_NOTIFY_SUCCESS				2	// 证件信息提取成功
#define ESIDCARD_NOTIFY_REQID				3	// REQID 提取成功
#define ESIDCARD_NOTIFY_UUID				4	// UUID
#define ESIDCARD_NOTIFY_FIND_RF				5	// 发现射频卡
#define ESIDCARD_NOTIFY_RF_UID				6	// 射频卡ID
#define ESIDCARD_NOTIFY_RF_SUCCESS			7	// 射频卡信息提取完成
#define ESIDCARD_NOTIFY_IGNORE				8	// 卡忽略 - 相同身份证快速重刷优化机制

enum EsIdcardStatus {
	ES_IDCARD_ERROR_IO = -2,				// 读写失败 - 自动重新尝试
	ES_IDCARD_ERROR_CONNECT = -1,			// 连接失败(专用) - 通常设备掉线所致 - 自动重新尝试
	ES_IDCARD_ERROR = -1,					// 连接失败(专用) - 通常设备掉线所致 - 自动重新尝试
	ES_IDCARD_STOP = 0,						// 未启动
	ES_IDCARD_STOPPING,						// 停止中
	ES_IDCARD_STARTING,						// 启动中
	ES_IDCARD_RUNNING,						// 运行中 - 恢复连接
	ES_IDCARD_CONNECT = ES_IDCARD_RUNNING,	// 运行中 - 恢复连接
	ES_IDCARD_PAUSE							// 暂停
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

typedef  void   * PVOID;
typedef  void   * LPVOID;
typedef bool BOOL;




typedef void(*PEsIdcardCB)(int notify, const char* data, int dataLen, LPVOID lpVoid);
typedef void(*PEsIdcardStatusCB)(EsIdcardStatus status, LPVOID lpVoid);

typedef void(*PEsTempCB)(float temp, LPVOID lpVoid);
typedef void(*PEsTempStatusCB)(EsTempStatus status, LPVOID lpVoid);

typedef void(*PEsIrCB)(int v, LPVOID lpVoid);
typedef void(*PEsIrStatusCB)(EsIrStatus status, LPVOID lpVoid);

typedef void(*PEsApduCB)(unsigned char cmd, unsigned char userParam, const unsigned char* inData, int inDataLen, LPVOID lpVoid);

// 通讯方式
#define ES_EID_SDK_IOTYPE_USBHID	"USBHID"
#define ES_EID_SDK_IOTYPE_USBHID_SS	"USBHID_SS"		// SS专版 - 旧接口 - 不再支持
#define ES_EID_SDK_IOTYPE_UART		"UART"
#define ES_EID_SDK_IOTYPE_BLE		"BLE"

// 解码服务器
#define ES_EID_SDK_HOSTIDX_SIZE		0x02
#define ES_EID_SDK_HOSTIDX_J		0x00
#define ES_EID_SDK_HOSTIDX_F		0x01
