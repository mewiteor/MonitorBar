#pragma once
#include "Temperature.h"
class CAMDTemperature :
	public CTemperature
{
public:
	CAMDTemperature();
	virtual ~CAMDTemperature();
protected:
	STATUS_CODE _ReadPciConfig(unsigned long, unsigned char,unsigned int*);
	STATUS_CODE _WritePciConfig(unsigned long, unsigned char, unsigned int);
	STATUS_CODE _GetPciAddress(DWORD, unsigned char, unsigned short,unsigned int*);
private:
	const DWORD dwIOCTL_READ_PCI_CONFIG,
		dwIOCTL_WRITE_PCI_CONFIG;
	const unsigned char cPCI_BUS,
		cPCI_BASE_DEVICE,
		cDEVICE_VENDOR_ID_REGISTER;
	const unsigned short sAMD_VENDOR_ID;
};

