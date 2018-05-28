// hook_mfc_dialog.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "hook_mfc_dialog.h"
#include <GdiPlus.h>

#pragma comment(lib, "GdiPlus.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TARGET_WND L"mfc_dialog"

#define IDC_BUTTON_MINIMIZE 1101
#define IDC_BUTTON_CLOSE 1102

HINSTANCE g_hinstance = NULL;
HHOOK g_hhook1 = NULL;
HHOOK g_hhook2 = NULL;

BOOL g_once = TRUE;
HWND g_hwnd = NULL;
HWND g_hwnd_minimize = NULL;
HWND g_hwnd_close = NULL;

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

void fill_rect(HDC hdc, Gdiplus::Rect rect, Gdiplus::Color *color, float *position, int i)
{
	Gdiplus::Graphics graphics(hdc);

	Gdiplus::LinearGradientBrush gradient_brush(
		rect,
		Gdiplus::Color(255, 255, 255, 0),
		Gdiplus::Color(255, 255, 0, 0),
		Gdiplus::LinearGradientModeVertical
		);

	gradient_brush.SetInterpolationColors(color, position, i);
	graphics.FillRectangle(&gradient_brush, rect);
}

LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (g_once)
	{
		g_once = FALSE;

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

		/*SetWindowLong(hwnd, GWL_STYLE,
			WS_CAPTION | WS_VISIBLE | WS_CLIPSIBLINGS | WS_OVERLAPPED |  WS_THICKFRAME | 0x00008000);
		SetWindowLong(hwnd, GWL_EXSTYLE,
			WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR | WS_EX_WINDOWEDGE);*/

		// round rectangle
		RECT window_rect;
		HRGN hrgn;
		GetWindowRect(hwnd, &window_rect);
		hrgn = CreateRoundRectRgn(
			0,
			0,
			window_rect.right - window_rect.left,
			window_rect.bottom - window_rect.top,
			(window_rect.right - window_rect.left) / 16,
			(window_rect.bottom - window_rect.top) / 16
			);
		SetWindowRgn(hwnd, hrgn, TRUE);

		// draw minimize and close button
		RECT client_rect;
		GetClientRect(hwnd, &client_rect);

		g_hwnd_minimize = CreateWindow(L"BUTTON", L"", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
			client_rect.right - 32 * 2,
			client_rect.top,
			32,
			32,
			hwnd,
			(HMENU)IDC_BUTTON_MINIMIZE,
			NULL,
			NULL);

		g_hwnd_close = CreateWindow(L"BUTTON", L"", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
			client_rect.right - 32 * 1,
			client_rect.top,
			32,
			32,
			hwnd,
			(HMENU)IDC_BUTTON_CLOSE,
			NULL,
			NULL);
	}

	CWPSTRUCT *p = (CWPSTRUCT *)lParam;
	LPDRAWITEMSTRUCT pdis = (LPDRAWITEMSTRUCT)p->lParam;

	switch (p->message)
	{
	case WM_NCHITTEST:
		{
			RECT rect;
			GetWindowRect(g_hwnd, &rect);

			CRect crect = rect;
			CPoint cpoint(GET_X_LPARAM(p->lParam), GET_Y_LPARAM(p->lParam));
			GetCursorPos(&cpoint);
			if (crect.PtInRect(cpoint))
			{
				// OutputDebugString(L"in dialog");
				return HTCAPTION;
			}
			else
			{
				/*OutputDebugString(L"NOT in dialog");*/
			}
		}
		break;
	case WM_DRAWITEM:
		{
			Gdiplus::GdiplusStartupInput gdiplusStartupInput;
			ULONG_PTR gdiplusToken;
			GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

			// SetBkMode(pdis->hDC, TRANSPARENT);
			FillRect(pdis->hDC, &pdis->rcItem, NULL);
			FrameRect(pdis->hDC, &pdis->rcItem, (HBRUSH)GetStockObject(BLACK_BRUSH));

			switch(pdis->CtlID)
			{
			case IDC_BUTTON_MINIMIZE:
				{
					Gdiplus::Graphics graphics(pdis->hDC);
					Gdiplus::Rect grect(0, 0, 40, 30);
					Gdiplus::Color gcolor[] =
					{
						Gdiplus::Color(247, 251, 255),
						Gdiplus::Color(239, 239, 247),
						Gdiplus::Color(222, 227, 231),
						Gdiplus::Color(231, 235, 239),
					};
					float position[] =
					{
						0.0f,
						0.333333f,
						0.666666f,
						1.0f
					};
					fill_rect(pdis->hDC, grect, gcolor, position, sizeof(position) / sizeof(position[0]));

					Gdiplus::SolidBrush gbrush(Gdiplus::Color(255, 0, 0, 0));
					Gdiplus::PointF point1(11.0f, 17.0f);
					Gdiplus::PointF point2(21.0f, 17.0f);
					Gdiplus::PointF point3(21.0f, 19.0f);
					Gdiplus::PointF point4(11.0f, 19.0f);
					Gdiplus::PointF points[4] = {point1, point2, point3, point4};
					graphics.FillPolygon(&gbrush, points, 4, Gdiplus::FillModeAlternate);
				}
				break;
			case IDC_BUTTON_CLOSE:
				{
					Gdiplus::Graphics graphics(pdis->hDC);
					Gdiplus::Rect grect(0, 0, 40, 30);
					Gdiplus::Color gcolor[] =
					{
						Gdiplus::Color(247, 251, 255),
						Gdiplus::Color(239, 239, 247),
						Gdiplus::Color(222, 227, 231),
						Gdiplus::Color(231, 235, 239),
					};
					float position[] =
					{
						0.0f,
						0.333333f,
						0.666666f,
						1.0f
					};
					fill_rect(pdis->hDC, grect, gcolor, position, sizeof(position) / sizeof(position[0]));

					Gdiplus::SolidBrush gbrush(Gdiplus::Color(255, 0, 0, 0));
					Gdiplus::PointF point1(11.0f, 13.0f);
					Gdiplus::PointF point2(13.0f, 11.0f);
					Gdiplus::PointF point3(23.0f, 21.0f);
					Gdiplus::PointF point4(21.0f, 23.0f);
					Gdiplus::PointF points1[4] = {point1, point2, point3, point4};
					graphics.FillPolygon(&gbrush, points1, 4, Gdiplus::FillModeAlternate);

					Gdiplus::PointF point5(23.0f, 13.0f);
					Gdiplus::PointF point6(21.0f, 11.0f);
					Gdiplus::PointF point7(11.0f, 21.0f);
					Gdiplus::PointF point8(13.0f, 23.0f);
					Gdiplus::PointF points2[4] = {point5, point6, point7, point8};
					graphics.FillPolygon(&gbrush, points2, 4, Gdiplus::FillModeAlternate);
				}
				break;
			}

			if (pdis->itemState & ODS_FOCUS)
			{
				pdis->rcItem.left += 2;
				pdis->rcItem.top += 2;
				pdis->rcItem.right -= 2;
				pdis->rcItem.bottom -= 2;

				DrawFocusRect(pdis->hDC, &pdis->rcItem);
			}
			Gdiplus::GdiplusShutdown(gdiplusToken);
			break;
		}
	case WM_COMMAND:
		{
			switch (p->wParam)
			{
			case IDC_BUTTON_MINIMIZE:
				ShowWindow(p->hwnd, SW_SHOWMINIMIZED);
				break;
			case IDC_BUTTON_CLOSE:
				PostQuitMessage(0);
				break;
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
