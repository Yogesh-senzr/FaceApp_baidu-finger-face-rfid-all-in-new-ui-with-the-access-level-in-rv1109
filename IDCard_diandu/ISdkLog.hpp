#pragma once

enum ES_LOG_LEVEL {
	ES_LOG_LEVEL_NULL = 0,
	ES_LOG_LEVEL_DEBUG = 1,
	ES_LOG_LEVEL_INFO = 2,
	ES_LOG_LEVEL_WARN = 3,
	ES_LOG_LEVEL_ERROR = 4,
};

typedef void(*PES_LogCB)(ES_LOG_LEVEL level, const char* szMsg, void* lpVoid);

