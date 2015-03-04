#pragma once
#include<Windows.h>
#include<ShlObj.h>
#include"IMonitor.h"

class CDeskBand :
	public IDeskBand2,
	public IPersistStream,
	public IObjectWithSite,
	public IInputObject
{
public:
	// IUnknown
	STDMETHODIMP QueryInterface(REFIID, void**);
	STDMETHODIMP_(ULONG) AddRef( );
	STDMETHODIMP_(ULONG) Release( );

	// IOleWindow
	STDMETHODIMP GetWindow(HWND*);
	STDMETHODIMP ContextSensitiveHelp(BOOL){return E_NOTIMPL;}

	// IDockingWindow
	STDMETHODIMP ShowDW(BOOL);
	STDMETHODIMP CloseDW(DWORD);
	STDMETHODIMP ResizeBorderDW(const RECT*, IUnknown*, BOOL){ return E_NOTIMPL; }

	// IDeskBand (needed for all deskbands)
	STDMETHODIMP GetBandInfo(DWORD dwBandID, DWORD, DESKBANDINFO*);

	// IDeskBand2 (needed for glass deskband)
	STDMETHODIMP CanRenderComposited(BOOL*);
	STDMETHODIMP SetCompositionState(BOOL);
	STDMETHODIMP GetCompositionState(BOOL*);

	// IPersist
	STDMETHODIMP GetClassID(CLSID*);

	// IPersistStream
	STDMETHODIMP IsDirty( );
	STDMETHODIMP Load(IStream*){return S_OK;}
	STDMETHODIMP Save(IStream*, BOOL);
	STDMETHODIMP GetSizeMax(ULARGE_INTEGER*){ return E_NOTIMPL; }

	// IObjectWithSite
	STDMETHODIMP SetSite(IUnknown*);
	STDMETHODIMP GetSite(REFIID, void**);

	// IInputObject
	STDMETHODIMP UIActivateIO(BOOL, MSG*);
	STDMETHODIMP HasFocusIO( );
	STDMETHODIMP TranslateAcceleratorIO(MSG*){ return S_FALSE; }

	CDeskBand( );
protected:
	~CDeskBand( );
private:
	void __Init(size_t);
	static LRESULT CALLBACK __WndProc(HWND, UINT, WPARAM, LPARAM);
	LRESULT __OnCreate( HWND);
	LRESULT __OnPaint(HWND, HDC = nullptr);
	LRESULT __OnFocus( BOOL);
	LRESULT __OnDestroy( HWND);
	LRESULT __OnTimer(HWND);
	LRESULT __OnRButtonUp(HWND);
	LRESULT __OnMenuReset();
	LRESULT __OnMenuClose(HWND);
	const std::wstring __ChangeString(const std::wstring&)const;
private:
	ULONG				m_lRef;
	HWND				m_hWnd;
	HWND				m_hWndParent;
	DWORD				m_dwBandID;
	BOOL				m_bCanCompositionEnabled;
	BOOL				m_bIsDirty;
	IInputObjectSite*	m_pSite;
	static LPCTSTR		sm_lpszClassName;
	BOOL				m_bHasFocus;
	HFONT				m_hFont;
	IMonitor*			m_iMonitors[3];
	bool				m_bIsRegisterClassed;
	HWND				m_hToolTip;
	const UINT_PTR		nTIMER_ID;
	HMENU				m_hMenu;
};

