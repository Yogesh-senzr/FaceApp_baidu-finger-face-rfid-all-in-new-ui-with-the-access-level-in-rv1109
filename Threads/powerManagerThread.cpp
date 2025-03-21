#include "powerManagerThread.h"
#include "FaceMainFrm.h" 
#include "Application/FaceApp.h"
#ifdef Q_OS_LINUX
#include "PCIcore/GPIO.h"
#endif
#include <QThread>
#include <QTimer>

powerManagerThread::powerManagerThread(QObject *parent)
    : QObject(parent)
    , mFillLightMode(0)
    , m_pFillLightTimer(new QTimer)
{
    QThread *thread = new QThread;
    m_pFillLightTimer->moveToThread(thread);
    this->moveToThread(thread);


    connect(m_pFillLightTimer, &QTimer::timeout, this, &powerManagerThread::slotFillLightTimerOut);

    thread->start();
}

powerManagerThread::~powerManagerThread()
{
}

void powerManagerThread::setFillLightMode(const int &mode)
{
    this->mFillLightMode = mode;
#ifdef Q_OS_LINUX
    YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Light_Red, 0);
    YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Light_Green, 0);
    YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Light_White, mode ? 1 : 0);
#endif
}

void powerManagerThread::setIdentifyState(const bool state)
{
    Q_UNUSED(state);
#if 1
    if(this->mFillLightMode)
    {
#ifdef Q_OS_LINUX
        if(state)
        {
            YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Light_Red, 0);
            YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Light_Green, 1);
            YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Light_White, 0);
        }else
        {
            YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Light_Red, 1);//0
            YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Light_Green, 0);
            YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Light_White, 0);
        }
#endif
    }
#endif
}

void powerManagerThread::slotDisMissMessage(const bool state)
{
    Q_UNUSED(state);
    //printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
    switch(this->mFillLightMode)
    {
    case 0://关闭
    {//如果是关闭的管他有没有人都不启用
#ifdef Q_OS_LINUX
        YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Light_Red, 0);
        YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Light_Green, 0);
        YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Light_White, 0);
#endif
    }break;
    case 1://常开
    {
#ifdef Q_OS_LINUX
        if(!state)
        {//如果没有人时不理会， 一直让他常亮白光
            YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Light_Red, 0);
            YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Light_Green, 0);
            YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Light_White, 1);//
        }
#endif
    }break;
    case 2:
    {//
#ifdef Q_OS_LINUX
        if(!state && !m_pFillLightTimer->isActive())
        {//如果没有人时不理会， 一直让他常亮白光
            YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Light_Red, 0);
            YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Light_Green, 0);
            YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Light_White, 1);
        }else if(state && m_pFillLightTimer->isActive())
        {
            YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Light_Red, 0);
            YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Light_Green, 0);
            YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Light_White, 1);
        }
#endif
    }break;
    }

    if(state && m_pFillLightTimer->isActive())
    {//如果有人来的， 定时器启动说明背光关了让他开起来
#ifdef Q_OS_LINUX
        YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_LCD_BL, 1);
#endif
        m_pFillLightTimer->stop();
    }
    else if(!state && !m_pFillLightTimer->isActive())
        m_pFillLightTimer->start(30 *1000);
}

void powerManagerThread::slotFillLightTimerOut()
{
#ifdef Q_OS_LINUX
	if (qXLApp->GetFaceMainFrm()->getStatus() ==0)
	{
	    if(this->mFillLightMode == 2 )
	    {
	        YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Light_Red, 0);
	        YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Light_Green, 0);
	        YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Light_White, 0);
	    }
//	    YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_LCD_BL, 0);
	}
#endif
}
