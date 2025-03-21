#ifndef SETTINGBASEFRM_H
#define SETTINGBASEFRM_H

#include <QWidget>

//所有配置功能基类
class SettingBaseFrm : public QWidget
{
    Q_OBJECT
public:
    explicit SettingBaseFrm(QWidget *parent = nullptr);
public:
    virtual void setEnter(){}//进入
    virtual void setLeaveEvent(){}//退出
public://发送显示窗口名称
    Q_SIGNAL void sigShowFrm(const QString name);
    Q_SIGNAL void sigShowLevelFrm();
private:
    bool event(QEvent *event);
#ifdef SCREENCAPTURE  //ScreenCapture     
    void mouseDoubleClickEvent(QMouseEvent*);            
#endif    
};

#endif // SETTINGBASEFRM_H
