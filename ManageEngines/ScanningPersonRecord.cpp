#include "ScanningPersonRecord.h"

#include <QWaitCondition>
#include <QMutex>
#include <QMap>
#include <QDebug>

typedef struct
{
    QString name;
    int type;//类型
    int TimeoutSec;//超时时长
    int Tick;//时间
}PersonRecord_t;

class ScanningPersonRecordPrivate
{
    Q_DECLARE_PUBLIC(ScanningPersonRecord)
public:
    ScanningPersonRecordPrivate(ScanningPersonRecord *dd);
private:
    void DealWaitQueueData();
private:
    QMap<QString, PersonRecord_t>mPersonRecord;
private:
    mutable QMutex sync;
    QWaitCondition pauseCond;
private:
    ScanningPersonRecord *const q_ptr;
};

ScanningPersonRecordPrivate::ScanningPersonRecordPrivate(ScanningPersonRecord *dd)
    : q_ptr(dd)
{
}

ScanningPersonRecord::ScanningPersonRecord(QObject *parent)
    : QThread(parent)
    , d_ptr(new ScanningPersonRecordPrivate(this))
{
    this->start();
}

ScanningPersonRecord::~ScanningPersonRecord()
{

}

void ScanningPersonRecordPrivate::DealWaitQueueData()
{
    for (QMap<QString, PersonRecord_t>::iterator it = this->mPersonRecord.begin(); it != this->mPersonRecord.end();)
    {
        auto &key = it.key();
        auto &t = it.value();
        if (++t.Tick > t.TimeoutSec)
        {
            qWarning() << QString("【%1】【超时删除记录】：Fault_026").arg(key);
            it = this->mPersonRecord.erase(it);
        }
        else ++it;
    }
}

void ScanningPersonRecord::appPersonRecord(const QString &name, const int &type, const int &waitmsc)
{
    Q_D(ScanningPersonRecord);
    d->sync.lock();
    d->mPersonRecord.insert(name, PersonRecord_t{name, type, waitmsc, 0});
    d->sync.unlock();
}

bool ScanningPersonRecord::CheckQueueValid(const QString &name)
{
    Q_D(ScanningPersonRecord);
    QMutexLocker lock(&d->sync);
    return d->mPersonRecord.contains(name);
}

void ScanningPersonRecord::run()
{
    Q_D(ScanningPersonRecord);
    while (!isInterruptionRequested())
    {
        d->sync.lock();
        d->DealWaitQueueData();
        d->pauseCond.wait(&d->sync, 1000);
        d->sync.unlock();
    }
}
