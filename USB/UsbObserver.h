#ifndef USBOBSERVER_H_
#define USBOBSERVER_H_

#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include <QDebug>

#define MAX_USB_DEV_NUM 64
#define MAX_USB_DEV_NAME_LEN 256
#define USB_STORAGE_INTERFACE_CLASS (0x8)

typedef struct _UsbDevPath_
{
    char path[MAX_USB_DEV_NAME_LEN];
    unsigned int vid;
    unsigned int pid;
    unsigned int nInterfaceClass;
} UsbDevPath;

class UsbObserver : public QThread
{
    Q_OBJECT
public:
    UsbObserver(QObject *parent = Q_NULLPTR);
public:
    static inline UsbObserver* GetInstance(){static UsbObserver g;return &g;}
public:
    void addUsbDev(const char *path, unsigned int vid, unsigned int pid, unsigned int nInterfaceClass);
    void removeUsbDev(const char *path);
    bool isUsbStroagePlugin();
    void dousbStoragePlugIn();
    bool doCheckUpdateImage(QString path);
    void doDeviceUpdate();

private:
    void run();
public:
    Q_SIGNAL void sigUpDateTip(const QString);//发送升级提示
private:
    UsbDevPath mUsbDevPath[MAX_USB_DEV_NUM];
private:
    mutable QMutex sync;
    QWaitCondition pauseCond;
    volatile bool is_pause;
};

#endif /* USBOBSERVER_H_ */
