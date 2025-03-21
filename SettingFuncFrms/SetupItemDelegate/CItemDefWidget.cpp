#include "CItemDefWidget.h"

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QApplication>
#include <QDesktopWidget>

class CItemDefWidgetPrivate
{
    Q_DECLARE_PUBLIC(CItemDefWidget)
public:
    CItemDefWidgetPrivate(CItemDefWidget *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QLabel *m_pLeftPicLabel;
    QLabel *m_pNameLabel;
    QLabel *m_pRightPicLabel;
private:
    CItemDefWidget *const q_ptr;
};

CItemDefWidgetPrivate::CItemDefWidgetPrivate(CItemDefWidget *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

CItemDefWidget::CItemDefWidget(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new CItemDefWidgetPrivate(this))
{

}

CItemDefWidget::~CItemDefWidget()
{

}

void CItemDefWidgetPrivate::InitUI()
{
    m_pLeftPicLabel = new QLabel;
    m_pNameLabel = new QLabel;
    m_pRightPicLabel = new QLabel;

    QHBoxLayout *layout = new QHBoxLayout(q_func());
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addSpacing(15);
    layout->addWidget(m_pLeftPicLabel);
    //layout->addSpacing(10);
    layout->addWidget(m_pNameLabel);
    layout->addStretch();
    layout->addWidget(m_pRightPicLabel);
    int width = QApplication::desktop()->screenGeometry().width();
	if (width==480)
	  layout->addSpacing(20);//120,40
    else 
      layout->addSpacing(15);
}

void CItemDefWidgetPrivate::InitData()
{

}

void CItemDefWidgetPrivate::InitConnect()
{

}

void CItemDefWidget::setData(const QString &Name, const QString &qstrPic)
{
    Q_D(CItemDefWidget);
    d->m_pLeftPicLabel->setPixmap(QPixmap(qstrPic));
    d->m_pNameLabel->setText(Name);
    d->m_pRightPicLabel->setPixmap(QPixmap(":/Images/ListWidgetTmpRight.png"));
}

QString CItemDefWidget::getNameText() const
{
    return d_func()->m_pNameLabel->text();
}
