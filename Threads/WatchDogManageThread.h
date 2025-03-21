#ifndef WATCHDOGMANAGETHREAD_H
#define WATCHDOGMANAGETHREAD_H

#include <QThread>
#include <QWaitCondition>
#include <QMutex>

//看门狗线程
class WatchDogManageThread : public QThread
{
    Q_OBJECT
public:
    WatchDogManageThread(QObject *parent = Q_NULLPTR);
    ~WatchDogManageThread();
public:
    static inline WatchDogManageThread *GetInstance(){static WatchDogManageThread g;return &g;}
private:
    void run();
private:
    mutable QMutex sync;
    QWaitCondition pauseCond;
};

#endif // WATCHDOGMANAGETHREAD_H
