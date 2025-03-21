#ifndef SensorManager_H
#define SensorManager_H

#include <QThread>


#define TEMP_LOW 34.0

//温度传感器管理
class SensorManagerPrivate;
class SensorManager : public QThread
{
    Q_OBJECT
public:
    explicit SensorManager(QObject *parent = nullptr);
    ~SensorManager();
public:
    void setRunSensor(const int sensorType);
    void setTempComp(const float &);//温度补偿
    void setTempMode(const int);//测温环境(户外就是假的数据)
public:
    float GetSensorFloatValue()const;//获取温度值
public:
    Q_SIGNAL void sigTemperatureValue(const float);
    //Q_SIGNAL void sigYKHealthCodeMsg(const int type, const QString name, const QString idCard, const int qrCodeType, const double warningTemp, const QString msg);    
private:
    void run();
private:
    QScopedPointer<SensorManagerPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(SensorManager)
    Q_DISABLE_COPY(SensorManager)
};

#endif // SensorManager_H
