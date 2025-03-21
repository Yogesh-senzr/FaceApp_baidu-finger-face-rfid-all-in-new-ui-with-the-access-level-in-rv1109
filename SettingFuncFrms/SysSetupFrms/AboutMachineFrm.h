#ifndef AboutMachineFrm_H
#define AboutMachineFrm_H

#include "SettingFuncFrms/SettingBaseFrm.h"

//关于设备
class QListWidgetItem;
class AboutMachineFrmPrivate;
class AboutMachineFrm : public SettingBaseFrm
{
    Q_OBJECT
public:
    explicit AboutMachineFrm(QWidget *parent = nullptr);
    ~AboutMachineFrm();
private:
    virtual void setEnter();//进入
    virtual void setLeaveEvent();//退出    
private:
    Q_SLOT void slotIemClicked(QListWidgetItem *item);
private:
    QScopedPointer<AboutMachineFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture        
    void mouseDoubleClickEvent(QMouseEvent*);         
#endif     
private:
    Q_DECLARE_PRIVATE(AboutMachineFrm)
    Q_DISABLE_COPY(AboutMachineFrm)
};

#endif // AboutMachineFrm_H
