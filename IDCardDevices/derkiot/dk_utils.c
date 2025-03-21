#include <stddef.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "dk_utils.h"
#include "stdio.h"

void Sleep(int ms) {
	struct timeval delay;
	delay.tv_sec = 0;
	delay.tv_usec = ms * 1000;
	select(0, NULL, NULL, NULL, &delay);
}

char char_ascii[] = "0123456789abcdef";
void byteHexToSting(uint8_t *hex, uint16_t hex_len, char *hex_str) {
	uint16_t i = 0;
	for( i = 0; i < hex_len; i++) {
		hex_str[2*i] =char_ascii[(hex[i]&0xf0) >> 4] ;
		hex_str[2*i +1] = char_ascii[(hex[i]&0x0f)];
	}
	hex_str[2*i] = '\0' ;

}

//异或校验
int calc_lrc(uint8_t *in_data, int len) {
	unsigned char lrc;

	if (in_data == NULL || len <= 0)
		return 0;
	lrc = 0;
	while (len-- > 0) {
		lrc ^= *in_data++;
	}
	return lrc;
}
#define TIME_COUNT 1

#ifdef TIME_COUNT
void system_time_printf(void) {
	char buf[32] = {0};
	struct timeval tv;
	struct tm      tm;
	size_t         len= 28;
			
	memset(&tv, 0, sizeof(tv));
	memset(&tm, 0, sizeof(tm));
	gettimeofday(&tv, NULL);
	localtime_r(&tv.tv_sec, &tm);
	strftime(buf, len, "%Y-%m-%d %H:%M:%S", &tm);
	len = strlen(buf);
	sprintf(buf + len, ".%-6.3d", (int)(tv.tv_usec/1000)); 
	printf("%s\n", buf);
}

#else 
#define system_time_printf(...)

#endif 


