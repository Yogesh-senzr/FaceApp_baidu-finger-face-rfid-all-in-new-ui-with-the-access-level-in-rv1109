#include "GPIO.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
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
#include <sys/prctl.h>
#include <string.h>

unsigned int YNH_LJX::GPIO::nLightInfraredLedState = -1;
unsigned int YNH_LJX::GPIO::nLightWhiteLedState = -1;
unsigned int YNH_LJX::GPIO::nLightGreenLedState = -1;
unsigned int YNH_LJX::GPIO::nLightRedLedState = -1;
unsigned int YNH_LJX::GPIO::nRelayState = -1;
unsigned int YNH_LJX::GPIO::nRelay2State = -1;
unsigned int YNH_LJX::GPIO::n12VState = -1;
unsigned int YNH_LJX::GPIO::n5VState = -1;
unsigned int YNH_LJX::GPIO::nLcdbackligth = -1;

int YNH_LJX::GPIO::GPIO_GetGPIOValue(const int &nGPIOGroup, const int &nGPIOIndex)
{
    FILE *fp;
    char file_name[50];
    unsigned int gpio_num;
    unsigned char buf[10];

    gpio_num = nGPIOGroup * 32 + nGPIOIndex;

    sprintf(file_name, "/sys/class/gpio/export");
    fp = fopen(file_name, "w");
    if (fp == NULL)
    {
        printf("Cannot open %s.\n", file_name);
        return -1;
    }
    fprintf(fp, "%d", gpio_num);
    fclose(fp);

    sprintf(file_name, "/sys/class/gpio/gpio%d/direction", gpio_num);
    fp = fopen(file_name, "rb+");
    if (fp == NULL)
    {
        printf("Cannot open %s.\n", file_name);
        return -1;
    }
    fprintf(fp, "in");
    fclose(fp);

    sprintf(file_name, "/sys/class/gpio/gpio%d/value", gpio_num);
    fp = fopen(file_name, "rb+");
    if (fp == NULL)
    {
        printf("Cannot open %s.\n", file_name);
        return -1;
    }
    memset(buf, 0, 10);
    fread(buf, sizeof(char), sizeof(buf) - 1, fp);
    fclose(fp);

    sprintf(file_name, "/sys/class/gpio/unexport");
    fp = fopen(file_name, "w");
    if (fp == NULL)
    {
        printf("Cannot open %s.\n", file_name);
        return -1;
    }
    fprintf(fp, "%d", gpio_num);
    fclose(fp);

    return (int) (buf[0] - 48);
}

#if 0
int YNH_LJX::GPIO::GPIO_SetGPIODirection(const int &nGPIOGroup, const int &nGPIOIndex,const int out)
{
    FILE *fp;
    char file_name[50];
    unsigned int gpio_num;
    unsigned char buf[10];

    gpio_num = nGPIOGroup * 32 + nGPIOIndex;

    sprintf(file_name, "/sys/class/gpio/export");
    fp = fopen(file_name, "w");
    if (fp == NULL)
    {
        printf("Cannot open %s.\n", file_name);
        return -1;
    }
    fprintf(fp, "%d", gpio_num);
    fclose(fp);

    sprintf(file_name, "/sys/class/gpio/gpio%d/direction", gpio_num);
    fp = fopen(file_name, "rb+");
    if (fp == NULL)
    {
        printf("Cannot open %s.\n", file_name);
        return -1;
    }
    if(out == 0)
    {
        fprintf(fp, "in");
    }else
    {
        fprintf(fp, "out");
    }
    
    fclose(fp);

    sprintf(file_name, "/sys/class/gpio/unexport");
    fp = fopen(file_name, "w");
    if (fp == NULL)
    {
        printf("Cannot open %s.\n", file_name);
        return -1;
    }
    fprintf(fp, "%d", gpio_num);
    fclose(fp);

    return (int) (buf[0] - 48);
}

#endif 

void YNH_LJX::GPIO::GPIO_SetGPIOValue(const int &nGPIOGroup, const int &nGPIOIndex, const int &nValue)
{

    FILE *fp;
    char file_name[50];
    char buf[10];
    unsigned int gpio_num;
    gpio_num = nGPIOGroup * 32 + nGPIOIndex;

    sprintf(file_name, "/sys/class/gpio/export");
    fp = fopen(file_name, "w");
    if (fp == NULL)
    {
        printf("Cannot open %s.\n", file_name);
        return;
    }
    fprintf(fp, "%d", gpio_num);
    fclose(fp);

    sprintf(file_name, "/sys/class/gpio/gpio%d/direction", gpio_num);
    fp = fopen(file_name, "rb+");
    if (fp == NULL)
    {
        printf("Cannot open %s.\n", file_name);
        return;
    }
    fprintf(fp, "out");
    fclose(fp);

    sprintf(file_name, "/sys/class/gpio/gpio%d/value", gpio_num);
    fp = fopen(file_name, "rb+");
    if (fp == NULL)
    {
        printf("Cannot open %s.\n", file_name);
        return;
    }
    if (nValue)
        strcpy(buf, "1");
    else
        strcpy(buf, "0");

    fwrite(buf, sizeof(char), sizeof(buf) - 1, fp);
    //	printf("%s: gpio%d_%d = %s\n", __func__, nGPIOGroup, nGPIOIndex, buf);
    fclose(fp);

    sprintf(file_name, "/sys/class/gpio/unexport");
    fp = fopen(file_name, "w");
    if (fp == NULL)
    {
        printf("Cannot open %s.\n", file_name);
        return;
    }
    fprintf(fp, "%d", gpio_num);
    fclose(fp);
}

void YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_EN enDev, unsigned int state)
{
    switch (enDev)
    {
    case DEVICE_Light_InfraredLed:
    {
        if (nLightInfraredLedState != state)
        {
            if (state)
            {
                GPIO_SetGPIOValue(2, 24, 1); //GPIO2_D0
                GPIO_SetGPIOValue(2, 9, 1); //GPIO2_B1
            } else
            {
                GPIO_SetGPIOValue(2, 24, 0); //GPIO2_D0
                GPIO_SetGPIOValue(2, 9, 0); //GPIO2_B1
            }
            nLightInfraredLedState = state;
        }
        break;
    }
    case DEVICE_Light_White:
    {
        if (nLightWhiteLedState != state)
        {
            if (state)
            {
                if (nLightGreenLedState == 0 && nLightRedLedState == 0)
                {
                    GPIO_SetGPIOValue(2, 26, 1); //GPIO2_D2
                    nLightWhiteLedState = state;
                }
            } else
            {
                GPIO_SetGPIOValue(2, 26, 0); //GPIO2_D2
                nLightWhiteLedState = state;
            }

        }
        break;
    }
    case DEVICE_Light_Green:
    {
        if (nLightGreenLedState != state)
        {
            if (state)
            {
                GPIO_SetGPIOValue(2, 27, 1); //GPIO2_D3
            } else
            {
                GPIO_SetGPIOValue(2, 27, 0); //GPIO2_D3
            }
            nLightGreenLedState = state;
        }
        break;
    }
    case DEVICE_Light_Red:
    {
        if (nLightRedLedState != state)
        {
            if (state)
            {
                if (nLightGreenLedState == 0)
                {
                   // GPIO_SetGPIOValue(2, 25, 1); //GPIO2_D1
                    nLightRedLedState = state;
                }
            } else
            {
                //GPIO_SetGPIOValue(2, 25, 0); //GPIO2_D1
                nLightRedLedState = state;
            }
        }
        break;
    }
    case DEVICE_Relay:
    {
        if (nRelayState != state)
        {
            if (state)
            {
                GPIO_SetGPIOValue(2, 6, 1); //GPIO2_A6
            } else
            {
                GPIO_SetGPIOValue(2, 6, 0); //GPIO2_A6
            }
            nRelayState = state;
        }
        break;
    }
    case DEVICE_Relay2:
    {
        if (nRelay2State != state)
        {
            if (state)
            {
                GPIO_SetGPIOValue(2, 7, 1); //GPIO2_A7
            } else
            {
                GPIO_SetGPIOValue(2, 7, 0); //GPIO2_A7
            }
            nRelay2State = state;
        }
        break;
    }
    case DEVICE_12V:
    {

        if (n12VState != state)
        {
            if (state)
            {
                GPIO_SetGPIOValue(2, 8, 1); //GPIO2_B0
            } else
            {
                GPIO_SetGPIOValue(2, 8, 0); //GPIO2_B0
            }
            n12VState = state;
        }
        break;
    }
    case DEVICE_5V:
    {

        if (n5VState != state)
        {
            if (state)
            {
                GPIO_SetGPIOValue(2, 4, 1); //GPIO2_A4
            } else
            {
                GPIO_SetGPIOValue(2, 4, 0); //GPIO2_A4
            }
            n5VState = state;
        }
        break;
    }
    case DEVICE_LCD_BL:
    {
        if (nLcdbackligth != state)
        {
            char szValue[1] = { 0 };
            FILE *fp = NULL;
            fp = fopen("/sys/devices/platform/backlight/backlight/backlight/bl_power", "w");
            if (state)
            {
                if (fp)
                {
                    sprintf(szValue,"%d", 0);//0为开背光
                }
            } else
            {
                if (fp)
                {
                    sprintf(szValue,"%d", 1);//1为关背光
                }
            }
            if(fp)
            {
                fwrite(szValue, 1, 1, fp);
                fclose(fp);
            }
            nLcdbackligth = state;
        }
        break;
    }
    default:
        break;
    }
}

int YNH_LJX::GPIO::Device_GetDeviceState(DEVICE_EN enDev)
{
    switch (enDev)
    {
    case DEVICE_Light_InfraredLed:
    {
        return nLightInfraredLedState;
    }
    case DEVICE_Light_White:
    {
        return nLightWhiteLedState;
    }
    case DEVICE_Light_Green:
    {
        return nLightGreenLedState;
    }
    case DEVICE_Light_Red:
    {
        return nLightRedLedState;
    }
    case DEVICE_Relay:
    {
        return nRelayState;
    }
    case DEVICE_Relay2:
    {
        return nRelay2State;
    }
    case DEVICE_12V:
    {
        return n12VState;
    }
    case DEVICE_5V:
    {
        return n5VState;
    }
    case DEVICE_LCD_BL:
    {
        return nLcdbackligth;
    }
    default:
        break;
    }
    return 0;
}

