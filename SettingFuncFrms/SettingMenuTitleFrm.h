#ifndef SETTINGMENUTITLEFRM_H
#define SETTINGMENUTITLEFRM_H

#include <QWidget>

//菜单选择项的标题UI
class SettingMenuTitleFrmPrivate;
class SettingMenuTitleFrm : public QWidget
{
    Q_OBJECT
public:
    explicit SettingMenuTitleFrm(QWidget *parent = nullptr);
    ~SettingMenuTitleFrm();
public:
    void setTitleText(const QString &);
    QString getTitleText();
public:
    Q_SIGNAL void sigReturnSuperiorClicked();
private:
    QScopedPointer<SettingMenuTitleFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture        
    void mouseDoubleClickEvent(QMouseEvent*);       
#endif     
private:
    Q_DECLARE_PRIVATE(SettingMenuTitleFrm)
    Q_DISABLE_COPY(SettingMenuTitleFrm)
};

#endif // SETTINGMENUTITLEFRM_H
