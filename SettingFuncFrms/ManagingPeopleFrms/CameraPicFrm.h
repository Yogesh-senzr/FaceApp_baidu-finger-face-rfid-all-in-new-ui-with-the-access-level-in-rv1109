#ifndef CAMERAPICFRM_H
#define CAMERAPICFRM_H

#include "SettingFuncFrms/SettingBaseFrm.h"

//实时显示相同内容
class CameraPicFrmPrivate;
class CameraPicFrm : public SettingBaseFrm
{
    Q_OBJECT
public:
    explicit CameraPicFrm(QWidget *parent = nullptr);
    ~CameraPicFrm();
public:
    void setShowImage(const QImage &img);
    QImage getImage()const;
    bool getImgisNull()const;
private:
    void paintEvent(QPaintEvent *event);
private:
    QScopedPointer<CameraPicFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture     
    void mouseDoubleClickEvent(QMouseEvent*);      
#endif     
private:
    Q_DECLARE_PRIVATE(CameraPicFrm)
    Q_DISABLE_COPY(CameraPicFrm)
};

#endif // CAMERAPICFRM_H
