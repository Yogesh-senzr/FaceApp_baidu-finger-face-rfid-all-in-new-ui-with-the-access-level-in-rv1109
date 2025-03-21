// DeviceInfo.h
#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <QString>
#include <QObject>

class DeviceInfo : public QObject
{
    Q_OBJECT
public:
    static DeviceInfo* getInstance();
    QString getSerialNumber();

private:
    explicit DeviceInfo(QObject *parent = nullptr);
    QString readSerialFromFile();
    QString readSerialFromDMI();
    QString readSerialFromCPUInfo();
    
private:
    static DeviceInfo* m_instance;
    QString m_serialNumber;
};

#endif 
