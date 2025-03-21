#ifndef SYSSETUPFRM_H
#define SYSSETUPFRM_H

#include "SettingFuncFrms/SettingBaseFrm.h"

//系统配置UI
class QListWidgetItem;
class SysSetupFrmPrivate;
class QRCodeFrm;
class SysSetupFrm : public SettingBaseFrm
{
    Q_OBJECT
public:
    explicit SysSetupFrm(QWidget *parent = nullptr);
    ~SysSetupFrm();
private:
     virtual void setEnter();//进入
    virtual void setLeaveEvent();//退出
private:
    Q_SLOT void slotIemClicked(QListWidgetItem *item);
private:
    QScopedPointer<SysSetupFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture     
    void mouseDoubleClickEvent(QMouseEvent*);   
#endif          
private:
    Q_DECLARE_PRIVATE(SysSetupFrm)
    Q_DISABLE_COPY(SysSetupFrm)
};

#endif // SYSSETUPFRM_H
