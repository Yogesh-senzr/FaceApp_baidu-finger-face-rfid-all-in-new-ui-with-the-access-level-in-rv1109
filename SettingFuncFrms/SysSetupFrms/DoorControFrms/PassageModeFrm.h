#ifndef PASSAGEMODEFRM_H
#define PASSAGEMODEFRM_H

#include "SettingFuncFrms/SettingBaseFrm.h"

//开门方式
class PassageModeFrmPrivate;
class PassageModeFrm : public SettingBaseFrm
{
    Q_OBJECT
public:
    explicit PassageModeFrm(QWidget *parent = nullptr);
    ~PassageModeFrm();
public:
    bool getOpenIsEmpty();//获取开门模式是否为空
private:
    virtual void setEnter();//进入
    virtual void setLeaveEvent();//退出
private:
    Q_SLOT void slotMustCheckBox(const int &);
    Q_SLOT void slotSaveButton();
private:
    QScopedPointer<PassageModeFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture     
    void mouseDoubleClickEvent(QMouseEvent*);            
#endif     
private:
    Q_DECLARE_PRIVATE(PassageModeFrm)
    Q_DISABLE_COPY(PassageModeFrm)
};

#endif // PASSAGEMODEFRM_H
