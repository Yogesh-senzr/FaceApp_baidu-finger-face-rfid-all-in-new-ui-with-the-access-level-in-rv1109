/*
 * tcp_client.c
 *
 *  Created on: 2020.12.21
 *      Author: derkiot
 */
/************************************************************
*更改为只通过一个域名访问，域名已做负载均衡
*************************************************************/
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <setjmp.h>
#include <resolv.h>
#include <arpa/nameser.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <resolv.h>  
#include <sys/socket.h>
#include "dk_c_ares.h"
#include <assert.h>
#include <fcntl.h>
#include "tcp_client.h"
#include "dk_utils.h"
#include "netinet/tcp.h"

#define MAXLEN 4096

char 		test_ip[] = "47.113.79.7";
//char 		test_ip[] = "192.168.3.139";
char 		*url1 = "www.dkcloudid.cn";
char  		local_svr_ip[32] = {0};
const int 	port = 20006;
static volatile uint8_t tcp_conn_status = false;

int 		sockfd;
int			len;
char 		buffer[MAXLEN];
struct 		sockaddr_in servaddr;
const char  *src_ip;


bool tcp_is_connected(void)
{
    struct 	tcp_info info;
    int 	len;

	if(tcp_conn_status == false){
		return false;
	}
	
	len = sizeof(info);
 
    if (sockfd < 0){
		return false;
    }
 
    getsockopt(sockfd, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len);
    /*if (info.tcpi_state == TCP_ESTABLISHED) */
	
    if (info.tcpi_state == 1){  // Is connected
    	//printf("net online\r\n");
        return true;
    }
	printf("net offline\r\n");
  	return false;
    
}

//参数解释
//fd:网络连接描述符
//start:首次心跳侦测包发送之间的空闲时间 
 //interval:两次心跳侦测包之间的间隔时间
//count:探测次数，即将几次探测失败判定为TCP断开
int set_tcp_keepAlive(int fd, int start, int interval, int count) { 
    int keepAlive = 1; 
    if (fd < 0 || start < 0 || interval < 0 || count < 0) return -1;  //入口参数检查 ，编程的好习惯。
    //启用心跳机制，如果您想关闭，将keepAlive置零即可 
    if(setsockopt(fd,SOL_SOCKET,SO_KEEPALIVE,(void*)&keepAlive,sizeof(keepAlive)) == -1) 
    { 
        perror("setsockopt"); 
        return -1; 
    } 
    //启用心跳机制开始到首次心跳侦测包发送之间的空闲时间 
    if(setsockopt(fd,SOL_TCP,TCP_KEEPIDLE,(void *)&start,sizeof(start)) == -1) 
    { 
        perror("setsockopt"); 
        return -1; 
    } 
    //两次心跳侦测包之间的间隔时间 
    if(setsockopt(fd,SOL_TCP,TCP_KEEPINTVL,(void *)&interval,sizeof(interval)) == -1) 
    { 
        perror("setsockopt"); 
        return -1; 
    } 
    //探测次数，即将几次探测失败判定为TCP断开 
    if(setsockopt(fd,SOL_TCP,TCP_KEEPCNT,(void *)&count,sizeof(count)) == -1) 
    { 
        perror("setsockopt"); 
        return -1; 
    } 
    return 0; 
}

//检测TCP SOCKET是否保持连接
bool check_tcp_alive(void) {
	return tcp_is_connected();
}

int set_nonblock(int fd)
{
    int old_flag = fcntl(fd,F_GETFL);
    int new_flag = old_flag  | O_NONBLOCK;
    fcntl(fd,F_SETFL,new_flag);
    return old_flag;
}

int unblock_connect(const char *ip,int port,int outtime)
{
    struct sockaddr_in sockaddr;
    sockaddr.sin_family  = AF_INET;
    sockaddr.sin_port= htons(port);
    inet_pton(AF_INET,ip,&sockaddr.sin_addr);

    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    int oldflag = set_nonblock(sockfd);
	
    int ret = connect(sockfd,(struct sockaddr*)&sockaddr,sizeof(sockaddr));
    if(ret == 0)
    {
        /*如果连接成功，则恢复sockfd的属性，并立即返回*/
        printf("connect with server immediately\n");
        fcntl(sockfd,F_SETFL,oldflag);
        return sockfd;
    }
    else if(errno != EINPROGRESS)
    {
        perror("connnet error:");
        close(sockfd);
        return  -1;
    }

	if(set_tcp_keepAlive(sockfd , 2, 2, 2) == -1) {
		printf("set set_tcp_keepAlive fail\r\n");
	}

    struct timeval timeout;
    timeout.tv_sec = outtime;
    timeout.tv_usec = 0;

    fd_set writefd;
    FD_ZERO(&writefd);
    FD_SET(sockfd,&writefd);

    ret = select(sockfd + 1,NULL,&writefd,NULL,&timeout);
    if(ret == -1)
    {
        perror("select error:");
        close(sockfd);
        return -1;
    }
   
    if( ! FD_ISSET(sockfd,&writefd))
    {
        printf("no events on sockfd found\n");
        close(sockfd);
        return  -1;
    }

    int error =  0;
    socklen_t length = sizeof(error);
    /*调用getsockopt来获取并清除sockfd上的错误*/
    if(getsockopt(sockfd,SOL_SOCKET,SO_ERROR,&error,&length) < 0)
    {
        perror("getsockopt  error:");
        close(sockfd);
        return -1;
    }
   
    /*错误号不为0表示连接出错*/
    if(error != 0)
    {
        printf("connection failed after select with the error: %d\n",error);
        close(sockfd);
        return -1;
    }

    /*连接成功*/
    printf("connetion ready after select with the  socket: %d\n",sockfd);
    fcntl(sockfd,F_SETFL,oldflag);
    return sockfd;

}

//static int block_connect( const char* ip, int port)
//{
//  	uint8_t cnt = 0;
//	char svr_ip1[32];
//	int ret = -1;
//	
//	//creat socket
//	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
//		printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
//		return 0;
//	}
//
//	memset(&servaddr, 0, sizeof(servaddr));
//	servaddr.sin_family = AF_INET;
//	servaddr.sin_port = htons(port);
//
//
//	do{
//		if (inet_pton(AF_INET, ip, &servaddr.sin_addr) <= 0) {
//				printf("inet_pton1 error for %s\n", svr_ip1);
//		}
//		else {
//			if (connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0) {
//				printf("connect error: %s(errno: %d)\n", strerror(errno), errno);
//			}
//			else {
//				printf("connect:%s:%d\n", svr_ip1, port);
//				tcp_conn_status = true;
//			}
//		}
//	}
//	while(tcp_conn_status != true && (cnt++) < 2);
//	
//	if(tcp_conn_status == true) {
//		return 1;
//	}
//	return -1;
//}



int TcpClientStart(void) {
	
	char 	svr_ip1[32];
	int 	ret = -1;

	if(check_tcp_alive() == true) {
		return 1;
	}
	else {
		//必须要调用释放掉socket
		TcpClientClose();
	}

	 
	//必须要调用释放掉socket
	//pClientClose();
	
	//url to ip
	ret = dk_gethostbyname(url1, svr_ip1);
	if(ret == -1) {
		printf("dns analays fail\r\n");
	}
	else{
		printf("svr_ip1:%s\n", svr_ip1);
		if( ('0' <= svr_ip1[0] && '9' >= svr_ip1[0]) && ('0' <= svr_ip1[1] && '9' >= svr_ip1[1])){
			memcpy(local_svr_ip, svr_ip1, sizeof(svr_ip1));
		}
		else {
			ret = -1;
		}
	}
	tcp_conn_status = false;

	if(!('0' <= local_svr_ip[0] && '9' >= local_svr_ip[0]) ) {
		return -1;
	}
	
	//ret =  block_connect( (ret==-1)?local_svr_ip:svr_ip1, port);//
	sockfd = unblock_connect( (ret==-1)?local_svr_ip:svr_ip1, port, 5 );
	//sockfd = unblock_connect( test_ip, port, 5 );
	
	if(sockfd != -1) {
		tcp_conn_status = true;
		return 1;
	}
	return -1;
}

int TcpClientClose(void) {
	close(sockfd);
	tcp_conn_status = false;
	return 0;
}

int GetTcpClientStatus(void) {
	return tcp_conn_status;
}

// send packet to Server
int SendPacket(uint8_t *res, uint16_t len) {
	if(check_tcp_alive() == false) {
		TcpClientClose();
		return SVR_FAIL;
	}
	if (len != write(sockfd, res, len)) {
		printf("tcp write fail.\n");
		return SVR_FAIL;
	}
	//printf("tcp write ok.\n");
	return SVR_OK;
}

int ReadPacket(uint8_t *rcv_data, uint16_t *rcv_data_len) {

	uint8_t 	rcv_buffer[MAXLEN] = {0};
	int			size = 0;
	int			rest_size = 0;
	uint8_t 	wait_time = 0;
	uint16_t 	need_rcv_len = 0;
	uint16_t 	real_rcv_len = 0;

	int optlen = -1;    /* 整型的选项类型值 */
    int l_s32Ret = 0;

	if(check_tcp_alive() == false || rcv_data == NULL || rcv_data_len == NULL) {
		TcpClientClose();
		return SVR_FAIL;
	}

	struct timeval tv;
    tv.tv_sec = 2;    /* 1秒 */
    tv.tv_usec = 0;/* 200ms */
    optlen = sizeof(tv);
    l_s32Ret = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, optlen); /* 设置接收超时时间 */
    if(l_s32Ret == -1){/* 设置接收超时时间失败 */
        printf("set rcv timeout fail\r\n");            
    }
    
//    l_s32Ret = setsockopt(s32SocketFd, SOL_SOCKET, SO_SNDTIMEO, &tv, optlen);/* 设置发送超时时间 */
//    if(l_s32Ret == -1){
//        printf("设置发送超时时间失败\n");            
//    }
	
	while (1) {

		size = read(sockfd, rcv_buffer, sizeof(rcv_buffer));
		printf("size:%d\r\n", size);

		if(size <= 0) {
			tcp_conn_status = false;
		}
		
		if (size > 0) {
			*rcv_data_len = size;
			need_rcv_len =( (rcv_buffer[1] << 8) |rcv_buffer[0] ) + 2;

			//未接收完成
			if(size < need_rcv_len) {
				//拷贝第一次接收的
				memcpy(rcv_data, rcv_buffer, size);
				real_rcv_len  = size; 

				do {
					rest_size  = read(sockfd, rcv_buffer, sizeof(rcv_buffer) - real_rcv_len);
					if(rest_size > 0) {
						//拷贝剩下接收
						memcpy(rcv_data + real_rcv_len, rcv_buffer, rest_size);
						real_rcv_len = real_rcv_len + rest_size;
					}
					else {
						Sleep(1);
					}
					if(real_rcv_len  == need_rcv_len){
						*rcv_data_len = real_rcv_len;
						return SVR_OK;
					}
					else if(real_rcv_len  > need_rcv_len){
						*rcv_data_len = real_rcv_len;
						return SVR_FAIL;
					}
					
				}
				while(real_rcv_len != need_rcv_len && wait_time++ <= 200);
					
				if(real_rcv_len != need_rcv_len) {
					*rcv_data_len = real_rcv_len;
					return SVR_FAIL;
				}
				
			}
			else if(size == need_rcv_len) {
				memcpy(rcv_data, rcv_buffer, size);
				*rcv_data_len = size;
				return SVR_OK;
			}
			else {
				return SVR_FAIL;
			}
		
		} 
		else if (size == 0) {
			Sleep(10);
			if (wait_time++ >= 200) {
				return SVR_TIMEOUT;
			}
		}

		else {
			perror("tcp rcv fail");
			tcp_conn_status = false;
			TcpClientClose();
			*rcv_data_len = 0;
			return SVR_FAIL;
		}
	}
}

/*******************************************************************************
*函数名：  dkCloudTcpDataExchange
*功能:同服务器交互
*入口参数：initData - 要发送的数据
*        ：initData_len - 要发送的数据长度
		 ：rcv_data - 接收到服务器数据
		 ：rcv_data_len - 接收到服务器数据长度
*返回参数：true - 成功，false - 失败
*作者：    Frank
*******************************************************************************/
int dkCloudTcpDataExchange(uint8_t *initData, uint16_t initData_len, uint8_t *rcv_data, uint16_t *rcv_data_len) {
	int rcv_status = SVR_FAIL;
	uint8_t rcv_buffer[MAXLEN];
	uint16_t rcv_buffer_len = 0;
	
	if ((initData == NULL)) {
		return rcv_status;
	}

	if(SendPacket(initData, initData_len) == SVR_FAIL){
		return SVR_FAIL;
	}
	
	//等待接收数据，超时时间为2秒
	rcv_status = ReadPacket(rcv_buffer, &rcv_buffer_len);

	if(rcv_status ==  SVR_OK){
		dk_log_hex("rcv server data", rcv_buffer, rcv_buffer_len);
		*rcv_data_len = rcv_buffer_len - 2; 
		memcpy(rcv_data, rcv_buffer + 2, *rcv_data_len);
	}
	return rcv_status;
}


