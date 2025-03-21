#include "ReadConfig.h"
#include <QProcess> 
#include "Helper/myhelper.h"
#include "BaiduFace/BaiduFaceManager.h"
#include "PCIcore/Audio.h"
#include "PCIcore/Utils_Door.h"
#include "PCIcore/SensorManager.h"
#include "PCIcore/Wiegand.h"

#include <ManageEngines/PersonRecordToDB.h>
#include "DB/RegisteredFacesDB.h"
#include "FaceMainFrm.h"
#include "ManageEngines/IdentityManagement.h"
#include "Threads/powerManagerThread.h"

#include "Application/FaceApp.h"
#include "SystemMaintenanceManage.h"

#include <QtCore/QSettings>
#include <QtCore/QThread>

typedef struct
{
    int PanoramaImg;
    int FaceImg;
    int Stranger;
}Records_Manager_t;

typedef struct
{
    int Mode;
}Network_Manager_t;

typedef struct
{
    QString Name;
    QString Password;
}WIFI_t;

typedef struct
{
    int DHCP;
    QString IP;
    QString Maks;
    QString Gateway;
    QString DNS;
}LAN_t;

typedef struct
{
    QString Address;
    QString Password;
}Srv_Manager_t;

typedef struct
{
    QString Address;
    QString Password;
}Post_PersonRecord_t;

typedef struct
{
    QString Address;
    QString Password;
}Person_Registration_t;

typedef struct
{
    QString Address;
    QString Password;
}SyncUsers_t;

typedef struct
{
    QString Address;
    QString Password;
}UserDetail_t;

typedef struct
{
    int Mode;
}Language_t;

typedef struct
{
    int Value;
}Luminance_t;

typedef struct
{
    int Value;
}FillLight_t;

typedef struct
{
    int Value;
}Volume_t;

typedef struct
{
    int Value;
}ScreenOutDelay_t;

typedef struct
{
    int Value;
}DebugMode_t;

typedef struct
{
    int Type;
    int Wiggins;
    int Timer;
	int Relay;
    QString MustOpenMode;
    QString OptionalOpenMode;
    QString Password;
}Door_t;

typedef struct
{
    int autoTime;
    int autoZone;
    int Zone;
}Timer_Manager_t;

typedef struct
{
    int boot;
    QString bootTimer;
    int screenOffTime;
}Maintenance_t;

typedef struct
{
    float TempComp;
    float AlarmTemp;
    int CheckLiving;
    float Living_value;
    float Mask_value;
    float FqThreshold;
    int IdentifyInterval;
    int Identifycm;

    float Thanthreshold;
    float idcardThanthreshold;

    int TemperatureMode;//测温模式
}Identity_Manager_t;

typedef struct
{
    //显示设备号
    int DisplaySnNum;
    //显示MAC地址
    int DisplayMac;
    //显示IP地址
    int DisplayIP;
    //显示注册人数
    int DisplayPersonNum;
    //设备名
    QString DeviceName;
    //设备地理位置
    QString Location;
    //设备当前语言
    QString Language;
}HomeDisplay_t;

typedef struct
{
    QString serialNumber;  // Add this struct for device info
} DeviceInfo_t;

class ReadConfigPrivate
{
    Q_DECLARE_PUBLIC(ReadConfig)
public:
    ReadConfigPrivate(ReadConfig *dd);
private:
    void Init();
private:
    Records_Manager_t mRecords_Manager_t{};
    Network_Manager_t mNetwork_Manager_t{};
    WIFI_t mWIFI_t{};
    LAN_t mLAN_t{};
    Srv_Manager_t mSrv_Manager_t{};
    Post_PersonRecord_t mPost_PersonRecord_t{};
    Person_Registration_t mPerson_Registration_t{};
    SyncUsers_t mSyncUsers_t{};
    UserDetail_t mUserDetail_t{};
    Language_t mLanguage_t{};
    Luminance_t mLuminance_t{};
    FillLight_t mFillLight_t{};
    Volume_t mVolume_t{};
    ScreenOutDelay_t mScreenOutDelay_t{};
    DebugMode_t mDebugMode_t{};
    Door_t mDoor_t{};
    Timer_Manager_t mTimer_Manager_t{};
    Maintenance_t mMaintenance_t{};
    Identity_Manager_t mIdentity_Manager_t{};
    HomeDisplay_t mHomeDisplay_t{};
    DeviceInfo_t mDeviceInfo_t{};  // Add this member
private:
    int mTempSensorType;
    int mIrCameraRotation;
    int mRgbCameraRotation;
private:
    int mPlatformType;
    int mWiegandReverse;
    int mIdentity_Query_HealthCode;
    int mFaceRecognVedor;//厂商,0:ARC 虹软,1: baidu 百度 //Face Recognition vedor
private:
    QString mLoginPassword;//登陆密码
private:
    ReadConfig *const q_ptr;
};

ReadConfigPrivate::ReadConfigPrivate(ReadConfig *dd)
    : q_ptr(dd)
    , mPlatformType(0)
    , mWiegandReverse(0)
    , mIdentity_Query_HealthCode(0)
    , mFaceRecognVedor(1)
{
    this->Init();
}

ReadConfig::ReadConfig(QObject *parent)
    : QObject(parent)
    , d_ptr(new ReadConfigPrivate(this))
{
//    QThread *thread = new QThread;
//    this->moveToThread(thread);
//    thread->start();
}

ReadConfig::~ReadConfig()
{

}

void ReadConfigPrivate::Init()
{
    QSettings sysIniFile("/isc/isc_config.ini", QSettings::IniFormat);
    mRgbCameraRotation = sysIniFile.value("RGBCAMERA_ROTATION",  270).toInt();
    mIrCameraRotation = sysIniFile.value("IRCAMERA_ROTATION",  270).toInt();
    mTempSensorType = sysIniFile.value("TEMP_SENSOR_TYPE",  2).toInt();
    mPlatformType = sysIniFile.value("PlatformType",  0).toInt();
    mWiegandReverse = sysIniFile.value("WIEGAND_REVERSE",  0).toInt();
    mIdentity_Query_HealthCode = sysIniFile.value("Identity_Query_HealthCode",  0).toInt();
    mFaceRecognVedor = sysIniFile.value("FaceRecognVedor",  0).toInt();
#ifdef Q_OS_WIN
    QSettings IniFile("./parameters.ini", QSettings::IniFormat);
#else
    QSettings IniFile("/mnt/user/parameters.ini", QSettings::IniFormat);
#endif

    IniFile.beginGroup("Records_Manager");
    mRecords_Manager_t.PanoramaImg = IniFile.value("PanoramaImg",  0).toInt();
    mRecords_Manager_t.FaceImg = IniFile.value("FaceImg",  1).toInt();
    mRecords_Manager_t.Stranger = IniFile.value("Stranger",  0).toInt();
    IniFile.endGroup();

    IniFile.beginGroup("Network_Manager");
    mNetwork_Manager_t.Mode = IniFile.value("Mode",  1).toInt();//1 ,默认wifi
    IniFile.endGroup();

    IniFile.beginGroup("WIFI");
    mWIFI_t.Name = IniFile.value("Name",  "YNH").toString();
    mWIFI_t.Password = IniFile.value("Password",  "ynh123456").toString();
    IniFile.endGroup();

    IniFile.beginGroup("LAN");
    mLAN_t.DHCP = IniFile.value("DHCP",  1).toInt();
    mLAN_t.IP = IniFile.value("IP",  "192.168.8.123").toString();
    mLAN_t.Maks = IniFile.value("Maks",  "255.255.255.0").toString();
    mLAN_t.Gateway = IniFile.value("Gateway",  "192.168.8.1").toString();
    mLAN_t.DNS = IniFile.value("DNS",  "8.8.8.8").toString();
    IniFile.endGroup();

    IniFile.beginGroup("Srv_Manager");
    mSrv_Manager_t.Address = IniFile.value("Address",  "").toString();
    mSrv_Manager_t.Password = IniFile.value("Password",  "12345678").toString();
    IniFile.endGroup();

    IniFile.beginGroup("Post_PersonRecord");
    mPost_PersonRecord_t.Address = IniFile.value("Address",  "").toString();
    mPost_PersonRecord_t.Password = IniFile.value("Password",  "").toString();
    IniFile.endGroup();

    IniFile.beginGroup("Person_Registration");
    mPerson_Registration_t.Address = IniFile.value("Address",  "").toString();
    mPerson_Registration_t.Password = IniFile.value("Password",  "").toString();
    IniFile.endGroup();

    IniFile.beginGroup("SyncUsers");
    mSyncUsers_t.Address = IniFile.value("Address",  "").toString();
    mSyncUsers_t.Password = IniFile.value("Password",  "").toString();
    IniFile.endGroup();

    IniFile.beginGroup("UserDetail");
    mUserDetail_t.Address = IniFile.value("Address",  "").toString();
    mUserDetail_t.Password = IniFile.value("Password",  "").toString();
    IniFile.endGroup();
// Add this section with the other IniFile.beginGroup sections
    IniFile.beginGroup("DeviceInfo");
// Try to get device ID from system
mDeviceInfo_t.serialNumber = IniFile.value("SerialNumber", "DEFAULT_SERIAL").toString();
if (mDeviceInfo_t.serialNumber.isEmpty() || mDeviceInfo_t.serialNumber == "DEFAULT_SERIAL") {
    QFile file("/sys/class/net/eth0/address");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString mac = QString::fromUtf8(file.readAll()).trimmed();
        file.close();
        if (!mac.isEmpty()) {
            mDeviceInfo_t.serialNumber = "RV1109-" + mac.replace(":", "");
        }
    }
}
IniFile.endGroup();

    IniFile.beginGroup("Language");
    mLanguage_t.Mode = IniFile.value("Mode",  0).toInt();
    IniFile.endGroup();

    IniFile.beginGroup("Luminance");
    mLuminance_t.Value = IniFile.value("Value",  100).toInt();
    IniFile.endGroup();

    IniFile.beginGroup("FillLight");
    mFillLight_t.Value = IniFile.value("Value",  2).toInt();
    IniFile.endGroup();

    IniFile.beginGroup("Volume");
    mVolume_t.Value = IniFile.value("Value",  80).toInt();
    IniFile.endGroup();

    IniFile.beginGroup("ScreenOutDelay");
    mScreenOutDelay_t.Value = IniFile.value("Value",  0).toInt();
    IniFile.endGroup();

    IniFile.beginGroup("DebugMode");
    mDebugMode_t.Value = IniFile.value("Value",  0).toInt();
    IniFile.endGroup();
    
    IniFile.beginGroup("Door");
    mDoor_t.Type = IniFile.value("Type",  1).toInt();
    mDoor_t.Wiggins = IniFile.value("Wiggins",  1).toInt();
    mDoor_t.Timer = IniFile.value("Timer",  1).toInt();
    mDoor_t.MustOpenMode = IniFile.value("MustOpenMode", "2").toString();
    mDoor_t.OptionalOpenMode = IniFile.value("OptionalOpenMode",  "").toString();
    mDoor_t.Password = IniFile.value("Password",  "").toString();
    IniFile.endGroup();

    IniFile.beginGroup("Timer_Manager");
    mTimer_Manager_t.autoTime = IniFile.value("autoTime",  1).toInt();
    mTimer_Manager_t.autoZone = IniFile.value("autoZone",  1).toInt();
    mTimer_Manager_t.Zone = IniFile.value("Zone",  1).toInt();
    IniFile.endGroup();

    IniFile.beginGroup("Maintenance");
    mMaintenance_t.boot = IniFile.value("boot",  3).toInt();
    mMaintenance_t.bootTimer = IniFile.value("bootTimer",  "03:00").toString();
    mMaintenance_t.screenOffTime = IniFile.value("screenOffTime",  "0").toInt();
    IniFile.endGroup();

    IniFile.beginGroup("Identity_Manager");
    mIdentity_Manager_t.TempComp = IniFile.value("TempComp",  0).toFloat();
    mIdentity_Manager_t.AlarmTemp = IniFile.value("AlarmTemp",  37.3).toFloat();
    mIdentity_Manager_t.CheckLiving = IniFile.value("CheckLiving",  1).toInt();
    mIdentity_Manager_t.Living_value = IniFile.value("Living_value",  0.7).toFloat();
    mIdentity_Manager_t.Mask_value = IniFile.value("Mask_value",  0.25).toFloat();
    mIdentity_Manager_t.FqThreshold = IniFile.value("FqThreshold",  0.5).toFloat();
    mIdentity_Manager_t.IdentifyInterval = IniFile.value("IdentifyInterval",  5).toInt();
    mIdentity_Manager_t.Identifycm = IniFile.value("Identifycm",  1).toInt();

    mIdentity_Manager_t.Thanthreshold = IniFile.value("Thanthreshold",  0.8).toFloat();
    mIdentity_Manager_t.idcardThanthreshold = IniFile.value("idcardThanthreshold",  0.7).toFloat();
    //测温模式
    mIdentity_Manager_t.TemperatureMode = IniFile.value("TemperatureMode",  0).toInt();
    IniFile.endGroup();

    IniFile.beginGroup("HomeDisplay");
    //显示设备号
    mHomeDisplay_t.DisplaySnNum = IniFile.value("DisplaySnNum",  1).toInt();
    //显示MAC地址
    mHomeDisplay_t.DisplayMac = IniFile.value("DisplayMac",  1).toInt();
    //显示IP地址
    mHomeDisplay_t.DisplayIP = IniFile.value("DisplayIP",  1).toInt();
    //显示注册人数
    mHomeDisplay_t.DisplayPersonNum = IniFile.value("DisplayPersonNum",  1).toInt();
    mHomeDisplay_t.DeviceName = IniFile.value("DeviceName",  "").toString();
    mHomeDisplay_t.Location = IniFile.value("Location",  "").toString();
    mHomeDisplay_t.Language = IniFile.value("Language",  "zh").toString();
    IniFile.endGroup();

    IniFile.beginGroup("Login");
    mLoginPassword = IniFile.value("LoginPassword",  "123456").toString();
    IniFile.endGroup();
}

void ReadConfig::setReadConfig()
{
    Q_D(ReadConfig);
    /*控制播读音量*/
    YNH_LJX::Audio::setVolume(d->mVolume_t.Value);
    YNH_LJX::Utils_Door::GetInstance()->setOpenDoorWaitTime(d->mDoor_t.Timer);
    YNH_LJX::Wiegand::setWiegandReverse(d->mWiegandReverse);
    /*活体检测、活体阈值、温度补偿*/
    ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->setVivoDetection(d->mIdentity_Manager_t.CheckLiving);
    ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->setLivenessThreshold(d->mIdentity_Manager_t.Thanthreshold, d->mIdentity_Manager_t.Living_value);
    ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->setIdentityIdentifycm(d->mIdentity_Manager_t.Identifycm);
    qXLApp->GetSensorManager()->setTempComp(d->mIdentity_Manager_t.TempComp);
    qXLApp->GetSensorManager()->setTempMode(d->mIdentity_Manager_t.TemperatureMode);

    /*保存记录操作*/
    PersonRecordToDB::GetInstance()->setRecords_PanoramaImg(d->mRecords_Manager_t.PanoramaImg);
    PersonRecordToDB::GetInstance()->setRecords_FaceImg(d->mRecords_Manager_t.FaceImg);
    PersonRecordToDB::GetInstance()->setRecords_Stranger(d->mRecords_Manager_t.Stranger);
    /*识别算法 活体检测、报警温度、口罩阈值、人脸质量阈值、识别间隔、比对阈者、身份证比对阈值*/
    qXLApp->GetIdentityManagement()->setVivoDetection(d->mIdentity_Manager_t.CheckLiving);
    qXLApp->GetIdentityManagement()->setAlarmTemp(d->mIdentity_Manager_t.AlarmTemp);
    qXLApp->GetIdentityManagement()->setMask_value(d->mIdentity_Manager_t.Mask_value);
    qXLApp->GetIdentityManagement()->setFqThreshold(d->mIdentity_Manager_t.FqThreshold);
    qXLApp->GetIdentityManagement()->setIdentifyInterval(d->mIdentity_Manager_t.IdentifyInterval);
    qXLApp->GetIdentityManagement()->setThanthreshold(d->mIdentity_Manager_t.Thanthreshold);
    qXLApp->GetIdentityManagement()->setidcardThanthreshold(d->mIdentity_Manager_t.idcardThanthreshold);

    RegisteredFacesDB::GetInstance()->setThanthreshold(d->mIdentity_Manager_t.Thanthreshold);
    /*主界面显示设备标识*/
    qXLApp->GetFaceMainFrm()-> setHomeDisplay_SnNum(d->mHomeDisplay_t.DisplaySnNum);
    qXLApp->GetFaceMainFrm()-> setHomeDisplay_Mac(d->mHomeDisplay_t.DisplayMac);
    qXLApp->GetFaceMainFrm()-> setHomeDisplay_IP(d->mHomeDisplay_t.DisplayIP);
    qXLApp->GetFaceMainFrm()-> setHomeDisplay_PersonNum(d->mHomeDisplay_t.DisplayPersonNum);
    /*开门方式*/
    qXLApp->GetIdentityManagement()->setDoor_MustOpenMode(d->mDoor_t.MustOpenMode);
    qXLApp->GetIdentityManagement()->setDoor_OptionalOpenMode(d->mDoor_t.OptionalOpenMode);
    //qXLApp->GetIdentityManagement()->setDoor_hshours(d->mDoor_t.hshours);
    //qXLApp->GetFaceMainFrm()-> setHomeDisplay_DoorLock(d->mDoor_t.Password>"");
    //补光模式
    qXLApp->GetPowerManagerThread()->setFillLightMode(d->mFillLight_t.Value);
    SystemMaintenanceManage::GetInstance()->setBootTimer(d->mMaintenance_t.boot, d->mMaintenance_t.bootTimer);
}

void ReadConfig::setSaveConfig()
{
    Q_D(ReadConfig);
#ifdef Q_OS_WIN
    QSettings IniFile("./parameters.ini", QSettings::IniFormat);
#else
    QSettings IniFile("/mnt/user/parameters.ini", QSettings::IniFormat);
#endif
    IniFile.beginGroup("Records_Manager");
    IniFile.setValue("PanoramaImg", d->mRecords_Manager_t.PanoramaImg);
    IniFile.setValue("FaceImg", d->mRecords_Manager_t.FaceImg);
    IniFile.setValue("Stranger", d->mRecords_Manager_t.Stranger);
    IniFile.endGroup();

    IniFile.beginGroup("Network_Manager");
    IniFile.setValue("Mode", d->mNetwork_Manager_t.Mode);
    IniFile.endGroup();

    IniFile.beginGroup("WIFI");
    IniFile.setValue("Name", d->mWIFI_t.Name);
    IniFile.setValue("Password", d->mWIFI_t.Password);
    IniFile.endGroup();

    IniFile.beginGroup("LAN");
    IniFile.setValue("DHCP", d->mLAN_t.DHCP);
    IniFile.setValue("IP", d->mLAN_t.IP);
    IniFile.setValue("Maks", d->mLAN_t.Maks);
    IniFile.setValue("Gateway", d->mLAN_t.Gateway);
    IniFile.setValue("DNS", d->mLAN_t.DNS);
    IniFile.endGroup();

    IniFile.beginGroup("Srv_Manager");
    IniFile.setValue("Address", d->mSrv_Manager_t.Address);
    IniFile.setValue("Password", d->mSrv_Manager_t.Password);
    IniFile.endGroup();

    IniFile.beginGroup("Post_PersonRecord");
    IniFile.setValue("Address", d->mPost_PersonRecord_t.Address);
    IniFile.setValue("Password", d->mPost_PersonRecord_t.Password);
    IniFile.endGroup();

    IniFile.beginGroup("Person_Registration");
    IniFile.setValue("Address", d->mPerson_Registration_t.Address);
    IniFile.setValue("Password", d->mPerson_Registration_t.Password);
    IniFile.endGroup();

    IniFile.beginGroup("SyncUsers");
    IniFile.setValue("Address", d->mSyncUsers_t.Address);
    IniFile.setValue("Password", d->mSyncUsers_t.Password);
    IniFile.endGroup();

    IniFile.beginGroup("UserDetail");
    IniFile.setValue("Address", d->mUserDetail_t.Address);
    IniFile.setValue("Password", d->mUserDetail_t.Password);
    IniFile.endGroup();

    IniFile.beginGroup("Language");
    IniFile.setValue("Mode", d->mLanguage_t.Mode);
    IniFile.endGroup();

    IniFile.beginGroup("Luminance");
    IniFile.setValue("Value", d->mLuminance_t.Value);
    IniFile.endGroup();

    IniFile.beginGroup("FillLight");
    IniFile.setValue("Value", d->mFillLight_t.Value);
    IniFile.endGroup();

    IniFile.beginGroup("Volume");
    IniFile.setValue("Value", d->mVolume_t.Value);
    IniFile.endGroup();

    IniFile.beginGroup("ScreenOutDelay");
    IniFile.setValue("Value", d->mScreenOutDelay_t.Value);
    IniFile.endGroup();  

    IniFile.beginGroup("DebugMode");
    IniFile.setValue("Value", d->mDebugMode_t.Value);
    IniFile.endGroup();      

    IniFile.beginGroup("Door");
    IniFile.setValue("Type", d->mDoor_t.Type);
    IniFile.setValue("Wiggins", d->mDoor_t.Wiggins);
    IniFile.setValue("Timer", d->mDoor_t.Timer);
    IniFile.setValue("MustOpenMode",  d->mDoor_t.MustOpenMode);
    IniFile.setValue("OptionalOpenMode",  d->mDoor_t.OptionalOpenMode);
    IniFile.setValue("Password",  d->mDoor_t.Password);

    IniFile.endGroup();

    IniFile.beginGroup("Timer_Manager");
    IniFile.setValue("autoTime", d->mTimer_Manager_t.autoTime);
    IniFile.setValue("autoZone", d->mTimer_Manager_t.autoZone);
    IniFile.setValue("Zone", d->mTimer_Manager_t.Zone);
    IniFile.endGroup();

    IniFile.beginGroup("Maintenance");
    IniFile.setValue("boot", d->mMaintenance_t.boot);
    IniFile.setValue("bootTimer",  d->mMaintenance_t.bootTimer);
    IniFile.setValue("screenOffTime",  d->mMaintenance_t.screenOffTime);
    IniFile.endGroup();

    IniFile.beginGroup("Identity_Manager");
    IniFile.setValue("TempComp", QString::number(d->mIdentity_Manager_t.TempComp));
    IniFile.setValue("AlarmTemp", QString::number(d->mIdentity_Manager_t.AlarmTemp));
    IniFile.setValue("CheckLiving", d->mIdentity_Manager_t.CheckLiving);
    IniFile.setValue("Living_value", QString::number(d->mIdentity_Manager_t.Living_value));
    IniFile.setValue("Mask_value", QString::number(d->mIdentity_Manager_t.Mask_value));
    IniFile.setValue("FqThreshold", QString::number(d->mIdentity_Manager_t.FqThreshold));
    IniFile.setValue("IdentifyInterval", d->mIdentity_Manager_t.IdentifyInterval);
    IniFile.setValue("Identifycm", d->mIdentity_Manager_t.Identifycm);
    IniFile.setValue("Thanthreshold",  QString::number(d->mIdentity_Manager_t.Thanthreshold));
    IniFile.setValue("idcardThanthreshold",  QString::number(d->mIdentity_Manager_t.idcardThanthreshold));
    //测温模式
    IniFile.setValue("TemperatureMode",  QString::number(d->mIdentity_Manager_t.TemperatureMode));
    IniFile.endGroup();

    IniFile.beginGroup("HomeDisplay");
    IniFile.setValue("DisplaySnNum", d->mHomeDisplay_t.DisplaySnNum);
    IniFile.setValue("DisplayMac", d->mHomeDisplay_t.DisplayMac);
    IniFile.setValue("DisplayIP", d->mHomeDisplay_t.DisplayIP);
    IniFile.setValue("DisplayPersonNum", d->mHomeDisplay_t.DisplayPersonNum);
    IniFile.setValue("DeviceName", d->mHomeDisplay_t.DeviceName);
    IniFile.setValue("Language", d->mHomeDisplay_t.Language);
    IniFile.setValue("Location", d->mHomeDisplay_t.Location);
    IniFile.endGroup();

    IniFile.beginGroup("Login");
    IniFile.setValue("LoginPassword",  d->mLoginPassword);
    IniFile.endGroup();
    IniFile.sync();
    myHelper::Utils_ExecCmd("sync");

// Add this section with the other IniFile.beginGroup sections
    IniFile.beginGroup("DeviceInfo");
    IniFile.setValue("SerialNumber", d->mDeviceInfo_t.serialNumber);
    IniFile.endGroup();

    IniFile.beginGroup("Sync_Settings");
    IniFile.setValue("SyncEnabled", 0); // Default OFF state
    IniFile.endGroup();

}

int ReadConfig::getTempSensorType() const
{
    return d_func()->mTempSensorType;
}

int ReadConfig::getIrCameraRotation() const
{
    QSettings param("/param/RV1109_PARAM.txt", QSettings::IniFormat);
    int IrCameraRotation = param.value("IRCAMERA_ROTATION",  -1).toInt();
    if ( (IrCameraRotation == 0) || (IrCameraRotation == 90) || (IrCameraRotation ==180) || (IrCameraRotation ==270) )
    {
        return IrCameraRotation;   
    }
    else     
    return d_func()->mIrCameraRotation;
}

int ReadConfig::getRgbCameraRotation() const
{    
    QSettings param("/param/RV1109_PARAM.txt", QSettings::IniFormat);

    int RgbCameraRotation = param.value("RGBCAMERA_ROTATION",  -1).toInt();
    if ( (RgbCameraRotation == 0) || (RgbCameraRotation == 90) || (RgbCameraRotation ==180) || (RgbCameraRotation ==270) )
    {
        printf(">>>%s,%s,%d,RgbCameraRotation=%d\n",__FILE__,__func__,__LINE__,RgbCameraRotation);
        return RgbCameraRotation;   
    }
    else 
      return d_func()->mRgbCameraRotation;
}

int ReadConfig::getPlatformType() const
{
   return d_func()->mPlatformType;
}

int ReadConfig::getWiegandReverse() const
{
    return d_func()->mWiegandReverse;
}

int ReadConfig::getIdentity_Query_HealthCode() const
{
    return d_func()->mIdentity_Query_HealthCode;
}

int ReadConfig::getFaceRecognVedor() const
{
   return d_func()->mFaceRecognVedor;
}

void ReadConfig::setLoginPassword(const QString &pasw)
{
    Q_D(ReadConfig);
    d->mLoginPassword = pasw;
}

QString ReadConfig::getLoginPassword() const
{
    return d_func()->mLoginPassword;
}

int ReadConfig::getRecords_Manager_PanoramaImg() const
{
    return d_func()->mRecords_Manager_t.PanoramaImg;
}

int ReadConfig::getRecords_Manager_FaceImg() const
{
    return d_func()->mRecords_Manager_t.FaceImg;
}

int ReadConfig::getRecords_Manager_Stranger() const
{
    return d_func()->mRecords_Manager_t.Stranger;
}

void ReadConfig::setRecords_Manager_PanoramaImg(const int &value)
{
    Q_D(ReadConfig);
    d->mRecords_Manager_t.PanoramaImg = value;
    PersonRecordToDB::GetInstance()->setRecords_PanoramaImg(value);
}

void ReadConfig::setRecords_Manager_FaceImg(const int &value)
{
    Q_D(ReadConfig);
    d->mRecords_Manager_t.FaceImg = value;
    PersonRecordToDB::GetInstance()->setRecords_FaceImg(value);
}

void ReadConfig::setRecords_Manager_Stranger(const int &value)
{
    Q_D(ReadConfig);
    d->mRecords_Manager_t.Stranger = value;
    PersonRecordToDB::GetInstance()->setRecords_Stranger(value);
}

int ReadConfig::getNetwork_Manager_Mode() const
{
    return d_func()->mNetwork_Manager_t.Mode;
}

void ReadConfig::setNetwork_Manager_Mode(const int &value)
{
    Q_D(ReadConfig);
    d->mNetwork_Manager_t.Mode = value;
}

QString ReadConfig::getWIFI_Name() const
{
    return d_func()->mWIFI_t.Name;
}

QString ReadConfig::getWIFI_Password() const
{
    return d_func()->mWIFI_t.Password;
}

void ReadConfig::setWIFI_Name(const QString &name)
{
    Q_D(ReadConfig);
    d->mWIFI_t.Name = name;
}

void ReadConfig::setWIFI_Password(const QString &pasw)
{
    Q_D(ReadConfig);
    d->mWIFI_t.Password = pasw;
}

int ReadConfig::getLan_DHCP() const
{
    return d_func()->mLAN_t.DHCP;
}

QString ReadConfig::getLAN_IP() const
{
    return d_func()->mLAN_t.IP;
}

QString ReadConfig::getLAN_Maks() const
{
    return d_func()->mLAN_t.Maks;
}

QString ReadConfig::getLAN_Gateway() const
{
    return d_func()->mLAN_t.Gateway;
}

QString ReadConfig::getLAN_DNS() const
{
    return d_func()->mLAN_t.DNS;
}

void ReadConfig::setLAN_DHCP(const int &dhcp)
{
    Q_D(ReadConfig);
    d->mLAN_t.DHCP = dhcp;
}

void ReadConfig::setLAN_IP(const QString &ip)
{
    Q_D(ReadConfig);
    d->mLAN_t.IP = ip;
}

void ReadConfig::setLAN_Maks(const QString &maks)
{
    Q_D(ReadConfig);
    d->mLAN_t.Maks = maks;
}

void ReadConfig::setLAN_Gateway(const QString &gateway)
{
    Q_D(ReadConfig);
    d->mLAN_t.Gateway = gateway;
}

void ReadConfig::setLAN_DNS(const QString &dns)
{
    Q_D(ReadConfig);
    d->mLAN_t.DNS = dns;
}

QString ReadConfig::getSrv_Manager_Address() const
{
    return d_func()->mSrv_Manager_t.Address;
}

QString ReadConfig::getSrv_Manager_Password() const
{
    return d_func()->mSrv_Manager_t.Password;
}

void ReadConfig::setSrv_Manager_Address(const QString &address)
{
    Q_D(ReadConfig);
    d->mSrv_Manager_t.Address = address;
}

void ReadConfig::setSrv_Manager_Password(const QString &pawss)
{
    Q_D(ReadConfig);
    d->mSrv_Manager_t.Password =pawss;
}

QString ReadConfig::getPost_PersonRecord_Address() const
{
    return d_func()->mPost_PersonRecord_t.Address;
}

QString ReadConfig::getPost_PersonRecord_Password() const
{
    return d_func()->mPost_PersonRecord_t.Password;
}

void ReadConfig::setPost_PersonRecord_Address(const QString &address)
{
    Q_D(ReadConfig);
    d->mPost_PersonRecord_t.Address = address;
}

void ReadConfig::setPost_PersonRecord_Password(const QString &pawss)
{
    Q_D(ReadConfig);
    d->mPost_PersonRecord_t.Password =pawss;
}

QString ReadConfig::getPerson_Registration_Address() const
{
    return d_func()->mPerson_Registration_t.Address;
}

QString ReadConfig::getPerson_Registration_Password() const
{
    return d_func()->mPerson_Registration_t.Password;
}

void ReadConfig::setPerson_Registration_Address(const QString &address)
{
    Q_D(ReadConfig);
    d->mPerson_Registration_t.Address = address;
}

void ReadConfig::setPerson_Registration_Password(const QString &pawss)
{
    Q_D(ReadConfig);
    d->mPerson_Registration_t.Password =pawss;
}

QString ReadConfig::getSyncUsersAddress() const
{
    return d_func()->mSyncUsers_t.Address;
}

QString ReadConfig::getSyncUsersPassword() const
{
    return d_func()->mSyncUsers_t.Password;
}

void ReadConfig::setSyncUsersAddress(const QString &address)
{
    Q_D(ReadConfig);
    d->mSyncUsers_t.Address = address;
}

void ReadConfig::setSyncUsersPassword(const QString &pawss)
{
    Q_D(ReadConfig);
    d->mSyncUsers_t.Password =pawss;
}
QString ReadConfig::getUserDetailAddress() const
{
    return d_func()->mUserDetail_t.Address;
}

QString ReadConfig::getUserDetailPassword() const
{
    return d_func()->mUserDetail_t.Password;
}

void ReadConfig::setUserDetailAddress(const QString &address)
{
    Q_D(ReadConfig);
    d->mUserDetail_t.Address = address;
}

void ReadConfig::setUserDetailPassword(const QString &pawss)
{
    Q_D(ReadConfig);
    d->mUserDetail_t.Password =pawss;
}
int ReadConfig::getLanguage_Mode() const
{
    return d_func()->mLanguage_t.Mode;
}

void ReadConfig::setLanguage_Mode(const int &mode)
{
    Q_D(ReadConfig);
    d->mLanguage_t.Mode = mode;
}

int ReadConfig::getLuminance_Value() const
{
    return d_func()->mLuminance_t.Value;
}

void ReadConfig::setLuminance_Value(const int &value)
{
    Q_D(ReadConfig);
    d->mLuminance_t.Value = value;
}

int ReadConfig::getFillLight_Value() const
{
    return d_func()->mFillLight_t.Value;
}

void ReadConfig::setFillLight_Value(const int &value)
{
    Q_D(ReadConfig);
    d->mFillLight_t.Value = value;
    qXLApp->GetPowerManagerThread()->setFillLightMode(d->mFillLight_t.Value);
}

int ReadConfig::getVolume_Value() const
{
    return d_func()->mVolume_t.Value;
}

void ReadConfig::setVolume_Value(const int &value)
{
    Q_D(ReadConfig);
    d->mVolume_t.Value = value;
    YNH_LJX::Audio::setVolume(value);
}

int ReadConfig::getScreenOutDelay_Value() const
{
    return d_func()->mScreenOutDelay_t.Value;
}

void ReadConfig::setScreenOutDelay_Value(const int &value)
{
    Q_D(ReadConfig);
    d->mScreenOutDelay_t.Value = value;
}

int ReadConfig::getDebugMode_Value() const
{
    return d_func()->mDebugMode_t.Value;
}

void ReadConfig::setDebugMode_Value(const int &value)
{
    Q_D(ReadConfig);
    d->mDebugMode_t.Value = value;
}

int ReadConfig::getDoor_Type() const
{
    return d_func()->mDoor_t.Type;
}

int ReadConfig::getDoor_Relay() const
{
    return d_func()->mDoor_t.Relay;
}

int ReadConfig::getDoor_Wiggins() const
{
    return d_func()->mDoor_t.Wiggins;
}

int ReadConfig::getDoor_Timer() const
{
    return d_func()->mDoor_t.Timer;
}

QString ReadConfig::getDoor_MustOpenMode() const
{
    return d_func()->mDoor_t.MustOpenMode;
}

QString ReadConfig::getDoor_OptionalOpenMode() const
{
    return d_func()->mDoor_t.OptionalOpenMode;
}

QString ReadConfig::getDoor_Password() const
{
    return d_func()->mDoor_t.Password;
}

void ReadConfig::setDoor_Type(const int &value)
{
    Q_D(ReadConfig);
    d->mDoor_t.Type = value;
}


void ReadConfig::setDoor_Relay(const int &value)
{
    Q_D(ReadConfig);
    d->mDoor_t.Relay = value;
}

void ReadConfig::setDoor_Wiggins(const int &value)
{
    Q_D(ReadConfig);
    d->mDoor_t.Wiggins = value;
}

void ReadConfig::setDoor_Timer(const int &value)
{
    Q_D(ReadConfig);
    d->mDoor_t.Timer = value;
    YNH_LJX::Utils_Door::GetInstance()->setOpenDoorWaitTime(d->mDoor_t.Timer);
}

void ReadConfig::setDoor_MustOpenMode(const QString &mode)
{
    Q_D(ReadConfig);
    d->mDoor_t.MustOpenMode = mode;
    qXLApp->GetIdentityManagement()->setDoor_MustOpenMode(d->mDoor_t.MustOpenMode);
}

void ReadConfig::setDoor_OptionalOpenMode(const QString &mode)
{
    Q_D(ReadConfig);
    d->mDoor_t.OptionalOpenMode = mode;
    qXLApp->GetIdentityManagement()->setDoor_OptionalOpenMode(d->mDoor_t.OptionalOpenMode);
}
void ReadConfig::setDoor_Password(const QString &password)
{
    Q_D(ReadConfig);
    d->mDoor_t.Password = password;
}

int ReadConfig::getTimer_Manager_autoTime() const
{
    return d_func()->mTimer_Manager_t.autoTime;
}

int ReadConfig::getTimer_Manager_autoZone() const
{
    return d_func()->mTimer_Manager_t.autoZone;
}

int ReadConfig::getTimer_Manager_Zone() const
{
    return d_func()->mTimer_Manager_t.Zone;
}

void ReadConfig::setTimer_Manager_autoTime(const int &value)
{
    Q_D(ReadConfig);
    d->mTimer_Manager_t.autoTime = value;
}

void ReadConfig::setTimer_Manager_autoZone(const int &value)
{
    Q_D(ReadConfig);
    d->mTimer_Manager_t.autoZone = value;
}

void ReadConfig::setTimer_Manager_Zone(const int &value)
{
    Q_D(ReadConfig);
    d->mTimer_Manager_t.Zone = value;
}

int ReadConfig::getMaintenance_boot() const
{
    return d_func()->mMaintenance_t.boot;
}

QString ReadConfig::getMaintenance_bootTimer() const
{
    return d_func()->mMaintenance_t.bootTimer;
}

void ReadConfig::setMaintenance_boot(const int &value)
{
    Q_D(ReadConfig);
    d->mMaintenance_t.boot = value;
}

void ReadConfig::setMaintenance_bootTimer(const QString &value)
{
    Q_D(ReadConfig);
    d->mMaintenance_t.bootTimer = value;
    SystemMaintenanceManage::GetInstance()->setBootTimer(d->mMaintenance_t.boot, d->mMaintenance_t.bootTimer);
}

int ReadConfig::getMaintenance_screenOffTime() const
{
	return d_func()->mMaintenance_t.screenOffTime;
}

void ReadConfig::setMaintenance_screenOffTime(const int &value)
{
    Q_D(ReadConfig);
    d->mMaintenance_t.screenOffTime = value;
}

float ReadConfig::getIdentity_Manager_TempComp() const
{
    return d_func()->mIdentity_Manager_t.TempComp;
}

float ReadConfig::getIdentity_Manager_AlarmTemp() const
{
    return d_func()->mIdentity_Manager_t.AlarmTemp;
}

int ReadConfig::getIdentity_Manager_CheckLiving() const
{
    return d_func()->mIdentity_Manager_t.CheckLiving;
}

float ReadConfig::getIdentity_Manager_Living_value() const
{
    return d_func()->mIdentity_Manager_t.Living_value;
}

float ReadConfig::getIdentity_Manager_Mask_value() const
{
    return d_func()->mIdentity_Manager_t.Mask_value;
}

float ReadConfig::getIdentity_Manager_FqThreshold() const
{
    return d_func()->mIdentity_Manager_t.FqThreshold;
}

int ReadConfig::getIdentity_Manager_IdentifyInterval() const
{
    return d_func()->mIdentity_Manager_t.IdentifyInterval;
}

int ReadConfig::getIdentity_Manager_Identifycm() const
{
    return d_func()->mIdentity_Manager_t.Identifycm;
}

float ReadConfig::getIdentity_Manager_Thanthreshold() const
{
    return d_func()->mIdentity_Manager_t.Thanthreshold;
}

float ReadConfig::getIdentity_Manager_idcardThanthreshold() const
{
    return d_func()->mIdentity_Manager_t.idcardThanthreshold;
}

int ReadConfig::getIdentity_Manager_TemperatureMode() const
{
    return d_func()->mIdentity_Manager_t.TemperatureMode;
}

void ReadConfig::setIdentity_Manager_TempComp(const float &value)
{
    Q_D(ReadConfig);
    d->mIdentity_Manager_t.TempComp = value;
    qXLApp->GetSensorManager()->setTempComp(value);
}

void ReadConfig::setIdentity_Manager_AlarmTemp(const float &value)
{//报警温度
    Q_D(ReadConfig);
    d->mIdentity_Manager_t.AlarmTemp = value;
    qXLApp->GetIdentityManagement()->setAlarmTemp(value);
}

void ReadConfig::setIdentity_Manager_CheckLiving(const int &value)
{//检测活体
    Q_D(ReadConfig);
    d->mIdentity_Manager_t.CheckLiving = value;
    ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->setVivoDetection(value);
    qXLApp->GetIdentityManagement()->setVivoDetection(value);
}

void ReadConfig::setIdentity_Manager_Living_value(const float &value)
{//活体阈值
    Q_D(ReadConfig);
    d->mIdentity_Manager_t.Living_value = value;
    ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->setLivenessThreshold(d->mIdentity_Manager_t.Thanthreshold, value);
}

void ReadConfig::setIdentity_Manager_Mask_value(const float &value)
{//口罩阈值
    Q_D(ReadConfig);
    d->mIdentity_Manager_t.Mask_value = value;
    qXLApp->GetIdentityManagement()->setMask_value(value);
}

void ReadConfig::setIdentity_Manager_FqThreshold(const float &value)
{//人脸质量阈值
    Q_D(ReadConfig);
    d->mIdentity_Manager_t.FqThreshold = value;
    qXLApp->GetIdentityManagement()->setFqThreshold(value);
}

void ReadConfig::setIdentity_Manager_IdentifyInterval(const int &value)
{//识别间隔
    Q_D(ReadConfig);
    d->mIdentity_Manager_t.IdentifyInterval = value;
    qXLApp->GetIdentityManagement()->setIdentifyInterval(value);
}

void ReadConfig::setIdentity_Manager_Identifycm(const int &value)
{//识别距离
    Q_D(ReadConfig);
    d->mIdentity_Manager_t.Identifycm = value;
    ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->setIdentityIdentifycm(value);
}

void ReadConfig::setIdentity_Manager_Thanthreshold(const float &value)
{//比对阈值
    Q_D(ReadConfig);
    d->mIdentity_Manager_t.Thanthreshold = value;
    qXLApp->GetIdentityManagement()->setThanthreshold(value);
    RegisteredFacesDB::GetInstance()->setThanthreshold(d->mIdentity_Manager_t.Thanthreshold);
    ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->setLivenessThreshold(value, d->mIdentity_Manager_t.Living_value);
}

void ReadConfig::setIdentity_Manager_idcardThanthreshold(const float &value)
{//身份证比对阈值
    Q_D(ReadConfig);
    d->mIdentity_Manager_t.idcardThanthreshold = value;
    qXLApp->GetIdentityManagement()->setidcardThanthreshold(value);
}

void ReadConfig::setIdentity_Manager_TemperatureMode(const int &value)
{//测温环境
    Q_D(ReadConfig);
    d->mIdentity_Manager_t.TemperatureMode = value;
    qXLApp->GetSensorManager()->setTempMode(d->mIdentity_Manager_t.TemperatureMode);
}

void ReadConfig::setHomeDisplay_DisplaySnNum(const int &show)
{//显示设备号
    Q_D(ReadConfig);
    d->mHomeDisplay_t.DisplaySnNum = show;
    qXLApp->GetFaceMainFrm()-> setHomeDisplay_SnNum(d->mHomeDisplay_t.DisplaySnNum);
}

void ReadConfig::setHomeDisplay_DisplayMac(const int &show)
{//显示mac地址
    Q_D(ReadConfig);
    d->mHomeDisplay_t.DisplayMac = show;
    qXLApp->GetFaceMainFrm()-> setHomeDisplay_Mac(d->mHomeDisplay_t.DisplayMac);
}

void ReadConfig::setHomeDisplay_DisplayIP(const int &show)
{//显示IP
    Q_D(ReadConfig);
    d->mHomeDisplay_t.DisplayIP = show;
    qXLApp->GetFaceMainFrm()-> setHomeDisplay_IP(d->mHomeDisplay_t.DisplayIP);
}

void ReadConfig::setHomeDisplay_DisplayPersonNum(const int &show)
{//显示实名制人员
    Q_D(ReadConfig);
    d->mHomeDisplay_t.DisplayPersonNum = show;
    qXLApp->GetFaceMainFrm()-> setHomeDisplay_PersonNum(d->mHomeDisplay_t.DisplayPersonNum);
}

int ReadConfig::getHomeDisplay_DisplaySnNum() const
{
    return d_func()->mHomeDisplay_t.DisplaySnNum;
}

int ReadConfig::getHomeDisplay_DisplayMac() const
{
    return d_func()->mHomeDisplay_t.DisplayMac;
}

int ReadConfig::getHomeDisplay_DisplayIP() const
{
    return d_func()->mHomeDisplay_t.DisplayIP;
}

int ReadConfig::getHomeDisplay_DisplayPersonNum() const
{
    return d_func()->mHomeDisplay_t.DisplayPersonNum;
}

void ReadConfig::setHomeDisplay_DeviceName(const QString &deviceName)
{
//设置设备名称
    Q_D(ReadConfig);
    d->mHomeDisplay_t.DeviceName = deviceName;
}

QString ReadConfig::getHomeDisplay_DeviceName() const
{
	return d_func()->mHomeDisplay_t.DeviceName;
}

void ReadConfig::setHomeDisplay_Location(const QString &location)
{
    Q_D(ReadConfig);
    d->mHomeDisplay_t.Location = location;
}

QString ReadConfig::getHomeDisplay_Location()const
{
	return d_func()->mHomeDisplay_t.Location;
}


void ReadConfig::setHomeDisplay_DeviceLanguage(const QString &language)
{
    Q_D(ReadConfig);
    d->mHomeDisplay_t.Language = language;
}

QString ReadConfig::getHomeDisplay_Language()const
{
	return d_func()->mHomeDisplay_t.Location;
}

QString ReadConfig::getDeviceSerialNumber() const
{
    return d_func()->mDeviceInfo_t.serialNumber;
}

void ReadConfig::setDeviceSerialNumber(const QString &serialNumber)
{
    Q_D(ReadConfig);
    d->mDeviceInfo_t.serialNumber = serialNumber;
}

int ReadConfig::getSyncEnabled() const
{
    QSettings IniFile("/mnt/user/parameters.ini", QSettings::IniFormat);
    return IniFile.value("Sync_Settings/EnableSync", 0).toInt(); // Default OFF
}

void ReadConfig::setSyncEnabled(int state)
{
    QSettings IniFile("/mnt/user/parameters.ini", QSettings::IniFormat);
    IniFile.beginGroup("Sync_Settings");
    IniFile.setValue("EnableSync", state);
    IniFile.endGroup();
    IniFile.sync();
}
