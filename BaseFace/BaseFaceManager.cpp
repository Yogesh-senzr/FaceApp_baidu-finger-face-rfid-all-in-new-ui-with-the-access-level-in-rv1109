#include "BaseFaceManager.h"
//#include "CFaceEngine.h"
#include "PCIcore/RkUtils.h"

#include "Config/ReadConfig.h"

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

#include <QThread>
#include <QMutex>
#include <QReadWriteLock>
#include <QBuffer>
#include <QPixmap>
#include <QDebug>

#define NSCALE (16)
#define FACENUM (5)
#define MAX_THREAD  (15)
#define FACESCALEVAL (20)

#define DISSMISS_WAIT_FRAME (10)
#define MOK 0

using namespace std;

class BaseFaceManagerPrivate
{
    Q_DECLARE_PUBLIC(BaseFaceManager)
public:
    BaseFaceManagerPrivate(BaseFaceManager *dd);
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
    void CheckFaceQuality();
    //提取特征码
    void FaceFeatureExtract();
    void saveFaceImgToDisk(const QString &imgPath, const CORE_FACE_S &FaceTask);
private:
    bool CheckRange(const CORE_FACE_S &face);
    void DealFaceMove(const int &FaceNum, CORE_FACE_S *new_update_face_info[]);
private:
    int mInitEngineSuccess;//算法初始化状态,成功与否
    int mDismissFaceCount;//记录多少侦图像未出现人图
    int mIdentityIdentifycm;//识别距离模式
    bool mHasFace;
private:
    mutable	QMutex sync;
private:
    int mLiveMode;//当前检测模式
private:
    bool mRegFaceState;//注册状态
    bool mIdentifyState;//识别状态
private:
    BaseFaceManager *const q_ptr;
};

BaseFaceManagerPrivate::BaseFaceManagerPrivate(BaseFaceManager *dd) //
    : q_ptr(dd) //
    , mInitEngineSuccess(-1) //
	,mHasFace(false)
    , mLiveMode(2) //
    , mDismissFaceCount(0) //
    , mRegFaceState(false) //
    , mIdentifyState(false) //
    , mIdentityIdentifycm(2) //
{
   
}

BaseFaceManager::BaseFaceManager(QObject *parent)
    : QObject(parent)
    , d_ptr(new BaseFaceManagerPrivate(this))
{
    c_RkRgaInit();
    QThread *thread = new QThread;
    this->moveToThread(thread);
    thread->start();
}

BaseFaceManager::~BaseFaceManager()
{

}

void BaseFaceManager::setDeskSize(const int &w, const int &h)
{
    Q_D(BaseFaceManager);

}

void BaseFaceManager::setVivoDetection(const bool &value)
{
    
}

void BaseFaceManager::setRegFaceState(const bool &state)
{
    Q_D(BaseFaceManager);
    d->mRegFaceState = state;
    
    emit sigDisMissMessage(false);
}

bool BaseFaceManager::getRegFaceState()
{
    Q_D(BaseFaceManager);
    return d->mRegFaceState;
}

void BaseFaceManager::setIdentifyState(const bool &state)
{
    Q_D(BaseFaceManager);
    if(state != d->mIdentifyState)d->mIdentifyState = state;
}

void BaseFaceManager::setLivenessThreshold(const float &rgbThreshold, const float &irThreshold)
{
    Q_D(BaseFaceManager);
    
}

void BaseFaceManager::setIdentityIdentifycm(const int &value)
{
    Q_D(BaseFaceManager);
    d->mIdentityIdentifycm = value;
}
bool BaseFaceManager::getAlgoFaceInitState()const
{
    return d_func()->mInitEngineSuccess == 1;
}

bool BaseFaceManager::hasPerson()
{
	return d_func()->mHasFace;
}

int BaseColorSpaceConvert(MInt32 width, MInt32 height, MInt32 format, MUInt8 *imgData, ASVLOFFSCREEN &offscreen)
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

void BaseFaceManagerPrivate::InitFaceEngine()
{
    QMutexLocker locker(&sync);

    this->mInitEngineSuccess = (this->mInitEngineSuccess != 1) ? 0 : 1;
    sync.unlock();
}

int BaseFaceManagerPrivate::InitFtEngine()
{
    int ret =-1;

    return ret;
}

int BaseFaceManagerPrivate::InitFlEngine()
{
    int ret = -1;

    return ret;
}

int BaseFaceManagerPrivate::InitFrEngine()
{
    int ret =-1;
    return ret;
}

int BaseFaceManagerPrivate::InitFaEngine()
{    
    int ret =-1;
    
    return ret;
}

bool BaseFaceManagerPrivate::CheckRange(const CORE_FACE_S &face)
{
    int nMaxOnePersonFaceSize = 1;
    int nFaceRectSize = face.stFaceRect.nHeight * face.stFaceRect.nWidth;
    int nGuessRecognitionDistance = 0;

    return true;
}

void BaseFaceManagerPrivate::DealCamearDataToASVLOFFSCREEN(const int &LiveMode, unsigned long nYuvVirAddr, int width, int height, int rotation)
{
  
}

QList<CORE_FACE_RECT_S> BaseFaceManagerPrivate::FindMaxFaceDetect(const int &FaceNum, int &MatchCoreFaceIndex, int &MatchFaceRectSize, int &nCurTrackID,  CORE_FACE_S new_update_face_info[])
{
    QList<CORE_FACE_RECT_S>tList;
  
    return tList;
}

void BaseFaceManagerPrivate::CheckMaskDetecct()
{//检测口罩

}

void BaseFaceManagerPrivate::CheckLivenessDetect()
{

}

void BaseFaceManagerPrivate::CheckDetectFace3DAngle(const int &nMatchCoreFaceIndex)
{//检测人脸角度
  
}

void BaseFaceManagerPrivate::saveFaceImgToDisk(const QString &imgPath, const CORE_FACE_S &FaceTask)
{

}

void BaseFaceManagerPrivate::CheckFaceQuality()
{

}

void BaseFaceManagerPrivate::FaceFeatureExtract()
{ //提特征
     
}

void BaseFaceManagerPrivate::DealFaceMove(const int &FaceNum, CORE_FACE_S *new_update_face_info[])
{
    QList<CORE_FACE_RECT_S>tList;
    for (int i = 0; i < FaceNum; i++)
    {
        tList.append(new_update_face_info[i]->stFaceRect);
    }
    
    emit q_func()->sigDrawFaceRect(tList);
}

void BaseFaceManager::setRunFaceEngine()
{
    Q_D(BaseFaceManager);
    d->InitFaceEngine();
}

void BaseFaceManager::setCameraPreviewYUVData(int /*nPixelFormat*/, unsigned long nYuvVirAddr0, unsigned long /*nYuvPhyAddr0*/, int nWidth0, int nHeight0, int /*nSize0*/, int rotation0, unsigned long nYuvVirAddr1, unsigned long /*nYuvPhyAddr1*/, int nWidth1, int nHeight1, int /*nSize1*/, int rotation1)
{
    Q_D(BaseFaceManager);
}


int BaseFaceManager::RegistPerson(const QString &img, int &faceNum, double &threshold, QByteArray &Data)
{
    return -1;
}

QString BaseFaceManager::getCurFaceImgPath(const int quality)
{
    Q_D(BaseFaceManager);

    return QString("/mnt/user/facedb/RegImage.jpeg");
}

void BaseFaceManager::saveCurFaceImg(const QString path, const int quality)
{
    Q_D(BaseFaceManager);
}

double BaseFaceManager::getFaceFeatureCompare(unsigned char *FaceFeature, const int &FaceFeatureSize, const QByteArray & MatchFaceData)
{
    Q_D(BaseFaceManager);
    double similar = 0;

  int ret = -1;
    if (ret == MOK)return similar;
    return 0.0;
}

double BaseFaceManager::getFaceFeatureCompare(const QByteArray &FaceFeature, const QByteArray &FaceFeature1)
{
    Q_D(BaseFaceManager);
    double similar = 0;
  
    int ret =-1;
    if (ret == MOK)return similar;
    return 0.0;
}

double BaseFaceManager::getFaceFeatureCompare(unsigned char *FaceFeature1, const int &FaceFeatureSize1, unsigned char *FaceFeature2, const int &FaceFeatureSize2)
{
    Q_D(BaseFaceManager);
 
    return 0.0;
}

double BaseFaceManager::getFaceFeatureCompare_baidu(unsigned char * FaceFeature1, const int &FaceFeatureSize1, unsigned char * FaceFeature2, const int &FaceFeatureSize2)
{
    return 0.0;
}

double BaseFaceManager::getPersonIdCardCompare(const QString &idCardPath)
{
    Q_D(BaseFaceManager);

    return 0.0;
}

bool BaseFaceManager::algoActive(const QString activeKey)
{
    Q_D(BaseFaceManager);

	//当前算法执行到激活状态标识,没执行到状态,不执行激活相关逻辑
	int ret = ISC_ERROR;

    d->sync.unlock();
    return ret == ISC_OK;
}
