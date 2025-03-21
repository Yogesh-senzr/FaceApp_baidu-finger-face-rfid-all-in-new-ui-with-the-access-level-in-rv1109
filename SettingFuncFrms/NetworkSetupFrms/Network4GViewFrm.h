#ifndef MINGRUAN_COMMON_FACEAPP_SETTINGFUNCFRMS_NETWORKSETUPFRMS_NETWORK4GVIEWFRM_H_
#define MINGRUAN_COMMON_FACEAPP_SETTINGFUNCFRMS_NETWORKSETUPFRMS_NETWORK4GVIEWFRM_H_

#include "SettingFuncFrms/SettingBaseFrm.h"
#include <QtCore/QObject>

class Network4GViewFrmPrivate;
class Network4GViewFrm : public SettingBaseFrm
{
    Q_OBJECT
public:
    explicit Network4GViewFrm(QWidget *parent = nullptr);
    ~Network4GViewFrm();

private:
    Q_SLOT void slotNetwork4GSwitchState(const int);

private:
    virtual void setEnter();//进入
    virtual void setLeaveEvent();//退出

private:
    QScopedPointer<Network4GViewFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture        
    void mouseDoubleClickEvent(QMouseEvent*);         
#endif    
private:
    Q_DECLARE_PRIVATE(Network4GViewFrm)
    Q_DISABLE_COPY(Network4GViewFrm)
};

#endif /* MINGRUAN_COMMON_FACEAPP_SETTINGFUNCFRMS_NETWORKSETUPFRMS_NETWORK4GVIEWFRM_H_ */
