#include "DoorControFrm.h"
#include "../SetupItemDelegate/CItemWidget.h"

#include <QListWidget>
#include <QHBoxLayout>


class DoorControFrmPrivate
{
    Q_DECLARE_PUBLIC(DoorControFrm)
public:
    DoorControFrmPrivate(DoorControFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QListWidget *m_pListWidget;
private:
    DoorControFrm *const q_ptr;
};

DoorControFrmPrivate::DoorControFrmPrivate(DoorControFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

DoorControFrm::DoorControFrm(QWidget *parent)
    : SettingBaseFrm(parent)
    , d_ptr(new DoorControFrmPrivate(this))
{

}

DoorControFrm::~DoorControFrm()
{

}

void DoorControFrmPrivate::InitUI()
{
    m_pListWidget = new QListWidget;
    QHBoxLayout *layout = new QHBoxLayout(q_func());
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(m_pListWidget);
}

void DoorControFrmPrivate::InitData()
{
	int width = QApplication::desktop()->screenGeometry().width();
    m_pListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItem->setSizeHint(QSize(QApplication::desktop()->screenGeometry().width(), 90)); //800
        pItemWidget->setData(QObject::tr("AccessType"), ":/Images/SmallRound.png", QObject::tr("GateEntry"));//出入类型,入闸
        m_pListWidget->addItem(pItem);
		if (width==480)
			pItemWidget->setAddSpacing(140);		
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }

    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItem->setSizeHint(QSize(QApplication::desktop()->screenGeometry().width(), 90));//800
        pItemWidget->setData(QObject::tr("PassagePeriod"), ":/Images/SmallRound.png");//通行时段
        m_pListWidget->addItem(pItem);
		if (width==480)
			pItemWidget->setAddSpacing(140);		
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItem->setSizeHint(QSize(QApplication::desktop()->screenGeometry().width(), 90));//800
        pItemWidget->setData(QObject::tr("WiganFormat"), ":/Images/SmallRound.png", QObject::tr("26bit"));//韦根输出格式
        m_pListWidget->addItem(pItem);
		if (width==480)
			pItemWidget->setAddSpacing(140);		
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;
        pItem->setSizeHint(QSize(QApplication::desktop()->screenGeometry().width(), 90));//800
        pItemWidget->setData(QObject::tr("DoorOpeningDelay"), ":/Images/SmallRound.png", QObject::tr("100ms"));//门磁延时
        m_pListWidget->addItem(pItem);
		if (width==480)
			pItemWidget->setAddSpacing(140);
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }
}

void DoorControFrmPrivate::InitConnect()
{
    QObject::connect(m_pListWidget, &QListWidget::itemClicked, q_func(), &DoorControFrm::slotIemClicked);
}

void DoorControFrm::setEnter()
{

}

void DoorControFrm::setLeaveEvent()
{

}

void DoorControFrm::slotIemClicked(QListWidgetItem */*item*/)
{

}
#ifdef SCREENCAPTURE  //ScreenCapture 
void DoorControFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif 