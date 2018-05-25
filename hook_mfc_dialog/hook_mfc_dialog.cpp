// hook_mfc_dialog.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "hook_mfc_dialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TARGET_WND L"mfc_dialog"

HINSTANCE g_hinstance = NULL;
HHOOK g_hhook1 = NULL;
HHOOK g_hhook2 = NULL;

BOOL g_once = TRUE;
HWND g_hwnd = NULL;

//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

// Chook_mfc_dialogApp

BEGIN_MESSAGE_MAP(Chook_mfc_dialogApp, CWinApp)
END_MESSAGE_MAP()


// Chook_mfc_dialogApp construction

Chook_mfc_dialogApp::Chook_mfc_dialogApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only Chook_mfc_dialogApp object

Chook_mfc_dialogApp theApp;


// Chook_mfc_dialogApp initialization

BOOL Chook_mfc_dialogApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}

LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (g_once)
	{
		TCHAR sz[MAX_PATH] = {0};

		HWND hwnd = FindWindow(NULL, TARGET_WND);
		if (NULL == hwnd)
		{
			_stprintf_s(sz, L"%S(%d): FindWindow failed: %d",
				__FUNCTION__, __LINE__, GetLastError());
			OutputDebugString(sz);
			return CallNextHookEx(g_hhook1, nCode, wParam, lParam);
		}

		g_hwnd = hwnd;

		long new_style = WS_OVERLAPPED
			| WS_VISIBLE
			| WS_SYSMENU
			| WS_MINIMIZEBOX
			| WS_MAXIMIZEBOX
			| WS_CLIPCHILDREN
			| WS_CLIPSIBLINGS;

		long lstyle = GetWindowLong(hwnd, GWL_STYLE);
		lstyle &= new_style; // & will remove style from new_style
		SetWindowLong(hwnd, GWL_STYLE, lstyle);
		OutputDebugString(L"SetWindowLong done");

		g_once = FALSE;
	}

	CWPSTRUCT *p = (CWPSTRUCT *)lParam;
	LPDRAWITEMSTRUCT lpDrawItemStruct = (LPDRAWITEMSTRUCT)p->lParam;

	switch (p->message)
	{
	case WM_NCHITTEST:
		{
			OutputDebugString(L"WM_NCHITTEST hook");
			RECT rect;
			GetWindowRect(g_hwnd, &rect);

			CRect crect = rect;
			CPoint cpoint(GET_X_LPARAM(p->lParam), GET_Y_LPARAM(p->lParam));
			GetCursorPos(&cpoint);
			if (crect.PtInRect(cpoint))
			{
				OutputDebugString(L"in dialog");
				return HTCAPTION;
			}
			else
			{
				OutputDebugString(L"NOT in dialog");
			}
		}
		break;
	}

	return CallNextHookEx(g_hhook1, nCode, wParam, lParam);
}

LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	MSG *p = (MSG *)lParam;

	switch (p->message)
	{
	case WM_LBUTTONDOWN:
		OutputDebugString(L"WM_LBUTTONDOWN hook");
		// how to move dialog without title bar
		// https://www.cnblogs.com/huhu0013/p/4640728.html
		// SendMessage(g_hwnd, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, p->lParam);

		// it is best to use the following PostMessage, because clicking on the ok
		// button when using SendMessage requires more than one click
		PostMessage(g_hwnd, WM_NCLBUTTONDOWN, HTCAPTION, p->lParam);
		break;
	}
	return CallNextHookEx(g_hhook2, nCode, wParam, lParam);
}

extern "C" __declspec(dllexport) void BegDialogHook()
{
	TCHAR sz[MAX_PATH] = {0};

	HWND hwnd = FindWindow(NULL, TARGET_WND);
	if (NULL == hwnd)
	{
		_stprintf_s(sz, L"%S(%d): FindWindow failed: %d",
			__FUNCTION__, __LINE__, GetLastError());
		OutputDebugString(sz);
		return;
	}

	DWORD tid = GetWindowThreadProcessId(hwnd, NULL);
	if (0 == tid)
	{
		OutputDebugString(L"GetWindowThreadProcessId return 0");
		return;
	}

	g_hinstance = GetModuleHandle(L"hook_mfc_dialog.dll");
	g_hhook1 = SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, g_hinstance, tid);
	g_hhook2 = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, g_hinstance, tid);
}

extern "C" __declspec(dllexport) void EndDialogHook()
{
	if (NULL != g_hhook1)
		UnhookWindowsHookEx(g_hhook1);

	if (NULL != g_hhook2)
		UnhookWindowsHookEx(g_hhook2);
}
