#ifndef DEVICEPOWEROFFFRM_H
#define DEVICEPOWEROFFFRM_H

#include <QDialog>

class DevicePoweroffFrmPrivate;
class DevicePoweroffFrm : public QDialog
{
    Q_OBJECT
public:
    DevicePoweroffFrm(QWidget *parent = Q_NULLPTR);
    ~DevicePoweroffFrm();
private:
    QScopedPointer<DevicePoweroffFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture     
    void mouseDoubleClickEvent(QMouseEvent*);       
#endif    
private:
    Q_DECLARE_PRIVATE(DevicePoweroffFrm)
    Q_DISABLE_COPY(DevicePoweroffFrm)
};

#endif // DEVICEPOWEROFFFRM_H
