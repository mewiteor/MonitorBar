#pragma once
#include "IMonitor.h"
#include<Windows.h>
#include<winternl.h>
#include<map>

class CCpuUsage :
	public IMonitor
{
	typedef struct tagPROCESS_INFO
	{
		TCHAR name[MAX_PATH];
		unsigned long long last_time, last_system_time;
		double cpu_usage;
	}PROCESS_INFO;

public:
	CCpuUsage( );
	~CCpuUsage( );

	const std::basic_string<TCHAR> ToString( )const;
	const std::wstring ToLongString( )const;
	const double GetValue()const;
	bool Init( ){ return true; }
	void Update();
	void Reset( );
private:
	unsigned long long __FileTime2Utc(const FILETIME &);
	double __GetCpuUsage(unsigned long long &, unsigned long long &);
	double __GetCpuUsage(HANDLE,unsigned long long &, unsigned long long &);
	void __LoopForProcesses();
	const DWORD __GetCpuConut( )const;
private:
	double m_dMax, m_dMin, m_dCur;
	const DWORD dwNUMBER_OF_PROCESSORS;
	unsigned long long m_ullLastTime, m_ullLastIdleTime;
	std::map<DWORD, PROCESS_INFO> m_mapProcessMap;
	std::pair<DWORD, PROCESS_INFO> m_pairMaxProcesses[3];
};

