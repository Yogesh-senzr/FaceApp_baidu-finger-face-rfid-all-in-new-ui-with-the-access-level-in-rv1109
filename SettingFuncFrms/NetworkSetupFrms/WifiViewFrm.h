// Updated WifiViewFrm.h - Card-based theme support

#ifndef WIFIVIEWFRM_H
#define WIFIVIEWFRM_H

#include "SettingFuncFrms/SettingBaseFrm.h"
#include <QVariant>

class QListWidgetItem;
class WifiViewFrmPrivate;

class WifiViewFrm : public SettingBaseFrm
{
    Q_OBJECT
public:
    explicit WifiViewFrm(QWidget *parent = nullptr);
    ~WifiViewFrm();
    
    // Method to handle network card clicks
    void handleNetworkClicked(const QString &service, const QString &ssid);

private:
    virtual void setEnter();
    virtual void setLeaveEvent();
    
private slots:
    void slotWifiList(const QList<QVariant> obj);
    void slotWifiSwitchState(bool state);
    void slotIemClicked(QListWidgetItem *item); // Keep for compatibility
    void slotSaveAndRestart();

private:
    QScopedPointer<WifiViewFrmPrivate> d_ptr;
    
#ifdef SCREENCAPTURE
    void mouseDoubleClickEvent(QMouseEvent*);        
#endif    

private:
    Q_DECLARE_PRIVATE(WifiViewFrm)
    Q_DISABLE_COPY(WifiViewFrm)
};

#endif // WIFIVIEWFRM_H