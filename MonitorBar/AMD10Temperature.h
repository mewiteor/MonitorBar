#pragma once
#include "AMDTemperature.h"
class CAMD10Temperature :
	public CAMDTemperature
{
public:
	CAMD10Temperature();
	~CAMD10Temperature();
	STATUS_CODE Init( );
	void Update();
private:
	unsigned short m_sMiscellaneousControlDeviceId,
		m_sFAMILY_10H_MISCELLANEOUS_CONTROL_DEVICE_ID,
		m_sFAMILY_11H_MISCELLANEOUS_CONTROL_DEVICE_ID,
		m_sFAMILY_12H_MISCELLANEOUS_CONTROL_DEVICE_ID,
		m_sFAMILY_14H_MISCELLANEOUS_CONTROL_DEVICE_ID,
		m_sFAMILY_15H_MODEL_00_MISC_CONTROL_DEVICE_ID,
		m_sFAMILY_15H_MODEL_10_MISC_CONTROL_DEVICE_ID,
		m_sFAMILY_16H_MODEL_00_MISC_CONTROL_DEVICE_ID;
	unsigned int* m_pMiscellaneousControlAddress;
	const unsigned char cMISCELLANEOUS_CONTROL_FUNCTION;
	unsigned int nREPORTED_TEMPERATURE_CONTROL_REGISTER;
};

