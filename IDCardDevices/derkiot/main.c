#include <stdio.h>
#include <stdlib.h>
#include "stdint.h"
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include "uart.h"
#include "Reader.h"
#include "utf.h"
#include "tcp_client.h"
#include "dk_utils.h"
#include "id_data_decode.h"

volatile static uint8_t  analysis_rate = 0;
char *serialName = "/dev/ttyUSB0";
//char *serialName = "/dev/ttyAMA3";
//char *serialName = "/dev/ttyS3";


//在显示屏上可以增加身份证解析进度,提醒用户不要马上拿开身份证
void schedule_printf(uint8_t rate)
{
	printf("\r\nanalysis rate : %%%d\r\n", rate );

}
//读身份证线程
int *read_card_thread(void) {

	int 		ret;
	uint8_t 	out_data[4096];
	uint16_t 	out_len;
	uint8_t  	id_info[256], id_bitmap[4096];
	uint16_t 	wordMsgBytesLen;
	uint16_t 	imageMsgBytesLen;
	uint16_t  	useful_message_index;
	uint8_t		bcc_check = 0;
	uint8_t		version;

	if(open_uart_device(serialName) == UART_INIT_FAIL) {
		printf("open %s fail\r\n", serialName);
		while (1);
	}

	if(get_devices_version(&version) == 1) {
		printf("softversion:%02x\r\n", version);
	}
#ifdef ME_ANT
	close_beep();
#endif
	
	while (1) {
		
		//阻塞等待，没有读到身份证超时为3秒
		ret = get_idcard_data_with_block(schedule_printf, out_data, &out_len);
		
		switch (ret){
			
			case  ANALYSIS_OK:
				
				bcc_check = calc_lrc(out_data+5, out_len-5-1);
				printf("身份证解析成功\r\n");

				if(out_data[0] == 0xAA && out_data[1] == 0xAA && out_data[2] == 0xAA && out_data[3] == 0x96 && 
					out_data[4] == 0x69 &&  bcc_check == out_data[out_len-1]){
					useful_message_index = DN_ANALYSIS_EXTRA_LEN;
					wordMsgBytesLen = ((out_data[useful_message_index] & 0xff) << 8) | (out_data[useful_message_index +1] & 0xff); 
					imageMsgBytesLen =   ((out_data[useful_message_index + 2] & 0xff) << 8) | (out_data[useful_message_index +3] & 0xff); 
					printf("wordMsgBytesLen:%d,imageMsgBytesLen:%d\r\n",wordMsgBytesLen,imageMsgBytesLen );	

					if(wordMsgBytesLen > 220) {
						memcpy(id_info, out_data + useful_message_index + 4, wordMsgBytesLen ); //拷贝文字信息
					}
					if(imageMsgBytesLen > 512) {
						memcpy(id_bitmap, out_data + useful_message_index + 4 + wordMsgBytesLen, imageMsgBytesLen ); //拷贝照片信息
					}
					system_time_printf();
					
					id_data_decode(id_info, wordMsgBytesLen, id_bitmap, imageMsgBytesLen);

					//restart_read_card(); //测试模式
						
					//到这一步才算解析完成，才能拿开身份证，此时打开蜂鸣器提示
					//open_beep();
#ifdef ME_ANT
					oepn_beep(0x0A, 1);//DK26ME_ANT打开蜂鸣器
#endif
					
				}
				else {
					printf("dn data bcc check fail\r\n");
				}
				
				break;
				
			case ANALYSIS_NO_CARD :
				printf("没有检测到身份证\r\n");
				break;

			case ANALYSIS_FIND_M1_CARD:
				//AA060101c4216639
				printf("card type:%d, UID:", out_data[2]);

				for(int i = 0; i  < out_len -3; i++) {
					printf("%02x", out_data[3+i]);
				}
				printf("\r\n");
				//dk_log_hex("UID:", out_data+3, out_len -3);
				break;
				
			default:
				printf("analysis fail\r\n");
				//restart_read_card();//测试模式
				break;
		}
	}
	
	close_uart_device();
	return 0;

}

int main(int argc, char *argv[]) {

	int 		ret;
	pthread_t 	id1;

	ret = pthread_create(&id1, NULL, (void *)read_card_thread, NULL);
	if(ret){
		printf("Create read_card_thread error!\n");
		return 0;
	}

	pthread_join(id1,NULL);

	return 0;
}



