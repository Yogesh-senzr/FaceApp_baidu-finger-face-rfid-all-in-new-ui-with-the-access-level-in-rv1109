#ifndef InputWifiPasswordFrm_H
#define InputWifiPasswordFrm_H

#include <QDialog>

//连接WIFI输入密码
class InputWifiPasswordFrmPrivate;
class InputWifiPasswordFrm : public QDialog
{
    Q_OBJECT
public:
    explicit InputWifiPasswordFrm(QWidget *parent = nullptr);
    ~InputWifiPasswordFrm();
public:
    void setData(const QString &Service, const QString &Name);
    QString getPassword();
    QString getWifiName()const;
public:
private:
    QScopedPointer<InputWifiPasswordFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture       
    void mouseDoubleClickEvent(QMouseEvent*);         
#endif    
private:
    Q_DECLARE_PRIVATE(InputWifiPasswordFrm)
    Q_DISABLE_COPY(InputWifiPasswordFrm)
};

#endif // InputWifiPasswordFrm_H
