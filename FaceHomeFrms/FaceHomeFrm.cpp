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

    QHBoxLayout *layout = new QHBoxLayout;
         
    m_pPwdButton =new QPushButton; 
    m_pPwdButton->setIcon(QPixmap(":/Images/doorLock.png"));                             

    QHBoxLayout *layout2 = new QHBoxLayout;

    layout2->addSpacing(330);
    layout2->addWidget(m_pPwdButton);

    layout2->addSpacing(330);                
    layout->addStretch();
    layout->addLayout(vHintLayout);
    layout->addStretch();


    QVBoxLayout *vlayout = new QVBoxLayout(q_func());
    vlayout->setMargin(0);
    vlayout->addWidget(m_FaceHomeTitleFrm);
    vlayout->addLayout(layout);
    vlayout->addStretch(5);


    vlayout->addLayout(layout2); //密码 按纽    
    QHBoxLayout *hlayout3 = new QHBoxLayout;
    hlayout3->addStretch();
    hlayout3->addWidget(m_pBottomMessageHintLabel_3); //居中
    hlayout3->addStretch(); 
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
    int syncTime =ReadConfig::GetInstance()->getTimer_Manager_autoTime();  
//   if (syncTime) \
      NtpDateSync::GetInstance()->setSyncNtpDate();
	if (ReadConfig::GetInstance()->getNetwork_Manager_Mode()  == 3)
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
					network4GIP =  QString(strRet.c_str());
				}
			}
			if(network4GIP.size() > 0)
			{
				this->m_pFaceHomeBottomFrm->setNetInfo(network4GIP, myHelper::GetNetworkMac().replace(":",""));
				this->m_FaceHomeTitleFrm->setLinkState(true, 1);
				if (syncTime) \
					NtpDateSync::GetInstance()->setSyncNtpDate();

				return;
			}
		}
	#if 1
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
					network4GIP =  QString(strRet.c_str());
				}
			}
			if(network4GIP.size() > 0)
			{
				this->m_pFaceHomeBottomFrm->setNetInfo(network4GIP, myHelper::GetNetworkMac().replace(":",""));
				this->m_FaceHomeTitleFrm->setLinkState(true, 1);
				if (syncTime) \
					NtpDateSync::GetInstance()->setSyncNtpDate();

				return;
			}
		}    
	#endif 
	}
    QNetworkConfigurationManager mgr;
    //通过QNetworkInterface类来获取本机的IP地址和网络接口信息
    QList<QNetworkInterface> list = QNetworkInterface::allInterfaces();
    //获取所有网络接口的列表
    foreach(QNetworkInterface interface,list)
    {//硬件地址
        QList<QNetworkAddressEntry> entryList = interface.addressEntries();
        //获取IP地址条目列表，每个条目中包含一个IP地址，一个子网掩码和一个广播地址
        foreach(QNetworkAddressEntry entry,entryList)
        {
            if((entry.ip().protocol() == QAbstractSocket::IPv4Protocol) && (!entry.ip().toString().contains("127.0.0")))
            {
                this->m_pFaceHomeBottomFrm->setNetInfo(entry.ip().toString(), interface.hardwareAddress().replace(":", ""));
                if(interface.name() == "eth0")
                    this->m_FaceHomeTitleFrm->setLinkState(mgr.isOnline(), 1);
                else if(interface.name() == "wlan0")
                    this->m_FaceHomeTitleFrm->setLinkState(mgr.isOnline(), 2);
				else if(interface.name() == "usb0")
				{
					if (ReadConfig::GetInstance()->getNetwork_Manager_Mode() !=3) 
					system("ifconfig usb0 down");
				}
                else this->m_FaceHomeTitleFrm->setLinkState(mgr.isOnline(), 3);
                printf(">>>%s,%s,%d,name=%s\n",__FILE__,__func__,__LINE__,interface.name().toStdString().c_str());
                if (mgr.isOnline() && !((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->getAlgoFaceInitState()) //百度算法没激活
                {
                    sleep(10);
                    printf(">>>%s,%s,%d\n",__FILE__,__func__,__LINE__);
                    ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->setRunFaceEngine();		
                }
                printf(">>>%s,%s,%d,getAlgoFaceInitState=%d\n",__FILE__,__func__,__LINE__,((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->getAlgoFaceInitState());


                if (syncTime) \
                    NtpDateSync::GetInstance()->setSyncNtpDate();                  
                return;
            }
            //            qDebug()<<interface.name();
            //            qDebug()<<entry.ip().protocol();
            //            qDebug()<<entry.ip().toString();
        }
    }
    this->m_pFaceHomeBottomFrm->setNetInfo("", "");
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
    int count = 0;
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    query.exec("select * from person");
    if (query.driver()->hasFeature(QSqlDriver::QuerySize))
    {
        count = query.size();
    }
    else
    {
        count = queryRowCount(query);
    }
    this->m_pFaceHomeBottomFrm->setPersonNum(count);
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

    if(!d->m_pHealthCodeFrm->isHidden())d->m_pHealthCodeFrm->hide();
}

void FaceHomeFrm::setTipsMessage(const int &type, const int &pos, const QString &text)
{
    Q_D(FaceHomeFrm);    
    // if (((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->getAlgoFaceInitState())                         
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
            case 3:this->setBottomMessageHintText_3(text);break;
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
