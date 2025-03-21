#include "WifiViewFrm.h"
#include "../SetupItemDelegate/CItemWifiBoxWidget.h"
#include "../SetupItemDelegate/CItemWifiWidget.h"

#ifdef Q_OS_LINUX
#include "RkNetWork/NetworkControlThread.h"
#endif
#include "InputWifiPasswordFrm.h"
#include "ConfirmRestartFrm.h"
#include "Config/ReadConfig.h"
#include "Helper/myhelper.h"
#include "MessageHandler/Log.h"

#include <QListWidget>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QApplication>
#include <QScreen>
#include <QDesktopWidget>
#include <QtConcurrent/QtConcurrent>
#include <QScrollBar>


#if 0 
 1.列表 30 行
 未连接状态,连接前
 1---29 列表
 连接后
 已连接在第一位,其它后延
#endif 
class WifiViewFrmPrivate
{
    Q_DECLARE_PUBLIC(WifiViewFrm)
public:
    WifiViewFrmPrivate(WifiViewFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    void setHideRow(const int index);
private:
    QListWidget *m_pListWidget;
private:
    WifiViewFrm *const q_ptr;
    bool ExecOnce;              //是否首次执行，用于处理首次进入此页面且wifi已连接时的情况
    QString CurrentWiFi;        //当前正连上的WiFi
};

WifiViewFrmPrivate::WifiViewFrmPrivate(WifiViewFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

WifiViewFrm::WifiViewFrm(QWidget *parent)
    : SettingBaseFrm(parent)
    , d_ptr(new WifiViewFrmPrivate(this))
{
}

WifiViewFrm::~WifiViewFrm()
{

}

void WifiViewFrmPrivate::InitUI()
{
    m_pListWidget = new QListWidget;
    
    QHBoxLayout *layout = new QHBoxLayout(q_func());
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(m_pListWidget);
}

void WifiViewFrmPrivate::InitData()
{
    int width = QApplication::desktop()->screenGeometry().width();
    int spacing = 0;
    switch(width)
    {
    case 480:spacing = 0;break;		
    case 600:spacing = 10;break;
    case 720:spacing = 66;break;
    case 800:spacing = 10;break;
    }
	printf(">>>%s,%s,%d,\n",__FILE__,__func__,__LINE__);
    m_pListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pListWidget->setContextMenuPolicy(Qt::NoContextMenu);

    //QScrollBar *mScrollbar = m_pListWidget->verticalScrollBar();
    QScrollBar *mScrollbar = m_pListWidget->horizontalScrollBar();

    mScrollbar->setContextMenuPolicy(Qt::NoContextMenu);
   // ((QScrollBar *)m_pListWidget->verticalScrollBar())->setContextMenuPolicy(Qt::NoContextMenu);
    m_pListWidget->horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    QListWidgetItem *pItem = new QListWidgetItem;
    CItemWifiBoxWidget *pItemWidget = new CItemWifiBoxWidget;
    pItemWidget->setAddSpacing(spacing);
    pItemWidget->setAddSpacing(10);

    pItemWidget->setData("Wi-Fi");
	//if (width ==480)	 
	//	pItemWidget->setAddSpacing(120);//120,60,80,100
    m_pListWidget->addItem(pItem);
    m_pListWidget->setItemWidget(pItem, pItemWidget);

    if  (ReadConfig::GetInstance()->getNetwork_Manager_Mode() ==2 )
      pItemWidget->setWifiState(true);
    else 
      pItemWidget->setWifiState(false);
#if 1
//foreach (QWidget *w, qApp->topLevelWidgets())
     foreach(QObject *widget, qApp->allWidgets())
    {
        //QDialog *dlg = (QDialog *)w;
        //    dlg->done(1);
        //QScrollBar *scrollBar = dynamic_cast<QScrollBar*>(widget);
        QScrollBar *scrollBar = (QScrollBar *)widget;
        if(scrollBar)
        {
            scrollBar->setContextMenuPolicy(Qt::NoContextMenu);
        }
    }
#endif 
    QObject::connect(pItemWidget, &CItemWifiBoxWidget::sigWifiSwitchState, q_func(), &WifiViewFrm::slotWifiSwitchState);
    QObject::connect(pItemWidget, &CItemWifiBoxWidget::sigRestartTakeEffect, q_func(),&WifiViewFrm::slotSaveAndRestart);

    for(int i = 0; i<30; i++)
    {//动态创建支持30个WIFI查找例表
        QListWidgetItem *pItem = new QListWidgetItem;
        CItemWifiWidget *pItemWidget2 = new CItemWifiWidget;
        pItemWidget2->setData("WIFI", 0);
        m_pListWidget->addItem(pItem);
        m_pListWidget->setItemWidget(pItem, pItemWidget2);
        m_pListWidget->setRowHidden(i + 1, true);
    }
	if (ReadConfig::GetInstance()->getNetwork_Manager_Mode() !=2)
	{
		for(int i = 1; i<30; i++)
	    {
	        m_pListWidget->setRowHidden(i, true);
	    }
	}
}

void WifiViewFrmPrivate::InitConnect()
{
    QObject::connect(m_pListWidget, &QListWidget::itemDoubleClicked, q_func(), &WifiViewFrm::slotIemClicked);
#ifdef Q_OS_LINUX
    QObject::connect(NetworkControlThread::GetInstance(), &NetworkControlThread::sigWifiList, q_func(), &WifiViewFrm::slotWifiList);
#endif
}

void WifiViewFrmPrivate::setHideRow(const int index)
{
    for(int i = index; i<30; i++)
    {
        m_pListWidget->setRowHidden(i, true);
    }
}

void WifiViewFrm::setEnter()
{//进入
    Q_D(WifiViewFrm);//打开wifi扫描
    CItemWifiBoxWidget *pItemWidget = (CItemWifiBoxWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(0));
    pItemWidget->setWifiState((ReadConfig::GetInstance()->getNetwork_Manager_Mode() == 2) ? true : false);
    d->ExecOnce = false;
    d->CurrentWiFi.clear();
#ifdef Q_OS_LINUX
    if(pItemWidget->getWifiState())
    {        
        NetworkControlThread::GetInstance()->setWifiSearchMode(1);
        NetworkControlThread::GetInstance()->resume();
    }
#endif
}

void WifiViewFrm::setLeaveEvent()
{//退出
    Q_D(WifiViewFrm);
#ifdef Q_OS_LINUX
    NetworkControlThread::GetInstance()->pause();
#endif
    foreach (QWidget *w, qApp->topLevelWidgets()) {
        if(w->objectName() == "InputWifiPassword")
        {//如果存在顶层用户未关闭的窗口， 使其不可见
            QDialog *dlg = (QDialog *)w;
            dlg->done(1);
        }
    }
    d->setHideRow(1);
    //记录当前wifi开启状态
    CItemWifiBoxWidget *pItemWidget = (CItemWifiBoxWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(0));
    ReadConfig::GetInstance()->setNetwork_Manager_Mode(pItemWidget->getWifiState() ? 2 : 1);
    QtConcurrent::run(ReadConfig::GetInstance(), &ReadConfig::setSaveConfig);
    LogD("%s,%s,%d,getNetwork_Manager_Mode=%d\n",__FILE__,__func__,__LINE__,ReadConfig::GetInstance()->getNetwork_Manager_Mode());
	//myHelper::Utils_Reboot();
}

bool SubListSort(QVariant &inf1, QVariant &inf2)
{
    return inf1.toJsonObject().value("Strength").toDouble()>inf2.toJsonObject().value("Strength").toDouble();
}

void WifiViewFrm::slotWifiList(const QList<QVariant>obj)
{
    Q_D(WifiViewFrm);
    //QString wifiname = ReadConfig::GetInstance()->getWIFI_Name();
    int cnt = qMin(obj.count(), 29);
#if 0    
    for(int i = 0; i<cnt; i++)
    {
        const auto &jsObj = obj.at(i).toJsonObject();
        CItemWifiWidget *pItemWidget = (CItemWifiWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(i + 1));
        QString name = jsObj.value("sName").toString();
    //if (ssid.length()>0)
	 // LogD("%s,%s,%d,ssid=%s\n",__FILE__,__func__,__LINE__,ssid.toStdString().c_str());

        pItemWidget->setData(jsObj.value("sService").toString(), name, jsObj.value("Strength").toDouble()/100.f);
        d->m_pListWidget->setRowHidden(i + 1, false);
    }
#endif 
    for(int i = 0; i<cnt; i++)
    {
        const auto &jsObj = obj.at(i).toJsonObject();
        CItemWifiWidget *pItemWidget = (CItemWifiWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(i + 1));
        QString name = jsObj.value("sName").toString();
        if (name.length()>0) {
            LogD("%s,%s,%d,ssid=%s\n",__FILE__,__func__,__LINE__,name.toStdString().c_str());
            pItemWidget->setData(jsObj.value("sService").toString(), name, jsObj.value("Strength").toDouble()/100.f);
            d->m_pListWidget->setRowHidden(i + 1, false);
            //如果有已连接的WiFi，根据情况决定是否弹出重启弹框
            if(name.endsWith(QObject::tr("\nConnected"))){
                //判断是否是新连上的wifi
                QString wifiname = name;
                wifiname.chop(QObject::tr("\nConnected").length());
                // LogD("%s,%s,%d,name=%s\n",__FILE__,__func__,__LINE__,name.toStdString().c_str());
                // LogD("%s,%s,%d,wifiname=%s\n",__FILE__,__func__,__LINE__,wifiname.toStdString().c_str());
                if(wifiname.compare(d->CurrentWiFi)){
                    d->CurrentWiFi = wifiname;
                    //弹窗
                    if(d->ExecOnce){
                        ConfirmRestartFrm dlg(this);
                        dlg.setSizeGripEnabled(true);
                        dlg.setModal(true);
                        dlg.show();
                        if(dlg.exec() == 0){
                            slotSaveAndRestart();
                        }
                    }
                    
                }
            }
            if(name.endsWith(QObject::tr("\nConnecting"))){
                d->CurrentWiFi = name;
                d->CurrentWiFi.chop(QObject::tr("\nConnecting").length());
            }
        }
    }
    d->ExecOnce = true;
    if(cnt)d->setHideRow(cnt + 1);
}

void WifiViewFrm::slotWifiSwitchState(const int state)
{
    Q_UNUSED(state);
    Q_D(WifiViewFrm);
#ifdef Q_OS_LINUX
    //NetworkControlThread::GetInstance()->setNetworkType(state ? 2 : 1);
    LogD(">>>%s,%s,%d,state=%d\n",__FILE__,__func__,__LINE__,state);    
    if (state) 
    {
       NetworkControlThread::GetInstance()->setNetworkType(2);
       //system("ifconfig p2p0 down");
       ReadConfig::GetInstance()->setNetwork_Manager_Mode(2);        
       QtConcurrent::run(ReadConfig::GetInstance(), &ReadConfig::setSaveConfig);
	   NetworkControlThread::GetInstance()->setWifiSearchMode(1);
       NetworkControlThread::GetInstance()->resume();
    
	//if (ReadConfig::GetInstance()->getNetwork_Manager_Mode() ==2) 
	  //NetworkControlThread::GetInstance()->setNetworkType(2);
    //if(state)NetworkControlThread::GetInstance()->resume();
	//if (ReadConfig::GetInstance()->getNetwork_Manager_Mode() ==2) {
	   //NetworkControlThread::GetInstance()->resume();

	   
	    int cnt = 29; //28       
	    for(int i = 0; i<cnt; i++)
	    {
	        CItemWifiWidget *pItemWidget = (CItemWifiWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(i + 1));
            d->m_pListWidget->setRowHidden(i + 1, false);
	    }	   
	   
	}
    else
	{
       NetworkControlThread::GetInstance()->setNetworkType(1); 
       ReadConfig::GetInstance()->setNetwork_Manager_Mode(1);
       //system("ifconfig p2p0 down");
	   NetworkControlThread::GetInstance()->pause();
	    int cnt = 29;//28
	    for(int i = 0; i<cnt; i++)
	    {
	       // const auto &jsObj = obj.at(i).toJsonObject();
	        CItemWifiWidget *pItemWidget = (CItemWifiWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(i + 1));
	        //QString name = jsObj.value("sName").toString();
	       // pItemWidget->setData(jsObj.value("sService").toString(), name, jsObj.value("Strength").toInt()/100.f);
	        d->m_pListWidget->setRowHidden(i + 1, true);
	    }	 
        d->CurrentWiFi.clear();  
	}
#endif
    d->setHideRow(1);
}

void WifiViewFrm::slotIemClicked(QListWidgetItem *item)
{
    Q_D(WifiViewFrm);
#ifdef SCREENCAPTURE  //ScreenCapture        
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");     
#endif     
    if(item == d->m_pListWidget->item(0))return;
#if 1
    CItemWifiWidget *pItemWidget = (CItemWifiWidget *)d->m_pListWidget->itemWidget(item);
    InputWifiPasswordFrm dlg(this);
    dlg.setData(pItemWidget->getServiceText(), pItemWidget->getSSIDText());
    dlg.show();
    if(dlg.exec() == 0)
    {
        ReadConfig::GetInstance()->setWIFI_Name(dlg.getWifiName());
        ReadConfig::GetInstance()->setWIFI_Password(dlg.getPassword());
#ifdef Q_OS_LINUX
        NetworkControlThread::GetInstance()->DisconnectAllWifi();
        NetworkControlThread::GetInstance()->setLinkWlanSSID(dlg.getWifiName(), dlg.getPassword());
        NetworkControlThread::GetInstance()->setLinkWlan(pItemWidget->getServiceText(), dlg.getPassword());
#endif
    }
#else
    NetworkControlThread::GetInstance()->getCurrentWifiName();
#endif
}
#ifdef SCREENCAPTURE  //ScreenCapture    
void WifiViewFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif 

void WifiViewFrm::slotSaveAndRestart()
{
    Q_D(WifiViewFrm);
    //记录当前wifi开启状态
    NetworkControlThread::GetInstance()->setNetworkType( 2);            
    CItemWifiBoxWidget *pItemWidget = (CItemWifiBoxWidget *)d->m_pListWidget->itemWidget(d->m_pListWidget->item(0));
    ReadConfig::GetInstance()->setNetwork_Manager_Mode(pItemWidget->getWifiState() ? 2 : 1);
    //QtConcurrent::run(ReadConfig::GetInstance(), &ReadConfig::setSaveConfig)
    //即将关机，用线程的话并不能保证关机之前此操作能完成，所以改为直接调用
    ReadConfig::GetInstance()->setSaveConfig();
    LogD(">>>%s,%s,%d,getNetwork_Manager_Mode=%d\n",__FILE__,__func__,__LINE__,ReadConfig::GetInstance()->getNetwork_Manager_Mode());
    myHelper::Utils_Reboot();
}