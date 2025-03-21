#ifndef DOORLOCKFRM_H
#define DOORLOCKFRM_H

#include <QDialog>


//登陆密码修改
class DoorLockFrmPrivate;
class DoorLockFrm : public QDialog
{
    Q_OBJECT
public:
    explicit DoorLockFrm(QWidget *parent = nullptr);
    ~DoorLockFrm();
private:
    void showEvent(QShowEvent *event);
    void paintEvent(QPaintEvent *);
    void setInputValue(int index);
    QString m_pPwdInputValue;
      
private slots:
    //按键处理
    void btn_clicked();    
private:
    QScopedPointer<DoorLockFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture     
    void mouseDoubleClickEvent(QMouseEvent*);   
#endif            
private:
    Q_DECLARE_PRIVATE(DoorLockFrm)
    Q_DISABLE_COPY(DoorLockFrm)
};

#endif // DOORLOCKFRM_H
