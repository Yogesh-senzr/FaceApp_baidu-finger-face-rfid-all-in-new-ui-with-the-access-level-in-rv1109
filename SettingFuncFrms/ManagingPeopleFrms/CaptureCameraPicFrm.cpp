#include "CaptureCameraPicFrm.h"

#include <QPainter>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

class CaptureCameraPicFrmPrivate
{
    Q_DECLARE_PUBLIC(CaptureCameraPicFrm)
public:
    CaptureCameraPicFrmPrivate(CaptureCameraPicFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QLabel *m_pFacePicLabel;
    QLabel *m_pHintLabel;
    QPushButton *m_pCaptureFaceButton;
private:
    CaptureCameraPicFrm *const q_ptr;
};

CaptureCameraPicFrmPrivate::CaptureCameraPicFrmPrivate(CaptureCameraPicFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

CaptureCameraPicFrm::CaptureCameraPicFrm(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new CaptureCameraPicFrmPrivate(this))
{
/*    QPalette pal;
    pal.setColor(QPalette::Background, QColor(0,122,83,255));
    this->setAutoFillBackground(true);
    this->setPalette(pal)*/;
}

CaptureCameraPicFrm::~CaptureCameraPicFrm()
{

}

void CaptureCameraPicFrmPrivate::InitUI()
{
    m_pFacePicLabel = new QLabel;
    m_pHintLabel = new QLabel;
    m_pCaptureFaceButton = new QPushButton;

    QVBoxLayout *vlayout = new QVBoxLayout(q_func());
    vlayout->setSpacing(0);
    vlayout->setMargin(0);
    vlayout->addWidget(m_pFacePicLabel);
    vlayout->addSpacing(10);
    vlayout->addWidget(m_pHintLabel);
    vlayout->addStretch();
    vlayout->addWidget(m_pCaptureFaceButton);
}

QPixmap pixmapScale(const QPixmap& image, const int &width, const int &height)
{
        QPixmap r_image;
        r_image = image.scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        return r_image;
}


void CaptureCameraPicFrmPrivate::InitData()
{
    q_func()->setFixedWidth(300);

    m_pCaptureFaceButton->setFixedSize(q_func()->width(), 62);
    m_pCaptureFaceButton->setText(QObject::tr("人脸采集"));

    m_pHintLabel->setStyleSheet("font-size:22px;color:#d4237a");
    m_pHintLabel->setAlignment(Qt::AlignCenter);
    m_pHintLabel->setText(QObject::tr("请确保脸部正对摄像头"));
    m_pFacePicLabel->setFixedHeight(500);
    m_pFacePicLabel->setStyleSheet(" border: 1px solid rgb(4, 82, 177);");
    m_pFacePicLabel->setAlignment(Qt::AlignCenter);
    m_pFacePicLabel->setPixmap(QPixmap(":/Images/profilephoto.png"));
}

void CaptureCameraPicFrmPrivate::InitConnect()
{

}
#ifdef SCREENCAPTURE  //ScreenCapture 
void CaptureCameraPicFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif