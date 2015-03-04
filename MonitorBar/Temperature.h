#pragma once
#include<Windows.h>

enum CPU_TYPE
{
	CPU_TYPE_INTEL,
	CPU_TYPE_AMD0F,
	CPU_TYPE_AMD10,
	CPU_TYPE_UNKNOWN
};

enum STATUS_CODE
{
	STATUS_CODE_NO_ERROR,
	STATUS_CODE_DRIVER_NOT_LOADED,
	STATUS_CODE_CPUID_LOW,
	STATUS_CODE_UNSUPPORT,
	STATUS_CODE_INVALID_ARG,
	STATUS_CODE_ALIGNMENT_CHECK_ERROR,
	STATUS_CODE_UNKNOWN_ERROR
};

class CTemperature
{
public:
	CTemperature( );
	virtual ~CTemperature();
	virtual STATUS_CODE Init( );
	virtual void Update() = 0;
	short GetValue( )const{ return m_sCur; }
	virtual double GetPercent()const { return 0; }
	void Reset( ){ m_sCur = 0; }
	const DWORD GetCpuCoreCount()const{ return m_dwCpuCoreCount; }
	const short GetCpuCoreTemp(DWORD i)const
	{ return i < m_dwCpuCoreCount ? m_pEachCpuCoreTemp[i] : SHRT_MIN; }
protected:
	const DWORD _GetIoctlType()const{ return dwIOCTL_TYPE; }
	const HANDLE _GetDriverHandle( )const{ return m_hDriver; }
	const bool _GetIsMsr( )const{ return m_bIsMsr; }
	const bool _GetIsNT()const{ return m_bIsNT; }
	const DWORD _GetCpuCount()const{ return m_dwCpuCount; }
	const unsigned short _GetFamily()const{ return m_sFamily; }
	const unsigned short _GetModel()const{ return m_sModel; }
	const unsigned short _GetStepping()const{ return m_sStepping; }
	const KAFFINITY _GetMask(DWORD dw)const
	{ return dw < m_dwCpuCoreCount ? m_pMasks[dw] : 0; }
private:
	const bool __IsNT()const;
	const bool __IsCpuid()const;
	const bool __IsMsr()const;
	STATUS_CODE __Init();
	void __InitProcessorsInfo();
protected:
	short m_sCur;
	short* m_pEachCpuCoreTemp;
private:
	bool m_bIsNT;
	bool m_bIsCpuid;
	bool m_bIsMsr;
	HANDLE m_hDriver;
	const DWORD dwIOCTL_TYPE;
	unsigned short m_sFamily, m_sModel, m_sStepping;
	DWORD m_dwCpuCoreCount,m_dwCpuCount;
	KAFFINITY* m_pMasks;
};



const CPU_TYPE GetCpuType( );