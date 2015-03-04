#pragma once
class CWin
{
public:
	CWin( HINSTANCE,int);
	~CWin( );
	int MessageLoop();
private:
	template<typename T>
	void SafeRelease(T* &p)
	{
		if (p)
		{
			p->Release();
			p = nullptr;
		}
	}
	static DWORD Err(HWND,LPSTR, DWORD, LPSTR,DWORD);
private:
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
	bool OnCreate(HWND,HINSTANCE);
	bool OnDestroy();
	bool OnPaint(HWND);
	bool OnLButtonUp(HWND,HINSTANCE,WORD,WORD);

	bool IsPointInRect(WORD, WORD, RECT*);
	void DoPaint(HDC,RECT*);
	void RefreshSwich(HWND,HINSTANCE );
	void PaintSwitch(HDC, long, long, bool);
	HRESULT IsRegister(HWND);
	HRESULT IsSystemInstallDriver(HWND);
	void RegisterSwitch(HWND );
	void InstallSystemDriverSwitch(HWND, HINSTANCE);

	bool InitDriver( HINSTANCE);
	bool InstallDriver(HWND, SC_HANDLE, LPCTSTR);
	bool RemoveDriver(HWND, SC_HANDLE, LPCTSTR);
	bool StartDriver(HWND, SC_HANDLE, LPCTSTR);
	bool StopDriver(HWND, SC_HANDLE, LPCTSTR);
	bool SystemInstallDriver(HWND, SC_HANDLE, LPCTSTR);
	bool IsSystemInstallDriver(HWND, SC_HANDLE, LPCTSTR);

	bool IsX64( );
	bool IsWow64( );
private:
	LPCTSTR m_lpClassName, m_lpWindowTitle;
	HWND m_hRegister, m_hUnregister, m_hInstallSystemDriver, m_hUninstallSystemDriver;
	TCHAR m_szDriverPath[MAX_PATH];
	HFONT m_hFont;
	bool m_fSystemInstallDriver;
	bool m_fRegister;
	HBRUSH m_hBlueBrush;
	HBRUSH m_hGrayBrush;
	LPCTSTR m_lpRegisterStatus,
		m_lpRegistered,
		m_lpUnregistered,
		m_lpSystemInstallDriverStatus,
		m_lpSystemInstalledDriver,
		m_lpSystemUninstalledDriver;
	int m_nRegisterStatus,
		m_nRegistered,
		m_nUnregistered,
		m_nSystemInstallDriverStatus,
		m_nSystemInstalledDriver,
		m_nSystemUninstalledDriver;
	RECT m_rcRegister,m_rcSystemInstallDriver;
};

