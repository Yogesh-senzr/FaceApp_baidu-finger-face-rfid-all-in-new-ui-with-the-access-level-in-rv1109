#ifndef SETTINGMENUFRM_H
#define SETTINGMENUFRM_H

#include <QWidget>

//配置菜单框架UI
class SettingMenuFrmPrivate;
class SettingMenuFrm : public QWidget
{
    Q_OBJECT
public:
    explicit SettingMenuFrm(QWidget *parent = nullptr);
    ~SettingMenuFrm();
public:
    void setShowMenuFrm();
public://显示人脸主界面
    Q_SIGNAL void sigShowFaceHomeFrm(const int index = 0);
public:
    static inline SettingMenuFrm *GetInstance(){static SettingMenuFrm g;return &g;}   
private:
    //显示窗口名称
    Q_SLOT void slotShowFrm(const QString name);
private://标题返回按钮
    Q_SLOT void slotReturnSuperiorClicked();
private:
    void paintEvent(QPaintEvent *event);
private:
    QScopedPointer<SettingMenuFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture      
    void mouseDoubleClickEvent(QMouseEvent*);        
#endif     
private:
    Q_DECLARE_PRIVATE(SettingMenuFrm)
    Q_DISABLE_COPY(SettingMenuFrm)
};

#endif // SETTINGMENUFRM_H
