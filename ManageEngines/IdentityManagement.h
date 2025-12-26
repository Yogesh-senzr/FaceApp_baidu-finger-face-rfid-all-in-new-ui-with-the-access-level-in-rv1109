#ifndef IDENTITYMANAGEMENT_H
#define IDENTITYMANAGEMENT_H

#include <QThread>
#include "SharedInclude/GlobalDef.h"

class IdentityManagementPrivate;

class OptimizedDisplayHelper {
public:
    static QString createVerifiedDisplay(const QString &name, const QString &idCard) {
        const int totalSize = 26 + name.length() + idCard.length();
        
        QString result;
        result.reserve(totalSize);
        
        result += "Name: ";      // 6 chars
        result += name;          
        result += "\nEMP ID: "; // 9 chars 
        result += idCard;        
        result += "\nVerified";  // 9 chars
        
        return result;
    }
};

class IdentityManagementPrivate;
class IdentityManagement : public QThread
{
    Q_OBJECT
public:
    IdentityManagement(QObject *parent = nullptr);
    IdentityManagement(IdentityManagementPrivate *q, QObject *parent = nullptr);
    ~IdentityManagement();
public:
    /*算法识别控制*/
    void setVivoDetection(const bool &);//活体检测
    void setAlarmTemp(const float &);//报警温度
    void setMask_value(const float &);//口罩阈值
    void setFqThreshold(const float &);//人脸质量阈值
    void setThanthreshold(const float &);//比对阈值
    void setidcardThanthreshold(const float &);//身份证比对阈值
    void setIdentifyInterval(const int &);//识别间隔
    void ShowLRHealthCodeAndOpenDoor(HEALTINFO_t info,const int nKind); //显示健康码并开门 nKind:1://粤2.身份证
public://开门方式
    void setDoor_MustOpenMode(const QString &);//必须项
    void setDoor_OptionalOpenMode(const QString &);//可选项
    void setDeviceAccessLevel(const QString& level);
public:
    //当前识别到人脸
    Q_SLOT virtual void slotMatchCoreFace(const CORE_FACE_S);
    //当前无人脸消息
    Q_SLOT virtual void slotDisMissMessage(const bool state);//当前无人脸
    //ic卡模块
    Q_SLOT virtual void slotReadIccardNum(const QString iccard);
    Q_SLOT virtual void slotFingerprintMatched(const QString &personId, 
                               const QString &name, 
                               const int &confidence);
    Q_SLOT virtual void slotFingerprintNotRecognized(); 
    //平台健康码
    Q_SLOT virtual void slotHealthCodeInfo(const int type, const QString name, const QString idCard, const int qrCodeType, const double warningTemp, const QString msg); 
    //Q_SLOT virtual void slotLRHealthCodeInfo(const int type, const QString name, const QString idCard, const int qrCodeType, const double warningTemp, const QString Msg );    
    //身份证识别信息
    Q_SLOT virtual void slotIdentityCardInfo(const QString name, const QString idCard, const QString sex, const QString path);
    Q_SLOT virtual void slotIdentityCardFullInfo(const QString name,const QString idCardNo,const QString address,const QString sex, 
        const QString nationality,const QString birthDate,const QString issuingAuthority,const QString startDate, const QString endDate);
      
   
   // virtual void queryHealtCodeWithIDCard(const char *IdNumber, const char* IdName);        
    //温度值
    Q_SLOT virtual void slotTemperatureValue(const float value); 
    //清空开门信息
    Q_SLOT virtual void slotDisClearMessage();//
    Q_SLOT void slotLRHealthCodeInfo2(HEALTINFO_t info);    
public:
    /*参数：位置(顶与底), 序号(上下各4项)， 提示消息*/
    Q_SIGNAL void sigTipsMessage(const int, const int, const QString);
    /*显示健康码信息*/
    Q_SIGNAL void sigShowHealthCode(const int type, const QString name, const QString idCard, const int qrCodeType, const double warningTemp, const QString msg);
    Q_SIGNAL void sigShowLRHealthCode(HEALTINFO_t info);
    /*查询DK身份证健康码*/
    Q_SIGNAL void sigDKQueryHealthCode(const QString name, const QString idCard, const QString sex);

    /*显示人脸相关参数*/
    Q_SIGNAL void sigShowAlgoStateAboutFace(const QString);
    /*显示口罩信息*/
    Q_SIGNAL void sigMaskFaceMessage(const QString);   
    
    Q_SIGNAL void sigShowPersonImage(const QString &imagePath, const QString &personName);
    Q_SIGNAL void sigClearPersonImage();
    Q_SIGNAL void sigRecognizedPerson(const QString &name, const int &personId, 
                           const QString &uuid, const QString &idcard);
private:
    //人脸识完成时回调通知（任务号、是否陌生人、uuid）
    void EchoFaceRecognition(const int &id, const int &FaceType, 
                           const int &face_personid, const int &face_persontype, 
                           const QString &face_name, const QString &face_sex, 
                           const QString &face_uuid, const QString &face_idcardnum, 
                           const QString &face_iccardnum, const QString &face_gids, 
                           const QString &face_aids, const QByteArray &face_feature);
    void EchoFingerprintRecognition(
    const int &face_personid,
    const int &FaceType, 
    const QString &face_name,
    const QString &face_uuid,
    const QString &face_idcardnum,
    const QString &face_sex,
    const int &face_persontype);
    bool CheckAccessLevelFromDB(int userAccessLevel);
private:
    void run();
    QString m_deviceAccessLevel;
protected:
    QScopedPointer<IdentityManagementPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(IdentityManagement)
    Q_DISABLE_COPY(IdentityManagement)

public:
    void clearThenSetBottomMessage(const QString &message);
};

#endif // IDENTITYMANAGEMENT_H