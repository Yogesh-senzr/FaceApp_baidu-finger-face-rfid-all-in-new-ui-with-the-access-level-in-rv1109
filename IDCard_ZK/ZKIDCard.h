#ifndef __ZKIDCARD_H__
#define __ZKIDCARD_H__

#include <pthread.h>
#include <QObject>

#define ERROR_OK                    0
#define ERROR_FAILED                -1
#define ERROR_USB_DEVICE_INIT       -2
#define ERROR_USB_DEVICE_OPEN       -3
#define ERROR_SERIAL_PORT           -4
#define ERROR_MEMORY_IS_NULL        -5
#define ERROR_MEMORY_IS_NOT_ENOUGH  -6
#define ERROR_FILE_NOT_EIXT         -7
#define ERROR_READ_FILE             -8
#define ERROR_UNSUPPORT             -9
#define ERROR_DLOPEN                -10
#define ERROR_DLSYM                 -11

#define ERROR_MSG_LEN -1001
#define ERROR_MF_GET_SER_NUM -1002

//logLevel
 #define    VERBOSE 2
 #define    DEBUG 3
 #define    INFO 4
 #define    WARN 5
 #define    ERROR 6
 #define    ASSERT 7

typedef struct __MW_OPTIONS1__{
    unsigned char name[30];		/* 姓名  */
    unsigned char gender[2];
    unsigned char national[4];
    unsigned char birthday[16];
    unsigned char address[70];
    unsigned char id[36];
    unsigned char maker[30];
    unsigned char start_date[16];
    unsigned char end_date[16];
    unsigned char reserved[36];

}__attribute__((packed)) Tidentify_info, *Pidentify_info;

typedef struct __MW_OPTIONS2__{
    unsigned char englishName[120];//英文姓名
    unsigned char gender[2];//性别
    unsigned char id[30];//永久居留证号码
    unsigned char areaCode[6];//国籍或所在区域代码
    unsigned char chineseName[30];//中文姓名
    unsigned char start_date[16];//证件签发日期
    unsigned char end_date[16];//证件终止日志
    unsigned char birthday[16];//出生日期
    unsigned char versionNumber[4];//证件版本号
    unsigned char officeNumber[8];//当次申请授理机关代码
    unsigned char cardType[2];//证件类型标识
    unsigned char reserved[36];//预留项

}__attribute__((packed)) Treside_info, *Preside_info;

typedef struct __MW_OPTIONS3__{
    unsigned char name[30];		/* 姓名  */
    unsigned char gender[2]; //性别
    unsigned char reserved1[4];//预留区
    unsigned char birthday[16];//出生日期
    unsigned char address[70];//住址
    unsigned char id[36];//公民身份证号码
    unsigned char maker[30];//签发机关
    unsigned char start_date[16];//有效起始日期
    unsigned char end_date[16];//有效截止日期

    unsigned char passNumber[18];//通行证号码
    unsigned char lssueTimes[4];//签发次数
    unsigned char reserved2[6];//预留区
    unsigned char cardType[2];//证件类型标识
    unsigned char reserved3[6];//预留区

}__attribute__((packed)) Tgangaotai_info, *Pgangaotai_info;


#ifdef __cplusplus
extern "C"
{
#endif

int ZKID_Init();
void ZKID_Free();
int ZKID_OpenPort(int iPort);
int ZKID_OpenPortEx(const char * dev);
int ZKID_ClosePort(int iPort);
void ZKID_SetLogType(int logType,char*logFilePath);
void ZKID_SetLogLevel(int logLeve);

int ZKID_GetSAMStatus(int iPort,int bIfOpen);
int ZKID_ResetSAM(int iPort,int bIfOpen);
int ZKID_GetSAMIDToStr(int iPort,char *pcSAMID,int iIfOpen);
int ZKID_GetSAMID(int iPort,unsigned char *pucSAMID,int iIfOpen);
int ZKID_StartFindIDCard(int iPort,unsigned char *pucIIN,int bIfOpen);
int ZKID_GetCOMBaud(int iPort,unsigned int  *puiBaud);
int ZKID_SetCOMBaud(int iPort,unsigned int  uiCurrBaud,unsigned int  uiSetBaud);
int ZKID_SetMaxRFByte(int iPort,unsigned char ucByte,int bIfOpen);
int ZKID_SelectIDCard(int iPort,unsigned char *pucSN,int bIfOpen);
int ZKID_ReadBaseMsg(int iPort,unsigned char * pucCHMsg,unsigned int *	puiCHMsgLen,unsigned char * pucPHMsg,unsigned int  *puiPHMsgLen,int bIfOpen);
int ZKID_ReadBaseFPMsg(int iPort,unsigned char *pucCHMsg,unsigned int *puiCHMsgLen,unsigned char *pucPHMsg,unsigned int *puiPHMsgLen,unsigned char *pucFPMsg,unsigned int *puiFPMsgLen,int bIfOpen);
int ZKID_ReadNewAppMsg(int iPort, unsigned char *pucAppMsg, unsigned int *puiAppMsgLen, int iIfOpen);
int ZKID_ReadCardMsgSN(int iPort, unsigned char *pucMsgSN, int iIfOpen);
int ZKID_PHunpack(char *wltData, char *bgr,int cbBgr);
int ZKID_Wlt2bmpBuffer(unsigned char* wltData, unsigned char* imageData, int cbImageData);


int ZKID_MFInit(int iPort);
int ZKID_Mifare_REQA(int iPort,unsigned char mode);
int ZKID_SetBaudRate(int iPort,unsigned int baud);
int ZKID_Mifare_AnticollA(int iPort,unsigned int *CardNumber);
int ZKID_Mifare_SelectA(int iPort,unsigned int uid);
int ZKID_Mifare_GetSerNum(int iPort,unsigned char *SerNum);
int ZKID_Mifare_SetSerNum(int iPort,unsigned char *SerNum);
int ZKID_Mifare_HaltA(int iPort);
int ZKID_Mifare_Read(int iPort,unsigned char addr, unsigned char blocks, unsigned char *key,unsigned char auth,unsigned char mode,unsigned char *buf,unsigned int * uid);
int ZKID_Mifare_Write(int iPort,unsigned char addr,unsigned char blocks,unsigned char *key,unsigned char auth, unsigned char mode,unsigned char * buf, unsigned int * uid, int protect);
int ZKID_Mifare_Get_SNR(int iPort,unsigned char mode,unsigned char halt,unsigned char *serialnumber);
int ZKID_Mifare_Get_SNR_SFZ(int iPort,unsigned char mode,unsigned char halt,unsigned char *serialnumber);
void ZKID_Mifare_Perror(int errno);




void GetCardType(unsigned char* buffer,char*CardType);
/****************身份证*********************/
void getIDName(Tidentify_info info,char*Name);
void getIDGender(Tidentify_info info,char* Gender);
void getIDNational(Tidentify_info info,char*National);
void getIDBirthday(Tidentify_info info,char*Birthday);
void getIDAddress(Tidentify_info info,char*Address);
void getIDCardNumber(Tidentify_info info,char*IDCardNumber);
void getIDMaker(Tidentify_info info,char*Maker);
void getIDStartDate(Tidentify_info info,char*StartDate);
void getIDEndDate(Tidentify_info info,char*EndDate);
/*****************永居证*******************/
void getResideEnglishName(Treside_info info,char*EnglishName);
void getResideGender(Treside_info info,char*Gender);
void getResideId(Treside_info info,char*Id);
void getResideAreaCode(Treside_info info,char*AreaCode);
void getResideChineseName(Treside_info info,char*ChineseName);
void getResideStartDate(Treside_info info,char*StartDate);
void getResideEndDate(Treside_info info,char*EndDate);
void getResideBirthday(Treside_info info,char*Birthday);
void getResideVersionNumber(Treside_info info,char*VersionNumber);
void getResideOfficeNumber(Treside_info info,char*OfficeNumber);
void getResideCardType(Treside_info info,char*CardType);
void getResideReserved(Treside_info info,char*Reserved);

/****************港澳台居住证********************/
void getGATName(Tgangaotai_info info,char*Name);
void getGATGender(Tgangaotai_info info,char* Gender);
void getGATReserved1(Tgangaotai_info info,char*Reserved1);
void getGATBirthday(Tgangaotai_info info,char*Birthday);
void getGATAddress(Tgangaotai_info info,char*Address);
void getGATCardNumber(Tgangaotai_info info,char*IDCardNumber);
void getGATMaker(Tgangaotai_info info,char*Maker);
void getGATStartDate(Tgangaotai_info info,char*StartDate);
void getGATEndDate(Tgangaotai_info info,char*EndDate);
void getGATPassNumber(Tgangaotai_info info,char*PassNumber);
void getGATLssueTimes(Tgangaotai_info info,char*LssueTimes);
void getGATReserved2(Tgangaotai_info info,char*Reserved2);
void getGATCardType(Tgangaotai_info info,char*CardType);
void getGATReserved3(Tgangaotai_info info,char*Reserved3);


#ifdef __cplusplus
}
#endif

class ZKIDCard
{
public:
	~ZKIDCard();
	static ZKIDCard* getInstance();

private:
	ZKIDCard();
	static ZKIDCard *mThis;
	void usb_one_devie();
	void usb_fun(int deviceIndex);
	int read_info_ex(int index);
	int read_identfy_info(Tidentify_info info,unsigned char photo[],int photoLen);
	int writePhoto(unsigned char photo[],int photoLen,char *imageName);

	static bool isAddCard; //卡信息是否加载到数据库

public:
	void IDCardRun();

	void deleteIDCardInfo(int mdelay);
	bool getIDCardState();
	void setIDCardState(bool isAdd);
public://上传当前身份证信息
    Q_SIGNAL void sigIdCardInfo(const QString name, const QString idCard, const QString sex, const QString path);
public:
	pthread_mutex_t mIDCardMutex;
};


#endif
