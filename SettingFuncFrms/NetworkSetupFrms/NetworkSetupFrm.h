// Updated NetworkSetupFrm.h - Card-based theme support

#ifndef NETWORKSETUPFRM_H
#define NETWORKSETUPFRM_H

#include "SettingFuncFrms/SettingBaseFrm.h"
#include <QFrame>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

class QListWidgetItem;
class NetworkSetupFrmPrivate;

// Forward declaration for the custom card widget
class NetworkCardWidget;

class NetworkSetupFrm : public SettingBaseFrm
{
    Q_OBJECT
public:
    explicit NetworkSetupFrm(QWidget *parent = nullptr);
    ~NetworkSetupFrm();
    
    // Method to update network status (call this when network state changes)
    void updateNetworkStatus(const QString &networkType, const QString &status, bool connected);

private:
    virtual void setLeaveEvent();
    virtual void setEnter(); // 进入
    
private slots:
    void slotIemClicked(QListWidgetItem *item); // Keep for compatibility
    void onCardClicked(const QString &cardTitle); // New slot for card clicks

signals:
    void sigShowFrm(const QString &frmName);

private:
    QScopedPointer<NetworkSetupFrmPrivate> d_ptr;
    
#ifdef SCREENCAPTURE
    void mouseDoubleClickEvent(QMouseEvent*);        
#endif    

private:
    Q_DECLARE_PRIVATE(NetworkSetupFrm)
    Q_DISABLE_COPY(NetworkSetupFrm)
};

#endif // NETWORKSETUPFRM_H