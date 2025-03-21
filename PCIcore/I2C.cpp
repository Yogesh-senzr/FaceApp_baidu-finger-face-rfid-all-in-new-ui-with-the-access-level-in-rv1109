#include "I2C.h"
#include "i2c-dev.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <ctype.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <memory.h>

#define RAND_LEN_2016 8
#define RAND_LEN_MCU 6
#define DEV_ADDR_MCU	0x80
#define DOG_REG_MCU	0x01
#define ENC_REG_MCU 0x0
#define VERSION_REG_MCU 0x05
#define UNIT 32 //32ms
#define  TIMEOUT  60//60s

#define GPIO_I2C_READ   0x01
#define GPIO_I2C_SELCHN 0x05
#define GPIO_I2C_WRITE  0x03

void YNH_LJX::I2C::data_enc(const unsigned char * psrc, unsigned char * pdes)
{
    pdes[0] = (psrc[0] + psrc[3]) | (psrc[1] ^ psrc[2]);
    pdes[1] = (psrc[2] - psrc[0]) & (psrc[3] | psrc[1]);
    pdes[2] = (psrc[3] & psrc[1]) ^ (psrc[2] | psrc[0]);
    pdes[3] = (psrc[0] | psrc[1]) | (psrc[3] & psrc[2]);
    pdes[4] = pdes[1] ^ pdes[0];
    pdes[5] = pdes[2] | pdes[3];
}

int YNH_LJX::I2C::I2C_Open_EX()
{
    return open("/dev/gpioi2c-ex", O_RDWR, 0666);
}

int YNH_LJX::I2C::I2C_Write_EX(int nFd, unsigned int nSlaveAddr, unsigned int nDataAddr, int nSrcBuflen, unsigned char* szSrcBuf)
{
    int nRetval = 0;
    int nIndex = 0, nTmp;
    unsigned int nCurAddr;
    unsigned int nRegStep = 1;
    unsigned int nReg[3];

    nReg[0] = nSlaveAddr;
    nReg[1] = nDataAddr;

    for (nCurAddr = nDataAddr; nCurAddr < (nDataAddr + nSrcBuflen); nCurAddr += nRegStep)
    {
        nReg[2] = *(szSrcBuf++);
        printf("%s %s[%d] nSlaveAddr %x nDataAddr %x nData %x \n", __FILE__, __FUNCTION__, __LINE__, nReg[0], nReg[1], nReg[2]);
        ioctl(nFd, GPIO_I2C_WRITE, nReg);
        nReg[1] = nReg[1] + 1;
    }
    return 0;
}

int YNH_LJX::I2C::I2C_Read_EX(int nFd, unsigned int nSlaveAddr, unsigned int nDataAddr, int nDstBufLen, unsigned char* szDstBuf)
{

    int nRetval = 0;
    int nIndex = 0, nTmp;
    unsigned int nCurAddr;
    unsigned int nRegStep = 1;
    unsigned int nReg[3];

    nReg[0] = nSlaveAddr;
    nReg[1] = nDataAddr;

    for (nCurAddr = nDataAddr; nCurAddr < (nDataAddr + nDstBufLen); nCurAddr += nRegStep)
    {
        nReg[2] = ioctl(nFd, GPIO_I2C_READ, nReg);
        printf("%s %s[%d] nSlaveAddr %x nDataAddr %x nData %x \n", __FILE__, __FUNCTION__, __LINE__, nReg[0], nReg[1], nReg[2]);
        *(szDstBuf++) = nReg[2];
        nReg[1] = nReg[1] + 1;
    }
    return 0;

}

int YNH_LJX::I2C::I2C_OpenI2C(int nI2CNum)
{
    if (nI2CNum >= 0)
    {
        char szDevName[128] = { 0 };
        snprintf(szDevName, sizeof(szDevName), "/dev/i2c-%d", nI2CNum);
        return open(szDevName, O_RDWR, 0666);
    }
    return -1;
}

int YNH_LJX::I2C::I2C_CloseI2C(int nFd)
{
    if (nFd > 0)
    {
        return close(nFd);
    }
    return -1;
}

/*
 * buf[0] should be the target address
 */
int YNH_LJX::I2C::I2C_WriteI2C(int nFd, unsigned int nSlaveAddr, unsigned int nDataAddr, unsigned int nSrcBuflen, unsigned int* szSrcBuf)
{
    int nRetval = -1;
    int nIndex = 0, nTmp;
    unsigned char szTmpBuf[4];
    unsigned int nData = 0;
    unsigned int nCurAddr;
    unsigned int nRegWidth = 1, nDataWidth = 1;
    unsigned int nRegStep = 1;

    nRetval = ioctl(nFd, I2C_SLAVE_FORCE, nSlaveAddr);
    if (nRetval < 0)
    {
        printf("set i2c device address error!\n");
        nRetval = -1;
        goto end0;
    }

    for (nCurAddr = nDataAddr; nCurAddr < (nDataAddr + nSrcBuflen); nCurAddr += nRegStep)
    {
        nData = *(szSrcBuf++);
        if (nRegWidth == 2)
        {
            szTmpBuf[nIndex] = (nCurAddr >> 8) & 0xff;
            nIndex++;
            szTmpBuf[nIndex] = nCurAddr & 0xff;
            nIndex++;
        } else
        {
            szTmpBuf[nIndex] = nCurAddr & 0xff;
            nIndex++;
        }

        if (nDataWidth == 2)
        {
            szTmpBuf[nIndex] = (nData >> 8) & 0xff;
            nIndex++;
            szTmpBuf[nIndex] = nData & 0xff;
            nIndex++;
        } else
        {
            szTmpBuf[nIndex] = nData & 0xff;
            nIndex++;
        }

        nRetval = write(nFd, szTmpBuf, (nRegWidth + nDataWidth));
        if (nRetval < 0)
        {
            printf("i2c write error!\n");
            nRetval = -1;
            goto end0;
        }
    }

    nRetval = 0;

end0: return nRetval;
}

int YNH_LJX::I2C::I2C_ReadI2C(int nFd, unsigned int nSlaveAddr, unsigned int nDataAddr, int nDstBufLen, unsigned int* szDstBuf)
{
    int nRetval = -1;
    int nTmp;
    unsigned char szTmpBuf[4];
    unsigned int nRegWidth = 1, nDataWidth = 1, nRegStep = 1;
    unsigned int nCurAddr;
    static struct i2c_rdwr_ioctl_data stI2CData;
    static struct i2c_msg stI2CMsg[2];
    unsigned int nData;

    memset(szTmpBuf, 0x0, 4);

    nRetval = ioctl(nFd, I2C_SLAVE_FORCE, nSlaveAddr);
    if (nRetval < 0)
    {
        printf("CMD_SET_I2C_SLAVE error!\n");
        nRetval = -1;
        goto end0;
    }

    stI2CMsg[0].addr = nSlaveAddr;
    stI2CMsg[0].flags = 0;
    stI2CMsg[0].len = nRegWidth;
    stI2CMsg[0].buf = szTmpBuf;

    stI2CMsg[1].addr = nSlaveAddr;
    stI2CMsg[1].flags = 0;
    stI2CMsg[1].flags |= I2C_M_RD;
    stI2CMsg[1].len = nDataWidth;
    stI2CMsg[1].buf = szTmpBuf;

    stI2CData.msgs = &stI2CMsg[0];
    stI2CData.nmsgs = (__u32 ) 2;
    for (nCurAddr = nDataAddr; nCurAddr < (nDataAddr + nDstBufLen); nCurAddr += nRegStep)
    {
        if (nRegWidth == 2)
        {
            szTmpBuf[0] = (nCurAddr >> 8) & 0xff;
            szTmpBuf[1] = nCurAddr & 0xff;
        } else
            szTmpBuf[0] = nCurAddr & 0xff;

        nRetval = ioctl(nFd, I2C_RDWR, &stI2CData);
        if (nRetval != 2)
        {
            printf("CMD_I2C_READ error!\n");
            nRetval = -1;
            goto end0;
        }

        if (nDataWidth == 2)
        {
            nData = szTmpBuf[1] | (szTmpBuf[0] << 8);
        } else
            nData = szTmpBuf[0];

        *(szDstBuf++) = nData;
        printf("0x%x: 0x%x\n", nCurAddr, nData);
    }
    nRetval = 0;

end0: return nRetval;
}

int YNH_LJX::I2C::I2C_CheckAuthorized()
{
    system("chmod 777 /dev/mcu");
    int nMcuFd = open("/dev/mcu", O_RDWR);
#define RAND_LEN_MCU 6
    unsigned char srcData0[RAND_LEN_MCU] = { 0xE9, 0x13, 0xEB, 0xDD, 0x53, 0xF1 };
    unsigned char dstData0[RAND_LEN_MCU] = { 0xFE, 0x02, 0xFA, 0xFB, 0xFC, 0xFB };
    unsigned char tmpData[RAND_LEN_MCU] = { 0 };
    bool bCheckOk = true;
    if (nMcuFd > 0)
    {
        write(nMcuFd, srcData0, sizeof(srcData0));
        read(nMcuFd, tmpData, sizeof(tmpData));
        for (int j = 0; j < RAND_LEN_MCU; j++)
        {
            if (tmpData[j] != dstData0[j])
            {
                bCheckOk = false;
                break;
            }
        }
        close(nMcuFd);
    } else
    {
        bCheckOk = false;
    }
    return (bCheckOk == true) ? 0 : -1;
}
