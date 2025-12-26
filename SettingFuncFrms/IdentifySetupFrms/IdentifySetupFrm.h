// Updated IdentifySetupFrm.h - Card-based theme support

#ifndef IDENTIFYSETUPFRM_H
#define IDENTIFYSETUPFRM_H

#include "SettingFuncFrms/SettingBaseFrm.h"

class QListWidgetItem;
class IdentifySetupFrmPrivate;

class IdentifySetupFrm : public SettingBaseFrm
{
    Q_OBJECT
public:
    explicit IdentifySetupFrm(QWidget *parent = nullptr);
    ~IdentifySetupFrm();
    
    // Method to handle card clicks (replaces individual item handling)
    void handleCardClicked(int cardIndex);

private:
    virtual void setLeaveEvent();
    virtual void setEnter();
    
private slots:
    void slotIemClicked(QListWidgetItem *item); // Keep for compatibility

private:
    QScopedPointer<IdentifySetupFrmPrivate> d_ptr;
    
#ifdef SCREENCAPTURE
    void mouseDoubleClickEvent(QMouseEvent*);        
#endif    

private:
    Q_DECLARE_PRIVATE(IdentifySetupFrm)
    Q_DISABLE_COPY(IdentifySetupFrm)
};

#endif // IDENTIFYSETUPFRM_H