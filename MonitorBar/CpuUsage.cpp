#include "CpuUsage.h"
#include<TlHelp32.h>
#include<sstream>
#include<iomanip>
#include"Log.h"

CCpuUsage::CCpuUsage( )
:m_dMax(-1)
, m_dMin(101)
, m_dCur(0)
, dwNUMBER_OF_PROCESSORS(__GetCpuConut( ))
, m_ullLastTime(0)
, m_ullLastIdleTime(0)
{
	LOGOUT("CCpuUsage构造");
}

CCpuUsage::~CCpuUsage( )
{
	LOGOUT("CCpuUsage析构");
}

const std::basic_string<TCHAR> CCpuUsage::ToString( )const
{
	std::basic_ostringstream<TCHAR> oss;
	oss << _T("C P U 使用率:") << std::setw(6) << std::setprecision(2)
		<< std::fixed << m_dCur << _T("％");
	return oss.str( );
}

const std::wstring CCpuUsage::ToLongString( )const
{
	std::wostringstream ret;
	ret << L"CPU最大使用率:" << std::setw(6) << std::setprecision(2) << std::fixed << m_dMax
		<< L"%\nCPU最小使用率:" << std::setw(6) << std::setprecision(2) << std::fixed << m_dMin
		<< L"%\nCPU使用率前三的进程\n"
		<< std::setw(5) << L"进程ID"
		<< std::setw(8) << L"CPU使用率"
		<< L"\t进程名" << std::endl;
	for (auto & proc : m_pairMaxProcesses)
		ret << std::setw(6) << proc.first
		<< std::setw(9) << proc.second.cpu_usage << L"%\t"
		<< proc.second.name << std::endl;
	return ret.str( );
}

const double CCpuUsage::GetValue( )const
{
	return m_dCur;
}

void CCpuUsage::Update( )
{
	double usage = __GetCpuUsage(m_ullLastTime, m_ullLastIdleTime);
	if (usage >= 0)
	{
		m_dCur = usage;
		if (m_dCur > m_dMax)m_dMax = m_dCur;
		if (m_dCur < m_dMin)m_dMin = m_dCur;
	}
	__LoopForProcesses( );

}

unsigned long long CCpuUsage::__FileTime2Utc(const FILETIME &ft)
{
	LARGE_INTEGER li;
	li.LowPart = ft.dwLowDateTime;
	li.HighPart = ft.dwHighDateTime;
	return li.QuadPart;
}

double CCpuUsage::__GetCpuUsage(unsigned long long &ullLastTime, unsigned long long &ullLastIdleTime)
{
	FILETIME idleTime, kernelTime, userTime;
	if (!GetSystemTimes(&idleTime, &kernelTime, &userTime))
	{
		LOGERR("GetSystemTimes", GetLastError( ));
		return -1;
	}
	unsigned long long _time = __FileTime2Utc(kernelTime) + __FileTime2Utc(userTime),
		_idle_time = __FileTime2Utc(idleTime);
	if (!ullLastTime || !ullLastIdleTime)
	{
		ullLastIdleTime = _idle_time;
		ullLastTime = _time;
		return -1;
	}
	unsigned long long idle = _idle_time - ullLastIdleTime,
		usage = _time - ullLastTime;
	ullLastIdleTime = _idle_time;
	ullLastTime = _time;
	if (!usage)
		return -1;
	return ( usage - idle )*100.0 / usage;
}

double CCpuUsage::__GetCpuUsage(HANDLE hProcess, unsigned long long&last_time, unsigned long long&last_system_time)
{
	FILETIME now, creation_time, exit_time, kernel_time, user_time;
	unsigned long long system_time, time, system_time_delta, time_delta;

	GetSystemTimeAsFileTime(&now);

	if (!GetProcessTimes(hProcess, &creation_time, &exit_time, &kernel_time, &user_time))
	{
		LOGERR("GetProcessTimes", GetLastError( ));
		return -1;
	}
	system_time = __FileTime2Utc(kernel_time) + __FileTime2Utc(user_time);
	time = __FileTime2Utc(now);
	if (!last_system_time || !last_time)
	{
		last_system_time = system_time;
		last_time = time;
		return -1;
	}
	system_time_delta = system_time - last_system_time;
	time_delta = time - last_time;
	if (!time_delta)
		return -1;
	last_system_time = system_time;
	last_time = time;
	return system_time_delta*100.0 / time_delta / dwNUMBER_OF_PROCESSORS;
}

void CCpuUsage::__LoopForProcesses( )
{
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hProcessSnap)
	{
		LOGERR("CreateToolhelp32Snapshot", GetLastError( ));
		return;
	}
	BOOL b;
	PROCESSENTRY32 pe32 = { sizeof( PROCESSENTRY32 ) };
	std::map<DWORD, PROCESS_INFO> oldPi;
	for (auto &x : m_pairMaxProcesses)
		x.second.cpu_usage = 0.0;
	m_mapProcessMap.swap(oldPi);
	for (b = Process32First(hProcessSnap, &pe32); b; b = Process32Next(hProcessSnap, &pe32))
	{
		if (!pe32.th32ProcessID)continue;
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pe32.th32ProcessID);
		if (!hProcess)
		{
			if (ERROR_ACCESS_DENIED != GetLastError( ))
			{
				LOGERR("OpenProcess", GetLastError( ));
				return;
			}
		}
		else
		{
			PROCESS_INFO tp = { 0 };
			auto it = oldPi.find(pe32.th32ProcessID);
			if (it != oldPi.end( ))
				tp = it->second;
			else
				_tcscpy_s(tp.name, pe32.szExeFile);
			double usage = __GetCpuUsage(hProcess, tp.last_time, tp.last_system_time);
			CloseHandle(hProcess);
			if (usage >= 0)
				tp.cpu_usage = usage;
			m_mapProcessMap.insert(std::make_pair(pe32.th32ProcessID, tp));
			if (usage >= 0)
			{
				for (size_t i = 0; i < _countof(m_pairMaxProcesses); ++i)
					if (tp.cpu_usage>m_pairMaxProcesses[i].second.cpu_usage)
					{
						for (size_t j = _countof(m_pairMaxProcesses) - 1; j > i; --j)
							m_pairMaxProcesses[j] = m_pairMaxProcesses[j - 1];
						m_pairMaxProcesses[i] = std::make_pair(pe32.th32ProcessID, tp);
						break;
					}
			}
		}
	}
	CloseHandle(hProcessSnap);
}

void CCpuUsage::Reset( )
{
	m_dMax = -1;
	m_dMin = 101;
	m_dCur = 0;
	m_ullLastTime = 0;
	m_ullLastIdleTime = 0;
}

const DWORD CCpuUsage::__GetCpuConut( )const
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	return si.dwNumberOfProcessors;
}