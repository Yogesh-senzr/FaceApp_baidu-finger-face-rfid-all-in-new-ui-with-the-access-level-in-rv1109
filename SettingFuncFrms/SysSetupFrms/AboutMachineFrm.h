// Updated AboutMachineFrm.h - Card-based theme support

#ifndef ABOUTMACHINEFRM_H
#define ABOUTMACHINEFRM_H

#include "SettingFuncFrms/SettingBaseFrm.h"

class QListWidgetItem;
class AboutMachineFrmPrivate;

class AboutMachineFrm : public SettingBaseFrm
{
    Q_OBJECT
public:
    explicit AboutMachineFrm(QWidget *parent = nullptr);
    ~AboutMachineFrm();
    
    // Method to handle card clicks
    void handleCardClicked(int cardIndex);

private:
    virtual void setLeaveEvent();
    virtual void setEnter();
    
private slots:
    void slotIemClicked(QListWidgetItem *item); // Keep for compatibility

private:
    QScopedPointer<AboutMachineFrmPrivate> d_ptr;
    
#ifdef SCREENCAPTURE
    void mouseDoubleClickEvent(QMouseEvent*);        
#endif    

private:
    Q_DECLARE_PRIVATE(AboutMachineFrm)
    Q_DISABLE_COPY(AboutMachineFrm)
};

#endif // ABOUTMACHINEFRM_H