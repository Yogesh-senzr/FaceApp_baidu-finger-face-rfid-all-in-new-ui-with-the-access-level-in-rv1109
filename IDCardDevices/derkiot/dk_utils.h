#ifndef __DK_UTILS_H__
#define __DK_UTILS_H__
#include "stdint.h"

#ifndef bool
#define bool int
#define false 0
#define true 1
#endif

#undef DK_DEBUG
#undef UART_OTA_ENABLE	
#undef ME_ANT

#ifdef DK_DEBUG
#define DEBUG_Printf(...) printf(__VA_ARGS__)  
#define dk_log_hex(title, data, len)	do{printf("%s",title);\
                                                               for(int i = 0; i < len; i++){printf("%02x",*(data+i) );} printf("\r\n");}while(0)
#else
#define DEBUG_Printf(...)
#define dk_log_hex(...)
#endif

void system_time_printf(void);
void Sleep(int ms);
int calc_lrc(uint8_t *in_data, int len);
void byteHexToSting(uint8_t *hex, uint16_t hex_len, char *hex_str);
#endif
