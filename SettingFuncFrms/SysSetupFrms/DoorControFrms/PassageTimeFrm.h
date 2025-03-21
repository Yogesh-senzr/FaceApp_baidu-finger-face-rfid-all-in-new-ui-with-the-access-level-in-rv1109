#ifndef PassageTimeFrm_H
#define PassageTimeFrm_H

#include "SettingFuncFrms/SettingBaseFrm.h"

//通行时段
class PassageTimeFrmPrivate;
class PassageTimeFrm : public SettingBaseFrm
{
    Q_OBJECT
public:
    explicit PassageTimeFrm(QWidget *parent = nullptr);
    ~PassageTimeFrm();
private:
    virtual void setEnter();//进入
    virtual void setLeaveEvent();//退出
private:
    Q_SLOT void slotAddTimeInterval();//增加时段
    Q_SLOT void slotDelTimeInterval();//删除时段
private:
    QScopedPointer<PassageTimeFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture     
    void mouseDoubleClickEvent(QMouseEvent*);  
#endif               
private:
    Q_DECLARE_PRIVATE(PassageTimeFrm)
    Q_DISABLE_COPY(PassageTimeFrm)
};

#endif // PassageTimeFrm_H
