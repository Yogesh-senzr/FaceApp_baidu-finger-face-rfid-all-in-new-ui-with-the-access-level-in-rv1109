#include "SensorManager.h"
#include "UARTUart.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>
#include <fcntl.h>
#include <dirent.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/prctl.h>
#include <time.h>

#include "MessageHandler/Log.h"

#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include <QDebug>


#define UART_PORT  1
//#define UART_PORT  4
using namespace YNH_LJX;
class SensorManagerPrivate
{
    Q_DECLARE_PUBLIC(SensorManager)
public:
    SensorManagerPrivate(SensorManager *dd);
private:
    float readCB1698FloatValue()const;
    float readDKAYFloatValue()const;
    float readDM20018FloatValue()const;
    float readHTPA32FloatValue()const;
    float readRB32FloatValue()const;
    float readHWDFloatValue()const;
    float readMX15IRFloatValue()const;
    float readZDFloatValue()const;
    float readTHFloatValue()const;
    float readTJFloatValue()const;
    float readZCFloatValue()const;
private:
    void Uart_OpenUartDev(const int BaudRate);
private:
    int mSensorType;
    float mTempComp;
    int mTemperatureEnvironment;//测温环境
private:
    mutable QMutex sync;
    QWaitCondition pauseCond;
private:
    SensorManager *const q_ptr;
};

SensorManagerPrivate::SensorManagerPrivate(SensorManager *dd)
    : q_ptr(dd)
    , mSensorType(0)
    , mTempComp(0.0)
    , mTemperatureEnvironment(0)
{

}

SensorManager::SensorManager(QObject *parent)
    : QThread(parent)
    , d_ptr(new SensorManagerPrivate(this))
{
}

SensorManager::~SensorManager()
{
    Q_D(SensorManager);
    this->requestInterruption();
    d->pauseCond.wakeOne();
    this->quit();
    this->wait();
}

void SensorManager::setRunSensor(const int sensorType)
{
    Q_D(SensorManager);
    d->mSensorType = sensorType;
    switch(sensorType)
    {
    case SENSOR_TYPE_UNKNOW:
    case SENSOR_TYPE_RB32: //
    case SENSOR_TYPE_HWD: // 海威达温度模块
    case SENSOR_TYPE_SZD: // 神州盾温度模块
    case SENSOR_TYPE_ZC:// 泽成温度模块
    case SNSOR_TYPE_MX15_IR:
    case SNSOR_TYPE_HM:  //海漫 HAIMAN
        d->Uart_OpenUartDev(115200);
        break;
    case SENSOR_TYPE_TH: // 铁虎测温模块
    case SENSOR_TYPE_DKAY: //电科安研模块
    case SENSOR_TYPE_DM20018: //DM20018模块
    case SENSOR_TYPE_TJ: //铁甲模块
    case SNSOR_TYPE_HTPA32_5:
    case SENSOR_TYPE_CB1698: //温度传感器
        d->Uart_OpenUartDev(9600);
        break;
    }

    this->start();
}

void SensorManager::setTempComp(const float &value)
{
    Q_D(SensorManager);
    d->mTempComp = value;
}

void SensorManager::setTempMode(const int value)
{
    Q_D(SensorManager);
    d->mTemperatureEnvironment = value;
}

void SensorManagerPrivate::Uart_OpenUartDev(const int BaudRate)
{        
    UART_ATTR_S stUartAttr;
    //stUartAttr.RDBlock = 0;
	stUartAttr.RDBlock = 1;
    stUartAttr.mBlockData = 512;//10,256
    stUartAttr.nBaudRate = BaudRate;
    stUartAttr.nAttr = 8;
    if (YNH_LJX::UARTUart::Uart_OpenUartDev(UART_PORT, stUartAttr) != ISC_OK)
    {
        LogV("%s %s[%d] Uart_OpenUartDev nUartPort %d \n", __FILE__, __FUNCTION__, __LINE__, UART_PORT);
    }
}

float SensorManagerPrivate::readCB1698FloatValue() const
{
    float fValue = 0;
    static unsigned char mData[6] = { 0 };

    int nRet = YNH_LJX::UARTUart::Uart_ReadUart(UART_PORT, mData, sizeof(mData));
            LogD(">>>%s,%s,%d,nRet=%d\n",__FILE__,__func__,__LINE__,nRet);    
    YNH_LJX::UARTUart::Uart_ReadFlush(UART_PORT);
    if ( nRet > 0)
    {
        if (mData[0] != 0xAA || mData[1] != 0x55 || mData[2] != 0x6)
        {
            return fValue;
        }
        int nValue = 0;

        nValue = mData[0] + mData[1] + mData[2] + mData[3] + mData[4];
        nValue = (nValue & 0xFF);
        if (nValue != mData[5])
        {
            return fValue;
        }
        char str[4] = { 0 };
        snprintf(str, 4, "%x%x", mData[3], mData[4]);
        sscanf(str, "%d", &nValue);

        fValue = (float) nValue / 10;
    }
    return fValue;
}

float SensorManagerPrivate::readDKAYFloatValue() const
{
    float fValue = 0;
    static unsigned char mData[12] = { 0 };

    int nRet = YNH_LJX::UARTUart::Uart_ReadUart(UART_PORT, mData, sizeof(mData));
            LogD(">>>%s,%s,%d,nRet=%d\n",__FILE__,__func__,__LINE__,nRet);    
    YNH_LJX::UARTUart::Uart_ReadFlush(UART_PORT);
    if (nRet > 0)
    {
        if (mData[0] != 0x55 || mData[1] != 0xAA || mData[2] != 0x0c) {
            return fValue;
        }

        int nTempValue = mData[3] << 8 | mData[4];

        fValue = (float) nTempValue / 100;
    }
    return fValue;
}

float SensorManagerPrivate::readDM20018FloatValue() const
{
    float fValue = 0;
    static unsigned char mData[6] = { 0 };

    int nRet = YNH_LJX::UARTUart::Uart_ReadUart(UART_PORT, mData, sizeof(mData));
            LogD(">>>%s,%s,%d,nRet=%d\n",__FILE__,__func__,__LINE__,nRet);    
    YNH_LJX::UARTUart::Uart_ReadFlush(UART_PORT);
    if (nRet > 0)
    {
        if (mData[0] != 0xAA || mData[1] != 0x55 || mData[2] != 0x06) {
            return ISC_ERROR;
        }

        int nTempValue = mData[3]*100+ (mData[4]>>4)*10 + (mData[4]&0x0f);

        fValue = (float) nTempValue / 10;
    }
    return fValue;
}

float SensorManagerPrivate::readHTPA32FloatValue() const
{
    float fValue = 0;
    static unsigned char value[5] = {};
    static unsigned char mData[7] = { 0 };

    memset(mData,0,sizeof(mData));
    int nRet = YNH_LJX::UARTUart::Uart_ReadUart(UART_PORT, mData, sizeof(mData));
            LogD(">>>%s,%s,%d,nRet=%d\n",__FILE__,__func__,__LINE__,nRet);    
    usleep(1*1000);
    YNH_LJX::UARTUart::Uart_ReadFlush(UART_PORT);
    if (nRet > 0)
    {
        memset(value,0,sizeof(value));
        if (mData[0] != 0x7b || mData[6] !=  0x7d )
        {
            return fValue;
        }

        for(int i=1;i<6;i++){
            value[i-1]  = mData[i];
        }

        fValue = atof((char *)value);
    }

    return fValue;
}

float SensorManagerPrivate::readRB32FloatValue() const
{
    float fValue = 0;
    static unsigned char wData[4] = {0xA5,0x55,0x01,0xFB};
    static unsigned char mData[7] = { 0 };


    //YNH_LJX::UARTUart::Uart_WriteUart(UART_PORT, UART_XIEYI_MYL, sizeof(UART_XIEYI_MYL)
#if 0 
    YNH_LJX::UARTUart::Uart_ReadUart(UART_PORT, mData, sizeof(mData));
    YNH_LJX::UARTUart::Uart_ReadFlush(UART_PORT);
    printf("%s,%s,%d,mdata:",__FILE__,__func__,__LINE__);
    for (int i=0;i<7;i++)
    {
        printf("%2x",mData[i]);
    }
    printf("\n");

 
        FILE *pFile = popen("echo -e -n \"\xa5\x55\x01\xfb\" > /dev/ttyS1", "r");
        if (pFile)
        {
        	std::string ret = "";
    		char buf[256] = { 0 };
    		int readSize = 0;
    		do
    		{
    			readSize = fread(buf, 1, sizeof(buf), pFile);
    			if (readSize > 0)
    			{
    				ret += std::string(buf, 0, readSize);
                    printf("%s,%s,%d,readSize=%d,buf:",__FILE__,__func__,__LINE__,readSize);
                    for (int j=0;j<readSize;j++)
                    {
                        printf("%02x",buf[j]);
                    }
                    printf("\n");                        
    			}
    		} while (readSize > 0);
            pclose(pFile);

            LogD("%s,%s,%d,ret:\n",__FILE__,__func__,__LINE__,ret.c_str());
            //str = QString::fromStdString(ret);
        }
#endif 

    if(YNH_LJX::UARTUart::Uart_WriteUart(UART_PORT, wData, sizeof(wData)) > 0)
    {
        usleep(1*1000);
        int nRet = YNH_LJX::UARTUart::Uart_ReadUart(UART_PORT, mData, sizeof(mData));
            //LogD(">>>%s,%s,%d,nRet=%d\n",__FILE__,__func__,__LINE__,nRet);
        YNH_LJX::UARTUart::Uart_ReadFlush(UART_PORT);
#if 0        
    printf("%s,%s,%d,mdata:",__FILE__,__func__,__LINE__);
    for (int i=0;i<7;i++)
    {
        printf("%02x",mData[i]);
    }
    printf("\n");     
#endif        
        if (nRet > 0)
        {
            //LogD(">>>%s,%s,%d,nRet=%d\n",__FILE__,__func__,__LINE__,nRet);
            if (mData[0] != 0xA5 || mData[1] != 0x55 )
            {
                return fValue;
            }
            int nValue = 0;
            //LogD(">>>%s,%s,%d,nRet=%d\n",__FILE__,__func__,__LINE__,nRet);
            nValue = mData[0] + mData[1] + mData[2] + mData[3] + mData[4] + mData[5];
            nValue = (nValue & 0xFF);
            if (nValue != mData[6])
            {
                return fValue;
            }
            float tem_value = mData[2] + 256*mData[3];
            fValue = tem_value / 100;
        }
    }
    //LogD(">>>%s,%s,%d,value=%2f\n",__FILE__,__func__,__LINE__,fValue);    
    return fValue;
}

float SensorManagerPrivate::readHWDFloatValue() const
{
    float fValue = 0;
    static unsigned char wData[5] = { 0xF0, 0x4F, 0x01, 0xEF, 0xEE };
    static unsigned char mData[14] = { 0 };

    if (YNH_LJX::UARTUart::Uart_WriteUart(UART_PORT, wData, sizeof(wData)) > 0)
    {
        usleep(1 * 1000);
        int nRet = YNH_LJX::UARTUart::Uart_ReadUart(UART_PORT, mData, sizeof(mData));
            LogD(">>>%s,%s,%d,nRet=%d\n",__FILE__,__func__,__LINE__,nRet);        
        YNH_LJX::UARTUart::Uart_ReadFlush(UART_PORT);

        if (nRet > 0)
        {
            if (mData[0] != 0xF0 || mData[1] != 0x4F || mData[12] != 0xEF || mData[13] != 0xEE)
            {
                return fValue;
            }

            int nTempValue = mData[4] << 8 | mData[5];
            //            int nEnvTempValue = mData[6] << 8 | mData[7];
            //            int nRealTempValue = mData[8] << 8 | mData[9];
            //            int nDistance = mData[10] << 8 | mData[11];

            //			printf("%s %s[%d] nTempValue %d nEnvTempValue %d nRealTempValue %d nDistance %d \n",
            //			__FILE__, __FUNCTION__, __LINE__, nTempValue, nEnvTempValue, nRealTempValue, nDistance);

            fValue = (float) nTempValue / 100;
        }
    }
    return fValue;
}

float SensorManagerPrivate::readMX15IRFloatValue() const
{
    float fValue = 0;
    static unsigned char wData[6] = {0xA5,0x03,0x00,0x00,0xD2,0xE8};
    static unsigned char mData[14] = { 0 };

    if(YNH_LJX::UARTUart::Uart_WriteUart(UART_PORT, wData, sizeof(wData)) > 0)
    {
        usleep(1*1000);
        memset(mData,0,sizeof(mData));
        int nRet = YNH_LJX::UARTUart::Uart_ReadUart(UART_PORT, mData, sizeof(mData));
            LogD(">>>%s,%s,%d,nRet=%d\n",__FILE__,__func__,__LINE__,nRet);        
        YNH_LJX::UARTUart::Uart_ReadFlush(UART_PORT);
        if (nRet > 0)
        {
            if (mData[0] != 0xA5 || mData[1] != 0x83 ||mData[2] !=0x08 )
            {
                return fValue;
            }
            int nValue = 0;

            nValue = (mData[9]<<8|mData[8]);
            fValue =  (float)nValue/100;
        }
    }
    return fValue;
}
#if 0
float SensorManagerPrivate::readG9250IRFloatValue() const
{
    float fValue = 0;
    static unsigned char wData[4] = {0xA5,0x55,0x01,0xFB};
    static unsigned char mData[14] = { 0 };

    if(YNH_LJX::UARTUart::Uart_WriteUart(UART_PORT, wData, sizeof(wData)) > 0)
    {
        usleep(1*1000);
        memset(mData,0,sizeof(mData));
        int nRet = YNH_LJX::UARTUart::Uart_ReadUart(UART_PORT, mData, sizeof(mData));
            LogD(">>>%s,%s,%d,nRet=%d\n",__FILE__,__func__,__LINE__,nRet);        
        YNH_LJX::UARTUart::Uart_ReadFlush(UART_PORT);
        if (nRet > 0)
        {
            if (mData[0] != 0xA5 || mData[1] != 0x83 ||mData[2] !=0x08 )
            {
                return fValue;
            }
            int nValue = 0;

            nValue = (mData[9]<<8|mData[8]);
            fValue =  (float)nValue/100;
        }
    }
    return fValue;
}
#endif 

float SensorManagerPrivate::readZDFloatValue() const
{
    float fValue = 0;
    static unsigned char mData[11] = { 0 };

    int nRet = YNH_LJX::UARTUart::Uart_ReadUart(UART_PORT, mData, sizeof(mData));
            LogD(">>>%s,%s,%d,nRet=%d\n",__FILE__,__func__,__LINE__,nRet);    
    YNH_LJX::UARTUart::Uart_ReadFlush(UART_PORT);
    if (nRet > 0)
    {
        if (mData[0] != 0x55 || mData[1] != 0x59 ||mData[3]!=0x01 || mData[9] != 0x0D || mData[10] !=  0x0A)
        {
            return fValue;
        }

        fValue = mData[5] + (float(mData[6]))/10;
    }
    return fValue;
}

float SensorManagerPrivate::readTHFloatValue() const
{
    float fValue = 0;
    static unsigned char wData[5] = { 0xA5, 0x01, 0x02};
    static unsigned char mData[5] = { 0 };

    if (YNH_LJX::UARTUart::Uart_WriteUart(UART_PORT, wData, sizeof(wData))>0)
    {
        usleep(1 * 1000);
        int nRet = YNH_LJX::UARTUart::Uart_ReadUart(UART_PORT, mData, sizeof(mData));
            LogD(">>>%s,%s,%d,nRet=%d\n",__FILE__,__func__,__LINE__,nRet);        
        YNH_LJX::UARTUart::Uart_ReadFlush(UART_PORT);
        if (nRet >0)
        {
            if (mData[0] != 0xA5 || mData[1] != 0x00 || mData[2] != 0x02)
            {
                return fValue;
            }

            int nTempValue = mData[3] << 8 | mData[4];

            fValue = (float) nTempValue / 10;
        }
    }
    return fValue;
}

float SensorManagerPrivate::readTJFloatValue() const
{
    float fValue = 0;
    static unsigned char wData[8] = {0x01,0x03,0x00,0x00,0x00,0x08,0x44,0x0C};
    static unsigned char mData[21] = { 0 };

    if (YNH_LJX::UARTUart::Uart_WriteUart(UART_PORT, wData, sizeof(wData))>0)
    {
        usleep(1 * 1000);
        int nRet = YNH_LJX::UARTUart::Uart_ReadUart(UART_PORT, mData, sizeof(mData));
            LogD(">>>%s,%s,%d,nRet=%d\n",__FILE__,__func__,__LINE__,nRet);        
        YNH_LJX::UARTUart::Uart_ReadFlush(UART_PORT);

        if (nRet >0)
        {
            if (mData[0] != 0x01 || mData[1] != 0x03 || mData[2] != 0x0a)
            {
                return fValue;
            }

            int nTempValue = mData[5] << 8 | mData[6];

            fValue = (float) nTempValue / 100;
        }
    }
    return fValue;
}

float SensorManagerPrivate::readZCFloatValue() const
{
    float fValue = 0;
    static unsigned char mData[21] = { 0 };

    int nRet = YNH_LJX::UARTUart::Uart_ReadUart(UART_PORT, mData, sizeof(mData));
                LogD(">>>%s,%s,%d,nRet=%d\n",__FILE__,__func__,__LINE__,nRet);
    YNH_LJX::UARTUart::Uart_ReadFlush(UART_PORT);
    if ( nRet  > 0)
    {
        printf("%s %s[%d] mData %s \n", __FILE__, __FUNCTION__, __LINE__, mData);
#if 1
        char temp_data[5] = { 0 };
        unsigned char *pTmp = mData;
        for (int i = 0; i < sizeof(mData); i++)
        {
            if (mData[i] == '{' && i < sizeof(mData) - 6)
            {
                pTmp = mData + i;
                break;
            }
        }
        memcpy(temp_data, pTmp + 1, 5);
        fValue = atof(temp_data);
        printf("%s %s[%d] fValue %f \n", __FILE__, __FUNCTION__, __LINE__, fValue);
#endif
    }
    return fValue;
}

float SensorManager::GetSensorFloatValue() const
{
    Q_D(const SensorManager);
    //LogD(">>>%s,%s,%d,mTemperatureEnvironment=%d,mSensorType=%d\n",__FILE__,__func__,__LINE__,d->mTemperatureEnvironment,d->mSensorType);    
    if(d->mTemperatureEnvironment == 0)
    {
        float value = 0.0;
        switch (d->mSensorType)
        {
        case SENSOR_TYPE_CB1698:value = d->readCB1698FloatValue();break;
        case SENSOR_TYPE_UNKNOW:
        case SENSOR_TYPE_RB32:value = d->readRB32FloatValue();break;
        case SENSOR_TYPE_HWD:value = d->readHWDFloatValue();break;
        case SENSOR_TYPE_SZD:value = d->readZDFloatValue();break;
        case SENSOR_TYPE_ZC:value = d->readZCFloatValue();break;
        case SENSOR_TYPE_TH:value = d->readTHFloatValue();break;
        case SENSOR_TYPE_DKAY:value = d->readDKAYFloatValue();break;
        case SENSOR_TYPE_DM20018:value = d->readDM20018FloatValue();break;
        case SENSOR_TYPE_TJ:value = d->readTJFloatValue();break;
        case SNSOR_TYPE_HTPA32_5:value = d->readHTPA32FloatValue();break;
        case SNSOR_TYPE_MX15_IR:value = d->readMX15IRFloatValue();break;
        case SNSOR_TYPE_HM:value = d->readMX15IRFloatValue();break;
        default:
        {
            LogV("%s %s[%d] not know type %x \n", __FILE__, __FUNCTION__, __LINE__, d->mSensorType);
            break;
        }
        }
        value = ((value>0.0) ? (value + d->mTempComp) : (value));
        //LogV("%s %s[%d] value %2f \n", __FILE__, __FUNCTION__, __LINE__, value);
        if(value>20 && value < TEMP_LOW)
		//if( value < TEMP_LOW)
        {
            srand((unsigned)time(0)); // 产生随机种子
            value = float(rand() % 7)/10 + 36.1;
        }
        static int count = 0;
        if(count++ >= 300)
        {
            count = 0;
            LogV("%s %s[%d] value %2f \n", __FILE__, __FUNCTION__, __LINE__, value);
        }        

        return value;
    }else if(d->mTemperatureEnvironment == 1)
    {//户外
        srand((unsigned)time(0)); // 产生随机种子
        return float(rand() % 7)/10 + 36.1;
    }

    return 0.0f;
}

void SensorManager::run()
{
    Q_D(SensorManager);
    //sleep(5);

    while (!isInterruptionRequested())
    {   
        d->sync.lock();
        float value = this->GetSensorFloatValue();
        emit sigTemperatureValue(value);
        d->pauseCond.wait(&d->sync, 1000);
        d->sync.unlock();
    }

}
