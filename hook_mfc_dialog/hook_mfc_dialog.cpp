// hook_mfc_dialog.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "hook_mfc_dialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

HINSTANCE g_hinstance = NULL;
HHOOK g_hhook1 = NULL;

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
	CWPSTRUCT *p = (CWPSTRUCT *)lParam;
	LPDRAWITEMSTRUCT lpDrawItemStruct = (LPDRAWITEMSTRUCT)p->lParam;

	switch (p->message)
	{
	case WM_SIZE:
		{
			OutputDebugString(L"WM_SIZE hook");
			break;
		}
	}

	return CallNextHookEx(g_hhook1, nCode, wParam, lParam);
}

extern "C" __declspec(dllexport) void BegDialogHook()
{
	HWND hwnd = FindWindow(NULL, L"mfc_dialog");
	if (NULL == hwnd)
	{
		OutputDebugString(L"FindWindow failed");
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
}

extern "C" __declspec(dllexport) void EndDialogHook()
{
	if (NULL != g_hhook1)
		UnhookWindowsHookEx(g_hhook1);
}
