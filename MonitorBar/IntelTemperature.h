#pragma once
#include "Temperature.h"
class CIntelTemperature :
	public CTemperature
{
public:
	CIntelTemperature();
	~CIntelTemperature( );
	STATUS_CODE Init();
	void Update();
	double GetPercent()const;
private:
	STATUS_CODE __Init();
	STATUS_CODE __ReadMsr(unsigned long, KAFFINITY, unsigned long long*)const;
	void __InitTjMax(BYTE);
	void __InitTjMaxFromMsr();
private:
	PBYTE m_pTjMax;
	const DWORD dwIOCTL_READ_MSR;
	int m_nAllTemp;
	DWORD m_dwNumOfAvailableProcesses;
private:
	const unsigned int nIA32_THERM_STATUS_MSR;
	const unsigned int nIA32_TEMPERATURE_TARGET;
};

