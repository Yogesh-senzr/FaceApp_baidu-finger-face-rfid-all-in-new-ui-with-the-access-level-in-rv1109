#include "FaceMainFrm.h"
#include "SettingFuncFrms/SettingMenuFrm.h"
#include "FaceHomeFrms/FaceHomeFrm.h"
#include "SettingFuncFrms/InputLoginPasswordFrm.h"
#include "OperationTipsFrm.h"
#include "PCIcore/GPIO.h"

#include "Helper/myhelper.h"
#include "vmKeyboardInput/frminput.h"
#include "SettingFuncFrms/ManagingPeopleFrms/AddUserFrm.h"
#include "SettingFuncFrms/ManagingPeopleFrms/CameraPicFrm.h"

#include "MessageHandler/Log.h"

#include <fcntl.h>
#include <unistd.h>
#ifdef Q_OS_LINUX
#include "RkNetWork/NetworkControlThread.h"
#endif
#include "BaiduFace/BaiduFaceManager.h"
#include "Config/ReadConfig.h"
#include "Application/FaceApp.h"
#include "RkNetWork/Network4GControlThread.h"

#include <QMouseEvent>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <unistd.h>
#include <qpainter.h>

class FaceMainFrmPrivate
{
    Q_DECLARE_PUBLIC(FaceMainFrm)
public:
    FaceMainFrmPrivate(FaceMainFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    //设置菜单主页
    SettingMenuFrm *m_pSettingMenuFrm;
    //界面框架主页
    FaceHomeFrm *m_pFaceHomeFrm;
    //新增人员
    AddUserFrm *m_pAddUserFrm;
private:
    InputLoginPasswordFrm *m_pInputLoginPasswordFrm;
private:
    QStackedWidget *m_pStackedWidget;
    double mStart;
    CameraPicFrm *m_pCameraPicFrm;//实时显示相机的图片
    QPixmap m_pixmap;  
    QLabel *mLabel;
private:
    FaceMainFrm *const q_ptr;
    QLabel *m_pUpdateHintLabel;
};


FaceMainFrmPrivate::FaceMainFrmPrivate(FaceMainFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

FaceMainFrm::FaceMainFrm(QWidget *parent)
#ifdef Q_OS_LINUX
    : QWidget(parent, Qt::FramelessWindowHint)
    #else
    : QWidget(parent/*, Qt::FramelessWindowHint*/)
    #endif
    , d_ptr(new FaceMainFrmPrivate(this))
{
    d_func()->m_pCameraPicFrm = new CameraPicFrm(this);//实时显示相机的图片
    d_func()->m_pCameraPicFrm->setFixedSize(800, 1280);
    d_func()->m_pCameraPicFrm->hide();
}

FaceMainFrm::~FaceMainFrm()
{

}

void FaceMainFrmPrivate::InitUI()
{
    m_pStackedWidget = new QStackedWidget;
    m_pAddUserFrm = new AddUserFrm;
    m_pInputLoginPasswordFrm = new InputLoginPasswordFrm(q_func());
	
	printf(">>>>%s,%s,%d, width=%d,height=%d\n",__FILE__,__func__,__LINE__,
	   m_pInputLoginPasswordFrm->width(), m_pInputLoginPasswordFrm->height());
	//if (m_pInputLoginPasswordFrm->width()>=QApplication::desktop()->screenGeometry().width())
	if(DeskTopWidth<=480 && DeskTopHeight<=854)
	{
	  //m_pInputLoginPasswordFrm->setFixedSize(QApplication::desktop()->screenGeometry().width()-400, m_pInputLoginPasswordFrm->height());
	  //m_pInputLoginPasswordFrm->setFixedSize(320 , 300);//400 , 100
	  //m_pInputLoginPasswordFrm->setGeometry(40, DeskTopHeight/2-120 , 320, 240);
	  //m_pInputLoginPasswordFrm->resize(80, 240);
	  m_pInputLoginPasswordFrm->resize(440, 240);
	}

    QHBoxLayout *layout = new QHBoxLayout(q_func());
    layout->setMargin(0);
    layout->addWidget(m_pStackedWidget);

    m_pFaceHomeFrm = new FaceHomeFrm;
    m_pSettingMenuFrm = new SettingMenuFrm;
    m_pStackedWidget->addWidget(m_pFaceHomeFrm);
    m_pStackedWidget->addWidget(m_pSettingMenuFrm);
    m_pStackedWidget->addWidget(m_pAddUserFrm);
#if 0
    mLabel = new QLabel(q_func());
    mLabel->setAttribute(Qt::WA_TranslucentBackground);

    mLabel->setStyleSheet("background:transparent");
#endif 
    //mLabel->setPixmap(QPixmap("/mnt/user/screenshot/SrvSetupFrm.png"));      
    //mLabel->show();
    ///mnt/user/facedb/RegImage.jpeg

    m_pUpdateHintLabel = new QLabel(q_func());
    m_pUpdateHintLabel->move(300,300);
    QFont font ("Microsoft YaHei", 40, 75); //第一个属性是字体（微软雅黑），第二个是大小，第三个是加粗（权重是75）
    m_pUpdateHintLabel->setFont(font);
    
    m_pUpdateHintLabel->setAttribute(Qt::WA_NoSystemBackground); // No background
    m_pUpdateHintLabel->setStyleSheet("color:#FE0000;}");
}



void FaceMainFrmPrivate::InitData()
{
#ifdef Q_OS_LINUX
    q_func()->setAttribute(Qt::WA_TranslucentBackground);
#endif
  mStart = 0;

  //mLabel->show();
   // mLabel->show();
   // mLabel->setGeometry(30,30,740,1220);//0,0,800,1280


}

void FaceMainFrmPrivate::InitConnect()
{
    QObject::connect(m_pSettingMenuFrm, &SettingMenuFrm::sigShowFaceHomeFrm, q_func(), &FaceMainFrm::slotShowFaceHomeFrm);
    QObject::connect(m_pAddUserFrm, &AddUserFrm::sigShowFaceHomeFrm, q_func(), &FaceMainFrm::slotShowFaceHomeFrm);
}

void FaceMainFrm::setHomeDisplay_SnNum(const int &show)
{
    Q_D(FaceMainFrm);
    d->m_pFaceHomeFrm->setHomeDisplay_SnNum(show);
}

void FaceMainFrm::setHomeDisplay_Mac(const int &show)
{
    Q_D(FaceMainFrm);
    d->m_pFaceHomeFrm->setHomeDisplay_Mac(show);
}

void FaceMainFrm::setHomeDisplay_IP(const int &show)
{
    Q_D(FaceMainFrm);
    d->m_pFaceHomeFrm->setHomeDisplay_IP(show);
}

void FaceMainFrm::updateHome_PersonNum()
{
    Q_D(FaceMainFrm);
    FaceApp::connectReady(d->m_pFaceHomeFrm, "onCheckPersonNum()");
}

void FaceMainFrm::setHomeDisplay_PersonNum(const int &show)
{
    Q_D(FaceMainFrm);
    d->m_pFaceHomeFrm->setHomeDisplay_PersonNum(show);
}

void FaceMainFrm::setHomeDisplay_DoorLock(const int &show)
{
    Q_D(FaceMainFrm);
    d->m_pFaceHomeFrm->setHomeDisplay_DoorLock(show);
}

void FaceMainFrm::slotUpDateTip(const QString text)
{
    Q_D(FaceMainFrm);
    //LogD("%s %s[%d],text=%s \n",__FILE__,__FUNCTION__,__LINE__,text.toStdString().c_str());
    //if (((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->getRegFaceState())
    //  return ;
    //AlgorithmDisable
    if(text.length() == 0){
        d->m_pUpdateHintLabel->clear();
        d->m_pUpdateHintLabel->setVisible(false);
    }else if(text.length() > 0){
        d->m_pUpdateHintLabel->setText(text);
        d->m_pUpdateHintLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        d->m_pUpdateHintLabel->adjustSize();
        d->m_pUpdateHintLabel->show();
    }

    if (text.length()==0 && ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->getAlgoFaceInitState())
    {
        LogD("%s %s[%d],text=%s \n",__FILE__,__FUNCTION__,__LINE__,text.toStdString().c_str());
        //d->m_pFaceHomeFrm->setUpDateTip(text); 
    }
	
    if (text.length()>0 &&  !((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->getRegFaceState() )
    {
        LogD("%s %s[%d],text=%s \n",__FILE__,__FUNCTION__,__LINE__,text.toStdString().c_str());
        //d->m_pFaceHomeFrm->setUpDateTip(text);
    }

    if (  text.contains(QObject::tr("UnzipTheFirmware"))   ||  text.contains(QObject::tr("CopyFirmware"))
         ||  text.contains(QObject::tr("update_img_check_md5"))  ||  text.contains(QObject::tr("UpgradingFirmware")) ) ////正在升级固件...,请耐心等待
    {
        //不再识别人脸
        ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->setRegFaceState(true);
    }
    if (text.contains(QObject::tr("FirmwareVerificationFailed"))) //固件校验失败,Firmware verification failed
    {
        //再开始识别人脸
        ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->setRegFaceState(false);
    }    
}

void FaceMainFrm::slotDisMissMessage(const bool state)
{
    Q_D(FaceMainFrm);
    d->m_pFaceHomeFrm->setDisMissMessage(state);

    int mdelay= ReadConfig::GetInstance()->getScreenOutDelay_Value();//60;
    if(mdelay > 0)
    {    
    if (state==0)
    {
        int mdelay= ReadConfig::GetInstance()->getScreenOutDelay_Value();//60;
        if (d->mStart ==0) d->mStart= (double)clock();
        if (d->mStart>0)
        {
            int passtimer = ((double)clock()- d->mStart)/1000/1000;
            if (passtimer>= mdelay) 
            {              // 熄屏,关喇叭
                YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Light_White, 0);            
                YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_LCD_BL, 0);
                d->mStart = 0;
            }
        }        
    } else if (state==1)
    {
        d->mStart = 0;
        if (ReadConfig::GetInstance()->getFillLight_Value() > 0 )
           YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Light_White, 1);        
        YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_LCD_BL, 1);
    }    
    }
}

void FaceMainFrm::slotTipsMessage(const int type, const int pos, const QString text)
{
    Q_D(FaceMainFrm);
    d->m_pFaceHomeFrm->setTipsMessage(type, pos, text);
}

void FaceMainFrm::slotShowAlgoStateAboutFace(const QString text)
{
    Q_D(FaceMainFrm);
    d->m_pFaceHomeFrm->setAlgoStateAboutFace(text);
}

void FaceMainFrm::slotHealthCodeInfo(const int type, const QString name, const QString idCard, const int qrCodeType, const double /*warningTemp*/, const QString msg)
{
    Q_D(FaceMainFrm);
    d->m_pFaceHomeFrm->setHealthCodeInfo(type, name, idCard, qrCodeType, msg);
}

void FaceMainFrm::slotShowFaceHomeFrm(const int index)
{
    Q_D(FaceMainFrm);

    if(!index)
    {
#ifdef Q_OS_LINUX
        ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->setRegFaceState(false);
#endif
        FaceApp::connectReady(d->m_pFaceHomeFrm, "onCheckNet()");
        FaceApp::connectReady(d->m_pFaceHomeFrm, "onCheckPersonNum()");
    }
    d->m_pStackedWidget->setCurrentIndex(index);
  
}
#define F_OK (0)
void FaceMainFrm::onReady()
{
    Q_D(FaceMainFrm);

    LogD("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
    if(DeskTopWidth>=800 && DeskTopHeight<=1280)
        this->setStyleSheet(myHelper::setStyleSheet(":/CSS/AppStyle800×1280.css"));
    else if(DeskTopWidth>=720 && DeskTopHeight<=1280)
        this->setStyleSheet(myHelper::setStyleSheet(":/CSS/AppStyle720×1280.css"));
    else if(DeskTopWidth>=600 && DeskTopHeight<=1024)
        this->setStyleSheet(myHelper::setStyleSheet(":/CSS/AppStyle600×1024.css"));
    else if(DeskTopWidth>=480 && DeskTopHeight<=854)
        this->setStyleSheet(myHelper::setStyleSheet(":/CSS/AppStyle480×854.css"));        
    printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
#ifndef Q_OS_LINUX
    printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
    this->setFixedSize(800, 800);
#endif
    printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
    this->show();

    int Mode = ReadConfig::GetInstance()->getNetwork_Manager_Mode();
    LogD("%s %s[%d] Mode %d \n",__FILE__,__FUNCTION__,__LINE__,Mode);
#if 0    
    if (Mode == 2 )
    {
        if(access("/sys/class/net/wlan0/",F_OK))
        {
    	    //listName<<QObject::tr("Wi-FiSetup"); //Wi-Fi设置
            ReadConfig::GetInstance()->setNetwork_Manager_Mode(1);
            Mode = 1 ;
        }
    }
    if (Mode == 3 )
    {
        if(access("/sys/class/net/p2p0/",F_OK))
        {
            //listName<<QObject::tr("4G"); //4G设置
            ReadConfig::GetInstance()->setNetwork_Manager_Mode(1);
            Mode = 1 ;            
        }        
    }
#endif     
    LogD("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
    NetworkControlThread::GetInstance()->setNetworkType(Mode);
    QString Name = ReadConfig::GetInstance()->getWIFI_Name();
    QString pasw = ReadConfig::GetInstance()->getWIFI_Password();
    NetworkControlThread::GetInstance()->setLinkWlanSSID(Name, pasw);
    LogD("%s %s[%d] Network4GControlThread::GetInstance()\n",__FILE__,__FUNCTION__,__LINE__);
    //Network4GControlThread::GetInstance();

    char szMac[60]={0};
    
    if(Mode == 1)
    {//LAN 测试发现lan下， 无需重复配置也可以使用
        //        NetworkControlThread::GetInstance()->setLinkLan(ReadConfig::GetInstance()->getLan_DHCP(), ReadConfig::GetInstance()->getLAN_IP(),
        //                                                        ReadConfig::GetInstance()->getLAN_Maks(), ReadConfig::GetInstance()->getLAN_Gateway(),
        //                                                        ReadConfig::GetInstance()->getLAN_DNS());
  
    }else if(Mode == 2)
    {//wlan
        if(!Name.isEmpty())
        {
            NetworkControlThread::GetInstance()->DisconnectAllWifi();
            NetworkControlThread::GetInstance()->setWifiSearchMode(2);
            NetworkControlThread::GetInstance()->resume();
        }
    }else if(Mode == 3)
    {// 4G
#if 1
        sprintf(szMac,"ifconfig eth0 hw ether %s",myHelper::GetNetworkMac().toStdString().c_str());        
        myHelper::Utils_ExecCmd("ifconfig eth0 down;");
        myHelper::Utils_ExecCmd(szMac);
        myHelper::Utils_ExecCmd("ifconfig eth0 up;");    
        
#endif 
    	Network4GControlThread::GetInstance();
        
    }

    FaceApp::connectReady(d->m_pFaceHomeFrm, "onCheckNet()");
    FaceApp::connectReady(d->m_pFaceHomeFrm, "onCheckSN()");
    FaceApp::connectReady(d->m_pFaceHomeFrm, "onCheckPersonNum()");

#ifdef Q_OS_LINUX
    QTimer::singleShot(3500, this, [&]{
        if(((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->getAlgoFaceInitState() != true) d_func()->m_pFaceHomeFrm->setUpDateTip(QObject::tr("AlgorithmDisable"));//算法未激活 Algorithm is not activated.
    });
#endif
    qDebug()<<"DeskW:"<<DeskTopWidth<<" DeskH:"<<DeskTopHeight;
   
}


void FaceMainFrm::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_D(FaceMainFrm);
    auto pos = event->pos();
#if 0// def SCREENCAPTURE  //ScreenCapture 

    mPath = ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->getCurFaceImgPath();
    mDraw = true;     
    printf(">>>>%s,%s,%d,mPath=%s, mDraw=%d\n",__FILE__,__func__,__LINE__, mPath.toStdString().c_str(), mDraw);     
    QImage img = QImage(mPath);///mnt/user/facedb/RegImage.jpeg
    d->m_pixmap = QPixmap::fromImage(img);  
    update();
    grab().save(QString("/mnt/user/screenshot/painterFaceMainFrm.png"),"png");    
#endif     
  
    if(pos.x()<80 && pos.y()<80 && (0 == d->m_pStackedWidget->currentIndex()))
    {
#ifdef Q_OS_LINUX
        ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->setRegFaceState(true);
#endif
        d->m_pInputLoginPasswordFrm->show();
        int ret = d->m_pInputLoginPasswordFrm->exec();
        if(ret == 0)d->m_pStackedWidget->setCurrentIndex(1);
#ifdef Q_OS_LINUX
        else ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->setRegFaceState(false);
#endif
	
		if(ret == 0) {
			setStatus(1);
		}


    }
    
    QWidget::mouseDoubleClickEvent(event);
}




void FaceMainFrm::mousePressEvent(QMouseEvent *event)
{
#if 0
   Q_D(FaceMainFrm);
    static int clicktimer = 0;

    printf(">>>>>>>>>%s,%s,%d,\n",__FILE__,__func__,__LINE__);
    auto pos = event->pos();
    if(pos.x()<80 && pos.y()<80 && (0 == d->m_pStackedWidget->currentIndex()))
    {
        if  (clicktimer ==0) 
           clicktimer = (double)clock(); ///1000/1000;  
        else  if (clock() - clicktimer <1000 ) 
        {
            // 双击
    #ifdef Q_OS_LINUX
            ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->setRegFaceState(true);
    #endif
            d->m_pInputLoginPasswordFrm->show();
            int ret = d->m_pInputLoginPasswordFrm->exec();
            if(ret == 0)d->m_pStackedWidget->setCurrentIndex(1);
    #ifdef Q_OS_LINUX
            else ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->setRegFaceState(false);
    #endif
        
            if(ret == 0) {
                setStatus(1);
            }            
        }
    }    
    printf(">>>>>>>>>%s,%s,%d,\n",__FILE__,__func__,__LINE__);	
#endif 
#if 1
  if (event->button()==Qt::LeftButton)
  {
        printf(">>>>>>%s,%s,%d, \n",__FILE__, __func__, __LINE__);    
  }

   Q_D(FaceMainFrm);
    auto pos = event->pos();
  
    printf(">>>>>>%s,%s,%d,x=%d ,y=%d,currentIndex=%d \n",__FILE__, __func__, __LINE__,pos.x(), pos.y(),d->m_pStackedWidget->currentIndex());
	if (DeskTopWidth<= 480  && DeskTopHeight<=854) 
	{
	    if(pos.x()<80 && pos.y()<80 && (0 == d->m_pStackedWidget->currentIndex()))
	    {
	#ifdef Q_OS_LINUX
	        ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->setRegFaceState(true);
	#endif
	        d->m_pInputLoginPasswordFrm->show();
	        int ret = d->m_pInputLoginPasswordFrm->exec();
	        if(ret == 0)d->m_pStackedWidget->setCurrentIndex(1);
	#ifdef Q_OS_LINUX
	        else ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->setRegFaceState(false);
	#endif
	
			if(ret == 0) {
				setStatus(1);
			}


	    }
	}
#endif 
}

void FaceMainFrm::paintEvent(QPaintEvent *event)
{

    Q_D(FaceMainFrm);
#ifdef SCREENCAPTURE  //ScreenCapture 
    //printf(">>>>%s,%s,%d,mPath=%s, mDraw=%d\n",__FILE__,__func__,__LINE__, mPath.toStdString().c_str(), mDraw); 
    if (!mPath.isEmpty() && mDraw)
    {
        mDraw = false;
        Q_UNUSED(event);  
        QPainter painter(this);  
        QPixmap pix;
        pix.load(mPath);
        painter.drawPixmap(0,0,800,1280,pix);//0,0,600,800 0,0,800,1280
    printf(">>>>%s,%s,%d\n",__FILE__,__func__,__LINE__);        
        grab().save(QString("/mnt/user/screenshot/painterFaceMainFrm.png"),"png"); 
    }
#endif 
}
