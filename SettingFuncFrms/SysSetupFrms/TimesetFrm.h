#ifndef TimesetFrm_H
#define TimesetFrm_H

#include "SettingFuncFrms/SettingBaseFrm.h"

//时间设置
class TimesetFrmPrivate;
class TimesetFrm : public SettingBaseFrm
{
    Q_OBJECT
public:
    explicit TimesetFrm(QWidget *parent = nullptr);
    ~TimesetFrm();
private:
    virtual void setEnter();//进入
    virtual void setLeaveEvent();//退出
private:
    Q_SLOT void slotAlterBtn();
private:
    QScopedPointer<TimesetFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture     
    void mouseDoubleClickEvent(QMouseEvent*);  
#endif        
private:
    Q_DECLARE_PRIVATE(TimesetFrm)
    Q_DISABLE_COPY(TimesetFrm)
};

#endif // TimesetFrm_H
