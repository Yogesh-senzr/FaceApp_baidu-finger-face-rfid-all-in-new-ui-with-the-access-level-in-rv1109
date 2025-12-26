#include "FaceHomeFrm.h"

#include "FaceHomeTitleFrm.h"
#include "FaceHomeBottomFrm.h"

#include "Helper/myhelper.h"
#include "SharedInclude/GlobalDef.h"
#include "HealthCodeFrm.h"
#include "NtpDate/NtpDateSync.h"
#include "Config/ReadConfig.h"
#include "PCIcore/Utils_Door.h"
#include "SettingFuncFrms/SysSetupFrms/DoorControFrms/DoorLockFrm.h"
#include "ManageEngines/PersonRecordToDB.h"
#include "Application/FaceApp.h"
#include "BaiduFace/BaiduFaceManager.h"
#include "MessageHandler/Log.h"
#include "RkNetWork/NetworkControlThread.h"

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <sys/stat.h>
#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QHBoxLayout>
#include <QtNetwork/QNetworkConfigurationManager>
#include <QtCore/QDebug>
#include <QtNetwork/QNetworkInterface>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDriver>
#include <QtGui/QPainter>
#include <QtCore/QDebug>
#include <QtConcurrent/QtConcurrent>
#include <QPushButton>
#include <QLineEdit>

class FaceHomeFrmPrivate
{
    Q_DECLARE_PUBLIC(FaceHomeFrm)
public:
    FaceHomeFrmPrivate(FaceHomeFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    void CheckNet();
    void CheckPersonNum();
private:
    FaceHomeTitleFrm *m_FaceHomeTitleFrm;
    HomeBottomBaseFrm *m_pFaceHomeBottomFrm;
    DoorLockFrm *m_pDoorLockFrm;
private:
    QLabel *m_pTopMessageHintLabel_1;//消息提示
    QLabel *m_pTopMessageHintLabel_2;//
    QLabel *m_pTopMessageHintLabel_3;//
    QLabel *m_pTopMessageHintLabel_4;//
    QLabel *m_pTopMessageHintLabel_5;//

    QLabel *m_pBottomMessageHintLabel_1;
    QLabel *m_pBottomMessageHintLabel_2;
    QLabel *m_pBottomMessageHintLabel_3;
    QLabel *m_pBottomMessageHintLabel_4;

    
    QLabel *m_pPersonImageLabel; 

    QPushButton *m_pPwdButton;
private:
    QNetworkConfigurationManager *ncmger;
private:
    HealthCodeFrm *m_pHealthCodeFrm;
private:
    FaceHomeFrm *const q_ptr;

public:
    //用于断连时重扫描WiFi
    pthread_t m_WifiThread;
    pthread_mutex_t m_WifiMutex;
    pthread_cond_t m_WifiCond;
};

FaceHomeFrmPrivate::FaceHomeFrmPrivate(FaceHomeFrm *dd)
    : q_ptr(dd)
    , ncmger(new QNetworkConfigurationManager)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

FaceHomeFrm::FaceHomeFrm(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new FaceHomeFrmPrivate(this))
{
    d_func()->m_pHealthCodeFrm = new HealthCodeFrm(this);
}

FaceHomeFrm::~FaceHomeFrm()
{
    pthread_cancel(d_func()->m_WifiThread);
}

void FaceHomeFrmPrivate::InitUI()
{
    m_FaceHomeTitleFrm = new FaceHomeTitleFrm;
    m_pFaceHomeBottomFrm = new FaceHomeBottomFrm;
    m_pDoorLockFrm = new DoorLockFrm;
    m_pTopMessageHintLabel_1 = new QLabel;//消息提示
    m_pTopMessageHintLabel_2 = new QLabel;
    m_pTopMessageHintLabel_3 = new QLabel;
    m_pTopMessageHintLabel_4 = new QLabel;
    m_pTopMessageHintLabel_5 = new QLabel;

    m_pBottomMessageHintLabel_1 = new QLabel;
    m_pBottomMessageHintLabel_2 = new QLabel;
    m_pBottomMessageHintLabel_3 = new QLabel;

    QVBoxLayout *vHintLayout = new QVBoxLayout;
    vHintLayout->setSpacing(5);
    {
        QHBoxLayout *hlayout1 = new QHBoxLayout;
        hlayout1->addStretch();
        hlayout1->addWidget(m_pTopMessageHintLabel_1);
        hlayout1->addStretch();

        QHBoxLayout *hlayout2 = new QHBoxLayout;
        hlayout2->addStretch();
        hlayout2->addWidget(m_pTopMessageHintLabel_2);
        hlayout2->addStretch();

        QHBoxLayout *hlayout3 = new QHBoxLayout;
        hlayout3->addStretch();
        hlayout3->addWidget(m_pTopMessageHintLabel_3);
        hlayout3->addStretch();

        QHBoxLayout *hlayout4 = new QHBoxLayout;
        hlayout4->addStretch();
        hlayout4->addWidget(m_pTopMessageHintLabel_4);
        hlayout4->addStretch();

        QHBoxLayout *hlayout5 = new QHBoxLayout;
        hlayout5->addStretch();
        hlayout5->addWidget(m_pTopMessageHintLabel_5);
        hlayout5->addStretch();

        vHintLayout->addLayout(hlayout1);
        vHintLayout->addLayout(hlayout2);
        vHintLayout->addLayout(hlayout3);
        vHintLayout->addLayout(hlayout4);
        vHintLayout->addLayout(hlayout5);
        vHintLayout->addStretch();
    }
    {
        QHBoxLayout *hlayout1 = new QHBoxLayout;
        hlayout1->addStretch();
        hlayout1->addWidget(m_pBottomMessageHintLabel_1);
        hlayout1->addStretch();

        QHBoxLayout *hlayout2 = new QHBoxLayout;
        hlayout2->addStretch();
        hlayout2->addWidget(m_pBottomMessageHintLabel_2);
        hlayout2->addStretch();
#if 0
        QHBoxLayout *hlayout3 = new QHBoxLayout;
        hlayout3->addStretch();
        hlayout3->addWidget(m_pBottomMessageHintLabel_3);
        hlayout3->addStretch();
#endif 
        vHintLayout->addLayout(hlayout1);
        vHintLayout->addLayout(hlayout2);
        //vHintLayout->addLayout(hlayout3);
    }

// Create person image label with absolute positioning
m_pPersonImageLabel = new QLabel(q_func()); // Make sure parent is set correctly
m_pPersonImageLabel->setFixedSize(120, 160); // Passport size
m_pPersonImageLabel->setStyleSheet("border: 2px solid white; background-color: #333333;");
m_pPersonImageLabel->setAlignment(Qt::AlignCenter);
m_pPersonImageLabel->setScaledContents(true);

// Position it absolutely - move it down significantly
int nDeskW = QApplication::desktop()->screenGeometry().width();
int nDeskH = QApplication::desktop()->screenGeometry().height();

// Position on right side, moved down from center
m_pPersonImageLabel->setGeometry(nDeskW - 150, nDeskH/2 + 100, 120, 160); // +100 moves it down

m_pPersonImageLabel->hide(); // Initially hidden
m_pPersonImageLabel->raise(); // Bring to front

QHBoxLayout *layout = new QHBoxLayout;
         
m_pPwdButton =new QPushButton; 
m_pPwdButton->setIcon(QPixmap(":/Images/doorLock.png"));                             

QHBoxLayout *layout2 = new QHBoxLayout;

layout2->addSpacing(330);
layout2->addWidget(m_pPwdButton);
layout2->addSpacing(330);                

// Create main content layout with left and right sections
QHBoxLayout *mainContentLayout = new QHBoxLayout;

// Left side - existing hint messages
QVBoxLayout *leftLayout = new QVBoxLayout;
leftLayout->addLayout(vHintLayout);

// Right side - person image
QVBoxLayout *rightLayout = new QVBoxLayout;
rightLayout->addStretch(3);
rightLayout->addWidget(m_pPersonImageLabel);
rightLayout->addStretch(1);

mainContentLayout->addLayout(leftLayout);
mainContentLayout->addStretch();
mainContentLayout->addLayout(rightLayout);
mainContentLayout->addSpacing(50); // Right margin

layout->addLayout(mainContentLayout);


    QVBoxLayout *vlayout = new QVBoxLayout(q_func());
    vlayout->setMargin(0);
    vlayout->addWidget(m_FaceHomeTitleFrm);
    vlayout->addLayout(layout);
    vlayout->addStretch(5);


    vlayout->addLayout(layout2); //密码 按纽    


//vlayout->addStretch(5);//50

QHBoxLayout *hlayout3 = new QHBoxLayout;
hlayout3->addSpacing(50); // Left margin - adjust this value as needed
hlayout3->addWidget(m_pBottomMessageHintLabel_3); // Left aligned
hlayout3->addStretch(); // Only stretch on the right
vlayout->addLayout(hlayout3);

//vlayout->addStretch(5);//50

vlayout->addWidget(m_pFaceHomeBottomFrm);
	{
		int nDeskW = QApplication::desktop()->screenGeometry().width();
		int nDeskH = QApplication::desktop()->screenGeometry().height();

		m_pBottomMessageHintLabel_4 = new QLabel(q_func());
		m_pBottomMessageHintLabel_4->setGeometry(nDeskW - 120, nDeskH - 60, 120, 40);
		m_pBottomMessageHintLabel_4->setStyleSheet("color:white;font-size:16px;");
	}
 
}

//当处于WiFi未连接且网路也未连接的状态时，每隔一段时间重新扫描一次wifi
static void *RescanWifiThread(void *data)
{
    FaceHomeFrmPrivate *f = (FaceHomeFrmPrivate *)data;
    QNetworkConfigurationManager mgr;
    while(1){
        pthread_mutex_lock(&f->m_WifiMutex);
        pthread_cond_wait(&f->m_WifiCond,&f->m_WifiMutex);
        pthread_mutex_unlock(&f->m_WifiMutex);

        if(!access("/param/license.ini",F_OK) && (qXLApp->GetAlgoFaceManager()) && ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->getAlgoFaceInitState() == false){
            sleep(1);
            continue;
        }
        
        NetworkControlThread::GetInstance()->resume();
        sleep(5);
    }
    return NULL;
}

void FaceHomeFrmPrivate::InitData()
{
    m_pTopMessageHintLabel_1->setObjectName("TopMessageHintLabel_1");
    m_pTopMessageHintLabel_2->setObjectName("TopMessageHintLabel_2");
    m_pTopMessageHintLabel_3->setObjectName("TopMessageHintLabel_3");
    m_pTopMessageHintLabel_4->setObjectName("TopMessageHintLabel_4");
    m_pTopMessageHintLabel_5->setObjectName("TopMessageHintLabel_5");

    m_pBottomMessageHintLabel_1->setObjectName("BottomMessageHintLabel_1");
    m_pBottomMessageHintLabel_2->setObjectName("BottomMessageHintLabel_2");
    m_pBottomMessageHintLabel_3->setObjectName("BottomMessageHintLabel_3");
    // Add this in InitData() function
    m_pTopMessageHintLabel_2->setFixedSize(540, 80); //200, 40
    if (ReadConfig::GetInstance()->getDoor_Password()=="")     
    {
        m_pPwdButton->setVisible(false);         
    }
    else {
        m_pPwdButton->setFixedSize(120, 60);
        m_pPwdButton->setVisible(true);        
    }    
    m_pTopMessageHintLabel_2->setAlignment(Qt::AlignCenter);
#if 0
    m_pTopMessageHintLabel_1->setText("消息提示1");
    m_pTopMessageHintLabel_2->setText("消息提示2");
    m_pTopMessageHintLabel_3->setText("消息提示3");
    m_pTopMessageHintLabel_4->setText("消息提示4");
    m_pTopMessageHintLabel_5->setText("消息提示5");

    m_pBottomMessageHintLabel_1->setText("消息提示1");
    m_pBottomMessageHintLabel_2->setText("消息提示2");
#endif
    m_pTopMessageHintLabel_2->hide();
    m_pBottomMessageHintLabel_3->setText("");

    //初始化扫变量线程相关的变量
    pthread_mutex_init(&m_WifiMutex,NULL);
    pthread_cond_init(&m_WifiCond,NULL);
    pthread_create(&m_WifiThread,NULL,RescanWifiThread,this);
    pthread_detach(m_WifiThread);
}

void FaceHomeFrmPrivate::InitConnect()
{
    QObject::connect(ncmger,SIGNAL(onlineStateChanged(bool)),q_func(),SLOT(slotNetChange(bool)));
    QObject::connect(m_pPwdButton, &QPushButton::clicked, [&] {
            m_pDoorLockFrm->show();
            // m_pDoorLockFrm->move(70, 520);//380,400,440,500
        });
}

void FaceHomeFrmPrivate::CheckNet()
{
    //时间同步
    int syncTime = ReadConfig::GetInstance()->getTimer_Manager_autoTime();  
    if (syncTime) 
        NtpDateSync::GetInstance()->setSyncNtpDate();
        
    if (ReadConfig::GetInstance()->getNetwork_Manager_Mode() == 3)
    {
        if (!access("/sys/class/net/p2p0/", F_OK))
        {
            QString network4GIP;
            FILE *pFile = ISC_NULL;
            pFile = popen("ifconfig p2p0", "r");
            if (pFile)
            {
                char buf[256] = { 0 };
                fread(buf, sizeof(buf), 1, pFile);
                pclose(pFile);
                char *str = strstr(buf,"inet addr:");
                if(str)
                {
                    std::string strRet;
                    str += strlen("inet addr:");
                    for (int i = 0; i < 15; i++)
                    {
                        if (str[i] == ' ')
                        {
                            break;
                        }
                        strRet += str[i];
                    }
                    network4GIP = QString(strRet.c_str());
                }
            }
            if(network4GIP.size() > 0)
            {
                // Keep the network icon update but remove the bottom display
                this->m_FaceHomeTitleFrm->setLinkState(true, 1);
                if (syncTime) 
                    NtpDateSync::GetInstance()->setSyncNtpDate();
                return;
            }
        }
        
        if (!access("/sys/devices/virtual/net/ppp0", F_OK))
        {
            QString network4GIP;
            FILE *pFile = ISC_NULL;
            pFile = popen("ifconfig ppp0", "r");
            if (pFile)
            {
                char buf[256] = { 0 };
                fread(buf, sizeof(buf), 1, pFile);
                pclose(pFile);
                char *str = strstr(buf,"inet addr:");
                if(str)
                {
                    std::string strRet;
                    str += strlen("inet addr:");
                    for (int i = 0; i < 15; i++)
                    {
                        if (str[i] == ' ')
                        {
                            break;
                        }
                        strRet += str[i];
                    }
                    network4GIP = QString(strRet.c_str());
                }
            }
            if(network4GIP.size() > 0)
            {
                // Keep the network icon update but remove the bottom display
                this->m_FaceHomeTitleFrm->setLinkState(true, 1);
                if (syncTime) 
                    NtpDateSync::GetInstance()->setSyncNtpDate();
                return;
            }
        }    
    }
    
    QNetworkConfigurationManager mgr;
    //通过QNetworkInterface类来获取本机的IP地址和网络接口信息
    QList<QNetworkInterface> list = QNetworkInterface::allInterfaces();
    //获取所有网络接口的列表
    foreach(QNetworkInterface interface, list)
    {
        //硬件地址
        QList<QNetworkAddressEntry> entryList = interface.addressEntries();
        //获取IP地址条目列表，每个条目中包含一个IP地址，一个子网掩码和一个广播地址
        foreach(QNetworkAddressEntry entry, entryList)
        {
            if((entry.ip().protocol() == QAbstractSocket::IPv4Protocol) && (!entry.ip().toString().contains("127.0.0")))
            {
                // KEEP the network icon updates
                if(interface.name() == "eth0")
                    this->m_FaceHomeTitleFrm->setLinkState(mgr.isOnline(), 1);
                else if(interface.name() == "wlan0")
                    this->m_FaceHomeTitleFrm->setLinkState(mgr.isOnline(), 2);
                else if(interface.name() == "usb0")
                {
                    if (ReadConfig::GetInstance()->getNetwork_Manager_Mode() != 3) 
                        system("ifconfig usb0 down");
                }
                else 
                    this->m_FaceHomeTitleFrm->setLinkState(mgr.isOnline(), 3);
                    
                printf(">>>%s,%s,%d,name=%s\n",__FILE__,__func__,__LINE__,interface.name().toStdString().c_str());
                
                if (mgr.isOnline() && !((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->getAlgoFaceInitState()) 
                {
                    sleep(10);
                    printf(">>>%s,%s,%d\n",__FILE__,__func__,__LINE__);
                    ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->setRunFaceEngine();		
                }
                printf(">>>%s,%s,%d,getAlgoFaceInitState=%d\n",__FILE__,__func__,__LINE__,((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->getAlgoFaceInitState());

                if (syncTime) 
                    NtpDateSync::GetInstance()->setSyncNtpDate();                  
                return;
            }
        }
    }
    
    // Set offline state for the network icon
    this->m_FaceHomeTitleFrm->setLinkState(mgr.isOnline(), 1);

    //如果没有网络,且当前配置为wifi模式，唤醒WiFi扫描线程
    if(!mgr.isOnline() && (2 == ReadConfig::GetInstance()->getNetwork_Manager_Mode())){ 
        pthread_cond_signal(&m_WifiCond);
    }
}
static inline int queryRowCount(QSqlQuery &query)
{
    int initialPos = query.at();
    // Very strange but for no records .at() returns -2
    int pos = 0;
    if (query.last()) {
        pos = query.at() + 1;
    }
    else {
        pos = 0;
    }
    // Important to restore initial pos
    query.seek(initialPos);
    return pos;
}

void FaceHomeFrmPrivate::CheckPersonNum()
{
    // Get the current local user count
    int localCount = 0;
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    query.exec("select * from person");
    if (query.driver()->hasFeature(QSqlDriver::QuerySize))
    {
        localCount = query.size();
    }
    else
    {
        localCount = queryRowCount(query);
    }
    
    // For now, show local count / local count
    // This will be updated by the sync process with actual server counts
    q_func()->updateSyncUserCount(localCount, localCount);
}

void FaceHomeFrm::setDisMissMessage(const bool &state)
{
    Q_D(FaceHomeFrm);
    if(state)return;

    d->m_pTopMessageHintLabel_1->clear();
    if(!d->m_pTopMessageHintLabel_2->isHidden())d->m_pTopMessageHintLabel_2->hide();

    d->m_pTopMessageHintLabel_3->clear();
    d->m_pTopMessageHintLabel_4->clear();
    //d->m_pTopMessageHintLabel_5->clear();

    d->m_pBottomMessageHintLabel_1->clear();
    d->m_pBottomMessageHintLabel_2->clear();
    //d->m_pBottomMessageHintLabel_3->setText(QObject::tr("<font color=\"#FFFFFF\"></font>"));
    d->m_pBottomMessageHintLabel_3->clear();
    d->m_pBottomMessageHintLabel_4->clear();

    // ADD THIS LINE:
    d->m_pPersonImageLabel->hide();

    if(!d->m_pHealthCodeFrm->isHidden())d->m_pHealthCodeFrm->hide();
}

void FaceHomeFrm::setTipsMessage(const int &type, const int &pos, const QString &text)
{
    Q_D(FaceHomeFrm);    
    // if (((UnifiedFaceManager *)qXLApp->GetAlgoFaceManager())->getAlgoFaceInitState())                         
    // {
    //    // d->m_pTopMessageHintLabel_5->clear();
    // }    
    switch(type)
    {
        case TOP_MESSAGE:
        {
            switch(pos)
            {
            case 1:
                {
                    QFont ft;
                    ft.setPointSize(12);
                    d->m_pTopMessageHintLabel_1->setFont(ft);
                    this->setTopMessageHintText_1(text);
                }
                break;
            case 2:
                {
                    QFont ft;
                    ft.setPointSize(12);
                    d->m_pTopMessageHintLabel_2->setFont(ft);
                    this->setTopMessageHintText_2(text);
                }
                break;
            case 3:this->setTopMessageHintText_3(text);break;
            case 4:this->setTopMessageHintText_4(text);break;
            case 5:
                  { 
    LogD("%s %s[%d] m_pTopMessageHintLabel_5=%s  \n",__FILE__,__FUNCTION__,__LINE__,text.toStdString().c_str());                      
                  this->setTopMessageHintText_5(text);
                  }
                  break;
            }
        }break;
        case BOTTOM_MESSAGE:
{
    switch(pos)
    {
    case 1:this->setBottomMessageHintText_1(text);break;
    case 2:this->setBottomMessageHintText_2(text);break;
    case 3: // Recognition message position
                {
                    // Configure label for bottom-center display
                    d->m_pBottomMessageHintLabel_3->setAlignment(Qt::AlignCenter);
                    
                    // Update CSS for bottom-center positioning
                    QString style = QString(
                        "QLabel {"
                        "   position: absolute;"
                        "   left: 50%;"
                        "   transform: translateX(-50%);"
                        "   bottom: 100px;"  // Adjust this value as needed
                        "   color: #00FF00;"  // Green text
                        "   font-size: 68px;"
                        "   font-weight: bold;"
                        "   background-color: transparent;"
                        "   text-align: center;"
                        "   min-width: %1px;"
                        "   max-width: %1px;"
                        "}").arg(this->width() * 0.8);  // 80% of screen width
                    
                    d->m_pBottomMessageHintLabel_3->setStyleSheet(style);
                    
                    // Set label geometry to cover bottom center area
                    int labelWidth = this->width() * 0.8;  // 80% of screen width
                    int labelHeight = 200;  // Adjust as needed
                    int xPos = (this->width() - labelWidth) / 2;
                    int yPos = this->height() - labelHeight - 100;  // 100px from bottom
                    
                    d->m_pBottomMessageHintLabel_3->setGeometry(xPos, yPos, labelWidth, labelHeight);
                    this->setBottomMessageHintText_3(text);
                }
                break;
    }
}break;
        case ALARM_MESSAGE:
        {
            switch(pos)
            {
                case 1:
                    {
                        QFont ft;
                        ft.setPointSize(30);
                        d->m_pTopMessageHintLabel_1->setFont(ft);
                        this->setTopMessageHintText_1(text);
                    }
                    break;
                case 2:
                    {
                        QFont ft;
                        ft.setPointSize(30);
                        d->m_pTopMessageHintLabel_2->setFont(ft);
                        this->setTopMessageHintText_2(text);

                   
                    }
                    break;                    
            }
        }break;        
    }
}

void FaceHomeFrm::setAlgoStateAboutFace(const QString &text)
{
	Q_D(FaceHomeFrm);
	if(d->m_pBottomMessageHintLabel_4->isVisible())
	{
		d->m_pBottomMessageHintLabel_4->setText(text);
	}
}

void FaceHomeFrm::setUpDateTip(const QString &text)
{
    Q_D(FaceHomeFrm);
    LogD("%s %s[%d],text=%s \n",__FILE__,__FUNCTION__,__LINE__,text.toStdString().c_str());    
    d->m_pTopMessageHintLabel_5->setText(text);
    #if 0
        if (text.length()>0)
        d->m_pTopMessageHintLabel_5->setText(text);
        else 
        {
        d->m_pTopMessageHintLabel_5->clear();
        //d->m_pTopMessageHintLabel_5->hide();
        //d->m_pTopMessageHintLabel_5->setStyleSheet("background:transparent");      
        }
    #endif 


}

void FaceHomeFrm::setHealthCodeInfo(const int &type, const QString &name, const QString &idCard, const int &qrCodeType, const QString &msg)
{
    Q_D(FaceHomeFrm);
    d->m_pHealthCodeFrm->setHealthCodeInfo(type, name, idCard, qrCodeType, msg);
}



void FaceHomeFrm::setHomeDisplay_SnNum(const int &show)
{
    Q_D(FaceHomeFrm);
    d->m_pFaceHomeBottomFrm->setHomeDisplay_SnNum(show);
}

void FaceHomeFrm::setHomeDisplay_Mac(const int &show)
{
    Q_D(FaceHomeFrm);
    d->m_pFaceHomeBottomFrm->setHomeDisplay_Mac(show);
}

void FaceHomeFrm::setHomeDisplay_IP(const int &show)
{
    Q_D(FaceHomeFrm);
    d->m_pFaceHomeBottomFrm->setHomeDisplay_IP(show);
}

void FaceHomeFrm::setHomeDisplay_PersonNum(const int &show)
{
    Q_D(FaceHomeFrm);
    d->m_pFaceHomeBottomFrm->setHomeDisplay_PersonNum(show);
}

void FaceHomeFrm::setHomeDisplay_DoorLock(const int &show)
{
    Q_D(FaceHomeFrm);
    d->InitData();
}

void FaceHomeFrm::setTopMessageHintText_1(const QString &text)
{
    Q_D(FaceHomeFrm);
    d->m_pTopMessageHintLabel_1->setText(text);
}

void FaceHomeFrm::setTopMessageHintText_2(const QString &text)
{
    Q_D(FaceHomeFrm);
    d->m_pTopMessageHintLabel_2->setText(text);
    if(d->m_pTopMessageHintLabel_2->isHidden())
        d->m_pTopMessageHintLabel_2->show();
    if (text.length()==0)
    {
        d->m_pTopMessageHintLabel_2->hide();
    }
}

void FaceHomeFrm::setTopMessageHintText_3(const QString &text)
{
    Q_D(FaceHomeFrm);
    d->m_pTopMessageHintLabel_3->setText(text);
}

void FaceHomeFrm::setTopMessageHintText_4(const QString &text)
{
    Q_D(FaceHomeFrm);
    d->m_pTopMessageHintLabel_4->setText(text);
}

void FaceHomeFrm::setTopMessageHintText_5(const QString &text)
{
    Q_D(FaceHomeFrm);
    LogD("%s %s[%d] m_pTopMessageHintLabel_5=%s  \n",__FILE__,__FUNCTION__,__LINE__,text.toStdString().c_str());              
    d->m_pTopMessageHintLabel_5->setText(text);
    LogD("%s %s[%d] m_pTopMessageHintLabel_5=%s  \n",__FILE__,__FUNCTION__,__LINE__,text.toStdString().c_str());              
}

void FaceHomeFrm::setBottomMessageHintText_1(const QString &text)
{
    Q_D(FaceHomeFrm);
    d->m_pBottomMessageHintLabel_1->setText(text);
}

void FaceHomeFrm::setBottomMessageHintText_2(const QString &text)
{
    Q_D(FaceHomeFrm);
    d->m_pBottomMessageHintLabel_2->setText(text);
}

void FaceHomeFrm::setBottomMessageHintText_3(const QString &text)
{
    Q_D(FaceHomeFrm);
    d->m_pBottomMessageHintLabel_3->setText(text);
    
}

void FaceHomeFrm::setTenantName(const QString &tenantName)
{
    Q_D(FaceHomeFrm);
    if (d->m_pFaceHomeBottomFrm) {
        // Cast to FaceHomeBottomFrm and call the public method
        FaceHomeBottomFrm* bottomFrm = qobject_cast<FaceHomeBottomFrm*>(d->m_pFaceHomeBottomFrm);
        if (bottomFrm) {
            bottomFrm->setTenantName(tenantName);
        }
    }
}

void FaceHomeFrm::updateSyncUserCount(int currentCount, int totalCount)
{
    Q_D(FaceHomeFrm);
    if (d->m_pFaceHomeBottomFrm) {
        static_cast<FaceHomeBottomFrm*>(d->m_pFaceHomeBottomFrm)->setSyncUserCount(currentCount, totalCount);
    }
}

void FaceHomeFrm::updateSyncStatus(const QString &status)
{
    Q_D(FaceHomeFrm);
    if (d->m_pFaceHomeBottomFrm) {
        static_cast<FaceHomeBottomFrm*>(d->m_pFaceHomeBottomFrm)->setSyncStatus(status);
    }
}

void FaceHomeFrm::updateLastSyncTime(const QString &time)
{
    Q_D(FaceHomeFrm);
    if (d->m_pFaceHomeBottomFrm) {
        static_cast<FaceHomeBottomFrm*>(d->m_pFaceHomeBottomFrm)->setLastSyncTime(time);
    }
}

void FaceHomeFrm::updateLocalFaceCount(int localCount, int totalCount)
{
    Q_D(FaceHomeFrm);
    if (d->m_pFaceHomeBottomFrm) {
        // Cast to FaceHomeBottomFrm and call the public method
        FaceHomeBottomFrm* bottomFrm = qobject_cast<FaceHomeBottomFrm*>(d->m_pFaceHomeBottomFrm);
        if (bottomFrm) {
            bottomFrm->setLocalFaceCount(localCount, totalCount);
        }
    }
}


void FaceHomeFrm::onCheckSN()
{
    Q_D(FaceHomeFrm);
    d->m_pFaceHomeBottomFrm->setSNNUm(myHelper::getCpuSerial());
}

void FaceHomeFrm::onCheckNet()
{
#ifdef Q_OS_WIN
    Q_D(FaceHomeFrm);
    d->CheckNet();
#else
    QtConcurrent::run(d_func(), &FaceHomeFrmPrivate::CheckNet);
#endif
}

void FaceHomeFrm::onCheckPersonNum()
{
    QtConcurrent::run(d_func(), &FaceHomeFrmPrivate::CheckPersonNum);
}

void FaceHomeFrm::setPersonImage(const QString &imagePath, const QString &personName)
{
    Q_D(FaceHomeFrm);
    
    qDebug() << "setPersonImage called with path:" << imagePath << "name:" << personName;
    
    // Check multiple possible image locations
    QStringList possiblePaths;
    QString uuid = imagePath.split("/").last().split(".").first(); // Extract UUID from path
    
    possiblePaths << imagePath; // Original path
    possiblePaths << QString("/mnt/user/%1.jpg").arg(uuid);
    possiblePaths << QString("/mnt/user/facedb/%1.jpg").arg(uuid);
    possiblePaths << QString("/mnt/user/face_crop_image/%1.jpg").arg(uuid);
    possiblePaths << QString("/mnt/user/face_crop_image/%1/%2.jpg").arg(QDate::currentDate().toString("yyyy-MM-dd")).arg(uuid);
    
    QString actualImagePath;
    foreach(const QString &path, possiblePaths) {
        qDebug() << "Checking path:" << path;
        if (QFile::exists(path)) {
            actualImagePath = path;
            qDebug() << "Found image at:" << actualImagePath;
            break;
        }
    }
    
    if (actualImagePath.isEmpty()) {
        qDebug() << "No image found in any location for UUID:" << uuid;
        
        // Let's also check what directories exist
        QStringList dirsToCheck;
        dirsToCheck << "/mnt/user/" << "/mnt/user/facedb/" << "/mnt/user/face_crop_image/";
        
        foreach(const QString &dirPath, dirsToCheck) {
    QDir dir(dirPath);
    if (dir.exists()) {
        QStringList files = dir.entryList(QDir::Files);
        qDebug() << "Files in" << dirPath << ":" << files.mid(0, 10); // Show first 10 files
    } else {
        qDebug() << "Directory does not exist:" << dirPath;
    }
}
        
        d->m_pPersonImageLabel->hide();
        return;
    }
    
    // Load and scale the image
    QPixmap originalPixmap(actualImagePath);
    if (originalPixmap.isNull()) {
        qDebug() << "Failed to load image from:" << actualImagePath;
        d->m_pPersonImageLabel->hide();
        return;
    }
    
    qDebug() << "Successfully loaded image, size:" << originalPixmap.size();
    
    // Scale to passport size while maintaining aspect ratio
    QPixmap scaledPixmap = originalPixmap.scaled(120, 160, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    
    // Create a passport-sized canvas with white background
    QPixmap passportPixmap(120, 160);
    passportPixmap.fill(Qt::white);
    
    // Center the scaled image on the canvas
    QPainter painter(&passportPixmap);
    int x = (120 - scaledPixmap.width()) / 2;
    int y = (160 - scaledPixmap.height()) / 2;
    painter.drawPixmap(x, y, scaledPixmap);
    
    d->m_pPersonImageLabel->setPixmap(passportPixmap);
    d->m_pPersonImageLabel->show();
    
    qDebug() << "Image should now be visible";
    
    // Optional: Add person name as tooltip
    d->m_pPersonImageLabel->setToolTip(personName);
}

void FaceHomeFrm::clearPersonImage()
{
    Q_D(FaceHomeFrm);
    d->m_pPersonImageLabel->clear();
    d->m_pPersonImageLabel->hide();
}

void FaceHomeFrm::slotNetChange(bool /*status*/)
{
#ifdef Q_OS_WIN
    Q_D(FaceHomeFrm);
    d->CheckNet();
#else
    QtConcurrent::run(d_func(), &FaceHomeFrmPrivate::CheckNet);
#endif
}

//void FaceHomeFrm::paintEvent(QPaintEvent *event)
//{
//    Q_D(FaceHomeFrm);
//    if(!d->mHealthCodePix.isNull())
//    {
//        QPainter painter(this);
//        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform, true);

//        int x = (this->width()/2 - 200);
//        int y = (this->height()/2 - 350);
//        painter.drawPixmap(x, y, 400, 600, d->mHealthCodePix);

//        {
//            QFont fnt = this->font();
//            fnt.setPixelSize(40);
//            fnt.setBold(true);
//            painter.setFont(fnt);

//            QFontMetrics fm(fnt);
//            int fntw = fm.width(d->midCardName);
//            painter.setPen(Qt::white);
//            painter.drawText((this->width()/2) - (fntw/2), y + 290, d->midCardName);

//        }
//        {
//            QFont fnt = this->font();
//            fnt.setPixelSize(25);
//            painter.setFont(fnt);

//            QFontMetrics fm(fnt);
//            int fntw = fm.width(d->midCardNum);
//            painter.setPen(Qt::white);
//            painter.drawText((this->width()/2) - (fntw/2), y + 325, d->midCardNum);
//        }
//    }
//    QWidget::paintEvent(event);
//}
