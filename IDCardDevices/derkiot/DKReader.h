/*
 * Reader.h
 *
 *  Created on: 2020-9-7
 *      Author: derkiot
 */

#ifndef DKREADER_H_
#define DKREADER_H_
#include "stdint.h"
#define	 SAM_V_FRAME_START_CODE 0xBB
#define  SAM_V_INIT_COM 	0x32   	  /* 解析服务器开始请求解析命令 */
#define  SAM_V_APDU_COM 	0x33   	  /* 解析服务器APDU命令 */
#define  SAM_V_FINISH_COM 	0x34   	  /* 解析服务器解析完成命令 */
#define  SAM_V_ERROR_COM	0x35   	  /* 解析服务器解析错误命令 */
#define  SAM_V_GET_AES_KEY_COM 0x36 /* 获取明文解谜密钥 */

#define DK_ANALYSIS_EXTRA_LEN	3/*解析成功返回德科额外信息*/
#define DN_ANALYSIS_EXTRA_LEN	10 /*身份证信息帧头、应答等信号*/

typedef enum{
	ANALYSIS_CONTINUTE = 2,
	ANALYSIS_OK = 1,
	ANALYSIS_FIND_M1_CARD = 0,
	ANALYSIS_TIMEOUT = -1,
	ANALYSIS_FAIL = -2,
	ANALYSIS_NO_CARD = -3,
}analysis_status;

typedef void (*LCDFUN) (uint8_t rate);

int restart_read_card(void);
int close_beep(void);
int oepn_beep(uint8_t delay, uint8_t time);
int get_devices_version(uint8_t *version) ;
int open_uart_device(char *serialName);
int close_uart_device(void);
int get_idcard_data_with_block(LCDFUN lcd_dis, uint8_t *out_data, uint16_t *out_len);



//解码照片函数，wltBuffer原始照片1024字节wlt数据，bmpPath 保存照片位置
int saveWlt2Bmp( char* wltBuffer,const char* bmpPath);
int saveWlt2BmpUseFork( char* wltBuffer,const char* bmpPath);


#define DCTRI_RET_BASE          -0x6000
#define DCTRI_RET_OK            0
#define DCTRI_RET_ERR_PARA      -(DCTRI_RET_BASE + 1)
#define DCTRI_RET_ERR_RESP      -(DCTRI_RET_BASE + 2)
#define DCTRI_RET_ERR_COMM      -(DCTRI_RET_BASE + 3)
#define DCTRI_RET_ERR_DATA      -(DCTRI_RET_BASE + 4)
#define DCTRI_RET_ERR_OVERFLOW  -(DCTRI_RET_BASE + 5)
#define DCTRI_RET_ERR_NOCARD    -(DCTRI_RET_BASE + 6)
int continue_to_parse(uint8_t *initdata, uint16_t initdata_len, uint8_t *out_data, uint16_t *out_len) ;
void remote_get_img(void);
#endif /* READER_H_ */
