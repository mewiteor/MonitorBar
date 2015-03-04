#include"Temperature.h"

#include<intrin.h>
#include<tchar.h>
#include"Log.h"

CTemperature::CTemperature( )
:m_bIsNT(__IsNT( ))
, m_bIsCpuid(__IsCpuid())
, m_bIsMsr(false)
, m_hDriver(INVALID_HANDLE_VALUE)
, dwIOCTL_TYPE(40000)
, m_sFamily(0)
, m_sModel(0)
, m_sStepping(0)
, m_dwCpuCount(0)
, m_dwCpuCoreCount(0)
, m_pMasks(nullptr)
, m_sCur(SHRT_MIN)
, m_pEachCpuCoreTemp(nullptr)
{
	LOGOUT("CTemperature¹¹Ôì");
}

CTemperature::~CTemperature( )
{
	if (m_pMasks)
		delete[] m_pMasks;
	if (m_pEachCpuCoreTemp)
		delete[] m_pEachCpuCoreTemp;
	if (INVALID_HANDLE_VALUE != m_hDriver)
		CloseHandle(m_hDriver);
	LOGOUT("CTemperatureÎö¹¹");
}

STATUS_CODE CTemperature::Init( )
{
	STATUS_CODE sc = __Init( );
	if (STATUS_CODE::STATUS_CODE_NO_ERROR == sc)
		__InitProcessorsInfo( );
	return sc;
}

const bool CTemperature::__IsNT( )const
{
	OSVERSIONINFOEX osie = { sizeof( OSVERSIONINFOEX ), 0, 0, 0, VER_PLATFORM_WIN32_NT };
	return !!VerifyVersionInfo(&osie, VER_PLATFORMID, VerSetConditionMask(0, VER_PLATFORMID, VER_EQUAL));
}

const bool CTemperature::__IsCpuid( )const
{
	__try
	{
		int info[4];
		__cpuid(info, 0);
		return true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return false;
	}
}

const bool CTemperature::__IsMsr( )const
{
	int info[4];
	__cpuid(info, 1);
	return !!( ( info[3] >> 5 ) & 1 );
}

STATUS_CODE CTemperature::__Init()
{
	if (m_bIsCpuid)
	{
		m_bIsMsr = __IsMsr();
		__try
		{
			int info[4];
			__cpuid(info, 1);
			m_sFamily = info[0] >> 8 & 0xf;
			m_sModel = info[0] >> 4 & 0xf;
			m_sStepping = info[0] & 0xf;
			if (0xf == m_sFamily ||
				6 == m_sFamily)
				m_sModel |= info[0] >> 12 & 0xf0;
			if (0xf == m_sFamily)
				m_sFamily += info[0] >> 20 & 0xff;
		}
		__except (EXCEPTION_EXECUTE_HANDLER){}
	}
	if (m_bIsNT)
	{
		m_hDriver = CreateFile(_T("\\\\.\\WinRing0_1_2_0"),
							   GENERIC_READ | GENERIC_WRITE, 0, nullptr,
							   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (INVALID_HANDLE_VALUE == m_hDriver)
			return STATUS_CODE::STATUS_CODE_DRIVER_NOT_LOADED;
		return STATUS_CODE::STATUS_CODE_NO_ERROR;
	}
	return STATUS_CODE::STATUS_CODE_UNKNOWN_ERROR;
}

void CTemperature::__InitProcessorsInfo()
{
	DWORD len;
	GetLogicalProcessorInformationEx(
		RelationProcessorCore,
		NULL,
		&len);
	PBYTE buffer = new BYTE[len];
	GetLogicalProcessorInformationEx(
		RelationProcessorCore,
		reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>( buffer),
		&len);
	m_dwCpuCoreCount = 0;
	for (PBYTE bufferTmp = buffer;
		 bufferTmp < buffer + len;
		 bufferTmp += reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>( bufferTmp )->Size)
		++m_dwCpuCoreCount;
	if (m_dwCpuCoreCount)
	{
		m_pMasks = new KAFFINITY[m_dwCpuCoreCount];
		m_dwCpuCount = 0;
		size_t i = 0;
		for (PBYTE bufferTmp = buffer;
			 bufferTmp < buffer + len;
			 bufferTmp += reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>( bufferTmp )->Size,++i)
		{
			auto ptr = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>( bufferTmp );
			if (ptr->Processor.GroupCount != 1)
				m_pMasks[i] = 0;
			else
			{
				m_pMasks[i] = ptr->Processor.GroupMask[0].Mask;
				for (auto mask = m_pMasks[i]; mask; mask &= mask - 1)
					++m_dwCpuCount;
			}
		}
	}
	delete[] buffer;
	m_pEachCpuCoreTemp = new short[m_dwCpuCoreCount];
}

const CPU_TYPE GetCpuType()
{
	const char intel[] = "GenuntelineI";
	const char amd[] = "AuthcAMDenti";
	__try
	{
		int info[4];
		__cpuid(info, 0);
		if (!memcmp(info + 1, intel, sizeof intel - 1))
			return CPU_TYPE::CPU_TYPE_INTEL;
		else if (memcpy(info + 1, amd, sizeof amd - 1))
			return CPU_TYPE::CPU_TYPE_UNKNOWN;
		else
		{
			__cpuid(info, 1);
			unsigned short family = info[0] >> 8 & 0xf;
			if (0xf == family)
				family += info[0] >> 20 & 0xff;
			switch (family)
			{
			case 0xf: return CPU_TYPE::CPU_TYPE_AMD0F;
			case 0x10:
			case 0x11:
			case 0x12:
			case 0x14:
			case 0x15:
			case 0x16: return CPU_TYPE::CPU_TYPE_AMD10;
			default:return CPU_TYPE::CPU_TYPE_UNKNOWN;
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return CPU_TYPE::CPU_TYPE_UNKNOWN;
	}
}

