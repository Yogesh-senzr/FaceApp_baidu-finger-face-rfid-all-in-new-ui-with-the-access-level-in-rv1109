#ifndef IDENTITYMANAGEMENTPRIVATE_H
#define IDENTITYMANAGEMENTPRIVATE_H

#include "IdentityManagement.h"
#include <QMutex>
#include <QWaitCondition>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

//身份证
typedef struct
{
    QString name;
    QString sex;
    QString idCard;
    QString pathImage;//身份证图片路径
    uint timer;//当前时间
}IdentityCard_t;

typedef struct
{
    QString number;//ic卡号
    uint timer;//当前时间
}icCard_t;
//健康码
typedef struct
{
    QString name;
    QString sex;
    QString idCard;
    QString msg;
    int qrcode;//健康码状态
    int type;//类型(健康码扫描头、身份证)
    double warningTemp;
    uint timer;//当前时间
}HealthCode_t;

typedef struct 
{
    char szName[64];
    char szIDCardNum[64];
    char szIDAddress[200];
    char szSex[8];
    char szNationality[20];
    char szBirthDate[20];
    char szIssuingAuthority[200];
    char szStartDate[20];
    char szEndDate[20];    
} IDCardInfo;

class FaceDataResolverObj;
class IdentityManagementPrivate
{
    Q_DECLARE_PUBLIC(IdentityManagement)
public:
    IdentityManagementPrivate(IdentityManagement *dd);
    ~IdentityManagementPrivate();
protected:
    virtual bool DistributeTheTasks();
protected:
    //检测口罩模式的数据
    virtual void CheckMaskMode(const CORE_FACE_S &face);
    virtual void CheckNotMaskMode(const CORE_FACE_S &face);
    virtual bool CheckLiveness(const CORE_FACE_S &face);
    //检测口罩状态
    virtual bool CheckMaskState(const CORE_FACE_S &face);
    //检测温度状态
    virtual bool CheckTempState();
    //检测开门方式
    virtual bool CheckDoorOpenMode(IdentifyFaceRecord_t &);
    //检测能否开门
    virtual bool CanOpenDoor();    
protected:
    virtual bool ComparingFace(const CORE_FACE_S &face, const IdentifyFaceRecord_t&oldFace);
protected:
    static QString Door_OpenModeToSTR(const _DOOR_OPEN_MODE mode);
protected:
    QList<FaceDataResolverObj *> m_ThreadObjs;
protected:
    IdentifyFaceRecord_t mIdentifyFaceRecord;
private:
    QNetworkAccessManager *m_manager;
    QNetworkReply *m_Reply;    
protected:
    icCard_t micCard;//ic卡信息
    IdentityCard_t mIdentityCard;//身份证信息
    HealthCode_t mHealthCode;//健康码信息
    IDCardInfo*  mIDCardInfo;//身份证信息    
protected:
    /*算法识别控制*/
    bool mVivoDetection;//活体检测
    float mAlarmTemp;//报警温度
    float mMaskValue;//口罩阈值
    float mFqThreshold;//人脸质量
    float mThanthreshold;//比对阈值
    float midcardThanthreshold;//身份证比对阈值
    int mIdentifyInterval;//识别间隔
    int mFirstTime ; //是否初次通行 1, 首次 , 0    
protected:
    float mCurTemperatureValue;//当前采集到的温度值
protected:
    QString mMustOpenMode;//开门模式必选项
    QString mOptionalOpenMode;//开门模式可选
protected:
    mutable QMutex sync;
    QWaitCondition pauseCond;
protected:
    IdentityManagement *const q_ptr;

private:
    QString mAlgoStateAboutFace;
};

#endif // IDENTITYMANAGEMENTPRIVATE_H
