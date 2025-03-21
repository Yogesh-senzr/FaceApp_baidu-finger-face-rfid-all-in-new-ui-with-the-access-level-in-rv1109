/* 
 * File:   main.cpp
 * Author: lfx
 *
 * Created on 2020年9月2日, 上午10:20
 */
#include "stdafx.h"
#include <cstdlib>
#include <sys/time.h>

char getch()
{
	char c;
	system("stty -echo");
	system("stty -icanon");
	c = getchar();
	system("stty icanon");
	system("stty echo");
	return c;
}

#define DEMO_LIB
// #define DEMO_MSG_CLI

#ifdef DEMO_LIB // 项目挂载
#include "EsEidSdk.hpp"

long EsLogTime() {
	// 真实时间
	//struct timeval tv;
	//gettimeofday(&tv, NULL);
	//return tv.tv_sec * 1000 + tv.tv_usec / 1000;
	// 开机毫秒数
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}


void ESLog(const char* fmt, ...)
{
	va_list argList;
	va_start(argList, fmt);
	// time
	long tick = EsLogTime();
	int msecs = (int)(tick % 1000);
	tick /= 1000;
	int secs = (int)(tick % 60);
	tick /= 60;
	int minutes = (int)(tick % 60);
	tick /= 60;
	int hours = (int)tick;
	// buf
	char buf[64] = { 0 };
	sprintf(buf, "[%02d:%02d:%02d:%03d]", hours, minutes, secs, msecs);
	printf("%-16s", buf);
	vprintf(fmt, argList);
	printf("\n");
	va_end(argList);
}

void LogCB(ES_LOG_LEVEL level, const char* szMsg, LPVOID lpVoid)
{
	ESLog(szMsg);
}

void EsIdcardCB(int notify, const char* data, int dataLen, LPVOID lpVoid)
{
printf(">>>>%s,%s,%d,notify=%d\n",__FILE__,__func__,__LINE__,notify);	
	if (notify == ESIDCARD_NOTIFY_FIND) {
		printf("EsIdcardCB: find card\n");
	}
	if (data && notify == ESIDCARD_NOTIFY_SUCCESS) {
    printf(">>>>%s,%s,%d\n",__FILE__,__func__,__LINE__);
		PEsIdcardInfo pInfo = (PEsIdcardInfo)data;
		ESLog("EsIdcardCB: name  : %s", pInfo->szName);
		ESLog("EsIdcardCB: sex   : %s", pInfo->szSex[0] ? "男" : "女");
		ESLog("EsIdcardCB: nation: %s", pInfo->szNation);
		ESLog("EsIdcardCB: cert  : %s", pInfo->szCert);
		ESLog("EsIdcardCB: addr  : %s", pInfo->szAddr);
	}
}
void test(int argc, char** argv) {
	// ----------------------------------------------------
	// flag: 厂商标记，eid_demo 是测试标记，发布前请联系服务商获取正式版标记
	if (argc == 3 && strcmp(argv[1], "uart") == 0) {
		ES_EIDSDK_SetUart(argv[2], 115200);
		ES_EIDSDK_Init(ES_EID_SDK_IOTYPE_UART, "eid_demo");
	}
	else if (argc == 2 && strcmp(argv[1], "uart") == 0) {
		ES_EIDSDK_SetUart("/dev/ttySLB3", 115200);
		ES_EIDSDK_Init(ES_EID_SDK_IOTYPE_UART, "eid_demo");
	}
	else {
		ES_EIDSDK_Init(ES_EID_SDK_IOTYPE_USBHID, "eid_demo");
	}
	ES_EIDSDK_SetLogCB(LogCB, NULL);
	ES_EIDSDK_SetIdcardCB(EsIdcardCB, NULL);
	// ----------------------------------------------------
	if (!ES_EIDSDK_Start())
	{
		ESLog("ES_EIDSDK_Start error");
		return;
	}
	ESLog("ES_EIDSDK_Start");

	getchar();

	ESLog("-------------- stop --------------\n");
	ES_EIDSDK_Stop();
}
#endif

int main(int argc, char** argv)
{
    setlocale(LC_CTYPE, setlocale(LC_ALL, "zh_CN.UTF-8"));
	test(argc, argv);
    return 0;
}

