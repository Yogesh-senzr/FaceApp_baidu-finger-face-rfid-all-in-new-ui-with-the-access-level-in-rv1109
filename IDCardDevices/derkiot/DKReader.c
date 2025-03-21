#include "DKReader.h"

#include<string.h>
#include <stdio.h>
#include "uart.h"
#include "stdint.h"
#include <stddef.h>
#include "dk_utils.h"
#include "tcp_client.h"
#include "uart.h"
#include "id_data_decode.h"
#include <time.h>
#include <sys/time.h>

//卡片重新上电命令
int restart_read_card(void) {
	uint8_t 	restart_cmd[] = {0xAA, 0x01, 0x18};

	return uart_send_data(restart_cmd, 3);
}

int close_beep(void) {
	uint8_t 	close_beep_cmd[] = {0xAA, 0x08, 0xA1, 0x08, 0x04, 0x14, 0x76, 0xFF, 0x00, 0x00};
	return uart_send_data(close_beep_cmd, sizeof(close_beep_cmd));
}




//蜂鸣器打开命令,delay:每次响时间，time:次数
int oepn_beep(uint8_t delay, uint8_t time) {
	uint8_t 	open_beep_cmd[] = {0xAA, 0x03, 0xB2,0x0A, 0x01};
	
	open_beep_cmd[3] = delay;
	open_beep_cmd[4] = time;
	return uart_send_data(open_beep_cmd, sizeof(open_beep_cmd));
}


//获取设备版本号命
int get_devices_version(uint8_t *version) {
	int ret;
	uint8_t out_data[] = {0xAA, 0x01, 0xB0};
	uint8_t data[64];
	
	ret = uart_send_data(out_data, sizeof(out_data));

	if((ret = wait_uart_data(3))) {
		
		if(uart_read_data_noblock(data, 4, 10000) == 4) {
			if(data[0] == 0xAA && data[1] == 0x02 && data[2] == 0xB0) {
				*version = data[3];
				return 1;
			}
		}
		if(data[0] == 0x43){
			return 0;
		}
	}
	return -1;
}


int continue_to_parse(uint8_t *initdata, uint16_t initdata_len, uint8_t *out_data, uint16_t *out_len) {

	uint8_t 	rcv_svr_data[4096];
	uint16_t 	rcv_svr_len;
	uint8_t 	send_uart_data[4096];
	int			status;

	if(out_data == NULL) {
		return ANALYSIS_FAIL;
	}

	status = dkCloudTcpDataExchange(initdata, initdata_len, rcv_svr_data, &rcv_svr_len);

	if ( (status == SVR_FAIL) || (rcv_svr_len < 2)
			|| ((rcv_svr_data[0] != 0x03) && (rcv_svr_data[0] != 0x04)) ) {

		if ( rcv_svr_len == 0 ) {
			printf("svr return data is null\n");
		}
		else if (rcv_svr_data[0] == 0x05) {
			printf("analysis timeout\r\n");
			return ANALYSIS_TIMEOUT;
			
		}
		else if (rcv_svr_data[0] == 0x06) {
			printf("设备未注册\r\n");
		}
		else if (rcv_svr_data[0] == 0x07) {
			printf("该设备已被禁用, 请联系www.derkiot.com");
		}
		else if (rcv_svr_data[0] == 0x08) {
			printf("该账号已被禁用, 请联系www.derkiot.com");
		}
		else if (rcv_svr_data[0] == 0x09) {
			printf("余额不足, 请联系www.derkiot.com充值");
		}
		else {
			printf("未知错误");
		}
		//TcpClientClose();
		return ANALYSIS_FAIL;
	}

	if ((status == SVR_OK) && (rcv_svr_len > 300) && (rcv_svr_data[0] == 0x04) ) {

		//TcpClientClose();
		memcpy(out_data, rcv_svr_data + DK_ANALYSIS_EXTRA_LEN, rcv_svr_len-DK_ANALYSIS_EXTRA_LEN);
		*out_len = rcv_svr_len-DK_ANALYSIS_EXTRA_LEN;
		return ANALYSIS_OK;
	}
	else if ( (rcv_svr_len != 0) && (rcv_svr_len < 300) && (rcv_svr_data[0] == 0x03) ) {

		//将数据发送给NFC模块
		//不包含服务器下发的前两个字节(长度)
		int cmdLen = rcv_svr_len + 1;
		send_uart_data[0] = SAM_V_FRAME_START_CODE;
		send_uart_data[1] = ((cmdLen & 0xff00) >> 8);
		send_uart_data[2] = (cmdLen & 0x00ff);
		send_uart_data[3] = SAM_V_APDU_COM;
		memcpy(send_uart_data+4, rcv_svr_data, rcv_svr_len);
		send_uart_data[4+rcv_svr_len] = calc_lrc(send_uart_data, 4+ rcv_svr_len);
		dk_log_hex("send to uart", send_uart_data, 5+ rcv_svr_len);
		if(uart_send_data(send_uart_data, 5+ rcv_svr_len) <= 0) {
			return ANALYSIS_FAIL;
		}
		return ANALYSIS_CONTINUTE;
	}
	return ANALYSIS_FAIL;
	
}

int open_uart_device(char *serialName){

	//uart init	
	if(uart_comm_init(serialName) == -1)  {
		printf("uart init fail\r\n");
		return UART_INIT_FAIL;
	}
	return UART_INIT_OK;
}

int close_uart_device(void){
	return uart_comm_deinit();
}


int get_idcard_data_with_block(LCDFUN lcd_dis , uint8_t *out_data, uint16_t *out_len) {

	int 		ret;
	uint8_t  	rcv_uart_data[4096];
	uint8_t 	initdata[4096];
	uint16_t 	initdatalen;
	static int 	err_cnt = 0;
	const int 	NUMBER_OF_REPARSING = 5;     /*解析失败时，重新解析的次数*/
	int 		rate = 0;
	uint8_t 	restart_cmd[] = {0xAA, 0x01, 0x18};
	uint8_t 	send_svr_first_pkg_data[256];
	uint16_t 	send_svr_first_pkg_len;

	while (1){
		//detect id card
		if(wait_uart_data(3) == -1) {
			return ANALYSIS_NO_CARD;
		}
		else {
			break;
		}
	}
	
	while (1) {
		ret = uart_comm_rcv_listener(rcv_uart_data);
		DEBUG_Printf("ret%d\n",ret);

		if (ret != 1) {
			TcpClientClose();
			if(ret == 0) {
				*out_len = 1 + rcv_uart_data[1];
				memcpy(out_data, rcv_uart_data+1,  *out_len); //除掉帧头AA
				return ANALYSIS_FIND_M1_CARD;
			}
			return ANALYSIS_NO_CARD;
		}

		if(rate == 0) {
			system_time_printf();
			printf("开始解析，请勿拿开身份证...\r\n");
		}
		//eg:bb 004c 32 b13130303030303031000051a040848aca3f206f3ecc7aa9836d9ce4b4a3414a4653cf78b21f85c3f59cd86f5e4c51f6e10953c2d3cf841b7f2fe92c564d882cd42111c4345231f13a8bf7 e8
#ifdef DK_DEBUG
		int rcv_uart_total_len = (uint16_t) (rcv_uart_data[1] << 8) + rcv_uart_data[2] + FRAME_OL_HEAD_LEN + FRAME_OL_LENS + FRAME_OL_LRC_LEN;  
		dk_log_hex("rcv_origin uart data",rcv_uart_data, rcv_uart_total_len);
#endif
		
		
		//*analysis_rate = 25 * rate;
		if(lcd_dis != NULL) {
			lcd_dis(25 * rate);
		}
	
		switch(rcv_uart_data[3]) {

			case SAM_V_INIT_COM:      //接收到开始解析请求

				printf("cloud id analysis start \n");
				if(TcpClientStart() == -1) {
					ret = ANALYSIS_FAIL;
					break;
				}
				system_time_printf();

				initdatalen =(uint16_t) (rcv_uart_data[1] << 8) + rcv_uart_data[2] -1; //去掉CMD
				initdata[0] = initdatalen&0xFF; //低8位
				initdata[1] = (initdatalen >> 8)&0xFF; //高8位
				memcpy(initdata + 2, rcv_uart_data + 4 , initdatalen );

				dk_log_hex("send to svr", initdata, initdatalen + 2);
				if(initdatalen + 2 < 256) {
					memcpy(send_svr_first_pkg_data, initdata, initdatalen + 2);
					send_svr_first_pkg_len = initdatalen + 2;
				}
				ret = continue_to_parse(initdata, initdatalen + 2, out_data, out_len);
				rate = 1;
				break;

			case SAM_V_APDU_COM:
				initdatalen = (rcv_uart_data[1] << 8) + rcv_uart_data[2]- 1;
				initdata[0] = initdatalen & 0xFF; //低8位
				initdata[1] = (initdatalen >> 8) & 0xFF; //高8位
				memcpy(initdata + 2, rcv_uart_data + 4, initdatalen );

				dk_log_hex("send to svr", initdata, initdatalen+2);
				ret = continue_to_parse(initdata, initdatalen+2, out_data, out_len);
				rate++;
				if(rate == 5) {
					rate = 1;
				}
				break;

			case SAM_V_ERROR_COM:
				printf("analysis errorCode:%02x%02x\r\n", rcv_uart_data[4],rcv_uart_data[5]);
				//TcpClientClose();
				if (err_cnt++ < NUMBER_OF_REPARSING) {
					printf("analysis restarting\n");
					if(TcpClientStart() == -1) {
						ret = ANALYSIS_FAIL;
						break;
					}
					uart_send_data(restart_cmd, 3);
					rate = 1;
					ret = ANALYSIS_CONTINUTE;

				} else {
					err_cnt = 0;
					rate = 0;
					//TcpClientClose();
					ret = ANALYSIS_FAIL;
				}
			break;

			default :
				ret = ANALYSIS_FAIL;
				break;
		}
		//服务器返回超时
		if(ret ==  ANALYSIS_TIMEOUT){
			//TcpClientClose();
			if(err_cnt++ < NUMBER_OF_REPARSING){
				if(TcpClientStart() == -1) {
					ret = ANALYSIS_FAIL;
					break;
				}
				printf("analysis restarting\n");
				rate = 1;
				ret = continue_to_parse(send_svr_first_pkg_data, send_svr_first_pkg_len, out_data, out_len);
				ret = ANALYSIS_CONTINUTE;
			}
			else {
				err_cnt = 0;
				rate = 1;
				ret = ANALYSIS_FAIL;
			}
		}
		
		else if(ret != ANALYSIS_CONTINUTE) {
			err_cnt = 0;
			rate = 0;
			//TcpClientClose();
			break;
		}
	}
	//uart_comm_deinit();
	return ret;
}





