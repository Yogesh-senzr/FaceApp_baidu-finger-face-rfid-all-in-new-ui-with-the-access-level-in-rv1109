#include "CItemWifiCheckBoxWidget.h"

#include <QLabel>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QApplication>
#include <QDesktopWidget>

class CItemWifiCheckBoxWidgetPrivate
{
    Q_DECLARE_PUBLIC(CItemWifiCheckBoxWidget)
public:
    CItemWifiCheckBoxWidgetPrivate(CItemWifiCheckBoxWidget *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QLabel *m_pNameLabel;
    QCheckBox *m_pWifiSwitchBox;
private:
    CItemWifiCheckBoxWidget *const q_ptr;
};

CItemWifiCheckBoxWidgetPrivate::CItemWifiCheckBoxWidgetPrivate(CItemWifiCheckBoxWidget *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

CItemWifiCheckBoxWidget::CItemWifiCheckBoxWidget(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new CItemWifiCheckBoxWidgetPrivate(this))
{

}

CItemWifiCheckBoxWidget::~CItemWifiCheckBoxWidget()
{

}

void CItemWifiCheckBoxWidgetPrivate::InitUI()
{
    m_pNameLabel = new QLabel;
    m_pWifiSwitchBox = new QCheckBox;

    QHBoxLayout *layout = new QHBoxLayout(q_func());
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addSpacing(15);
    layout->addWidget(m_pNameLabel);
    layout->addStretch();
    layout->addWidget(m_pWifiSwitchBox);
    int width = QApplication::desktop()->screenGeometry().width();
	if (width==480)
	  layout->addSpacing(20);//120,40
	else 
      layout->addSpacing(15);
}

void CItemWifiCheckBoxWidgetPrivate::InitData()
{
    m_pWifiSwitchBox->setObjectName("WifiSwitchBox");
}

void CItemWifiCheckBoxWidgetPrivate::InitConnect()
{
    QObject::connect(m_pWifiSwitchBox, &QCheckBox::stateChanged, q_func(), &CItemWifiCheckBoxWidget::sigWifiSwitchState);
}

void CItemWifiCheckBoxWidget::setData(const QString &Name)
{
    Q_D(CItemWifiCheckBoxWidget);
    d->m_pNameLabel->setText(Name);
}

bool CItemWifiCheckBoxWidget::getWifiState() const
{
    return d_func()->m_pWifiSwitchBox->checkState();
}
