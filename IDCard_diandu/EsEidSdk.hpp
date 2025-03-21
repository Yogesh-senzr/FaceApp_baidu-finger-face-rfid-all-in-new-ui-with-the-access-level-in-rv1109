#pragma once
#include "EidSdkStruct.hpp"
#include "ISdkLog.hpp"
#ifdef WIN32
#ifdef ESEIDSDK_EXPORTS
#define ESEIDSDK_API __declspec(dllexport)
#else
#define ESEIDSDK_API __declspec(dllimport)
#endif	// ESEIDSDK_EXPORTS
#elif ESEIDSDK_EXPORTS
#define ESEIDSDK_API __attribute__ ((visibility("default")))
#else
#define ESEIDSDK_API 
#endif	// WIN32
// SDK 版本号
#define ESEIDSDK_VER "22.05.21.17"
// ----------------------------------------------------------------------------
// 前置参数配置，必须再Init之前配置
// ----------------------------------------------------------------------------
ESEIDSDK_API BOOL ES_EIDSDK_SetPublicKey(int hostIdx, const char* szPubKey);
ESEIDSDK_API BOOL ES_EIDSDK_SetAppID(const char* szAppID);
ESEIDSDK_API BOOL ES_EIDSDK_SetHostEnable(int hostIdx, bool bEnable);
ESEIDSDK_API BOOL ES_EIDSDK_SetHost(int hostIdx, const char* szHost, int port);
ESEIDSDK_API BOOL ES_EIDSDK_SetUart(const char* szDevPath, int bps);
ESEIDSDK_API BOOL ES_EIDSDK_SetUsbHid(int vid, int pid);
ESEIDSDK_API BOOL ES_EIDSDK_SetUsbHidEx(int vid, int pid, int idx);
ESEIDSDK_API BOOL ES_EIDSDK_SetTransDelay(int ms);
ESEIDSDK_API BOOL ES_EIDSDK_SetCfg(int idx, int v, char* data, int len);
// ----------------------------------------------------------------------------
// 参数配置：即时生效
// ----------------------------------------------------------------------------
ESEIDSDK_API BOOL ES_EIDSDK_SetOnlyReqID(BOOL bOnlyReqID);
ESEIDSDK_API BOOL ES_EIDSDK_SetAutoSdt(BOOL bAutoSdt);
// ----------------------------------------------------------------------------
// 证件图片信息获取开关，不需要图片信息时配置FALSE，可以提高读证速度
ESEIDSDK_API BOOL ES_EIDSDK_SetImg(BOOL on);
// ----------------------------------------------------------------------------
// 服务管理
// ----------------------------------------------------------------------------
ESEIDSDK_API BOOL ES_EIDSDK_Init(const char* ioType, const char* flag);
ESEIDSDK_API void ES_EIDSDK_Release();
ESEIDSDK_API BOOL ES_EIDSDK_Connect();
ESEIDSDK_API BOOL ES_EIDSDK_Disconnect();
ESEIDSDK_API BOOL ES_EIDSDK_ReadCard();
ESEIDSDK_API BOOL ES_EIDSDK_Start();
ESEIDSDK_API BOOL ES_EIDSDK_IsStart();
ESEIDSDK_API BOOL ES_EIDSDK_Stop();
ESEIDSDK_API BOOL ES_EIDSDK_Pause();
ESEIDSDK_API BOOL ES_EIDSDK_IsPause();
ESEIDSDK_API BOOL ES_EIDSDK_Resume();
ESEIDSDK_API EsIdcardStatus ES_EIDSDK_GetStatus();
// ----------------------------------------------------------------------------
// 设备指令
// ----------------------------------------------------------------------------
ESEIDSDK_API BOOL ES_EIDSDK_SendCmd(unsigned char cmdType, unsigned char cmd, unsigned char cmdParam, const unsigned char* inData, int inDataLen);
ESEIDSDK_API BOOL ES_EIDSDK_SendSdt(PEsIdcardInfo pInfo);
// 设备重启
ESEIDSDK_API BOOL ES_EIDSDK_Restart();
// 重置射频芯片，用于快速重置读卡状态，重新开始读卡
ESEIDSDK_API BOOL ES_EIDSDK_RestartNfc();
// ---------------------------------------------------------------------------------
// 回调应尽快返回，不要影响程序正常执行
// ---------------------------------------------------------------------------------
ESEIDSDK_API void ES_EIDSDK_SetLogCB(PES_LogCB cb, LPVOID lpVoid);
ESEIDSDK_API void ES_EIDSDK_SetIdcardCB(PEsIdcardCB cb, LPVOID lpVoid);
ESEIDSDK_API void ES_EIDSDK_SetIdcardStatusCB(PEsIdcardStatusCB cb, LPVOID lpVoid);
// ---------------------------------------------------------------------------------
// 信息获取
// ---------------------------------------------------------------------------------
ESEIDSDK_API BOOL ES_EIDSDK_GetSamID(char* id);
ESEIDSDK_API BOOL ES_EIDSDK_GetSdtID(char* id);
ESEIDSDK_API BOOL ES_EIDSDK_GetProVersion(char* ver);
ESEIDSDK_API BOOL ES_EIDSDK_GetHardVersion(char* ver);
ESEIDSDK_API BOOL ES_EIDSDK_GetSlaveVersion(char* ver);
ESEIDSDK_API int ES_EIDSDK_GetIrMin();
ESEIDSDK_API int ES_EIDSDK_GetIrMax();
// ---------------------------------------------------------------------------------
// 测温
// ---------------------------------------------------------------------------------
ESEIDSDK_API BOOL ES_EIDSDK_SetTempEnable(BOOL enable);
ESEIDSDK_API void ES_EIDSDK_SetTempCB(PEsTempCB cb, LPVOID lpVoid);
ESEIDSDK_API void ES_EIDSDK_SetTempStatusCB(PEsTempStatusCB cb, LPVOID lpVoid);
ESEIDSDK_API void ES_EIDSDK_SetIrCB(PEsIrCB cb, LPVOID lpVoid);
ESEIDSDK_API void ES_EIDSDK_SetIrStatusCB(PEsIrStatusCB cb, LPVOID lpVoid);
// ---------------------------------------------------------------------------------
// 升级
// ---------------------------------------------------------------------------------
ESEIDSDK_API BOOL ES_EIDSDK_Update(const char *szBinFile);
ESEIDSDK_API BOOL ES_EIDSDK_UpdateBegin();
ESEIDSDK_API BOOL ES_EIDSDK_UpdateData(const unsigned char* inData, int inDataLen);
ESEIDSDK_API BOOL ES_EIDSDK_UpdateSuccess();
ESEIDSDK_API BOOL ES_EIDSDK_UpdateError();
// ---------------------------------------------------------------------------------
// APDU模式
// ---------------------------------------------------------------------------------
ESEIDSDK_API BOOL ES_EIDSDK_SetApduEnable(BOOL enable);
ESEIDSDK_API void ES_EIDSDK_SetApduCB(PEsApduCB cb, LPVOID lpVoid);
ESEIDSDK_API BOOL ES_EIDSDK_SendApdu(unsigned char cmd, unsigned char userParam, const unsigned char* inData, int inDataLen);
// ---------------------------------------------------------------------------------
// Wlt文件解码: wltLen=1024, bmpLen=38862
// ---------------------------------------------------------------------------------
ESEIDSDK_API BOOL ES_EIDSDK_WltUnpack(const char* wltData, int wltLen, char* bmp, int bmpLen);
