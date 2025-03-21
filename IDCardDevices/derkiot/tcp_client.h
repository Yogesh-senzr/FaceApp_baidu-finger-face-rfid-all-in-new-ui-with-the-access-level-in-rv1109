/*
 * tcp_client.h
 *
 *  Created on: 2020年11月3日
 *      Author: derkiot
 */

#ifndef TCP_CLIENT_H_
#define TCP_CLIENT_H_
#include "dk_utils.h"

typedef enum {
	SVR_OK = 1,
	SVR_INIT_OK = 0,
	SVR_INIT_FAIL = -1,
	SVR_TIMEOUT = -2,
	SVR_FAIL = -3,
}svr_status;

int TcpClientStart(void);
int TcpClientClose(void);
int GetTcpClientStatus(void);
bool check_tcp_alive(void);
int SendPacket( uint8_t *res, uint16_t len);
int ReadPacket(uint8_t *rcv_data, uint16_t *rcv_data_len);
int dkCloudTcpDataExchange(uint8_t *initData, uint16_t initData_len, uint8_t *rcv_data, uint16_t *rcv_data_len);
#endif /* TCP_CLIENT_H_ */
