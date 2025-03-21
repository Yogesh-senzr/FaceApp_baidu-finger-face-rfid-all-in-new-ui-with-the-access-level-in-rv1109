#ifndef MINGRUAN_COMMON_FACEAPP_RKNETWORK_NETWORK4GCONTROLTHREAD_H_
#define MINGRUAN_COMMON_FACEAPP_RKNETWORK_NETWORK4GCONTROLTHREAD_H_

#include <QtCore/QThread>
#include <QtCore/QWaitCondition>
#include <QtCore/QMutex>

class Network4GControlThread : public QThread
{
    Q_OBJECT
public:
	Network4GControlThread(QObject *parent = Q_NULLPTR);
    ~Network4GControlThread();
public:
    static inline Network4GControlThread *GetInstance(){static Network4GControlThread g;return &g;}

private:
    void run();

private:
    mutable	QMutex sync;
    QWaitCondition pauseCond;
};



#endif /* MINGRUAN_COMMON_FACEAPP_RKNETWORK_NETWORK4GCONTROLTHREAD_H_ */
