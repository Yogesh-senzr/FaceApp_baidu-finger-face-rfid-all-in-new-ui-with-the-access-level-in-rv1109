#include "Wiegand.h"
#include "MessageHandler/Log.h"
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

#define WG_CMD	0xFB
#define WG_26_CMD		_IO(WG_CMD, 0x01)
#define WG_34_CMD		_IO(WG_CMD, 0x02)
int YNH_LJX::Wiegand::nWiegandReadFd = 0;
int YNH_LJX::Wiegand::nWiegandWriteFd = 0;
int YNH_LJX::Wiegand::nWiegandReverse = 0;

int YNH_LJX::Wiegand::Wiegand_OpenInputWiegand()
{
    if (nWiegandReadFd <= 0)
    {
        nWiegandReadFd = open("/dev/wiegand_input", O_RDONLY, 0666);
    }
    return nWiegandReadFd;
}

int YNH_LJX::Wiegand::Wiegand_ReadInputWiegand(unsigned char *pData, unsigned int nDataSize)
{
    int nReadSize = 0;
    if (nWiegandReadFd > 0 && pData != NULL && nDataSize > 0)
    {
        nReadSize = read(nWiegandReadFd, pData, nDataSize);
        if (nReadSize > 2)
        {
            if (nWiegandReverse)
            {
                unsigned int nNewICCardNum = 0;
                unsigned int nOldICCardNum = atoi((char*) pData);

                memset(pData, 0, nDataSize);
                snprintf((char*) pData, nDataSize, "%x", nOldICCardNum);
                LogD("%s %s[%d] WIEGAND_REVERSE nOldICCardNum %d %x \n", __FILE__, __FUNCTION__, __LINE__, nOldICCardNum, nOldICCardNum);
                for (int i = 0; i < strlen((char*) pData) / 2; i++)
                {
                    nNewICCardNum = (nNewICCardNum << 8);
                    nNewICCardNum |= (nOldICCardNum & 0xFF);
                    nOldICCardNum = (nOldICCardNum >> 8);
                }
                memset(pData, 0, nDataSize);
                snprintf((char*) pData, nDataSize, "%u", nNewICCardNum);
                LogD("%s %s[%d] WIEGAND_REVERSE nNewICCardNum %u %x \n", __FILE__, __FUNCTION__, __LINE__, nNewICCardNum, nNewICCardNum);
                LogD("%s %s[%d] WIEGAND_REVERSE pData %s \n", __FILE__, __FUNCTION__, __LINE__, pData);
            }
        }
    }
    return nReadSize;
}

void YNH_LJX::Wiegand::Wiegand_CloseInputWiegand()
{
    if (nWiegandReadFd > 0)
    {
        printf("%s %s[%d] \n", __FILE__, __FUNCTION__, __LINE__);
        unsigned int stop_flag = 0x10020;
        ioctl(nWiegandReadFd, stop_flag, 0);
        close(nWiegandReadFd);
        nWiegandReadFd = 0;
    }
}

int YNH_LJX::Wiegand::Wiegand_OpenOutputWiegand()
{
    if (nWiegandWriteFd <= 0)
    {
        nWiegandWriteFd = open("/dev/wiegand_output", O_RDONLY, 0666);
    }
    return nWiegandWriteFd;
}

void YNH_LJX::Wiegand::Wiegand_WriteOutputWiegand(unsigned char *pData, unsigned int nDataSize)
{
    if (nWiegandWriteFd > 0 && pData != NULL && nDataSize > 0)
    {
        unsigned char wg_output_buff[4] = { 0 };
        unsigned short wg_hid = 0;
        unsigned short wg_pid = 0;
        long long value = atoll((const char*) pData);
        //		sscanf((const char*)pData, "%lld", &value);
        wg_hid = (value >> 16) & 0xFFFF;
        wg_pid = value & 0xFFFF;
        memset(wg_output_buff, 0, sizeof(wg_output_buff));
        if (nDataSize > 8)
        {
            wg_output_buff[0] = wg_hid;
            wg_output_buff[1] = wg_hid >> 8;
            wg_output_buff[2] = wg_pid;
            wg_output_buff[3] = wg_pid >> 8;
            ioctl(nWiegandWriteFd, WG_34_CMD, wg_output_buff);
        } else
        {
            wg_output_buff[0] = wg_hid;
            wg_output_buff[1] = wg_pid;
            wg_output_buff[2] = wg_pid >> 8;
            ioctl(nWiegandWriteFd, WG_26_CMD, wg_output_buff);
        }
    }
}

void YNH_LJX::Wiegand::Wiegand_CloseOutputWiegand()
{
    if (nWiegandWriteFd > 0)
    {
        close(nWiegandWriteFd);
        nWiegandWriteFd = 0;
    }
}

void YNH_LJX::Wiegand::setWiegandReverse(const int state)
{
	LogD("%s %s[%d] state %d \n",__FILE__,__FUNCTION__,__LINE__,state);
    nWiegandReverse = state;
}
