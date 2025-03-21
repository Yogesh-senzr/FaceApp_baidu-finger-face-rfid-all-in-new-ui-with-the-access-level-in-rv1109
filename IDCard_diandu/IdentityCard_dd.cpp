#include "IdentityCard_dd.h"
#include "PCIcore/RkUtils.h"
#include "PCIcore/Audio.h"
#include "EidSdkStruct.hpp"
#include "EsEidSdk.hpp"
#include "ISdkLog.hpp"

#include "Config/ReadConfig.h"

#include <QWaitCondition>
#include <QDebug>
#include <QMutex>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "MessageHandler/Log.h"
#include <QKeyEvent>
#include <QApplication>
#include <QProcess>

#define __USE_USB_PORT__

#define UART_PORT  ("/dev/ttyS4")
extern char DK_IDCardNumber[20];

extern int g_cardRegister;
#define ISC_NULL                 0L	
class IdentityCardDDPrivate
{
    Q_DECLARE_PUBLIC(IdentityCardDD)
public:
    IdentityCardDDPrivate(IdentityCardDD *dd);
private:
    void DDCheckIdCard();
    //int read_identfy_info(Tidentify_info info,unsigned char photo[],int photoLen);
   static void EsIdcardCB(int notify, const char* data, int dataLen, LPVOID lpVoid);    	
   static void EsIdcardStatusCB(EsIdcardStatus status, LPVOID lpVoid);
   //void EsIdcardCB(int notify, const char* data, int dataLen, LPVOID lpVoid);  
  // void sendparam(PEsIdcardInfo &pInfo);  	
private:
    mutable QMutex sync;
    QWaitCondition pauseCond;
	pthread_mutex_t mIDCardMutex;	
private:
    bool mExit;
private:
    IdentityCardDD *const q_ptr;
};


long EsLogTime() {
	// 真实时间
	//struct timeval tv;
	//gettimeofday(&tv, NULL);
	//return tv.tv_sec * 1000 + tv.tv_usec / 1000;
	// 开机毫秒数
printf(">>>>%s,%s,%d \n",__FILE__,__func__,__LINE__);		
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

void ESLog(const char* fmt, ...)
{
	printf(">>>>%s,%s,%d \n",__FILE__,__func__,__LINE__);	
	va_list argList;
	va_start(argList, fmt);
	// time
	long tick = EsLogTime();
	int msecs = (int)(tick % 1000);
	tick /= 1000;
	int secs = (int)(tick % 60);
	tick /= 60;
	int minutes = (int)(tick % 60);
	tick /= 60;
	int hours = (int)tick;
	// buf
	char buf[64] = { 0 };
	sprintf(buf, "[%02d:%02d:%02d:%03d]", hours, minutes, secs, msecs);
	printf("%-16s", buf);
	vprintf(fmt, argList);
	printf("\n");
	va_end(argList);
}

void LogCB(ES_LOG_LEVEL level, const char* szMsg, LPVOID lpVoid)
{
	printf(">>>>%s,%s,%d \n",__FILE__,__func__,__LINE__);	
    printf("%s\n", szMsg);
}

IdentityCardDDPrivate::IdentityCardDDPrivate(IdentityCardDD *dd)
    : q_ptr(dd)
    , mExit(false)
{ 
	LogD(">>>>%s,%s,%d \n",__FILE__,__func__,__LINE__);	
    pthread_mutex_init(&mIDCardMutex, ISC_NULL);
    //QObject::connect(q_func(), &IdentityCardDD::sigIdCardInfoDD, q_func(), &IdentityCardDD::slotIdCardInfoDDlocal);    	
}
#if 1

#if 0
void IdentityCardDDPrivate::sendparam(PEsIdcardInfo &pInfo)
{
  emit q_func()->sigIdCardInfoDD(QString::fromLocal8Bit(pInfo->szName), QString(pInfo->szCert), QString(pInfo->szSex[0] ? "男" : "女"), QString("/mnt/user/esphoto.bmp"));
}
#endif 

//void IdentityCardDD::EsIdcardCB(int notify, const char* data, int dataLen, LPVOID lpVoid)
void IdentityCardDDPrivate::EsIdcardCB(int notify, const char* data, int dataLen, LPVOID lpVoid)
{
	printf(">>>>%s,%s,%d \n",__FILE__,__func__,__LINE__);	
	#if 1
printf(">>>>%s,%s,%d,notify=%d\n",__FILE__,__func__,__LINE__,notify);	
	if (notify == ESIDCARD_NOTIFY_FIND) {
		printf("EsIdcardCB: find card\n");
	}
	if (data && notify == ESIDCARD_NOTIFY_SUCCESS) {
    printf(">>>>%s,%s,%d\n",__FILE__,__func__,__LINE__);
		PEsIdcardInfo pInfo = (PEsIdcardInfo)data;
		printf("EsIdcardCB: name: %s\n", pInfo->szName);
		printf("EsIdcardCB: cert: %s\n", pInfo->szCert);
		printf("EsIdcardCB: addr: %s\n", pInfo->szAddr);

		//printf("EsIdcardCB: addr: %s\n", pInfo->szAddr);
		char bmp[38862];//char bmp[38862];
		ES_EIDSDK_WltUnpack(pInfo->wlt, 1024, bmp,38862);

		FILE *fp;
		fp = fopen("/mnt/user/esphoto.bmp","w");
        fwrite(bmp, sizeof(bmp) , 1, fp );

		QString szName = QString(pInfo->szName);
 
        fclose(fp);
		//sendparam((PEsIdcardInfo &)pInfo);
        //emit q_func()->sigIdCardInfo(QString::fromLocal8Bit(pInfo->szName), QString(pInfo->szCert), QString(pInfo->szSex[0] ? "男" : "女"), QString("/mnt/user/esphoto.bmp"));
	    printf(">>>>%s,%s,%d step  1 \n",__FILE__,__func__,__LINE__);	
		IdentityCardDD::GetInstance()->sendIdCardInfo(QString(pInfo->szName), QString(pInfo->szCert), QString(pInfo->szSex[0] ? "男" : "女"), QString("/mnt/user/esphoto.bmp"));
	}

	if  (data && notify == ESIDCARD_NOTIFY_ERROR_SOCKET) { 

		static int count = 0;
		if(count++ == 3)
		{
			count = 0;
			YNH_LJX::Audio::Audio_PlayCustomerPcm("zh", "Buzzer_neterror.wav", false);//放在这里嘀 //Buzzer.wav
			//sleep(1);
			//YNH_LJX::Audio::Audio_PlayCustomerPcm("zh", "net_error.wav", false);//放在这里嘀    		
		}

	}
#endif 	
}

void IdentityCardDDPrivate::EsIdcardStatusCB(EsIdcardStatus status, LPVOID lpVoid) 
{
	printf(">>>>%s,%s,%d \n",__FILE__,__func__,__LINE__);	
	switch (status)
	{
	case ES_IDCARD_ERROR_IO:
		ESLog("ES_IDCARD_ERROR_IO");
		break;
	case ES_IDCARD_ERROR_CONNECT:
		// 设备连接失败
		// 1. 硬件接线 不正确触发
		// 2. 主动发送 设备重启指令触发
		ESLog("ES_IDCARD_ERROR_CONNECT");
		break;
	case ES_IDCARD_STOP:
		ESLog("ES_IDCARD_STOP");
		break;
	case ES_IDCARD_STOPPING:
		ESLog("ES_IDCARD_STOPPING");
		break;
	case ES_IDCARD_STARTING:
		ESLog("ES_IDCARD_STARTING");
		break;
	case ES_IDCARD_RUNNING: {
		ESLog("ES_IDCARD_RUNNING");
		// 只有进入 ES_IDCARD_RUNNING 状态之后，才可以成功获取设备编号
		// TODO: 设备序号, 建议显示到界面或通过其他方式管理, 以便于后续对读卡器设备进行授权管控
		char szSamID[32] = { 0 };
		ES_EIDSDK_GetSamID(szSamID);
		ESLog("SamID: %s", szSamID);
		// 固件版本号
		char proVersion[32] = { 0 };
		ES_EIDSDK_GetProVersion(proVersion);
		ESLog("Soft Version: %s", proVersion);
		// 硬件版本号 - 用于区分读卡器型号
		char hardVersion[32] = { 0 };
		ES_EIDSDK_GetHardVersion(hardVersion);
		ESLog("Hard Version: %s", proVersion);
		break;
	}
	case ES_IDCARD_PAUSE:
		ESLog("ES_IDCARD_PAUSE");
		break;
	default:
		break;
	}
}
#endif 

void IdentityCardDD::sendIdCardInfo(const QString name, const QString idCard, const QString sex, const QString path)
{
	printf(">>>>%s,%s,%d \n",__FILE__,__func__,__LINE__);	
	//printf(">>>>%s,%s,%d step  1.1 name=%s,idCard=%s,sex=%s,path=%s\n",__FILE__,__func__,__LINE__,\
	   name.toStdString().c_str(), idCard.toStdString().c_str(), sex.toStdString().c_str(), path.toStdString().c_str());	 
    emit sigIdCardInfoDD(name, idCard,  sex,  path);
	//printf(">>>>%s,%s,%d step  1.2 \n",__FILE__,__func__,__LINE__);	 
}

IdentityCardDD::IdentityCardDD(QObject *parent)
    : QThread(parent),
     d_ptr(new IdentityCardDDPrivate(this))
{ 
	LogD(">>>>%s,%s,%d \n",__FILE__,__func__,__LINE__);
	Q_D(IdentityCardDD);	
	g_cardRegister  = 0;

#if 0
	ES_EIDSDK_SetUart("/dev/ttyS4", 115200);
	ES_EIDSDK_Init(ES_EID_SDK_IOTYPE_UART, "eid_demo");

	//ES_EIDSDK_SetLogCB(LogCB, NULL);
	ES_EIDSDK_SetIdcardCB(*(d->EsIdcardCB),NULL);
	//ES_EIDSDK_SetIdcardStatusCB(EsIdcardStatusCB, NULL);	
	// ----------------------------------------------------
    
#if 0	
    QString door_mode = ReadConfig::GetInstance()->getDoor_MustOpenMode();
    QString Optional_door_mode = ReadConfig::GetInstance()->getDoor_OptionalOpenMode();
    //包含  人证比对 及身份证 

 //   if ((door_mode.contains(Door_OpenModeToSTR(_DOOR_OPEN_MODE::PERSON_IDCARD)) || Optional_door_mode.contains(Door_OpenModeToSTR(_DOOR_OPEN_MODE::PERSON_IDCARD)) )\
	    && (door_mode.contains(Door_OpenModeToSTR(_DOOR_OPEN_MODE::IDCARD)) || Optional_door_mode.contains(Door_OpenModeToSTR(_DOOR_OPEN_MODE::IDCARD)) ))
		
    if ((door_mode.contains("7") || Optional_door_mode.contains("7") )   && (door_mode.contains("6") || Optional_door_mode.contains("6") ))
	  ES_EIDSDK_SetImg(true);
	else 
	  ES_EIDSDK_SetImg(false);
#endif 	  

	if (!ES_EIDSDK_Start())
	{
		LogD("ES_EIDSDK_Start error");
		return;
	}
	LogD("ES_EIDSDK_Start\n");
#endif 
 	
	LogD(">>>>%s,%s,%d \n",__FILE__,__func__,__LINE__);
    //return str;
   
	//getchar();

	//printf("-------------- stop --------------\n");
	//ES_EIDSDK_Stop();
	
	   this->start();
}

#if 0
void IdentityCardDD::slotIdCardInfoDDlocal(const QString name, const QString idCard, const QString sex, const QString path)
{
	printf(">>>>%s,%s,%d step  1.1 name=%s,idCard=%s,sex=%s,path=%s\n",__FILE__,__func__,__LINE__,
	   name.toStdString().c_str(), idCard.toStdString().c_str(), sex.toStdString().c_str(), path.toStdString().c_str());
}	
#endif 
IdentityCardDD::~IdentityCardDD()
{
	LogD(">>>>%s,%s,%d \n",__FILE__,__func__,__LINE__);
    Q_D(IdentityCardDD);
    Q_UNUSED(d);
    this->requestInterruption();
    d->mExit = true;
    d->pauseCond.wakeOne();

    this->quit();
    this->wait();
}

//在显示屏上可以增加身份证解析进度,提醒用户不要马上拿开身份证
static inline void schedule_printf(uint8_t rate)
{
    printf(">>>>%s,%s,%d \n",__FILE__,__func__,__LINE__);	
}

static inline int Utils_getFileSize(const char *path)
{
	printf(">>>>%s,%s,%d \n",__FILE__,__func__,__LINE__);	
    int size = 0;
    struct stat buf;
    if (stat(path, &buf) < 0)
    {
        return 0;
    }
    size = buf.st_size;
    return size;
}


void IdentityCardDDPrivate::DDCheckIdCard()
{
	LogD(">>>>%s,%s,%d \n",__FILE__,__func__,__LINE__);	
	pthread_mutex_lock(&mIDCardMutex);	
    int ret;
    uint8_t out_data[4096];
    uint16_t out_len;
    uint8_t id_info[256], id_bitmap[4096];
    uint16_t wordMsgBytesLen;
    uint16_t imageMsgBytesLen;
    uint16_t useful_message_index;
    //阻塞等待，没有读到身份证超时为3秒
    ret = 0;

	unsigned char ucSAMID[16];
	unsigned char messageInfo[256];
	unsigned int info_len = 0;
	unsigned int photo_len = 0;
	unsigned char photo[1024+16];//16 encrypt Type
	unsigned char ucIIN[4];
	unsigned char ucSN[8];
	unsigned int FPMsg_len = 0;
	unsigned char FPMsg[1024+16];


	pthread_mutex_unlock(&mIDCardMutex);
  
}


void IdentityCardDD::run()
{
    Q_D(IdentityCardDD);
#if 0
	while(true)
	{

	   d->DDCheckIdCard();
   	   usleep(100*1000);//1000*100;1000*1000

	}
#endif 	
					printf(">>>>>%s,%s,%d,es_idcard_demo\n",__FILE__,__func__,__LINE__);
#if 0	
    QString cmd ="/isc/bin/es_idcard_demo uart /dev/ttyS4";
	//QString cmd ="/isc/bin/ddDemo uart /dev/ttyS4 &";
	QString str = "";
	printf(">>>>>%s,%s,%d,\n",__FILE__,__func__,__LINE__);	
	//while (true) 
	{
		FILE *pFile = popen(cmd.toStdString().c_str(), "r");
		if (pFile)
		{
		printf(">>>>>%s,%s,%d,\n",__FILE__,__func__,__LINE__);			
			std::string ret = "";
			char buf[256] = { 0 };
			int readSize = 0;
			while(1)
			{

		printf(">>>>>%s,%s,%d,\n",__FILE__,__func__,__LINE__);				
				readSize = fread(buf, 1, sizeof(buf), pFile);
		printf(">>>>>%s,%s,%d,\n",__FILE__,__func__,__LINE__);				
				if (readSize > 0)
				{
					ret += std::string(buf, 0, readSize);
					printf(">>>>>%s,%s,%d,ret=%s\n",__FILE__,__func__,__LINE__,ret.c_str());
				}
		printf(">>>>>%s,%s,%d,\n",__FILE__,__func__,__LINE__);				
				if (readSize == 0) {
				ret="";
				}
			}
		printf(">>>>>%s,%s,%d,\n",__FILE__,__func__,__LINE__);				
				usleep(100*1000);
		printf(">>>>>%s,%s,%d,\n",__FILE__,__func__,__LINE__);				 


				printf(">>>>>%s,%s,%d,\n",__FILE__,__func__,__LINE__);		
			pclose(pFile);

			//str = QString::fromStdString(ret);
		}
	}
					printf(">>>>>%s,%s,%d,\n",__FILE__,__func__,__LINE__);	
#endif
#if 0
    QProcess p(0);

    p.start("cmd");
    p.waitForStarted();
    //p.write(QString("/isc/bin/ddDemo uart /dev/ttyS4 "));
	p.write("/isc/bin/ddDemo uart /dev/ttyS4 ");
    p.closeWriteChannel();
    p.waitForFinished(1000);
    p.close();

#endif 
#if 0
QString command="/isc/bin/ddDemo uart /dev/ttyS4" ;
QProcess geomprocess(this);
geomprocess.setWorkingDirectory("工作空间");
geomprocess.start(command);
geomprocess.waitForStarted();
QEventLoop loop; //创建一个事件循环，等待进程调用完毕
//绑定进程可以读取通道内容信号，当有可以读取的内容时，readyReadStandardOutput信号被发出
connect(&geomprocess,&QProcess::readyReadStandardOutput,this,[&loop,&geomprocess]()
{
 //读取通道内所有内容
    QString data=QString::fromLocal8Bit(geomprocess.readAllStandardOutput());

	printf(">>>ddDemo  data=%s\n",data.toStdString().c_str());
    //判断读取到的内容是否包含特定字符串
#if 0	
    if(data.contains("press enter key to stop"))
    {
        geomprocess.close();
        geomprocess.kill();
        loop.quit(); //杀死进程，退出事件循环
    }
#endif 	
});
loop.exec();
#endif 
LogD(">>>>%s,%s,%d \n",__FILE__,__func__,__LINE__);	
#if 1
	ES_EIDSDK_SetUart("/dev/ttyS4", 115200);
	ES_EIDSDK_Init(ES_EID_SDK_IOTYPE_UART, "eid_demo");

	ES_EIDSDK_SetLogCB(LogCB, NULL);
	ES_EIDSDK_SetIdcardCB(*(d->EsIdcardCB),NULL);
	ES_EIDSDK_SetIdcardStatusCB(d->EsIdcardStatusCB, NULL);	
	// ----------------------------------------------------
    
#if 0	
    QString door_mode = ReadConfig::GetInstance()->getDoor_MustOpenMode();
    QString Optional_door_mode = ReadConfig::GetInstance()->getDoor_OptionalOpenMode();
    //包含  人证比对 及身份证 

 //   if ((door_mode.contains(Door_OpenModeToSTR(_DOOR_OPEN_MODE::PERSON_IDCARD)) || Optional_door_mode.contains(Door_OpenModeToSTR(_DOOR_OPEN_MODE::PERSON_IDCARD)) )\
	    && (door_mode.contains(Door_OpenModeToSTR(_DOOR_OPEN_MODE::IDCARD)) || Optional_door_mode.contains(Door_OpenModeToSTR(_DOOR_OPEN_MODE::IDCARD)) ))
		
    if ((door_mode.contains("7") || Optional_door_mode.contains("7") )   && (door_mode.contains("6") || Optional_door_mode.contains("6") ))
	  ES_EIDSDK_SetImg(true);
	else 
	  ES_EIDSDK_SetImg(false);
#endif 	  

	if (!ES_EIDSDK_Start())
	{
		LogD("ES_EIDSDK_Start error");
		return;
	}
	LogD("ES_EIDSDK_Start\n");
#endif 

}

