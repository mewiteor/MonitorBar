#include<Windows.h>
#include "AMDTemperature.h"
#include"Log.h"

CAMDTemperature::CAMDTemperature()
: dwIOCTL_READ_PCI_CONFIG(CTL_CODE(_GetIoctlType(), 0x851, METHOD_BUFFERED, FILE_READ_ACCESS))
, dwIOCTL_WRITE_PCI_CONFIG(CTL_CODE(_GetIoctlType(), 0x852, METHOD_BUFFERED, FILE_WRITE_ACCESS))
, cPCI_BUS(0)
, cPCI_BASE_DEVICE(0x18)
, cDEVICE_VENDOR_ID_REGISTER(0)
, sAMD_VENDOR_ID(0x1022)
{
	LOGOUT("CAMDTemperature¹¹Ôì");
}


CAMDTemperature::~CAMDTemperature( )
{
	LOGOUT("CAMDTemperatureÎö¹¹");
}

STATUS_CODE CAMDTemperature::_ReadPciConfig(unsigned long pciAddress,
											 unsigned char  regAddress,
											 unsigned int* ret)
{
	if (!ret)
		return STATUS_CODE::STATUS_CODE_INVALID_ARG;
	// alignment check
	if (( regAddress & sizeof(ret)-1 ) != 0)
		return STATUS_CODE::STATUS_CODE_ALIGNMENT_CHECK_ERROR;

	DWORD	returnedLength = 0;
	BOOL	result = FALSE;

	unsigned long inBuf[] = { pciAddress, regAddress };

	result = DeviceIoControl(
		_GetDriverHandle(),
		dwIOCTL_READ_PCI_CONFIG,
		&inBuf,
		sizeof( inBuf ),
		&ret,
		sizeof ret,
		&returnedLength,
		NULL
		);

	if (!result)
		return STATUS_CODE::STATUS_CODE_UNKNOWN_ERROR;
	return STATUS_CODE::STATUS_CODE_NO_ERROR;
}

STATUS_CODE CAMDTemperature::_WritePciConfig(unsigned long pciAddress,
									  unsigned char regAddress,
									  unsigned int value)
{
	DWORD	returnedLength = 0;
	BOOL	result = FALSE;
	struct
	{
		unsigned long PciAddress;
		unsigned long PciOffset;
		unsigned int Data;
	}inBuf;

	// alignment check
	if (( regAddress & sizeof(value)-1 ) != 0)
		return STATUS_CODE::STATUS_CODE_ALIGNMENT_CHECK_ERROR;

	inBuf.Data=value;
	inBuf.PciAddress = pciAddress;
	inBuf.PciOffset = regAddress;
	result = DeviceIoControl(
		_GetDriverHandle(),
		dwIOCTL_WRITE_PCI_CONFIG,
		&inBuf,
		sizeof inBuf,
		NULL,
		0,
		&returnedLength,
		NULL
		);

	if (!result)
		return STATUS_CODE::STATUS_CODE_UNKNOWN_ERROR;
	return STATUS_CODE::STATUS_CODE_NO_ERROR;
}

STATUS_CODE CAMDTemperature::_GetPciAddress(DWORD dwProcessorIndex,
											 unsigned char function,
											 unsigned short deviceId,
											 unsigned int* ret)
{
	if (!ret)
		return STATUS_CODE::STATUS_CODE_INVALID_ARG;
	// assemble the pci address
	*ret =
		( cPCI_BUS & 0xff ) << 8 |
		( cPCI_BASE_DEVICE + dwProcessorIndex & 0x1f ) << 3 |
		function & 7;

	// verify that we have the correct bus, device and function
	unsigned int deviceVendor;
	STATUS_CODE sc = _ReadPciConfig(*ret, cDEVICE_VENDOR_ID_REGISTER, &deviceVendor);
	if (STATUS_CODE::STATUS_CODE_NO_ERROR != sc)
		return sc;

	if (deviceVendor != ( deviceId << 16 | sAMD_VENDOR_ID ))
		return STATUS_CODE::STATUS_CODE_UNKNOWN_ERROR;
	return STATUS_CODE::STATUS_CODE_NO_ERROR;
}