#pragma once
#include "IMonitor.h"
#include<Windows.h>

class CMemoryUsage :
	public IMonitor
{
public:
	CMemoryUsage( );
	~CMemoryUsage( );

	const std::basic_string<TCHAR> ToString()const;
	const std::wstring ToLongString()const;
	const double GetValue()const;
	bool Init( );
	void Update();
	void Reset();
private:
	void __LoopForProcesses();
	const std::wstring __Bytes2String(unsigned long long)const;
private:
	DWORD m_dwMin, m_dwMax, m_dwCur;
	struct
	{
		DWORD pid;
		TCHAR name[MAX_PATH];
		size_t size;
	}m_maxProcesses[3];
	HANDLE m_hToken;
	bool m_bNotAllAssigned;
};

