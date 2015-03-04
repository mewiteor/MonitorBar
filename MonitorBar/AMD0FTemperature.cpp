#include<Windows.h>
#include "AMD0FTemperature.h"
#include<intrin.h>
#include"Log.h"

CAMD0FTemperature::CAMD0FTemperature( )
: m_cOffset(0)
, m_cThermSenseCoreSelCPU0(0)
, m_cThermSenseCoreSelCPU1(0)
, m_pMiscellaneousControlAddress(nullptr)
, cMISCELLANEOUS_CONTROL_FUNCTION(3)
, sMISCELLANEOUS_CONTROL_DEVICE_ID(0x1103)
, nTHERMTRIP_STATUS_REGISTER(0xE4)
{
	LOGOUT("CAMD0FTemperature¹¹Ôì");
}

CAMD0FTemperature::~CAMD0FTemperature( )
{
	if (m_pMiscellaneousControlAddress)
	{
		delete[] m_pMiscellaneousControlAddress;
		m_pMiscellaneousControlAddress = nullptr;
	}
	LOGOUT("CAMD0FTemperatureÎö¹¹");
}

STATUS_CODE CAMD0FTemperature::Init( )
{
	STATUS_CODE sc = CTemperature::Init( );
	if (STATUS_CODE::STATUS_CODE_NO_ERROR != sc)
		return sc;
	m_cOffset = -49;
	if (_GetModel( ) >= 0x69 &&
		_GetModel( ) != 0xc1 &&
		_GetModel( ) != 0x6c &&
		_GetModel( ) != 0x7c)
		m_cOffset += 21;
	if (_GetModel( ) < 40)
	{
		m_cThermSenseCoreSelCPU0 = 0;
		m_cThermSenseCoreSelCPU1 = 4;
	}
	else
	{
		m_cThermSenseCoreSelCPU0 = 4;
		m_cThermSenseCoreSelCPU1 = 0;
	}
	int info[4];
	__cpuid(info, 0x80000000);
	if (info[0] <= 7)
		return STATUS_CODE::STATUS_CODE_UNSUPPORT;
	__cpuid(info, 0x80000007);
	if (!( info[3] & 1 ))
		return STATUS_CODE::STATUS_CODE_UNSUPPORT;
	m_pMiscellaneousControlAddress = new unsigned int[GetCpuCoreCount( )]( );
	for (DWORD i = 0; i < GetCpuCoreCount( ); ++i)
	{
		if (STATUS_CODE::STATUS_CODE_NO_ERROR !=
			_GetPciAddress(
			i,
			cMISCELLANEOUS_CONTROL_FUNCTION,
			sMISCELLANEOUS_CONTROL_DEVICE_ID, m_pMiscellaneousControlAddress + i))
			m_pMiscellaneousControlAddress[i] = 0;
	}
	return STATUS_CODE::STATUS_CODE_NO_ERROR;
}

void CAMD0FTemperature::Update( )
{
	int all=0, count = 0;
	for (size_t i = 0; i < GetCpuCoreCount( ); ++i)
	{
		unsigned int val;
		STATUS_CODE sc = _WritePciConfig(m_pMiscellaneousControlAddress[i],
							 nTHERMTRIP_STATUS_REGISTER,
							 i > 0 ? m_cThermSenseCoreSelCPU1 : m_cThermSenseCoreSelCPU0);
		if (STATUS_CODE::STATUS_CODE_NO_ERROR == sc)
			sc = _ReadPciConfig(m_pMiscellaneousControlAddress[i], nTHERMTRIP_STATUS_REGISTER, &val);
		if (STATUS_CODE::STATUS_CODE_NO_ERROR == sc)
		{
			short temp = ( val >> 16 & 0xff ) + m_cOffset;
			if (m_pEachCpuCoreTemp)
				m_pEachCpuCoreTemp[i] = temp;
			all += temp;
			++count;
		}
		if (STATUS_CODE::STATUS_CODE_NO_ERROR != sc&&m_pEachCpuCoreTemp)
			m_pEachCpuCoreTemp[i] = SHRT_MIN;
	}
	if (count)
		m_sCur = short(all / count);
}
