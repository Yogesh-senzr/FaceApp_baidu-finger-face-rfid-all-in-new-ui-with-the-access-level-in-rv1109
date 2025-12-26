// Updated SysSetupFrm.h - Card-based theme support

#ifndef SYSSETUPFRM_H
#define SYSSETUPFRM_H

#include "SettingFuncFrms/SettingBaseFrm.h"

class QListWidgetItem;
class SysSetupFrmPrivate;

class SysSetupFrm : public SettingBaseFrm
{
    Q_OBJECT
public:
    explicit SysSetupFrm(QWidget *parent = nullptr);
    ~SysSetupFrm();
    
    // Method to handle card clicks
    void handleCardClicked(int cardIndex);

    void performFirmwareUpdate();
    void downloadFirmwareFile(const QString &url, const QString &destPath);
    void continueAfterDownload();
    void showUpdateProgress(const QString &message);

private:
    virtual void setLeaveEvent();
    virtual void setEnter();
    
private slots:
    void slotIemClicked(QListWidgetItem *item); // Keep for compatibility

signals:
    void sigShowFrm(const QString &frmName);

private:
    QScopedPointer<SysSetupFrmPrivate> d_ptr;
    
#ifdef SCREENCAPTURE
    void mouseDoubleClickEvent(QMouseEvent*);        
#endif    

private:
    Q_DECLARE_PRIVATE(SysSetupFrm)
    Q_DISABLE_COPY(SysSetupFrm)
};

#endif // SYSSETUPFRM_H