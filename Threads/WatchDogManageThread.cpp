#include "WatchDogManageThread.h"
#include "Config/ReadConfig.h"
#include "MessageHandler/Log.h"
#include "PCIcore/Watchdog.h"
#include "Helper/myhelper.h"
#include <QtCore/QDateTime>
#include<unistd.h>

WatchDogManageThread::WatchDogManageThread(QObject *parent)
    : QThread(parent)
{
	if (access("/udisk/debug_mode", F_OK) && access("/udisk/update.img", F_OK))
	{
		YNH_LJX::Watchdog::WatchDog_OpenWatchDog();
		YNH_LJX::Watchdog::WatchDog_FeedWatchDog(-1);
	}
    this->start();
}

WatchDogManageThread::~WatchDogManageThread()
{
    this->requestInterruption();
    this->pauseCond.wakeOne();

    this->quit();
    this->wait();
}

void WatchDogManageThread::run()
{
    while (!isInterruptionRequested())
    {
        this->sync.lock();
        //喂看门狗
        if (access("/udisk/debug_mode", F_OK) && access("/udisk/update.img", F_OK))
		{
        	YNH_LJX::Watchdog::WatchDog_FeedWatchDog(-1);
		}else
		{
			YNH_LJX::Watchdog::WatchDog_CloseWatchDog();
		}

        if(ReadConfig::GetInstance()->getMaintenance_boot() != 0)
        {
            QDateTime curDateTime = QDateTime::currentDateTime();
            QString strBootTime = ReadConfig::GetInstance()->getMaintenance_bootTimer();
            QString strBootDateTime = curDateTime.toString("yyyy/MM/dd")+" " + strBootTime;
            QDateTime bootDateTime = QDateTime::fromString(strBootDateTime,"yyyy/MM/dd hh:mm:ss");

            if(bootDateTime.secsTo(curDateTime) > 10 && bootDateTime.secsTo(curDateTime) < 20)
            {
            	LogD("%s %s[%d] time to reboot \n",__FILE__,__FUNCTION__,__LINE__);
            	myHelper::Utils_Reboot();
            }
        }

        this->pauseCond.wait(&this->sync, 3000);
        this->sync.unlock();
    }
}
