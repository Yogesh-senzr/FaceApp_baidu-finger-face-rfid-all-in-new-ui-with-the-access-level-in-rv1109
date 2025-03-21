#include "SettingMenuFrm.h"
#include "SettingMenuTitleFrm.h"

#include "SettingFuncFrms/HomeMenuFrm.h"
#include "SettingFuncFrms/ManagingPeopleFrms/ManagingPeopleFrm.h"
#include "SettingFuncFrms/ManagingPeopleFrms/ImportUserFrm.h"
#include "SettingFuncFrms/ManagingPeopleFrms/UserViewFrm.h"

#include "SettingFuncFrms/RecordsManagementFrms/RecordsManagementFrm.h"
#include "SettingFuncFrms/RecordsManagementFrms/ViewAccessRecordsFrm.h"

#include "SettingFuncFrms/NetworkSetupFrms/NetworkSetupFrm.h"
#include "SettingFuncFrms/NetworkSetupFrms/EthernetViewFrm.h"
#include "SettingFuncFrms/NetworkSetupFrms/WifiViewFrm.h"
#include "SettingFuncFrms/NetworkSetupFrms/Network4GViewFrm.h"

#include "SettingFuncFrms/SrvSetupFrms/SrvSetupFrm.h"
#include "SettingFuncFrms/SysSetupFrms/SysSetupFrm.h"
#include "SettingFuncFrms/SysSetupFrms/AboutMachineFrm.h"
#include "SettingFuncFrms/SysSetupFrms/DoorControFrms/DoorControFrm.h"
#include "SettingFuncFrms/SysSetupFrms/DoorControFrms/PassageModeFrm.h"
#include "SettingFuncFrms/SysSetupFrms/MainsetFrm.h"
#include "SettingFuncFrms/SysSetupFrms/TimesetFrm.h"
#include "SettingFuncFrms/SysSetupFrms/DoorControFrms/PassageTimeFrm.h"
#include "SettingFuncFrms/SysSetupFrms/StorageCapacityFrm.h"

#include "SettingFuncFrms/IdentifySetupFrms/IdentifySetupFrm.h"
#include "SettingFuncFrms/SysSetupFrms/LanguageFrm.h"

#include "OperationTipsFrm.h"
#include <fcntl.h>
#include <unistd.h>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QPainter>
#include <QStyleOption>
#include <QPalette>
#include <QMap>
#include <QQueue>
#include <QLineEdit>

class SettingMenuFrmPrivate
{
    Q_DECLARE_PUBLIC(SettingMenuFrm)
public:
    SettingMenuFrmPrivate(SettingMenuFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    HomeMenuFrm *m_pHomeMenuFrm;
private://人员管理
    ManagingPeopleFrm *m_pManagingPeopleFrm;
    ImportUserFrm *m_pImportUserFrm;
    UserViewFrm *m_pUserViewFrm;
private://记录管理
    RecordsManagementFrm *m_pRecordsManagementFrm;
    ViewAccessRecordsFrm *m_pViewAccessRecordsFrm;
private:
    NetworkSetupFrm *m_pNetworkSetupFrm;
    EthernetViewFrm *m_pEthernetViewFrm;
    WifiViewFrm *m_pWifiViewFrm;
    Network4GViewFrm *m_pNetwork4GViewFrm;
private:
    SrvSetupFrm *m_pSrvSetupFrm; 
private:
    SysSetupFrm *m_pSysSetupFrm;
    AboutMachineFrm *m_pAboutMachineFrm;
    DoorControFrm *m_pDoorControFrm;
    MainsetFrm *m_pMainsetFrm;
    TimesetFrm *m_pTimesetFrm;
    PassageTimeFrm *m_pPassageTimeFrm;
    PassageModeFrm *m_pPassageModeFrm;
    StorageCapacityFrm *m_pStorageCapacityFrm;
private:
    IdentifySetupFrm *m_pIdentifySetupFrm;
private:
    SettingMenuTitleFrm *m_pSettingMenuTitleFrm;
    QStackedWidget *m_pStackedWidget;
private:
    QMap<QString, QWidget *>mObjMap;
    QQueue<QWidget *>mOldObj;//记录显示的窗口
private:
    SettingMenuFrm *const q_ptr;
};


SettingMenuFrmPrivate::SettingMenuFrmPrivate(SettingMenuFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

SettingMenuFrm::SettingMenuFrm(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new SettingMenuFrmPrivate(this))
{
}

SettingMenuFrm::~SettingMenuFrm()
{

}

void SettingMenuFrmPrivate::InitUI()
{
    m_pSettingMenuTitleFrm = new SettingMenuTitleFrm;
    m_pStackedWidget = new QStackedWidget;

    QVBoxLayout *malayout = new QVBoxLayout(q_func());
    malayout->setMargin(0);
    malayout->setSpacing(0);
    malayout->addWidget(m_pSettingMenuTitleFrm);
    malayout->addWidget(m_pStackedWidget);

    m_pHomeMenuFrm = new HomeMenuFrm;
    m_pManagingPeopleFrm = new ManagingPeopleFrm;
    m_pImportUserFrm = new ImportUserFrm;
    m_pUserViewFrm= new UserViewFrm;

    m_pRecordsManagementFrm = new RecordsManagementFrm;
    m_pViewAccessRecordsFrm = new ViewAccessRecordsFrm;

    m_pNetworkSetupFrm = new NetworkSetupFrm;
    m_pEthernetViewFrm = new EthernetViewFrm;
    if(!access("/sys/class/net/wlan0/",F_OK))
    {
    	m_pWifiViewFrm = new WifiViewFrm;
    }
    //if(!access("/sys/class/net/p2p0/",F_OK) || !access("/sys/devices/virtual/net/ppp0/",F_OK))
    {
    	m_pNetwork4GViewFrm = new Network4GViewFrm;
    }

    m_pSrvSetupFrm = new SrvSetupFrm;

    m_pSysSetupFrm = new SysSetupFrm;
    m_pAboutMachineFrm = new AboutMachineFrm;
    m_pDoorControFrm= new DoorControFrm;
    m_pMainsetFrm = new MainsetFrm;
    m_pTimesetFrm = new TimesetFrm;
    m_pPassageTimeFrm = new PassageTimeFrm;
    m_pPassageModeFrm = new PassageModeFrm;
    m_pStorageCapacityFrm = new StorageCapacityFrm;

    m_pIdentifySetupFrm = new IdentifySetupFrm;

    m_pStackedWidget->addWidget(m_pHomeMenuFrm);
    m_pStackedWidget->addWidget(m_pManagingPeopleFrm);
    m_pStackedWidget->addWidget(m_pImportUserFrm);
    m_pStackedWidget->addWidget(m_pUserViewFrm);

    m_pStackedWidget->addWidget(m_pRecordsManagementFrm);
    m_pStackedWidget->addWidget(m_pViewAccessRecordsFrm);

    m_pStackedWidget->addWidget(m_pNetworkSetupFrm);
    m_pStackedWidget->addWidget(m_pEthernetViewFrm);
    if(!access("/sys/class/net/wlan0/",F_OK))
    {
    	m_pStackedWidget->addWidget(m_pWifiViewFrm);
    }
    //if(!access("/sys/class/net/p2p0/",F_OK))
    {
    	m_pStackedWidget->addWidget(m_pNetwork4GViewFrm);
    }

    m_pStackedWidget->addWidget(m_pSrvSetupFrm);
    m_pStackedWidget->addWidget(m_pSysSetupFrm);
    m_pStackedWidget->addWidget(m_pAboutMachineFrm);
    m_pStackedWidget->addWidget(m_pDoorControFrm);
    m_pStackedWidget->addWidget(m_pMainsetFrm);
    m_pStackedWidget->addWidget(m_pTimesetFrm);
    m_pStackedWidget->addWidget(m_pPassageTimeFrm);
    m_pStackedWidget->addWidget(m_pPassageModeFrm);
    m_pStackedWidget->addWidget(m_pStorageCapacityFrm);

    m_pStackedWidget->addWidget( m_pIdentifySetupFrm);
}

void traversalControl(const QObjectList& q)
{
    for (int i=0; i<q.length();i++)
     {
         QObject* item =q.at(i);
 
         if (item->inherits("QLineEdit"))
         {
             QLineEdit *bItem = qobject_cast<QLineEdit*>(item); 
             bItem->setContextMenuPolicy(Qt::NoContextMenu);
         }
     #if 0   
        if (!q.at(i)->children().empty())
            traversalControl(q.at(i)->children());
        else{
            QObject* item =q.at(i);
            qDebug()<<"line:"<<__LINE__<<",QObjectList:"<<q<<" QObject:"<<item;            
            if (item->inherits("QLineEdit"))//QLineEdit
            {
                QLineEdit *bItem = qobject_cast<QLineEdit*>(item);                
                bItem->setContextMenuPolicy(Qt::NoContextMenu);
                qDebug()<<"QObjectList:"<<q<<" QLineEdit:"<<bItem;
            }
        }
       #endif  
         
    }

}

void SettingMenuFrmPrivate::InitData()
{
    q_func()->setObjectName("NotTransparencyFrm");

    mOldObj.append(m_pHomeMenuFrm);
    mObjMap.insert(QObject::tr("Menu"), m_pHomeMenuFrm);//菜单
    mObjMap.insert(QObject::tr("PersonManaging"), m_pManagingPeopleFrm);//人员管理
  
    mObjMap.insert(QObject::tr("ImportPersonRecord"), m_pImportUserFrm);//导入人员
    mObjMap.insert(QObject::tr("InqueryPerson"), m_pUserViewFrm);//查看人员

    mObjMap.insert(QObject::tr("RecordsManagement"), m_pRecordsManagementFrm);//记录管理
    mObjMap.insert(QObject::tr("ViewPassRecord"), m_pViewAccessRecordsFrm);//查看通行记录

    mObjMap.insert(QObject::tr("NetworkSetup"), m_pNetworkSetupFrm);//网络设置
    mObjMap.insert(QObject::tr("EthernetSetup"), m_pEthernetViewFrm);//网口设置
    if(!access("/sys/class/net/wlan0/",F_OK))
    {
    	mObjMap.insert(QObject::tr("Wi-FiSetup"), m_pWifiViewFrm);//Wi-Fi设置
    }
    //if(!access("/sys/class/net/p2p0/",F_OK))
    {
    	mObjMap.insert(QObject::tr("4G"), m_pNetwork4GViewFrm);//4G设置
    }

    mObjMap.insert(QObject::tr("HostSetup"), m_pSrvSetupFrm);//服务器设置


    mObjMap.insert(QObject::tr("SystemSetup"), m_pSysSetupFrm);//系统配置

    mObjMap.insert(QObject::tr("About"), m_pAboutMachineFrm);//关于设备
    mObjMap.insert(QObject::tr("AccessControlSettings"), m_pDoorControFrm);//门禁设置
    mObjMap.insert(QObject::tr("PassagePeriod"), m_pPassageTimeFrm);//通行时段
    mObjMap.insert(QObject::tr("DoorOpeningMode"), m_pPassageModeFrm);//开门方式
    mObjMap.insert(QObject::tr("MainUISettings"), m_pMainsetFrm);//主界面设置
    mObjMap.insert(QObject::tr("TimeSetting"), m_pTimesetFrm);//时间设置
    mObjMap.insert(QObject::tr("StorageCapacity"), m_pStorageCapacityFrm);//存储容量

    mObjMap.insert(QObject::tr("IdentifySetup"), m_pIdentifySetupFrm);//识别设置
}

void SettingMenuFrmPrivate::InitConnect()
{
    QObject::connect(m_pHomeMenuFrm, &HomeMenuFrm::sigShowFrm, q_func(), &SettingMenuFrm::slotShowFrm);
    QObject::connect(m_pManagingPeopleFrm, &ManagingPeopleFrm::sigShowFrm, q_func(), &SettingMenuFrm::slotShowFrm);
    QObject::connect(m_pUserViewFrm, &UserViewFrm::sigShowFrm, q_func(), &SettingMenuFrm::slotShowFrm);//修改用户 
    QObject::connect(m_pRecordsManagementFrm, &RecordsManagementFrm::sigShowFrm, q_func(), &SettingMenuFrm::slotShowFrm);
    QObject::connect(m_pNetworkSetupFrm, &NetworkSetupFrm::sigShowFrm, q_func(), &SettingMenuFrm::slotShowFrm);
    QObject::connect(m_pSrvSetupFrm, &SrvSetupFrm::sigShowFrm, q_func(), &SettingMenuFrm::slotShowFrm);
    QObject::connect(m_pSysSetupFrm, &SysSetupFrm::sigShowFrm, q_func(), &SettingMenuFrm::slotShowFrm);
    QObject::connect(m_pDoorControFrm, &DoorControFrm::sigShowFrm, q_func(), &SettingMenuFrm::slotShowFrm);
    QObject::connect(m_pIdentifySetupFrm, &IdentifySetupFrm::sigShowFrm, q_func(), &SettingMenuFrm::slotShowFrm);

    QObject::connect(m_pSettingMenuTitleFrm, &SettingMenuTitleFrm::sigReturnSuperiorClicked, q_func(), &SettingMenuFrm::slotReturnSuperiorClicked);


    QObject::connect(m_pHomeMenuFrm, &HomeMenuFrm::sigShowLevelFrm, q_func(), &SettingMenuFrm::slotReturnSuperiorClicked);
    QObject::connect(m_pManagingPeopleFrm, &ManagingPeopleFrm::sigShowLevelFrm, q_func(), &SettingMenuFrm::slotReturnSuperiorClicked);
    QObject::connect(m_pRecordsManagementFrm, &RecordsManagementFrm::sigShowLevelFrm, q_func(), &SettingMenuFrm::slotReturnSuperiorClicked);
    QObject::connect(m_pNetworkSetupFrm, &NetworkSetupFrm::sigShowLevelFrm, q_func(), &SettingMenuFrm::slotReturnSuperiorClicked);
    QObject::connect(m_pSrvSetupFrm, &SrvSetupFrm::sigShowLevelFrm, q_func(), &SettingMenuFrm::slotReturnSuperiorClicked);
    QObject::connect(m_pSysSetupFrm, &SysSetupFrm::sigShowLevelFrm, q_func(), &SettingMenuFrm::slotReturnSuperiorClicked);
    QObject::connect(m_pDoorControFrm, &DoorControFrm::sigShowLevelFrm, q_func(), &SettingMenuFrm::slotReturnSuperiorClicked);
    QObject::connect(m_pIdentifySetupFrm, &IdentifySetupFrm::sigShowLevelFrm, q_func(), &SettingMenuFrm::slotReturnSuperiorClicked);

    QObject::connect(m_pTimesetFrm, &TimesetFrm::sigShowLevelFrm, q_func(), &SettingMenuFrm::slotReturnSuperiorClicked);
    QObject::connect(m_pStorageCapacityFrm, &StorageCapacityFrm::sigShowLevelFrm, q_func(), &SettingMenuFrm::slotReturnSuperiorClicked);
    QObject::connect(m_pAboutMachineFrm, &AboutMachineFrm::sigShowLevelFrm, q_func(), &SettingMenuFrm::slotReturnSuperiorClicked);
    QObject::connect(m_pViewAccessRecordsFrm, &ViewAccessRecordsFrm::sigShowLevelFrm, q_func(), &SettingMenuFrm::slotReturnSuperiorClicked);
    QObject::connect(m_pImportUserFrm, &ImportUserFrm::sigShowLevelFrm, q_func(), &SettingMenuFrm::slotReturnSuperiorClicked);
    QObject::connect(m_pUserViewFrm, &UserViewFrm::sigShowLevelFrm, q_func(), &SettingMenuFrm::slotReturnSuperiorClicked);
    QObject::connect(m_pEthernetViewFrm, &EthernetViewFrm::sigShowLevelFrm, q_func(), &SettingMenuFrm::slotReturnSuperiorClicked);
    if(!access("/sys/class/net/wlan0/",F_OK))
    {
    	QObject::connect(m_pWifiViewFrm, &WifiViewFrm::sigShowLevelFrm, q_func(), &SettingMenuFrm::slotReturnSuperiorClicked);
    }
    //if(!access("/sys/class/net/p2p0/",F_OK) || !access("/sys/devices/virtual/net/ppp0/",F_OK))//这句有问题，会跑飞
    //if(!access("/sys/class/net/p2p0/",F_OK) )
    {
    	QObject::connect(m_pNetwork4GViewFrm, &Network4GViewFrm::sigShowLevelFrm, q_func(), &SettingMenuFrm::slotReturnSuperiorClicked);
    }

    QObject::connect(m_pMainsetFrm, &MainsetFrm::sigShowLevelFrm, q_func(), &SettingMenuFrm::slotReturnSuperiorClicked);
    QObject::connect(m_pPassageTimeFrm, &PassageTimeFrm::sigShowLevelFrm, q_func(), &SettingMenuFrm::slotReturnSuperiorClicked);
    QObject::connect(m_pPassageModeFrm, &PassageModeFrm::sigShowLevelFrm, q_func(), &SettingMenuFrm::slotReturnSuperiorClicked);
}

void SettingMenuFrm::setShowMenuFrm()
{
    Q_D(SettingMenuFrm);    
 #if 0   
    this->slotShowFrm(QObject::tr("Menu"));//菜单
    //遍历 19个窗口, 找出lineEdit 控件, 并设置菜单
    //m_pPasswordEdit->setContextMenuPolicy(Qt::NoContextMenu);
	printf(">>>%s,%s,%d,count=%d\n",__FILE__,__func__,__LINE__,d->m_pStackedWidget->count());    
    for (int i=0; i<d->m_pStackedWidget->count(); i++)
    {

        QWidget *theForm = d->m_pStackedWidget->widget(i);
      #if 0  
        qDebug()<<"form :"<<i <<" "<<d->m_pStackedWidget->widget(i)<<" widget: "<<theForm->children()<<" count:"<<theForm->children().length();        
        traversalControl(theForm->children());//
       #endif 
    }
#endif 

    //遍历 19个窗口, 多语种设置     
}

void SettingMenuFrm::slotShowFrm(const QString name)
{
        printf(">>>%s,%s,%d,name=%s\n",__FILE__,__func__,__LINE__,name.toStdString().c_str());     
    Q_D(SettingMenuFrm);
    if(name == QObject::tr("AddPerson"))//新增人员
    {
        emit sigShowFaceHomeFrm(2);
    } else if(name == QObject::tr("ModifyPerson"))//修改人员
    {
        emit sigShowFaceHomeFrm(2);
    }
    else
    {
        QWidget *w = d->mObjMap.value(name);
        printf(">>>%s,%s,%d,name=%s\n",__FILE__,__func__,__LINE__,name.toStdString().c_str()); 
        if(w == Q_NULLPTR)return;
        printf(">>>%s,%s,%d,name=%s\n",__FILE__,__func__,__LINE__,name.toStdString().c_str()); 
        d->m_pSettingMenuTitleFrm->setTitleText(name);
        printf(">>>%s,%s,%d,name=%s\n",__FILE__,__func__,__LINE__,name.toStdString().c_str()); 
        //通知窗口进入
        static_cast<SettingBaseFrm *>(w)->setEnter();
        printf(">>>%s,%s,%d,name=%s\n",__FILE__,__func__,__LINE__,name.toStdString().c_str()); 
        d->mOldObj.append(w);
        printf(">>>%s,%s,%d,name=%s\n",__FILE__,__func__,__LINE__,name.toStdString().c_str()); 
        d->m_pStackedWidget->setCurrentWidget(w);
        printf(">>>%s,%s,%d,name=%s\n",__FILE__,__func__,__LINE__,name.toStdString().c_str()); 
    }
}

void SettingMenuFrm::slotReturnSuperiorClicked()
{
    Q_D(SettingMenuFrm);
    QWidget *w = Q_NULLPTR;
    if(d->mOldObj.size())
    {//行删除最后一个元素
        if(d->mOldObj.last() == d->m_pPassageModeFrm)
        {
            if(d->m_pPassageModeFrm->getOpenIsEmpty())
            {
                OperationTipsFrm dlg(this);
                //dlg.setMessageBox(QObject::tr("温馨提示"), QObject::tr("必须满足项不允许为空!!!"));
                dlg.setMessageBox(QObject::tr("Tips"), QObject::tr("requiredAndOptionalNotNULL"));
                return;
            }
        }
        if (static_cast<SettingBaseFrm *>(d->mOldObj.last()) != Q_NULLPTR )
            static_cast<SettingBaseFrm *>(d->mOldObj.last())->setLeaveEvent();          

        d->mOldObj.removeLast();
    }

    if(d->mOldObj.size()) w = d->mOldObj.last();

    if(w == Q_NULLPTR)
    {
        d->mOldObj.append(d->m_pHomeMenuFrm);
        emit sigShowFaceHomeFrm();
    }else
    {
        d->m_pSettingMenuTitleFrm->setTitleText(d->mObjMap.key(w));
        //通知窗口进入
        static_cast<SettingBaseFrm *>(w)->setEnter();
        d->m_pStackedWidget->setCurrentWidget(w);
    }
}

void SettingMenuFrm::paintEvent(QPaintEvent *event)
{        
    QPainter painter(this);
    QStyleOption opt;
    opt.init(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
    QWidget::paintEvent(event);
}

#ifdef SCREENCAPTURE  //ScreenCapture  
void SettingMenuFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    Q_D(SettingMenuFrm);    
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");   
    d->m_pStackedWidget->grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png"); 
}	
#endif 