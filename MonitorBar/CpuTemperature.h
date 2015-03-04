#pragma once
#include "IMonitor.h"
#include"Temperature.h"

class CCpuTemperature :
	public IMonitor
{
public:
	CCpuTemperature();
	~CCpuTemperature( );
	const std::basic_string<TCHAR> ToString()const;
	const std::wstring ToLongString()const;
	const double GetValue()const;
	bool Init( );
	void Update();
	void Reset();
private:
	short m_sMax, m_sMin;
	short* m_pMaxOfEachCpuCoreTemp,* m_pMinOfEachCpuCoreTemp;
	const CPU_TYPE eCPU_TYPE;
	CTemperature *m_pCpuTemp;
};

