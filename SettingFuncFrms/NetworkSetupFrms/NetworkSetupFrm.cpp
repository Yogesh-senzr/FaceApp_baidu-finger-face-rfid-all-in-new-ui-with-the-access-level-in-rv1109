#include "NetworkSetupFrm.h"

#include "../SetupItemDelegate/CItemWidget.h"
#include "Config/ReadConfig.h"

#include "../SetupItemDelegate/CItemBoxWidget.h"
#ifdef Q_OS_LINUX
#include "RkNetWork/NetworkControlThread.h"
#endif
#include "MessageHandler/Log.h"
#include "RkNetWork/Network4GControlThread.h"

#include <fcntl.h>
#include <unistd.h>
#include <QListWidget>
#include <QHBoxLayout>
#include <QtConcurrent/QtConcurrent>

class NetworkSetupFrmPrivate
{
    Q_DECLARE_PUBLIC(NetworkSetupFrm)
public:
    NetworkSetupFrmPrivate(NetworkSetupFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QListWidget *m_pListWidget;
private:
    NetworkSetupFrm *const q_ptr;
};

NetworkSetupFrmPrivate::NetworkSetupFrmPrivate(NetworkSetupFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

NetworkSetupFrm::NetworkSetupFrm(QWidget *parent)
    : SettingBaseFrm(parent)
    , d_ptr(new NetworkSetupFrmPrivate(this))
{

}

NetworkSetupFrm::~NetworkSetupFrm()
{

}

void NetworkSetupFrmPrivate::InitUI()
{
    m_pListWidget = new QListWidget;
    QHBoxLayout *layout = new QHBoxLayout(q_func());
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(m_pListWidget);
}

void NetworkSetupFrmPrivate::InitData()
{
    QStringList listName;
    if(!access("/sys/class/net/wlan0/",F_OK))
    {
    	listName<<QObject::tr("Wi-FiSetup"); //Wi-Fi设置
    }
    listName<<QObject::tr("EthernetSetup"); //网口设置
#if 1
    //if(!access("/sys/class/net/p2p0/",F_OK) || !access("/sys/devices/virtual/net/ppp0/",F_OK))
    {
    	listName<<QObject::tr("4G"); //4G设置
    }
#endif     

    for(int i = 0; i<listName.count(); i++)
    {
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWidget *pItemWidget = new CItemWidget;;
        pItemWidget->setData(listName.at(i), ":/Images/SmallRound.png");
        m_pListWidget->addItem(pItem);
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    }
#if 0    
    {    
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemBoxWidget *pItemWidget = new CItemBoxWidget;
        pItemWidget->setAddSpacing(0);
        pItemWidget->setAddSpacing(12);
        pItemWidget->setData(QObject::tr("4G"), ":/Images/SmallRound.png");//4G设置
        m_pListWidget->addItem(pItem);
        m_pListWidget->setItemWidget(pItem, pItemWidget);
    } 
#endif          
}

void NetworkSetupFrmPrivate::InitConnect()
{
    QObject::connect(m_pListWidget, &QListWidget::itemClicked, q_func(), &NetworkSetupFrm::slotIemClicked);
}

void NetworkSetupFrm::setLeaveEvent()
{
    QtConcurrent::run(ReadConfig::GetInstance(), &ReadConfig::setSaveConfig);
}

void NetworkSetupFrm::setEnter()
{
   Q_D(NetworkSetupFrm);    
#if 0  
   d->m_pListWidget->clear();
   d->InitData();
   //d->InitConnect();   
#endif    
}

void NetworkSetupFrm::slotIemClicked(QListWidgetItem *item)
{
    Q_D(NetworkSetupFrm);

#ifdef SCREENCAPTURE  //ScreenCapture     
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");        
#endif    
    CItemWidget *pItem = (CItemWidget *)d->m_pListWidget->itemWidget(item);


#if 0   
    if (pItem->getNameText()=="4G" )
    {//开发者模式　
	    	ReadConfig::GetInstance()->setNetwork_Manager_Mode(3);
	    	ReadConfig::GetInstance()->setSaveConfig();
            NetworkControlThread::GetInstance()->setNetworkType(3);

            Network4GControlThread::GetInstance();
            LogD("%s %s[%d] Mode=%d  \n",__FILE__,__FUNCTION__,__LINE__,ReadConfig::GetInstance()->getNetwork_Manager_Mode());
    } else 
#endif     
    {
        CItemWidget *pItem = (CItemWidget *)d->m_pListWidget->itemWidget(item);
        emit sigShowFrm(pItem->getNameText());
    }
}
#ifdef SCREENCAPTURE  //ScreenCapture    
void NetworkSetupFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif