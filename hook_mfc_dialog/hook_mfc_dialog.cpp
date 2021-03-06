// hook_mfc_dialog.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "hook_mfc_dialog.h"
#include <GdiPlus.h>
#include "transparent_button.h"
#include "CWMPPlayer4.h"
#include "WndShadow.h"

#pragma comment(lib, "GdiPlus.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TARGET_WND L"mfc_dialog"

#define IDC_BUTTON_MINIMIZE 1101
#define IDC_BUTTON_CLOSE 1102
#define IDC_MFC_CBUTTON_TEST 1103
#define IDC_TRANSPARENT_BUTTON_TEST 1104
#define IDC_TRANSPARENT_BUTTON_MINIMIZE 1105
#define IDC_TRANSPARENT_BUTTON_CLOSE 1106

#define CORNER_SIZE 2

HINSTANCE g_hinstance = NULL;
HHOOK g_hhook1 = NULL;
HHOOK g_hhook2 = NULL;
HHOOK g_hhook3 = NULL;

HWND g_hwnd = NULL;
HWND g_hwnd_minimize = NULL;
HWND g_hwnd_close = NULL;

CButton g_cbutton;
transparent_button g_tb_button;
transparent_button g_tb_button_minimize;
transparent_button g_tb_button_close;
HWND g_tb_button_hwnd = NULL;
HWND g_tb_button_minimize_hwnd = NULL;
HWND g_tb_button_close_hwnd = NULL;

WNDPROC g_old_proc;

class mfc_gdi_gif
{
public:
	mfc_gdi_gif()
		: m_p_image(NULL)
		, m_p_dimension_id(NULL)
		, m_p_item(NULL)
		, m_frame_count(0)
		, m_current_frame(0)
	{}

	~mfc_gdi_gif()
	{
		if (m_p_image)
			delete m_p_image;

		if (m_p_dimension_id)
			delete []m_p_dimension_id;

		if (m_p_item)
			free(m_p_item);
	}

public:
	Gdiplus::Image *m_p_image;
	GUID *m_p_dimension_id;
	Gdiplus::PropertyItem *m_p_item;
	UINT m_frame_count;
	UINT m_current_frame;
};

mfc_gdi_gif g_gif;
HWND g_gif_hwnd;

#define IDC_STATIC_GIF 0x3e8
#define TIMER_GIF 1

CWMPPlayer4 *g_wmp;

CWndShadow g_wnd_shadow;

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

void DrawBK(HDC dc, CImage *img, BUTTON_STATUS button_status, const transparent_button *tb, HWND tb_hwnd)
{
	if (!img)
	{
		return;
	}

	CRect rc;
	GetClientRect(tb_hwnd, &rc);
	CRect temp_rect;
	int nX = 0;
	int nY = 0;
	int nW = 0;
	int nH = 0;

	if (tb->m_b_autosize == true)
	{
		temp_rect.SetRect(0, 0, rc.Width(), rc.Height());
		if (img)
		{
			img->Draw(dc, temp_rect);
		}
	}
	else
	{
		if(button_status == BUTTON_NORMAL)
		{
			nW = tb->m_button_png_normal.width;
			nH = tb->m_button_png_normal.height;
		}
		else if (button_status == BUTTON_HOVER)
		{
			nW = tb->m_button_png_hover.width;
			nH = tb->m_button_png_hover.height;
		}
		else if (button_status == BUTTON_CLICK)
		{
			nW = tb->m_button_png_click.width;
			nH = tb->m_button_png_click.height;
		}
		else
		{
			nW = tb->m_button_png_disable.width;
			nH = tb->m_button_png_disable.height;
		}

		nX = (rc.Width() - nW) / 2;
		nY = (rc.Height() - nH) / 2;
		temp_rect.SetRect(nX, nY, nW + nX, nH + nY);
		if (img)
		{
			img->Draw(dc, temp_rect);
		}
	}
}

void DrawButtonText(HDC dc, const CString &strText, int nMove, BUTTON_STATUS button_status, HWND tb_hwnd)
{
	CRect rect;
	GetClientRect(tb_hwnd, &rect);
	rect.DeflateRect(nMove, nMove, 0, 0);

	CDC::FromHandle(dc)->SetBkMode(TRANSPARENT);

	if (button_status == BUTTON_NORMAL)
	{
		CDC::FromHandle(dc)->SetTextColor(RGB(30, 30, 30));
	}
	else if (button_status == BUTTON_HOVER)
	{
		CDC::FromHandle(dc)->SetTextColor(RGB(30, 30, 30));
	}
	else if (button_status == BUTTON_CLICK)
	{
		CDC::FromHandle(dc)->SetTextColor(RGB(30, 30, 30));
	}
	else
	{
		CDC::FromHandle(dc)->SetTextColor(RGB(100, 100, 100));
	}

	CDC::FromHandle(dc)->DrawText(strText, rect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
}

void DrawBorder(Gdiplus::Graphics& g)
{
	CRect client;
	GetClientRect(g_gif_hwnd, &client);

	Gdiplus::Rect gdi_rect(client.left, client.top, client.Width(), client.Height());
	Gdiplus::SolidBrush gdi_brush(Gdiplus::Color::Blue);

	g.FillRectangle(&gdi_brush, gdi_rect);
}

LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
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
	case WM_ERASEBKGND:
		{
			/*TCHAR sz[MAX_PATH] = {0};
			_stprintf_s(sz, L"WM_ERASEBKGND hwnd: %x", p->hwnd);
			OutputDebugString(sz);*/
		}
		break;
	case WM_DRAWITEM:
		{
			Gdiplus::GdiplusStartupInput gdiplusStartupInput;
			ULONG_PTR gdiplusToken;
			GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

			SetBkMode(pdis->hDC, TRANSPARENT);
			FillRect(pdis->hDC, &pdis->rcItem, NULL);
			FrameRect(pdis->hDC, &pdis->rcItem, (HBRUSH)GetStockObject(NULL_BRUSH));

			switch(pdis->CtlID)
			{
			case IDC_BUTTON_MINIMIZE:
				{
					Gdiplus::Graphics graphics(pdis->hDC);
					/*Gdiplus::Rect grect(0, 0, 40, 30);
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
					fill_rect(pdis->hDC, grect, gcolor, position, sizeof(position) / sizeof(position[0]));*/

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
					/*Gdiplus::Rect grect(0, 0, 40, 30);
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
					fill_rect(pdis->hDC, grect, gcolor, position, sizeof(position) / sizeof(position[0]));*/

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
			case IDC_TRANSPARENT_BUTTON_TEST:
				{
					g_tb_button_hwnd = GetDlgItem(g_hwnd, IDC_TRANSPARENT_BUTTON_TEST);
					if (NULL == g_tb_button_hwnd)
						break;

					CDC *pDC = CDC::FromHandle(pdis->hDC);
					CRect rect = pdis->rcItem;
					TCHAR strText[MAX_PATH] = {0};
					GetWindowText(g_tb_button_hwnd, strText, MAX_PATH);

					if(pdis->itemState & ODS_DISABLED)
					{
						DrawBK(*pDC, g_tb_button.m_button_png_disable.pimage, BUTTON_DISABLE,
							&g_tb_button, g_tb_button_hwnd);
					}
					else if(pdis->itemState & ODS_SELECTED
						|| (g_tb_button.m_b_ishover && g_tb_button.m_b_isclicked))
					{
						DrawBK(*pDC, g_tb_button.m_button_png_click.pimage, BUTTON_CLICK,
							&g_tb_button, g_tb_button_hwnd);
					}
					else if(g_tb_button.m_b_ishover)
					{
						DrawBK(*pDC, g_tb_button.m_button_png_hover.pimage, BUTTON_HOVER,
							&g_tb_button, g_tb_button_hwnd);
					}
					else
					{
						DrawBK(*pDC, g_tb_button.m_button_png_normal.pimage, BUTTON_NORMAL,
							&g_tb_button, g_tb_button_hwnd);
					}

					CString strTemp(strText);
					strTemp.Remove(' ');
					if (!strTemp.IsEmpty())
					{
						if(pdis->itemState & ODS_DISABLED)
						{
							DrawButtonText(*pDC, strText, 0, BUTTON_DISABLE, g_tb_button_hwnd);
						}
						else if(pdis->itemState & ODS_SELECTED
							|| (g_tb_button.m_b_ishover && g_tb_button.m_b_isclicked))
						{
							DrawButtonText(*pDC, strText, 1, BUTTON_CLICK, g_tb_button_hwnd);
						}
						else if(g_tb_button.m_b_ishover)
						{
							DrawButtonText(*pDC, strText, 0, BUTTON_HOVER, g_tb_button_hwnd);
						}
						else
						{
							DrawButtonText(*pDC, strText, 0, BUTTON_NORMAL, g_tb_button_hwnd);
						}
					}
				}
				break;
			case IDC_TRANSPARENT_BUTTON_MINIMIZE:
				{
					g_tb_button_minimize_hwnd = GetDlgItem(g_hwnd, IDC_TRANSPARENT_BUTTON_MINIMIZE);
					if (NULL == g_tb_button_minimize_hwnd)
						break;

					CDC *pDC = CDC::FromHandle(pdis->hDC);
					CRect rect = pdis->rcItem;
					TCHAR strText[MAX_PATH] = {0};
					GetWindowText(g_tb_button_minimize_hwnd, strText, MAX_PATH);

					if(pdis->itemState & ODS_DISABLED)
					{
						DrawBK(*pDC, g_tb_button_minimize.m_button_png_disable.pimage, BUTTON_DISABLE,
							&g_tb_button_minimize, g_tb_button_minimize_hwnd);
					}
					else if(pdis->itemState & ODS_SELECTED
						|| (g_tb_button_minimize.m_b_ishover && g_tb_button_minimize.m_b_isclicked))
					{
						DrawBK(*pDC, g_tb_button_minimize.m_button_png_click.pimage, BUTTON_CLICK,
							&g_tb_button_minimize, g_tb_button_minimize_hwnd);
					}
					else if(g_tb_button_minimize.m_b_ishover)
					{
						DrawBK(*pDC, g_tb_button_minimize.m_button_png_hover.pimage, BUTTON_HOVER,
							&g_tb_button_minimize, g_tb_button_minimize_hwnd);
					}
					else
					{
						DrawBK(*pDC, g_tb_button_minimize.m_button_png_normal.pimage, BUTTON_NORMAL,
							&g_tb_button_minimize, g_tb_button_minimize_hwnd);
					}

					CString strTemp(strText);
					strTemp.Remove(' ');
					if (!strTemp.IsEmpty())
					{
						if(pdis->itemState & ODS_DISABLED)
						{
							DrawButtonText(*pDC, strText, 0, BUTTON_DISABLE, g_tb_button_minimize_hwnd);
						}
						else if(pdis->itemState & ODS_SELECTED
							|| (g_tb_button_minimize.m_b_ishover && g_tb_button_minimize.m_b_isclicked))
						{
							DrawButtonText(*pDC, strText, 1, BUTTON_CLICK, g_tb_button_minimize_hwnd);
						}
						else if(g_tb_button_minimize.m_b_ishover)
						{
							DrawButtonText(*pDC, strText, 0, BUTTON_HOVER, g_tb_button_minimize_hwnd);
						}
						else
						{
							DrawButtonText(*pDC, strText, 0, BUTTON_NORMAL, g_tb_button_minimize_hwnd);
						}
					}
				}
				break;
			case IDC_TRANSPARENT_BUTTON_CLOSE:
				{
					g_tb_button_close_hwnd = GetDlgItem(g_hwnd, IDC_TRANSPARENT_BUTTON_CLOSE);
					if (NULL == g_tb_button_close_hwnd)
						break;

					CDC *pDC = CDC::FromHandle(pdis->hDC);
					CRect rect = pdis->rcItem;
					TCHAR strText[MAX_PATH] = {0};
					GetWindowText(g_tb_button_close_hwnd, strText, MAX_PATH);

					if(pdis->itemState & ODS_DISABLED)
					{
						DrawBK(*pDC, g_tb_button_close.m_button_png_disable.pimage, BUTTON_DISABLE,
							&g_tb_button_close, g_tb_button_close_hwnd);
					}
					else if(pdis->itemState & ODS_SELECTED
						|| (g_tb_button_close.m_b_ishover && g_tb_button_close.m_b_isclicked))
					{
						DrawBK(*pDC, g_tb_button_close.m_button_png_click.pimage, BUTTON_CLICK,
							&g_tb_button_close, g_tb_button_close_hwnd);
					}
					else if(g_tb_button_close.m_b_ishover)
					{
						DrawBK(*pDC, g_tb_button_close.m_button_png_hover.pimage, BUTTON_HOVER,
							&g_tb_button_close, g_tb_button_close_hwnd);
					}
					else
					{
						DrawBK(*pDC, g_tb_button_close.m_button_png_normal.pimage, BUTTON_NORMAL,
							&g_tb_button_close, g_tb_button_close_hwnd);
					}

					CString strTemp(strText);
					strTemp.Remove(' ');
					if (!strTemp.IsEmpty())
					{
						if(pdis->itemState & ODS_DISABLED)
						{
							DrawButtonText(*pDC, strText, 0, BUTTON_DISABLE, g_tb_button_close_hwnd);
						}
						else if(pdis->itemState & ODS_SELECTED
							|| (g_tb_button_close.m_b_ishover && g_tb_button_close.m_b_isclicked))
						{
							DrawButtonText(*pDC, strText, 1, BUTTON_CLICK, g_tb_button_close_hwnd);
						}
						else if(g_tb_button_close.m_b_ishover)
						{
							DrawButtonText(*pDC, strText, 0, BUTTON_HOVER, g_tb_button_close_hwnd);
						}
						else
						{
							DrawButtonText(*pDC, strText, 0, BUTTON_NORMAL, g_tb_button_close_hwnd);
						}
					}
				}
				break;
			case IDC_STATIC_GIF:
				{
					Gdiplus::Graphics g(pdis->hDC);
					DrawBorder(g);

					CRect client;
					GetClientRect(g_gif_hwnd, &client);

					g.DrawImage(g_gif.m_p_image, client.left, client.top, client.Width(), client.Height());
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
			case IDC_TRANSPARENT_BUTTON_MINIMIZE:
				ShowWindow(p->hwnd, SW_SHOWMINIMIZED);
				break;
			case IDC_TRANSPARENT_BUTTON_CLOSE:
				PostQuitMessage(0);
				break;
			case IDC_TRANSPARENT_BUTTON_TEST:
				MessageBox(NULL, L"Debug Assertion Failed!", L"Test", 0);
				break;
			}
		}
		break;
	case WM_PAINT:
		{
			HDC hdc = ::GetDC(g_hwnd);
			CDC	*pDC = CDC::FromHandle(hdc);

			RECT rect;
			GetClientRect(g_hwnd, &rect);

			// outside of window border
			CPen *old_pen = NULL;
			CPen new_pen1(PS_SOLID, 1, RGB(27, 147, 186));
			old_pen = pDC->SelectObject(&new_pen1);

			pDC->MoveTo(rect.left, CORNER_SIZE);
			pDC->LineTo(CORNER_SIZE, rect.top);
			pDC->LineTo(rect.right - CORNER_SIZE - 1, rect.top);
			pDC->LineTo(rect.right - 1, CORNER_SIZE);
			pDC->LineTo(rect.right - 1, rect.bottom - CORNER_SIZE - 1);
			pDC->LineTo(rect.right - CORNER_SIZE - 1, rect.bottom - 1);
			pDC->LineTo(CORNER_SIZE, rect.bottom - 1);
			pDC->LineTo(rect.left, rect.bottom - CORNER_SIZE - 1);
			pDC->LineTo(rect.left, CORNER_SIZE);

			// fill in gaps
			pDC->MoveTo(rect.left + 1, CORNER_SIZE);
			pDC->LineTo(CORNER_SIZE + 1, rect.top);
			pDC->MoveTo(rect.right - CORNER_SIZE - 1, rect.top + 1);
			pDC->LineTo(rect.right - 1, CORNER_SIZE + 1);
			pDC->MoveTo(rect.right - 2, rect.bottom - CORNER_SIZE - 1);
			pDC->LineTo(rect.right - CORNER_SIZE - 1, rect.bottom - 1);
			pDC->MoveTo(CORNER_SIZE, rect.bottom - 2);
			pDC->LineTo(rect.left, rect.bottom - CORNER_SIZE - 2);

			pDC->SelectObject(old_pen);

			// inside of window border
			CPen new_pen2(PS_SOLID, 1, RGB(196, 234, 247));
			old_pen = pDC->SelectObject(&new_pen2);

			pDC->MoveTo(rect.left + 1, CORNER_SIZE + 1);
			pDC->LineTo(CORNER_SIZE + 1, rect.top + 1);
			pDC->LineTo(rect.right - CORNER_SIZE - 2, rect.top + 1);
			pDC->LineTo(rect.right - 2, CORNER_SIZE + 1);
			pDC->LineTo(rect.right - 2, rect.bottom - CORNER_SIZE - 2);
			pDC->LineTo(rect.right - CORNER_SIZE - 2, rect.bottom - 2);
			pDC->LineTo(CORNER_SIZE + 1, rect.bottom - 2);
			pDC->LineTo(rect.left + 1, rect.bottom - CORNER_SIZE - 2);
			pDC->LineTo(rect.left + 1, CORNER_SIZE + 1);
		}
		break;
	case WM_SIZE:
		{
			if (p->hwnd == g_hwnd)
			{
				// remove the four sharp corners of the border
				if (p->wParam != SIZE_MAXIMIZED)
				{
					RECT rc;
					GetClientRect(g_hwnd, &rc);

					CRgn rgn;
					CPoint points[8] =
					{
						CPoint(rc.left, CORNER_SIZE),
						CPoint(CORNER_SIZE, rc.top),
						CPoint(rc.right - CORNER_SIZE, rc.top),
						CPoint(rc.right, CORNER_SIZE),
						CPoint(rc.right, rc.bottom - CORNER_SIZE - 1),
						CPoint(rc.right - CORNER_SIZE - 1, rc.bottom),
						CPoint(CORNER_SIZE + 1, rc.bottom),
						CPoint(rc.left, rc.bottom - CORNER_SIZE - 1)
					};

					int nPolyCounts[1] = {8};
					int dd = rgn.CreatePolyPolygonRgn(points, nPolyCounts, 1, WINDING);
					SetWindowRgn(g_hwnd, rgn, TRUE);
				}
				else
				{
					SetWindowRgn(g_hwnd, NULL, FALSE);
				}
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
		if (g_hwnd == p->hwnd)
		{
			PostMessage(p->hwnd, WM_NCLBUTTONDOWN, HTCAPTION, p->lParam);
		}
		break;
	case WM_TIMER:
		{
			switch (p->wParam)
			{
			case TIMER_GIF:
				{
					/*KillTimer(g_gif_hwnd, p->wParam);*/

					GUID guid = Gdiplus::FrameDimensionTime;
					g_gif.m_p_image->SelectActiveFrame(&guid, g_gif.m_current_frame);

					/*SetTimer(g_gif_hwnd, TIMER_GIF,((UINT *)g_gif.m_p_item[0].value)[g_gif.m_current_frame] * 1000, NULL);*/
					g_gif.m_current_frame = (++g_gif.m_current_frame) % g_gif.m_frame_count;

					RECT gif_rect;
					GetClientRect(g_gif_hwnd, &gif_rect);
					InvalidateRect(g_gif_hwnd, &gif_rect, TRUE);
				}
				break;
			}
		}
		break;
	}

	return CallNextHookEx(g_hhook2, nCode, wParam, lParam);
}

LRESULT CALLBACK new_proc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Msg == WM_CTLCOLORBTN)
	{
		HBRUSH hbrush;
		hbrush = (HBRUSH)GetStockObject(NULL_BRUSH);
		SetBkMode((HDC)wParam, TRANSPARENT);
		return ((LRESULT)hbrush);
	}

	if (Msg == WM_DESTROY)
	{
		OutputDebugString(L"new_proc WM_DESTROY");
		KillTimer(g_gif_hwnd, TIMER_GIF);

		if (g_wmp)
			delete g_wmp;
	}

	return CallWindowProc(g_old_proc, hWnd, Msg, wParam, lParam);
}

LRESULT CALLBACK CallWndRetProc(int nCode, WPARAM wParam,LPARAM lParam)
{
	CWPRETSTRUCT *p = (CWPRETSTRUCT *)lParam;
	switch (p->message)
	{
	case WM_INITDIALOG:
		{
			// Here it is guaranteed that WM_INITDIALOG is executed only once,
			// but if it is returned from the next window in IS, WM_INITDIALOG
			// will not be executed here.
			static bool subclassed = false;
			if (subclassed)
			{
				break;
			}
			subclassed = true;

			OutputDebugString(L"after hook_mfc_dialog WM_INITDIALOG");
			TCHAR sz[MAX_PATH] = {0};
			
			HWND hwnd = FindWindow(NULL, TARGET_WND);
			if (NULL == hwnd)
			{
				_stprintf_s(sz, L"%S(%d): FindWindow failed: %d",
					__FUNCTION__, __LINE__, GetLastError());
				OutputDebugString(sz);
				break;
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
			RECT client_rect;
			GetClientRect(hwnd, &client_rect);
			MoveWindow(hwnd,
				(GetSystemMetrics(SM_CXSCREEN) - client_rect.right - client_rect.left) / 2,
				(GetSystemMetrics(SM_CYSCREEN) - client_rect.bottom - client_rect.top) / 2,
				client_rect.right - client_rect.left,
				client_rect.bottom - client_rect.top,
				TRUE
				);
			/*HRGN hrgn;
			hrgn = CreateRoundRectRgn(
				0,
				0,
				client_rect.right - client_rect.left,
				client_rect.bottom - client_rect.top,
				16,
				16
				);
			SetWindowRgn(hwnd, hrgn, TRUE);*/

			// draw minimize and close button
			/*g_hwnd_minimize = CreateWindow(L"BUTTON", L"", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
				client_rect.right - 32 * 2 - 4,
				client_rect.top,
				32,
				32,
				hwnd,
				(HMENU)IDC_BUTTON_MINIMIZE,
				NULL,
				NULL);

			g_hwnd_close = CreateWindow(L"BUTTON", L"", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
				client_rect.right - 32 * 1 - 4,
				client_rect.top,
				32,
				32,
				hwnd,
				(HMENU)IDC_BUTTON_CLOSE,
				NULL,
				NULL);*/

			g_cbutton.Create(L"MFC CButton", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				CRect(0, 160, 100, 180), CWnd::FromHandle(hwnd), IDC_MFC_CBUTTON_TEST);

			g_tb_button.Create(L"tb button", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				CRect(120, 160, 200, 180), CWnd::FromHandle(hwnd), IDC_TRANSPARENT_BUTTON_TEST);
			g_tb_button.Load(IDB_BUTTON, 244);

			g_tb_button_minimize.Create(
				L"",
				WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				CRect(
				client_rect.right - 40 * 2 - 4,
				client_rect.top + 4,
				client_rect.right - 40 * 2 - 4 + 40,
				client_rect.top + 28
				),
				CWnd::FromHandle(hwnd),
				IDC_TRANSPARENT_BUTTON_MINIMIZE
				);
			g_tb_button_minimize.SetAutoSize(false);
			g_tb_button_minimize.Load(IDB_MINIMIZE, 28);

			g_tb_button_close.Create(
				L"",
				WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				CRect(
				client_rect.right - 40 * 1 - 4,
				client_rect.top + 4,
				client_rect.right - 40 * 1 - 4 + 40,
				client_rect.top + 28
				),
				CWnd::FromHandle(hwnd),
				IDC_TRANSPARENT_BUTTON_CLOSE
				);
			g_tb_button_close.SetAutoSize(false);
			g_tb_button_close.Load(IDB_CLOSE, 39);

			g_old_proc = (WNDPROC)SetWindowLong(hwnd, GWL_WNDPROC, (LONG)new_proc);

			// test gif
			g_gif_hwnd = GetDlgItem(hwnd, IDC_STATIC_GIF);
			if (g_gif_hwnd)
			{
				lstyle = GetWindowLong(g_gif_hwnd, GWL_STYLE);
				lstyle |= SS_OWNERDRAW;
				SetWindowLong(g_gif_hwnd, GWL_STYLE, lstyle);

				g_gif.m_p_image = Gdiplus::Image::FromFile(L"D:\\demos\\custom_dialog_demo\\hook_mfc_dialog\\sample.gif");

				UINT count = g_gif.m_p_image->GetFrameDimensionsCount();
				g_gif.m_p_dimension_id = new GUID[count];
				g_gif.m_p_image->GetFrameDimensionsList(g_gif.m_p_dimension_id, count);

				TCHAR tc_guid[39] = {0};
				StringFromGUID2(g_gif.m_p_dimension_id[0], tc_guid, 39);

				g_gif.m_frame_count = g_gif.m_p_image->GetFrameCount(&g_gif.m_p_dimension_id[0]);

				UINT total_buffer = g_gif.m_p_image->GetPropertyItemSize(PropertyTagFrameDelay);
				g_gif.m_p_item = (Gdiplus::PropertyItem *)malloc(total_buffer);
				g_gif.m_p_image->GetPropertyItem(PropertyTagFrameDelay, total_buffer, g_gif.m_p_item);

				g_gif.m_current_frame = 0;
				GUID guid = Gdiplus::FrameDimensionTime;
				g_gif.m_p_image->SelectActiveFrame(&guid, g_gif.m_current_frame);

				SetTimer(g_gif_hwnd, TIMER_GIF, ((UINT *)g_gif.m_p_item[0].value)[g_gif.m_current_frame] * 1000, NULL);
				++g_gif.m_current_frame;
				RECT gif_rect;
				GetClientRect(g_gif_hwnd, &gif_rect);
				InvalidateRect(g_gif_hwnd, &gif_rect, TRUE);
			}

			// test mp4
			g_wmp = new CWMPPlayer4;
			RECT wmp_rect;
			wmp_rect.left = 2;
			wmp_rect.top = 2;
			wmp_rect.right = 160;
			wmp_rect.bottom = 160;

			g_wmp->Create(NULL, WS_CHILD | WS_VISIBLE, wmp_rect, CWnd::FromHandle(hwnd), 0x444, NULL);
			g_wmp->put_uiMode(L"none");
			g_wmp->put_URL(L"D:\\test.mp4");

			// shadow window
			CWndShadow::Initialize(AfxGetInstanceHandle());

			g_wnd_shadow.Create(g_hwnd);
			g_wnd_shadow.SetSize(8);
			g_wnd_shadow.SetPosition(0, 0);

			COLORREF colorref = RGB(0, 173, 239);
			g_wnd_shadow.SetColor(colorref);
		}
		break;
	}

	return CallNextHookEx(g_hhook3, nCode, wParam, lParam);
}

extern "C" __declspec(dllexport) void BegDialogHook(HWND hwnd)
{
	DWORD tid = GetWindowThreadProcessId(hwnd, NULL);
	if (0 == tid)
	{
		OutputDebugString(L"GetWindowThreadProcessId return 0");
		return;
	}

	g_hinstance = GetModuleHandle(L"hook_mfc_dialog.dll");
	g_hhook1 = SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, g_hinstance, tid);
	g_hhook2 = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, g_hinstance, tid);
	g_hhook3 = SetWindowsHookEx(WH_CALLWNDPROCRET, CallWndRetProc, g_hinstance, tid);
}

extern "C" __declspec(dllexport) void EndDialogHook()
{
	if (NULL != g_hhook1)
		UnhookWindowsHookEx(g_hhook1);

	if (NULL != g_hhook2)
		UnhookWindowsHookEx(g_hhook2);

	if (NULL != g_hhook3)
		UnhookWindowsHookEx(g_hhook3);
}
