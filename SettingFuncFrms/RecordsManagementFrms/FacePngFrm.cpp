#include "FacePngFrm.h"
#include <QPainter>


FacePngFrm::FacePngFrm(const QString &img_path, QWidget *parent)
    : QWidget(parent)
    , mFaceImg(QImage(img_path))
{
}

void FacePngFrm::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawImage(this->rect(), this->mFaceImg/*.scaled(this->width(),this->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation)*/);
    QWidget::paintEvent(event);
}
#ifdef SCREENCAPTURE  //ScreenCapture   
void FacePngFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif 