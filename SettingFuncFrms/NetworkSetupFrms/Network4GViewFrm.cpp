#include "Network4GViewFrm.h"
#include "Config/ReadConfig.h"
#include "MessageHandler/Log.h"
#include "Helper/myhelper.h"
#include "../SetupItemDelegate/CItemWifiBoxWidget.h"
#include "../SetupItemDelegate/CItemWifiWidget.h"
#include "OperationTipsFrm.h"

#ifdef Q_OS_LINUX
#include "RkNetWork/NetworkControlThread.h"
#endif
#include "../SetupItemDelegate/CItemBoxWidget.h"

#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QApplication>
#include <QtGui/QScreen>
#include <QtWidgets/QDesktopWidget>

class Network4GViewFrmPrivate
{
    Q_DECLARE_PUBLIC(Network4GViewFrm)
public:
    Network4GViewFrmPrivate(Network4GViewFrm *dd);
private:
    void InitUI();

private:
    QListWidget *m_pListWidget;
    //CItemWifiBoxWidget *m_pItemWidget;
    CItemBoxWidget *m_pItemWidget;
private:
    Network4GViewFrm *const q_ptr;
};

Network4GViewFrmPrivate::Network4GViewFrmPrivate(Network4GViewFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
}

void Network4GViewFrmPrivate::InitUI()
{
    m_pListWidget = new QListWidget;
    QHBoxLayout *layout = new QHBoxLayout(q_func());
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(m_pListWidget);


    int width = QApplication::desktop()->screenGeometry().width();
    int spacing = 0;
    switch(width)
    {
    case 480:spacing = 10;break;		
    case 600:spacing = 10;break;
    case 720:spacing = 66;break;
    case 800:spacing = 10;break;
    }

    m_pListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QListWidgetItem *pItem = new QListWidgetItem;
    //m_pItemWidget = new CItemWifiBoxWidget;
    m_pItemWidget = new CItemBoxWidget;
    m_pItemWidget->setAddSpacing(spacing);
    m_pItemWidget->setAddSpacing(10);
    //m_pItemWidget->setState((ReadConfig::GetInstance()->getNetwork_Manager_Mode() == 3) ? true : false);
    //(CItemBoxWidget *)
    m_pItemWidget->setCheckBoxState((ReadConfig::GetInstance()->getNetwork_Manager_Mode() == 3) ? true : false); 

    m_pItemWidget->setData("4G");
    m_pListWidget->addItem(pItem);
    m_pListWidget->setItemWidget(pItem, m_pItemWidget);
	if (width ==480)
		m_pItemWidget->setAddSpacing(120);
    //QObject::connect(m_pItemWidget, &CItemWifiBoxWidget::sigWifiSwitchState, q_func(), &Network4GViewFrm::slotNetwork4GSwitchState);
    QObject::connect(m_pItemWidget, &CItemBoxWidget::sigSwitchState, q_func(), &Network4GViewFrm::slotNetwork4GSwitchState);
}

Network4GViewFrm::Network4GViewFrm(QWidget *parent)
    : SettingBaseFrm(parent)
    , d_ptr(new Network4GViewFrmPrivate(this))
{
}

Network4GViewFrm::~Network4GViewFrm()
{
}

void Network4GViewFrm::slotNetwork4GSwitchState(const int state)
{
	LogD("%s %s[%d] state %d visible %d \n",__FILE__,__FUNCTION__,__LINE__,state,this->isVisible());
	if(state == 2 && this->isVisible())
	{
        OperationTipsFrm dlg(this);
        //还原所有设置,还原所有设置后所有配置信息将被清除,是否继续进行该操作？
        int ret = dlg.setMessageBox(QObject::tr("Reboot"), "");
	    if (ret == 0)
	    {
	    	ReadConfig::GetInstance()->setNetwork_Manager_Mode(3);
	    	ReadConfig::GetInstance()->setSaveConfig();
            LogD("%s %s[%d] Mode=%d  \n",__FILE__,__FUNCTION__,__LINE__,ReadConfig::GetInstance()->getNetwork_Manager_Mode());
	    	myHelper::Utils_Reboot();
	    }
	    emit sigShowLevelFrm();
	}
}

void Network4GViewFrm::setEnter()
{//进入
    Q_D(Network4GViewFrm);//打开wifi扫描
    //d->m_pItemWidget->setWifiState((ReadConfig::GetInstance()->getNetwork_Manager_Mode() == 3) ? true : false);
	d->m_pItemWidget->setCheckBoxState((ReadConfig::GetInstance()->getNetwork_Manager_Mode() == 3) ? true : false);
}

void Network4GViewFrm::setLeaveEvent()
{//退出
}
#ifdef SCREENCAPTURE  //ScreenCapture    
void Network4GViewFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif