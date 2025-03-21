#ifndef ETHERNETVIEWFRM_H
#define ETHERNETVIEWFRM_H

#include "SettingFuncFrms/SettingBaseFrm.h"

//网口配置
class EthernetViewFrmPrivate;
class EthernetViewFrm : public SettingBaseFrm
{
    Q_OBJECT
public:
    explicit EthernetViewFrm(QWidget *parent = nullptr);
    ~EthernetViewFrm();
private:
    virtual void setEnter();//进入
private:
    Q_SLOT void slotSaveButtonClicked();
    Q_SLOT void slotLanSwitchState(const int state);
private:
    QScopedPointer<EthernetViewFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture     
    void mouseDoubleClickEvent(QMouseEvent*);     
#endif    
private:
    Q_DECLARE_PRIVATE(EthernetViewFrm)
    Q_DISABLE_COPY(EthernetViewFrm)
};

#endif // ETHERNETVIEWFRM_H
