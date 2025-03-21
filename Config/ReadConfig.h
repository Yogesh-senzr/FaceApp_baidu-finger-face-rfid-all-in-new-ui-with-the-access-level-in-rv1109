#ifndef READCONFIG_H
#define READCONFIG_H

#include <QtCore/QObject>

class ReadConfigPrivate;
class ReadConfig : public QObject
{
    Q_OBJECT
public:
    explicit ReadConfig(QObject *parent = nullptr);
    ~ReadConfig();
public:
    static inline ReadConfig *GetInstance(){static ReadConfig g;return &g;}
public:
    void setReadConfig();
    void setSaveConfig();
public:
    int getTempSensorType()const;
    int getIrCameraRotation()const;
    int getRgbCameraRotation()const;
public:
    int getPlatformType()const;
    int getWiegandReverse()const;
    int getIdentity_Query_HealthCode()const;//身份证查询健康码
    int getFaceRecognVedor()const;
public:
    void setLoginPassword(const QString &);
    QString getLoginPassword()const;
public:
    int getRecords_Manager_PanoramaImg()const;
    int getRecords_Manager_FaceImg()const;
    int getRecords_Manager_Stranger()const;

    void setRecords_Manager_PanoramaImg(const int &value);
    void setRecords_Manager_FaceImg(const int &value);
    void setRecords_Manager_Stranger(const int &value);
    //1是LAN， 2是WIFI， 3是4G
    int getNetwork_Manager_Mode()const;
    void setNetwork_Manager_Mode(const int &value);

    QString getWIFI_Name()const;
    QString getWIFI_Password()const;
    void setWIFI_Name(const QString &);
    void setWIFI_Password(const QString &);

    int getLan_DHCP()const;
    QString getLAN_IP()const;
    QString getLAN_Maks()const;
    QString getLAN_Gateway()const;
    QString getLAN_DNS()const;
    void setLAN_DHCP(const int &);
    void setLAN_IP(const QString &);
    void setLAN_Maks(const QString &);
    void setLAN_Gateway(const QString &);
    void setLAN_DNS(const QString &);

    QString getSrv_Manager_Address()const;
    QString getSrv_Manager_Password()const;
    void setSrv_Manager_Address(const QString &);
    void setSrv_Manager_Password(const QString &);

    QString getPost_PersonRecord_Address()const;
    QString getPost_PersonRecord_Password()const;
    void setPost_PersonRecord_Address(const QString &);
    void setPost_PersonRecord_Password(const QString &);

    QString getPerson_Registration_Address()const;
    QString getPerson_Registration_Password()const;
    void setPerson_Registration_Address(const QString &);
    void setPerson_Registration_Password(const QString &);

    QString getSyncUsersAddress()const;
    QString getSyncUsersPassword()const;
    void setSyncUsersAddress(const QString &);
    void setSyncUsersPassword(const QString &);

    QString getUserDetailAddress()const;
    QString getUserDetailPassword()const;
    void setUserDetailAddress(const QString &);
    void setUserDetailPassword(const QString &);

    int getLanguage_Mode()const;
    void setLanguage_Mode(const int &);

    int getLuminance_Value()const;
    void setLuminance_Value(const int &);

    int getFillLight_Value()const;
    void setFillLight_Value(const int &);

    int getVolume_Value()const;
    void setVolume_Value(const int &);

    int getScreenOutDelay_Value()const;
    void setScreenOutDelay_Value(const int &);

    int getDebugMode_Value()const;
    void setDebugMode_Value(const int &);

    int getDoor_Type()const;
    int getDoor_Wiggins()const;
    int getDoor_Timer()const;
	int getDoor_Relay()const;
    QString getDoor_MustOpenMode()const;
    QString getDoor_OptionalOpenMode()const;
    QString getDoor_Password()const;
    void setDoor_Type(const int &);
    void setDoor_Wiggins(const int &);
    void setDoor_Timer(const int &);
    void setDoor_MustOpenMode(const QString &);
    void setDoor_OptionalOpenMode(const QString &);
    void setDoor_Password(const QString &);
	void setDoor_Relay(const int &);

    int getTimer_Manager_autoTime()const;
    int getTimer_Manager_autoZone()const;
    int getTimer_Manager_Zone()const;
    void setTimer_Manager_autoTime(const int &);
    void setTimer_Manager_autoZone(const int &);
    void setTimer_Manager_Zone(const int &);

    int getMaintenance_boot()const;
    QString getMaintenance_bootTimer()const;
    void setMaintenance_boot(const int &);
    void setMaintenance_bootTimer(const QString &);
    int getMaintenance_screenOffTime()const;
    void setMaintenance_screenOffTime(const int &);

    float getIdentity_Manager_TempComp()const;
    float getIdentity_Manager_AlarmTemp()const;
    int getIdentity_Manager_CheckLiving()const;
    float getIdentity_Manager_Living_value()const;
    float getIdentity_Manager_Mask_value()const;
    float getIdentity_Manager_FqThreshold()const;
    int getIdentity_Manager_IdentifyInterval()const;
    int getIdentity_Manager_Identifycm()const;
    float getIdentity_Manager_Thanthreshold()const;
    float getIdentity_Manager_idcardThanthreshold()const;
    int getIdentity_Manager_TemperatureMode()const;

    void setIdentity_Manager_TempComp(const float &);
    void setIdentity_Manager_AlarmTemp(const float &);
    void setIdentity_Manager_CheckLiving(const int &);
    void setIdentity_Manager_Living_value(const float &);
    void setIdentity_Manager_Mask_value(const float &);
    void setIdentity_Manager_FqThreshold(const float &);
    void setIdentity_Manager_IdentifyInterval(const int &);
    void setIdentity_Manager_Identifycm(const int &);
    void setIdentity_Manager_Thanthreshold(const float &);
    void setIdentity_Manager_idcardThanthreshold(const float &);
    void setIdentity_Manager_TemperatureMode(const int &);

    void setHomeDisplay_DisplaySnNum(const int &);
    void setHomeDisplay_DisplayMac(const int &);
    void setHomeDisplay_DisplayIP(const int &);
    void setHomeDisplay_DisplayPersonNum(const int &);
    void setHomeDisplay_DeviceName(const QString &);
    void setHomeDisplay_Location(const QString &);
    void setHomeDisplay_DeviceLanguage(const QString &);
    int getHomeDisplay_DisplaySnNum()const;
    int getHomeDisplay_DisplayMac()const;
    int getHomeDisplay_DisplayIP()const;
    int getHomeDisplay_DisplayPersonNum()const;
    QString getHomeDisplay_DeviceName()const;
    QString getHomeDisplay_Location()const;
    QString getHomeDisplay_Language()const;

public:
    int getSyncEnabled() const;  // Function to get sync state
    void setSyncEnabled(int state);  // Function to set sync state
    

public:
    QString getDeviceSerialNumber() const;
    void setDeviceSerialNumber(const QString &serialNumber);
private:
    QScopedPointer<ReadConfigPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(ReadConfig)
    Q_DISABLE_COPY(ReadConfig)
};

#endif // READCONFIG_H

