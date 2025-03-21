#ifndef SettingEditTextFrm_H
#define SettingEditTextFrm_H

#include <QDialog>

//配置功能显示模板UI
class SettingEditTextFrmPrivate;
class SettingEditTextFrm : public QDialog
{
    Q_OBJECT
public:
    explicit SettingEditTextFrm(QWidget *parent = nullptr);
    ~SettingEditTextFrm();
public:
    void setTitle(const QString &Name);
    void setData(const QString &Name);
private:
    QScopedPointer<SettingEditTextFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture       
    void mouseDoubleClickEvent(QMouseEvent*);      
#endif    
private:
    Q_DECLARE_PRIVATE(SettingEditTextFrm)
    Q_DISABLE_COPY(SettingEditTextFrm)
};

#endif // SettingEditTextFrm_H
