#include "CameraPicFrm.h"

#include <QPainter>
#include <QDebug>

class CameraPicFrmPrivate
{
    Q_DECLARE_PUBLIC(CameraPicFrm)
public:
    CameraPicFrmPrivate(CameraPicFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QImage mFaceImg;
private:
    CameraPicFrm *const q_ptr;
};

QPixmap pixmapScale(const QPixmap& image, const int &width, const int &height)
{
    QPixmap r_image;
    r_image = image.scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    return r_image;
}

CameraPicFrmPrivate::CameraPicFrmPrivate(CameraPicFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

CameraPicFrm::CameraPicFrm(QWidget *parent)
    : SettingBaseFrm(parent)
    , d_ptr(new CameraPicFrmPrivate(this))
{
    //    QPalette pal;
    //    pal.setColor(QPalette::Background, QColor(111,175,239,255));
    //    this->setAutoFillBackground(true);
    //    this->setPalette(pal);
}

CameraPicFrm::~CameraPicFrm()
{

}

void CameraPicFrmPrivate::InitUI()
{

}

void CameraPicFrmPrivate::InitData()
{
}

void CameraPicFrmPrivate::InitConnect()
{

}

void CameraPicFrm::setShowImage(const QImage &img)
{
    Q_D(CameraPicFrm);
    d->mFaceImg = img;
    //qDebug()<<__FILE__<<__FUNCTION__<<__LINE__<<"img"<<img.isNull();
    this->update();

}

QImage CameraPicFrm::getImage() const
{
    return d_func()->mFaceImg;
}

bool CameraPicFrm::getImgisNull() const
{
    return d_func()->mFaceImg.isNull();
}

void CameraPicFrm::paintEvent(QPaintEvent *event)
{
    Q_D(CameraPicFrm);

    if(!d->mFaceImg.isNull())
    {
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing /*| QPainter::SmoothPixmapTransform*/);
        painter.drawImage(0,0, d->mFaceImg.scaled(this->width(), this->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
#ifdef SCREENCAPTURE  //ScreenCapture
    QPixmap map(this->width(), this->height());
    map.fill(Qt::transparent);
    painter.begin(&map);
    painter.drawRect(QRect(0, 0, 100, 100)); //Draw
    painter.end();
      
    map.save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png"); //Save
#endif     

    }
    QWidget::paintEvent(event);

}
#ifdef SCREENCAPTURE  //ScreenCapture  
void CameraPicFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif