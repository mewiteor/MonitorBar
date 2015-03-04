#include "ClassFactory.h"
#include<Shlwapi.h>

#include"DeskBand.h"
#include<cassert>
#include"Log.h"

extern ULONG g_lDllRef;

CClassFactory::CClassFactory( )
:m_lRef(1)
{
	InterlockedIncrement(&g_lDllRef);
#ifdef _DEBUG
	char str[64];
	sprintf_s(str, "g_lDllRef=%lu,CClassFactory¹¹Ôì", g_lDllRef);
	LOGOUT(str);
#endif
}


CClassFactory::~CClassFactory( )
{
	InterlockedDecrement(&g_lDllRef);
#ifdef _DEBUG
	char str[64];
	sprintf_s(str, "g_lDllRef=%lu,CClassFactoryÎö¹¹", g_lDllRef);
	LOGOUT(str);
#endif
}

STDMETHODIMP CClassFactory::QueryInterface(REFIID riid, void **ppv)
{
	QITAB qitab[] =
	{
		QITABENT(CClassFactory, IClassFactory),
		{ 0 },
	};
	return QISearch(this, qitab, riid, ppv);
}

STDMETHODIMP_(ULONG) CClassFactory::AddRef( )
{
	return InterlockedIncrement(&m_lRef);
}

STDMETHODIMP_(ULONG) CClassFactory::Release( )
{
	auto l = InterlockedDecrement(&m_lRef);
	if (!l)delete this;
	return l;
}

STDMETHODIMP CClassFactory::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppv)
{
	if (pUnkOuter)return CLASS_E_NOAGGREGATION;
	CDeskBand *pDeskBand = new CDeskBand;
	if (!pDeskBand)return E_OUTOFMEMORY;
	HRESULT hr = pDeskBand->QueryInterface(riid, ppv);
	pDeskBand->Release( );
	return hr;
}

STDMETHODIMP CClassFactory::LockServer(BOOL fLock)
{
	if (fLock) InterlockedIncrement(&g_lDllRef);
	else InterlockedDecrement(&g_lDllRef);
	return S_OK;
}