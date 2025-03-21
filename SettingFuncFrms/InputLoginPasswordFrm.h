#ifndef InputLoginPasswordFrm_H
#define InputLoginPasswordFrm_H

#include <QDialog>

//登陆输入密码
class InputLoginPasswordFrmPrivate;
class InputLoginPasswordFrm : public QDialog
{
    Q_OBJECT
public:
    explicit InputLoginPasswordFrm(QWidget *parent = nullptr);
    ~InputLoginPasswordFrm();
public:
    QString getInputValue()const;
private:
    void showEvent(QShowEvent *);
private:
    QScopedPointer<InputLoginPasswordFrmPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(InputLoginPasswordFrm)
    Q_DISABLE_COPY(InputLoginPasswordFrm)
protected:
#ifdef SCREENCAPTURE  //ScreenCapture   
    void mouseDoubleClickEvent(QMouseEvent*);    
#endif    
};

#endif // InputLoginPasswordFrm_H
