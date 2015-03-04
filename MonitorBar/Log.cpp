#include "Log.h"
#include <Windows.h>
#include<fstream>
#include<ctime>

extern HINSTANCE g_hInst;

CLog::CLog(const char* message)
{
	char szFileName[MAX_PATH];
	GetModuleFileNameA(g_hInst, szFileName, _countof(szFileName));
	char* pstr = strrchr(szFileName, '.');
	if (pstr)
	{
		*( pstr + 1 ) = 0;
		strcat_s(szFileName, "log");
		std::ofstream fs(szFileName, std::ios::app);
		time_t _t = time(NULL);
		char _time[256];
		tm _tm;
		localtime_s(&_tm, &_t);
		strftime(_time, _countof(_time), "%Y年%m月%d日 %H时%M分%S秒", &_tm);
		fs << "[" << _time << "] " << message << std::endl;
		fs.close( );
	}
}

CLog::CLog(const char* func, unsigned int line, const char* f, unsigned long errCode)
{
	char szFileName[MAX_PATH];
	GetModuleFileNameA(g_hInst, szFileName, _countof(szFileName));
	char* pstr = strrchr(szFileName, '.');
	if (pstr)
	{
		*( pstr + 1 ) = 0;
		strcat_s(szFileName, "log");
		std::ofstream fs(szFileName, std::ios::app);
		time_t _t = time(NULL);
		char _time[256];
		tm _tm;
		localtime_s(&_tm, &_t);
		strftime(_time, _countof(_time), "%Y年%m月%d日 %H时%M分%S秒", &_tm);
		char* lp;
		FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
					   FORMAT_MESSAGE_FROM_SYSTEM |
					   FORMAT_MESSAGE_IGNORE_INSERTS,
					   nullptr, errCode,
					   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (char*)&lp,
					   0, nullptr);
		fs << "[" << _time << "] Error:{\t"
			<< "error position:" << func << ",\t"
			<< "error line:" << line << ",\t"
			<< "error function:" << f << ",\t"
			<< "error code:" << errCode << ",\t"
			<< "error information:" << lp << "}"
			<< std::endl;
		LocalFree(lp);
		fs.close();
	}
}