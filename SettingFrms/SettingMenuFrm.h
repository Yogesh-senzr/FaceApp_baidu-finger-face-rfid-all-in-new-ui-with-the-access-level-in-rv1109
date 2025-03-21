#ifndef SETTINGMENUFRM_H
#define SETTINGMENUFRM_H

#include <QWidget>

//配置菜单框架
class SettingMenuFrmPrivate;
class SettingMenuFrm : public QWidget
{
    Q_OBJECT
public:
    explicit SettingMenuFrm(QWidget *parent = nullptr);
    ~SettingMenuFrm();
private:
    void paintEvent(QPaintEvent *event);
private:
    QScopedPointer<SettingMenuFrmPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(SettingMenuFrm)
    Q_DISABLE_COPY(SettingMenuFrm)
#ifdef SCREENCAPTURE  //ScreenCapture 
protected:
    void mouseDoubleClickEvent(QMouseEvent*);    
#endif     
};

#endif // SETTINGMENUFRM_H
