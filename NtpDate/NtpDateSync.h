#ifndef NTPDATESYNC_H
#define NTPDATESYNC_H

#include <QThread>
#include <QWaitCondition>
#include <QMutex>

class NtpDateSync : public QThread
{
    Q_OBJECT
public:
    explicit NtpDateSync(QObject *parent = nullptr);
    ~NtpDateSync();
public:
    static inline NtpDateSync *GetInstance(){static NtpDateSync g;return &g;}
public:
    inline void setSyncNtpDate()
    {
        if(this->is_pause){
            this->sync.lock();
            this->is_pause = false;
            this->sync.unlock();
            this->pauseCond.wakeOne();
        }
    }
private:
    void run();
private:
    mutable QMutex sync;
    QWaitCondition pauseCond;
    volatile bool is_pause;
};

#endif // NTPDATESYNC_H
