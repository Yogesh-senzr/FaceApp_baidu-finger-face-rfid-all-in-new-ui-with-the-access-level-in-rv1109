#include "Utils_Door.h"
#include "GPIO.h"
#include "Wiegand.h"

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <QDebug>

YNH_LJX::Utils_Door::Utils_Door(QObject *parent)
    : QThread(parent)
    , is_pause(true)
    , mOpenDoorWaitTime(5000)
{
    this->start();
}

void YNH_LJX::Utils_Door::run()
{
    while (!isInterruptionRequested())
    {
        this->sync.lock();
        if (this->is_pause)this->pauseCond.wait(&this->sync);
        YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Light_Green, 1);        
        YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Relay, 1);
        int nWriteFd = YNH_LJX::Wiegand::Wiegand_OpenOutputWiegand();
        if (nWriteFd > 0)
        {
            QByteArray icNum = this->mszICCardNum.toLatin1();
            YNH_LJX::Wiegand::Wiegand_WriteOutputWiegand((unsigned char*) icNum.data(), icNum.size());
            YNH_LJX::Wiegand::Wiegand_CloseOutputWiegand();
        }
        this->pauseCond.wait(&this->sync, this->mOpenDoorWaitTime);
        YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Relay, 0);
        YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Light_Green, 0);          
        this->is_pause = true;
        this->sync.unlock();
    }
}
