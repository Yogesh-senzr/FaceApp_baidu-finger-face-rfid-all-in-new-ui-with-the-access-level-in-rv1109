#include "Display.h"
#include <stdio.h>
int YNH_LJX::Display::Display_SetScreenBrightness(int value)
{
    FILE *fp = NULL;
    fp = fopen("/sys/devices/platform/backlight/backlight/backlight/brightness", "w");
    if (fp)
    {		
        float fValue = (float)value / 100;
        int nValue = 0;
        char szValue[6] = { 0 };
        nValue = 45+ fValue * (210); //255,亮度最小值为 45 
        snprintf(szValue, 6, "%d", nValue);
        fwrite(szValue, sizeof(char), sizeof(szValue) - 1, fp);
        fclose(fp);
        return true;
    }

    return false;
}
