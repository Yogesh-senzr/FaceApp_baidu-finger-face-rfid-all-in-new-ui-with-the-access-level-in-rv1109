#include "FaceMainFrm.h"
#include "SettingFuncFrms/SettingMenuFrm.h"
#include "FaceHomeFrms/FaceHomeFrm.h"
#include "FaceHomeFrms/FaceHomeTitleFrm.h"
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
#include "DB/RegisteredFacesDB.h"

#include <QMouseEvent>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>
#include <QStackedWidget>
#include <QBitmap>
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
    QTimer *m_pScreenTimeoutTimer;
    QTimer *m_pReturnHomeTimer;  // New timer for return to home before screen off
    bool m_bScreenOn;
    bool m_bCameraProcessingActive;
    int m_nScreenOffFromIndex; // Store which screen we turned off from
    static const int SCREEN_TIMEOUT_MS = 30000; // 30 seconds
    static const int RETURN_HOME_DELAY_MS = 28000; // 28 seconds - return home 2 seconds before screen off
    QLabel *m_pTimerLabel;           // Label to show current time on white screen
    QLabel *m_pDateLabel; 
    QTimer *m_pClockUpdateTimer;     // Timer to update the clock every second
    QWidget *m_recognitionWidget = nullptr;
    QDateTime m_screenOffTime;       // Time when screen went to white mode

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
    , m_bScreenOn(true)
    , m_bCameraProcessingActive(true)
    , m_nScreenOffFromIndex(0)
{
    // Initialize screen timeout timer
    m_pScreenTimeoutTimer = new QTimer();
    m_pScreenTimeoutTimer->setSingleShot(true);
    m_pScreenTimeoutTimer->setInterval(SCREEN_TIMEOUT_MS);
    // Initialize return home timer (28 seconds - 2 seconds before screen off)
    m_pReturnHomeTimer = new QTimer();
    m_pReturnHomeTimer->setSingleShot(true);
    m_pReturnHomeTimer->setInterval(RETURN_HOME_DELAY_MS);
    m_pClockUpdateTimer = new QTimer();
    m_pClockUpdateTimer->setInterval(1000); // Update every second

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
    
    m_pTimerLabel = new QLabel(q_func());
    m_pTimerLabel->setAlignment(Qt::AlignCenter);
    m_pTimerLabel->setStyleSheet("QLabel { "
                                "color: black; "
                                "font-size: 120px; "
                                "font-weight: bold; "
                                "background-color: white; "
                                "}");
    m_pTimerLabel->hide(); // Initially hidden
    
    // Create DATE label for white screen - **INCREASED FONT SIZE**
    m_pDateLabel = new QLabel(q_func());
    m_pDateLabel->setAlignment(Qt::AlignCenter);
    m_pDateLabel->setStyleSheet("QLabel { "
                               "color: black; "
                               "font-size: 44px; "
                               "font-weight: normal; "
                               "background-color: white; "
                               "}");
    m_pDateLabel->hide(); // Initially hidden
	
    printf(">>>>%s,%s,%d, width=%d,height=%d\n",__FILE__,__func__,__LINE__,
	   m_pInputLoginPasswordFrm->width(), m_pInputLoginPasswordFrm->height());
	if(DeskTopWidth<=480 && DeskTopHeight<=854)
	{
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

    m_pUpdateHintLabel = new QLabel(q_func());
    m_pUpdateHintLabel->move(300,300);
    QFont font ("Microsoft YaHei", 40, 75);
    m_pUpdateHintLabel->setFont(font);
    
    m_pUpdateHintLabel->setAttribute(Qt::WA_NoSystemBackground);
    m_pUpdateHintLabel->setStyleSheet("color:#FE0000;}");
}

void FaceMainFrmPrivate::InitData()
{
#ifdef Q_OS_LINUX
    q_func()->setAttribute(Qt::WA_TranslucentBackground);
#endif
    mStart = 0;
}

void FaceMainFrmPrivate::InitConnect()
{
    QObject::connect(m_pSettingMenuFrm, &SettingMenuFrm::sigShowFaceHomeFrm, q_func(), &FaceMainFrm::slotShowFaceHomeFrm);
    QObject::connect(m_pAddUserFrm, &AddUserFrm::sigShowFaceHomeFrm, q_func(), &FaceMainFrm::slotShowFaceHomeFrm);
    QObject::connect(m_pScreenTimeoutTimer, &QTimer::timeout, q_func(), &FaceMainFrm::slotScreenTimeout);
    QObject::connect(m_pClockUpdateTimer, &QTimer::timeout, q_func(), &FaceMainFrm::slotUpdateTimerClock);
    
    if (m_pStackedWidget->currentIndex() == 0) {
        LogD("%s %s[%d] === INIT === Starting on camera feed, starting timers\n", 
             __FILE__, __FUNCTION__, __LINE__);
        m_pScreenTimeoutTimer->start();
        m_pReturnHomeTimer->start();
    } else {
        LogD("%s %s[%d] === INIT === Starting on menu, timers stopped\n", 
             __FILE__, __FUNCTION__, __LINE__);
    }
}

void FaceMainFrm::slotScreenTimeout()
{
    Q_D(FaceMainFrm);
    
    int currentIndex = d->m_pStackedWidget->currentIndex();
    
    if (currentIndex != 0) {
        LogD("%s %s[%d] === SCREEN TIMEOUT === Not on camera feed (index %d), ignoring timeout\n",
             __FILE__, __FUNCTION__, __LINE__, currentIndex);
        return;
    }
    
    LogD("%s %s[%d] === SCREEN TIMEOUT === Showing white screen with timer after 30 seconds of no face on camera feed\n",
         __FILE__, __FUNCTION__, __LINE__);
    
    d->m_bScreenOn = false;
    d->m_bCameraProcessingActive = false;
    
    YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Light_White, 0);
    
    d->m_pStackedWidget->setVisible(false);
    
    this->setStyleSheet("QWidget { background-color: white; }");
    
    QWidget *whiteOverlay = new QWidget(this);
    whiteOverlay->setObjectName("whiteOverlay");
    whiteOverlay->setStyleSheet("QWidget#whiteOverlay { background-color: white; }");
    whiteOverlay->setGeometry(0, 0, this->width(), this->height());
    whiteOverlay->show();
    whiteOverlay->raise();
    
    whiteOverlay->setAttribute(Qt::WA_AcceptTouchEvents, true);
    whiteOverlay->setAttribute(Qt::WA_NoMousePropagation, true);
    whiteOverlay->setFocusPolicy(Qt::StrongFocus);
    whiteOverlay->installEventFilter(this);
    whiteOverlay->grabMouse();
    
    int timeY = this->height() / 2 - 80;
    d->m_pTimerLabel->setParent(whiteOverlay);
    d->m_pTimerLabel->setGeometry(0, timeY, this->width(), 150);
    d->m_pTimerLabel->show();
    d->m_pTimerLabel->raise();
    
    int dateY = timeY + 140;
    d->m_pDateLabel->setParent(whiteOverlay);
    d->m_pDateLabel->setGeometry(50, dateY, this->width() - 100, 80);
    d->m_pDateLabel->show();
    d->m_pDateLabel->raise();
    
    if (!d->m_pClockUpdateTimer->isActive()) {
        d->m_pClockUpdateTimer->start();
        LogD("%s %s[%d] === TIMER STARTED === Clock update timer is now running\n",
             __FILE__, __FUNCTION__, __LINE__);
    }
    
    slotUpdateTimerClock();
    
    LogD("%s %s[%d] === WHITE SCREEN === Full white screen overlay with properly positioned vertical time display\n",
         __FILE__, __FUNCTION__, __LINE__);
}

void FaceMainFrm::slotUpdateTimerClock()
{
    Q_D(FaceMainFrm);
    
    LogD("%s %s[%d] === TIMER UPDATE === Timer tick - updating clock display\n",
         __FILE__, __FUNCTION__, __LINE__);
    
    if (d->m_bScreenOn) {
        LogD("%s %s[%d] === TIMER SKIP === Screen is on, not updating white screen timer\n",
             __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    
    if (!d->m_pTimerLabel->isVisible()) {
        LogD("%s %s[%d] === TIMER SKIP === Timer label not visible, not updating\n",
             __FILE__, __FUNCTION__, __LINE__);
        return;
    }
    
    QDateTime currentTime = QDateTime::currentDateTime();
    QString timeString = currentTime.toString("hh:mm");  
    QString dateString = currentTime.toString("ddd dd MMM");  
    
    LogD("%s %s[%d] === TIMER UPDATE === Time: %s, Date: %s\n",
         __FILE__, __FUNCTION__, __LINE__, 
         timeString.toStdString().c_str(), dateString.toStdString().c_str());
    
    d->m_pTimerLabel->setText(timeString);
    d->m_pDateLabel->setText(dateString);
}

void FaceMainFrm::slotScreenWakeUp()
{
    Q_D(FaceMainFrm);
    
    if (!d->m_bScreenOn) {
        LogD("%s %s[%d] === SCREEN WAKE UP === Face detected, returning to camera feed\n",
             __FILE__, __FUNCTION__, __LINE__);
        
        d->m_pClockUpdateTimer->stop();
        d->m_pTimerLabel->hide();
        d->m_pDateLabel->hide();
        
        QWidget *whiteOverlay = this->findChild<QWidget*>("whiteOverlay");
        if (whiteOverlay) {
            whiteOverlay->deleteLater();
        }
        
        d->m_pTimerLabel->setParent(this);
        d->m_pDateLabel->setParent(this);
        
        d->m_pStackedWidget->setVisible(true);
        
        d->m_bScreenOn = true;
        d->m_bCameraProcessingActive = true;
        
        if (ReadConfig::GetInstance()->getFillLight_Value() > 0)
            YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Light_White, 1);
        YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_LCD_BL, 1);
        
        LogD("%s %s[%d] === RETURNING TO HOME === Switching to camera feed after wake up\n",
             __FILE__, __FUNCTION__, __LINE__);
        
        d->m_pStackedWidget->setCurrentIndex(0);
        setStatus(0);
        
#ifdef Q_OS_LINUX
        ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->setRegFaceState(false);
#endif
        
        if(DeskTopWidth>=800 && DeskTopHeight<=1280)
            this->setStyleSheet(myHelper::setStyleSheet(":/CSS/AppStyle800×1280.css"));
        else if(DeskTopWidth>=720 && DeskTopHeight<=1280)
            this->setStyleSheet(myHelper::setStyleSheet(":/CSS/AppStyle720×1280.css"));
        else if(DeskTopWidth>=600 && DeskTopHeight<=1024)
            this->setStyleSheet(myHelper::setStyleSheet(":/CSS/AppStyle600×1024.css"));
        else if(DeskTopWidth>=480 && DeskTopHeight<=854)
            this->setStyleSheet(myHelper::setStyleSheet(":/CSS/AppStyle480×854.css"));
        
        LogD("%s %s[%d] === SCREEN ON === Display restored to camera feed, resuming face processing\n",
             __FILE__, __FUNCTION__, __LINE__);
    }
    
    d->m_pScreenTimeoutTimer->start();
    d->m_pReturnHomeTimer->start();
}

bool FaceMainFrm::eventFilter(QObject *obj, QEvent *event)
{
    Q_D(FaceMainFrm);
    
    QWidget *whiteOverlay = this->findChild<QWidget*>("whiteOverlay");
    if (obj == whiteOverlay && whiteOverlay && !d->m_bScreenOn) {
        
        if (event->type() == QEvent::TouchBegin || 
            event->type() == QEvent::TouchEnd ||
            event->type() == QEvent::MouseButtonPress ||
            event->type() == QEvent::MouseButtonRelease ||
            event->type() == QEvent::MouseMove) {
            
            LogD("%s %s[%d] === TOUCH/CLICK DETECTED === User interacted with white screen (event type: %d), returning to camera feed\n",
                 __FILE__, __FUNCTION__, __LINE__, event->type());
            
            slotScreenWakeUp();
            return true;
        }
    }
    
    return QWidget::eventFilter(obj, event);
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
    }
	
    if (text.length()>0 &&  !((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->getRegFaceState())
    {
        LogD("%s %s[%d],text=%s \n",__FILE__,__FUNCTION__,__LINE__,text.toStdString().c_str());
    }

    if (text.contains(QObject::tr("UnzipTheFirmware"))   ||  text.contains(QObject::tr("CopyFirmware"))
         ||  text.contains(QObject::tr("update_img_check_md5"))  ||  text.contains(QObject::tr("UpgradingFirmware")))
    {
        ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->setRegFaceState(true);
    }
    if (text.contains(QObject::tr("FirmwareVerificationFailed")))
    {
        ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->setRegFaceState(false);
    }    
}

void FaceMainFrm::slotDisMissMessage(const bool state)
{
    Q_D(FaceMainFrm);
    d->m_pFaceHomeFrm->setDisMissMessage(state);

    int currentIndex = d->m_pStackedWidget->currentIndex();
    LogD("%s %s[%d] === DISMISS MESSAGE === State: %s, Screen On: %s, Current Index: %d\n",
         __FILE__, __FUNCTION__, __LINE__,
         state ? "FACE_DETECTED" : "NO_FACE", d->m_bScreenOn ? "TRUE" : "FALSE", currentIndex);

    if (currentIndex == 0) {
        if (state) {
            LogD("%s %s[%d] === FACE DETECTED ON CAMERA FEED === Resetting timers (restart 30s countdown)\n",
                 __FILE__, __FUNCTION__, __LINE__);
            
            d->m_pScreenTimeoutTimer->start();
            d->m_pReturnHomeTimer->start();
            
            if (!d->m_bScreenOn) {
                slotScreenWakeUp();
            }
        } else {
            LogD("%s %s[%d] === NO FACE ON CAMERA FEED === Timers continue counting towards 30s timeout\n",
                 __FILE__, __FUNCTION__, __LINE__);
        }
    } else {
        LogD("%s %s[%d] === IN MENU/SETTINGS === Ensuring timers are stopped, no timeout in menus\n",
             __FILE__, __FUNCTION__, __LINE__);
        
        d->m_pScreenTimeoutTimer->stop();
        d->m_pReturnHomeTimer->stop();
        
        if (state && !d->m_bScreenOn) {
            slotScreenWakeUp();
        }
    }

    int mdelay = ReadConfig::GetInstance()->getScreenOutDelay_Value();
    if(mdelay > 0)
    {    
        if (state == 0)
        {
            if (d->m_bScreenOn && currentIndex == 0) {
                if (d->mStart == 0) {
                    d->mStart = (double)clock();
                    LogD("%s %s[%d] === OLD TIMEOUT START === Starting old timeout system (%d seconds)\n",
                         __FILE__, __FUNCTION__, __LINE__, mdelay);
                }
                
                if (d->mStart > 0) {
                    int passtimer = ((double)clock() - d->mStart) / 1000 / 1000;
                    if (passtimer >= mdelay) {
                        LogD("%s %s[%d] === OLD TIMEOUT TRIGGERED === Old timeout reached, turning off screen\n",
                             __FILE__, __FUNCTION__, __LINE__);
                        
                        if (d->m_bScreenOn) {
                            YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Light_White, 0);            
                            YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_LCD_BL, 0);
                        }
                        d->mStart = 0;
                    }
                }
            }
        } 
        else if (state == 1)
        {
            d->mStart = 0;
            LogD("%s %s[%d] === OLD TIMEOUT RESET === Face detected, resetting old timeout system\n",
                 __FILE__, __FUNCTION__, __LINE__);
            
            if (d->m_bScreenOn) {
                if (ReadConfig::GetInstance()->getFillLight_Value() > 0)
                   YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Light_White, 1);        
                YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_LCD_BL, 1);
                
                LogD("%s %s[%d] === LCD RESTORED === LCD backlight restored for recognition display\n",
                     __FILE__, __FUNCTION__, __LINE__);
            }
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

    LogD("%s %s[%d] === SHOW FRAME === Switching to index %d (0=Home, 1=Settings, 2=AddUser)\n",
         __FILE__, __FUNCTION__, __LINE__, index);

    if(!index)
    {
        LogD("%s %s[%d] === SWITCHING TO CAMERA FEED === Starting timeout timers for 30s countdown\n",
             __FILE__, __FUNCTION__, __LINE__);
        
#ifdef Q_OS_LINUX
        ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->setRegFaceState(false);
#endif
        FaceApp::connectReady(d->m_pFaceHomeFrm, "onCheckNet()");
        FaceApp::connectReady(d->m_pFaceHomeFrm, "onCheckPersonNum()");
        
        d->m_pScreenTimeoutTimer->start();
        d->m_pReturnHomeTimer->start();
        
        LogD("%s %s[%d] === TIMERS STARTED === 30-second countdown started for screen timeout\n",
             __FILE__, __FUNCTION__, __LINE__);
    } else {
        LogD("%s %s[%d] === SWITCHING TO MENU/SETTINGS === Stopping timeout timers\n",
             __FILE__, __FUNCTION__, __LINE__);
        
        d->m_pScreenTimeoutTimer->stop();
        d->m_pReturnHomeTimer->stop();
    }
    
    d->m_pStackedWidget->setCurrentIndex(index);
    
    if (index == 0) {
        setStatus(0);
    } else if (index == 1) {
        setStatus(1);
    }
}

void FaceMainFrm::setPersonImage(const QString &imagePath, const QString &personName)
{
    Q_D(FaceMainFrm);
    d->m_pFaceHomeFrm->setPersonImage(imagePath, personName);
}

void FaceMainFrm::clearPersonImage()
{
    Q_D(FaceMainFrm);
    d->m_pFaceHomeFrm->clearPersonImage();
}

void FaceMainFrm::setTenantName(const QString &tenantName)
{
    Q_D(FaceMainFrm);
    if (d->m_pFaceHomeFrm) {
        d->m_pFaceHomeFrm->setTenantName(tenantName);
        qDebug() << "DEBUG: FaceMainFrm forwarded tenant name:" << tenantName;
    } else {
        qDebug() << "WARNING: m_pFaceHomeFrm is null in FaceMainFrm::setTenantName";
    }
}

void FaceMainFrm::updateSyncStatus(const QString &status)
{
    Q_D(FaceMainFrm);
    if (d->m_pFaceHomeFrm) {
        d->m_pFaceHomeFrm->updateSyncStatus(status);
        qDebug() << "DEBUG: FaceMainFrm forwarded sync status:" << status;
    } else {
        qDebug() << "WARNING: m_pFaceHomeFrm is null in FaceMainFrm::updateSyncStatus";
    }
}

void FaceMainFrm::updateSyncUserCount(int currentCount, int totalCount)
{
    Q_D(FaceMainFrm);
    if (d->m_pFaceHomeFrm) {
        d->m_pFaceHomeFrm->updateSyncUserCount(currentCount, totalCount);
        qDebug() << "DEBUG: FaceMainFrm forwarded sync user count:" << currentCount << "/" << totalCount;
    } else {
        qDebug() << "WARNING: m_pFaceHomeFrm is null in FaceMainFrm::updateSyncUserCount";
    }
}

void FaceMainFrm::updateLastSyncTime(const QString &time)
{
    Q_D(FaceMainFrm);
    if (d->m_pFaceHomeFrm) {
        d->m_pFaceHomeFrm->updateLastSyncTime(time);
        qDebug() << "DEBUG: FaceMainFrm forwarded last sync time:" << time;
    } else {
        qDebug() << "WARNING: m_pFaceHomeFrm is null in FaceMainFrm::updateLastSyncTime";
    }
}

void FaceMainFrm::updateLocalFaceCount(int localCount, int totalCount)
{
    Q_D(FaceMainFrm);
    if (d->m_pFaceHomeFrm) {
        d->m_pFaceHomeFrm->updateLocalFaceCount(localCount, totalCount);
        qDebug() << "DEBUG: FaceMainFrm forwarded local face count:" << localCount << "/" << totalCount;
    } else {
        qDebug() << "WARNING: m_pFaceHomeFrm is null in FaceMainFrm::updateLocalFaceCount";
    }
}

void FaceMainFrm::triggerSettings()
{
    Q_D(FaceMainFrm);
    
    if (d->m_pStackedWidget->currentIndex() != 0) {
        return;
    }
    
    LogD("%s %s[%d] === TRIGGER SETTINGS === Going to settings, stopping timers\n",
         __FILE__, __FUNCTION__, __LINE__);
    
#ifdef Q_OS_LINUX
    ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->setRegFaceState(true);
#endif
    
    d->m_pScreenTimeoutTimer->stop();
    d->m_pReturnHomeTimer->stop();
    
    d->m_pStackedWidget->setCurrentIndex(1);
    setStatus(1);
    
    LogD("%s %s[%d] === SETTINGS OPENED === Timers stopped, no timeout in settings\n",
         __FILE__, __FUNCTION__, __LINE__);
}

#define F_OK (0)
void FaceMainFrm::onReady()
{
    Q_D(FaceMainFrm);

    LogD("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
         QTimer::singleShot(350, this, [this]() {
        if (DeskTopWidth >= 800 && DeskTopHeight <= 1280)
            this->setStyleSheet(myHelper::setStyleSheet(":/CSS/AppStyle800×1280.css"));
        else if (DeskTopWidth >= 720 && DeskTopHeight <= 1280)
            this->setStyleSheet(myHelper::setStyleSheet(":/CSS/AppStyle720×1280.css"));
        else if (DeskTopWidth >= 600 && DeskTopHeight <= 1024)
            this->setStyleSheet(myHelper::setStyleSheet(":/CSS/AppStyle600×1024.css"));
        else if (DeskTopWidth >= 480 && DeskTopHeight <= 854)
            this->setStyleSheet(myHelper::setStyleSheet(":/CSS/AppStyle480×854.css"));
    });      
    printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
#ifndef Q_OS_LINUX
    printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
    this->setFixedSize(800, 800);
#endif
    printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
    this->show();

    int Mode = ReadConfig::GetInstance()->getNetwork_Manager_Mode();
    LogD("%s %s[%d] Mode %d \n",__FILE__,__FUNCTION__,__LINE__,Mode);
    
    LogD("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
    NetworkControlThread::GetInstance()->setNetworkType(Mode);
    QString Name = ReadConfig::GetInstance()->getWIFI_Name();
    QString pasw = ReadConfig::GetInstance()->getWIFI_Password();
    NetworkControlThread::GetInstance()->setLinkWlanSSID(Name, pasw);
    LogD("%s %s[%d] Network4GControlThread::GetInstance()\n",__FILE__,__FUNCTION__,__LINE__);
    
    char szMac[60]={0};
    
    if(Mode == 1)
    {
    }else if(Mode == 2)
    {
        if(!Name.isEmpty())
        {
            NetworkControlThread::GetInstance()->DisconnectAllWifi();
            NetworkControlThread::GetInstance()->setWifiSearchMode(2);
            NetworkControlThread::GetInstance()->resume();
        }
    }else if(Mode == 3)
    {
        sprintf(szMac,"ifconfig eth0 hw ether %s",myHelper::GetNetworkMac().toStdString().c_str());        
        myHelper::Utils_ExecCmd("ifconfig eth0 down;");
        myHelper::Utils_ExecCmd(szMac);
        myHelper::Utils_ExecCmd("ifconfig eth0 up;");    
        
    	Network4GControlThread::GetInstance();
    }

    FaceApp::connectReady(d->m_pFaceHomeFrm, "onCheckNet()");
    FaceApp::connectReady(d->m_pFaceHomeFrm, "onCheckSN()");
    FaceApp::connectReady(d->m_pFaceHomeFrm, "onCheckPersonNum()");

#ifdef Q_OS_LINUX
    QTimer::singleShot(3500, this, [&]{
        if(((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->getAlgoFaceInitState() != true) d_func()->m_pFaceHomeFrm->setUpDateTip(QObject::tr("AlgorithmDisable"));
    });
#endif
    qDebug()<<"DeskW:"<<DeskTopWidth<<" DeskH:"<<DeskTopHeight;
}

void FaceMainFrm::createInstantPopupWithPlaceholder(const QString &name, int personId, const QString &idcard) {
    Q_D(FaceMainFrm);
    
    // Pre-calculated dimensions for speed
    int widgetHeight = this->height() / 4;
    int widgetWidth = this->width();
    int imageSize = widgetHeight - 20; // Increased from 30 to 20 for slightly larger size
    
    // Create recognition widget
    d->m_recognitionWidget = new QWidget(this);
    d->m_recognitionWidget->setObjectName("recognitionWidget");
    d->m_recognitionWidget->setGeometry(0, this->height() - widgetHeight, widgetWidth, widgetHeight);
    d->m_recognitionWidget->setStyleSheet(
        "QWidget#recognitionWidget {"
        "   background-color: rgba(0, 0, 0, 200);"
        "   border-top: 3px solid #00FF00;"
        "   border-radius: 8px;"
        "}"
    );
    
    // Circular image placeholder with increased size
    QLabel *imageLabel = new QLabel(d->m_recognitionWidget);
    imageLabel->setObjectName("faceImage");
    imageLabel->setGeometry(15, 15, imageSize, imageSize);
    
    // Make it circular with updated styling - FORCE circular shape
    int borderRadius = imageSize / 2; // Perfect circle
    imageLabel->setStyleSheet(
        QString("QLabel {"
        "   border: 3px solid #00FF00;"
        "   border-radius: %1px;" // Circular border
        "   background-color: #222222;"
        "   color: #888888;"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "   min-width: %2px;"
        "   max-width: %2px;"
        "   min-height: %2px;"
        "   max-height: %2px;"
        "}").arg(borderRadius).arg(imageSize)
    );
    imageLabel->setText("LOADING...");
    imageLabel->setAlignment(Qt::AlignCenter);
    
    // Force the label to be exactly square for perfect circle
    imageLabel->setFixedSize(imageSize, imageSize);
    
    // Text area positioning - adjusted for larger image
    int textX = imageSize + 50; // Increased spacing from image
    int textY = 15;
    
    // Status indicator - moved slightly left
    QLabel *statusLabel = new QLabel("RECOGNIZED", d->m_recognitionWidget);
    statusLabel->setGeometry(textX - 20, textY, 200, 30); // Moved 20px to the left
    statusLabel->setStyleSheet(
        "QLabel {"
        "   color: #00FF00;"
        "   font-size: 24px;"
        "   font-weight: bold;"
        "   background-color: rgba(0, 255, 0, 50);"
        "   padding: 4px 8px;"
        "   border-radius: 4px;"
        "}"
    );
    statusLabel->setAlignment(Qt::AlignCenter);
    
    // Name label - adjusted positioning
    QLabel *nameLabel = new QLabel(name, d->m_recognitionWidget);
    nameLabel->setGeometry(textX - 20, textY + 40, widgetWidth - textX, 50); // Also moved left
    nameLabel->setStyleSheet(
        "QLabel {"
        "   color: #00FF00;"
        "   font-size: 42px;"
        "   font-weight: bold;"
        "}"
    );
    
    // ID label - adjusted positioning
    QString idText = !idcard.isEmpty() ? idcard : QString("ID: %1").arg(personId);
    QLabel *idLabel = new QLabel(idText, d->m_recognitionWidget);
    idLabel->setGeometry(textX - 20, textY + 100, widgetWidth - textX, 40); // Also moved left
    idLabel->setStyleSheet(
        "QLabel {"
        "   color: white;"
        "   font-size: 32px;"
        "   font-weight: normal;"
        "}"
    );
    
    // Show widget and set auto-hide timer
    d->m_recognitionWidget->show();
    d->m_recognitionWidget->raise();
    
    QTimer::singleShot(3000, this, [d]() {
        if (d->m_recognitionWidget) {
            d->m_recognitionWidget->hide();
            d->m_recognitionWidget->deleteLater();
            d->m_recognitionWidget = nullptr;
        }
    });
    
    LogD("%s %s[%d] === INSTANT POPUP === Circular placeholder shown for %s\n", 
         __FILE__, __FUNCTION__, __LINE__, name.toStdString().c_str());
}

void FaceMainFrm::handleImageLoaded(const QPixmap &pixmap, const QString &imagePath) {
    Q_D(FaceMainFrm);
    updatePopupWithImage(pixmap, imagePath);
}

void FaceMainFrm::loadFaceImageAsync(const QString &name, int personId, const QString &uuid, const QString &idcard) {
    Q_D(FaceMainFrm);
    
    LogD("%s %s[%d] === LOADING STORED IMAGE === Name: %s, EmployeeID: %s\n", 
         __FILE__, __FUNCTION__, __LINE__, name.toStdString().c_str(), idcard.toStdString().c_str());
    
    // Load stored image in background thread
    QtConcurrent::run([this, d, name, personId, uuid, idcard]() {
        
        qint64 startTime = QDateTime::currentMSecsSinceEpoch();
        
        // Use the new stored image search function
        QString imagePath = findStoredFaceImage(personId, name, uuid, idcard);
        
        qint64 searchTime = QDateTime::currentMSecsSinceEpoch() - startTime;
        LogD("%s %s[%d] === IMAGE SEARCH === Found in %lld ms: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, searchTime, 
             imagePath.isEmpty() ? "NO_IMAGE" : imagePath.toStdString().c_str());
        
        // Load image if found
        QPixmap pixmap;
        if (!imagePath.isEmpty()) {
            qint64 loadStart = QDateTime::currentMSecsSinceEpoch();
            pixmap.load(imagePath);
            qint64 loadTime = QDateTime::currentMSecsSinceEpoch() - loadStart;
            
            LogD("%s %s[%d] === IMAGE LOADED === in %lld ms\n", 
                 __FILE__, __FUNCTION__, __LINE__, loadTime);
        }
        
        QMetaObject::invokeMethod(this, "handleImageLoaded", 
                         Qt::QueuedConnection,
                         Q_ARG(QPixmap, pixmap),
                         Q_ARG(QString, imagePath));
    });
}


QString FaceMainFrm::findFaceImageFast(int personId, const QString &name) {
    LogD("%s %s[%d] === SEARCHING FOR STORED FACE IMAGE ===\n", __FILE__, __FUNCTION__, __LINE__);
    LogD("%s %s[%d] Person ID: %d, Name: %s\n", __FILE__, __FUNCTION__, __LINE__, personId, name.toStdString().c_str());
    
    // ✅ PRIORITY 1: Check stored cropped images by Employee ID
    // We need to find the employee ID from the person ID first
    QString employeeId = getEmployeeIdFromPersonId(personId);
    
    if (!employeeId.isEmpty()) {
        QString storedCroppedPath = QString("/mnt/user/reg_face_image/%1.jpg").arg(employeeId);
        LogD("%s %s[%d] Checking stored cropped image: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, storedCroppedPath.toStdString().c_str());
        
        if (QFileInfo::exists(storedCroppedPath)) {
            QImage img(storedCroppedPath);
            if (!img.isNull() && img.width() > 50 && img.height() > 50) {
                LogD("%s %s[%d] === FOUND STORED CROPPED IMAGE === Using: %s\n", 
                     __FILE__, __FUNCTION__, __LINE__, storedCroppedPath.toStdString().c_str());
                return storedCroppedPath;
            } else {
                LogD("%s %s[%d] Stored image exists but invalid: %s\n", 
                     __FILE__, __FUNCTION__, __LINE__, storedCroppedPath.toStdString().c_str());
            }
        } else {
            LogD("%s %s[%d] Stored cropped image not found: %s\n", 
                 __FILE__, __FUNCTION__, __LINE__, storedCroppedPath.toStdString().c_str());
        }
    } else {
        LogD("%s %s[%d] Could not find employee ID for person ID: %d\n", 
             __FILE__, __FUNCTION__, __LINE__, personId);
    }
    
    // ✅ PRIORITY 2: Search by name pattern in stored directory
    QDir storedDir("/mnt/user/reg_face_image");
    if (storedDir.exists()) {
        LogD("%s %s[%d] Searching stored directory by name pattern...\n", __FILE__, __FUNCTION__, __LINE__);
        
        QStringList filters;
        filters << "*.jpg" << "*.jpeg" << "*.png";
        QFileInfoList files = storedDir.entryInfoList(filters, QDir::Files, QDir::Time);
        
        // Look for files that might match this person
        for (const QFileInfo &file : files) {
            QString fileName = file.baseName().toLower();
            QString nameLower = name.toLower();
            
            // Check if filename contains the name or person ID
            if (fileName.contains(nameLower) || fileName.contains(QString::number(personId))) {
                QString filePath = file.absoluteFilePath();
                QImage img(filePath);
                if (!img.isNull() && img.width() > 50 && img.height() > 50) {
                    LogD("%s %s[%d] === FOUND BY NAME PATTERN === Using: %s\n", 
                         __FILE__, __FUNCTION__, __LINE__, filePath.toStdString().c_str());
                    return filePath;
                }
            }
        }
    }
    
    // ✅ PRIORITY 3: Check database for stored image path
    QString dbImagePath = getImagePathFromDatabase(employeeId.isEmpty() ? QString::number(personId) : employeeId);
    if (!dbImagePath.isEmpty() && QFileInfo::exists(dbImagePath)) {
        LogD("%s %s[%d] === FOUND IN DATABASE === Using: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, dbImagePath.toStdString().c_str());
        return dbImagePath;
    }
    
    // ✅ FALLBACK: Use any recent stored image as last resort
    QDir storedDir2("/mnt/user/reg_face_image");
    if (storedDir2.exists()) {
        QStringList filters;
        filters << "*.jpg" << "*.jpeg" << "*.png";
        QFileInfoList files = storedDir2.entryInfoList(filters, QDir::Files, QDir::Time);
        
        if (!files.isEmpty()) {
            QString fallbackPath = files.first().absoluteFilePath();
            LogD("%s %s[%d] === FALLBACK === Using most recent stored image: %s\n", 
                 __FILE__, __FUNCTION__, __LINE__, fallbackPath.toStdString().c_str());
            return fallbackPath;
        }
    }
    
    LogD("%s %s[%d] === NO STORED IMAGE FOUND === No stored face image found for %s\n", 
         __FILE__, __FUNCTION__, __LINE__, name.toStdString().c_str());
    return QString();
}

QString FaceMainFrm::getEmployeeIdFromPersonId(int personId) {
    LogD("%s %s[%d] === FINDING EMPLOYEE ID === For person ID: %d\n", 
         __FILE__, __FUNCTION__, __LINE__, personId);
    
    // Get all persons from RegisteredFacesDB
    RegisteredFacesDB* faceDB = RegisteredFacesDB::GetInstance();
    QList<PERSONS_t> allPersons = faceDB->GetAllPersonFromRAM();
    
    for (const PERSONS_t &person : allPersons) {
        if (person.personid == personId) {
            LogD("%s %s[%d] Found employee ID: %s for person ID: %d\n", 
                 __FILE__, __FUNCTION__, __LINE__, person.idcard.toStdString().c_str(), personId);
            return person.idcard; // idcard field contains the employee ID
        }
    }
    
    LogD("%s %s[%d] Employee ID not found for person ID: %d\n", 
         __FILE__, __FUNCTION__, __LINE__, personId);
    return QString();
}

QString FaceMainFrm::getImagePathFromDatabase(const QString &employeeId) {
    LogD("%s %s[%d] === CHECKING DATABASE === For employee ID: %s\n", 
         __FILE__, __FUNCTION__, __LINE__, employeeId.toStdString().c_str());
    
    RegisteredFacesDB* faceDB = RegisteredFacesDB::GetInstance();
    QList<PERSONS_t> allPersons = faceDB->GetAllPersonFromRAM();
    
    for (const PERSONS_t &person : allPersons) {
        if (person.idcard == employeeId) {
            // Check if the person has a stored image path in the database
            // This assumes the database stores the path to the registered face image
            QString imagePath = QString("/mnt/user/reg_face_image/%1.jpg").arg(employeeId);
            if (QFileInfo::exists(imagePath)) {
                LogD("%s %s[%d] Found database image path: %s\n", 
                     __FILE__, __FUNCTION__, __LINE__, imagePath.toStdString().c_str());
                return imagePath;
            }
        }
    }
    
    LogD("%s %s[%d] No database image path found for employee ID: %s\n", 
         __FILE__, __FUNCTION__, __LINE__, employeeId.toStdString().c_str());
    return QString();
}

bool FaceMainFrm::isValidFaceCrop(const QImage &img) {
    if (img.isNull()) return false;
    
    // Basic checks for proper face crop
    float aspectRatio = (float)img.width() / img.height();
    
    // Typical face crop aspect ratios (adjust as needed)
    if (aspectRatio < 0.7 || aspectRatio > 1.3) {
        LogD("%s %s[%d] === INVALID ASPECT RATIO === %.2f\n", 
             __FILE__, __FUNCTION__, __LINE__, aspectRatio);
        return false;
    }
    
    // Check if image contains mostly face (simple color distribution check)
    int skinTonePixels = 0;
    for (int y = 0; y < img.height(); y += 5) { // Sample every 5th pixel
        for (int x = 0; x < img.width(); x += 5) {
            QRgb pixel = img.pixel(x, y);
            int r = qRed(pixel);
            int g = qGreen(pixel);
            int b = qBlue(pixel);
            
            // Simple skin tone detection (adjust thresholds as needed)
            if (r > 100 && g > 50 && b > 30 && 
                abs(r - g) > 10 && r > g && r > b) {
                skinTonePixels++;
            }
        }
    }
    
    float skinRatio = (float)skinTonePixels / ((img.width()/5) * (img.height()/5));
    bool valid = skinRatio > 0.3; // At least 30% skin tones
    
    LogD("%s %s[%d] === FACE VALIDATION === Skin ratio: %.2f, Valid: %d\n", 
         __FILE__, __FUNCTION__, __LINE__, skinRatio, valid);
    
    return valid;
}

void FaceMainFrm::updatePopupWithImage(const QPixmap &pixmap, const QString &imagePath) {
    Q_D(FaceMainFrm);
    
    if (!d->m_recognitionWidget) {
        return;
    }
    
    QLabel *imageLabel = d->m_recognitionWidget->findChild<QLabel*>("faceImage");
    if (!imageLabel) {
        return;
    }

    // Get the current container size (should match the size set in createInstantPopupWithPlaceholder)
    int containerSize = imageLabel->width(); // This will be the increased size
    
    if (!pixmap.isNull()) {
        // Force square container first
        imageLabel->setFixedSize(containerSize, containerSize);
        
        // Create a circular pixmap with proper masking
        QPixmap circularPixmap(containerSize, containerSize);
        circularPixmap.fill(Qt::transparent);
        
        // First, scale the input image to fit the container
        QPixmap scaledInput = pixmap.scaled(containerSize, containerSize, 
                                          Qt::KeepAspectRatioByExpanding, 
                                          Qt::SmoothTransformation);
        
        // Create a circular mask
        QBitmap mask(containerSize, containerSize);
        mask.fill(Qt::color0); // Fill with transparent
        
        QPainter maskPainter(&mask);
        maskPainter.setBrush(Qt::color1); // Fill with opaque
        maskPainter.setPen(Qt::NoPen);
        maskPainter.setRenderHint(QPainter::Antialiasing);
        maskPainter.drawEllipse(0, 0, containerSize, containerSize);
        maskPainter.end();
        
        // Apply the scaled image to the circular pixmap
        QPainter imagePainter(&circularPixmap);
        imagePainter.setRenderHint(QPainter::Antialiasing);
        imagePainter.setRenderHint(QPainter::SmoothPixmapTransform);
        
        // Center the scaled image
        int xOffset = (containerSize - scaledInput.width()) / 2;
        int yOffset = (containerSize - scaledInput.height()) / 2;
        imagePainter.drawPixmap(xOffset, yOffset, scaledInput);
        imagePainter.end();
        
        // Apply the circular mask to make it round
        circularPixmap.setMask(mask);
        
        // Set the circular pixmap to the label
        imageLabel->setPixmap(circularPixmap);
        imageLabel->setText("");
        
        // Apply circular styling with border
        int borderRadius = containerSize / 2;
        imageLabel->setStyleSheet(
            QString("QLabel {"
                   "border: 3px solid #00FF00;"
                   "border-radius: %1px;"
                   "background-color: transparent;"
                   "min-width: %2px;"
                   "max-width: %2px;"
                   "min-height: %2px;"
                   "max-height: %2px;"
                   "}").arg(borderRadius).arg(containerSize)
        );
    } else {
        // Show circular placeholder when no image is available
        imageLabel->setFixedSize(containerSize, containerSize);
        imageLabel->setText("NO\nFACE");
        
        int borderRadius = containerSize / 2;
        imageLabel->setStyleSheet(
            QString("QLabel {"
                   "border: 3px solid #666666;"
                   "border-radius: %1px;"
                   "background-color: #333333;"
                   "color: #666666;"
                   "font-size: 16px;"
                   "font-weight: bold;"
                   "min-width: %2px;"
                   "max-width: %2px;"
                   "min-height: %2px;"
                   "max-height: %2px;"
                   "}").arg(borderRadius).arg(containerSize)
        );
    }
}

QString FaceMainFrm::findStoredFaceImage(int personId, const QString &name, const QString &uuid, const QString &idcard) {
    LogD("%s %s[%d] === SEARCHING STORED FACE IMAGE (STRICT MATCHING) ===\n", __FILE__, __FUNCTION__, __LINE__);
    LogD("%s %s[%d] PersonID: %d, Name: '%s', EmployeeID: '%s'\n", 
         __FILE__, __FUNCTION__, __LINE__, personId, name.toStdString().c_str(), idcard.toStdString().c_str());
    
    // ✅ PRIORITY 1: Exact Employee ID match (most reliable)
    if (!idcard.isEmpty() && idcard != "0" && idcard != "000000") {
        QString directPath = QString("/mnt/user/reg_face_image/%1.jpg").arg(idcard);
        LogD("%s %s[%d] Checking exact employee ID path: %s\n", 
             __FILE__, __FUNCTION__, __LINE__, directPath.toStdString().c_str());
        
        QFileInfo fileInfo(directPath);
        if (fileInfo.exists() && fileInfo.size() > 1000) { // At least 1KB
            LogD("%s %s[%d] === EXACT MATCH FOUND === Using: %s (%lld bytes)\n", 
                 __FILE__, __FUNCTION__, __LINE__, directPath.toStdString().c_str(), fileInfo.size());
            return directPath;
        } else if (fileInfo.exists()) {
            LogD("%s %s[%d] File exists but too small: %s (%lld bytes)\n", 
                 __FILE__, __FUNCTION__, __LINE__, directPath.toStdString().c_str(), fileInfo.size());
        } else {
            LogD("%s %s[%d] Exact file not found: %s\n", 
                 __FILE__, __FUNCTION__, __LINE__, directPath.toStdString().c_str());
        }
    } else {
        LogD("%s %s[%d] Employee ID is empty or invalid: '%s'\n", 
             __FILE__, __FUNCTION__, __LINE__, idcard.toStdString().c_str());
    }
    
    // ✅ PRIORITY 2: Search by exact filename pattern matching (strict)
    QDir storedDir("/mnt/user/reg_face_image");
    if (storedDir.exists()) {
        QStringList filters;
        filters << "*.jpg" << "*.jpeg" << "*.png";
        QFileInfoList files = storedDir.entryInfoList(filters, QDir::Files, QDir::Time);
        
        LogD("%s %s[%d] Searching %d files with strict matching...\n", 
             __FILE__, __FUNCTION__, __LINE__, files.size());
        
        // STRICT MATCHING: Only exact employee ID matches
        if (!idcard.isEmpty() && idcard != "0" && idcard != "000000") {
            for (const QFileInfo &file : files) {
                QString fileName = file.baseName(); // Get filename without extension
                
                // Exact employee ID match
                if (fileName == idcard) {
                    if (file.size() > 1000) {
                        LogD("%s %s[%d] === EXACT FILENAME MATCH === Using: %s\n", 
                             __FILE__, __FUNCTION__, __LINE__, file.absoluteFilePath().toStdString().c_str());
                        return file.absoluteFilePath();
                    }
                }
            }
            
            // Also try with case-insensitive matching
            QString idcardLower = idcard.toLower();
            for (const QFileInfo &file : files) {
                QString fileName = file.baseName().toLower();
                
                if (fileName == idcardLower) {
                    if (file.size() > 1000) {
                        LogD("%s %s[%d] === CASE-INSENSITIVE MATCH === Using: %s\n", 
                             __FILE__, __FUNCTION__, __LINE__, file.absoluteFilePath().toStdString().c_str());
                        return file.absoluteFilePath();
                    }
                }
            }
        }
        
        // ✅ PRIORITY 3: Try name-based matching (only if name is specific enough)
        if (!name.isEmpty() && name.length() > 3) {
            QString nameLower = name.toLower().replace(" ", "").replace("_", "").replace("-", "");
            
            for (const QFileInfo &file : files) {
                QString fileName = file.baseName().toLower().replace(" ", "").replace("_", "").replace("-", "");
                
                // Only match if the filename is exactly the name (avoid partial matches)
                if (fileName == nameLower) {
                    if (file.size() > 1000) {
                        LogD("%s %s[%d] === EXACT NAME MATCH === Using: %s\n", 
                             __FILE__, __FUNCTION__, __LINE__, file.absoluteFilePath().toStdString().c_str());
                        return file.absoluteFilePath();
                    }
                }
            }
        }
        
        LogD("%s %s[%d] No exact matches found in directory\n", __FILE__, __FUNCTION__, __LINE__);
    } else {
        LogD("%s %s[%d] Stored directory does not exist: /mnt/user/reg_face_image\n", 
             __FILE__, __FUNCTION__, __LINE__);
    }
    
    // ✅ NO FALLBACK: Don't show wrong images
    LogD("%s %s[%d] === NO MATCH FOUND === No stored image for this specific person\n", 
         __FILE__, __FUNCTION__, __LINE__);
    LogD("%s %s[%d] Will show placeholder instead of wrong image\n", __FILE__, __FUNCTION__, __LINE__);
    
    return QString(); // Return empty string - this will show placeholder
}

void FaceMainFrm::slotDisplayRecognizedPerson(const QString &name, const int &personId, 
                                             const QString &uuid, const QString &idcard) {
    Q_D(FaceMainFrm);
    
    qint64 startTime = QDateTime::currentMSecsSinceEpoch();
    
    LogD("%s %s[%d] === RECOGNITION EVENT === Name: '%s', PersonID: %d, UUID: '%s', EmployeeID: '%s'\n", 
         __FILE__, __FUNCTION__, __LINE__, name.toStdString().c_str(), personId, 
         uuid.toStdString().c_str(), idcard.toStdString().c_str());
    
    // ✅ STEP 1: Rate limiting (< 1ms)
    static QTime lastDisplayTime;
    static QString lastDisplayEmployeeId;
    
    if (idcard == lastDisplayEmployeeId && 
        lastDisplayTime.msecsTo(QTime::currentTime()) < 1000) {
        LogD("%s %s[%d] === POPUP SKIPPED === Same employee ID too soon (%d ms ago): %s\n", 
             __FILE__, __FUNCTION__, __LINE__, lastDisplayTime.msecsTo(QTime::currentTime()), idcard.toStdString().c_str());
        return;
    }
    
    lastDisplayTime = QTime::currentTime();
    lastDisplayEmployeeId = idcard;
    
    // ✅ STEP 2: Immediate cleanup (< 2ms)
    if (d->m_recognitionWidget) {
        d->m_recognitionWidget->hide();
        d->m_recognitionWidget->deleteLater();
        d->m_recognitionWidget = nullptr;
    }
    
    // ✅ STEP 3: Create instant popup with placeholder (< 10ms)
    createInstantPopupWithPlaceholder(name, personId, idcard);
    
    // ✅ STEP 4: Load stored face image asynchronously (doesn't block UI)
     loadFaceImageAsync(name, personId, uuid, idcard);
    
    qint64 elapsed = QDateTime::currentMSecsSinceEpoch() - startTime;
    LogD("%s %s[%d] === POPUP DISPLAYED === in %lld ms for employee: %s\n", 
         __FILE__, __FUNCTION__, __LINE__, elapsed, idcard.toStdString().c_str());
}

void FaceMainFrm::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_D(FaceMainFrm);
    auto pos = event->pos();
  
    if(pos.x()<80 && pos.y()<80 && (0 == d->m_pStackedWidget->currentIndex()))
    {
#ifdef Q_OS_LINUX
        ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->setRegFaceState(true);
#endif
        d->m_pStackedWidget->setCurrentIndex(1);
        setStatus(1);
    }
    
    QWidget::mouseDoubleClickEvent(event);
}

void FaceMainFrm::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "🔍 UI Thread Check:";
    qDebug() << "  Current thread:" << QThread::currentThread();
    qDebug() << "  UI thread:" << qApp->thread();
    qDebug() << "  Same thread?" << (QThread::currentThread() == qApp->thread());
    qDebug() << "  Touch position:" << event->pos();
    
    QWidget::mousePressEvent(event);
  
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
            d->m_pStackedWidget->setCurrentIndex(1);
            setStatus(1);
	    }
	}
}

void FaceMainFrm::paintEvent(QPaintEvent *event)
{
    Q_D(FaceMainFrm);
#ifdef SCREENCAPTURE
    if (!mPath.isEmpty() && mDraw)
    {
        mDraw = false;
        Q_UNUSED(event);  
        QPainter painter(this);  
        QPixmap pix;
        pix.load(mPath);
        painter.drawPixmap(0,0,800,1280,pix);
        grab().save(QString("/mnt/user/screenshot/painterFaceMainFrm.png"),"png"); 
    }
#endif 
}