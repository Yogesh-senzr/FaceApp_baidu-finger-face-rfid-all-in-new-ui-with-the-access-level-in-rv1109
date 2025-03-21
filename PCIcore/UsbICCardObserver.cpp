#include "UsbIcCardObserver.h"

#include <string.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include "MessageHandler/Log.h"

#define USBICCARD "/dev/input/usbiccard0"

UsbICCardObserver::UsbICCardObserver(QObject *parent)
    : QThread(parent)
{
    this->start();
}

UsbICCardObserver::~UsbICCardObserver()
{
    this->requestInterruption();
    this->pauseCond.wakeOne();
    this->quit();
    this->wait();
}

void UsbICCardObserver::run()
{
    int fd = 0;
    int rc;
    struct input_event event;
    fd = open(USBICCARD, O_RDWR, 0);
    if(fd <=0){
        LogV("open %s failed !\n",USBICCARD);
        return;
    }
    char szBuf[128] = { 0 };
    bool once_read = false;

    while (!isInterruptionRequested())
    {
        this->sync.lock();
        while ((rc = read(fd, &event, sizeof(event))) > 0)
        {
            //LogV("Key %d (0x%x) %s", event.code & 0xff, event.code & 0xff,event.value ? "press" : "release");

            if (event.type == EV_KEY && event.value == 0)
            {
                switch (event.code) {
                case KEY_0:
                    strcat(szBuf, "0");
                    break;
                case KEY_1:
                    strcat(szBuf, "1");
                    break;
                case KEY_2:
                    strcat(szBuf, "2");
                    break;
                case KEY_3:
                    strcat(szBuf, "3");
                    break;
                case KEY_4:
                    strcat(szBuf, "4");
                    break;
                case KEY_5:
                    strcat(szBuf, "5");
                    break;
                case KEY_6:
                    strcat(szBuf, "6");
                    break;
                case KEY_7:
                    strcat(szBuf, "7");
                    break;
                case KEY_8:
                    strcat(szBuf, "8");
                    break;
                case KEY_9:
                    strcat(szBuf, "9");
                    break;
                case KEY_ENTER:
                    once_read = true;
                    break;
                }

                if(once_read)
                {
                    int nReadSize = strlen(szBuf);
                    once_read = false;
//                    if (nReadSize == 10 || nReadSize == 8)
                    {
                        printf("%s %s[%d] UsbICCardObserver %p \n",__FILE__,__FUNCTION__,__LINE__,this);
                    	printf("%s %s[%d] szBuf=%s \n",__FILE__,__FUNCTION__,__LINE__,szBuf);
                        emit sigReadIccardNum(QByteArray().append((char *)szBuf, nReadSize));
                    }
                    memset(szBuf,0,sizeof(szBuf));
                }
            }
        }
        this->pauseCond.wait(&this->sync, 200);
        this->sync.unlock();
    }
    close(fd);
}
