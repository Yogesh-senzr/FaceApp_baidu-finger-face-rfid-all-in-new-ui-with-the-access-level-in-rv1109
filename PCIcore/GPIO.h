#ifndef GPIO_H
#define GPIO_H

//////////外围设备开关控制，灯/继电器/////////////
/**
 *  外围设备
 */
typedef enum _DEVICE_EN
{
    DEVICE_Light_InfraredLed, //红外补光控制
    DEVICE_Light_White, //白光补光控制
    DEVICE_Light_Green, //绿色指示LED控制
    DEVICE_Light_Red, //红色指示LED控制
    DEVICE_USB1, // USB1口控制
    DEVICE_USB2, // USB2口控制
    DEVICE_5V, // 5V控制
    DEVICE_12V, //  12V控制
    DEVICE_Relay, //  继电器控制
    DEVICE_Relay2, //  继电器2控制
    DEVICE_LCD_BL, //  LCD背光控制
} DEVICE_EN;

namespace YNH_LJX {
class GPIO
{
public:
    static void Device_SetDeviceState(DEVICE_EN enDev, unsigned int state);
    static int Device_GetDeviceState(DEVICE_EN enDev);
private:
    static int GPIO_GetGPIOValue(const int &nGPIOGroup, const int &nGPIOIndex);
    static void GPIO_SetGPIOValue(const int &nGPIOGroup, const int &nGPIOIndex, const int &nValue);
private:
    static unsigned int nLightInfraredLedState;
    static unsigned int nLightWhiteLedState;
    static unsigned int nLightGreenLedState;
    static unsigned int nLightRedLedState;
    static unsigned int nRelayState;
    static unsigned int nRelay2State;
    static unsigned int n12VState;
    static unsigned int n5VState;
    static unsigned int nLcdbackligth;
};
}
#endif // GPIO_H
