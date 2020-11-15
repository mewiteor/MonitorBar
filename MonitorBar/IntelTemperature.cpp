#include<Windows.h>
#include "IntelTemperature.h"
#include<intrin.h>
#include"Log.h"

CIntelTemperature::CIntelTemperature( )
: m_pTjMax(nullptr)
, dwIOCTL_READ_MSR(CTL_CODE(_GetIoctlType( ), 0x821, METHOD_BUFFERED, FILE_ANY_ACCESS))
, m_nAllTemp(0)
, m_dwNumOfAvailableProcesses(0)
, nIA32_THERM_STATUS_MSR(0x019C)
, nIA32_TEMPERATURE_TARGET(0x01A2)
{
	LOGOUT("CIntelTemperature¹¹Ôì");
}

CIntelTemperature::~CIntelTemperature( )
{
	if (m_pTjMax)
	{
		delete[] m_pTjMax;
		m_pTjMax = nullptr;
	}
	LOGOUT("CIntelTemperatureÎö¹¹");
}

STATUS_CODE CIntelTemperature::Init( )
{
	STATUS_CODE sc = CTemperature::Init();
	if (STATUS_CODE::STATUS_CODE_NO_ERROR == sc)
		sc = __Init( );
	return sc;
}

STATUS_CODE CIntelTemperature::__Init( )
{
	int info[4];
	__try
	{
		__cpuid(info, 0);
		if (info[0] < 6)
			return STATUS_CODE::STATUS_CODE_CPUID_LOW;
		__cpuid(info, 6);
		if (!( info[0] & 1 ))
			return STATUS_CODE::STATUS_CODE_UNSUPPORT;
		switch (_GetFamily( ))
		{
		case 0x06: {
			switch (_GetModel( )) {
			case 0x0F: // Intel Core 2 (65nm)
				switch (_GetStepping( )) {
				case 0x06: // B2
					switch (_GetCpuCount( )) {
					case 2: __InitTjMax(80 + 10); break;
					case 4: __InitTjMax(90 + 10); break;
					default: __InitTjMax(85 + 10); break;
					}
				case 0x0B: // G0
					__InitTjMax(90 + 10); break;
				case 0x0D: // M0
					__InitTjMax(85 + 10); break;
				default:
					__InitTjMax(85 + 10); break;
				} break;
			case 0x17: // Intel Core 2 (45nm)
				__InitTjMax(100); break;
			case 0x1C: // Intel Atom (45nm)
				switch (_GetStepping( )) {
				case 0x02: // C0
					__InitTjMax(90); break;
				case 0x0A: // A0, B0
					__InitTjMax(100); break;
				default:
					__InitTjMax(90); break;
				} break;
			case 0x1A: // Intel Core i7 LGA1366 (45nm)
			case 0x1E: // Intel Core i5, i7 LGA1156 (45nm)
			case 0x1F: // Intel Core i5, i7 
			case 0x25: // Intel Core i3, i5, i7 LGA1156 (32nm)
			case 0x2C: // Intel Core i7 LGA1366 (32nm) 6 Core
			case 0x2E: // Intel Xeon Processor 7500 series (45nm)
			case 0x2F: // Intel Xeon Processor (32nm)
				__InitTjMaxFromMsr( );
				break;
			case 0x2A: // Intel Core i5, i7 2xxx LGA1155 (32nm)
			case 0x2D: // Next Generation Intel Xeon, i7 3xxx LGA2011 (32nm)
				__InitTjMaxFromMsr( );
				break;
			case 0x3A: // Intel Core i5, i7 3xxx LGA1155 (22nm)
			case 0x3E: // Intel Core i7 4xxx LGA2011 (22nm)
				__InitTjMaxFromMsr( );
				break;
			case 0x3C: // Intel Core i5, i7 4xxx LGA1150 (22nm)              
			case 0x3F: // Intel Xeon E5-2600/1600 v3, Core i7-59xx
				// LGA2011-v3, Haswell-E (22nm)
			case 0x45: // Intel Core i5, i7 4xxxU (22nm)
			case 0x46:
				__InitTjMaxFromMsr( );
				break;
			case 0x3D: // Intel Core M-5xxx (14nm)
				__InitTjMaxFromMsr( );
				break;
			case 0x36: // Intel Atom S1xxx, D2xxx, N2xxx (32nm)
				__InitTjMaxFromMsr( );
				break;
			case 0x37: // Intel Atom E3xxx, Z3xxx (22nm)
			case 0x4A:
			case 0x4D: // Intel Atom C2xxx (22nm)
			case 0x5A:
			case 0x5D:
				__InitTjMaxFromMsr( );
				break;
			default:
				__InitTjMax(100);
				break;
			}
		} break;
		case 0x0F: {
			switch (_GetModel( )) {
			case 0x00: // Pentium 4 (180nm)
			case 0x01: // Pentium 4 (130nm)
			case 0x02: // Pentium 4 (130nm)
			case 0x03: // Pentium 4, Celeron D (90nm)
			case 0x04: // Pentium 4, Pentium D, Celeron D (90nm)
			case 0x06: // Pentium 4, Pentium D, Celeron D (65nm)
				__InitTjMax(100);
				break;
			default:
				__InitTjMax(100);
				break;
			}
		} break;
		default:
			__InitTjMax(100);
			break;
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return STATUS_CODE::STATUS_CODE_UNKNOWN_ERROR;
	}
	return STATUS_CODE::STATUS_CODE_NO_ERROR;
}

STATUS_CODE CIntelTemperature::__ReadMsr(unsigned long index, KAFFINITY mask, unsigned long long* ret)const
{
	if (_GetDriverHandle( ) == INVALID_HANDLE_VALUE)
		return STATUS_CODE::STATUS_CODE_UNKNOWN_ERROR;
	if (!_GetIsMsr( ))
		return STATUS_CODE::STATUS_CODE_UNKNOWN_ERROR;
	if (!ret)
		return STATUS_CODE::STATUS_CODE_INVALID_ARG;
	DWORD				returnedLength = 0;
	BOOL				result = FALSE;
	HANDLE hThread = nullptr;
	DWORD_PTR oldMask;
	if (_GetIsNT( ))
	{
		hThread = GetCurrentThread( );
		oldMask = SetThreadAffinityMask(hThread, mask);
		if (!oldMask)
			return STATUS_CODE::STATUS_CODE_UNKNOWN_ERROR;
	}
	result = DeviceIoControl(
		_GetDriverHandle( ),
		dwIOCTL_READ_MSR,
		&index,
		sizeof index,
		ret,
		sizeof *ret,
		&returnedLength,
		nullptr
		);
	if (_GetIsNT( ))
		SetThreadAffinityMask(hThread, oldMask);
	if (!result)
		return STATUS_CODE::STATUS_CODE_UNKNOWN_ERROR;
	return STATUS_CODE::STATUS_CODE_NO_ERROR;
}

void CIntelTemperature::Update( )
{
	m_nAllTemp = 0;
	m_dwNumOfAvailableProcesses = 0;
	byte negativeTempCount = 0;
	for (DWORD i = 0; i < GetCpuCoreCount( ); ++i)
	{
		unsigned long long val;
		if (STATUS_CODE::STATUS_CODE_NO_ERROR != __ReadMsr(nIA32_THERM_STATUS_MSR, _GetMask(i), &val))
		{
			if (m_pEachCpuCoreTemp)
				m_pEachCpuCoreTemp[i] = SHRT_MIN;
		}
		else
		{
			short temp = (short)m_pTjMax[i] - (short)( val >> 16 & 0x7f );
			if (m_pEachCpuCoreTemp)
				m_pEachCpuCoreTemp[i] = temp;
			if (temp <= 0)
				++negativeTempCount;
			else
				m_nAllTemp += temp;
			++m_dwNumOfAvailableProcesses;
		}
	}
	if (m_dwNumOfAvailableProcesses)
		m_sCur = short(m_nAllTemp / (m_dwNumOfAvailableProcesses - negativeTempCount));
}

double CIntelTemperature::GetPercent( )const
{
	if (!m_pTjMax)
		return 0;
	int all = 0, n = 0;
	for (DWORD i = 0; i < GetCpuCoreCount( ); ++i)
	{
		if (m_pTjMax[i])
		{
			all += m_pTjMax[i];
			++n;
		}
	}
	return 100.0 - (double)( all * m_dwNumOfAvailableProcesses - m_nAllTemp *n ) / ( n*m_dwNumOfAvailableProcesses );
}

void CIntelTemperature::__InitTjMax(BYTE tjMax)
{
	if (m_pTjMax)
		delete[] m_pTjMax;
	m_pTjMax = new BYTE[GetCpuCoreCount( )];
	for (size_t i = 0; i < 4; ++i)
		m_pTjMax[i] = tjMax;
}

void CIntelTemperature::__InitTjMaxFromMsr( )
{
	if (m_pTjMax)
		delete[] m_pTjMax;
	m_pTjMax = new BYTE[GetCpuCoreCount( )]( );
	if (m_pTjMax)
		for (DWORD i = 0; i < GetCpuCoreCount( ); ++i)
		{
			unsigned long long val;
			if (STATUS_CODE::STATUS_CODE_NO_ERROR != __ReadMsr(nIA32_TEMPERATURE_TARGET, _GetMask(i), &val))
				m_pTjMax[i] = 0;
			else
				m_pTjMax[i] = val >> 16 & 0xff;
		}
}
