/*
 * uart.c
 *
 *  Created on: 2020-9-5
 *      Author: derkiot
 */
#include <assert.h>
#include <errno.h>
#include "stdio.h"
#include <string.h>
#include <stdlib.h>
#include "stddef.h"
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <errno.h>
#include "uart.h"
#include "stdint.h"
#include "dk_utils.h"
//#define serialName "/dev/ttyUSB0"


volatile int fd;


enum UART_STATUS {
    UART_STATUS_IDLE = 0,
    UART_STATUS_READY
};

int uart_status = UART_STATUS_IDLE;

//串口设置
int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop)
{
	struct termios newtio,oldtio;
	if  ( tcgetattr( fd,&oldtio)  !=  0) {
		perror("SetupSerial");
		return -1;
	}
	bzero( &newtio, sizeof( newtio ) );
	newtio.c_cflag  |=  CLOCAL | CREAD;
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
		case 115200:
			cfsetispeed(&newtio, B115200);
			cfsetospeed(&newtio, B115200);
			break;
		case 460800:
			cfsetispeed(&newtio, B460800);
			cfsetospeed(&newtio, B460800);
			break;
		default:
			cfsetispeed(&newtio, B9600);
			cfsetospeed(&newtio, B9600);
			break;
	}
	if( nStop == 1 ){
		newtio.c_cflag &=  ~CSTOPB;
	}
	else if ( nStop == 2 ){
		newtio.c_cflag |=  CSTOPB;
	}
	newtio.c_cc[VTIME]  = 10;///* 设置超时1 seconds,unit:100ms*/
	newtio.c_cc[VMIN] = 0;
	tcflush(fd,TCIFLUSH);
	if((tcsetattr(fd,TCSANOW,&newtio))!=0)
	{
		perror("com set error");
		return -1;
	}

	printf("uart set success!\n\r");
	return 1;
}



//串口初始化
int uart_comm_init(char *serialName)
{
	if(serialName ==  NULL) {
		return -1;
	}
    if (uart_status == UART_STATUS_READY){
        return 1;
    }

	//无阻塞模式
    fd = open(serialName,O_RDWR|O_NONBLOCK|O_NDELAY);
    if (fd < 0) {
		return -1;
    }

    set_opt(fd, 115200, 8, 'N', 1);

    printf("Baud: 115200, WordLength: 8bit, StopBits:1bit, Parity: none, HardwareFlowControl: none\n");
    uart_status = UART_STATUS_READY;
    return 1;
}

//串口去初始化
int uart_comm_deinit(void)
{
    if (uart_status == UART_STATUS_IDLE){
        return 1;
    }
    close(fd);
    uart_status = UART_STATUS_IDLE;
    return 1;
}

/*******************************************************************************
*函数名：  uart_send_data
*功能：     串口发送数据
*入口参数：buf：要发送的数据
		   len：要发送的数据长度
*返回参数：成功发送的数据长度
*作者： Frank
*******************************************************************************/
int uart_send_data(uint8_t *buf, uint16_t len) {
	int ret;
	if(buf ==  NULL || len == 0) {

		return -1;
	}
	ret =  write(fd, buf, len);
	//printf("uart send ret:%d\n", ret);
	return ret;
}



/*******************************************************************************
*函数名：  wait_uart_data
*功能	：    等待是否存在串口数据
*入口参数：time_out：超时时�?

*返回参数：有数据 0：无数据
*作者： Frank
*******************************************************************************/
int wait_uart_data(uint8_t time_out) {
	
	int fs_sel;  
    fd_set fs_read;  
     
    struct timeval time;  
     
    FD_ZERO(&fs_read);  
    FD_SET(fd,&fs_read);  
     
    time.tv_sec = time_out;  
    time.tv_usec = 0;  
	
    //判断是否存在数据
    fs_sel = select(fd+1,&fs_read,NULL,NULL,&time);  
    DEBUG_Printf("fs_sel = %d\r\n",fs_sel);  
    if(fs_sel > 0)  //更正为负数的情况
    {  
		return 1;
    }
	return -1;
}


/*******************************************************************************
*函数名：  uart_read_data
*功能:   串口接收数据,阻塞读
*入口参数：buf：接收到的数据
		   len：要接收的数据
*返回参数：接收到的数据长度
*作者： Frank
*******************************************************************************/
int uart_read_data(uint8_t *buf, int len) {
	int 		ret;
	uint16_t 	i = 0;
	uint8_t 	rcv_buf;
	//uint16_t 	retry_cnt = 0;

	for(i = 0; i < len; i++) {
		//retry_cnt = 0;
		do{
			
			ret = read(fd,&rcv_buf, 1);
			//printf("ret:%d\n",ret);
		}
		while(ret <= 0 /*&& retry_cnt++ < 5000*/);
		buf[i] = rcv_buf;
	}
	if(i <= 0) {
		return -1;
	}
	return i;
}

int uart_read_data_noblock(uint8_t *buf, int len, uint16_t time_out) {
	int 		ret;
	uint16_t 	j = 0;
	uint8_t 	rcv_buf;
	uint16_t	try_cnt = 0;

	for(j = 0; j < len; j++) {
		try_cnt = 0;
		do{
			ret = read(fd,&rcv_buf, 1);
			//printf("ret:%d\n",ret);
		}
		while(ret <= 0 && (try_cnt++ <time_out));

		if(ret <= 0) {
			break;
		}
		buf[j] = rcv_buf;
		
	}
	if(j <= 0) {
		return -1;
	}
	return j;
}


/*******************************************************************************
*函数名：  uart_comm_rcv_listener
*功能：   串口数据状态机接收
*入口参数：rcv_data：接收到的数据，eg:BB 004C b1313030303030303100004a37c315b8fdd703d6787a1048b760dbdefb1c2dd91b719a613180fc6860c63cf30bdb38e6e9be3d5157d6aad6f2299f23ec85e26bdc293527acf61a094a5943
		   len：要接收的数据长度
*返回参数：接收到的数据长度
*作者： Frank
*******************************************************************************/
int uart_comm_rcv_listener(uint8_t *rcv_data) {
	uint8_t buf[4096];
	int len = -1;
	int rest_len = 0;
	int ret;
	uint8_t lrc;
	
	ret = uart_read_data(buf, FRAME_OL_HEAD_LEN);
	DEBUG_Printf("buf[0]:%x\n", buf[0]);
	
	if (ret < 0) {
		return -1;
	}
	if (ret != FRAME_OL_HEAD_LEN) {
		return -2;
	}

	//不是0xBB
	if (memcmp(buf, FRAME_OL_HEAD, FRAME_OL_HEAD_LEN)) {
		
		//0xAA, card moved
		if (memcmp(buf, FRAME_HEAD, FRAME_OL_HEAD_LEN) == 0) {
			ret = uart_read_data(buf, 1); //长度字节
		   	if (ret != 1) {
			   return -2;
		   	}
			
			len = buf[0];
			ret = uart_read_data( &buf[1], len);
	    	if (ret != len) {
	       	 	return -3;
	   	 	}

			dk_log_hex("AA", buf, len+1);
			
			if(len == 0x01 && buf[1] == 0xEA) {
                //printf("card move\r\n");
				return -4;
			}
			else if(len > 1 && len < 50) {
				rcv_data[0] = 0xAA;
				memcpy(rcv_data + 1, buf, len+1);
				//dk_log_hex("buf", buf, rcv_buffer_len);
				return 0;
			}
			else {
				return -5;
			}
						
		}
		return -5;
	}

	//获取长度字节
	ret = uart_read_data(buf+FRAME_OL_HEAD_LEN, FRAME_OL_LENS);
	if (ret != FRAME_OL_LENS) {
		return -4;
	}

	//计算剩下的长度
	rest_len = (buf[1] << 8) + buf[2]; 
	if (rest_len  > sizeof(buf) - FRAME_OF_LENS - FRAME_OL_LRC_LEN){
		return -5;
	}
	//读取剩下的数据
	ret = uart_read_data(buf+FRAME_OL_HEAD_LEN+FRAME_OL_LENS, rest_len);
	if (ret != rest_len) {
		return -6;
	}
	
	//读取异或校验
	ret = uart_read_data(buf+FRAME_OL_HEAD_LEN+FRAME_OL_LENS+rest_len, FRAME_OL_LRC_LEN);
	
	if (ret != FRAME_OL_LRC_LEN) {
		return -7;
	}
	//比较异或校验
	lrc = calc_lrc(buf, FRAME_OL_HEAD_LEN+FRAME_OL_LENS+rest_len);
	if(*(buf+FRAME_OL_HEAD_LEN+FRAME_OL_LENS+rest_len) != lrc){
		printf("--- lrc: 0x%02x\r\n", lrc);
		printf("--- lrc_real: 0x%02x\r\n", *(buf+FRAME_OL_HEAD_LEN+FRAME_OL_LENS+rest_len));
		return -8;
	}
	memcpy(rcv_data, buf, FRAME_OL_HEAD_LEN+FRAME_OL_LENS+rest_len+FRAME_OL_LRC_LEN);
	return 1;

}

