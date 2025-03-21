#include "FaceHomeBottomFrm.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QHBoxLayout>
#include <QtCore/QDateTime>
#include <QtWidgets/QStyleOption>
#include <QtGui/QPainter>

class FaceHomeBottomFrmPrivate
{
    Q_DECLARE_PUBLIC(FaceHomeBottomFrm)
public:
    FaceHomeBottomFrmPrivate(FaceHomeBottomFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QLabel *m_pEsslLabel;
    QLabel *m_pSNLabel;//设备号
    QLabel *m_pMacLabel;//mac
    QLabel *m_pIPLabel;//ip
    QLabel *m_pPeopleLabel;//人数
private:
    FaceHomeBottomFrm *m_FaceHomeBottomFrm;
private:
    FaceHomeBottomFrm *const q_ptr;
};


FaceHomeBottomFrmPrivate::FaceHomeBottomFrmPrivate(FaceHomeBottomFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

FaceHomeBottomFrm::FaceHomeBottomFrm(QWidget *parent)
    : HomeBottomBaseFrm(parent)
    , d_ptr(new FaceHomeBottomFrmPrivate(this))
{

}

FaceHomeBottomFrm::~FaceHomeBottomFrm()
{

}

void FaceHomeBottomFrmPrivate::InitUI()
{
    m_pEsslLabel = new QLabel;
    m_pSNLabel = new QLabel;;//设备号
    m_pMacLabel = new QLabel;//mac
    m_pIPLabel = new QLabel;//ip
    m_pPeopleLabel = new QLabel;//人数

    QVBoxLayout *vlayout = new QVBoxLayout(q_func());
    vlayout->addStretch();
    vlayout->setContentsMargins(20, 0, 0, 5);
    vlayout->setSpacing(5);
    vlayout->addWidget(m_pEsslLabel);
    vlayout->addWidget(m_pPeopleLabel);
    vlayout->addWidget(m_pIPLabel);
    vlayout->addWidget(m_pMacLabel);
    vlayout->addWidget(m_pSNLabel);
}

void FaceHomeBottomFrmPrivate::InitData()
{
    m_pEsslLabel->setObjectName("FaceHomeBottomLabel");
    m_pSNLabel->setObjectName("FaceHomeBottomLabel");
    m_pMacLabel->setObjectName("FaceHomeBottomLabel");
    m_pIPLabel->setObjectName("FaceHomeBottomLabel");
    m_pPeopleLabel->setObjectName("FaceHomeBottomLabel");

    m_pEsslLabel->setText("eSSL");
    m_pSNLabel->setText(QObject::tr("SN:%1"));//QObject::tr("SN码:")
    m_pMacLabel->setText(QObject::tr("MAC:%1:%2:%3:%4:%5:%6"));//QObject::tr("MAC:")
    m_pIPLabel->setText(QObject::tr("IP:%1"));//QObject::tr("IP:127.0.0.1")
    m_pPeopleLabel->setText(QObject::tr("real-name registration:%1/people"));//QObject::tr("实名制人员:0/人")

    q_func()->setFixedHeight(110);
}

void FaceHomeBottomFrmPrivate::InitConnect()
{

}

void FaceHomeBottomFrm::setHomeDisplay_SnNum(const int &show)
{
    Q_D(FaceHomeBottomFrm);
    d->m_pSNLabel->setVisible(show);
}

void FaceHomeBottomFrm::setHomeDisplay_Mac(const int &show)
{
    Q_D(FaceHomeBottomFrm);
    d->m_pMacLabel->setVisible(show);
}

void FaceHomeBottomFrm::setHomeDisplay_IP(const int &show)
{
    Q_D(FaceHomeBottomFrm);
    d->m_pIPLabel->setVisible(show);
}

void FaceHomeBottomFrm::setHomeDisplay_PersonNum(const int &show)
{
    Q_D(FaceHomeBottomFrm);
    d->m_pPeopleLabel->setVisible(show);
}

void FaceHomeBottomFrm::setPersonNum(const int &num)
{
    Q_D(FaceHomeBottomFrm);
#if 0
    if(num<=9) d->m_pPeopleLabel->setText(QObject::tr("实名制人员:00%1/人").arg(num));
    else if(num>=10 && num<=99)d->m_pPeopleLabel->setText(QObject::tr("实名制人员:0%1/人").arg(num));
    else d->m_pPeopleLabel->setText(QObject::tr("实名制人员:%1/人").arg(num));
#else
    //d->m_pPeopleLabel->setText(QObject::tr("实名制人员:") + QString::asprintf("%0.3d", num) + QObject::tr("/人"));
    d->m_pPeopleLabel->setText(QObject::tr("real-name registration:%1/people").arg(num) );
#endif
}

void FaceHomeBottomFrm::setNetInfo(const QString &address, const QString &make)
{
    Q_D(FaceHomeBottomFrm);
    d->m_pMacLabel->setText(QObject::tr("MAC:%1:%2:%3:%4:%5:%6").arg(make.mid(0,2)).arg(make.mid(2,2)).arg(make.mid(4,2)).arg(make.mid(6,2)).arg(make.mid(8,2)).arg(make.mid(10,2)));
    d->m_pIPLabel->setText(QObject::tr("IP:%1").arg(address));
}

void FaceHomeBottomFrm::setSNNUm(const QString &sn)
{
    Q_D(FaceHomeBottomFrm);
    //d->m_pSNLabel->setText(QObject::tr("SN码:%1").arg(sn));
    d->m_pSNLabel->setText(QObject::tr("SN:%1").arg(sn));
}

void FaceHomeBottomFrm::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QStyleOption opt;
    opt.init(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
    QWidget::paintEvent(event);
}
