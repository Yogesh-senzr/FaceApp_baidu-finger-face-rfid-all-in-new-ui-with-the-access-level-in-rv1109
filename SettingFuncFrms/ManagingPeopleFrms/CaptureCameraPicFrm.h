#ifndef CaptureCameraPicFrm_H
#define CaptureCameraPicFrm_H

#include <QWidget>

//显示用户抓拍相机人脸图
class CaptureCameraPicFrmPrivate;
class CaptureCameraPicFrm : public QWidget
{
    Q_OBJECT
public:
    explicit CaptureCameraPicFrm(QWidget *parent = nullptr);
    ~CaptureCameraPicFrm();
private:
    QScopedPointer<CaptureCameraPicFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture     
    void mouseDoubleClickEvent(QMouseEvent*);      
#endif    
private:
    Q_DECLARE_PRIVATE(CaptureCameraPicFrm)
    Q_DISABLE_COPY(CaptureCameraPicFrm)
};

#endif // CaptureCameraPicFrm_H
