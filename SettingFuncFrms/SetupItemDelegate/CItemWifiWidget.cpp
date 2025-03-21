#include "CItemWifiWidget.h"
#include "MessageHandler/Log.h"

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QApplication>
#include <QDesktopWidget>



class CItemWifiWidgetPrivate
{
    Q_DECLARE_PUBLIC(CItemWifiWidget)
public:
    CItemWifiWidgetPrivate(CItemWifiWidget *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QLabel *m_pNameLabel;
    QLabel *m_pRightPicLabel;
private:
    QString mService;
private:
    CItemWifiWidget *const q_ptr;
};

CItemWifiWidgetPrivate::CItemWifiWidgetPrivate(CItemWifiWidget *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

CItemWifiWidget::CItemWifiWidget(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new CItemWifiWidgetPrivate(this))
{

}

CItemWifiWidget::~CItemWifiWidget()
{

}

void CItemWifiWidgetPrivate::InitUI()
{
    m_pNameLabel = new QLabel;
    m_pRightPicLabel = new QLabel;

    QHBoxLayout *layout = new QHBoxLayout(q_func());
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addSpacing(25);
    layout->addWidget(m_pNameLabel);
    layout->addStretch();
    layout->addWidget(m_pRightPicLabel);
    int width = QApplication::desktop()->screenGeometry().width();
	if (width==480)
	  layout->addSpacing(20);//120,40
	else 
      layout->addSpacing(10);
}

void CItemWifiWidgetPrivate::InitData()
{
    q_func()->setObjectName("CItemWidget");
}

void CItemWifiWidgetPrivate::InitConnect()
{

}

void CItemWifiWidget::setData(const QString &Service, const QString &ssid, const float &level)
{
    Q_D(CItemWifiWidget);
    d->mService = Service;
    //d->m_pNameLabel->setText(ssid.isEmpty() ? QObject::tr("TheHideAccessPoint") : ssid);//隐藏的网络
    d->m_pNameLabel->setText( ssid);
    if (ssid.length()>0)
	  LogD("%s,%s,%d,ssid=%s\n",__FILE__,__func__,__LINE__,ssid.toStdString().c_str());
	//
    //		if ((*iter)->strengthMax > 0)
    //		{
    //			level = (float) (*iter)->strength / (float) (*iter)->strengthMax;
    //		}
    if (level >= 0.8)
        d->m_pRightPicLabel->setPixmap(QPixmap(":/Images/wifi_strength_4.png"));
    else if (level >= 0.6)
        d->m_pRightPicLabel->setPixmap(QPixmap(":/Images/wifi_strength_3.png"));
    else if (level >= 0.4)
        d->m_pRightPicLabel->setPixmap(QPixmap(":/Images/wifi_strength_2.png"));
    else if (level >= 0.2)
        d->m_pRightPicLabel->setPixmap(QPixmap(":/Images/wifi_strength_1.png"));
    else
        d->m_pRightPicLabel->setPixmap(QPixmap(":/Images/wifi_strength_0.png"));
}

QString CItemWifiWidget::getSSIDText() const
{
    return d_func()->m_pNameLabel->text();
}

QString CItemWifiWidget::getServiceText() const
{
    return d_func()->mService;
}

void CItemWifiWidget::setAddSpacing(const int spcaing)
{
    ((QHBoxLayout *)this->layout())->addSpacing(spcaing);
}
