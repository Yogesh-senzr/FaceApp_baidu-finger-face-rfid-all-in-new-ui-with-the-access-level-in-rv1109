#ifndef BASEFACEMANAGER_H
#define BASEFACEMANAGER_H

#include <QObject>
#include <QList>
#include "SharedInclude/GlobalDef.h"

class BaseFaceManagerPrivate;
class BaseFaceManager : public QObject
{
    Q_OBJECT
public:
    explicit BaseFaceManager(QObject *parent = nullptr);
    ~BaseFaceManager();
public:
    virtual void setDeskSize(const int &, const int &);
    virtual void setVivoDetection(const bool &);//活体检测
    virtual void setRegFaceState(const bool& state);//设置进行注册状态
    virtual bool getRegFaceState();
    virtual void setIdentifyState(const bool &);//识别状态追踪人脸框颜色(true:绿色,false：红色)
    virtual void setLivenessThreshold(const float &, const float &);
    virtual void setIdentityIdentifycm(const int &);//识别距离
public:
    virtual bool getAlgoFaceInitState() const;
    virtual bool algoActive(const QString);
    virtual bool hasPerson();

public:
    //MInt32 faceNum;// 检测到的人脸个数
    virtual int RegistPerson(const QString &, int &faceNum, double &threshold, QByteArray &);
    //获取当前识别的人脸路片路径
    virtual QString getCurFaceImgPath(const int quality = 90);
    //保存当前脸图片
    void saveCurFaceImg(const QString, const int quality = 90);

    double getFaceFeatureCompare(unsigned char * FaceFeature, const int &FaceFeatureSize, const QByteArray &MatchFaceData);
    double getFaceFeatureCompare(const QByteArray &FaceFeature, const QByteArray &FaceFeature1);
    double getFaceFeatureCompare(unsigned char * FaceFeature1, const int &FaceFeatureSize1, unsigned char * FaceFeature2, const int &FaceFeatureSize2);
    virtual double getFaceFeatureCompare_baidu(unsigned char * FaceFeature1, const int &FaceFeatureSize1, unsigned char * FaceFeature2, const int &FaceFeatureSize2);       
    //人证比对函数会主动获取当前的图片进行比对
    double getPersonIdCardCompare(const QString &idCardPath);
public:
    void setRunFaceEngine();
public:
    void setCameraPreviewYUVData(int nPixelFormat, unsigned long  nYuvVirAddr0, unsigned long  nYuvPhyAddr0, int nWidth0, int nHeight0, int nSize0, int rotation0,
                                 unsigned long  nYuvVirAddr1, unsigned long  nYuvPhyAddr1, int nWidth1, int nHeight1, int nSize1, int rotation1);
public:
    Q_SIGNAL void sigDisMissMessage(const bool);//当前无人脸（false没人脸， true有人脸）
    Q_SIGNAL void sigDrawFaceRect(const QList<CORE_FACE_RECT_S>);//画人脸移动
    Q_SIGNAL void sigMatchCoreFace(const CORE_FACE_S);
private:
    QScopedPointer<BaseFaceManagerPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(BaseFaceManager)
    Q_DISABLE_COPY(BaseFaceManager)
};

#endif // BASEFACEMANAGER_H
