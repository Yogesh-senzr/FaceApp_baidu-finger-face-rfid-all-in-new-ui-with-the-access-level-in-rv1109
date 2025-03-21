#include "ZKIDCard.h"
#include "PCIcore/RkUtils.h"
#include "PCIcore/Audio.h"

#include <QWaitCondition>
#include <QDebug>
#include <QMutex>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/dir.h>
#include"assert.h"
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <errno.h>
//#include "utils/Utils.h"
#include "Utils.h"
//#include "public_if.h"

#include <time.h>
#include <sys/time.h>
#include <csignal>
using namespace std;

#define LogDebug (printf("[%d]<%s>: ", __LINE__, __FILE__), printf)

#define _ZKIDCard_ENABLE_

void ZKIDCard::deleteIDCardInfo(int mdelay)
{
	/*if(mdelay >0 &&isAddCard ==true){
		usleep(1000*mdelay);
	}
	int person_count;
	Person_S* person_addr;
	Ai_GetAllPerson(&person_count, &person_addr);
	if(person_count >0)
		Ai_DeletePerson(&person_addr,person_count);
	isAddCard = false;*/

}

//解码照片信息成bmp文件流并转为jpg图片
int ZKIDCard::writePhoto(unsigned char photo[],int photoLen,char *imageName)
{
	int ret =0;
	unsigned char *imageData = NULL;
	unsigned char *rgbData = NULL;
	int outLen = (102*3+2)*126+54;
	int rgbLen = 102*3*126;
	imageData = (unsigned char*)malloc(outLen);
	rgbData =(unsigned char*)malloc(rgbLen);
//#ifdef _ZKIDCard_ENABLE_
#if 1
	if((ret = ZKID_Wlt2bmpBuffer(photo, imageData, outLen)) == 0)
	{
		//将身份证数据转换为纯RGB
		for(int i =0;i<126;i++)
		{
			for(int j=0;j<102*3;j++){
			     rgbData[(125-i)*102*3+j] = imageData[i*102*3+j+54+2*i];
			}
		}
		printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
		RGBtoJPEG(imageName, rgbData,102,126); //liwen
		LogDebug("decode image success ! \n");
		if(imageData !=NULL){
			free(imageData);
		}
		if(rgbData !=NULL)
		    free(rgbData);
	}
	else
	{
		LogDebug("decode image fail, ret=%d\n", ret);
	}
#endif
	return 0;
}
char* log_Time(void)
{
	struct tm *ptm;
	//struct timeb stTimeb;
	static char szTime[19];
#if 0
	//ftime(&stTimeb);
	ptm = localtime(&stTimeb.time);
	sprintf(szTime, "%02d-%02d %02d:%02d:%02d.%03d", ptm->tm_mon + 1,
			ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec,
			stTimeb.millitm);
#endif 			
	szTime[18] = 0;
	return szTime;
}

//公民身份证
int ZKIDCard::read_identfy_info(Tidentify_info info,unsigned char photo[],int photoLen)
{

	deleteIDCardInfo(0);
#if 1
	//start add person;
#if  0	
	Person_S *pPerson = (Person_S*) Utils_Malloc(sizeof(Person_S));
	memset(pPerson,0,sizeof(Person_S));
#endif 

	char buffer[300]={0x0};
	//get idCardNumber
	memset(buffer,0x0,300);
#ifdef _ZKIDCard_ENABLE_
	getIDCardNumber(info,buffer);
	LogDebug("IDCardNumber=%s\n",buffer);


	//memcpy(pPerson->szIDCardNum,buffer,strlen(buffer));

	//get name
	memset(buffer,0x0,300);
	getIDName(info,buffer);
	LogDebug("Name=%s\n",buffer);
	//memcpy(pPerson->szName,buffer,strlen(buffer));


	//get gender
	memset(buffer,0x0,300);
	getIDGender(info,buffer);
	LogDebug("Gender=%s\n",buffer);
#endif
#if 0
	if(strcmp(buffer,"男")==0){
		memcpy(pPerson->szSex,"male",strlen("male"));
	}else{
		memcpy(pPerson->szSex,"female",strlen("female"));
	}
#endif 
	//get national
	memset(buffer,0x0,300);
	getIDNational(info,buffer);
	LogDebug("National=%s\n",buffer);

	//get address
	memset(buffer,0x0,300);
	getIDAddress(info,buffer);
	LogDebug("Address=%s\n",buffer);


	//get birthday
	memset(buffer,0x0,300);
	getIDBirthday(info,buffer);
	LogDebug("Birthday=%s\n",buffer);


	//get maker
	memset(buffer,0x0,300);
	getIDMaker(info,buffer);
	LogDebug("Maker=%s\n",buffer);


	//get startData
	memset(buffer,0x0,300);
	getIDStartDate(info,buffer);
	LogDebug("StartDate=%s\n",buffer);

	//get endDate
	memset(buffer,0x0,300);
	getIDEndDate(info,buffer);
	LogDebug("EndDate=%s\n",buffer);

	char imageName[256];
	memset(imageName,0,sizeof(imageName));
	sdsssssssssss
	//sprintf(imageName,"/mnt/user/facedb/img/idcard.jpg");
	sprintf(imageName,"/mnt/user/facedb/img/idcard.jpg");
	unlink(imageName);
	writePhoto(photo,photoLen,imageName);

	//memcpy(pPerson->szImage,imageName,sizeof(pPerson->szImage));
	//Ai_SavePersonbyIDCard(pPerson);
	isAddCard = true;



	//Utils_Free(pPerson);
//	unlink(imageName);
#endif 
	return 0;
}


//寻卡-->选卡-->读卡-->解析
int ZKIDCard::read_info_ex(int index)
{
	pthread_mutex_lock(&mIDCardMutex);
	int ret = 0;

#ifdef _ZKIDCard_ENABLE_

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
	ret = ZKID_StartFindIDCard(index, ucIIN, 0);
	if(ret < 0)
	{
		pthread_mutex_unlock(&mIDCardMutex);
		LogDebug("index=%d,find no card,ret=%d\n",index,ret);
		return 0;
	}

	ret = ZKID_SelectIDCard(index, ucSN, 0);
	if(ret < 0)
	{
		pthread_mutex_unlock(&mIDCardMutex);
		LogDebug("index=%d,select card  failed,ret=%d\n",index,ret);
		return 0;
	}

	/*读取卡信息*/
	memset(messageInfo, 0x0, 256);
	memset(photo, 0x0, sizeof(photo));
	ret = ZKID_ReadBaseMsg(index,messageInfo, &info_len, photo,&photo_len, 0);
#if 0
	//finger Message
	ret = ZKID_ReadBaseFPMsg(index,messageInfo,&info_len,photo,&photo_len,FPMsg,FPMsg_len,0);
#endif
	if(ret>=0)
	{
		char CardType[2];
		memset(CardType,0x0,2);
		GetCardType(messageInfo,CardType);
		LogDebug("CardType=%s\n",CardType);

		LogDebug("身份证\n");
		Tidentify_info info;
		memset(&info,0x0,sizeof(Tidentify_info));
		memcpy(&info,messageInfo,256);
		read_identfy_info(info,photo,photo_len);

		LogDebug("身份证\n");
	}

#endif
	pthread_mutex_unlock(&mIDCardMutex);
	return ret;
}


void ZKIDCard::usb_fun(int deviceIndex)
{
#ifdef _ZKIDCard_ENABLE_

	int ret = 0;
	char SAMIDString[100]={0x0};
	LogDebug("deviceIndex=%d !\n",deviceIndex);
	if(deviceIndex<0)
	{
		return ;
	}
	int index = ZKID_OpenPort(deviceIndex);
	if(index < 0)
	{
		LogDebug("open device index%d failed !\n",deviceIndex);
		return ;
	}
	LogDebug("open device index%d Successed !\n",deviceIndex);


	/* 读取设备状态, 检测设备是否可以正常工作     */
	if(ZKID_GetSAMStatus(index, 0) < 0)
	{
        usleep(100*1000);
        if(ZKID_GetSAMStatus(index, 0)<0)
        {
            LogDebug("devce index%d,get sam mode status failed\n",index);
            return ;
        }
	}

	/*读取设备唯一标识符*/
	ZKID_GetSAMIDToStr(index,SAMIDString,0);
	LogDebug("device index%d SAMID =%s\n",index,SAMIDString);
	int count=0;

	while(true)
	{
       read_info_ex(index);
   	   usleep(1000*100);

	}
	ZKID_ClosePort(index);

#endif

	return ;
}

void ZKIDCard::usb_one_devie()
{
 //   ZKID_SetLogType(1,"log.txt");zkidreader.log
 //   ZKID_SetLogLevel(VERBOSE);
#ifdef _ZKIDCard_ENABLE_
	int ret = 0;
	int deviceCount=ZKID_Init();
	LogDebug("deviceCount=%d\n",deviceCount);
	if(deviceCount<=0)
	{
		LogDebug("device Init failed!\n");
		return;
	}
	int deviceIndex=0;
	printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
	usb_fun(deviceIndex);
	ZKID_Free();

#endif
}


void ZKIDCard::IDCardRun()
{
	usb_one_devie();
}


ZKIDCard* ZKIDCard::mThis = NULL;
bool ZKIDCard::isAddCard = false;


bool ZKIDCard::getIDCardState()
{
	return isAddCard;
}

void ZKIDCard::setIDCardState(bool isAdd)
{
	isAddCard = isAdd;
}

ZKIDCard::ZKIDCard()
{
#define ISC_NULL                 0L	
	isAddCard = false;
	pthread_mutex_init(&mIDCardMutex, ISC_NULL);
	IDCardRun();
}

ZKIDCard::~ZKIDCard()
{
	pthread_mutex_destroy(&mIDCardMutex);
}

ZKIDCard* ZKIDCard::getInstance()
{
	if (mThis == NULL)
	{
		mThis = new ZKIDCard;
	}
	return mThis;
}




