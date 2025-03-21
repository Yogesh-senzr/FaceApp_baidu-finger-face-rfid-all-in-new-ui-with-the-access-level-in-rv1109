#ifndef _UDPBROADCASTTHREAD_H_
#define _UDPBROADCASTTHREAD_H_

#include <QtCore/QThread>
#include <QtCore/QWaitCondition>
#include <QtCore/QMutex>

class UdpBroadcastThread : public QThread
{
    Q_OBJECT
public:
	UdpBroadcastThread(QObject *parent = Q_NULLPTR);
    ~UdpBroadcastThread();
public:
    static inline UdpBroadcastThread *GetInstance(){static UdpBroadcastThread g;return &g;}
private:
    void run();
private:
    mutable QMutex sync;
    QWaitCondition pauseCond;
};

#endif // _UDPBROADCASTTHREAD_H_
