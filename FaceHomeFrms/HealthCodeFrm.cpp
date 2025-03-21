#include "HealthCodeFrm.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>

class HealthCodeFrmPrivate
{
    Q_DECLARE_PUBLIC(HealthCodeFrm)
public:
    HealthCodeFrmPrivate(HealthCodeFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QLabel *m_pLeftPngLabel;//身份证图像
    QLabel *m_pNameLabel;//姓名
    QLabel *m_pNumberLabel;//身份证编号
    QLabel *m_pAddressLabel;//地址
    QLabel *m_pHintLabel;//提示描述
    QLabel *m_pRightPngLabel;//健康图像
private:
    HealthCodeFrm *const q_ptr;
};

HealthCodeFrmPrivate::HealthCodeFrmPrivate(HealthCodeFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

HealthCodeFrm::HealthCodeFrm(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new HealthCodeFrmPrivate(this))
{
    QPalette pal;
    pal.setColor(QPalette::Background, QColor(0,0,0,120));
    this->setAutoFillBackground(true);
    this->setPalette(pal);
}

HealthCodeFrm::~HealthCodeFrm()
{

}

void HealthCodeFrmPrivate::InitUI()
{
    m_pLeftPngLabel = new QLabel;//身份证图像
    m_pNameLabel = new QLabel;//姓名
    m_pNumberLabel = new QLabel;//身份证编号
    m_pAddressLabel = new QLabel;//地址
    m_pHintLabel = new QLabel;//提示描述
    m_pRightPngLabel = new QLabel;//健康图像

    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->setContentsMargins(10, 15, 10, 15);
    vlayout->addStretch();
    vlayout->addWidget(m_pNameLabel);
    vlayout->addWidget(m_pNumberLabel);
    vlayout->addWidget(m_pAddressLabel);
    vlayout->addWidget(m_pHintLabel);
    vlayout->addStretch();

    QFrame *f = new QFrame;
    f->setLayout(vlayout);

    QHBoxLayout *hlayout = new QHBoxLayout(q_func());
    hlayout->setMargin(5);
    hlayout->addWidget(m_pLeftPngLabel);
    hlayout->addWidget(f);
    hlayout->addWidget(m_pRightPngLabel);
}

void HealthCodeFrmPrivate::InitData()
{
    m_pLeftPngLabel->setFixedSize(102, 126);
    m_pRightPngLabel->setFixedSize(102, 126);

    m_pNameLabel->setObjectName("HealthCodeFrmLabel");
    m_pNumberLabel->setObjectName("HealthCodeFrmLabel");
    m_pAddressLabel->setObjectName("HealthCodeFrmLabel");
    m_pHintLabel->setObjectName("HealthCodeFrmLabel");
#if 0
    m_pNameLabel->setText("林坚雄");
    m_pNumberLabel->setText("**************5890");
    m_pAddressLabel->setText("广东省*********************");
    m_pHintLabel->setText("绿码,核验通过");
    m_pLeftPngLabel->setPixmap(QPixmap("/mnt/user/photo.bmp"));
    m_pRightPngLabel->setPixmap(QPixmap(":/Images/QRcode_green.png").scaled(102, 126));
#endif
    m_pAddressLabel->hide();
    int width = QApplication::desktop()->screenGeometry().width();
    int height = QApplication::desktop()->screenGeometry().height();

    q_func()->setFixedSize(530, 142);
    q_func()->move((width - 530)/2, height/2 - q_func()->height()/2);//344
    q_func()->hide();
}

void HealthCodeFrmPrivate::InitConnect()
{

}

void HealthCodeFrm::setHealthCodeInfo(const int &type, const QString &Name, const QString &idCard, const int &qrCodeType, const QString &tip)
{
    Q_D(HealthCodeFrm);

    d->m_pNameLabel->setText(Name.isEmpty() ? QObject::tr("Unknown") : Name);//未知
    d->m_pNumberLabel->setText(idCard.isEmpty() ? QObject::tr("Unknown") : idCard);//未知
    d->m_pLeftPngLabel->setPixmap((type == 1) ? QPixmap(":/Images/Person.png").scaled(102, 126) : QPixmap("/mnt/user/photo.bmp"));

    switch(qrCodeType)
    {
    case 0:
    {
        d->m_pHintLabel->setText(tip + QObject::tr("verifyHint"));//核验通过,,be verified and qualified
        d->m_pRightPngLabel->setPixmap(QPixmap(":/Images/QRcode_green.png").scaled(102, 126));
    }
        break;
    case 1:
    {
        d->m_pHintLabel->setText(tip + QObject::tr("verifyHint"));//",核验通过"
        d->m_pRightPngLabel->setPixmap(QPixmap(":/Images/QRcode_yellow.png").scaled(102, 126));
    }
        break;
    case 2:
    case 10:
    {
        d->m_pHintLabel->setText(tip + QObject::tr("verifyHint"));//",核验通过"
        d->m_pRightPngLabel->setPixmap(QPixmap(":/Images/QRcode_red.png").scaled(102, 126));
    }
        break;
    default:
    {
        d->m_pHintLabel->setText(tip);
        d->m_pRightPngLabel->setPixmap(QPixmap(":/Images/QRcode_grey.png").scaled(102, 126));
    }
        break;
    }
    this->show();
}
