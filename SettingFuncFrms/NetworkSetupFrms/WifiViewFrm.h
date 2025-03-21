#ifndef WIFIVIEWFRM_H
#define WIFIVIEWFRM_H

#include "SettingFuncFrms/SettingBaseFrm.h"

#include <QVariant>
#include <QList>
//用天显示wifi例表
class QListWidgetItem;
class WifiViewFrmPrivate;
class WifiViewFrm : public SettingBaseFrm
{
    Q_OBJECT
public:
    explicit WifiViewFrm(QWidget *parent = nullptr);
    ~WifiViewFrm();
private:
    virtual void setEnter();//进入
    virtual void setLeaveEvent();//退出
private:
    Q_SLOT void slotWifiList(const QList<QVariant>);
    Q_SLOT void slotWifiSwitchState(const int);
    // Q_SLOT void slotWifiConnected();
    // Q_SLOT void slotWifiDisconnected();
    Q_SLOT void slotSaveAndRestart();
private:
    Q_SLOT void slotIemClicked(QListWidgetItem *item);
private:
    QScopedPointer<WifiViewFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture        
    void mouseDoubleClickEvent(QMouseEvent*);        
#endif    
private:
    Q_DECLARE_PRIVATE(WifiViewFrm)
    Q_DISABLE_COPY(WifiViewFrm)
};

#endif // WIFIVIEWFRM_H
