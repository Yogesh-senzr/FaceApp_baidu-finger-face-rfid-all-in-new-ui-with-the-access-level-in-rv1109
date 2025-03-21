#ifndef LogPasswordChangeFrm_H
#define LogPasswordChangeFrm_H

#include <QDialog>

//登陆密码修改
class LogPasswordChangeFrmPrivate;
class LogPasswordChangeFrm : public QDialog
{
    Q_OBJECT
public:
    explicit LogPasswordChangeFrm(QWidget *parent = nullptr);
    ~LogPasswordChangeFrm();
public:
    QString getOldPasw();
    QString getNewPasw();
    QString getConfirmNewPasw();
private:
    void showEvent(QShowEvent *event);
private:
    QScopedPointer<LogPasswordChangeFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture     
    void mouseDoubleClickEvent(QMouseEvent*);  
#endif          
private:
    Q_DECLARE_PRIVATE(LogPasswordChangeFrm)
    Q_DISABLE_COPY(LogPasswordChangeFrm)
};

#endif // LogPasswordChangeFrm_H
