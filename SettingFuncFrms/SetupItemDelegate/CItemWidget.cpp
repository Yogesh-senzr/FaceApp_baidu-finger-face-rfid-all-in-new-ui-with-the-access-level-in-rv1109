#include "CItemWidget.h"

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QApplication>
#include <QDesktopWidget>


class CItemWidgetPrivate
{
    Q_DECLARE_PUBLIC(CItemWidget)
public:
    CItemWidgetPrivate(CItemWidget *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QLabel *m_pLeftPicLabel;
    QLabel *m_pNameLabel;

    QLabel *m_pRightNameLabel;
    QLabel *m_pRightPicLabel;
private:
    CItemWidget *const q_ptr;
};

CItemWidgetPrivate::CItemWidgetPrivate(CItemWidget *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

CItemWidget::CItemWidget(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new CItemWidgetPrivate(this))
{

}

CItemWidget::~CItemWidget()
{

}

void CItemWidgetPrivate::InitUI()
{
    m_pLeftPicLabel = new QLabel;
    m_pNameLabel = new QLabel;
    m_pRightPicLabel = new QLabel;
    m_pRightNameLabel = new QLabel;

    QHBoxLayout *layout = new QHBoxLayout(q_func());
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addSpacing(25);
    layout->addWidget(m_pLeftPicLabel);
    layout->addSpacing(10);
    layout->addWidget(m_pNameLabel);
    layout->addStretch();
    layout->addWidget(m_pRightNameLabel);
    layout->addWidget(m_pRightPicLabel);
    int width = QApplication::desktop()->screenGeometry().width();
	if (width==480)
	  layout->addSpacing(20);//120,40,140
	else 
      layout->addSpacing(25);//10

}

void CItemWidgetPrivate::InitData()
{
    q_func()->setObjectName("CItemWidget");
}

void CItemWidgetPrivate::InitConnect()
{

}

void CItemWidget::setData(const QString &Name, const QString &qstrPic, const QString &text)
{
    Q_D(CItemWidget);
    if(!qstrPic.isEmpty())d->m_pLeftPicLabel->setPixmap(QPixmap(qstrPic));
    else d->m_pLeftPicLabel->hide();
    d->m_pNameLabel->setText(Name);
    d->m_pRightNameLabel->setText(text);
    d->m_pRightPicLabel->setPixmap(QPixmap(":/Images/ListWidgetTmpRight.png"));
}

void CItemWidget::setHideRPng()
{
    Q_D(CItemWidget);
    d->m_pRightPicLabel->hide();
}

void CItemWidget::setRNameText(const QString &text)
{
    Q_D(CItemWidget);
    d->m_pRightNameLabel->setText(text);
}

QString CItemWidget::getNameText() const
{
    return d_func()->m_pNameLabel->text();
}

QString CItemWidget::getRNameText() const
{
    return d_func()->m_pRightNameLabel->text();
}

void CItemWidget::setAddSpacing(const int spacing)
{
    ((QHBoxLayout *)this->layout())->addSpacing(spacing);
}
