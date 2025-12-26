#include "BaiduFaceManager.h"
#include "CBaiduFaceEngine.h"
#include "PCIcore/RkUtils.h"
#include "SharedInclude/GlobalDef.h"
#include "HttpServer/ConnHttpServerThread.h"
#include "interface/bface_types.h"
#include "helper/face_utils.hpp"
#include "helper/image_convert.hpp"
#include "helper/timer/timer.h"
#include "interface/faceid_interface.h"
#include "interface/bd_default_param.h"
#include "baidu_face_sdk.h"

#include "SettingFuncFrms/ManagingPeopleFrms/AddUserFrm.h"
#include "RkNetWork/NetworkControlThread.h"

#include "FaceMainFrm.h"
#include "Helper/myhelper.h"
#include "PCIcore/Audio.h"
#include "Config/ReadConfig.h"
#include "Application/FaceApp.h"
#include <openssl/md5.h>

#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>
#include <fcntl.h>
#include <dirent.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include "MessageHandler/Log.h"

#include <sys/time.h>

#include <QThread>
#include <QMutex>
#include <QReadWriteLock>
#include <QBuffer>
#include <QPixmap>
#include <chrono>
#include <time.h>
#include <stdexcept>
#include <QDir>
#include <QFileInfo>
//#define NSCALE (16)
//#define FACENUM (5)
//#define MAX_THREAD  (15)
//#define FACESCALEVAL (20)

#define DISSMISS_WAIT_FRAME (10)

#define _PROCESS_NORMAL_ (0)
#define _PROCESS_SEARCH_ (1)
#define _PROCESS_REGIST_ (2)

#define MAX_FACES (10)

#define MOK 0


//算法默认阈值
//#define BD_TRACKING_RGB_CONF (0.5)
//#define MASK_SCORE (0.5)

//using namespace std;
using namespace bface;

static float IOU(const bface::Rectf_t& b1, const bface::Rectf_t& b2)
{
	float w = std::min(b1.left + b1.width, b2.left + b2.width) - std::max(b1.left, b2.left);
	float h = std::min(b1.top + b1.height, b2.top + b2.height) - std::max(b1.top, b2.top);

	if (w <= 0 || h <= 0)
		return 0;
	return (w * h) / ((b1.height * b1.width) + (b2.height * b2.width) - (w * h));
}

static int match_bbox_iou(std::vector<bface::BoundingBox_t> bbox_list, bface::BoundingBox_t bbox_target, float iou_thr)
{
	for (int i = 0; i < bbox_list.size(); i++)
	{
		float value = IOU(bbox_list[i].rect, bbox_target.rect);
		if (value > iou_thr)
		{
			return i;
		}
	}
	// no matched
	return -1;
}

static void writeFaceOfflineAppId()
{
	if(!access("/param/license.key",F_OK))
	{
		LogD("%s %s[%d] already baidu sdk_init  !!!! \n",__FILE__,__FUNCTION__,__LINE__);
        myHelper::Utils_ExecCmd("cp -rf /param/license.key  /isc/models_encrypted/");
    }
}

class BaiduFaceManagerPrivate
{
    Q_DECLARE_PUBLIC(BaiduFaceManager)
public:
    BaiduFaceManagerPrivate(BaiduFaceManager *dd);
private:
    void InitFaceEngine();
private:
    int InitFtEngine();
    int InitFlEngine();
    int InitFrEngine();
    int InitFaEngine();
private://处理摄像头数据
    void DealCamearDataToASVLOFFSCREEN(const int &, unsigned long nYuvVirAddr, int width, int height, int rotation);
    //找出最大人脸索引
    QList<CORE_FACE_RECT_S> FindMaxFaceDetect(const int &FaceNum, int &MatchCoreFaceIndex, int &MatchFaceRectSize, int &nCurTrackID,  CORE_FACE_S new_update_face_info[]);
    //检测口罩
    void CheckMaskDetecct();
    //检测活体
    void CheckLivenessDetect();
    //检测角度
    void CheckDetectFace3DAngle(const int &);
    //检测图片质量
    float CheckFaceQuality();
    //提取特征码
    void FaceFeatureExtract();
    void saveFaceImgToDisk(const QString &imgPath, const CORE_FACE_S &FaceTask);
private:
    bool CheckRange(const CORE_FACE_S &face);
    void DealFaceMove(const int &FaceNum, CORE_FACE_S *new_update_face_info[]);
private:
    void* m_pFtEngine;
    void* m_pFlEngine;
    void* m_pFrEngine;
    //for register
    void* m_pFaEngine;
    //pthread_mutex_t m_stCoreFacesLock;
    pthread_mutex_t m_stCoreFacesLock = PTHREAD_MUTEX_INITIALIZER;
    Face_S *m_stFace = ISC_NULL;
    bool m_bAiInitFinished = false;
    //pthread_mutex_t m_stAiMutex;
    pthread_mutex_t m_stAiMutex = PTHREAD_MUTEX_INITIALIZER;
    bface::Image_t m_stRGBImg;
    bface::Image_t m_stIRImg;
private:
    int mInitEngineSuccess;//算法初始化状态,成功与否
    int mDismissFaceCount;//记录多少侦图像未出现人图
    int mIdentityIdentifycm;//识别距离模式
private:
    ASVLOFFSCREEN m_FaceRgbOffscreen;//保存Rgb图片信息
    ASVLOFFSCREEN m_FaceIrOffscreen;//保存Ir图片信息

    LPBAIDU_MultiFaceInfo m_pMultiFaceInfo;//保存当前Rgb的多人脸
    CORE_FACE_S mMatchCoreFace;//当前最大的人脸数据
private:
    mutable	QMutex sync;
private:
    int mLiveMode;//当前检测模式
private:
    bool mRegFaceState;//注册状态
    bool mIdentifyState;//识别状态
	bool minit;
private:
    BaiduFaceManager *const q_ptr;
};

BaiduFaceManagerPrivate::BaiduFaceManagerPrivate(BaiduFaceManager *dd) //
    : q_ptr(dd) //
    , m_pFtEngine(new CBaiduFaceEngine) // ASF_FACE_DETECT
    , m_pFlEngine(new CBaiduFaceEngine) //ASF_LIVENESS ,ASF_IR_LIVENESS
    , m_pFrEngine(NULL) //ASF_FACERECOGNITION | ASF_MASKDETECT | ASF_FACE3DANGLE
    , m_pFaEngine(new CBaiduFaceEngine) //for register
    , mInitEngineSuccess(-1) //
    , mLiveMode(2) //
    , mDismissFaceCount(0) //
    , mRegFaceState(false) //
    , mIdentifyState(false) //
    , mIdentityIdentifycm(2) //
	,minit(false)
{

    //多人脸数据 //可能导致程序跑飞,待确认
 
   //多人脸数据

    memset(&m_FaceRgbOffscreen, 0, sizeof(ASVLOFFSCREEN));
    memset(&m_FaceIrOffscreen, 0, sizeof(ASVLOFFSCREEN));
    memset(&mMatchCoreFace, 0, sizeof(CORE_FACE_S));


	minit = true;
}

BaiduFaceManager::BaiduFaceManager(QObject *parent)
    : BaseFaceManager(parent)
    , d_ptr(new BaiduFaceManagerPrivate(this))
{
}

BaiduFaceManager::~BaiduFaceManager()
{

}

static std::string getTime()
{
	time_t timep;
	time(&timep);
	char tmp[64];
	strftime(tmp, sizeof(tmp), "%Y%m%d", localtime(&timep));
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

static bool checkBaiduLicenseOnlineFlag()
{
	/***
	 * 当U盘里面有 license.key文件且私有分区没有时，表示设备没有激活过，且需要U盘在线激活。
	 * 			将U盘里面的license.key 拷贝到私有分区，用户U盘激活时同步录入APPID
	 */
	if(!access("/udisk/license.key",F_OK) && access("/param/license.key",F_OK))
	{
		myHelper::Utils_ExecCmd("cp -rf /udisk/license.key  /param/license.key");
	    myHelper::Utils_ExecCmd("sync");
	}
	if(!access("/udisk/ynh360_baidu_license_online.txt",F_OK))
	{
		std::string dataTime  = getTime();
		QString md5_0 = myHelper::ReadFile("/udisk/ynh360_baidu_license_online.txt");
		QString md5_1 = QString::fromStdString(md5sum(dataTime));
		LogD("%s %s[%d] md5_0 %s \n",__FILE__,__FUNCTION__,__LINE__,md5_0.toStdString().c_str());
		LogD("%s %s[%d] md5_1 %s \n",__FILE__,__FUNCTION__,__LINE__,md5_1.toStdString().c_str());
		LogD("%s %s[%d] md5_0 is equal md5_1 %d \n",__FILE__,__FUNCTION__,__LINE__,(!md5_1.compare(md5_0)));
		return !md5_1.compare(md5_0);
	}
	return false;
}


bool BaiduFaceManager::hasPerson()
{
	Q_D(BaiduFaceManager);
	float faceQuality = d->CheckFaceQuality();
	printf("%s %s[%d]  faceQuality  %f \n",__FILE__,__FUNCTION__,__LINE__,faceQuality);
	return faceQuality > 0.3;
}
void BaiduFaceManager::setDeskSize(const int &w, const int &h)
{
    Q_D(BaiduFaceManager);

    mDeskWidth = w;
    mDeskHeight = h;

    d->m_FaceRgbOffscreen.ppu8Plane[0] = (unsigned char*) malloc(w * h * 3 / 2);
    d->mMatchCoreFace.FaceOffscreen.ppu8Plane[0] = (unsigned char*) malloc(w * h * 3 / 2);

    if (d->m_FaceRgbOffscreen.ppu8Plane[0] == Q_NULLPTR)
    {        
        return;
    }


    d->m_FaceRgbOffscreen.ppu8Plane[1] = d->m_FaceRgbOffscreen.ppu8Plane[0] + w * h;
    d->m_FaceRgbOffscreen.i32Width = w;
    d->m_FaceRgbOffscreen.i32Height = h;
    d->m_FaceRgbOffscreen.pi32Pitch[0] = w;
    d->m_FaceRgbOffscreen.pi32Pitch[1] = w;
    d->m_FaceRgbOffscreen.u32PixelArrayFormat = ASVL_PAF_NV12;
    memset(d->m_FaceRgbOffscreen.ppu8Plane[0], 0, (d->m_FaceRgbOffscreen.pi32Pitch[0] * d->m_FaceRgbOffscreen.i32Height * 3 / 2));

    d->mMatchCoreFace.FaceOffscreen.ppu8Plane[1] = d->mMatchCoreFace.FaceOffscreen.ppu8Plane[0] + w * h;
    d->mMatchCoreFace.FaceOffscreen.i32Width = w;
    d->mMatchCoreFace.FaceOffscreen.i32Height = h;
    d->mMatchCoreFace.FaceOffscreen.pi32Pitch[0] = w;
    d->mMatchCoreFace.FaceOffscreen.pi32Pitch[1] = w;
    d->mMatchCoreFace.FaceOffscreen.u32PixelArrayFormat = ASVL_PAF_NV12;
    memset(d->mMatchCoreFace.FaceOffscreen.ppu8Plane[0], 0, (d->mMatchCoreFace.FaceOffscreen.pi32Pitch[0] *d->mMatchCoreFace.FaceOffscreen.i32Height * 3 / 2));

    d->m_FaceIrOffscreen.ppu8Plane[0] = (unsigned char*) malloc(w * h * 3 / 2);
    if (d->m_FaceIrOffscreen.ppu8Plane[0] == Q_NULLPTR)
    {
        return;
    }

    d->m_FaceIrOffscreen.ppu8Plane[1] = d->m_FaceIrOffscreen.ppu8Plane[0] + w * h;
    d->m_FaceIrOffscreen.i32Width = w;
    d->m_FaceIrOffscreen.i32Height = h;
    d->m_FaceIrOffscreen.pi32Pitch[0] = w;
    d->m_FaceIrOffscreen.pi32Pitch[1] = w;
    d->m_FaceIrOffscreen.u32PixelArrayFormat = ASVL_PAF_NV12;
    memset(d->m_FaceIrOffscreen.ppu8Plane[0], 0, (d->m_FaceIrOffscreen.pi32Pitch[0] * d->m_FaceIrOffscreen.i32Height * 3 / 2));
}

void BaiduFaceManager::setVivoDetection(const bool &value)
{
    mVivoDetection = value;
}

void BaiduFaceManager::setRegFaceState(const bool &state)
{
    Q_D(BaiduFaceManager);
    d->mRegFaceState = state;
    if (state)    
    {
        QList<CORE_FACE_RECT_S> tfacelist1;
        emit sigDrawFaceRect(tfacelist1); //if(!d->mRegFaceState)         
    }
    emit sigDisMissMessage(false);
}

bool BaiduFaceManager::getRegFaceState()
{
    Q_D(BaiduFaceManager);
    return d->mRegFaceState;
}

/*void BaiduFaceManager::setIdentifyState(const bool &state)
{
    Q_D(BaiduFaceManager);
    if(state != d->mIdentifyState)d->mIdentifyState = state;
}*/

void BaiduFaceManager::setLivenessThreshold(const float &rgbThreshold, const float &irThreshold)
{
    Q_D(BaiduFaceManager);
    if((d->mInitEngineSuccess != 1))return;
    ((CBaiduFaceEngine *)d->m_pFlEngine)->setLivenessThreshold(rgbThreshold, irThreshold);
}

void BaiduFaceManager::setIdentityIdentifycm(const int &value)
{
    Q_D(BaiduFaceManager);
    d->mIdentityIdentifycm = value;
}

static pthread_t reportDeviceThread = NULL;
static pthread_mutex_t reportDeviceThreadMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t reportDeviceThreadCond = PTHREAD_COND_INITIALIZER;
static void * reportDeviceThreadFunc(void *data)
{
    bool isReportDeviceInfo = false;
    while(!isReportDeviceInfo){
        pthread_mutex_lock(&reportDeviceThreadMutex);
        pthread_cond_wait(&reportDeviceThreadCond,&reportDeviceThreadMutex);
        pthread_mutex_unlock(&reportDeviceThreadMutex);

        isReportDeviceInfo = ConnHttpServerThread::GetInstance()->reportDeviceInfo();

        LogD("%s %s[%d] reportDeviceInfo  isReportDeviceInfo %d ...\n", __FILE__, __FUNCTION__, __LINE__,isReportDeviceInfo);
        if(checkBaiduLicenseOnlineFlag()){
            LogD("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
            YNH_LJX::Audio::Audio_PlayCustomerPcm("zh","device_reporting_info.wav",true);
        }
        if(isReportDeviceInfo){
            if(checkBaiduLicenseOnlineFlag() && !access("/param/license.ini",F_OK)&& !access("/param/license.key",F_OK)){
                while(1)
                {
                    myHelper::Utils_ExecCmd("sync");
                    LogD("%s %s[%d] algo_actived ...\n", __FILE__, __FUNCTION__, __LINE__);
                    YNH_LJX::Audio::Audio_PlayCustomerPcm("zh","algo_actived.wav",true);
                    sleep(1);
                }
            }
        } 
    }
    *(bool *)data = isReportDeviceInfo;
    pthread_exit(NULL);
}

bool BaiduFaceManager::getAlgoFaceInitState() const
{
    static time_t last_check = 0;
    time_t current_time = time(nullptr);
    
    // Log engine state every 60 seconds
    if (current_time - last_check > 60) {
        LogD("%s %s[%d] === ENGINE HEALTH CHECK === State: %d, Time: %ld\n",
            __FILE__, __FUNCTION__, __LINE__, d_func()->mInitEngineSuccess, current_time);
        last_check = current_time;
    }
    
    bool result = d_func()->mInitEngineSuccess == 1;
    
    if (result) {
        static bool first_success = true;
        if (first_success) {
            LogD("%s %s[%d] === ENGINE SUCCESSFULLY INITIALIZED ===\n", __FILE__, __FUNCTION__, __LINE__);
            first_success = false;
        }
        
        static bool isReportDeviceInfo = false;
        if (isReportDeviceInfo == false) {
            if (!reportDeviceThread) {
                LogD("%s %s[%d] Starting device report thread...\n", __FILE__, __FUNCTION__, __LINE__);
                pthread_create(&reportDeviceThread, NULL, reportDeviceThreadFunc, &isReportDeviceInfo);
                pthread_detach(reportDeviceThread);
            }
            pthread_cond_signal(&reportDeviceThreadCond);
        }
    }
    
    return result;
}

int BaiduColorSpaceConvert(MInt32 width, MInt32 height, MInt32 format, MUInt8 *imgData, ASVLOFFSCREEN &offscreen)
{
    offscreen.u32PixelArrayFormat = (unsigned int)format;
    offscreen.i32Width = width;
    offscreen.i32Height = height;

    switch (offscreen.u32PixelArrayFormat)
    {
    case ASVL_PAF_RGB24_B8G8R8:
        offscreen.pi32Pitch[0] = offscreen.i32Width * 3;
        offscreen.ppu8Plane[0] = imgData;
        break;
    case ASVL_PAF_I420:
        offscreen.pi32Pitch[0] = width;
        offscreen.pi32Pitch[1] = width >> 1;
        offscreen.pi32Pitch[2] = width >> 1;
        offscreen.ppu8Plane[0] = imgData;
        offscreen.ppu8Plane[1] = offscreen.ppu8Plane[0] + offscreen.i32Height*offscreen.i32Width;
        offscreen.ppu8Plane[2] = offscreen.ppu8Plane[0] + offscreen.i32Height*offscreen.i32Width * 5 / 4;
        break;
    case ASVL_PAF_NV12:
    case ASVL_PAF_NV21:
        offscreen.pi32Pitch[0] = offscreen.i32Width;
        offscreen.pi32Pitch[1] = offscreen.pi32Pitch[0];
        offscreen.ppu8Plane[0] = imgData;
        offscreen.ppu8Plane[1] = offscreen.ppu8Plane[0] + offscreen.pi32Pitch[0] * offscreen.i32Height;
        break;
    case ASVL_PAF_YUYV:
    case ASVL_PAF_DEPTH_U16:
        offscreen.pi32Pitch[0] = offscreen.i32Width * 2;
        offscreen.ppu8Plane[0] = imgData;
        break;
    case ASVL_PAF_GRAY:
        offscreen.pi32Pitch[0] = offscreen.i32Width;
        offscreen.ppu8Plane[0] = imgData;
        break;
    default:
        return 0;
    }
    return 1;

}
float g_feature_to_search[128] = { 1.4175, 0.6462, 2.6052, 2.9411, -0.3792, 0.057, 0.6174, 0.0966, -2.4033, -4.0003, 1.603, 2.6598, 1.0838, 2.7604,
		-3.3849, 1.872, 0.8116, -1.5772, -1.7824, 2.1362, 3.6447, -1.8351, 1.9779, -1.9968, 1.1021, 2.1265, 1.1005, -1.1884, -2.9931, 1.6831,
		1.2894, 0.1552, 1.2352, -0.9619, 0.1585, -3.3196, -1.1455, 1.5594, 0.0425, 1.3663, -0.835, 0.9663, -1.488, 1.0022, 1.539, -1.4767, -0.157,
		2.884, 1.0058, 2.348, 0.9686, -3.434, -0.1137, -0.4041, -1.9656, 1.7024, -1.5491, -4.3192, -1.3501, 1.1709, 1.7482, -1.749, -3.5904,
		-0.4242, -1.3397, -1.8743, 4.932, 1.031, 1.7817, -2.0246, 5.0514, 0.1119, 2.2664, -2.8964, -2.1067, 0.0397, 2.4965, 1.8584, 0.0841, -0.3909,
		-0.6102, 2.6941, 2.9927, 3.6387, 0.7072, 2.7035, 1.966, -1.0554, -0.2446, -1.3494, -1.7108, -1.6142, -0.794, 2.6849, 1.7834, -1.0144,
		-0.3481, 3.0564, 0.1742, 0.3836, 1.3326, 3.9182, 3.2617, 3.7032, 1.397, -2.1992, 1.3779, -0.7733, -1.5366, -0.8994, -1.7289, 2.8974, 1.5557,
		-0.7201, 2.9041, -1.1284, 5.6848, -1.5342, 0.1227, 0.2792, 1.1647, -1.9223, -0.962, 0.45, -0.0908, 0.1528, 0.8212, 0.3575 };

void BaiduFaceManagerPrivate::InitFaceEngine()
{
    QMutexLocker locker(&sync);

	//pthread_mutex_init(&m_stCoreFacesLock, ISC_NULL);
	m_stFace = (Face_S*) malloc(sizeof(Face_S));
	memset(m_stFace, 0, sizeof(Face_S));
	m_stFace->nTrackID = -1;

    char szVersionInfo[32] = { 0 };
    bface::BFACE_STATUS status = bface::BFACE_SUCCESS;

	myHelper::Utils_ExecCmd("chmod 777 /isc/bin/get_device_id");
	myHelper::Utils_ExecCmd("/isc/bin/get_device_id > /param/baidu_device_id.txt");
	YNH_LJX::RkUtils::Utils_ExecCmd("sync");

	bface::bface_get_version(szVersionInfo, sizeof(szVersionInfo));

	/***********算法激活前，激活码判断，首先判断是否有离线激活****************/
	/*********** 判断是否离线激活过****************/
	if(!access("/param/license.key",F_OK))  //存在离线激活的license.key
	{
        myHelper::Utils_ExecCmd("cp -rf /param/license.key  /isc/models_encrypted/");
    }else //不存在 ，则未离线激活过
    {
    	/*********** 判断是否在线激活过****************/
    	if(!access("/param/license.ini",F_OK)) //存在在线激活的license.ini ，表示板子之前在线激活过
    	{
    		writeFaceOfflineAppId();
    	}else //不存在在线激活的license.ini ,可能是空板
    	{
    		if(checkBaiduLicenseOnlineFlag())
    		{
    			writeFaceOfflineAppId();
    		}
    	}
    }
	
	if(!access("/param/license.ini",F_OK)) //存在离线激活的license.ini
	{
		myHelper::Utils_ExecCmd("cp -rf /param/license.ini  /isc/models_encrypted/");
	}
	/***********算法激活前，激活码判断 完成****************/
   
    int Mode = ReadConfig::GetInstance()->getNetwork_Manager_Mode();
    char szMac[60]={0};
	
	// /mnt/private1/license.ini
    if (Mode==1 || Mode==2)
    {
		FILE *pFile = ISC_NULL;
        
		//if(!access("/udisk/license_offline",F_OK))
			
		if(!access("/param/license.ini",F_OK))
		  NetworkControlThread::GetInstance()->setNetworkType(1);	
		sprintf(szMac,"ifconfig eth0 hw ether %s",myHelper::GetNetworkMac().toStdString().c_str());
		//myHelper::Utils_ExecCmd("ifconfig eth0 down;");
		myHelper::Utils_ExecCmd(szMac);
		//myHelper::Utils_ExecCmd("ifconfig eth0 up;");		

    }
    status = bface::bd_sdk_init("/isc/models_encrypted", false);//true,false
    myHelper::Utils_ExecCmd("rm -rf /isc/models_encrypted/license.key");
    myHelper::Utils_ExecCmd("sync");

    //LogD("%s %s[%d] bd_sdk_init status :%d.,Mode=%d\n", __FILE__, __FUNCTION__, __LINE__, status,Mode);
	if (bface::BFACE_SUCCESS != status)
	{
		return ;
	}
	
	status = bface::bface_init();
	//LogD("%s %s[%d] bface_init status :%d.\n", __FILE__, __FUNCTION__, __LINE__, status);
	if (bface::BFACE_SUCCESS != status)
	{
		LogE("%s %s[%d] bface_init() failed.\n", __FILE__, __FUNCTION__, __LINE__);
		return ;
	}

	FeatureId_t new_feature;
	std::vector<FeatureId_t> feature_lib;
	new_feature.feature_id = -1;
	std::vector<float> face_feature(g_feature_to_search, g_feature_to_search + sizeof(g_feature_to_search) / sizeof(float));
	new_feature.feature.assign(face_feature.begin(), face_feature.end());
	feature_lib.push_back(new_feature);
	status = bface_feature_lib_init(feature_lib);	   


    this->mInitEngineSuccess = 1;
    if(access("/param/license.ini",F_OK))
    {
    	myHelper::Utils_ExecCmd("cp -rf /isc/models_encrypted/license.ini  /param/");
    }
	
    sync.unlock();
    
    pthread_mutex_lock(&m_stAiMutex);
	m_bAiInitFinished = true;	
	pthread_mutex_unlock(&m_stAiMutex);
}

int BaiduFaceManagerPrivate::InitFtEngine()
{
}

int BaiduFaceManagerPrivate::InitFlEngine()
{
    int ret = 0;//-1;
 
    return ret;
}

int BaiduFaceManagerPrivate::InitFrEngine()
{ 
    return -1;
}

int BaiduFaceManagerPrivate::InitFaEngine()
{
    return -1;
}

bool BaiduFaceManagerPrivate::CheckRange(const CORE_FACE_S &face)
{
    int nMaxOnePersonFaceSize = 1;
    int nFaceRectSize = face.stFaceRect.nHeight * face.stFaceRect.nWidth;
    int nGuessRecognitionDistance = 0;

    if(q_func()->mDeskWidth * q_func()->mDeskHeight < 800 * 1280)
    {
    	nMaxOnePersonFaceSize = 320000;
    	nGuessRecognitionDistance = (nMaxOnePersonFaceSize/nFaceRectSize) * 5;
    }else
    {
        if (this->mIdentityIdentifycm==0 || this->mIdentityIdentifycm==1)        
        {
    	//nMaxOnePersonFaceSize = 180000; 此段为实际调整值
        nMaxOnePersonFaceSize = 180000*4;//*6;*10 ;//180000/15; //14
    	nGuessRecognitionDistance = (nMaxOnePersonFaceSize/nFaceRectSize)*10;
        }
        if (this->mIdentityIdentifycm==2)
        {


            nMaxOnePersonFaceSize = 90000*3;//*4,*6;*10 ;//180000/15; //14
            nGuessRecognitionDistance = (nMaxOnePersonFaceSize/nFaceRectSize)*10; 
       printf(">>%s,%s,%d,nGuessRecognitionDistance=%ld,nMaxOnePersonFaceSize=%ld,nFaceRectSize=%ld\n"
         ,__FILE__,__func__,__LINE__,nGuessRecognitionDistance,nMaxOnePersonFaceSize,nFaceRectSize);                       
        }
    }

    switch(this->mIdentityIdentifycm)
    {
    case 0:
    {
        //0--50
        if(nGuessRecognitionDistance>50)return false;
    }break;
    case 1:
    {
        if(q_func()->mDeskWidth * q_func()->mDeskHeight < 800 * 1280)
        {
        	if(nGuessRecognitionDistance>150)return false;
        }else
        {
            //0--100
        	if(nGuessRecognitionDistance>100 )return false;
        }
    }break;
    case 2:
    {
        printf(">>%s,%s,%d,nGuessRecognitionDistance=%d\n",__FILE__,__func__,__LINE__,nGuessRecognitionDistance);
        if(q_func()->mDeskWidth * q_func()->mDeskHeight < 800 * 1280)
        {
        	if(nGuessRecognitionDistance>300)return false;
        }else
        {
            //0--150 
        	if(nGuessRecognitionDistance>150 )return false;
        }
    }
        break;
    }
    return true;
}

bool BaiduFaceManager::getLastDetectedFaceRect(QRect &faceRect)
{
    Q_D(BaiduFaceManager);
    
    if (m_hasFaceRect) {
        faceRect = m_lastFaceRect;
        return true;
    }
    return false;
}

bool BaiduFaceManager::cropAndSaveFaceImage(const QString &employeeId, const QString &sourceImagePath)
{
    Q_D(BaiduFaceManager);
    
    LogD("%s %s[%d] === Starting cropAndSaveFaceImage ===\n", __FILE__, __FUNCTION__, __LINE__);
    LogD("%s %s[%d] Employee ID: %s\n", __FILE__, __FUNCTION__, __LINE__, employeeId.toStdString().c_str());
    LogD("%s %s[%d] Source image path: %s\n", __FILE__, __FUNCTION__, __LINE__, sourceImagePath.toStdString().c_str());
    
    if (employeeId.isEmpty() || sourceImagePath.isEmpty()) {
        LogE("%s %s[%d] Invalid parameters\n", __FILE__, __FUNCTION__, __LINE__);
        return false;
    }
    
    // Check if source file exists
    if (access(sourceImagePath.toStdString().c_str(), F_OK)) {
        LogE("%s %s[%d] Source file does not exist: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, sourceImagePath.toStdString().c_str());
        return false;
    }
    
    // Load the image using OpenCV (similar to your RegistPerson function)
    cv::Mat img_mat = cv::imread(sourceImagePath.toStdString());
    if (img_mat.empty()) {
        LogE("%s %s[%d] Failed to load image: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, sourceImagePath.toStdString().c_str());
        return false;
    }
    
    // Set up image for face detection
    Image_t img2;
    img2.pixel_format = BFACE_INTERLEAVE_U8C3_BGR;
    img2.image_type = BFACE_IMAGE_RGB;
    img2.width = img_mat.cols;
    img2.height = img_mat.rows;
    img2.vir_addr[0] = (bface_pointer_type)img_mat.data;
    
    // Detect face to get bounding box
    std::vector<BoundingBox_t> bbox_list;
    bface::BFACE_STATUS status = bface_detect_face(img2, &bbox_list);
    
    if (status != BFACE_SUCCESS || bbox_list.size() == 0) {
        LogE("%s %s[%d] Face detection failed or no face found\n", __FILE__, __FUNCTION__, __LINE__);
        return false;
    }
    
    // Get the face rectangle from the first detected face
    BoundingBox_t face_bbox = bbox_list[0];
    
    // Convert to QRect and store for later use
    m_lastFaceRect = QRect(
        face_bbox.rect.left,
        face_bbox.rect.top,
        face_bbox.rect.width,
        face_bbox.rect.height
    );
    m_hasFaceRect = true;
    
    LogD("%s %s[%d] Detected face at: (%d,%d,%d,%d)\n", 
         __FILE__, __FUNCTION__, __LINE__, 
         m_lastFaceRect.x(), m_lastFaceRect.y(), 
         m_lastFaceRect.width(), m_lastFaceRect.height());
    
    // Add padding around face (15% on each side for better framing)
    int padding_x = m_lastFaceRect.width() * 0.15;
    int padding_y = m_lastFaceRect.height() * 0.15;
    
    QRect paddedRect(
        qMax(0, m_lastFaceRect.x() - padding_x),
        qMax(0, m_lastFaceRect.y() - padding_y),
        qMin(img_mat.cols - m_lastFaceRect.x() + padding_x, m_lastFaceRect.width() + 2 * padding_x),
        qMin(img_mat.rows - m_lastFaceRect.y() + padding_y, m_lastFaceRect.height() + 2 * padding_y)
    );
    
    // Ensure crop rectangle is within image bounds
    paddedRect = paddedRect.intersected(QRect(0, 0, img_mat.cols, img_mat.rows));
    
    // Crop the face region using OpenCV
    cv::Rect crop_rect(paddedRect.x(), paddedRect.y(), paddedRect.width(), paddedRect.height());
    cv::Mat cropped_image = img_mat(crop_rect);
    
    // Create output directory if it doesn't exist
    QString dirPath = "/mnt/user/reg_face_image";
    QDir dir;
    if (!dir.exists(dirPath)) {
        if (!dir.mkpath(dirPath)) {
            LogE("%s %s[%d] Failed to create directory: %s\n", 
                 __FILE__, __FUNCTION__, __LINE__, dirPath.toStdString().c_str());
            return false;
        }
        LogD("%s %s[%d] Created directory: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, dirPath.toStdString().c_str());
    }
    
    // Save the cropped image
    QString outputPath = QString("/mnt/user/reg_face_image/%1.jpg").arg(employeeId);
    
    // Set JPEG quality to 95% for high quality
    std::vector<int> compression_params;
    compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
    compression_params.push_back(95);
    
    bool saveResult = cv::imwrite(outputPath.toStdString(), cropped_image, compression_params);
    
    if (saveResult) {
        LogD("%s %s[%d] Successfully saved cropped face image: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, outputPath.toStdString().c_str());
        
        // Sync to ensure data is written to disk
        system("sync");
        
        // Verify file was created
        QFileInfo fileInfo(outputPath);
        if (fileInfo.exists()) {
            LogD("%s %s[%d] File verification successful: %s (size: %lld bytes)\n", 
                 __FILE__, __FUNCTION__, __LINE__, outputPath.toStdString().c_str(), fileInfo.size());
            return true;
        } else {
            LogE("%s %s[%d] File verification failed: %s\n", 
                 __FILE__, __FUNCTION__, __LINE__, outputPath.toStdString().c_str());
            return false;
        }
    } else {
        LogE("%s %s[%d] Failed to save cropped image: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, outputPath.toStdString().c_str());
        return false;
    }
}

bool BaiduFaceManager::cropCurrentFaceAndSave(const QString &employeeId)
{
    Q_D(BaiduFaceManager);
    
    LogD("%s %s[%d] === Starting cropCurrentFaceAndSave ===\n", __FILE__, __FUNCTION__, __LINE__);
    LogD("%s %s[%d] Employee ID: %s\n", __FILE__, __FUNCTION__, __LINE__, employeeId.toStdString().c_str());
    
    if (employeeId.isEmpty()) {
        LogE("%s %s[%d] Employee ID is empty\n", __FILE__, __FUNCTION__, __LINE__);
        return false;
    }
    
    // Check if we have current face data
    if (d->mMatchCoreFace.stFaceRect.nWidth <= 0 || d->mMatchCoreFace.stFaceRect.nHeight <= 0) {
        LogE("%s %s[%d] No valid face data available\n", __FILE__, __FUNCTION__, __LINE__);
        return false;
    }
    
    // Use the existing saveFaceImgToDisk function but modify the path
    QString outputPath = QString("/mnt/user/reg_face_image/%1.jpg").arg(employeeId);
    
    // Create directory if it doesn't exist
    QString dirPath = "/mnt/user/reg_face_image";
    QDir dir;
    if (!dir.exists(dirPath)) {
        if (!dir.mkpath(dirPath)) {
            LogE("%s %s[%d] Failed to create directory: %s\n", 
                 __FILE__, __FUNCTION__, __LINE__, dirPath.toStdString().c_str());
            return false;
        }
    }
    
    // Use the existing cropping logic from saveFaceImgToDisk
    unsigned char* pTmpBuf = (unsigned char*) YNH_LJX::RkUtils::Utils_Malloc(mDeskWidth * mDeskHeight * 3 / 2);
    unsigned char* pTmpBuf_1 = (unsigned char*) YNH_LJX::RkUtils::Utils_Malloc(mDeskWidth * mDeskHeight * 3 / 2);
    
    if (pTmpBuf == NULL || pTmpBuf_1 == NULL) {
        LogE("%s %s[%d] Memory allocation failed\n", __FILE__, __FUNCTION__, __LINE__);
        if (pTmpBuf) free(pTmpBuf);
        if (pTmpBuf_1) free(pTmpBuf_1);
        return false;
    }
    
    int x, y, width, height;
    x = d->mMatchCoreFace.stFaceRect.nX;
    y = d->mMatchCoreFace.stFaceRect.nY;
    width = d->mMatchCoreFace.stFaceRect.nWidth;
    height = d->mMatchCoreFace.stFaceRect.nHeight;
    
    // Add padding around face (20% for better framing)
    int padding_x = width * 0.2;
    int padding_y = height * 0.2;
    
    x = qMax(0, x - padding_x);
    y = qMax(0, y - padding_y);
    width = qMin(mDeskWidth - x, width + 2 * padding_x);
    height = qMin(mDeskHeight - y, height + 2 * padding_y);
    
    // Ensure even dimensions for YUV processing
    x = x + (x % 2 ? -1 : 0);
    y = y + (y % 2 ? -1 : 0);
    if ((width % 4) != 0)
        width = 4 * ((width / 4));
    if ((height % 4) != 0)
        height = 4 * ((height / 4));
    
    // Boundary checks
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if ((x + width) > mDeskWidth) width = (mDeskWidth - x);
    if ((y + height) > mDeskHeight) height = (mDeskHeight - y);
    
    LogD("%s %s[%d] Cropping region: (%d,%d,%d,%d)\n", 
         __FILE__, __FUNCTION__, __LINE__, x, y, width, height);
    
    // Crop the image
    int result = YNH_LJX::RkUtils::NV21CutImage(
        (unsigned char*)d->mMatchCoreFace.FaceOffscreen.ppu8Plane[0], 
        mDeskWidth, mDeskHeight, 
        pTmpBuf, width * height * 3 / 2, 
        x, y, x + width, y + height
    );
    
    if (result < 0) {
        LogE("%s %s[%d] Image cropping failed\n", __FILE__, __FUNCTION__, __LINE__);
        YNH_LJX::RkUtils::Utils_Free(pTmpBuf);
        YNH_LJX::RkUtils::Utils_Free(pTmpBuf_1);
        return false;
    }
    
    // Convert color format
    YNH_LJX::RkUtils::Utils_YVU420SPConvertToYUV420P((unsigned long)pTmpBuf, (unsigned long)pTmpBuf_1, width, height);
    
    // Save as JPEG with high quality
    YNH_LJX::RkUtils::YUVtoJPEG(outputPath.toLatin1(), (unsigned char*)pTmpBuf_1, width, height, 95);
    
    // Cleanup
    YNH_LJX::RkUtils::Utils_Free(pTmpBuf);
    YNH_LJX::RkUtils::Utils_Free(pTmpBuf_1);
    
    // Sync to disk
    system("sync");
    
    // Verify file was created
    QFileInfo fileInfo(outputPath);
    if (fileInfo.exists()) {
        LogD("%s %s[%d] Successfully saved cropped current face: %s (size: %lld bytes)\n", 
             __FILE__, __FUNCTION__, __LINE__, outputPath.toStdString().c_str(), fileInfo.size());
        return true;
    } else {
        LogE("%s %s[%d] File verification failed: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, outputPath.toStdString().c_str());
        return false;
    }
}

bool BaiduFaceManager::extractFeaturesFromCroppedImage(const QString &employeeId, QByteArray &faceFeature, double &quality)
{
    LogD("%s %s[%d] === Starting extractFeaturesFromCroppedImage ===\n", __FILE__, __FUNCTION__, __LINE__);
    LogD("%s %s[%d] Employee ID: %s\n", __FILE__, __FUNCTION__, __LINE__, employeeId.toStdString().c_str());
    
    if (employeeId.isEmpty()) {
        LogE("%s %s[%d] Employee ID is empty\n", __FILE__, __FUNCTION__, __LINE__);
        return false;
    }
    
    // Construct path to saved cropped image
    QString croppedImagePath = QString("/mnt/user/reg_face_image/%1.jpg").arg(employeeId);
    
    // Check if cropped image exists
    if (access(croppedImagePath.toStdString().c_str(), F_OK)) {
        LogE("%s %s[%d] Cropped image not found: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, croppedImagePath.toStdString().c_str());
        return false;
    }
    
    LogD("%s %s[%d] Processing cropped image: %s\n", 
         __FILE__, __FUNCTION__, __LINE__, croppedImagePath.toStdString().c_str());
    
    // Extract features from the cropped image
    return extractFeaturesFromImagePath(croppedImagePath, faceFeature, quality);
}

bool BaiduFaceManager::extractFeaturesFromImagePath(const QString &imagePath, QByteArray &faceFeature, double &quality)
{
    Q_D(BaiduFaceManager);
    
    LogD("%s %s[%d] === Starting extractFeaturesFromImagePath ===\n", __FILE__, __FUNCTION__, __LINE__);
    LogD("%s %s[%d] Image path: %s\n", __FILE__, __FUNCTION__, __LINE__, imagePath.toStdString().c_str());
    
    if (imagePath.isEmpty()) {
        LogE("%s %s[%d] Image path is empty\n", __FILE__, __FUNCTION__, __LINE__);
        return false;
    }
    
    // Check if image file exists
    if (access(imagePath.toStdString().c_str(), F_OK)) {
        LogE("%s %s[%d] Image file not found: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, imagePath.toStdString().c_str());
        return false;
    }
    
    // Load the cropped image using OpenCV
    cv::Mat img_mat = cv::imread(imagePath.toStdString());
    if (img_mat.empty()) {
        LogE("%s %s[%d] Failed to load image: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, imagePath.toStdString().c_str());
        return false;
    }
    
    LogD("%s %s[%d] Image loaded successfully: %dx%d\n", 
         __FILE__, __FUNCTION__, __LINE__, img_mat.cols, img_mat.rows);
    
    // Set up image structure for Baidu SDK
    Image_t img_baidu;
    img_baidu.pixel_format = BFACE_INTERLEAVE_U8C3_BGR;
    img_baidu.image_type = BFACE_IMAGE_RGB;
    img_baidu.width = img_mat.cols;
    img_baidu.height = img_mat.rows;
    img_baidu.vir_addr[0] = (bface_pointer_type)img_mat.data;
    
    // Step 1: Detect face in cropped image
    std::vector<BoundingBox_t> bbox_list;
    bface::BFACE_STATUS status = bface_detect_face(img_baidu, &bbox_list);
    
    if (status != BFACE_SUCCESS) {
        LogE("%s %s[%d] Face detection failed with status: %d\n", 
             __FILE__, __FUNCTION__, __LINE__, status);
        return false;
    }
    
    if (bbox_list.size() == 0) {
        LogE("%s %s[%d] No face detected in cropped image\n", __FILE__, __FUNCTION__, __LINE__);
        return false;
    }
    
    if (bbox_list.size() > 1) {
        LogE("%s %s[%d] Multiple faces detected (%d), using the first one\n", 
             __FILE__, __FUNCTION__, __LINE__, (int)bbox_list.size());
    }
    
    LogD("%s %s[%d] Face detected successfully in cropped image\n", __FILE__, __FUNCTION__, __LINE__);
    
    // Step 2: Extract facial landmarks
    Landmark_t landmarks;
    status = bface_alignment(img_baidu, bbox_list[0], &landmarks);
    
    if (status != BFACE_SUCCESS) {
        LogE("%s %s[%d] Face alignment failed with status: %d\n", 
             __FILE__, __FUNCTION__, __LINE__, status);
        return false;
    }
    
    LogD("%s %s[%d] Face alignment successful\n", __FILE__, __FUNCTION__, __LINE__);
    
    // Step 3: Calculate face quality score
    float quality_score = 0.0f;
    status = bface_quality_score(img_baidu, bbox_list[0], &quality_score);
    
    if (status != BFACE_SUCCESS) {
        LogE("%s %s[%d] Quality score calculation failed with status: %d\n", 
             __FILE__, __FUNCTION__, __LINE__, status);
        return false;
    }
    
    quality = quality_score;
    LogD("%s %s[%d] Face quality score: %f\n", __FILE__, __FUNCTION__, __LINE__, quality_score);
    
    // Check if quality meets minimum threshold
    float minQuality = ReadConfig::GetInstance()->getIdentity_Manager_FqThreshold();
    if (quality_score < minQuality) {
        LogE("%s %s[%d] Face quality too low: %f (minimum: %f)\n", 
             __FILE__, __FUNCTION__, __LINE__, quality_score, minQuality);
        return false;
    }
    
    // Step 4: Extract face features
    std::vector<Byte_t> feature_vector;
    
    LogD("%s %s[%d] Starting feature extraction...\n", __FILE__, __FUNCTION__, __LINE__);
    auto extract_start = std::chrono::high_resolution_clock::now();
    
    status = bface_extract_feature(img_baidu, landmarks, &feature_vector);
    
    auto extract_end = std::chrono::high_resolution_clock::now();
    auto extract_duration = std::chrono::duration_cast<std::chrono::milliseconds>(extract_end - extract_start);
    
    if (status != BFACE_SUCCESS) {
        LogE("%s %s[%d] Feature extraction failed with status: %d\n", 
             __FILE__, __FUNCTION__, __LINE__, status);
        return false;
    }
    
    if (feature_vector.empty()) {
        LogE("%s %s[%d] Feature vector is empty\n", __FILE__, __FUNCTION__, __LINE__);
        return false;
    }
    
    LogD("%s %s[%d] Feature extraction successful: %d bytes in %lld ms\n", 
         __FILE__, __FUNCTION__, __LINE__, (int)feature_vector.size(), extract_duration.count());
    
    // Step 5: Convert to QByteArray
    faceFeature = QByteArray(reinterpret_cast<const char*>(feature_vector.data()), feature_vector.size());
    
    LogD("%s %s[%d] === Feature extraction completed successfully ===\n", __FILE__, __FUNCTION__, __LINE__);
    LogD("%s %s[%d] Final results - Feature size: %d bytes, Quality: %f\n", 
         __FILE__, __FUNCTION__, __LINE__, faceFeature.size(), quality);
    
    return true;
}

void BaiduFaceManagerPrivate::DealCamearDataToASVLOFFSCREEN(const int &LiveMode, unsigned long nYuvVirAddr, int width, int height, int rotation)
{
    YNH_LJX::RectF srcrect = { 0, 0, width, height };
    YNH_LJX::RectF dstrect = { 0, 0, q_func()->mDeskWidth, q_func()->mDeskHeight };
    switch(LiveMode)
    {
    case 1:
    {
        YNH_LJX::RkUtils::Utils_RgaDrawImage((unsigned char *) nYuvVirAddr, RK_FORMAT_YCbCr_420_SP, srcrect, width, height, this->m_FaceRgbOffscreen.ppu8Plane[0],
                RK_FORMAT_YCbCr_420_SP, dstrect, q_func()->mDeskWidth, q_func()->mDeskHeight, rotation, 0);
    }break;
    case 2:
    {
        YNH_LJX::RkUtils::Utils_RgaDrawImage((unsigned char *) nYuvVirAddr, RK_FORMAT_YCbCr_420_SP, srcrect, width, height,this->m_FaceIrOffscreen.ppu8Plane[0],
                RK_FORMAT_YCbCr_420_SP, dstrect, q_func()->mDeskWidth, q_func()->mDeskHeight, rotation, 0);
    }break;
    }
}

QList<CORE_FACE_RECT_S> BaiduFaceManagerPrivate::FindMaxFaceDetect(const int &FaceNum, int &MatchCoreFaceIndex, int &MatchFaceRectSize, int &nCurTrackID,  CORE_FACE_S new_update_face_info[])
{

    QList<CORE_FACE_RECT_S>tList;
    //找出最大脸索引
  
    for (int i = 0; i < FaceNum; i++)
    {
        int width = this->m_pMultiFaceInfo->faceRect[i].right - this->m_pMultiFaceInfo->faceRect[i].left;
        int height = this->m_pMultiFaceInfo->faceRect[i].bottom - this->m_pMultiFaceInfo->faceRect[i].top;
        if (MatchFaceRectSize < width * height)
        {
            MatchCoreFaceIndex = i;
            MatchFaceRectSize = width * height;
            nCurTrackID = this->m_pMultiFaceInfo->faceID[i];
            new_update_face_info[i].stFaceRect.nColor = (this->mIdentifyState) ? 0x00ff00 : 0xff0000;
        }

        new_update_face_info[i].stFaceRect.nX = this->m_pMultiFaceInfo->faceRect[i].left;
        new_update_face_info[i].stFaceRect.nY = this->m_pMultiFaceInfo->faceRect[i].top;
        new_update_face_info[i].stFaceRect.nWidth = width;
        new_update_face_info[i].stFaceRect.nHeight = height;
        new_update_face_info[i].enFaceType = CORE_FACE_RECT_TYPE_MOVING;
        new_update_face_info[i].track_id = this->m_pMultiFaceInfo->faceID[i];

        tList.append(new_update_face_info[i].stFaceRect);
    }
    return tList;
}


QString Door_OpenModeToSTR(const _DOOR_OPEN_MODE mode)
{
    switch(mode)
    {
    case _DOOR_OPEN_MODE::ICCARD:return QString("1");//刷卡
    case _DOOR_OPEN_MODE::SWIPING_FACE:return QString("2");//刷脸
    case _DOOR_OPEN_MODE::THERMOMETRY:return QString("3");//测温
    case _DOOR_OPEN_MODE::MASK:return QString("4");//口罩
    case _DOOR_OPEN_MODE::QRCODE:return QString("5");//二唯码
    case _DOOR_OPEN_MODE::IDCARD:return QString("6");//身份证
    case _DOOR_OPEN_MODE::PERSON_IDCARD:return QString("7");//人证比对
    case _DOOR_OPEN_MODE::QRCODE_LOCAL:return QString("8");//粤康码
    default:return QString("255");
    }
}


void BaiduFaceManagerPrivate::CheckMaskDetecct()
{
    int nMask = 0;
    //检查是否戴口罩 ,带不带口罩提取的特征值有区别，需要先检测带没带口罩，再提特征
    //((CBaiduFaceEngine *)m_pFrEngine)->maskDetecct(&m_FaceRgbOffscreen, m_pMultiFaceInfo, &nMask);
    nMask = (nMask <= 0) ? 0 : nMask;
    if (mMatchCoreFace.attr_info.pose_pitch > -20) //人脸低头很容易造成口罩误识别，增加 pitch判断
        mMatchCoreFace.attr_info.face_mask = nMask;
    else mMatchCoreFace.attr_info.face_mask = 0;

	static int m_nDetectMaskWaitFrame = 20; //每隔多少帧检测一次口罩
    bool isFaceMaskMode = false;

    QString door_mode = ReadConfig::GetInstance()->getDoor_MustOpenMode();
    QString Optional_door_mode = ReadConfig::GetInstance()->getDoor_OptionalOpenMode();
 
    if (door_mode.contains(Door_OpenModeToSTR(_DOOR_OPEN_MODE::MASK)) || Optional_door_mode.contains(Door_OpenModeToSTR(_DOOR_OPEN_MODE::MASK)) )
       isFaceMaskMode = true;

    if (isFaceMaskMode == true)
    {
        //检测口罩
        pthread_mutex_lock(&m_stCoreFacesLock);
        if ((m_stFace->nFaceFrameCounter % m_nDetectMaskWaitFrame) == 0)
        {
            float fMaskScore;
            BFACE_STATUS status;
			std::vector<BoundingBox_t> bboxList;            
			status = bface_detect_face(m_stRGBImg, &bboxList);
			if (status != BFACE_SUCCESS)
			{
				pthread_mutex_unlock(&m_stCoreFacesLock);
				return;
			}
           
			int idx = match_bbox_iou(bboxList, q_func()->stTrackedFaceList[q_func()->nMatchCoreFaceIndex].bbox, 0.3);
			if (idx < 0)
			{
				pthread_mutex_unlock(&m_stCoreFacesLock);
				return;
			}

			Landmark_t landmarks;
			status = bface_alignment(m_stRGBImg, bboxList[idx], &landmarks);            
			if (BFACE_SUCCESS != status)
			{
				pthread_mutex_unlock(&m_stCoreFacesLock);
				return;
			}

             std::vector<float> scores;
            status = bface_mouthmask_multi_class(m_stRGBImg, landmarks, &scores);

            if (status != BFACE_SUCCESS)
            {
                pthread_mutex_unlock(&m_stCoreFacesLock);
                return;
            }
            //printf(">>>>>%s,%s,%d,score=%f\n",__FILE__,__func__,__LINE__,scores[1]);
            if (scores[1] >= ReadConfig::GetInstance()->getIdentity_Manager_Mask_value()) //MASK_SCORE
            {
                mMatchCoreFace.attr_info.face_mask = 1;
            }

            //退出当前帧
            pthread_mutex_unlock(&m_stCoreFacesLock);
            return;
        }
        pthread_mutex_unlock(&m_stCoreFacesLock);
    }
}

void BaiduFaceManagerPrivate::CheckLivenessDetect()
{//检测活体
    int ret = -1;
    int nLiveness = -1;
    int m_nDetectLivenessWaitFrame = 7; //每隔多少帧检测一次活体

 

	{
		pthread_mutex_lock(&m_stCoreFacesLock);
        
        if ((mMatchCoreFace.attr_info.liveness_ir == 0) && ((m_stFace->nFaceFrameCounter % m_nDetectLivenessWaitFrame) == 0))
		{

				
			std::vector<BoundingBox_t> bboxList;
			BFACE_STATUS status;
			
			auto detect_start = std::chrono::high_resolution_clock::now();
			status = bface_detect_face(m_stIRImg, &bboxList);
			auto detect_end = std::chrono::high_resolution_clock::now();
			auto detect_duration = std::chrono::duration_cast<std::chrono::milliseconds>(detect_end - detect_start);
			
			if (status != BFACE_SUCCESS)
			{

				pthread_mutex_unlock(&m_stCoreFacesLock);
				return;
			}
			
	
           
			int idx = match_bbox_iou(bboxList, q_func()->stTrackedFaceList[q_func()->nMatchCoreFaceIndex].bbox, 0.3);
			if (idx < 0)
			{
			
				pthread_mutex_unlock(&m_stCoreFacesLock);
				return;
			}

			Landmark_t stIrLandmarks;
			auto align_start = std::chrono::high_resolution_clock::now();
			status = bface_alignment(m_stIRImg, bboxList[idx], &stIrLandmarks);
			auto align_end = std::chrono::high_resolution_clock::now();
			auto align_duration = std::chrono::duration_cast<std::chrono::milliseconds>(align_end - align_start);
			
			if (BFACE_SUCCESS != status)
			{

				pthread_mutex_unlock(&m_stCoreFacesLock);
				return;
			}
			

             
			float fIrLivenessConf = 0;
			auto liveness_start = std::chrono::high_resolution_clock::now();
			status = bface_liveness(m_stIRImg, stIrLandmarks, &fIrLivenessConf);
			auto liveness_end = std::chrono::high_resolution_clock::now();
			auto liveness_duration = std::chrono::duration_cast<std::chrono::milliseconds>(liveness_end - liveness_start);
			
			if (BFACE_SUCCESS != status)
			{
	
				pthread_mutex_unlock(&m_stCoreFacesLock);
				return;
			}
        

		
			if (fIrLivenessConf >= ReadConfig::GetInstance()->getIdentity_Manager_Living_value() || 
				(fIrLivenessConf == 0 && stIrLandmarks.score >= fIrLivenessConf))
			{
                float fRgbLivenessConf = 0;
                auto rgb_liveness_start = std::chrono::high_resolution_clock::now();
                status = bface_liveness(m_stRGBImg, stIrLandmarks, &fRgbLivenessConf);
                auto rgb_liveness_end = std::chrono::high_resolution_clock::now();
                auto rgb_liveness_duration = std::chrono::duration_cast<std::chrono::milliseconds>(rgb_liveness_end - rgb_liveness_start);



                if (fRgbLivenessConf >= ReadConfig::GetInstance()->getIdentity_Manager_Living_value())
                {
                    mMatchCoreFace.attr_info.liveness_ir = 1;
 
                }
                else
                {
         
                }
			}
			else
			{

			}

			pthread_mutex_unlock(&m_stCoreFacesLock);
			return;
		}
		pthread_mutex_unlock(&m_stCoreFacesLock);
	}    
    mMatchCoreFace.attr_info.liveness_ir = (ret == MOK) ? nLiveness : -1;
}

void BaiduFaceManagerPrivate::CheckDetectFace3DAngle(const int &nMatchCoreFaceIndex)
{//检测人脸角度
//BFACE_STATUS bface_face_pose(const Landmark_t& landmarks, Pose_t* pose_ptr)
  // bface_face_pose()
}

void BaiduFaceManagerPrivate::saveFaceImgToDisk(const QString &imgPath, const CORE_FACE_S &FaceTask)
{
    QString path = imgPath;
    int result = -1;
    unsigned char* pTmpBuf = (unsigned char*) YNH_LJX::RkUtils::Utils_Malloc(q_func()->mDeskWidth * q_func()->mDeskHeight * 3 / 2);
    unsigned char* pTmpBuf_1 = (unsigned char*) YNH_LJX::RkUtils::Utils_Malloc(q_func()->mDeskWidth * q_func()->mDeskHeight * 3 / 2);
    if(pTmpBuf == NULL)
    {

    	return;
    }
    if(pTmpBuf_1 == NULL)
    {
    	if(pTmpBuf != NULL)
    	{
    		free(pTmpBuf);
    	}
    	return;
    }
    int x, y, width, height;
    x = FaceTask.stFaceRect.nX;
    y = FaceTask.stFaceRect.nY;
    width = FaceTask.stFaceRect.nWidth;
    height = FaceTask.stFaceRect.nHeight;

    x = x + (x % 2 ? -1 : 0);
    y = y + (y % 2 ? -1 : 0);
    if ((width % 4) != 0)
        width = 4 * ((width / 4) - 1);
    if ((height % 4) != 0)
        height = 4 * ((height / 4) - 1);
    if (x < 0)
    {
        x = 0;
    }
    if (y < 0)
    {
        y = 0;
    }
    if ((x + width) > 736)
        width = (736 - x);
    if ((y + height) > q_func()->mDeskHeight )
        height = (q_func()->mDeskHeight  - y);

    result = YNH_LJX::RkUtils::NV21CutImage((unsigned char *) FaceTask.FaceOffscreen.ppu8Plane[0], q_func()->mDeskWidth, q_func()->mDeskHeight , pTmpBuf,
            width * height * 3 / 2, x, y, x + width, y + height);

    if(result < 0)
    {
        YNH_LJX::RkUtils::Utils_Free(pTmpBuf);
        YNH_LJX::RkUtils::Utils_Free(pTmpBuf_1);
    	return;
    }
    YNH_LJX::RkUtils::Utils_YVU420SPConvertToYUV420P((unsigned long)pTmpBuf, (unsigned long) pTmpBuf_1, width,height);
    YNH_LJX::RkUtils::YUVtoJPEG(path.toLatin1(), (unsigned char*) pTmpBuf_1, width, height);
    YNH_LJX::RkUtils::Utils_ExecCmd("sync");
    YNH_LJX::RkUtils::Utils_Free(pTmpBuf);
    YNH_LJX::RkUtils::Utils_Free(pTmpBuf_1);
    YNH_LJX::RkUtils::Utils_ExecCmd("sync");
}

float BaiduFaceManagerPrivate::CheckFaceQuality() //包含在 FaceFeatureExtract 中
{
	float quality_score = 0.f;
   	pthread_mutex_lock(&m_stAiMutex);
	if (m_bAiInitFinished == false)
	{
		
		pthread_mutex_unlock(&m_stAiMutex);
		return 0.0;
	}
	 if(!mRegFaceState)
	{
		BFACE_STATUS status;
		std::vector<BoundingBox_t> bbox_list;
		
		
		auto start_time = std::chrono::high_resolution_clock::now();
		
		status = bface_detect_face(m_stRGBImg, &bbox_list);
		
		auto end_time = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
		
		if (status != BFACE_SUCCESS)
		{
			
			pthread_mutex_unlock(&m_stAiMutex);
			return 0;
		}
		
		
		
		if (bbox_list.size() == 0)
		{
			
			pthread_mutex_unlock(&m_stAiMutex);
			return 0;
		}

	    bface::helper::MicrosecondTimer quality_timer;
	    
	    
	    auto quality_start = std::chrono::high_resolution_clock::now();
	    
	    status = bface_quality_score(m_stRGBImg, bbox_list[0], &quality_score);
	    
	    auto quality_end = std::chrono::high_resolution_clock::now();
	    auto quality_duration = std::chrono::duration_cast<std::chrono::milliseconds>(quality_end - quality_start);
	    
	    if (status != BFACE_SUCCESS)
	    {
	    
	        pthread_mutex_unlock(&m_stAiMutex);
	        return 0;
	    }
	    
	
	}
	
	// Debug print for quality threshold check
    if(quality_score > 0.5 && (mMatchCoreFace.attr_info.quality > mMatchCoreFace.catch_face_quality || 
    	mMatchCoreFace.catch_face_track_id != mMatchCoreFace.track_id))
    {
    	
    	
    	 if(ReadConfig::GetInstance()->getRecords_Manager_FaceImg() == 1)
    	 {
    	    	memset(mMatchCoreFace.FaceImgPath,0,sizeof(mMatchCoreFace.FaceImgPath));
    	    	std::string path = "/mnt/user/face_crop_image/"+ QDateTime::currentDateTime().toString("yyyy-MM-dd").toStdString()+"/";
    	    	if(access(path.c_str(),F_OK))
    	    	{
    	    		mkdir(path.c_str(),0666);
    	    	}
    	    	path += QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz").toStdString()+"_"+std::to_string(mMatchCoreFace.track_id)+".jpg";
    	    	strncpy(mMatchCoreFace.FaceImgPath,path.c_str(),sizeof(mMatchCoreFace.FaceImgPath));
    	    	
    	

    	    	mMatchCoreFace.catch_face_quality = quality_score;
    	    	mMatchCoreFace.catch_face_track_id = mMatchCoreFace.track_id;
    	    	saveFaceImgToDisk(QString::fromUtf8(mMatchCoreFace.FaceImgPath,strlen(mMatchCoreFace.FaceImgPath)), mMatchCoreFace);
    	 }
    }
	pthread_mutex_unlock(&m_stAiMutex);
	return quality_score;
}

void BaiduFaceManagerPrivate::FaceFeatureExtract()
{
   
    Image_t img = m_stRGBImg;
    
    /// 执行人脸检测
    std::vector<BoundingBox_t> bbox_list;
    
    bface::BFACE_STATUS status = bface_detect_face(img, &bbox_list);
    
    if (status != BFACE_SUCCESS) {
        
        return;
    }
    if (bbox_list.size() == 0) {
       
        return;
    }

    /// 提取人脸关键点
    Landmark_t landmarks;
    
    status = bface_alignment(img, bbox_list[0], &landmarks);
    
    
    if (status != BFACE_SUCCESS) {
        
        return;
    }

    /// 计算人脸得分
    float quality_score = 0.f;
    
    status = bface_quality_score(img, bbox_list[0], &quality_score);
    
    
    if (status != BFACE_SUCCESS) {
        
        return;
    }

    if (quality_score < ReadConfig::GetInstance()->getIdentity_Manager_FqThreshold()) {
        
        return;
    }
   
    /// 提取人脸特征 - CRITICAL SECTION
    std::vector<Byte_t> feature;
    
    
    // Set up a timeout mechanism here if needed
    time_t start_time = time(NULL);
    
    // THIS IS WHERE IT LIKELY HANGS - if you don't see the "RETURNED" message, it hung here
    status = bface_extract_feature(img, landmarks, &feature);
    
    time_t end_time = time(NULL);
    long duration = end_time - start_time;
    
    
    
    
    
    if (status != BFACE_SUCCESS) {
        
        return;
    }
    
    if (mMatchCoreFace.nFaceFeatureSize < feature.size())
    {
        if (mMatchCoreFace.pFaceFeature != ISC_NULL)
        {
            delete []mMatchCoreFace.pFaceFeature;
        }
        mMatchCoreFace.pFaceFeature = new unsigned char[feature.size()];
        if(mMatchCoreFace.pFaceFeature == ISC_NULL)
        {            
        
            return;
        }
    }

    mMatchCoreFace.nFaceFeatureSize = feature.size();
    memcpy(mMatchCoreFace.pFaceFeature, &feature[0], feature.size());
    
}

void BaiduFaceManagerPrivate::DealFaceMove(const int &FaceNum, CORE_FACE_S *new_update_face_info[])
{
}


void *InitFaceEngineThread(void *bdFaceManager)
{
#if 1
    BaiduFaceManager *FaceManager = (BaiduFaceManager *)bdFaceManager;
    char MacCmd[60] = {0};
    static pthread_mutex_t Init = PTHREAD_MUTEX_INITIALIZER; 

    sprintf(MacCmd,"ifconfig eth0 hw ether %s",myHelper::GetNetworkMac().toStdString().c_str());

    LogD("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
    while(!FaceManager->getAlgoFaceInitState()){
        //LogD("%s %s %d\n",__FILE__,__func__,__LINE__);
        system(MacCmd);
        pthread_mutex_lock(&Init);
        FaceManager->setRunFaceEngine();
        pthread_mutex_unlock(&Init);
        if(!FaceManager->getAlgoFaceInitState()){
        	if(checkBaiduLicenseOnlineFlag()){ //如果当前未激活状态，而且U盘里面有激活文件，即不延时，直接再走一遍激活流程，提示U盘激活速度
        		LogD("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
        	}else{
        		sleep(6);   //如果仍是未激活的状态，则6秒后再次尝试激活
        	}
        }else{
        	//如果当前未激活状态，而且U盘里面有激活文件，即一直语音播报，避免已激活后拔电，再上电听不到语音播报
            if(checkBaiduLicenseOnlineFlag() && !access("/param/license.ini",F_OK)&& !access("/param/license.key",F_OK))
            {
                while(1)
                {
                    myHelper::Utils_ExecCmd("sync");
                    LogD("%s %s[%d] algo_actived ...\n", __FILE__, __FUNCTION__, __LINE__);
                    YNH_LJX::Audio::Audio_PlayCustomerPcm("zh","algo_actived.wav",true);
                    sleep(1);
                }
            }
            //while(1);
            //已经激活，清除主屏幕上“算法未激活”这几个字
            ((FaceMainFrm *)qXLApp->GetFaceMainFrm())->slotUpDateTip("");
            pthread_exit(NULL);
        }   
    }
#endif
}

void BaiduFaceManager::startInitFaceEngineThread()
{
    Q_D(BaiduFaceManager);
    pthread_t pid;
    pthread_attr_t pattr;
    //LogD("%s %s %d\n",__FILE__,__func__,__LINE__);
    //判断标志位,是否需要初始化FaceEngine,且使用线程的方式来运行这个初始化函数
    if(!getAlgoFaceInitState()){
        pthread_attr_init(&pattr);
        pthread_create(&pid,&pattr,InitFaceEngineThread,this);
        pthread_detach(pid);
    }
}

void BaiduFaceManager::setRunFaceEngine()
{
    Q_D(BaiduFaceManager);    
    d->InitFaceEngine();
}

//static 
CORE_FACE_S * BaiduFaceManager::getCoreFace(unsigned int nTrackID, unsigned int nExistTrackID[MAX_FACES])
{
    Q_D(BaiduFaceManager);    
        
	CORE_FACE_S *pFoundCoreFace = ISC_NULL;		    
        
	pthread_mutex_lock(&d->m_stCoreFacesLock);
        
	unsigned long long lTrackID = ++nTrackID;
	//CORE_FACE_S *pFoundCoreFace = ISC_NULL;
      
	//查找当前内存里面是否有对应 trackid 的人
	for (int i = 0; i < MAX_FACES; i++)
	{

		if (d->m_stFace->stCoreFaces[i].track_id == lTrackID)
		{
			pFoundCoreFace = &(d->m_stFace->stCoreFaces[i]);			
			pthread_mutex_unlock(&d->m_stCoreFacesLock);
			return pFoundCoreFace;
		}
	}
    
	//上面的操作找不到内存里面对应的人，则从内存里面从头找一个不在当前nExistTrackID 里面的人
	if (pFoundCoreFace == ISC_NULL)
	{
		pFoundCoreFace = &(d->m_stFace->stCoreFaces[0]);
	}

	for (int i = 0; i < MAX_FACES; i++)
	{
		if (nExistTrackID != ISC_NULL)
		{
			//对比当前镜头前的人所对应的trackid，
			bool bEixst = false;
			for (int j = 0; j < MAX_FACES; j++)
			{
				if (d->m_stFace->stCoreFaces[i].track_id == nExistTrackID[j] && nExistTrackID[j] != 0)
				{					
					bEixst = true;
					break;
				}
			}
			if (bEixst == true)
			{				
				continue;
			}
		}
		pFoundCoreFace = &(d->m_stFace->stCoreFaces[i]);
		break;
	}
    
	if (pFoundCoreFace != ISC_NULL)
	{
		memset(pFoundCoreFace, 0, sizeof(CORE_FACE_S));
		pFoundCoreFace->track_id = lTrackID;
	}        
	pthread_mutex_unlock(&d->m_stCoreFacesLock);

	return pFoundCoreFace;
	
}

void BaiduFaceManager::setCameraPreviewYUVData(int nPixelFormat, unsigned long  nYuvVirAddr0, unsigned long  nYuvPhyAddr0, int nWidth0, int nHeight0, int nSize0, int rotation0,
                                 unsigned long  nYuvVirAddr1, unsigned long  nYuvPhyAddr1, int nWidth1, int nHeight1, int nSize1, int rotation1)
{
    Q_D(BaiduFaceManager);
        
    static int frame_count = 0;
    frame_count++;
    
    if((d->mInitEngineSuccess != 1)) return;
#if 1     

    if(d->mRegFaceState) 
    {	
		if (QApplication::activeWindow() )
		{
		   if ( QApplication::activeWindow()->objectName() !=nullptr )
		   {
			  if  ( strstr(QApplication::activeWindow()->objectName().toStdString().c_str() , "AddUserFrm"))		
				   return ;	
		   }
		
		}
    }
#endif     

	//图片注册状态下，暂停人脸运算
	if (d->m_stFace->nState == _PROCESS_REGIST_)
	{
		return;
	}    
	if (d->m_bAiInitFinished == false)
	{
		return;
	}    

    QMutexLocker locker(&d->sync);
	

#if 1 //人脸质量,及保存图片,通行记录中要求
    d->DealCamearDataToASVLOFFSCREEN(1,nYuvVirAddr0, nWidth0,nHeight0,rotation0);
    d->DealCamearDataToASVLOFFSCREEN(2,nYuvVirAddr1, nWidth1,nHeight1,rotation1);	

    memcpy(d->mMatchCoreFace.FaceOffscreen.ppu8Plane[0], d->m_FaceRgbOffscreen.ppu8Plane[0], (d->m_FaceRgbOffscreen.pi32Pitch[0] * d->m_FaceRgbOffscreen.i32Height * 3 / 2));

#endif 
#if  0
	//图片注册状态下，暂停人脸运算
 
	pthread_mutex_lock(&d->m_stCoreFacesLock);
	if (d->m_stFace->nState == _PROCESS_REGIST_)
	{
		pthread_mutex_unlock(&d->m_stCoreFacesLock);
		return;
	}
	pthread_mutex_unlock(&d->m_stCoreFacesLock);
#endif     
#if 0
    d->DealCamearDataToASVLOFFSCREEN(1,nYuvVirAddr0, nWidth0,nHeight0,rotation0);
    d->DealCamearDataToASVLOFFSCREEN(2,nYuvVirAddr1, nWidth1,nHeight1,rotation1);	
     
    memcpy(d->mMatchCoreFace.FaceOffscreen.ppu8Plane[0], d->m_FaceRgbOffscreen.ppu8Plane[0], (d->m_FaceRgbOffscreen.pi32Pitch[0] * d->m_FaceRgbOffscreen.i32Height * 3 / 2));
#endif 
	//TODO STEP 1 喂图片
	YNH_LJX::RectF rect = { 0, 0, mDeskWidth, mDeskHeight };

	d->m_stRGBImg.image_type = bface::BFACE_IMAGE_RGB;
	d->m_stRGBImg.pixel_format = bface::BFACE_INTERLEAVE_U8C3_BGR;

	d->m_stIRImg.image_type = bface::BFACE_IMAGE_NIR;
	d->m_stIRImg.pixel_format = bface::BFACE_INTERLEAVE_U8C3_BGR;

	if (m_pRGBYuvAddr == ISC_NULL)
	{
		m_pRGBYuvAddr = (unsigned char*) malloc(mDeskWidth * mDeskHeight * 3 / 2);
	}
 
	if (m_pRGBYuvAddr != ISC_NULL)
	{
		YNH_LJX::RkUtils::Utils_RgaDrawImage((unsigned char *) nYuvVirAddr0, RK_FORMAT_YCbCr_420_SP, rect, nWidth0, nHeight0, m_pRGBYuvAddr, RK_FORMAT_YCbCr_420_SP,
				rect, mDeskWidth, mDeskHeight, rotation0, 0);
	}   

	if (m_pIrYuvAddr == ISC_NULL)
	{
		m_pIrYuvAddr = (unsigned char*) malloc(mDeskWidth * mDeskHeight * 3 / 2);
	}
  
	if (m_pIrYuvAddr != ISC_NULL)
	{
		YNH_LJX::RkUtils::Utils_RgaDrawImage((unsigned char *) nYuvVirAddr1, RK_FORMAT_YCbCr_420_SP, rect, nWidth1, nHeight1, m_pIrYuvAddr, RK_FORMAT_YCbCr_420_SP,
				rect, mDeskWidth, mDeskHeight, rotation1, 0);
	}   

	if (m_pRGBYuvAddr == ISC_NULL || m_pIrYuvAddr == ISC_NULL)
	{
		return;
	}

	if (d->m_stRGBImg.vir_addr[0] == ISC_NULL)
	{
		d->m_stRGBImg.vir_addr[0] = (bface::bface_pointer_type) malloc(mDeskWidth * mDeskHeight * 3);
		d->m_stRGBImg.width = mDeskWidth;
		d->m_stRGBImg.height = mDeskHeight;
		d->m_stRGBImg.stride[0] = mDeskWidth;
		d->m_stRGBImg.stride[1] = mDeskWidth;
	}

	if (d->m_stIRImg.vir_addr[0] == ISC_NULL)
	{
		d->m_stIRImg.vir_addr[0] = (bface::bface_pointer_type) malloc(mDeskWidth * mDeskHeight * 3);
		d->m_stIRImg.width = mDeskWidth;
		d->m_stIRImg.height = mDeskHeight;
		d->m_stIRImg.stride[0] = mDeskWidth;
		d->m_stIRImg.stride[1] = mDeskWidth;
	}

	if (d->m_stRGBImg.vir_addr[0] == ISC_NULL || d->m_stIRImg.vir_addr[0] == ISC_NULL)
	{
		return;
	}

	YNH_LJX::RkUtils::Utils_RgaDrawImage(m_pRGBYuvAddr, RK_FORMAT_YCbCr_420_SP, rect, mDeskWidth, mDeskHeight, (unsigned char*) d->m_stRGBImg.vir_addr[0], RK_FORMAT_BGR_888,
			rect, mDeskWidth, mDeskHeight, 0, 0);


	YNH_LJX::RkUtils::Utils_RgaDrawImage(m_pIrYuvAddr, RK_FORMAT_YCbCr_420_SP, rect, mDeskWidth, mDeskHeight, (unsigned char*) d->m_stIRImg.vir_addr[0], RK_FORMAT_BGR_888,
			rect, mDeskWidth, mDeskHeight, 0, 0);

	int nFaceNum = 0;
	int nCurTrackID = 0;
	CORE_FACE_S *pstUpdateFaceInfo[MAX_FACES] = { 0 };
    //static int m_nDismissFaceCount = 0;
    //static CORE_FACE_S *m_stCurCoreFace = ISC_NULL;    

    int FaceNum = 0;
   
    ((CBaiduFaceEngine *)d->m_pFtEngine)->faceDetect(d->m_stRGBImg, d->m_pMultiFaceInfo, &FaceNum);
#if 0    
   	pthread_mutex_lock(&d->m_stAiMutex);
	if (d->m_bAiInitFinished == false)
	{
		pthread_mutex_unlock(&d->m_stAiMutex);
		return;
	}
#endif 


	{
		bface::BFACE_STATUS status = bface::bface_detect_and_track(d->m_stRGBImg, &stTrackedFaceList);
        
		if (status == bface::BFACE_SUCCESS)
		{
			float nMaxFaceSize = 0;
			for (int i = 0; i < stTrackedFaceList.size(); i++)
			{
				if ((i + 1) == MAX_FACES)
				{
					break;
				}

      
				//如果质量低于 BD_TRACKING_RGB_CONF 就忽略此人脸
				if (stTrackedFaceList[i].tracking_status == bface::TRACKED && stTrackedFaceList[i].bbox.conf >= ReadConfig::GetInstance()->getIdentity_Manager_FqThreshold())
				{
					float x = stTrackedFaceList[i].bbox.rect.left;
					float y = stTrackedFaceList[i].bbox.rect.top;
					float width = stTrackedFaceList[i].bbox.rect.width;
					float height = stTrackedFaceList[i].bbox.rect.height;					
                    
					pstUpdateFaceInfo[i] = getCoreFace(stTrackedFaceList[i].tracking_id, m_nExistTrackID);
                    
                    if ( pstUpdateFaceInfo[i] != ISC_NULL)
                    {                        
                        if (pstUpdateFaceInfo[i]->attr_info.liveness_ir == 1)
                        {
                            pstUpdateFaceInfo[i]->track_count++;
                        }
                                                
                        pstUpdateFaceInfo[i]->stFaceRect.nX = x;
                        pstUpdateFaceInfo[i]->stFaceRect.nY = y;
                        pstUpdateFaceInfo[i]->stFaceRect.nWidth = width;
                        pstUpdateFaceInfo[i]->stFaceRect.nHeight = height;

                        if (pstUpdateFaceInfo[i]->enFaceType != CORE_FACE_RECT_TYPE_MATCH)
                        {
                            pstUpdateFaceInfo[i]->enFaceType = CORE_FACE_RECT_TYPE_MOVING;
                        }
                        
                        m_nExistTrackID[i] = stTrackedFaceList[i].tracking_id;

                        if (width * height > nMaxFaceSize)
                        {
                            nMaxFaceSize = width * height;
                            nMatchCoreFaceIndex = i; //最大人脸框
                            nCurTrackID = stTrackedFaceList[i].tracking_id;
                        }
                        
                        float fFaceQuality = 0;
                        bface::BoundingBox_t mybox;

                        status = bface_quality_score(d->m_stRGBImg, mybox, &fFaceQuality);
                                                                                
                        if (status == bface::BFACE_SUCCESS)
                        {
                            d->mMatchCoreFace.attr_info.quality = fFaceQuality;
                            pstUpdateFaceInfo[nMatchCoreFaceIndex]->catch_face_quality = fFaceQuality;
                        }   

                        nFaceNum++;
                    } else printf("%s %s[%d]\n", __FILE__, __FUNCTION__, __LINE__);
				}
			}
		}
                    

		if (nFaceNum < 1)
		{
           #if 0 
			int nDismissWaitFrame = 6;
			m_nDismissFaceCount++;
			m_nDismissFaceCount = (m_nDismissFaceCount > (10 * nDismissWaitFrame)) ? (nDismissWaitFrame + 1) : (m_nDismissFaceCount);
			if (m_nDismissFaceCount == nDismissWaitFrame) //多少帧内没人就发dismiss消息
			{
              
			}
           #endif  
            QList<CORE_FACE_RECT_S> tfacelist1;
            emit sigDrawFaceRect(tfacelist1); //if(!d->mRegFaceState) //消除 框
            pthread_mutex_unlock(&d->m_stAiMutex);
            emit sigDisMissMessage(false);              
			return;
		}
		//m_nDismissFaceCount = 0;
	
	}

    pthread_mutex_unlock(&d->m_stAiMutex);

    FaceNum = nFaceNum;   
 
    if (FaceNum < 1)
    {
      // printf(">>>>%s,%s,%d,FaceNum=%d\n",__FILE__,__func__,__LINE__,FaceNum);             
        d->mDismissFaceCount++;
        if (d->mDismissFaceCount >= DISSMISS_WAIT_FRAME)
        {//发送消息
            d->mIdentifyState = false;
            d->mDismissFaceCount = 1;            
        }
        QList<CORE_FACE_RECT_S> tfacelist1;  
        emit sigDrawFaceRect(tfacelist1);  //if(!d->mRegFaceState) 
        emit sigDisMissMessage(false);        
        return;
    }else if(d->mDismissFaceCount)
        d->mDismissFaceCount = 0;
         
    emit sigDisMissMessage(true);

    FaceNum = (FaceNum >= MAX_FACES) ? (MAX_FACES) : FaceNum;
    //CORE_FACE_S new_update_face_info[MAX_FACES]{};

    //int nMatchFaceRectSize = 0;

    //查找最大脸索引,并发送追踪人脸框-界面矩形
    //auto tfacelist = d->FindMaxFaceDetect(FaceNum, nMatchCoreFaceIndex, nMatchFaceRectSize, nCurTrackID, new_update_face_info);


    CORE_FACE_S *pMatchCoreFace = pstUpdateFaceInfo[nMatchCoreFaceIndex];
    //d->mMatchCoreFace = pstUpdateFaceInfo[nMatchCoreFaceIndex];

    d->mMatchCoreFace.stFaceRect.nX = pMatchCoreFace->stFaceRect.nX;
    d->mMatchCoreFace.stFaceRect.nY = pMatchCoreFace->stFaceRect.nY;
    d->mMatchCoreFace.stFaceRect.nWidth = pMatchCoreFace->stFaceRect.nWidth;
    d->mMatchCoreFace.stFaceRect.nHeight = pMatchCoreFace->stFaceRect.nHeight;
    d->mMatchCoreFace.stFaceRect.nColor = (d->mIdentifyState) ? 0x00ff00 : 0xff0000;

    d->mMatchCoreFace.enFaceType = pMatchCoreFace->enFaceType;

    d->mMatchCoreFace.track_id = pMatchCoreFace->track_id;    
    d->mMatchCoreFace.attr_info.liveness_ir = pMatchCoreFace->attr_info.liveness_ir; 

    if(!d->CheckRange(d->mMatchCoreFace))
    {

        if(d->mDismissFaceCount == 0)
        {
            d->mDismissFaceCount = 1;            
            emit sigDisMissMessage(false);
        }
        return;
    }else if(!d->mRegFaceState) 
    {
        d->mMatchCoreFace.stFaceRect.nColor =0x00ff00;//0xff0000;//红// (this->mIdentifyState) ? 0x00ff00 : 0xff0000;
        QList<CORE_FACE_RECT_S> tfacelist;
        for (int i = 0; i < FaceNum ; i++)
        {
            if (nMatchCoreFaceIndex==i) 
            {
              //pstUpdateFaceInfo[i]->stFaceRect.nColor =0x00ff00;//绿
              pstUpdateFaceInfo[i]->stFaceRect.nColor = (d->mIdentifyState) ? 0x00ff00 : 0xff0000;
              pstUpdateFaceInfo[i]->enFaceType=pMatchCoreFace->enFaceType;
            }

            if (pstUpdateFaceInfo[i]!=ISC_NULL)
              tfacelist.append(pstUpdateFaceInfo[i]->stFaceRect);

        }
        
if (tfacelist.size() > 0) {
    LogD("%s %s[%d] === DEBUG FLOW === [%s] BaiduFaceManager passing %d rectangles to CameraManager\n",
         __FILE__, __FUNCTION__, __LINE__, 
         QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz").toStdString().c_str(),
         tfacelist.size());
    
    for (int i = 0; i < tfacelist.size(); i++) {
        LogD("%s %s[%d] === DEBUG FLOW === [%s] Rectangle %d: color=0x%06x, pos=(%d,%d), size=(%dx%d)\n",
             __FILE__, __FUNCTION__, __LINE__,
             QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz").toStdString().c_str(),
             i, tfacelist[i].nColor, tfacelist[i].nX, tfacelist[i].nY, 
             tfacelist[i].nWidth, tfacelist[i].nHeight);
    }
    
    emit sigDrawFaceRect(tfacelist);
} 

   //口罩
    d->CheckMaskDetecct();
    //是否开启活体
    if(mVivoDetection)d->CheckLivenessDetect();
    //d->CheckFaceQuality();//包含在 FaceFeatureExtract 中,预览时已取得质量数
    //printf(">>>>%s,%s,%d,FaceImgPath=%s\n",__FILE__,__func__,__LINE__,d->mMatchCoreFace.FaceImgPath);                        
    d->CheckFaceQuality();//独立出来
    //提取特征码
    d->FaceFeatureExtract();	
    //printf(">>>>%s,%s,%d,FaceImgPath=%s\n",__FILE__,__func__,__LINE__,d->mMatchCoreFace.FaceImgPath);  
    LogD("%s %s[%d] === DEBUG FLOW === [%s] BaiduFaceManager passing face data to IdentityManagement: track_id=%d, quality=%f, liveness=%d\n",
     __FILE__, __FUNCTION__, __LINE__,
     QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz").toStdString().c_str(),
     d->mMatchCoreFace.track_id, d->mMatchCoreFace.attr_info.quality, d->mMatchCoreFace.attr_info.liveness_ir);

    if(!d->mRegFaceState) emit sigMatchCoreFace(d->mMatchCoreFace);

        LogD("%s %s[%d] FINAL FACE ATTRIBUTES - Track ID: %d, Quality: %f, Liveness: %d, Mask: %d, Pose Pitch: %f\n",
    	__FILE__, __FUNCTION__, __LINE__, 
    	d->mMatchCoreFace.track_id,
    	d->mMatchCoreFace.attr_info.quality,
    	d->mMatchCoreFace.attr_info.liveness_ir,
    	d->mMatchCoreFace.attr_info.face_mask,
    	d->mMatchCoreFace.attr_info.pose_pitch);
}

}

#if 0
  0: 成功
  非0:不成功
  -1:path  not exit
  -2: bface_detect_face failed error 
  -3: bface_detect_face no face
  -4: bface_alignment failed error 
  -5:bface_quality_score failed error
  -6:bface_extract_feature failed error
#endif 

int BaiduFaceManager::RegistPerson(const QString &path, int &faceNum, double &threshold, QByteArray &Data)
{
    Q_D(BaiduFaceManager);
    QMutexLocker locker(&d->sync);

    Image_t img2;

    if(access(path.toStdString().c_str(),F_OK))
    {

        return -1;
    }

    // File validation and fixing code...
    printf("%s %s[%d] path=%s  \n",__FILE__,__FUNCTION__,__LINE__,path.toStdString().c_str());    
    char buf[3];  
    FILE *fp = fopen(path.toStdString().c_str(),"ab");
    fseek(fp, -2, SEEK_END);
    fread(buf, 2,1,fp );
    printf("%s %s[%d] buf[0]=%2x, buf[1]=%2x, \n",__FILE__,__FUNCTION__,__LINE__,buf[0] ,buf[1] );            
    if (buf[0]==0xFF && buf[1]==0xD9)
    {}
    else 
    {
        char wbuf[3-1];
        wbuf[0]=0xFF;
        wbuf[1]=0xD9;
        fseek(fp, 0, SEEK_END);
        fwrite(wbuf,2,1, fp);
        
    }
    fclose(fp);

    cv::Mat img_mat = cv::imread(path.toStdString());
    
    if (img_mat.empty()) {
        
        return -1;
    }

    

    img2.pixel_format = BFACE_INTERLEAVE_U8C3_BGR;
    img2.image_type = BFACE_IMAGE_RGB;
    img2.width = img_mat.cols;
    img2.height = img_mat.rows;
    img2.vir_addr[0] = (bface_pointer_type)img_mat.data;
    
    /// 执行人脸检测
    std::vector<BoundingBox_t> bbox_list;
    
    
    auto detect_start = std::chrono::high_resolution_clock::now();
    
    bface::BFACE_STATUS status = bface_detect_face(img2, &bbox_list);
    
    auto detect_end = std::chrono::high_resolution_clock::now();
    auto detect_duration = std::chrono::duration_cast<std::chrono::milliseconds>(detect_end - detect_start);
    
    if (status != BFACE_SUCCESS) {
        
        return -2;
    }
    
    
        
    if (bbox_list.size() == 0) {
        
        return -3;
    }

    faceNum = bbox_list.size();
   
    if (faceNum > 2)
    {
        
        return -7;    
    }
    
    /// 提取人脸关键点
    Landmark_t landmarks;
    
   
    auto align_start = std::chrono::high_resolution_clock::now();
    
    status = bface_alignment(img2, bbox_list[0], &landmarks);
    
    auto align_end = std::chrono::high_resolution_clock::now();
    auto align_duration = std::chrono::duration_cast<std::chrono::milliseconds>(align_end - align_start);
    
    if (status != BFACE_SUCCESS) {
        
        return -4;
    }
    
    

    Pose_t face_pose;
    int luminance_value = 0;
    
    /// 计算人脸得分
    float quality_score = 0.f;
    
   
    bface::helper::MicrosecondTimer quality_timer;
    auto quality_start = std::chrono::high_resolution_clock::now();
    
    status = bface_quality_score(img2, bbox_list[0], &quality_score);
    
    auto quality_end = std::chrono::high_resolution_clock::now();
    auto quality_duration = std::chrono::duration_cast<std::chrono::milliseconds>(quality_end - quality_start);
    
    if (status != BFACE_SUCCESS) {
        
        return -5;
    }
    
    threshold = quality_score;


    bface::helper::MicrosecondTimer timer;
    std::vector<Byte_t> feature;
    
    auto extract_start = std::chrono::high_resolution_clock::now();
    status = bface_extract_feature(img2, landmarks, &feature);
    auto extract_end = std::chrono::high_resolution_clock::now();
    auto extract_duration = std::chrono::duration_cast<std::chrono::milliseconds>(extract_end - extract_start);
    
    int timing = timer.end();
    
    if (status != BFACE_SUCCESS) {
    
        return -6;
    }

   

    Data = QByteArray(reinterpret_cast<const char*>(feature.data()), feature.size());
    
    
    return 0;
}

QString BaiduFaceManager::getCurFaceImgPath(const int quality)
{
    Q_D(BaiduFaceManager);
	if (m_pRGBYuvAddr == ISC_NULL)
    {
        return nullptr;
    }

    QMutexLocker locker(&d->sync);

    unsigned char* pTmpBuf = (unsigned char*) YNH_LJX::RkUtils::Utils_Malloc(mDeskWidth * mDeskHeight * 3 / 2);
    unsigned char* pTmpBuf_1 = (unsigned char*) YNH_LJX::RkUtils::Utils_Malloc(mDeskWidth * mDeskHeight * 3 / 2);
    YNH_LJX::RkUtils::NV21CutImage(m_pRGBYuvAddr, mDeskWidth, mDeskHeight, pTmpBuf, mDeskWidth * mDeskHeight * 3 / 2, 0, 0, mDeskWidth,  mDeskHeight);
    YNH_LJX::RkUtils::Utils_YVU420SPConvertToYUV420P((unsigned long) pTmpBuf, (unsigned long) pTmpBuf_1, mDeskWidth, mDeskHeight);
    {
        YNH_LJX::RkUtils::YUVtoJPEG("/mnt/user/facedb/RegImage.jpeg", (unsigned char*) pTmpBuf_1, mDeskWidth, mDeskHeight);
        system("cp /mnt/user/facedb/RegImage.jpeg /mnt/user/facedb/RegImage.jpg");
    }    

	YNH_LJX::RkUtils::Utils_Free(pTmpBuf);
	YNH_LJX::RkUtils::Utils_Free(pTmpBuf_1);


    return QString("/mnt/user/facedb/RegImage.jpeg");
}

void BaiduFaceManager::saveCurFaceImg(const QString path, const int quality)
{
    Q_D(BaiduFaceManager);
    QMutexLocker locker(&d->sync);
    unsigned char* pTmpBuf = (unsigned char*) YNH_LJX::RkUtils::Utils_Malloc(mDeskWidth * mDeskHeight * 3 / 2);

    YNH_LJX::RkUtils::Utils_YVU420SPConvertToYUV420P((unsigned long)d->mMatchCoreFace.FaceOffscreen.ppu8Plane[0], (unsigned long) pTmpBuf, mDeskWidth,mDeskHeight);
    {
        YNH_LJX::RkUtils::YUVtoJPEG(path.toLatin1().data(), (unsigned char*) pTmpBuf, mDeskWidth, mDeskHeight, quality);
    }
    YNH_LJX::RkUtils::Utils_Free(pTmpBuf);
}

double BaiduFaceManager::getFaceFeatureCompare(unsigned char *FaceFeature, const int &FaceFeatureSize, const QByteArray & MatchFaceData)
{
    Q_D(BaiduFaceManager);
    double similar = 0;

    int ret =-1;

    float score = 0.f;
    bface::StoreType cmp_type;
    cmp_type = RGB_FEATURE;
    if (ret == MOK)return similar;
    return 0.0;
}

double BaiduFaceManager::getFaceFeatureCompare(const QByteArray &FaceFeature, const QByteArray &FaceFeature1)
{
    Q_D(BaiduFaceManager);
    double similar = 0;

    int ret = -1;     
    if (ret == MOK)return similar;
    return 0.0;
}

double BaiduFaceManager::getFaceFeatureCompare(unsigned char *FaceFeature1, const int &FaceFeatureSize1, unsigned char *FaceFeature2, const int &FaceFeatureSize2)
{
    Q_D(BaiduFaceManager);
    double similar = 0;

    int ret =-1;
    if (ret == MOK)return similar;
    return 0.0;
}

double BaiduFaceManager::getFaceFeatureCompare_baidu(unsigned char *FaceFeature1, const int &FaceFeatureSize1, unsigned char *FaceFeature2, const int &FaceFeatureSize2)
{
    Q_D(BaiduFaceManager);
    
    static int recognition_count = 0;
    recognition_count++;
    
   

    double similar = 0;
    int ret = -1;
    float score = 0.f;
    bface::StoreType cmp_type = RGB_FEATURE;

    if (FaceFeatureSize1 > 0 && FaceFeatureSize2 > 0)
    {
        time_t start_time = time(NULL);
        
        // THIS MIGHT ALSO HANG
        bface::BFACE_STATUS status = bface_compare_features(
            std::vector<unsigned char>(FaceFeature1, FaceFeature1 + FaceFeatureSize1), 
            std::vector<unsigned char>(FaceFeature2, FaceFeature2 + FaceFeatureSize2), 
            &score, cmp_type);
        
        time_t end_time = time(NULL);
        long duration = end_time - start_time;
            
        
        

        if (bface::BFACE_SUCCESS == status) {
            return score;
        } else {
        
        }
    } else {
        
    }
    

    return 0.0;
}

double BaiduFaceManager::getPersonIdCardCompare(const QString &idCardPath)
{
// 当前人脸照
   Q_D(BaiduFaceManager);     
   
    Image_t imgFace ;
#if 0    
    cv::Mat img_mat = cv::imread(idCardPath.toStdString());
    printf(">>>>%s,%s,%d idCardPath=%s\n",__FILE__,__func__,__LINE__,idCardPath.toStdString().c_str());
    imgFace.pixel_format = BFACE_INTERLEAVE_U8C3_BGR;
    imgFace.image_type = BFACE_IMAGE_RGB;

    imgFace.width = img_mat.cols;
    imgFace.height = img_mat.rows;
    imgFace.vir_addr[0] = (bface_pointer_type)img_mat.data;
 #endif    
    imgFace = d->m_stRGBImg;
    /// 执行人脸检测
    std::vector<BoundingBox_t> bbox_list;
    bface::BFACE_STATUS status = bface_detect_face(imgFace, &bbox_list);
    if (status != BFACE_SUCCESS) {
        std::cerr << "bface_detect_face failed error : " << status << std::endl;
        return -1;
    }
    if (bbox_list.size() == 0) {
        return -1;
    }
    /// 提取人脸关键点
    Landmark_t landmarks1;
    status = bface_alignment(imgFace, bbox_list[0], &landmarks1);
    if (status != BFACE_SUCCESS) {
        std::cerr << "bface_alignment failed error : " << status << std::endl;
        return -1;
    }
    /// 提取人脸特征
    bface::helper::MicrosecondTimer timer;
    std::vector<Byte_t> feature1;
    status = bface_extract_feature(imgFace, landmarks1, &feature1);
    int timing = timer.end();
    //std::cout << "bface_extract_feature cost : "<< timing << " us." << std::endl;
    if (status != BFACE_SUCCESS) {
        std::cerr << "bface_extract_feature failed error : " << status << std::endl;
        return -1;
    }
//证件照
    //mnt/user/facedb/idcard.jpeg
    Image_t imgIDCard ;
    cv::Mat img_mat2 = cv::imread("/mnt/user/facedb/idcard.jpeg");

    imgIDCard.pixel_format = BFACE_INTERLEAVE_U8C3_BGR;
    imgIDCard.image_type = BFACE_IMAGE_RGB;

    imgIDCard.width = img_mat2.cols;
    imgIDCard.height = img_mat2.rows;
    imgIDCard.vir_addr[0] = (bface_pointer_type)img_mat2.data;
    /// 执行人脸检测
    std::vector<BoundingBox_t> bbox_list2;
    status = bface_detect_face(imgIDCard, &bbox_list2);
    if (status != BFACE_SUCCESS) {
        std::cerr << "bface_detect_face failed error : " << status << std::endl;
        return -1;
    }
    if (bbox_list2.size() == 0) {
        return -1;
    }
    /// 提取人脸关键点
    Landmark_t landmarks2;
    status = bface_alignment(imgIDCard, bbox_list2[0], &landmarks2);
    if (status != BFACE_SUCCESS) {
        std::cerr << "bface_alignment failed error : " << status << std::endl;
        return -1;
    }
    /// 提取人脸特征
    //bface::helper::MicrosecondTimer timer;
    std::vector<Byte_t> feature2;
    status = bface_extract_feature(imgIDCard, landmarks2, &feature2);

    if (status != BFACE_SUCCESS) {
        std::cerr << "bface_extract_feature failed error : " << status << std::endl;
        return -1;
    }

//比较
    float score = 0.f;
    bface::StoreType cmp_type = RGB_FEATURE;

    status = bface_compare_features(feature1, feature2, &score, cmp_type);  

    if (bface::BFACE_SUCCESS == status) return score;

	return 0;
}



bool BaiduFaceManager::algoActive(const QString activeKey)
{
	return false;
}

void BaiduFaceManager::setIdentifyState(const bool &state, const QString &name, const int &personId, const QString &uuid, const QString &idcard)
{
    Q_D(BaiduFaceManager);

    LogD("%s %s[%d] === SET IDENTIFY STATE === State: %s, Name: %s, IDCard: %s\n",
         __FILE__, __FUNCTION__, __LINE__,
         state ? "TRUE" : "FALSE", name.toStdString().c_str(), idcard.toStdString().c_str());

    // Set the state flag
    if (state != d->mIdentifyState)
        d->mIdentifyState = state;

    // ✅ Only emit if recognized and idcard is valid
    if (state && !name.isEmpty() && !idcard.isEmpty()) {

        // ✅ Use idcard instead of personId for uniqueness and rate-limit check
        static QTime lastEmissionTime = QTime::currentTime();
        static QString lastEmittedIdCard;

        bool isSamePerson = (idcard == lastEmittedIdCard);
        int timeSinceLastEmission = lastEmissionTime.msecsTo(QTime::currentTime());

        if (isSamePerson && timeSinceLastEmission < 1500) {
            LogD("%s %s[%d] === RATE LIMITED === Same person (IDCard:%s), only %d ms passed, need 1500ms\n",
                 __FILE__, __FUNCTION__, __LINE__,
                 idcard.toStdString().c_str(), timeSinceLastEmission);
            return; // Skip re-emission for same ID card
        }

        LogD("%s %s[%d] === FAST PATH === Emitting person: %s, IDCard: %s, UUID: %s\n",
             __FILE__, __FUNCTION__, __LINE__,
             name.toStdString().c_str(), idcard.toStdString().c_str(), uuid.toStdString().c_str());

        // 🔔 Emit recognized signal
        emit sigRecognizedPerson(name, personId, uuid, idcard);

        // ✅ Update tracking variables
        lastEmissionTime = QTime::currentTime();
        lastEmittedIdCard = idcard;
    }
}
