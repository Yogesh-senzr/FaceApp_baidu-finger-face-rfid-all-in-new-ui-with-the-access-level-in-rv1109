#ifndef MainsetFrm_H
#define MainsetFrm_H

#include "SettingFuncFrms/SettingBaseFrm.h"

//主界面设置
class QListWidgetItem;
class MainsetFrmPrivate;
class MainsetFrm : public SettingBaseFrm
{
    Q_OBJECT
public:
    explicit MainsetFrm(QWidget *parent = nullptr);
    ~MainsetFrm();
private:
    virtual void setEnter();//进入
    virtual void setLeaveEvent();//退出
private:
    Q_SLOT void slotIemClicked(QListWidgetItem *item);
private:
    QScopedPointer<MainsetFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture     
    void mouseDoubleClickEvent(QMouseEvent*);    
#endif         
private:
    Q_DECLARE_PRIVATE(MainsetFrm)
    Q_DISABLE_COPY(MainsetFrm)
};

#endif // MainsetFrm_H
