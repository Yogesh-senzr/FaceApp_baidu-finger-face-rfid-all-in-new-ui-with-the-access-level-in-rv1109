// DeviceInfo.cpp
#include "DeviceInfo.h"
#include <QFile>
#include <QProcess>
#include <QDebug>

DeviceInfo* DeviceInfo::m_instance = nullptr;

DeviceInfo* DeviceInfo::getInstance()
{
    if (m_instance == nullptr) {
        m_instance = new DeviceInfo();
    }
    return m_instance;
}

DeviceInfo::DeviceInfo(QObject *parent) : QObject(parent)
{
    // Try different methods to get serial number
    m_serialNumber = readSerialFromFile();
    if (m_serialNumber.isEmpty()) {
        m_serialNumber = readSerialFromDMI();
    }
    if (m_serialNumber.isEmpty()) {
        m_serialNumber = readSerialFromCPUInfo();
    }
}

QString DeviceInfo::getSerialNumber()
{
    return m_serialNumber;
}

QString DeviceInfo::readSerialFromFile()
{
    // First try to read from a custom file where device serial might be stored
    QFile file("/etc/device-serial");
    if (file.open(QIODevice::ReadOnly)) {
        QString serial = QString::fromUtf8(file.readAll()).trimmed();
        file.close();
        return serial;
    }
    return QString();
}

QString DeviceInfo::readSerialFromDMI()
{
    // Try to read system serial number using dmidecode
    QProcess process;
    process.start("dmidecode", QStringList() << "-s" << "system-serial-number");
    process.waitForFinished();
    if (process.exitCode() == 0) {
        QString serial = QString::fromUtf8(process.readAll()).trimmed();
        if (!serial.isEmpty() && serial != "System Serial Number") {
            return serial;
        }
    }
    return QString();
}

QString DeviceInfo::readSerialFromCPUInfo()
{
    // Try to read from CPU info as fallback
    QFile file("/proc/cpuinfo");
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        file.close();
        
        // Look for Serial or UUID in cpuinfo
        QStringList lines = QString::fromUtf8(data).split('\n');
        for (const QString& line : lines) {
            if (line.startsWith("Serial")) {
                return line.split(':').value(1).trimmed();
            }
        }
    }
    return QString();
}
