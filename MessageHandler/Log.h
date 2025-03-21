#ifndef LOG_H_
#define LOG_H_

#ifdef __cplusplus
extern "C"{
#endif

#define DEBUG_LEVEL_ERROR  (1)
#define DEBUG_LEVEL_DEBUG  (2)
#define DEBUG_LEVEL_ALL		(3)

void Log_Init();
void Log_Exit();
void Log_SetDebugLevel(int nDebugLevel);
void LogV(const char *format,...);
void LogD(const char *format,...);
void LogE(const char *format,...);
void LogDelete(int day);
#ifdef __cplusplus
}
#endif

#endif /* LOG_H_ */
