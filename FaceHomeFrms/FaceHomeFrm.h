#ifndef FaceHomeFrm_H
#define FaceHomeFrm_H

#include <QWidget>
#include "SharedInclude/GlobalDef.h"

//主要用于提示操作的透明UI
class FaceHomeFrmPrivate;
class FaceHomeFrm : public QWidget
{
    Q_OBJECT
public:
    explicit FaceHomeFrm(QWidget *parent = nullptr);
    ~FaceHomeFrm();
public:
    void setDisMissMessage(const bool &state);
    void setTipsMessage(const int &, const int &, const QString &);
    //固件更新
    void setUpDateTip(const QString&);
    //显示健康码信息
    void setHealthCodeInfo(const int &type, const QString &name, const QString &idCard, const int &qrCodeType, const QString &msg);

    //显示算法相关当前人脸参数
    void setAlgoStateAboutFace(const QString &);
public:
    void setHomeDisplay_SnNum(const int &);
    void setHomeDisplay_Mac(const int &);
    void setHomeDisplay_IP(const int &);
    void setHomeDisplay_PersonNum(const int &);
    void setHomeDisplay_DoorLock(const int &);    
public:
    void setTopMessageHintText_1(const QString &);
    void setTopMessageHintText_1_AlarmTemperature(const QString &);
    void setTopMessageHintText_2(const QString &);
    void setTopMessageHintText_3(const QString &);
    void setTopMessageHintText_4(const QString &);
    void setTopMessageHintText_5(const QString &);

    void setBottomMessageHintText_1(const QString &);
    void setBottomMessageHintText_2(const QString &);
    void setBottomMessageHintText_3(const QString &);
private:
    Q_SLOT void onCheckSN();
    Q_SLOT void onCheckNet();
    Q_SLOT void onCheckPersonNum();
private:
    Q_SLOT void slotNetChange(bool status);  
private:
   // void paintEvent(QPaintEvent *event);
private:
    QScopedPointer<FaceHomeFrmPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(FaceHomeFrm)
    Q_DISABLE_COPY(FaceHomeFrm)
};

#endif // HOMEFRM_H
