#ifndef SettingShowTmpFrm_H
#define SettingShowTmpFrm_H

#include <QWidget>

//配置功能显示模板UI
class SettingShowTmpFrmPrivate;
class SettingShowTmpFrm : public QWidget
{
    Q_OBJECT
public:
    explicit SettingShowTmpFrm(QWidget *parent = nullptr);
    ~SettingShowTmpFrm();
public:
    void setData(const QString &Name, const QString &qstrPic);
private:
    QScopedPointer<SettingShowTmpFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture       
    void mouseDoubleClickEvent(QMouseEvent*);      
#endif    
private:
    Q_DECLARE_PRIVATE(SettingShowTmpFrm)
    Q_DISABLE_COPY(SettingShowTmpFrm)
};

#endif // SettingShowTmpFrm_H
