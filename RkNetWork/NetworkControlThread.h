#ifndef NetworkControlThread_H
#define NetworkControlThread_H

#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include <QList>
#include <QVariant>

class NetworkControlThread : public QThread
{
    Q_OBJECT
public:
    NetworkControlThread(QObject *parent = Q_NULLPTR);
    ~NetworkControlThread();
public:
    static inline NetworkControlThread *GetInstance(){static NetworkControlThread g;return &g;}
public:
    Q_SIGNAL void sigWifiList(const QList<QVariant>);
public:
    //设置wifi查找模式(1:扫描wifi, 2:查找ssid的service)
    inline void setWifiSearchMode(const int &mode){this->mSearchMode = mode;this->mSearchCount = 0;}
    //设置网络类型
    void setNetworkType(const int &type);
public:
    //连开所有wifi连接
    void DisconnectAllWifi();
    //
    QString getCurrentWifiName();
    //连接wifi（通过wifi的service）连接
    void setLinkWlan(const QString &service, const QString &password);
    //连接wifi(查找wifi名称的service来进行连接)
    inline void setLinkWlanSSID(const QString &SSID, const QString &password){this->mSSID = SSID; this->mSSID_Password = password;}
    //设置以太网口(IP类型、IP、子网掩码、网关、DNS)
    void setLinkLan(const int &type, const QString &ip, const QString &make, const QString &gateway, const QString &dns);
public:
    void resume();//运行线程
    void pause();//暂停线程
private:
    void openLan();
    void openWifi();
    void closeLan();
    void closeWifi();
private:
    void parseJson(const QByteArray);
private:
    void run();
private:
    int mWaitLinkCnt;
    int mSearchCount;
    int mSearchMode;
    QString mSSID;
    QString mSSID_Password;
private:
    mutable	QMutex sync;
    QWaitCondition pauseCond;
    volatile bool is_pause;
};
#endif
