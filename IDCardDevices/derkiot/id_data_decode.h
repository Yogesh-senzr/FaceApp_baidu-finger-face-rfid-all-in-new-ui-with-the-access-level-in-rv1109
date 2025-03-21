/*
 * id_data_decode.h
 *
 *  Created on: 2020年11月4日
 *      Author: derkiot
 */

#ifndef ID_DATA_DECODE_H_
#define ID_DATA_DECODE_H_
#include "stdint.h"
#define LOCAL_DECODE_IMG 1
typedef struct IDCardData {
	unsigned char name[30];		     /* 姓名  */        // offset=0
	unsigned char gender[2];            // 30
	unsigned char national[4];        // 32
	unsigned char birthday[16];      // 36
	unsigned char address[70];      // 52
	unsigned char id[36];    // 122
	unsigned char maker[30];    // 158
	unsigned char start_date[16]; // 188
	unsigned char end_date[16];   // 204
	unsigned char reserved[36];
} __attribute__((__packed__)) St_IDCardData, *PSt_IDCardData;

void id_data_decode(uint8_t *id_info, uint16_t id_info_len, uint8_t *id_bitmap,
				uint16_t id_bitmap_len);

#endif /* ID_DATA_DECODE_H_ */
