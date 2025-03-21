#ifndef UARTUart_H_
#define UARTUart_H_

#include "SharedInclude/GlobalDef.h"

namespace YNH_LJX{
class UARTUart
{
public:
    static int Uart_setUartParms(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity);
    static int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop,int nWaitMinByte,int nWaitTime);
    static int Uart_OpenUartDev(unsigned int index, UART_ATTR_S stUartAttr);
    static int Uart_WriteUart(unsigned int index, unsigned char *pData,unsigned int nDataSize);
    static int Uart_WriteUart_Delay(unsigned int index, unsigned char *pData,unsigned int nDataSize,int us);
    static void Uart_WriteFlush(unsigned int index);
    static int Uart_ReadUart(unsigned int index, unsigned char *pData,unsigned int nDataSize);
    static int Uart_ReadUart2(unsigned int index, unsigned char *pData,unsigned int nDataSize);    
    static void Uart_ReadFlush(unsigned int index);
    static int Uart_CloseUartDev(unsigned int nUartDevIndex);
};
}

#endif /* UARTUart_H_ */
