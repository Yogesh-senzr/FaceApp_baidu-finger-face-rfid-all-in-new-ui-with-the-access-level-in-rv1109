/* 
 * File:   main.cpp
 * Author: lfx
 *
 * Created on 2020年9月2日, 上午10:20
 */
#include "stdafx.h"
#include <cstdlib>

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

void LogCB(ES_LOG_LEVEL level, const char* szMsg, LPVOID lpVoid)
{
    printf("%s\n", szMsg);
}
void EsIdcardCB(int notify, const char* data, int dataLen, LPVOID lpVoid)
{
	if (notify == ESIDCARD_NOTIFY_FIND) {
		printf("EsIdcardCB: find card\n");
	}
	if (data && notify == ESIDCARD_NOTIFY_SUCCESS) {

		PEsIdcardInfo pInfo = (PEsIdcardInfo)data;
		printf("EsIdcardCB: name: %s\n", pInfo->szName);
		printf("EsIdcardCB: cert: %s\n", pInfo->szCert);
		printf("EsIdcardCB: addr: %s\n", pInfo->szAddr);
	}
}
void test() {
	// ----------------------------------------------------
	// flag: 厂商标记，eid_demo 是测试标记，发布前请联系服务商获取正式版标记
	ES_EIDSDK_Init(ES_EID_SDK_IOTYPE_USBHID, "eid_demo");
	ES_EIDSDK_SetLogCB(LogCB, NULL);
	ES_EIDSDK_SetIdcardCB(EsIdcardCB, NULL);
	// ----------------------------------------------------
	if (!ES_EIDSDK_Start())
	{
		printf("ES_EIDSDK_Start error");
		return;
	}
	printf("ES_EIDSDK_Start");

	getchar();

	printf("-------------- stop --------------\n");
	ES_EIDSDK_Stop();
}
#endif

int main(int argc, char** argv)
{
    setlocale(LC_CTYPE, setlocale(LC_ALL, "zh_CN.UTF-8"));
	test();
    return 0;
}

