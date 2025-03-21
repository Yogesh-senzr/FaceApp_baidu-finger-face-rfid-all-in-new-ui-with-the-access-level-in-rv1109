#ifndef SRVSETUPFRM_H
#define SRVSETUPFRM_H

#include "SettingFuncFrms/SettingBaseFrm.h"

//服务器设置UI
class QListWidgetItem;
class SrvSetupFrmPrivate;
class SrvSetupFrm : public SettingBaseFrm
{
    Q_OBJECT
public:
    explicit SrvSetupFrm(QWidget *parent = nullptr);
    ~SrvSetupFrm();
private:
    virtual void setEnter();
private:
    Q_SLOT void slotConfirmButton();
    Q_SLOT void slotIemClicked(QListWidgetItem *item);
private:
    QScopedPointer<SrvSetupFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture      
    void mouseDoubleClickEvent(QMouseEvent*);        
#endif     
private:
    Q_DECLARE_PRIVATE(SrvSetupFrm)
    Q_DISABLE_COPY(SrvSetupFrm)
};

#endif // SRVSETUPFRM_H
