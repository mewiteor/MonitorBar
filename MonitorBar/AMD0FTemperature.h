#pragma once
#include "AMDTemperature.h"
class CAMD0FTemperature :
	public CAMDTemperature
{
public:
	CAMD0FTemperature( );
	~CAMD0FTemperature();
	STATUS_CODE Init( );
	void Update();
private:
	char m_cOffset;
	char m_cThermSenseCoreSelCPU0, m_cThermSenseCoreSelCPU1;
	unsigned int* m_pMiscellaneousControlAddress;
	const unsigned char cMISCELLANEOUS_CONTROL_FUNCTION;
	const unsigned short sMISCELLANEOUS_CONTROL_DEVICE_ID;
	const unsigned int nTHERMTRIP_STATUS_REGISTER;
};

