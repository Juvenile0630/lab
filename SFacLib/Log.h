#pragma once
#include <windows.h>

class Log
{
public:
	enum LOG_LEVEL{
		LOG_NONE = 0,
		LOG_ERROR,
		LOG_WARN,
		LOG_INFO,
		LOG_DEBUG,
		LOG_DETAIL,
	};
	static void SetLogLevel(LOG_LEVEL level) { m_level = level; };
	static void Error(LPCWSTR fmt, ...);
	static void Warn(LPCWSTR fmt, ...);
	static void Info(LPCWSTR fmt, ...);
	static void Debug(LPCWSTR fmt, ...);
	static void Detail(LPCWSTR fmt, ...);

private:
	static int getClock(LPWSTR pBuffer = NULL);
	static void formatOutput(LPCWSTR level, LPCWSTR pArgs);
	static LOG_LEVEL m_level;

};

