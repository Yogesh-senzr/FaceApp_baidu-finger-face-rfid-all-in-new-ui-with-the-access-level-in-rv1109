#ifndef DoorControFrm_H
#define DoorControFrm_H

#include "SettingFuncFrms/SettingBaseFrm.h"

//门禁设置
class QListWidgetItem;
class DoorControFrmPrivate;
class DoorControFrm : public SettingBaseFrm
{
    Q_OBJECT
public:
    explicit DoorControFrm(QWidget *parent = nullptr);
    ~DoorControFrm();
private:
    virtual void setEnter();//进入
    virtual void setLeaveEvent();//退出
private:
    Q_SLOT void slotIemClicked(QListWidgetItem *item);
private:
    QScopedPointer<DoorControFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture     
    void mouseDoubleClickEvent(QMouseEvent*);         
#endif     
private:
    Q_DECLARE_PRIVATE(DoorControFrm)
    Q_DISABLE_COPY(DoorControFrm)
};

#endif // DoorControFrm_H
