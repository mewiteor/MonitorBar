#include<Windows.h>
#include<d2d1.h>
#include "Win.h"
#include<tchar.h>
#include<sstream>
#include<iomanip>
#include<fstream>

#pragma comment(lib,"Comctl32.lib")

#define Ehd(h,f,d) Err((h),__FUNCTION__,__LINE__,(f),(d))
#define Ef(h,f) Ehd((h),(f),GetLastError())
#define Ed(f,d) Ehd(nullptr,(f),(d))
#define E(f) Ed((f),GetLastError())
#define CS(f) if(f)return 0;break

#define LOG(msg) Log(__FUNCTION__,__LINE__,(msg))
#define LOGERR(f) LogErr(__FUNCTION__,__LINE__,(f),GetLastError())
#define LOGERRD(f,d) LogErr(__FUNCTION__,__LINE__,(f),(d))
void Log(const char*function, size_t line, const std::string& msg);
void LogErr(const char*function, size_t line, const char*f, DWORD dw);

CWin::CWin(HINSTANCE hInstance, int nCmdShow)
: m_lpClassName(_T("ManageDriverAndDeskBand"))
, m_lpWindowTitle(_T("监视栏管理器"))
, m_hRegister(nullptr)
, m_hUnregister(nullptr)
, m_hInstallSystemDriver(nullptr)
, m_hUninstallSystemDriver(nullptr)
, m_hFont(nullptr)
, m_fSystemInstallDriver(false)
, m_fRegister(false)
, m_hBlueBrush(nullptr)
, m_hGrayBrush(nullptr)
, m_lpRegisterStatus(_T("MonitorBar注册状态开关"))
, m_lpRegistered(_T("已注册"))
, m_lpUnregistered(_T("未注册"))
, m_lpSystemInstallDriverStatus(_T("系统驱动安装开关"))
, m_lpSystemInstalledDriver(_T("已安装"))
, m_lpSystemUninstalledDriver(_T("未安装"))
, m_nRegisterStatus((int)_tcslen(m_lpRegisterStatus))
, m_nRegistered((int)_tcslen(m_lpRegistered))
, m_nUnregistered((int)_tcslen(m_lpUnregistered))
, m_nSystemInstallDriverStatus((int)_tcslen(m_lpSystemInstallDriverStatus))
, m_nSystemInstalledDriver((int)_tcslen(m_lpSystemInstalledDriver))
, m_nSystemUninstalledDriver((int)_tcslen(m_lpSystemUninstalledDriver))
, m_rcRegister({ 0 })
, m_rcSystemInstallDriver({ 0 })
{
	ZeroMemory(m_szDriverPath, _countof(m_szDriverPath));
	WNDCLASSEX wcex =
	{
		sizeof( WNDCLASSEX ), CS_HREDRAW | CS_VREDRAW,
		CWin::WndProc, 0, 0, hInstance, NULL, LoadCursor(nullptr, IDC_ARROW),
		( HBRUSH )::GetStockObject(WHITE_BRUSH), nullptr, m_lpClassName, nullptr
	};
	if (!RegisterClassEx(&wcex))
		throw E("RegisterClass");
	HWND hWnd = CreateWindow(m_lpClassName, m_lpWindowTitle,
							 WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
							 CW_USEDEFAULT, CW_USEDEFAULT, 230, 150, nullptr,
							 nullptr, hInstance, this);
	if (INVALID_HANDLE_VALUE == hWnd)
		throw E("CreateWindow");
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
}

CWin::~CWin( )
{
}

int CWin::MessageLoop( )
{
	MSG msg = { 0 };
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}

LRESULT CALLBACK CWin::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HINSTANCE hInstance = nullptr;
	static CWin* pWin = nullptr;
	if (WM_NCCREATE == uMsg)
	{
		LPCREATESTRUCT lpCreateStruct = reinterpret_cast<LPCREATESTRUCT>( lParam );
		if (lpCreateStruct)
		{
			pWin = reinterpret_cast<CWin*>( lpCreateStruct->lpCreateParams );
			hInstance = lpCreateStruct->hInstance;
		}
	}
	else
	{
		switch (uMsg)
		{
		case WM_CREATE:CS(pWin->OnCreate(hWnd, hInstance));
		case WM_DESTROY:CS(pWin->OnDestroy( ));
		case WM_PAINT:CS(pWin->OnPaint(hWnd));
		case WM_LBUTTONUP:CS(pWin->OnLButtonUp(hWnd, hInstance, LOWORD(lParam), HIWORD(lParam)));
		}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool CWin::OnCreate(HWND hWnd, HINSTANCE hInstance)
{
	RECT rc;
	GetClientRect(hWnd, &rc);
	m_hFont = CreateFont(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
						 DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
						 DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _T("微软雅黑"));

	m_hBlueBrush = CreateSolidBrush(RGB(0, 100, 249));
	m_hGrayBrush = CreateSolidBrush(RGB(166, 166, 166));
	RefreshSwich(hWnd, hInstance);
	return true;
}

bool CWin::OnDestroy( )
{
	if (m_hFont)
	{
		DeleteObject(m_hFont);
		m_hFont = nullptr;
	}
	if (m_hBlueBrush)
	{
		DeleteObject(m_hBlueBrush);
		m_hBlueBrush = nullptr;
	}
	if (m_hGrayBrush)
	{
		DeleteObject(m_hGrayBrush);
		m_hGrayBrush = nullptr;
	}
	PostQuitMessage(0);
	return true;
}

bool CWin::OnPaint(HWND hWnd)
{
	PAINTSTRUCT paint;
	HDC hdc = BeginPaint(hWnd, &paint);
	DoPaint(hdc, &paint.rcPaint);
	EndPaint(hWnd, &paint);
	return true;
}

bool CWin::OnLButtonUp(HWND hWnd, HINSTANCE hInstance, WORD x, WORD y)
{
	if (m_rcRegister.bottom&&
		IsPointInRect(x, y, &m_rcRegister))
		RegisterSwitch(hWnd);
	else if (m_rcSystemInstallDriver.bottom&&
			 IsPointInRect(x, y, &m_rcSystemInstallDriver))
			 InstallSystemDriverSwitch(hWnd, hInstance);
	else return false;
	HDC hdc = GetDC(hWnd);
	RECT rc;
	GetClientRect(hWnd, &rc);
	DoPaint(hdc, &rc);
	ReleaseDC(hWnd, hdc);
	return true;
}

bool CWin::IsPointInRect(WORD x, WORD y, RECT* rc)
{
	return x >= rc->left&&x <= rc->right&&y >= rc->top&&y <= rc->bottom;
}

void CWin::DoPaint(HDC hdc, RECT* prc)
{
	const long x = ( prc->left + prc->right ) / 2,
		y = ( prc->top + prc->bottom ) / 2;
	SIZE sz;
	RECT rc = { 0 };
	HGLOBAL hOldFont = nullptr;
	if (m_hFont)
		hOldFont = SelectObject(hdc, m_hFont);
	int iOldMode = SetBkMode(hdc, TRANSPARENT);
	FillRect(hdc, prc, (HBRUSH)GetStockObject(WHITE_BRUSH));

	// (x+5,y-25)-(x+55,y-5)
	PaintSwitch(hdc, x + 30, y - 15, m_fRegister);
	if (!m_rcRegister.bottom)
	{
		m_rcRegister.left = x + 5;
		m_rcRegister.top = y - 25;
		m_rcRegister.right = x + 55;
		m_rcRegister.bottom = y - 5;
	}
	GetTextExtentPoint32(hdc, m_lpRegisterStatus, m_nRegisterStatus, &sz);
	rc.bottom = y - 30;
	rc.left = x - sz.cx / 2;
	rc.top = rc.bottom - sz.cy;
	rc.right = rc.left + sz.cx;
	DrawText(hdc, m_lpRegisterStatus, m_nRegisterStatus, &rc, 0);
	if (m_fRegister)
	{
		GetTextExtentPoint32(hdc, m_lpRegistered, m_nRegistered, &sz);
		rc.right = x - 30;
		rc.left = rc.right - sz.cx;
		rc.top = y - 15 - sz.cy / 2;
		rc.bottom = rc.top + sz.cy;
		DrawText(hdc, m_lpRegistered, m_nRegistered, &rc, 0);
	}
	else
	{
		GetTextExtentPoint32(hdc, m_lpUnregistered, m_nUnregistered, &sz);
		rc.right = x - 30;
		rc.left = rc.right - sz.cx;
		rc.top = y - 15 - sz.cy / 2;
		rc.bottom = rc.top + sz.cy;
		DrawText(hdc, m_lpUnregistered, m_nUnregistered, &rc, 0);
	}

	GetTextExtentPoint32(hdc, m_lpSystemInstallDriverStatus, m_nSystemInstallDriverStatus, &sz);
	// (x+5,y + 10 + sz.cy)-(x+55,y + 30 + sz.cy)
	PaintSwitch(hdc, x + 30, y + 20 + sz.cy, m_fSystemInstallDriver);
	if (!m_rcSystemInstallDriver.bottom)
	{
		m_rcSystemInstallDriver.left = x + 5;
		m_rcSystemInstallDriver.top = y + 10 + sz.cy;
		m_rcSystemInstallDriver.right = x + 55;
		m_rcSystemInstallDriver.bottom = y + 30 + sz.cy;
	}
	rc.top = y + 5;
	rc.bottom = rc.top + sz.cy;
	rc.left = x - sz.cx / 2;
	rc.right = rc.left + sz.cx;
	DrawText(hdc, m_lpSystemInstallDriverStatus, m_nSystemInstallDriverStatus, &rc, 0);
	if (m_fSystemInstallDriver)
	{
		GetTextExtentPoint32(hdc, m_lpSystemInstalledDriver, m_nSystemInstalledDriver, &sz);
		rc.right = x - 30;
		rc.left = rc.right - sz.cx;
		rc.top = y + 20 + sz.cy / 2;
		rc.bottom = rc.top + sz.cy;
		DrawText(hdc, m_lpSystemInstalledDriver, m_nSystemInstalledDriver, &rc, 0);
	}
	else
	{
		GetTextExtentPoint32(hdc, m_lpSystemUninstalledDriver, m_nSystemUninstalledDriver, &sz);
		rc.right = x - 30;
		rc.left = rc.right - sz.cx;
		rc.top = y + 20 + sz.cy / 2;
		rc.bottom = rc.top + sz.cy;
		DrawText(hdc, m_lpSystemUninstalledDriver, m_nSystemUninstalledDriver, &rc, 0);
	}
	if (iOldMode)
		SetBkMode(hdc, iOldMode);
	if (m_hFont)
		SelectObject(hdc, hOldFont);
}

void CWin::RefreshSwich(HWND hWnd, HINSTANCE hInstance)
{
	HRESULT hr = IsRegister(hWnd);
	if (hr == S_OK)
		m_fRegister = true;
	else if (hr == S_FALSE)
		m_fRegister = false;

	if (*m_szDriverPath ||
		InitDriver(hInstance))
		IsSystemInstallDriver(hWnd);
}

void CWin::PaintSwitch(HDC hdc, long x, long y, bool b)
{
	HBRUSH hBCur = b ? m_hBlueBrush : m_hGrayBrush;

	RECT rc = { x - 25, y - 10, x + 25, y + 10 };
	FillRect(hdc, &rc, m_hGrayBrush);

	rc.left += 2;
	rc.right -= 2;
	rc.top += 2;
	rc.bottom -= 2;
	FillRect(hdc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));

	++rc.left;
	--rc.right;
	++rc.top;
	--rc.bottom;
	FillRect(hdc, &rc, hBCur);

	if (b)
	{
		rc.left = ( rc.right += 3 ) - 12;
		rc.top -= 3;
		rc.bottom += 3;
	}
	else
	{
		rc.right = ( rc.left -= 3 ) + 12;
		rc.top -= 3;
		rc.bottom += 3;
	}
	FillRect(hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
}

HRESULT CWin::IsRegister(HWND hWnd)
{
	HRESULT hr;
	auto hDll = LoadLibrary(_T("MonitorBar.dll"));
	if (!hDll)
	{
		DWORD dw = GetLastError( );
		if (ERROR_FILE_NOT_FOUND == dw)
			MessageBox(hWnd, _T("找不到MonitorBar.dll\n请将此程序与MonitorBar.dll放在同一文件夹下"), nullptr, MB_ICONERROR);
		else
			Ehd(hWnd, "LoadLibrary", dw);
		return E_FAIL;
	}
	auto func = ( HRESULT(__stdcall *)( ) )GetProcAddress(hDll, "DllIsRegisterServer");
	if (!func)
	{
		Ef(hWnd, "GetProcAddress");
		hr = E_FAIL;
	}
	else
	{
		hr = func( );
		if (FAILED(hr))
			Ehd(hWnd, "DllIsRegisterServer", hr);
	}
	FreeLibrary(hDll);
	return hr;
}

HRESULT CWin::IsSystemInstallDriver(HWND hWnd)
{
	if (!*m_szDriverPath)return E_FAIL;
	HANDLE hFile = NULL;
	LPCTSTR DriverId = _T("WinRing0_1_2_0");

	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (hSCManager == NULL)
	{
		Ef(hWnd, "OpenSCManager");
		return E_FAIL;
	}
	else
	{
		m_fSystemInstallDriver = IsSystemInstallDriver(hWnd, hSCManager, DriverId);
		CloseServiceHandle(hSCManager);
		return S_OK;
	}
}

void CWin::RegisterSwitch(HWND hWnd)
{
	HMODULE hDll = LoadLibrary(_T("MonitorBar.dll"));
	if (!hDll)
	{
		DWORD dw = GetLastError( );
		if (ERROR_FILE_NOT_FOUND == dw)
			MessageBox(hWnd, _T("找不到MonitorBar.dll\n请将此程序与MonitorBar.dll放在同一文件夹下"), nullptr, MB_ICONERROR);
		else
			Ehd(hWnd, "LoadLibrary", dw);
		return;
	}
	HRESULT(__stdcall *func)( ) = nullptr;
	HRESULT(__stdcall *show)( BOOL ) = nullptr;
	if (m_fRegister)
		func = ( HRESULT(__stdcall *)( ) )GetProcAddress(hDll, "DllUnregisterServer");
	else
		func = ( HRESULT(__stdcall *)( ) )GetProcAddress(hDll, "DllRegisterServer");
	show = ( HRESULT(__stdcall*)( BOOL ) )GetProcAddress(hDll, "DllShowMonitorBar");
	if (!func || !show)
		Ef(hWnd, "GetProcAddress");
	else if (m_fRegister)
	{
		HRESULT hr = show(FALSE);
		if (FAILED(hr))
			Ehd(hWnd, "DllShowMonitorBar", hr);
		hr = func( );
		if (SUCCEEDED(hr))
		{
			// CoFreeUnusedLibraries( );
			HWND hTaskbarWnd = FindWindow(TEXT("Shell_TrayWnd"), NULL);
			if (hTaskbarWnd)
				PostMessage(hTaskbarWnd, WM_TIMER, 24, 0);
		}
		else
		{
			if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
				MessageBox(hWnd, _T("已反注册"), _T("信息"), MB_ICONINFORMATION);
			else
				Ehd(hWnd, "DllUnregisterServer", hr);
		}
	}
	else
	{

		HRESULT hr = func( );
		if (FAILED(hr))
			Ehd(hWnd, "DllRegisterServer", hr);
		else
		{
			hr = show(TRUE);
			if (FAILED(hr))
				Ehd(hWnd, "DllShowMonitorBar", hr);
		}
	}
	FreeLibrary(hDll);
	HRESULT hr = IsRegister(hWnd);
	if (hr == S_OK)
		m_fRegister = true;
	else if (hr == S_FALSE)
		m_fRegister = false;
}

void CWin::InstallSystemDriverSwitch(HWND hWnd, HINSTANCE hInstance)
{
	if (!*m_szDriverPath)return;
	HANDLE hFile = nullptr;
	LPCTSTR DriverId = _T("WinRing0_1_2_0");
	LOG("InstallSystemDriverSwitch start");
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	LOG("OpenSCManager start");
	if (hSCManager == NULL)
	{
		Ef(hWnd, "OpenSCManager");
		return;
	}
	LOG("OpenSCManager ok");
	bool bIsInstall = IsSystemInstallDriver(hWnd, hSCManager, DriverId);
	LOG("IsSystemInstallDriver ok");
	if (bIsInstall&&m_fSystemInstallDriver)
	{
		LOG("SystemInstallDriver closing");
		StopDriver(hWnd, hSCManager, DriverId);
		LOG("StopDriver ok");
		RemoveDriver(hWnd, hSCManager, DriverId);
		LOG("RemoveDriver ok");
	}
	else if (!bIsInstall&&!m_fSystemInstallDriver)
	{
		LOG("SystemInstallDriver opening");
		hFile = CreateFile(
			_T("\\\\.\\WinRing0_1_2_0"),
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
			);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			LOG("hFile == INVALID_HANDLE_VALUE");
			StopDriver(hWnd, hSCManager, DriverId);
			LOG("StopDriver ok");
			RemoveDriver(hWnd, hSCManager, DriverId);
			LOG("RemoveDriver ok");
			if (InstallDriver(hWnd, hSCManager, DriverId))
			{
				LOG("InstallDriver ok");
				StartDriver(hWnd, hSCManager, DriverId);
				LOG("StartDriver ok");
			}
			hFile = CreateFile(
				"\\\\.\\WinRing0_1_2_0",
				GENERIC_READ | GENERIC_WRITE,
				0,
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL
				);
		}
		LOG("SystemInstallDriver start");
		SystemInstallDriver(hWnd, hSCManager, DriverId);
		LOG("SystemInstallDriver ok");
		if (hFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFile);
		}
	}

	if (hSCManager != NULL)
	{
		CloseServiceHandle(hSCManager);
	}

	if (*m_szDriverPath ||
		InitDriver(hInstance))
		IsSystemInstallDriver(hWnd);
}

bool CWin::InitDriver(HINSTANCE hInstance)
{
	OSVERSIONINFOEX osie = { sizeof( OSVERSIONINFOEX ) };
	DWORD plantformIDs[] =
	{
		VER_PLATFORM_WIN32s,
		VER_PLATFORM_WIN32_WINDOWS,
		VER_PLATFORM_WIN32_NT
	};
	size_t i;
	GetModuleFileName(hInstance, m_szDriverPath, _countof(m_szDriverPath));
	LPTSTR lpEnd = _tcsrchr(m_szDriverPath, _T('\\'));
	if (lpEnd)
		*++lpEnd = _T('\0');
	for (i = 0; i < _countof(plantformIDs); ++i)
	{
		osie.dwPlatformId = plantformIDs[i];
		if (VerifyVersionInfo(&osie, VER_PLATFORMID, VerSetConditionMask(0, VER_PLATFORMID, VER_EQUAL)))
		{
			switch (i)
			{
			case 0:
				return false;
			case 1:
				_tcscat_s(m_szDriverPath, _T("WinRing0.vxd"));
				return true;
			case 2:
#ifdef _WIN64
#ifdef _M_X64
				_tcscat_s(m_szDriverPath, _T("WinRing0x64.sys"));
				return true;
#else
				_tcscat_s(m_szDriverPath, _T("WinRing0ia64.sys"));
				return false;
#endif
#else
				if (!IsWow64( ))
				{
					_tcscat_s(m_szDriverPath, _T("WinRing0.sys"));
					return true;
				}
				else if (IsX64( ))
				{
					_tcscat_s(m_szDriverPath, _T("WinRing0x64.sys"));
					return true;
				}
				else
				{
					_tcscat_s(m_szDriverPath, _T("WinRing0ia64.sys"));
					return false;
				}
#endif
			}
		}
	}
	return false;
}

bool CWin::InstallDriver(HWND hWnd, SC_HANDLE hSCManager, LPCTSTR DriverId)
{
	bool rCode = false;
	LOG("InstallDriver start");
	SC_HANDLE hService =
		CreateService(hSCManager, DriverId, DriverId, SERVICE_ALL_ACCESS,
		SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
		m_szDriverPath, nullptr, nullptr, nullptr, nullptr, nullptr);
	LOG("CreateService end");
	if (hService == NULL)
	{
		DWORD dw = GetLastError( );
		LOGERRD("CreateService", dw);
		if (dw == ERROR_SERVICE_EXISTS)
			rCode = true;
		else if (dw != ERROR_SERVICE_MARKED_FOR_DELETE)
			Ehd(hWnd, "CreateService", dw);
	}
	else
	{
		rCode = true;
		CloseServiceHandle(hService);
	}
	LOG("CreateService ok");
	return rCode;
}

bool CWin::RemoveDriver(HWND hWnd, SC_HANDLE hSCManager, LPCTSTR DriverId)
{
	bool rCode = false;

	SC_HANDLE hService = OpenService(hSCManager, DriverId, SERVICE_ALL_ACCESS);
	LOG("OpenService start");
	if (hService == NULL)
	{
		LOGERR("OpenService");
		return true;
	}
	else
	{
		LOG("OpenService ok");
		rCode = !!DeleteService(hService);
		LOG(rCode ? "DeleteService ok" : "DeleteService failed");
		if (!rCode)
			LOGERR("DeleteService");
		CloseServiceHandle(hService);
	}

	return rCode;
}

bool CWin::StartDriver(HWND hWnd, SC_HANDLE hSCManager, LPCTSTR DriverId)
{
	bool rCode = false;

	SC_HANDLE hService = OpenService(hSCManager, DriverId, SERVICE_ALL_ACCESS);
	LOG("OpenService start");
	if (hService == NULL)
	{
		Ef(hWnd, "OpenService");
		return false;
	}
	else
	{
		LOG("OpenService ok");
		if (!StartService(hService, 0, NULL))
		{
			DWORD dw = GetLastError( );
			LOGERRD("StartService", dw);
			if (dw == ERROR_SERVICE_ALREADY_RUNNING)
				rCode = true;
			else if (dw != ERROR_SERVICE_MARKED_FOR_DELETE)
				Ehd(hWnd, "StartService", dw);
		}
		else
			rCode = true;
		CloseServiceHandle(hService);
	}

	return rCode;
}

bool CWin::StopDriver(HWND hWnd, SC_HANDLE hSCManager, LPCTSTR DriverId)
{
	SERVICE_STATUS	serviceStatus;
	bool rCode = false;

	SC_HANDLE hService = OpenService(hSCManager, DriverId, SERVICE_ALL_ACCESS);
	LOG("OpenService start");
	if (hService == NULL)
	{
		DWORD dw = GetLastError( );
		LOGERRD("OpenService", dw);
		if (dw != ERROR_SERVICE_DOES_NOT_EXIST&&dw != ERROR_ALREADY_EXISTS)
			Ehd(hWnd, "OpenService", dw);
		return false;
	}
	else
	{
		LOG("OpenService ok");
		rCode = !!ControlService(hService, SERVICE_CONTROL_STOP, &serviceStatus);
		LOG(rCode ? "ControlService ok" : "ControlService failed");
		if (!rCode)
			LOGERR("ControlService");
		CloseServiceHandle(hService);
	}

	return rCode;
}

bool CWin::SystemInstallDriver(HWND hWnd, SC_HANDLE hSCManager, LPCTSTR DriverId)
{
	bool rCode = false;

	SC_HANDLE hService = OpenService(hSCManager, DriverId, SERVICE_ALL_ACCESS);

	if (hService == NULL)
	{
		Ef(hWnd, "OpenService");
		return false;
	}
	else
	{
		rCode =
			!!ChangeServiceConfig(hService, SERVICE_KERNEL_DRIVER, SERVICE_AUTO_START,
			SERVICE_ERROR_NORMAL, m_szDriverPath, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
		if (!rCode)
		{
			DWORD dw = GetLastError( );
			if (dw != ERROR_SERVICE_MARKED_FOR_DELETE)
				Ehd(hWnd, "ChangeServiceConfig", dw);
			else
				MessageBox(hWnd, _T("安装失败，需重启电脑后安装"), nullptr, MB_ICONERROR);
		}
		CloseServiceHandle(hService);
	}

	return rCode;
}

bool CWin::IsSystemInstallDriver(HWND hWnd, SC_HANDLE hSCManager, LPCTSTR DriverId)
{
	bool rCode = false;
	LOG("OpenService start");
	SC_HANDLE hService = OpenService(hSCManager, DriverId, SERVICE_ALL_ACCESS);
	LOG("OpenService end");
	if (hService == NULL)
	{
		DWORD dw = GetLastError( );
		LOGERRD("OpenService", dw);
		if (dw != ERROR_SERVICE_DOES_NOT_EXIST&&dw!=ERROR_ALREADY_EXISTS)
			Ehd(hWnd, "OpenService", dw);
		return false;
	}
	else
	{
		LOG("OpenService ok");
		DWORD dwSize;
		QueryServiceConfig(hService, NULL, 0, &dwSize);
		auto lpsc = new byte[dwSize]( );
		if (!lpsc)
			Ef(hWnd, "new");
		else
		{
			LPQUERY_SERVICE_CONFIG lpServiceConfig = reinterpret_cast<LPQUERY_SERVICE_CONFIG>( lpsc );
			if (!QueryServiceConfig(hService, lpServiceConfig, dwSize, &dwSize))
				Ef(hWnd, "QueryServiceConfig");
			else
			{
				if (lpServiceConfig->dwStartType == SERVICE_AUTO_START)
					rCode = true;
			}
			delete[] lpsc;
		}
		CloseServiceHandle(hService);
	}
	return rCode;
}

bool CWin::IsX64( )
{
	SYSTEM_INFO si;
	GetNativeSystemInfo(&si);
	return si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64;
}

bool CWin::IsWow64( )
{
	HMODULE hMod = LoadLibrary(TEXT("kernel32"));
	bool result = false;
	if (hMod)
	{
		auto isWow64Process = ( BOOL(WINAPI*)( HANDLE, PBOOL ) )GetProcAddress(hMod, "IsWow64Process");
		if (isWow64Process)
		{
			BOOL bl;
			if (isWow64Process(GetCurrentProcess( ), &bl))
				result = !!bl;
		}
		FreeLibrary(hMod);
	}
	return result;
}

DWORD CWin::Err(HWND hWnd, LPSTR func, DWORD line, LPSTR lpszFunction, DWORD dw)
{
	char* lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&lpMsgBuf,
		0, NULL);
	std::ostringstream oss;
	oss << "error"
		<< "\nerror in function:" << func
		<< "\nerror at line:" << line
		<< "\nerror code:0x" << std::setbase(16) << std::setw(8) << std::setfill('0') << dw
		<< "\nerror information:" << lpMsgBuf << std::endl;
	MessageBoxA(hWnd, oss.str( ).c_str( ), lpszFunction, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	return dw;
}

void Log(const char*function, size_t line, const std::string& msg)
{
	char szFileName[MAX_PATH];
	GetModuleFileNameA(nullptr, szFileName, _countof(szFileName));
	char* pstr = strrchr(szFileName, '.');
	if (pstr)
	{
		*( pstr + 1 ) = 0;
		strcat_s(szFileName, "log");
		std::ofstream fs(szFileName, std::ios::app);
		time_t _t = time(NULL);
		char _time[256];
		tm _tm;
		localtime_s(&_tm, &_t);
		strftime(_time, _countof(_time), "%Y年%m月%d日 %H时%M分%S秒", &_tm);
		fs << "[" << _time << "] ["
			<< function << "," << line << "] "
			<< msg << std::endl;
		fs.close( );
	}
}

void LogErr(const char*function, size_t line, const char*f, DWORD dw)
{
	char szFileName[MAX_PATH];
	GetModuleFileNameA(nullptr, szFileName, _countof(szFileName));
	char* pstr = strrchr(szFileName, '.');
	if (pstr)
	{
		*( pstr + 1 ) = 0;
		strcat_s(szFileName, "log");
		std::ofstream fs(szFileName, std::ios::app);
		time_t _t = time(NULL);
		char _time[256];
		tm _tm;
		localtime_s(&_tm, &_t);
		strftime(_time, _countof(_time), "%Y年%m月%d日 %H时%M分%S秒", &_tm);
		char* lp;
		FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
					   FORMAT_MESSAGE_FROM_SYSTEM |
					   FORMAT_MESSAGE_IGNORE_INSERTS,
					   nullptr, dw,
					   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (char*)&lp,
					   0, nullptr);
		fs << "[" << _time << "] Error:{\t"
			<< "error position:" << function << ",\t"
			<< "error line:" << line << ",\t"
			<< "error function:" << f << ",\t"
			<< "error code:" << dw << ",\t"
			<< "error information:" << lp << "}"
			<< std::endl;
		LocalFree(lp);
		fs.close( );
	}
}