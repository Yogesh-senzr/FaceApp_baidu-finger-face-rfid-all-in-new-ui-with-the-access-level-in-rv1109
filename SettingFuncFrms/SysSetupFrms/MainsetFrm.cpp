#include "MainsetFrm.h"
#include "../SetupItemDelegate/CItemWidget.h"
#include "../SetupItemDelegate/CItemBoxWidget.h"

#include "DisplayEffectFrm.h"
#include "Config/ReadConfig.h"

#include <QListWidget>
#include <QHBoxLayout>
#include <QApplication>
#include <QScreen>
#include <QDesktopWidget>

class MainsetFrmPrivate
{
    Q_DECLARE_PUBLIC(MainsetFrm)
public:
    MainsetFrmPrivate(MainsetFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QListWidget *m_pListWidget;
private:
    MainsetFrm *const q_ptr;
};

MainsetFrmPrivate::MainsetFrmPrivate(MainsetFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

MainsetFrm::MainsetFrm(QWidget *parent)
    : SettingBaseFrm(parent)
    , d_ptr(new MainsetFrmPrivate(this))
{

}

MainsetFrm::~MainsetFrm()
{

}

void MainsetFrmPrivate::InitUI()
{
    m_pListWidget = new QListWidget;
    QHBoxLayout *layout = new QHBoxLayout(q_func());
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(m_pListWidget);
}

void MainsetFrmPrivate::InitData()
{
    int width = QApplication::desktop()->screenGeometry().width();
    int spacing = 0;
    switch(width)
    {
    case 600:spacing = 27;break;
    case 720:spacing = 37;break;
    case 800:spacing = 0;break;
    }

    m_pListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //    {
    //        QListWidgetItem *pItem = new QListWidgetItem;
    //        CItemWidget *pItemWidget = new CItemWidget;

    //        pItemWidget->setData(QObject::tr("显示效果"));
    //        m_pListWidget->addItem(pItem);
    //        m_pListWidget->setItemWidget(pItem, pItemWidget);
    //    }
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemBoxWidget *pItemWidget = new CItemBoxWidget;
        pItemWidget->setAddSpacing(spacing);

        pItemWidget->setData(QObject::tr("EchoDeviceID"));//显示设备号
        m_pListWidget->addItem(pItem);
		if (width==480)
			pItemWidget->setAddSpacing(140);			
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemBoxWidget *pItemWidget = new CItemBoxWidget;
        pItemWidget->setAddSpacing(spacing);

        pItemWidget->setData(QObject::tr("EchoMacAddress"));//显示MAC地址
        m_pListWidget->addItem(pItem);
		if (width==480)
			pItemWidget->setAddSpacing(140);			
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemBoxWidget *pItemWidget = new CItemBoxWidget;
        pItemWidget->setAddSpacing(spacing);

        pItemWidget->setData(QObject::tr("EchoIpAddress"));//显示IP地址
        m_pListWidget->addItem(pItem);
		if (width==480)
			pItemWidget->setAddSpacing(140);			
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemBoxWidget *pItemWidget = new CItemBoxWidget;
        pItemWidget->setAddSpacing(spacing);

        pItemWidget->setData(QObject::tr("EchoRegistration"));//显示注册人数
        m_pListWidget->addItem(pItem);
		if (width==480)
			pItemWidget->setAddSpacing(140);			
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }
}

void MainsetFrmPrivate::InitConnect()
{
    //QObject::connect(m_pListWidget, &QListWidget::itemClicked, q_func(), &MainsetFrm::slotIemClicked);
}

void MainsetFrm::setEnter()
{
    Q_D(MainsetFrm);
    ((CItemBoxWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(0)))->setCheckBoxState(ReadConfig::GetInstance()->getHomeDisplay_DisplaySnNum());
    ((CItemBoxWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(1)))->setCheckBoxState(ReadConfig::GetInstance()->getHomeDisplay_DisplayMac());
    ((CItemBoxWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(2)))->setCheckBoxState(ReadConfig::GetInstance()->getHomeDisplay_DisplayIP());
    ((CItemBoxWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(3)))->setCheckBoxState(ReadConfig::GetInstance()->getHomeDisplay_DisplayPersonNum());   
}

void MainsetFrm::setLeaveEvent()
{
    Q_D(MainsetFrm);
#ifdef SCREENCAPTURE  //ScreenCapture     
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");       
#endif     
    ReadConfig::GetInstance()->setHomeDisplay_DisplaySnNum(((CItemBoxWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(0)))->getCheckBoxState() ? 1 : 0);
    ReadConfig::GetInstance()->setHomeDisplay_DisplayMac(((CItemBoxWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(1)))->getCheckBoxState() ? 1 : 0);
    ReadConfig::GetInstance()->setHomeDisplay_DisplayIP(((CItemBoxWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(2)))->getCheckBoxState() ? 1 : 0);
    ReadConfig::GetInstance()->setHomeDisplay_DisplayPersonNum(((CItemBoxWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(3)))->getCheckBoxState() ? 1 : 0);
}

void MainsetFrm::slotIemClicked(QListWidgetItem *item)
{
    Q_D(MainsetFrm);
      
    if(item == d->m_pListWidget->item(0))
    {//显示效果
        DisplayEffectFrm dlg(this);
        dlg.exec();
    }
}
#ifdef SCREENCAPTURE  //ScreenCapture 
void MainsetFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif 