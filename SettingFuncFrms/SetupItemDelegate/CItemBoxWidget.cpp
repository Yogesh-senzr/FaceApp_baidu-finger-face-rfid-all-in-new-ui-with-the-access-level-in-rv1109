#include "CItemBoxWidget.h"

#include <QLabel>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QApplication>
#include <QDesktopWidget>

class CItemBoxWidgetPrivate
{
    Q_DECLARE_PUBLIC(CItemBoxWidget)
public:
    CItemBoxWidgetPrivate(CItemBoxWidget *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QLabel *m_pLeftPicLabel;
    QLabel *m_pNameLabel;
    QCheckBox *m_pCheckBox;
private:
    CItemBoxWidget *const q_ptr;
};

CItemBoxWidgetPrivate::CItemBoxWidgetPrivate(CItemBoxWidget *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

CItemBoxWidget::CItemBoxWidget(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new CItemBoxWidgetPrivate(this))
{

}

CItemBoxWidget::~CItemBoxWidget()
{

}

void CItemBoxWidgetPrivate::InitUI()
{
    m_pLeftPicLabel = new QLabel;
    m_pNameLabel = new QLabel;
    m_pCheckBox = new QCheckBox;

    QHBoxLayout *layout = new QHBoxLayout(q_func());
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addSpacing(25);
    layout->addWidget(m_pLeftPicLabel);
    layout->addSpacing(10);
    layout->addWidget(m_pNameLabel);
    layout->addStretch();
    layout->addWidget(m_pCheckBox);
    int width = QApplication::desktop()->screenGeometry().width();
	if (width==480)
	  layout->addSpacing(20);//120,40
	else 
      layout->addSpacing(10);
}

void CItemBoxWidgetPrivate::InitData()
{
    m_pCheckBox->setObjectName("CItemFrmBox");
    q_func()->setObjectName("CItemWidget");
}

void CItemBoxWidgetPrivate::InitConnect()
{
     QObject::connect(m_pCheckBox, &QCheckBox::stateChanged, q_func(), &CItemBoxWidget::sigSwitchState);
}

void CItemBoxWidget::setCheckBoxState(const int &b)
{
    Q_D(CItemBoxWidget);
    d->m_pCheckBox->setChecked(b);
}

int CItemBoxWidget::getCheckBoxState() const
{
    return d_func()->m_pCheckBox->checkState();
}

void CItemBoxWidget::setData(const QString &Name, const QString &qstrPic)
{
    Q_D(CItemBoxWidget);
    if(!qstrPic.isEmpty())d->m_pLeftPicLabel->setPixmap(QPixmap(qstrPic));
    else d->m_pLeftPicLabel->hide();
    d->m_pNameLabel->setText(Name);
}

QString CItemBoxWidget::getNameText() const
{
    return d_func()->m_pNameLabel->text();
}

void CItemBoxWidget::setAddSpacing(const int spcaing)
{
    ((QHBoxLayout *)this->layout())->addSpacing(spcaing);
}

void CItemBoxWidget::mousePressEvent(QMouseEvent *event)
{
    Q_D(CItemBoxWidget);
    d->m_pCheckBox->setChecked(!d->m_pCheckBox->checkState());
    QWidget::mousePressEvent(event);
}
