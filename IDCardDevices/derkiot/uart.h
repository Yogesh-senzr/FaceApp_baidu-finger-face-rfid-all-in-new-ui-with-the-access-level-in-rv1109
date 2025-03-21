/*
 * uart.h
 *
 *  Created on: 2020-9-5
 *      Author: derkiot
 */

#ifndef UART_H_
#define UART_H_
#include "stdint.h"

/*
"115200,8,N,1"
*/
#define FRAME_HEAD              "\xAA"
#define FRAME_HEAD_LEN          1
#define FRAME_CMD_LEN           1
#define FRAME_LENS              1
#define FRAME_ADDR_LEN          1
#define FRAME_DATA

#define ERR_TAG_TYPE            "\xAA\x01\xE0"
#define ERR_NO_FIND_TAG         "\xAA\x01\xE1"
#define ERR_KEY_NO_AUTH         "\xAA\x01\xE2"
#define ERR_READ_BLOCK          "\xAA\x01\xE3"
#define ERR_WRITE_BLOCK         "\xAA\x01\xE4"
#define ERR_VALUE_INIT          "\xAA\x01\xE5"
#define ERR_VALUE_ADD           "\xAA\x01\xE6"
#define ERR_VALUE_SUB           "\xAA\x01\xE7"


#define FRAME_OL_HEAD              "\xBB"
#define FRAME_OL_HEAD_LEN           1
#define FRAME_OL_CMD_LEN            1
#define FRAME_OL_LENS               2
#define FRAME_OL_ADDR_LEN           1
#define FRAME_OL_DATA
#define FRAME_OL_LRC_LEN            1



#define FRAME_OF_HEAD               "\xAA\xAA\xAA\x96\x69"
#define FRAME_OF_HEAD_LEN           5
#define FRAME_OF_CMD_LEN            2
#define FRAME_OF_LENS               2
#define FRAME_OF_ADDR_LEN           1
#define FRAME_OF_RESP_LEN           3
#define FRAME_OF_DATA
#define FRAME_OF_LRC_LEN            1

/* GENERIC OPS */
#define CMD_GET_CARD_UID        0x01
#define CMD_GET_CARD_TYPE       0x02
#define CMD_SET_DETECT          0x95
#define CMD_SET_BAUD            0xA0
#define CMD_SET_SYS_PARA        0xA1
#define CMD_GET_SYS_PARA        0xA2
#define CMD_GET_SW_VER          0xB0
#define CMD_GET_HW_VER          0xB1

/* MIFARE OPS */
#define CMD_M1_WRITE_A_KEY      0x03    
#define CMD_M1_READ_BLK         0x04
#define CMD_M1_WRITE_BLK        0x05
#define CMD_M1_OPER_INIT        0x06
#define CMD_M1_OPER_ADD         0x07
#define CMD_M1_OPER_MINUS       0x08
#define CMD_M1_WRITE_B_KEY      0x0B
#define CMD_M1_SET_KEY_TYPE     0x0C

/* ULTRALIGHT OPS */
#define CMD_UL_READ_BLK         0x09    
#define CMD_UL_READ_MBLK        0x1C
#define CMD_UL_WRITE_MBLK       0x1D

/* ISO14443 CPU CARD OPS */
#define CMD_ICC_ACTIVE          0x15
#define CMD_ICC_APDU_EXCHANGE   0x17            
#define CMD_ICC_DEACTIVE        0x18

/* ID CARD OPS */
#define CMD_IDC_ACTIVE          0x14
#define CMD_IDC_APDU_EXCHANGE   0x16
#define CMD_IDC_DEACTIVE        0x18

/* ISO15693 CARD OPS */
#define CMD_ISO_READ_BLK        0x90
#define CMD_ISO_READ_MBLK       0x91
#define CMD_ISO_WRITE_BLK       0x92
#define CMD_ISO_WIRTE_MBLK      0x93
#define CMD_ISO_LOCK_CLK        0x94

/* ERROR RESPONSE */
#define CMD_ERR_CARD_TYPE       0xE0
#define CMD_ERR_NOT_DETECT      0xE1
#define CMD_ERR_UNMATCH_KEY     0xE2
#define CMD_ERR_READ_BLK        0xE3
#define CMD_ERR_WRITE_BLK       0xE4
#define CMD_ERR_M1_INIT         0xE5
#define CMD_ERR_M1_ADD          0xE6
#define CMD_ERR_M1_MINUS        0xE7
#define CMD_ACK                 0xFE
#define CMD_NACK                0xFF

#define CMD_IDC_DETECT          "\x20\x01"
#define CMD_IDC_SELECT          "\x20\x02"
#define CMD_IDC_READ_PLAIN      "\x30\x01"

#define CMD_IDC_RESP_EXEC_OK        "\x00\x00\x90"
#define CMD_IDC_RESP_DETECT_OK      "\x00\x00\x9F"
#define CMD_IDC_RESP_ERR_LRC        "\x00\x00\x10"
#define CMD_IDC_RESP_ERR_LEN        "\x00\x00\x11"
#define CMD_IDC_RESP_ERR_CMD        "\x00\x00\x21"
#define CMD_IDC_RESP_ERR_OTHERS     "\x00\x00\x24"
#define CMD_IDC_RESP_ERR_READ       "\x00\x00\x41"
#define CMD_IDC_RESP_ERR_DETECT     "\x00\x00\x80"
#define CMD_IDC_RESP_ERR_SELECT     "\x00\x00\x81"


/*  115200,8,N,1
 * -------------------          OFFLINE IDC PACKET                --------------------
 * COMM TX: FRAME_OL_HEAD(5) + FRAME_LENS_HIGH(1) + FRAME_LENS_LOW(1) + FRAME_OF_CMD(1) + FRAME_OF_PARAM(1) + FRAME_DATA_META(N) + LRC(1)
 * COMM RX: FRAME_OL_HEAD(5) + FRAME_LENS_HIGH(1) + FRAME_LENS_LOW(1) + FRAME_OF_RESP(3) + FRAME_DATA_META(N) + LRC(1)
 * ----------------------------------------------------------------------------------
*/

typedef enum {
	UART_OK = 1,
	UART_INIT_OK = 0,
	UART_INIT_FAIL = -1,
	UART_SEND_FAIL = -2,
	UART_RCV_FAIL = -3,
		
}uart_status_t;

void Sleep(int ms);
int uart_send_data(uint8_t *buf, uint16_t len) ;
int wait_uart_data(uint8_t time_out);
int uart_read_data(uint8_t *buf, int len);
int uart_read_data_noblock(uint8_t *buf, int len, uint16_t time_out);
int uart_comm_init(char *serialName);
int uart_comm_deinit(void);
int uart_comm_rcv_listener(uint8_t *rcv_data);
//int idc_comm_exchange(uint8_t *cmd, uint8_t *in_data, int in_len, uint8_t *out_data, int *out_len);

#endif /* UART_H_ */
