#include "UARTUart.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <memory.h>

#include<iostream>
#include<errno.h>

#define UART_NUM_MAX 6
int fd[UART_NUM_MAX];

int YNH_LJX::UARTUart::Uart_setUartParms(int fd, int speed, int flow_ctrl, int databits,
                                         int stopbits, int parity) {
    struct termios options;
    int speed_arr[] = { B115200, B19200, B9600, B4800, B2400, B1200, B300 };
    int name_arr[] = { 115200, 19200, 9600, 4800, 2400, 1200, 300 };

    if (tcgetattr(fd, &options) != 0) {
        perror("SetupSerial 1");
        return -1;
    }
    //设置波特率
    for (int i = 0; i < sizeof(speed_arr) / sizeof(int); i++) {
        if (speed == name_arr[i]) {
            cfsetispeed(&options, speed_arr[i]);
            cfsetospeed(&options, speed_arr[i]);
        }
    }

    //set control model
    options.c_cflag |= CLOCAL;
    options.c_cflag |= CREAD;

    //set flow control
    switch (flow_ctrl) {

    case 0: //none
        options.c_cflag &= ~CRTSCTS;
        break;

    case 1: //use hard ware
        options.c_cflag |= CRTSCTS;
        break;
    case 2: //use sofware
        options.c_cflag |= IXON | IXOFF | IXANY;
        break;
    }

    //select data bit
    options.c_cflag &= ~CSIZE;
    switch (databits) {
    case 5:
        options.c_cflag |= CS5;
        break;
    case 6:
        options.c_cflag |= CS6;
        break;
    case 7:
        options.c_cflag |= CS7;
        break;
    case 8:
        options.c_cflag |= CS8;
        break;
    default:
        fprintf(stderr, "Unsupported data size\n");
        return -1;
    }
    //select parity bit
    switch (parity) {
    case 'n':
    case 'N':
        options.c_cflag &= ~PARENB;
        options.c_iflag &= ~INPCK;
        break;
    case 'o':
    case 'O':
        options.c_cflag |= (PARODD | PARENB);
        options.c_iflag |= INPCK;
        break;
    case 'e':
    case 'E':
        options.c_cflag |= PARENB;
        options.c_cflag &= ~PARODD;
        options.c_iflag |= INPCK;
        break;
    case 's':
    case 'S':
        options.c_cflag &= ~PARENB;
        options.c_cflag &= ~CSTOPB;
        break;
    default:
        fprintf(stderr, "Unsupported parity\n");
        return -1;
    }
    // set stopbit
    switch (stopbits) {
    case 1:
        options.c_cflag &= ~CSTOPB;
        break;
    case 2:
        options.c_cflag |= CSTOPB;
        break;
    default:
        fprintf(stderr, "Unsupported stop bits\n");
        return -1;
    }

    //set raw data output
    options.c_oflag &= ~OPOST;

    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    //options.c_lflag &= ~(ISIG | ICANON);

    //set wait time
    options.c_cc[VTIME] = 1;
    options.c_cc[VMIN] = 1;

    tcflush(fd, TCIFLUSH);

    //set the attribute to HiSerial device
    if (tcsetattr(fd, TCSANOW, &options) != 0) {
        printf("attr %d set error!\n", fd);
        return -1;
    }

    return 0;
}

int YNH_LJX::UARTUart::set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop,int nWaitMinByte,int nWaitTime)
{
    struct termios newtio,oldtio;
    if ( tcgetattr( fd,&oldtio) != 0) {
        printf("SetupSerial 1");
        return -1;
    }
    bzero( &newtio, sizeof( newtio ) );
    newtio.c_cflag |= CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;

    switch( nBits )
    {
    case 7:
        newtio.c_cflag |= CS7;
        break;
    case 8:
        newtio.c_cflag |= CS8;
        break;
    }

    switch( nEvent )
    {
    case 'O':
        newtio.c_cflag |= PARENB;
        newtio.c_cflag |= PARODD;
        newtio.c_iflag |= (INPCK | ISTRIP);
        break;
    case 'E':
        newtio.c_iflag |= (INPCK | ISTRIP);
        newtio.c_cflag |= PARENB;
        newtio.c_cflag &= ~PARODD;
        break;
    case 'N':
        newtio.c_cflag &= ~PARENB;
        break;
    }

    switch( nSpeed )
    {
    case 110:
        cfsetispeed(&newtio, B110);
        cfsetospeed(&newtio, B110);
        break;
    case 300:
        cfsetispeed(&newtio, B300);
        cfsetospeed(&newtio, B300);
        break;
    case 600:
        cfsetispeed(&newtio, B600);
        cfsetospeed(&newtio, B600);
        break;
    case 1200:
        cfsetispeed(&newtio, B1200);
        cfsetospeed(&newtio, B1200);
        break;
    case 2400:
        cfsetispeed(&newtio, B2400);
        cfsetospeed(&newtio, B2400);
        break;
    case 4800:
        cfsetispeed(&newtio, B4800);
        cfsetospeed(&newtio, B4800);
        break;
    case 9600:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    case 19200:
        cfsetispeed(&newtio, B19200);
        cfsetospeed(&newtio, B19200);
        break;
    case 38400:
        cfsetispeed(&newtio, B38400);
        cfsetospeed(&newtio, B38400);
        break;
    case 57600:
        cfsetispeed(&newtio, B57600);
        cfsetospeed(&newtio, B57600);
        break;
    case 115200:
        cfsetispeed(&newtio, B115200);
        cfsetospeed(&newtio, B115200);
        break;
    case 230400:
        cfsetispeed(&newtio, B230400);
        cfsetospeed(&newtio, B230400);
        break;
    default:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    }
    if( nStop == 1 )
        newtio.c_cflag &= ~CSTOPB;
    else if ( nStop == 2 )
        newtio.c_cflag |= CSTOPB;
    newtio.c_cc[VTIME] = nWaitTime;
    newtio.c_cc[VMIN] = nWaitMinByte;
    tcflush(fd,TCIFLUSH);
    if((tcsetattr(fd,TCSANOW,&newtio))!=0)
    {
        printf("com set error");
        return -1;
    }
    //printf("set done!\n");
    return 0;
}

int YNH_LJX::UARTUart::Uart_OpenUartDev(unsigned int index, UART_ATTR_S stUartAttr)
{
    if (index >= UART_NUM_MAX) {
        printf("serial %d not exist \n");
        return -1;
    }

    if (fd[index] <= 0) {
        char path[15];
        sprintf(path, "/dev/ttyS%d",index);
        fd[index] = open(path, O_RDWR|O_NOCTTY|O_NDELAY);
        if (fd[index] <= 0) {
            printf("open /dev/ttyS%d is failed ! \n", index);
            return -1;
        } else 
           printf("open /dev/ttyS%d sucess ! \n", index);

        if (stUartAttr.RDBlock == 0) {
            if (fcntl(fd[index], F_SETFL, FNDELAY) < 0) //非阻塞，覆盖前面open的属性
            {
                printf("fcntl failed\n");
            } else {
                printf("fcntl=%d\n", fcntl(fd[index], F_SETFL, FNDELAY));
            }
        } else {
            if (fcntl(fd[index], F_SETFL, 0) < 0)  //阻塞
                printf("fcntl failed!\n");
            else
                printf("fcntl=%d\n", fcntl(fd[index], F_SETFL, 0));
        }

        int res = -1; //= Uart_setUartParms(fd[index], stUartAttr.nBaudRate, 0, 8, 1,'N');
        if((res=set_opt(fd[index],stUartAttr.nBaudRate,8,'N',1,stUartAttr.mBlockData,stUartAttr.mBlockTime))<0){
            printf("set_opt error\n");
            return -1;
        }

    }
    return ISC_OK;
}

int YNH_LJX::UARTUart::Uart_WriteUart(unsigned int index, unsigned char *pData,unsigned int nDataSize) {
    if (index >= UART_NUM_MAX) {
        printf("serial %d not exist \n");
        return -1;
    }
    int len = 0;
    len = write(fd[index], pData, nDataSize);
    tcflush(fd[index], TCOFLUSH);
    return len;
}

int YNH_LJX::UARTUart::Uart_WriteUart_Delay(unsigned int index, unsigned char *pData,unsigned int nDataSize,int us)
{
    if (index >= UART_NUM_MAX) {
        printf("serial %d not exist \n");
        return -1;
    }
    int len = 0;
    len = write(fd[index], pData, nDataSize);
    usleep(us);
    return len;
}

void YNH_LJX::UARTUart::Uart_WriteFlush(unsigned int index)
{
    if (index >= UART_NUM_MAX) {
        printf("serial %d not exist \n");
        return;
    }
    tcflush(fd[index], TCOFLUSH);
}

int YNH_LJX::UARTUart::Uart_ReadUart(unsigned int index, unsigned char *pData,unsigned int nDataSize)
{
    if (index >= UART_NUM_MAX) {
        printf("serial %d not exist \n");
        return -1;
    }
    int len;
    len = read(fd[index], pData, nDataSize);
    //	tcflush(fd[index], TCIFLUSH);
    return len;
}


int SerialRecv(int fd,  char *rcv_buf, int data_len)
{
    static int count=0;
    int len, fs_sel;
    char rcv_data[1024];

    fd_set fs_read;
    struct timeval time;

    FD_ZERO(&fs_read);
    FD_SET(fd, &fs_read);

    time.tv_sec = 10;
    time.tv_usec = 0;

 //使用select实现串口的多路通信

    fs_sel = select(fd + 1, &fs_read, NULL, NULL, &time);
    printf(">>>>>%s,%s,%d\n", __FILE__,__func__,__LINE__);
    if (fs_sel)
    {
        len = read(fd, rcv_data, data_len);
        printf(">>>>>%s,%s,%d,rcv_data=%s\n", __FILE__,__func__,__LINE__,rcv_data);
    }
    else
    {
        return 0;
    }

    if(len==32)
    {
        strncpy(rcv_buf+count,rcv_data,32);
        printf(">>>>>%s,%s,%d,rcv_buf=%s\n", __FILE__,__func__,__LINE__,rcv_buf);
        count+=32;
    }

    if(len>0&&len<32)
    {
        strncpy(rcv_buf+count,rcv_data,len);
        count+=len;
        printf("count=%d\n",count);
        printf(">>>>>%s,%s,%d,rcv_buf=%s\n", __FILE__,__func__,__LINE__,rcv_buf);
        len=count;
        count=0;
        return len;
    }

    return len;

}

#if       1
int YNH_LJX::UARTUart::Uart_ReadUart2(unsigned int index, unsigned char *pData,unsigned int nDataSize)
{
    if (index >= UART_NUM_MAX) {
        printf("serial %d not exist \n");
        return -1;
    }
    int len = -1;
    int tlen =-1;
    memset(pData,0x0,sizeof(pData));
#if 1   
    while (1)
    {    
        len = read(fd[index], pData, nDataSize);
        usleep(30);//20
        pData+=len; 
        tlen +=len;
       printf(">>>%s,%s,%d,pData=%s\n",__FILE__,__func__,__LINE__,pData);        
        if (len<=0) break;
           
    }
    len =tlen;
#endif     
#if 0
char rcvdata[1024]={0};
char szData[1024]={0};


while (1) //循环读取数据
{
    len = SerialRecv(fd[index], rcvdata,  sizeof(rcvdata));
    if (len >0)
    {
        printf(">>>>>>%s,%s,%d, len: %d ,receive data : %s\n",__FILE__, __func__, __LINE__,len,rcvdata);
    }
    else
      break;

}
    len = strlen(rcvdata);
    memcpy(pData, rcvdata,sizeof(rcvdata));
    printf(">>>%s,%s,%d,pData=%s\n",__FILE__,__func__,__LINE__,pData);   
    sprintf((char *)pData,"%s",rcvdata);
    printf(">>>%s,%s,%d,pData=%s\n",__FILE__,__func__,__LINE__,pData);   
#endif     
    return len;
}
#endif 

void YNH_LJX::UARTUart::Uart_ReadFlush(unsigned int index)
{
    if (index >= UART_NUM_MAX) {
        printf("serial %d not exist \n");
        return;
    }
    tcflush(fd[index], TCIFLUSH);
}

int YNH_LJX::UARTUart::Uart_CloseUartDev(unsigned int nUartDevIndex) {
    if (nUartDevIndex >= UART_NUM_MAX) {
        printf("serial %d not exist \n");
        return -1;
    }

    if(fd[nUartDevIndex] > 0){
        close(fd[nUartDevIndex]);
        fd[nUartDevIndex] = -1;
        return ISC_OK;
    }
    return -1;
}
