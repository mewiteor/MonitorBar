#include "AMD10Temperature.h"
#include"Log.h"

CAMD10Temperature::CAMD10Temperature( )
: m_sMiscellaneousControlDeviceId(0)
, m_sFAMILY_10H_MISCELLANEOUS_CONTROL_DEVICE_ID(0x1203)
, m_sFAMILY_11H_MISCELLANEOUS_CONTROL_DEVICE_ID(0x1303)
, m_sFAMILY_12H_MISCELLANEOUS_CONTROL_DEVICE_ID(0x1703)
, m_sFAMILY_14H_MISCELLANEOUS_CONTROL_DEVICE_ID(0x1703)
, m_sFAMILY_15H_MODEL_00_MISC_CONTROL_DEVICE_ID(0x1603)
, m_sFAMILY_15H_MODEL_10_MISC_CONTROL_DEVICE_ID(0x1403)
, m_sFAMILY_16H_MODEL_00_MISC_CONTROL_DEVICE_ID(0x1533)
, m_pMiscellaneousControlAddress(nullptr)
, cMISCELLANEOUS_CONTROL_FUNCTION(3)
, nREPORTED_TEMPERATURE_CONTROL_REGISTER(0xA4)
{
	LOGOUT("CAMD10Temperature¹¹Ôì");
}


CAMD10Temperature::~CAMD10Temperature( )
{
	if (m_pMiscellaneousControlAddress)
	{
		delete[] m_pMiscellaneousControlAddress;
		m_pMiscellaneousControlAddress = nullptr;
	}
	LOGOUT("CAMD10TemperatureÎö¹¹");
}

STATUS_CODE CAMD10Temperature::Init()
{
	STATUS_CODE sc = CTemperature::Init();
	if (STATUS_CODE::STATUS_CODE_NO_ERROR != sc)
		return sc;
	switch (_GetFamily())
	{
	case 0x10: m_sMiscellaneousControlDeviceId =
		m_sFAMILY_10H_MISCELLANEOUS_CONTROL_DEVICE_ID; break;
	case 0x11: m_sMiscellaneousControlDeviceId =
		m_sFAMILY_11H_MISCELLANEOUS_CONTROL_DEVICE_ID; break;
	case 0x12: m_sMiscellaneousControlDeviceId =
		m_sFAMILY_12H_MISCELLANEOUS_CONTROL_DEVICE_ID; break;
	case 0x14: m_sMiscellaneousControlDeviceId =
		m_sFAMILY_14H_MISCELLANEOUS_CONTROL_DEVICE_ID; break;
	case 0x15:
		switch (_GetModel() & 0xF0) {
		case 0x00: m_sMiscellaneousControlDeviceId =
			m_sFAMILY_15H_MODEL_00_MISC_CONTROL_DEVICE_ID; break;
		case 0x10: m_sMiscellaneousControlDeviceId =
			m_sFAMILY_15H_MODEL_10_MISC_CONTROL_DEVICE_ID; break;
		default: m_sMiscellaneousControlDeviceId = 0; break;
		} break;
	case 0x16:
		switch (_GetModel() & 0xF0) {
		case 0x00: m_sMiscellaneousControlDeviceId =
			m_sFAMILY_16H_MODEL_00_MISC_CONTROL_DEVICE_ID; break;
		default: m_sMiscellaneousControlDeviceId = 0; break;
		} break;
	default: m_sMiscellaneousControlDeviceId = 0; break;
	}
	m_pMiscellaneousControlAddress = new unsigned int[GetCpuCoreCount()]();
	if (m_pMiscellaneousControlAddress)
		for (DWORD i = 0; i < GetCpuCoreCount(); ++i)
		{
			if (STATUS_CODE::STATUS_CODE_NO_ERROR !=
				_GetPciAddress(
				i,
				cMISCELLANEOUS_CONTROL_FUNCTION,
				m_sMiscellaneousControlDeviceId, m_pMiscellaneousControlAddress + i))
				m_pMiscellaneousControlAddress[i] = 0;
		}
	return STATUS_CODE::STATUS_CODE_NO_ERROR;
}

void CAMD10Temperature::Update()
{
	float all = 0;
	int count = 0;
	if (m_pMiscellaneousControlAddress)
		for (size_t i = 0; i < GetCpuCoreCount( ); ++i)
		{
			unsigned int val;
			if (STATUS_CODE::STATUS_CODE_NO_ERROR !=
				_ReadPciConfig(m_pMiscellaneousControlAddress[i],
				nREPORTED_TEMPERATURE_CONTROL_REGISTER, &val))
			{
				if (m_pEachCpuCoreTemp)
					m_pEachCpuCoreTemp[i] = SHRT_MIN;
			}
			else
			{
				float temp;
				if (_GetFamily( ) == 0x15 && ( val & 0x30000 ) == 0x30000) {
					if (_GetModel( ) & 0xF0)
						temp = ( val >> 21 & 0x7FF ) / 8.0f - 49;
					else
						temp = ( val >> 21 & 0x7FC ) / 8.0f - 49;
				}
				else if (_GetFamily( ) == 0x16 &&
						 ( ( val & 0x30000 ) == 0x30000 || ( val & 0x80000 ) == 0x80000 ))
						 temp = ( val >> 21 & 0x7FF ) / 8.0f - 49;
				else
					temp = ( ( val >> 21 ) & 0x7FF ) / 8.0f;
				if (m_pEachCpuCoreTemp)
					m_pEachCpuCoreTemp[i] = short(temp);
				all += temp;
				++count;
			}
		}
	if (count)
		m_sCur = short(all / count);
}