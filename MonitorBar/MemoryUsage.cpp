#include "MemoryUsage.h"
#include<TlHelp32.h>
#include<Psapi.h>
#include<sstream>
#include<iomanip>
#include"Log.h"

CMemoryUsage::CMemoryUsage( )
:m_dwMin(101)
, m_dwMax(0)
, m_dwCur(0)
, m_hToken(nullptr)
, m_bNotAllAssigned(false)
{
	LOGOUT("CMemoryUsage构造");
}

CMemoryUsage::~CMemoryUsage( )
{
	if (m_hToken)
		CloseHandle(m_hToken);
	LOGOUT("CMemoryUsage析构");
}

bool CMemoryUsage::Init()
{
	if (!OpenProcessToken(GetCurrentProcess( ), TOKEN_ADJUST_PRIVILEGES, &m_hToken))
	{
		LOGERR("OpenProcessToken", GetLastError( ));
		return false;
	}
	LUID luid;
	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid))
	{
		LOGERR("LookupPrivilegeValue", GetLastError());
		return false;
	}
	TOKEN_PRIVILEGES tp = { 1 };
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!AdjustTokenPrivileges(m_hToken, FALSE, &tp, sizeof( TOKEN_PRIVILEGES ), nullptr, nullptr))
	{
		LOGERR("AdjustTokenPrivileges", GetLastError());
		return false;
	}
	m_bNotAllAssigned = GetLastError() == ERROR_NOT_ALL_ASSIGNED;
	return true;
}

const std::basic_string<TCHAR> CMemoryUsage::ToString( )const
{
	std::basic_ostringstream<TCHAR> ret;
	ret << _T("内存使用率:") << std::setw(6) << m_dwCur << _T("％");
	return ret.str( );
}

const std::wstring CMemoryUsage::ToLongString( )const
{
	std::wostringstream ret;
	ret << L"内存最大使用率:" << std::setw(3) << m_dwMax
		<< L"%\n内存最小使用率:" << std::setw(3) << m_dwMin
		<< L"%\n内存使用率前三的进程\n"
		<< std::setw(5) << L"进程ID"
		<< std::setw(8) << L"工作集使用量"
		<< L"\t进程名" << std::endl;
	for (auto & proc : m_maxProcesses)
		ret << std::setw(6) << proc.pid
		<< std::setw(12) << __Bytes2String(proc.size)
		<< L'\t' << proc.name << std::endl;
	return ret.str( );
}

const double CMemoryUsage::GetValue( )const
{
	return m_dwCur;
}

void CMemoryUsage::Update( )
{
	MEMORYSTATUSEX msex = { sizeof( MEMORYSTATUSEX ) };
	if (!GlobalMemoryStatusEx(&msex))
	{
		LOGERR("GlobalMemoryStatusEx", GetLastError( ));
		return;
	}
	m_dwCur = msex.dwMemoryLoad;
	if (m_dwCur > m_dwMax)
		m_dwMax = m_dwCur;
	if (m_dwCur < m_dwMin)
		m_dwMin = m_dwCur;
	__LoopForProcesses( );
}

void CMemoryUsage::Reset( )
{
	m_dwMin = 101;
	m_dwMax = 0;
	m_dwCur = 0;
}

const std::wstring CMemoryUsage::__Bytes2String(unsigned long long sz)const
{
	std::wostringstream ret;
	if (sz < 1024) ret << sz << L" B";
	else if (sz < 1024 * 1024) ret << std::setprecision(2) << std::fixed << sz / 1024.0 << L" KB";
	else if (sz < 1024 * 1024 * 1024)ret << std::setprecision(2) << std::fixed << sz / 1024.0 / 1024.0 << L" MB";
	else if (sz < 1024ULL * 1024 * 1024 * 1024)ret << std::setprecision(2) << std::fixed << sz / 1024.0 / 1024.0 / 1024.0 << L" GB";
	else ret << std::setprecision(2) << std::fixed << sz / 1024.0 / 1024.0 / 1024.0 / 1024.0 << L" TB";
	return ret.str( );
}

void CMemoryUsage::__LoopForProcesses( )
{
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hProcessSnap)
	{
		LOGERR("CreateToolhelp32Snapshot", GetLastError( ));
		return;
	}
	BOOL b;
	PROCESSENTRY32 pe32 = { sizeof( PROCESSENTRY32 ) };
	for (auto &x : m_maxProcesses)
		x.size = 0;
	for (b = Process32First(hProcessSnap, &pe32); b; b = Process32Next(hProcessSnap, &pe32))
	{
		if (!pe32.th32ProcessID)continue;
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
		if (!hProcess)
		{
			if (5 != GetLastError())
			{
				LOGERR("OpenProcess", GetLastError());
				return;
			}
		}
		else
		{
			PROCESS_MEMORY_COUNTERS pmc = { sizeof( PROCESS_MEMORY_COUNTERS ) };
			if (GetProcessMemoryInfo(hProcess, &pmc, sizeof pmc))
				for (size_t i = 0; i < _countof(m_maxProcesses); ++i)
					if (m_maxProcesses[i].size < pmc.WorkingSetSize)
					{
						for (size_t j = _countof(m_maxProcesses) - 1; j >i; --j)
							m_maxProcesses[j] = m_maxProcesses[j - 1];
						_tcscpy_s(m_maxProcesses[i].name, pe32.szExeFile);
						m_maxProcesses[i].pid = pe32.th32ProcessID;
						m_maxProcesses[i].size = pmc.WorkingSetSize;
						break;
					}
			CloseHandle(hProcess);
		}
	}
	CloseHandle(hProcessSnap);
}