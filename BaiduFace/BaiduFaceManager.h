#ifndef BAIDUFACEMANAGER_H
#define BAIDUFACEMANAGER_H

#include <QObject>
#include <QList>
#include "BaseFace/BaseFaceManager.h"
#include "SharedInclude/GlobalDef.h"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "helper/face_utils.hpp"
#include "helper/image_convert.hpp"
#include "helper/io/io.h"
#include "helper/timer/timer.h"

//多人脸信息
typedef struct
{
	MRECT* faceRect;			// 人脸框信息
	//MInt32* faceOrient;		// 输入图像的角度，可以参考 ArcFaceCompare_OrientCode .
	MInt32 faceNum;			// 检测到的人脸个数
	MInt32* faceID;			// face ID，IMAGE模式下不返回FaceID
	//MFloat* wear_glasses;		//戴眼镜状态,0 未戴眼镜；1 戴眼镜
	//MInt32* left_eye_closed;	//左眼状态 0 未闭眼；1 闭眼
	//MInt32* right_eye_closed;	//右眼状态 0 未闭眼；1 闭眼
} Baidu_MultiFaceInfo, *LPBaidu_MultiFaceInfo;


class BaiduFaceManagerPrivate;
//class BaiduFaceManager : public QObject
class BaiduFaceManager : public BaseFaceManager
{
    Q_OBJECT
public:
    explicit BaiduFaceManager(QObject *parent = nullptr);
    ~BaiduFaceManager();
public:
    void setDeskSize(const int &, const int &);
    void setVivoDetection(const bool &);//活体检测
    void setRegFaceState(const bool& state);//设置进行注册状态
    bool getRegFaceState();
    void setIdentifyState(const bool &);//识别状态追踪人脸框颜色(true:绿色,false：红色)
    void setLivenessThreshold(const float &, const float &);
    void setIdentityIdentifycm(const int &);//识别距离
public:
    bool getAlgoFaceInitState() const;
    bool algoActive(const QString);
    bool hasPerson();
    CORE_FACE_S *getCoreFace(unsigned int nTrackID, unsigned int nExistTrackID[MAX_FACES]);
private:
    //static 
    unsigned int m_nExistTrackID[MAX_FACES] = { 0 };
    std::vector<bface::TrackedBox_t> stTrackedFaceList;
    int nMatchCoreFaceIndex = 0;
    double m_nIrThreshold = 0.7;            //ir活体阈值  
    unsigned char *m_pRGBYuvAddr;
    unsigned char *m_pIrYuvAddr;  
    int mDeskWidth = 800;//屏幕宽
    int mDeskHeight = 1280;//屏幕高
    bool mVivoDetection = true;//活体检测    

public:
    //MInt32 faceNum;// 检测到的人脸个数
    int RegistPerson(const QString &, int &faceNum, double &threshold, QByteArray &);
    //获取当前识别的人脸路片路径
    QString getCurFaceImgPath(const int quality = 90);
    //保存当前脸图片
    void saveCurFaceImg(const QString, const int quality = 90);

    double getFaceFeatureCompare(unsigned char * FaceFeature, const int &FaceFeatureSize, const QByteArray &MatchFaceData);
    double getFaceFeatureCompare(const QByteArray &FaceFeature, const QByteArray &FaceFeature1);
    double getFaceFeatureCompare(unsigned char * FaceFeature1, const int &FaceFeatureSize1, unsigned char * FaceFeature2, const int &FaceFeatureSize2);
    double getFaceFeatureCompare_baidu(unsigned char * FaceFeature1, const int &FaceFeatureSize1, unsigned char * FaceFeature2, const int &FaceFeatureSize2);    
    //人证比对函数会主动获取当前的图片进行比对
    double getPersonIdCardCompare(const QString &idCardPath);
public:
    void startInitFaceEngineThread();
    void setRunFaceEngine();
public:
    void setCameraPreviewYUVData(int nPixelFormat, unsigned long  nYuvVirAddr0, unsigned long  nYuvPhyAddr0, int nWidth0, int nHeight0, int nSize0, int rotation0,
                                 unsigned long  nYuvVirAddr1, unsigned long  nYuvPhyAddr1, int nWidth1, int nHeight1, int nSize1, int rotation1);
public:
    Q_SIGNAL void sigDisMissMessage(const bool);//当前无人脸（false没人脸， true有人脸）
    Q_SIGNAL void sigDrawFaceRect(const QList<CORE_FACE_RECT_S>);//画人脸移动
    Q_SIGNAL void sigMatchCoreFace(const CORE_FACE_S);
private:
    QScopedPointer<BaiduFaceManagerPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(BaiduFaceManager)
    Q_DISABLE_COPY(BaiduFaceManager)
};

#endif // BAIDUFACEMANAGER_H
