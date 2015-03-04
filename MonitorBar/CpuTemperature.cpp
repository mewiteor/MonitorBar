#include<Windows.h>
#include"IntelTemperature.h"
#include"AMD0FTemperature.h"
#include"AMD10Temperature.h"
#include "CpuTemperature.h"
#include<sstream>
#include<iomanip>
#include"Log.h"

CCpuTemperature::CCpuTemperature( )
: m_sMax(SHRT_MIN)
, m_sMin(SHRT_MAX)
, eCPU_TYPE(GetCpuType( ))
, m_pCpuTemp(nullptr)
, m_pMaxOfEachCpuCoreTemp(nullptr)
, m_pMinOfEachCpuCoreTemp(nullptr)
{
	LOGOUT("CCpuTemperature构造");
}


CCpuTemperature::~CCpuTemperature( )
{
	if (m_pCpuTemp)
	{
		delete m_pCpuTemp;
		m_pCpuTemp = nullptr;
	}
	if (m_pMaxOfEachCpuCoreTemp)
	{
		delete[] m_pMaxOfEachCpuCoreTemp;
		m_pMaxOfEachCpuCoreTemp = nullptr;
	}
	if (m_pMinOfEachCpuCoreTemp)
	{
		delete[] m_pMinOfEachCpuCoreTemp;
		m_pMinOfEachCpuCoreTemp = nullptr;
	}
	LOGOUT("CCpuTemperature析构");
}

bool CCpuTemperature::Init()
{
	switch (eCPU_TYPE)
	{
	case CPU_TYPE::CPU_TYPE_INTEL:m_pCpuTemp = new CIntelTemperature(); break;
	case CPU_TYPE::CPU_TYPE_AMD0F:m_pCpuTemp = new CAMD0FTemperature(); break;
	case CPU_TYPE::CPU_TYPE_AMD10:m_pCpuTemp = new CAMD10Temperature(); break;
	case CPU_TYPE::CPU_TYPE_UNKNOWN:
	default:
		return false;
	}
	if (STATUS_CODE::STATUS_CODE_NO_ERROR != m_pCpuTemp->Init( ))
	{
		delete m_pCpuTemp;
		m_pCpuTemp = nullptr;
		return false;
	}
	if (m_pCpuTemp->GetCpuCoreCount() > 1)
	{
		m_pMaxOfEachCpuCoreTemp = new short[m_pCpuTemp->GetCpuCoreCount()];
		m_pMinOfEachCpuCoreTemp = new short[m_pCpuTemp->GetCpuCoreCount()];
		for (DWORD i = 0; i < m_pCpuTemp->GetCpuCoreCount(); ++i)
		{
			m_pMaxOfEachCpuCoreTemp[i] = SHRT_MIN;
			m_pMinOfEachCpuCoreTemp[i] = SHRT_MAX;
		}
	}
	return true;
}

const std::basic_string<TCHAR> CCpuTemperature::ToString( )const
{
	std::basic_ostringstream<TCHAR> ret;
	if (m_pCpuTemp)
		ret << _T("C P U 温度:") << std::setw(6) << m_pCpuTemp->GetValue( ) << _T("℃");
	return ret.str( );
}

const std::wstring CCpuTemperature::ToLongString( )const
{
	std::wostringstream ret;
	if (m_sMax != SHRT_MIN)
		ret << L"CPU最高温度:" << std::setw(3) << m_sMax << L"℃" << std::endl;
	if (m_sMin != SHRT_MAX)
		ret << L"CPU最低温度:" << std::setw(3) << m_sMin << L"℃" << std::endl;
	if (m_pCpuTemp->GetCpuCoreCount( ) > 1)
	{
		ret << L"CPU每个核心的温度\n"
			<< L"核心号\t当前温度\t最高温度\t最低温度"
			<< std::endl;
		for (DWORD i = 0; i < m_pCpuTemp->GetCpuCoreCount( ); ++i)
		{
			ret << i;
			if (m_pCpuTemp->GetCpuCoreTemp(i) != SHRT_MIN)
				ret << L"\t" << m_pCpuTemp->GetCpuCoreTemp(i) << L"℃";
			else
				ret << L"\t";
			if (m_pMaxOfEachCpuCoreTemp[i] != SHRT_MIN)
				ret << L"\t\t" << m_pMaxOfEachCpuCoreTemp[i] << L"℃";
			else
				ret << L"\t\t";
			if (m_pMinOfEachCpuCoreTemp[i] != SHRT_MAX)
				ret << L"\t\t" << m_pMinOfEachCpuCoreTemp[i] << L"℃";
			else
				ret << L"\t\t";
			ret << std::endl;
		}
	}
	return ret.str( );
}

const double CCpuTemperature::GetValue( )const
{
	if (m_pCpuTemp)
		return m_pCpuTemp->GetPercent( );
	else
		return 0;
}

void CCpuTemperature::Update( )
{
	if (m_pCpuTemp)
	{
		m_pCpuTemp->Update( );
		if (m_pCpuTemp->GetValue( ) > m_sMax)m_sMax = m_pCpuTemp->GetValue( );
		if (m_pCpuTemp->GetValue( ) < m_sMin)m_sMin = m_pCpuTemp->GetValue( );
		if (m_pCpuTemp->GetCpuCoreCount( )>1)
			for (DWORD i = 0; i < m_pCpuTemp->GetCpuCoreCount( ); ++i)
			{
				if (m_pCpuTemp->GetCpuCoreTemp(i) != SHRT_MIN&&
					m_pCpuTemp->GetCpuCoreTemp(i) > m_pMaxOfEachCpuCoreTemp[i])
					m_pMaxOfEachCpuCoreTemp[i] = m_pCpuTemp->GetCpuCoreTemp(i);
				if (m_pCpuTemp->GetCpuCoreTemp(i) != SHRT_MIN&&
					m_pCpuTemp->GetCpuCoreTemp(i) < m_pMinOfEachCpuCoreTemp[i])
					m_pMinOfEachCpuCoreTemp[i] = m_pCpuTemp->GetCpuCoreTemp(i);
			}
	}
}

void CCpuTemperature::Reset( )
{
	if (m_pCpuTemp)
		m_pCpuTemp->Reset( );
	m_sMax = SHRT_MIN;
	m_sMin = SHRT_MAX;
	if (m_pMaxOfEachCpuCoreTemp)
		for (DWORD i = 0; i < m_pCpuTemp->GetCpuCoreCount( ); ++i)
			m_pMaxOfEachCpuCoreTemp[i] = SHRT_MIN;
	if (m_pMinOfEachCpuCoreTemp)
		for (DWORD i = 0; i < m_pCpuTemp->GetCpuCoreCount( ); ++i)
			m_pMinOfEachCpuCoreTemp[i] = SHRT_MAX;
}

