#include "Log.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <cstdarg>
#include <time.h>

Log::LOG_LEVEL Log::m_level = LOG_NONE;

void Log::Error(LPCWSTR fmt, ...)
{
    if (m_level < LOG_ERROR) { return; }
    va_list  args;
    va_start(args, fmt);
    LPWSTR buffer = new wchar_t[_vscwprintf(fmt, args) + 1];
    vswprintf(buffer, fmt, args);
    formatOutput(L" [E]: ", buffer);
    delete[] buffer;
    va_end(args);
}

void Log::Warn(LPCWSTR fmt, ...)
{
    if (m_level < LOG_WARN) { return; }
    va_list  args;
    va_start(args, fmt);
    LPWSTR buffer = new wchar_t[_vscwprintf(fmt, args) + 1];
    vswprintf(buffer, fmt, args);
    formatOutput(L" [W]: ", buffer);
    delete[] buffer;
    va_end(args);
}

void Log::Info(LPCWSTR fmt, ...)
{
    if (m_level < LOG_INFO) { return; }
    va_list  args;
    va_start(args, fmt);
    LPWSTR buffer = new wchar_t[_vscwprintf(fmt, args) + 1];
    vswprintf(buffer, fmt, args);
    formatOutput(L" [I]: ", buffer);
    delete[] buffer;
    va_end(args);
}

void Log::Debug(LPCWSTR fmt, ...)
{
    if (m_level < LOG_DEBUG) { return; }
    va_list  args;
    va_start(args, fmt);
    LPWSTR buffer = new wchar_t[_vscwprintf(fmt, args) + 1];
    vswprintf(buffer, fmt, args);
    formatOutput(L" [D]: ", buffer);
    delete[] buffer;
    va_end(args);
}

void Log::Detail(LPCWSTR fmt, ...)
{
    if (m_level < LOG_DETAIL) { return; }
    va_list  args;
    va_start(args, fmt);
    LPWSTR buffer = new wchar_t[_vscwprintf(fmt, args) + 1];
    vswprintf(buffer, fmt, args);
    formatOutput(L" [V]: ", buffer);
    delete[] buffer;
    va_end(args);
}

void Log::formatOutput(LPCWSTR pLevel, LPCWSTR pArgs)
{
    int stringLength = getClock() + wcslen(pLevel) + wcslen(pArgs) + wcslen(L"\n");
    LPWSTR pClock = new wchar_t[getClock() + 1];
    LPWSTR pBuffer = new wchar_t[stringLength + 1];
    getClock(pClock);
    _snwprintf(pBuffer, stringLength + 1, L"%s%s%s\n", pClock, pLevel, pArgs);
    OutputDebugString(pBuffer);
    delete[] pBuffer;
}

int Log::getClock(LPWSTR pBuffer)
{
    if (pBuffer == NULL) {
        return 24;
    }
    struct tm* pTm;
    struct timespec val;
    time_t t = time(NULL);
    int i = timespec_get(&val, TIME_UTC);
    pTm = localtime(&t);
    if (pTm == NULL) {
        return -1;
    }
    _snwprintf(pBuffer, 24, L"%04d/%02d/%02d %02d:%02d:%02d.%03d",
        pTm->tm_year + 1900, pTm->tm_mon + 1, pTm->tm_mday,
        pTm->tm_hour, pTm->tm_min, pTm->tm_sec, val.tv_nsec/1000/1000);
    return 24;
}

