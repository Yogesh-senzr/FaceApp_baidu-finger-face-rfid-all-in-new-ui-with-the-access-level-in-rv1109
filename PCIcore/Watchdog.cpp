#include "Watchdog.h"

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
#include <linux/watchdog.h>
#include "MessageHandler/Log.h"

#define DEV_FILE "/dev/watchdog"

static int m_nWatchdogFd = 0;
static inline void Utils_ExecCmd(const char* szCmd)
{
    char buf[64] = { 0 };
    if (szCmd != NULL)
    {
        FILE *pFile = popen(szCmd, "r");
        if (pFile)
        {
            while (fgets(buf, sizeof(buf), pFile) != NULL)
            {
            }
            pclose(pFile);
        }
    }
}

int YNH_LJX::Watchdog::WatchDog_OpenWatchDog()
{
    Utils_ExecCmd("chmod 777 /dev/watchdog");
    if (m_nWatchdogFd <= 0)
    {
        unlink("/isc/reboot_flag");
        m_nWatchdogFd = open(DEV_FILE, O_RDWR);
    }
    return (m_nWatchdogFd > 0) ? 0 : -1;
}

void YNH_LJX::Watchdog::WatchDog_FeedWatchDog(unsigned int nTimeOutSec)
{
    if (m_nWatchdogFd > 0)
    {
        if (nTimeOutSec > 0)
        {
            ioctl(m_nWatchdogFd, WDIOC_SETTIMEOUT, &nTimeOutSec);
        }
        ioctl(m_nWatchdogFd, WDIOC_KEEPALIVE, 0);
    }
}

void YNH_LJX::Watchdog::WatchDog_CloseWatchDog()
{
    if (m_nWatchdogFd > 0)
    {
        int disable = WDIOS_DISABLECARD;
        ioctl(m_nWatchdogFd,WDIOC_SETOPTIONS,&disable);
        close(m_nWatchdogFd);
        m_nWatchdogFd = 0;
    }
}

