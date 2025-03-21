#include "ZKIdentityCard.h"
#include "PCIcore/RkUtils.h"
#include "PCIcore/Audio.h"
#include "ZKIDCard.h"

#include <QWaitCondition>
#include <QDebug>
#include <QMutex>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "MessageHandler/Log.h"


#define __USE_USB_PORT__

#define UART_PORT  ("/dev/ttyS4")
extern char DK_IDCardNumber[20];

extern int g_cardRegister;
#define ISC_NULL                 0L	
class ZKIdentityCardPrivate
{
    Q_DECLARE_PUBLIC(ZKIdentityCard)
public:
    ZKIdentityCardPrivate(ZKIdentityCard *dd);
private:
    void ZKCheckIdCard();
    int read_identfy_info(Tidentify_info info,unsigned char photo[],int photoLen);
private:
    mutable QMutex sync;
    QWaitCondition pauseCond;
	pthread_mutex_t mIDCardMutex;	
private:
    bool mExit;
private:
    ZKIdentityCard *const q_ptr;
};


ZKIdentityCardPrivate::ZKIdentityCardPrivate(ZKIdentityCard *dd)
    : q_ptr(dd)
    , mExit(false)
{ 
    pthread_mutex_init(&mIDCardMutex, ISC_NULL);
}

ZKIdentityCard::ZKIdentityCard(QObject *parent)
    : QThread(parent)
    , d_ptr(new ZKIdentityCardPrivate(this))
{ 
	g_cardRegister  = 0;

	int ret = 0;
	int deviceCount=ZKID_Init();
	printf("deviceCount=%d\n",deviceCount);
	if(deviceCount<=0)
	{
		printf("device Init failed!\n");
		return;
	}
 
    int deviceIndex1 = 0;
	char SAMIDString[100]={0x0};
	printf("deviceIndex=%d !\n",deviceIndex);
	if(deviceIndex1<0)
	{
		return ;
	}
	int index = ZKID_OpenPort(deviceIndex1);
	if(index < 0)
	{
		printf("open device index%d failed !\n",deviceIndex1);
		return ;
	}
	printf("open device index%d Successed !\n",deviceIndex1);

	/* 读取设备状态, 检测设备是否可以正常工作     */
	if(ZKID_GetSAMStatus(index, 0) < 0)
	{
        usleep(100*1000);
        if(ZKID_GetSAMStatus(index, 0)<0)
        {
            printf("devce index%d,get sam mode status failed\n",index);
            return ;
        }
	}

	/*读取设备唯一标识符*/
	ZKID_GetSAMIDToStr(index,SAMIDString,0);

	g_cardRegister  = 1;
    deviceIndex = index;

	int count=0;    
    this->start();
}

ZKIdentityCard::~ZKIdentityCard()
{
    Q_D(ZKIdentityCard);
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
    
}

static inline int Utils_getFileSize(const char *path)
{
    int size = 0;
    struct stat buf;
    if (stat(path, &buf) < 0)
    {
        return 0;
    }
    size = buf.st_size;
    return size;
}

//解码照片信息成bmp文件流并转为jpg图片
int writePhoto(unsigned char photo[],int photoLen,char *imageName)
{
	int ret =0;
	unsigned char *imageData = NULL;
	unsigned char *rgbData = NULL;
	int outLen = (102*3+2)*126+54;
	int rgbLen = 102*3*126;
	imageData = (unsigned char*)malloc(outLen);
	rgbData =(unsigned char*)malloc(rgbLen);
	
	if((ret = ZKID_Wlt2bmpBuffer(photo, imageData, outLen)) == 0)
	{

		//将身份证数据转换为纯RGB
		for(int i =0;i<126;i++)
		{
			for(int j=0;j<102*3;j++){
			     rgbData[(125-i)*102*3+j] = imageData[i*102*3+j+54+2*i];
			}
		}
		
		YNH_LJX::RkUtils::RGBtoJPEG(imageName, rgbData,102,126,80); //liwen
		LogV("decode image success ! \n");
		if(imageData !=NULL){
			free(imageData);
		}
		if(rgbData !=NULL)
		    free(rgbData);
	}
	else
	{		
		LogV("decode image fail, ret=%d\n", ret);
	}
	return 0;
}

//公民身份证
int ZKIdentityCardPrivate::read_identfy_info(Tidentify_info info,unsigned char photo[],int photoLen)
{
    IDCardInfo *pIDCardInfo = (IDCardInfo*) YNH_LJX::RkUtils::Utils_Malloc(sizeof(IDCardInfo));
    memset(pIDCardInfo,0,sizeof(IDCardInfo));  

	char buffer[300]={0x0};
	//get idCardNumber
	memset(buffer,0x0,300);

	getIDCardNumber(info,buffer);
	printf("IDCardNumber=%s\n",buffer);

	memcpy(pIDCardInfo->szIDCardNum,buffer,strlen(buffer));

	//get name
	memset(buffer,0x0,300);
	getIDName(info,buffer);
	printf("Name=%s\n",buffer);
    memcpy(pIDCardInfo->szName,buffer,strlen(buffer));

	//get gender
	memset(buffer,0x0,300);
	getIDGender(info,buffer);
	printf("Gender=%s\n",buffer);
    memcpy(pIDCardInfo->szSex,buffer,strlen(buffer));

	//get national
	memset(buffer,0x0,300);
	getIDNational(info,buffer);
    memcpy(pIDCardInfo->szNationality,buffer,strlen(buffer));
	printf("National=%s\n",buffer);

	//get address
	memset(buffer,0x0,300);
	getIDAddress(info,buffer);
    memcpy(pIDCardInfo->szIDAddress,buffer,strlen(buffer));    
	printf("Address=%s\n",buffer);

	//get birthday
	memset(buffer,0x0,300);
	getIDBirthday(info,buffer);
    memcpy(pIDCardInfo->szBirthDate,buffer,strlen(buffer));        
	printf("Birthday=%s\n",buffer);

	//get maker
	memset(buffer,0x0,300);
	getIDMaker(info,buffer);
    memcpy(pIDCardInfo->szIssuingAuthority,buffer,strlen(buffer));         
	printf("Maker=%s\n",buffer);

	//get startData
	memset(buffer,0x0,300);
	getIDStartDate(info,buffer);
    memcpy(pIDCardInfo->szStartDate,buffer,strlen(buffer));      
	printf("StartDate=%s\n",buffer);

	//get endDate
	memset(buffer,0x0,300);
	getIDEndDate(info,buffer);
    memcpy(pIDCardInfo->szEndDate,buffer,strlen(buffer));     
	printf("EndDate=%s\n",buffer);

	char imageName[256];
	memset(imageName,0,sizeof(imageName));
	sprintf(imageName,"/mnt/user/facedb/img/idcard.jpg");
	unlink(imageName);//删除文件
	writePhoto(photo,photoLen,imageName);
    //到这一步才算解析完成，才能拿开身份证，此时打开蜂鸣器提示
    YNH_LJX::Audio::Audio_PlayCustomerPcm("zh", "Buzzer.wav", true);

    emit q_func()->sigIdCardFullInfo(pIDCardInfo->szName, pIDCardInfo->szIDCardNum, pIDCardInfo->szIDAddress, pIDCardInfo->szSex,  
        pIDCardInfo->szNationality, pIDCardInfo->szBirthDate,pIDCardInfo->szIssuingAuthority, pIDCardInfo->szStartDate, 
        pIDCardInfo->szEndDate);

    usleep(1000*100);
	YNH_LJX::RkUtils::Utils_Free(pIDCardInfo);

	return 0;
}

void ZKIdentityCardPrivate::ZKCheckIdCard()
{
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


	/* 读取 sam id ，不需要可以不读取  */
	ret = ZKID_StartFindIDCard(q_func()->deviceIndex, ucIIN, 0);
	if(ret < 0)
	{
		pthread_mutex_unlock(&mIDCardMutex);
		return ;
	}

	ret = ZKID_SelectIDCard(q_func()->deviceIndex, ucSN, 0);
	if(ret < 0)
	{
		pthread_mutex_unlock(&mIDCardMutex);
		return ;
	}

	/*读取卡信息*/
	memset(messageInfo, 0x0, 256);
	memset(photo, 0x0, sizeof(photo));
	ret = ZKID_ReadBaseMsg(q_func()->deviceIndex,messageInfo, &info_len, photo,&photo_len, 0);

	if(ret>=0)
	{
		char CardType[2];
		memset(CardType,0x0,2);
		GetCardType(messageInfo,CardType);
		printf("CardType=%s\n",CardType);

		printf("身份证\n");
		Tidentify_info info;
		memset(&info,0x0,sizeof(Tidentify_info));
		memcpy(&info,messageInfo,256);
		read_identfy_info(info,photo,photo_len);
      
	}

	pthread_mutex_unlock(&mIDCardMutex);
  
}

void ZKIdentityCard::run()
{
    Q_D(ZKIdentityCard);

	while(true)
	{

	   d->ZKCheckIdCard();
   	   usleep(1000*1000);//1000*100

	}

}

