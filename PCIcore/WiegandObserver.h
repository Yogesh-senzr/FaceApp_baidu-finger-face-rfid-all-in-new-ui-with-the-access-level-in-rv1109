#ifndef WIEGANDOBSERVER_H_
#define WIEGANDOBSERVER_H_

#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>

class WiegandObserver : public QThread
{
    Q_OBJECT
public:
    WiegandObserver(QObject *parent = Q_NULLPTR);
public:
    static inline WiegandObserver* GetInstance(){static WiegandObserver g;return &g;}
public:
    Q_SIGNAL void sigReadIccardNum(const QString);
private:
    void run();
private:
    mutable QMutex sync;
    QWaitCondition pauseCond;
    volatile bool is_pause;
};

#endif
