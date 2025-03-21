#include "WiegandObserver.h"
#include "Wiegand.h"
#include "MessageHandler/Log.h"
#include <QtCore/QDebug>

WiegandObserver::WiegandObserver(QObject *parent)
    : QThread(parent)
    , is_pause(false)
{
    this->start();
}

void WiegandObserver::run()
{
    unsigned char szBuf[128] = { 0 };
    while (!isInterruptionRequested())
    {
        this->sync.lock();
        if(this->is_pause) this->pauseCond.wait(&this->sync);

        int nReadFd = YNH_LJX::Wiegand::Wiegand_OpenInputWiegand();
        if (nReadFd > 0)
        {
            memset(szBuf, 0, sizeof(szBuf));
            int nReadSize = YNH_LJX::Wiegand::Wiegand_ReadInputWiegand(szBuf, sizeof(szBuf));
            LogD("%s %s[%d] nReadSize %d szBuf %s \n", __FILE__, __FUNCTION__, __LINE__,nReadSize, szBuf);
//            if (nReadSize == 10 || nReadSize == 8)
            {//传送到UI显示
            	printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
                emit sigReadIccardNum(QByteArray().append((char *)szBuf, nReadSize));
            }
        }
        this->pauseCond.wait(&this->sync, 100);
        this->sync.unlock();
    }
    YNH_LJX::Wiegand::Wiegand_CloseInputWiegand();
}
