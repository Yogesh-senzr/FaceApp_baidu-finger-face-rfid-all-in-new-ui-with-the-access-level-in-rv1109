#ifndef SystemMaintenanceFrm_H
#define SystemMaintenanceFrm_H

#include <QtWidgets/QDialog>

//系统维护
class SystemMaintenanceFrmPrivate;
class SystemMaintenanceFrm : public QDialog
{
    Q_OBJECT
public:
    explicit SystemMaintenanceFrm(QWidget *parent = nullptr);
    ~SystemMaintenanceFrm();
public:
    //设置数据
    void setData(const int &mode, const QString &time);
    int getTimeMode()const;//获取模式
    QString getTime()const;//获取时间
private:
    QScopedPointer<SystemMaintenanceFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture     
    void mouseDoubleClickEvent(QMouseEvent*);    
#endif     
private:
    Q_SLOT void slotConfirmButton();

private:
    Q_DECLARE_PRIVATE(SystemMaintenanceFrm)
    Q_DISABLE_COPY(SystemMaintenanceFrm)
};

#endif // SystemMaintenanceFrm_H
