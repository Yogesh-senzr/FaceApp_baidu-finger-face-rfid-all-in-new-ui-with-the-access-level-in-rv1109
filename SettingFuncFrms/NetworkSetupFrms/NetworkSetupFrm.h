#ifndef NETWORKSETUPFRM_H
#define NETWORKSETUPFRM_H

#include "SettingFuncFrms/SettingBaseFrm.h"

//网络设置UI
class QListWidgetItem;
class NetworkSetupFrmPrivate;
class NetworkSetupFrm : public SettingBaseFrm
{
    Q_OBJECT
public:
    explicit NetworkSetupFrm(QWidget *parent = nullptr);
    ~NetworkSetupFrm();
private:
    virtual void setLeaveEvent();
    virtual void setEnter();//进入
private:
    Q_SLOT void slotIemClicked(QListWidgetItem *item);
private:
    QScopedPointer<NetworkSetupFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture       
    void mouseDoubleClickEvent(QMouseEvent*);        
#endif    
private:
    Q_DECLARE_PRIVATE(NetworkSetupFrm)
    Q_DISABLE_COPY(NetworkSetupFrm)
};

#endif // NETWORKSETUPFRM_H
