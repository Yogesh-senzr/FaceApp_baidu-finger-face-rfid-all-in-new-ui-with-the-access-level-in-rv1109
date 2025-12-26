// Updated HomeMenuFrm.h - Card-based theme support

#ifndef HOMEMENUFRM_H
#define HOMEMENUFRM_H

#include "SettingFuncFrms/SettingBaseFrm.h"

class HomeMenuFrmPrivate;

class HomeMenuFrm : public SettingBaseFrm
{
    Q_OBJECT
public:
    explicit HomeMenuFrm(QWidget *parent = nullptr);
    ~HomeMenuFrm();
    
    // Method to handle card clicks
    void handleCardClicked(int cardIndex, const QString &title);

private slots:
    void slotManagingPeopleClicked();
    void slotRecordsManagementClicked();
    void slotNetworkSetupClicked();
    void slotSrvSetupClicked();
    void slotSysSetupClicked();
    void slotIdentifySetupClicked();

signals:
    void sigShowFrm(const QString &frmName);

private:
    QScopedPointer<HomeMenuFrmPrivate> d_ptr;
    
#ifdef SCREENCAPTURE
    void mouseDoubleClickEvent(QMouseEvent*);        
#endif    

private:
    Q_DECLARE_PRIVATE(HomeMenuFrm)
    Q_DISABLE_COPY(HomeMenuFrm)
};

#endif // HOMEMENUFRM_H