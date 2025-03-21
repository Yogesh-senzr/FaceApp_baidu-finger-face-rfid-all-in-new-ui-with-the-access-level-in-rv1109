#ifndef USBICCARDObserver_H_
#define USBICCARDObserver_H_

#include <QThread>
#include <QWaitCondition>
#include <QMutex>

class UsbICCardObserver:public QThread
{
    Q_OBJECT
public:
    UsbICCardObserver(QObject *parent = Q_NULLPTR);
    ~UsbICCardObserver();
public:
    static inline UsbICCardObserver *GetInstance(){static UsbICCardObserver g;return &g;}
public:
    Q_SIGNAL void sigReadIccardNum(const QString);
private:
    void run();
private:
    mutable QMutex sync;
    QWaitCondition pauseCond;
};

#endif /* FACE_APP_SRC_UsbICCardObserver_H_ */
