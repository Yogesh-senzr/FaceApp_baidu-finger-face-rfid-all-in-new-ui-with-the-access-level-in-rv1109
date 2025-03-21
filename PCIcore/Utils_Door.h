#ifndef UTILS_DOOR_H
#define UTILS_DOOR_H

#include <QtCore/QThread>
#include <QtCore/QWaitCondition>
#include <QtCore/QMutex>

namespace YNH_LJX
{
class Utils_Door : public QThread
{
    Q_OBJECT
public:
    Utils_Door(QObject *parent = Q_NULLPTR);
public:
    static inline Utils_Door *GetInstance(){static Utils_Door g;return &g;}
public:
    inline void setOpenDoorWaitTime(const int msc){this->mOpenDoorWaitTime = msc *1000;}
    inline void OpenDoor(const QString &ICCardNum)
    {
        if(this->is_pause)
        {
            this->sync.lock();
            this->mszICCardNum = ICCardNum.isEmpty() ? "000000" : ICCardNum;
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
private:
    int mOpenDoorWaitTime;
    QString mszICCardNum;
};
}

#endif // UTILS_DOOR_H
