// Updated SrvSetupFrm.h - Card-based theme support

#ifndef SRVSETUPFRM_H
#define SRVSETUPFRM_H

#include "SettingFuncFrms/SettingBaseFrm.h"

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
    
private slots:
    void slotConfirmButton();
    void slotIemClicked(QListWidgetItem *item);

private:
    QScopedPointer<SrvSetupFrmPrivate> d_ptr;
    
#ifdef SCREENCAPTURE
    void mouseDoubleClickEvent(QMouseEvent*);        
#endif    

private:
    Q_DECLARE_PRIVATE(SrvSetupFrm)
    Q_DISABLE_COPY(SrvSetupFrm)
};

#endif // SRVSETUPFRM_H