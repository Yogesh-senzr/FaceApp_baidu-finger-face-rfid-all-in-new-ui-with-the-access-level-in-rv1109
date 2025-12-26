
#if _MSC_VER >= 1600    // VC2010
#pragma  execution_character_set("UTF-8")
#endif
#include "FaceApp.h"

#include "FaceMainFrm.h"
#include "SharedInclude/CallBindDef.h"

#include "RKCamera/Camera/cameramanager.h"
#include "BaseFace/BaseFaceManager.h"
#include "BaiduFace/BaiduFaceManager.h"
#include "BaiduFace/FingerprintManager.h"

#include "PCIcore/SensorManager.h"
#include "PCIcore/Audio.h"
#include "PCIcore/GPIO.h"
#include "PCIcore/Watchdog.h"
#include "PCIcore/UsbIcCardObserver.h"
#include "PCIcore/Display.h"
#include "USB/UsbObserver.h"
#include "PCIcore/WiegandObserver.h"
#include <unistd.h>
#include <sys/stat.h>

#include "MessageHandler/Log.h"
#include "HealthCodeDevices/HealthCodeManage.h"
#include "IDCardDevices/IdentityCardManage.h"
#include "IDCard_ZK/IdentityCard_ZK_Manage.h"
#include "IDCard_diandu/IdentityCard_DD_Manage.h"
#include "Helper/myhelper.h"

#include "HealthCodeDevices/LRHealthCode.h"
#include "DB/FaceDB.h"
#include "DB/RegisteredFacesDB.h"
#include "Config/ReadConfig.h"
#include "Threads/powerManagerThread.h"
#include "Threads/ParsePersonXlsx.h"
#include "Threads/RecordsExport.h"
#include "Threads/WatchDogManageThread.h"
#include "Threads/ShrinkFaceImageThread.h"
#include "HttpServer/ConnHttpServerThread.h"
#include "HttpServer/PostPersonRecordThread.h"
#include "HttpServer/UdpBroadcastThread.h"

#include "ManageEngines/IdentityManagement.h"
#include "ManageEngines/HaoyuIdentityManagement.h"
#include "ManageEngines/ZhangjiakoudentityManagement.h"



#include <QtCore/QMetaMethod>
#include <QtWidgets/QStyleFactory>
#include <QtCore/QTimer> 
#include <QtCore/QDebug>
#include <QtWidgets/QSplashScreen>
#include <QtCore/QElapsedTimer>
#include <QtGui/QFontDatabase>
#include <QtCore/QResource>
#include <QtCore/QTime>
#include <QtConcurrent/QtConcurrent>


class FaceAppPrivate
{
	Q_DECLARE_PUBLIC(FaceApp)
public:
	FaceAppPrivate(FaceApp *ptr);
	~FaceAppPrivate();
private:
	void InitIdentityManagement();
	void InitTopUI();
	void InitUI();
	void InitCameraManage();
	void InitFaceManager();
	//void InitArcsoftFaceManager(); //合并在 InitFaceManager
	void InitPICcore();
	void InitCallBack();
	void InitFingerprintManager(); 
	void InitData();
	void InitConnect();
	QString GetParam(char *szParamName ); //1.身份证
private:
	FaceMainFrm *m_pFaceMainFrm;
private:
	CameraManager *m_pCameraManager;
	BaseFaceManager *m_pBaseFaceManager;
	//ArcsoftFaceManager *m_pArcsoftFaceManager;
	BaiduFaceManager *m_pBaiduFaceManager;
	SensorManager *m_pSensorManager;
	//识别管理
	IdentityManagement *m_pIdentityManagement;
	//处理批量人员导入导出
	ParsePersonXlsx *m_pParsePersonXlsx;
	//处理批量导出记录表的
	RecordsExport *m_pRecordsExport;
	PostPersonRecordThread *m_pPostPersonRecordThread = nullptr;
	//电源管理
	powerManagerThread *m_pPowerManagerThread;
	//国康码
	HealthCodeManage *m_pHealthCodeManage;
	LRHealthCode *m_pLRHealthCode;
	//身份证
	IdentityCardManage *m_pIdentityCardManage;
	IdentityCard_ZK_Manage *m_pIdentityCard_ZK_Manage; //ZK
	IdentityCard_DD_Manage *m_pIdentityCard_DD_Manage; //DD
	RegisteredFacesDB* m_registeredFacesDB;
	FingerprintManager *m_pFingerprintManager;  // NEW: Add this line
	QTimer *m_pFingerprintHealthTimer;
	void startFingerprintHealthMonitoring();
private:
	FaceApp * const q_ptr;
private:
	bool _ready;
};

FaceAppPrivate::FaceAppPrivate(FaceApp *ptr) :
		q_ptr(ptr) //
				, m_pHealthCodeManage(new HealthCodeManage) //
				, m_pIdentityCardManage(new IdentityCardManage) //
			//	, m_pIdentityCard_ZK_Manage(new IdentityCard_ZK_Manage) //
			//	, m_pIdentityCard_DD_Manage(new IdentityCard_DD_Manage) //
				, m_pParsePersonXlsx(new ParsePersonXlsx) //
				, m_pRecordsExport(new RecordsExport) //
				, m_pPostPersonRecordThread(new PostPersonRecordThread) //
				, m_pFingerprintManager(nullptr)  // NEW: Initialize to nullptr
				, m_pFingerprintHealthTimer(nullptr)
				, m_pPowerManagerThread(new powerManagerThread) //
				, _ready(true) //
{
	QString nParam = GetParam("IDENTITYCARD");
	switch(nParam.toInt())
	{
		case 0:
			LogD(">>>%s,%s,%d\n",__FILE__,__func__,__LINE__);
		  m_pIdentityCard_ZK_Manage = new IdentityCard_ZK_Manage(q_func());
		  break;
		case 1: //点度
			LogD(">>>%s,%s,%d\n",__FILE__,__func__,__LINE__);
						
		  m_pIdentityCard_DD_Manage = new IdentityCard_DD_Manage(q_func()); //
		  break;		  
		default:
		   m_pIdentityCard_ZK_Manage = new IdentityCard_ZK_Manage(q_func());
		   break;

	}

	Log_Init(); //日记库
	this->InitIdentityManagement();
	this->InitUI();
	this->InitTopUI();	
	this->InitFaceManager();
	//this->InitArcsoftFaceManager();//合并在InitFaceManager
	this->InitCameraManage();
	this->InitPICcore();
	this->InitCallBack();
	this->InitData();
	this->InitConnect();
     this->InitFingerprintManager();

}

FaceAppPrivate::~FaceAppPrivate()
{
	 // 🆕 ADD: Stop health monitoring
    if (m_pFingerprintHealthTimer) {
        m_pFingerprintHealthTimer->stop();
        m_pFingerprintHealthTimer->deleteLater();
        m_pFingerprintHealthTimer = nullptr;
    }
    
    // 🆕 ADD: Stop fingerprint monitoring
    if (m_pFingerprintManager) {
        m_pFingerprintManager->stopContinuousMonitoring();
    }

	m_pFaceMainFrm->deleteLater();
}

FaceApp::FaceApp(int &argc, char **argv) :
		QApplication(argc, argv), d_ptr(new FaceAppPrivate(this))
{
	Q_D(FaceApp);
	ReadConfig::GetInstance()->setReadConfig(); //读取配置信息	
	YNH_LJX::Display::Display_SetScreenBrightness(ReadConfig::GetInstance()->getLuminance_Value());
	FaceApp::connectReady(d_func()->m_pFaceMainFrm, "onReady()");

}

FaceApp::~FaceApp()
{
}

void FaceAppPrivate::InitUI()
{
	m_pFaceMainFrm = new FaceMainFrm;
}

void FaceAppPrivate::InitIdentityManagement()
{

	int type = ReadConfig::GetInstance()->getPlatformType();
	switch (type)
	{
	case 0: //默认平台
		m_pIdentityManagement = new IdentityManagement;
		break;
	case 1: //浩宇
		m_pIdentityManagement = new HaoyuIdentityManagement;
		break;
	case 2: //张家口
		m_pIdentityManagement = new ZhangjiakoudentityManagement;
		break;
	default:
		m_pIdentityManagement = new IdentityManagement;
		break;
	}
}

void FaceAppPrivate::InitCameraManage()
{	

	m_pCameraManager = new CameraManager(QSize(640, 480), false, ReadConfig::GetInstance()->getRgbCameraRotation(), QSize(640, 480),
			false, ReadConfig::GetInstance()->getIrCameraRotation(), DeskTopWidth, DeskTopHeight);

	m_pCameraManager->startPreview(true, QSize(DeskTopWidth, DeskTopHeight));

	m_pCameraManager->startAiProcess();

}



void FaceAppPrivate::InitFaceManager()
{
	int type = 1; //ReadConfig::GetInstance()->getFaceRecognVedor();

	switch (type)
	{
		case 0: //虹软
			
			break;
			
		case 1: //百度
			m_pBaiduFaceManager = new BaiduFaceManager;
			m_pBaiduFaceManager->setDeskSize(DeskTopWidth, DeskTopHeight);
			m_pBaiduFaceManager->setRunFaceEngine();
			break;
		default:
			m_pBaiduFaceManager = new BaiduFaceManager;
			m_pBaiduFaceManager->setDeskSize(DeskTopWidth, DeskTopHeight);
			//m_pUnifiedFaceManager->setRunFaceEngine();
			m_pBaiduFaceManager->startInitFaceEngineThread();			
			break;
	}

}

void FaceAppPrivate::InitPICcore()
{

	if (!access("/isc/boarttest_ok", F_OK))
	{
		LogV("%s %s[%d] return for /isc/boarttest_ok !\n", __FILE__, __FUNCTION__, __LINE__);
		exit(0);
		return;
	}
//	if (access("/mnt/user/facedb", F_OK) != 0)
	{
		mkdir("/mnt/user/facedb", 0777);
		mkdir("/mnt/user/facedb/img", 0777);
		mkdir("/mnt/user/face_crop_image", 0777);
	}
	system("chmod 777 /dev/wiegand_input");
	system("chmod 777 /dev/wiegand_output");

	myHelper::Utils_ExecCmd("amixer set 'Playback Path' 'HP'");
	//初始化音频输出
	YNH_LJX::Audio::Audio_InitDev();

	YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_5V, 1);
	//智慧需要拉高12V
	YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_12V, 1);
	YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Light_InfraredLed, 1);
	YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_LCD_BL, 1);
	//温度传感器
	m_pSensorManager = new SensorManager;
	m_pSensorManager->setRunSensor(ReadConfig::GetInstance()->getTempSensorType());
	//串口粤康码
	LogV("%s %s[%d] LRHealthCode \n", __FILE__, __FUNCTION__, __LINE__);
	m_pLRHealthCode = new LRHealthCode;
	LogV("%s %s[%d] LRHealthCode \n", __FILE__, __FUNCTION__, __LINE__);
	//喂狗线程
	WatchDogManageThread::GetInstance();

	//网络服务线程
	UdpBroadcastThread::GetInstance();
	ConnHttpServerThread::GetInstance();
}

void FaceAppPrivate::InitFingerprintManager()
{
    LogD("%s %s[%d] ╔═══════════════════════════════════════╗\n", 
         __FILE__, __FUNCTION__, __LINE__);
    LogD("%s %s[%d] ║  INITIALIZING FINGERPRINT MANAGER    ║\n", 
         __FILE__, __FUNCTION__, __LINE__);
    LogD("%s %s[%d] ╚═══════════════════════════════════════╝\n", 
         __FILE__, __FUNCTION__, __LINE__);
    
    // Step 1: Get singleton instance (no hardware init yet)
    m_pFingerprintManager = FingerprintManager::GetInstance();
    
    if (!m_pFingerprintManager) {
        LogE("%s %s[%d] ❌ ERROR: Failed to get FingerprintManager instance\n", 
             __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    
    LogD("%s %s[%d] ✅ FingerprintManager instance created\n", 
         __FILE__, __FUNCTION__, __LINE__);
    
    // ✅ CRITICAL FIX: Connect signals IMMEDIATELY after manager creation
    // This ensures the connections exist BEFORE any signals are emitted
    LogD("%s %s[%d] ╔═══════════════════════════════════════╗\n", 
         __FILE__, __FUNCTION__, __LINE__);
    LogD("%s %s[%d] ║  CONNECTING FINGERPRINT SIGNALS      ║\n", 
         __FILE__, __FUNCTION__, __LINE__);
    LogD("%s %s[%d] ╚═══════════════════════════════════════╝\n", 
         __FILE__, __FUNCTION__, __LINE__);
    
    // CONNECTION 1: FingerprintManager → IdentityManagement
    QObject::connect(
        m_pFingerprintManager, 
        static_cast<void(FingerprintManager::*)(const QString&, const QString&, const int&)>(
            &FingerprintManager::sigFingerprintMatched),
        m_pIdentityManagement, 
        &IdentityManagement::slotFingerprintMatched, 
        Qt::QueuedConnection
    );
    
    QObject::connect(
        m_pFingerprintManager,
        &FingerprintManager::sigFingerprintNotRecognized,
        m_pIdentityManagement,
        &IdentityManagement::slotFingerprintNotRecognized,
        Qt::QueuedConnection
    );
    
    LogD("%s %s[%d] ✅ FingerprintManager → IdentityManagement connected\n", 
         __FILE__, __FUNCTION__, __LINE__);
    
    // CONNECTION 2: IdentityManagement → FaceMainFrm (for popup)
    QObject::connect(
        m_pIdentityManagement,
        &IdentityManagement::sigRecognizedPerson,
        m_pFaceMainFrm,
        &FaceMainFrm::slotDisplayRecognizedPerson,
        Qt::QueuedConnection
    );
    
    LogD("%s %s[%d] ✅ IdentityManagement → FaceMainFrm connected\n", 
         __FILE__, __FUNCTION__, __LINE__);
    
    LogD("%s %s[%d] ╔═══════════════════════════════════════╗\n", 
         __FILE__, __FUNCTION__, __LINE__);
    LogD("%s %s[%d] ║  ✅ ALL SIGNALS CONNECTED!           ║\n", 
         __FILE__, __FUNCTION__, __LINE__);
    LogD("%s %s[%d] ╚═══════════════════════════════════════╝\n", 
         __FILE__, __FUNCTION__, __LINE__);
    
    // Step 2: Initialize sensor in background thread after delay
    QTimer::singleShot(3000, q_ptr, [this]() {
        LogD("%s %s[%d] === Starting fingerprint sensor initialization (background thread) ===\n", 
             __FILE__, __FUNCTION__, __LINE__);
        
        // Run in background thread to avoid blocking main thread
        QtConcurrent::run([this]() {
            try {
                LogD("Attempting to initialize fingerprint sensor...\n");
                
                // Initialize the sensor (this may take time)
                bool initSuccess = m_pFingerprintManager->initFingerprintSensor();
                
                if (initSuccess) {
                    LogD("%s %s[%d] ✅ Fingerprint sensor initialized successfully\n", 
                         __FILE__, __FUNCTION__, __LINE__);
                    
                    // Only start monitoring AFTER successful init
                    QTimer::singleShot(0, q_ptr, [this]() {
                        if (m_pFingerprintManager && m_pFingerprintManager->isSensorReady()) {
                            LogD("╔═══════════════════════════════════════╗\n");
                            LogD("║  STARTING CONTINUOUS MONITORING       ║\n");
                            LogD("╚═══════════════════════════════════════╝\n");
                            
                            m_pFingerprintManager->startContinuousMonitoring();
                            
                            LogD("✅ Monitoring started successfully\n");
                            
                            // Start health check timer
                            startFingerprintHealthMonitoring();
                        } else {
                            LogE("❌ Cannot start monitoring - sensor not ready\n");
                        }
                    });
                    
                } else {
                    LogE("%s %s[%d] ⚠️ Fingerprint sensor initialization FAILED\n", 
                         __FILE__, __FUNCTION__, __LINE__);
                    
                    // Retry initialization after delay
                    QTimer::singleShot(0, q_ptr, [this]() {
                        QTimer::singleShot(15000, q_ptr, [this]() {
                            LogD("🔄 Retrying fingerprint sensor initialization...\n");
                            InitFingerprintManager();
                        });
                    });
                }
                
            } catch (const std::exception& e) {
                LogE("%s %s[%d] ❌ Exception during fingerprint init: %s\n", 
                     __FILE__, __FUNCTION__, __LINE__, e.what());
            }
        });
    });
    
    LogD("%s %s[%d] Fingerprint manager created, sensor will initialize in 3 seconds\n", 
         __FILE__, __FUNCTION__, __LINE__);
}

void FaceAppPrivate::startFingerprintHealthMonitoring()
{
    LogD("╔═══════════════════════════════════════╗\n");
    LogD("║  🏥 STARTING UART HEALTH MONITORING   ║\n");
    LogD("╚═══════════════════════════════════════╝\n");
    
    if (!m_pFingerprintManager) {
        LogE("❌ Cannot start health monitoring - manager is null\n");
        return;
    }
    
    // Clean up existing timer if any
    if (m_pFingerprintHealthTimer) {
        m_pFingerprintHealthTimer->stop();
        m_pFingerprintHealthTimer->deleteLater();
    }
    
    // Create new health check timer
    m_pFingerprintHealthTimer = new QTimer(q_ptr);
    
    QObject::connect(m_pFingerprintHealthTimer, &QTimer::timeout, q_ptr, [this]() {
        if (!m_pFingerprintManager) {
            return;
        }
        
        // Perform health check (this will auto-recover if needed)
        if (!m_pFingerprintManager->checkUARTHealth()) {
            LogE("⚠️⚠️⚠️ Fingerprint UART health check FAILED!\n");
            LogE("   Auto-recovery attempt in progress...\n");
            
            // Check if sensor is completely dead
            if (!m_pFingerprintManager->isSensorReady()) {
                LogE("❌ Sensor marked as NOT READY\n");
                LogE("   Stopping monitoring and attempting full re-initialization...\n");
                
                // Stop monitoring
                m_pFingerprintManager->stopContinuousMonitoring();
                
                // Stop health timer temporarily
                if (m_pFingerprintHealthTimer) {
                    m_pFingerprintHealthTimer->stop();
                }
                
                // Wait 5 seconds then retry full initialization
                QTimer::singleShot(5000, q_ptr, [this]() {
                    LogD("🔄 Attempting full sensor re-initialization...\n");
                    
                    // Re-run initialization
                    QtConcurrent::run([this]() {
                        bool success = m_pFingerprintManager->initFingerprintSensor();
                        
                        if (success) {
                            LogD("✅ Re-initialization successful!\n");
                            
                            // Restart monitoring in main thread
                            QTimer::singleShot(0, q_ptr, [this]() {
                                if (m_pFingerprintManager && m_pFingerprintManager->isSensorReady()) {
                                    m_pFingerprintManager->startContinuousMonitoring();
                                    
                                    // Restart health timer
                                    if (m_pFingerprintHealthTimer) {
                                        m_pFingerprintHealthTimer->start(30000);
                                    }
                                }
                            });
                        } else {
                            LogE("❌ Re-initialization failed, will retry in 15 seconds\n");
                            
                            QTimer::singleShot(0, q_ptr, [this]() {
                                QTimer::singleShot(15000, q_ptr, [this]() {
                                    InitFingerprintManager();
                                });
                            });
                        }
                    });
                });
            }
        } else {
            // Health check passed
            static int healthCheckCount = 0;
            healthCheckCount++;
            
            if (healthCheckCount % 10 == 0) {  // Log every 10 checks (5 minutes)
                LogD("✅ Fingerprint UART health check passed (%d checks)\n", healthCheckCount);
            }
        }
    });
    
    // Check every 30 seconds
    m_pFingerprintHealthTimer->start(30000);
    
    LogD("✅ Health monitoring started\n");
    LogD("   Interval: 30 seconds\n");
    LogD("   Auto-recovery: Enabled\n");
}


void FaceAppPrivate::InitCallBack()
{

	int type = 1; //ReadConfig::GetInstance()->getFaceRecognVedor();
	switch (type)
	{		
		case 0: //虹软
		  break;
		case 1://百度
		  m_pCameraManager->setCameraPreviewYUVCallBack(CC_CALLBACK_13(BaiduFaceManager::setCameraPreviewYUVData, m_pBaiduFaceManager));
		  break;
		default:
		  m_pCameraManager->setCameraPreviewYUVCallBack(CC_CALLBACK_13(BaiduFaceManager::setCameraPreviewYUVData, m_pBaiduFaceManager));
		  break;
	}
}

void FaceAppPrivate::InitTopUI()
{
}

void FaceAppPrivate::InitData()
{

	FaceDB::GetFaceDB();
	RegisteredFacesDB::GetInstance();
	qRegisterMetaType<QList<CORE_FACE_RECT_S>>("QList<CORE_FACE_RECT_S>");
	qRegisterMetaType<CORE_FACE_S>("CORE_FACE_S");

	m_pFaceMainFrm->setFixedSize(DeskTopWidth, DeskTopHeight);
	QFontDatabase::addApplicationFont("isc/res/ko/gulim.ttc");

	int nIndex = QFontDatabase::addApplicationFont("/usr/local/share/fonts/msyh.ttc");
	if (nIndex >= 0)
	{
		QStringList strFontList(QFontDatabase::applicationFontFamilies(nIndex));
		if (strFontList.count() > 0)
		{
			QFont font(strFontList.at(0));
			font.setPointSize(12);
			QApplication::setFont(font);
		}
	}
}
QString FaceAppPrivate::GetParam(char *szParamName )
{
    //"IDENTITYCARD" //身份证模块　
    //0:是以前的身份证模块;1:点度身份证模块
	if (strstr(szParamName, "IDENTITYCARD"))
	{
//	algo_type=$(cat /param/RV1109_PARAM.txt | grep ALGO_TYPE | awk -F"=" '{ print $2 }' | sed 's/[[:space:]]//g' )
	//algo_type=${algo_type:0:1}  

 		QString str = "";
        FILE *pFile = popen("cat /param/RV1109_PARAM.txt | grep IDENTITYCARD | awk -F\"=\" '{ print $2 }' | sed 's/[[:space:]]//g'", "r");
        if (pFile)
        {
        	std::string ret = "";
    		char buf[256] = { 0 };
    		int readSize = 0;
    		do
    		{
    			readSize = fread(buf, 1, sizeof(buf), pFile);
    			if (readSize > 0)
    			{
    				ret += std::string(buf, 0, readSize);
    			}
    		} while (readSize > 0);
            pclose(pFile);

            str = QString::fromStdString(ret);
			str = str.mid(0,1);
			printf(">>>%s,%s,%d,IDENTITYCARD=%d\n",__FILE__,__func__,__LINE__,str.toInt());
            
        }  
		return str;   
	}

	return "0";
}

void FaceAppPrivate::InitConnect()
{
    int type = 1;  //ReadConfig::GetInstance()->getFaceRecognVedor();
    switch (type)
    {		
        case 0: //虹软	
        break;
        case 1://百度
        {
            QObject::connect(m_pBaiduFaceManager, &BaiduFaceManager::sigDrawFaceRect, m_pCameraManager, &CameraManager::slotDrawFaceRect,\
                    Qt::QueuedConnection);
            QObject::connect(m_pBaiduFaceManager, &BaiduFaceManager::sigDisMissMessage, m_pFaceMainFrm, &FaceMainFrm::slotDisMissMessage,\
                    Qt::QueuedConnection);
            QObject::connect(m_pBaiduFaceManager, &BaiduFaceManager::sigDisMissMessage, m_pCameraManager, &CameraManager::slotDisMissMessage,\
                    Qt::QueuedConnection);			
            QObject::connect(m_pBaiduFaceManager, &BaiduFaceManager::sigDisMissMessage, m_pIdentityManagement,
                    &IdentityManagement::slotDisMissMessage, Qt::QueuedConnection);			
            QObject::connect(m_pBaiduFaceManager, &BaiduFaceManager::sigMatchCoreFace, m_pIdentityManagement,
                    &IdentityManagement::slotMatchCoreFace, Qt::QueuedConnection);
            QObject::connect(m_pBaiduFaceManager, &BaiduFaceManager::sigRecognizedPerson, m_pFaceMainFrm, 
                &FaceMainFrm::slotDisplayRecognizedPerson, Qt::QueuedConnection);
        }
        break;
    }

    printf("%s %s[%d] UsbICCardObserver %p IdentityManagement %p \n",__FILE__,__FUNCTION__,__LINE__,UsbICCardObserver::GetInstance(),m_pIdentityManagement);
    QObject::connect(UsbICCardObserver::GetInstance(), &UsbICCardObserver::sigReadIccardNum, m_pIdentityManagement,
            &IdentityManagement::slotReadIccardNum, Qt::QueuedConnection);
    QObject::connect(WiegandObserver::GetInstance(), &WiegandObserver::sigReadIccardNum, m_pIdentityManagement,
            &IdentityManagement::slotReadIccardNum, Qt::QueuedConnection);
    QObject::connect(UsbObserver::GetInstance(), &UsbObserver::sigUpDateTip, m_pFaceMainFrm, &FaceMainFrm::slotUpDateTip,
            Qt::QueuedConnection);
    
    // ❌ REMOVE THE OLD FINGERPRINT CONNECTIONS FROM HERE
    // They are now in InitFingerprintManager() where they belong!
    
    QObject::connect(m_pSensorManager, &SensorManager::sigTemperatureValue, m_pIdentityManagement,
            &IdentityManagement::slotTemperatureValue, Qt::QueuedConnection);

    QObject::connect(m_pHealthCodeManage, &HealthCodeManage::sigHealthCodeInfo, m_pIdentityManagement,
            &IdentityManagement::slotHealthCodeInfo, Qt::QueuedConnection);
                        
    QObject::connect(m_pBaiduFaceManager, &BaiduFaceManager::sigDisMissMessage, m_pPowerManagerThread, \
            &powerManagerThread::slotDisMissMessage, Qt::QueuedConnection);

    QString nParam = GetParam("IDENTITYCARD");
    switch(nParam.toInt())
    {
        case 0:
            QObject::connect(m_pIdentityCardManage, &IdentityCardManage::sigIdentityCardInfo, m_pIdentityManagement,\
                    &IdentityManagement::slotIdentityCardInfo, Qt::QueuedConnection);	
            QObject::connect(m_pIdentityCard_ZK_Manage, &IdentityCard_ZK_Manage::sigZKIdentityCardFullInfo, (IdentityManagement *)m_pIdentityManagement,\
                    &IdentityManagement::slotIdentityCardFullInfo, Qt::QueuedConnection);	
            break;
        case 1: //点度
            QObject::connect(m_pIdentityCard_DD_Manage, &IdentityCard_DD_Manage::sigIdentityCardInfoDD, m_pIdentityManagement,\
                    &IdentityManagement::slotIdentityCardInfo, Qt::QueuedConnection);		
            break;		  
    }

    // 身份证信息传递到健康码中查询健康码
    QObject::connect(m_pIdentityManagement, &IdentityManagement::sigDKQueryHealthCode, m_pHealthCodeManage, &HealthCodeManage::slotDKQueryHealthCode, Qt::QueuedConnection);
    
    // 图片空间管理线程处理
    QObject::connect(ShrinkFaceImageThread::GetInstance(), &ShrinkFaceImageThread::sigShrinkTip, m_pFaceMainFrm, &FaceMainFrm::slotUpDateTip,Qt::QueuedConnection);

    QObject::connect(m_pIdentityManagement, &IdentityManagement::sigTipsMessage, m_pFaceMainFrm, &FaceMainFrm::slotTipsMessage,
            Qt::QueuedConnection);
    QObject::connect(m_pIdentityManagement, &IdentityManagement::sigShowHealthCode, m_pFaceMainFrm, &FaceMainFrm::slotHealthCodeInfo,
            Qt::QueuedConnection);
    QObject::connect(m_pLRHealthCode, &LRHealthCode::sigLRHealthCodeMsg, m_pIdentityManagement, &IdentityManagement::slotLRHealthCodeInfo2,
            Qt::QueuedConnection);
    QObject::connect(m_pIdentityManagement, &IdentityManagement::sigShowAlgoStateAboutFace, m_pFaceMainFrm, &FaceMainFrm::slotShowAlgoStateAboutFace,
            Qt::QueuedConnection);
}


void FaceApp::Sleep(int sec)
{
	QTime dieTime = QTime::currentTime().addMSecs(sec);
	while (QTime::currentTime() < dieTime)
	{
		QCoreApplication::processEvents();
	}
}

void FaceApp::connectReady(QObject *receiver, const char *member, Qt::ConnectionType type)
{

	QMetaMethod method = receiver->metaObject()->method(receiver->metaObject()->indexOfMethod(QMetaObject::normalizedSignature(member)));
	if (instance()->d_ptr->_ready && method.isValid())    //是否有效的
		method.invoke(receiver, type);
	else
		QObject::connect(instance(), QMetaMethod::fromSignal(&FaceApp::ready), receiver, method, type);
}

FaceMainFrm *FaceApp::GetFaceMainFrm()
{
	
	return instance()->d_func()->m_pFaceMainFrm;
}
CameraManager *FaceApp::GetCameraManager()
{
	return instance()->d_func()->m_pCameraManager;
}


BaseFaceManager *FaceApp::GetAlgoFaceManager()
{

	int type = 1;  // ReadConfig::GetInstance()->getFaceRecognVedor();

	switch (type)
	{
		case 0: //虹软			
			break;
		case 1: //百度
			return instance()->d_func()->m_pBaiduFaceManager;					
			break;
		default:
			return instance()->d_func()->m_pBaiduFaceManager;
			break;
	}		

}

SensorManager *FaceApp::GetSensorManager()
{
		
	return instance()->d_func()->m_pSensorManager;
}

IdentityManagement *FaceApp::GetIdentityManagement()
{

	return instance()->d_func()->m_pIdentityManagement;
}

ParsePersonXlsx *FaceApp::GetParsePersonXlsx()
{
	return instance()->d_func()->m_pParsePersonXlsx;
}

RecordsExport *FaceApp::GetRecordsExport()
{
	return instance()->d_func()->m_pRecordsExport;
}

PostPersonRecordThread *FaceApp::GetPostPersonRecordThread()
{
    return instance()->d_func()->m_pPostPersonRecordThread;
}

powerManagerThread *FaceApp::GetPowerManagerThread()
{
	
	return instance()->d_func()->m_pPowerManagerThread;
}

HealthCodeManage *FaceApp::GetHealthCodeManage()
{
	return instance()->d_func()->m_pHealthCodeManage;
}

IdentityCardManage *FaceApp::GetIdentityCardManage()
{
	return instance()->d_func()->m_pIdentityCardManage;
}

IdentityCard_ZK_Manage *FaceApp::GetIdentityCard_ZK_Manage()
{
	return instance()->d_func()->m_pIdentityCard_ZK_Manage;
}

FingerprintManager *FaceApp::GetFingerprintManager()
{
    return instance()->d_func()->m_pFingerprintManager;
}
