#include <windows.h>
#include "stdafx.h"
#include "fixLog.h"

#define WINDOW_WIDTH	300
#define WINDOW_HEIGTH	500

extern HINSTANCE hInst;

LRESULT CALLBACK fixHelpWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void fixLogUsageGuideUpdateWindow(HWND hWnd, HDC hdc);

void usageGuideMain(HWND hWnd)
{
	HWND hwnd;
	WNDCLASS winc;

	winc.style = CS_HREDRAW | CS_VREDRAW;
	winc.lpfnWndProc = fixHelpWndProc;
	winc.cbClsExtra = winc.cbWndExtra = 0;
	winc.hInstance = hInst;
	winc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winc.hCursor = LoadCursor(NULL, IDC_ARROW);
	winc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	winc.lpszMenuName = NULL;
	winc.lpszClassName = TEXT("Help");

	UnregisterClass(winc.lpszClassName, winc.hInstance);
	if (!RegisterClass(&winc)) return;

	hwnd = CreateWindow(
		TEXT("Help"),
		TEXT("ヘルプ"),				// タイトルバーに表示する文字列
		WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,	// ウィンドウの種類			
		0,				// ウィンドウを表示する位置（X座標）
		0,				// ウィンドウを表示する位置（Y座標）
		WINDOW_WIDTH,				// ウィンドウの幅
		WINDOW_HEIGTH,				// ウィンドウの高さ
		NULL,				// 親ウィンドウのウィンドウハンドル
		NULL,				// メニューハンドル
		hInst,				// インスタンスハンドル
		NULL				// その他の作成データ
	);

	if (hwnd == NULL) {
		return;
	}
	ShowWindow(hwnd, SW_SHOW);
}

LRESULT CALLBACK fixHelpWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HDC hmdc;
	static HBITMAP hBitmap;
	HDC hdc;
	static HWND hwndToolbar;
	static HBITMAP hToolBitmap;
	
	switch (message) {
		case WM_CREATE:
			hdc = GetDC(hWnd);							//デバイスコンテキストの取得
			hmdc = CreateCompatibleDC(hdc);				//ウィンドウのデバイスコンテキストに関連付けられたメモリDCを作成
			hBitmap = CreateCompatibleBitmap(hdc, WINDOW_WIDTH, WINDOW_HEIGTH);// ウィンドウのデバイスコンテキストと互換のあるビットマップを作成
			SelectObject(hmdc, hBitmap);				// メモリDCでビットマップを選択
			ReleaseDC(hWnd, hdc);						// デバイスコンテキストの解放	
			break;
		case WM_PAINT:
			fixLogUsageGuideUpdateWindow(hWnd, hmdc);
			break;
		case WM_DESTROY:
			DeleteDC(hmdc);					// メモリDCの削除
			DeleteObject(hBitmap);			// ビットマップ オブジェクトの削除
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void fixLogUsageGuideUpdateWindow(HWND hWnd, HDC hdc)
{
	HDC hdc2; 
	PAINTSTRUCT ps;
	HBRUSH hBrush;

	//WHITEブラシの作成&選択
	HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0xc0, 0xc0, 0xc0));
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
	SelectObject(hdc, hPen);
	hBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
	SelectObject(hdc, hBrush);
	Rectangle(hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGTH);

	SetBkMode(hdc, TRANSPARENT);

	HFONT hFont = CreateFont(15, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, VARIABLE_PITCH, NULL);
	SelectObject(hdc, hFont);

	SetTextColor(hdc, RGB(0, 0, 0));
	TextOut(hdc,  5,  10, "第1軸", 8);
	TextOut(hdc, 23,  35, "Th1(サーミスタ温度1)", 20);
	TextOut(hdc, 23,  60, "Th2(サーミスタ温度2)", 20);
	TextOut(hdc, 23,  85, "Th3(サーミスタ温度3)", 20);
	TextOut(hdc, 23, 110, "Th5(サーミスタ温度5)", 20);
	TextOut(hdc, 23, 135, "Ctl0(中央目標温度)", 20);
	TextOut(hdc, 23, 160, "Ctl1(サブ目標温度)", 20);
	TextOut(hdc, 23, 185, "Info(イベント)", 15);
	TextOut(hdc, 23, 210, "Duty(デューティー)", 20);
	TextOut(hdc, 23, 235, "DutySub(サブデューティー)", 25);

	TextOut(hdc,  5, 285, "第2軸", 8);
	TextOut(hdc, 23, 310, "State(状態)", 15);
	TextOut(hdc, 23, 335, "HTRMain(メインヒーター)", 20);
	TextOut(hdc, 23, 360, "HTRSub(サブヒーター)", 20);
	TextOut(hdc, 23, 385, "Press(圧状態)", 15);

	SetTextColor(hdc, Th1Color);
	TextOut(hdc, 7,  35, "―", 3);
	SetTextColor(hdc, Th2Color);
	TextOut(hdc, 7,  60, "―", 3);
	SetTextColor(hdc, Th3Color);
	TextOut(hdc, 7,  85, "―", 3);
	SetTextColor(hdc, Th5Color);
	TextOut(hdc, 7, 110, "―", 3);
	SetTextColor(hdc, Ctl0Color);
	TextOut(hdc, 7, 135, "―", 3);
	SetTextColor(hdc, Ctl1Color);
	TextOut(hdc, 7, 160, "―", 3);
	SetTextColor(hdc, InfoColor);
	TextOut(hdc, 7, 185, "●", 3);
	SetTextColor(hdc, DutyColor);
	TextOut(hdc, 7, 210, "●", 3);
	SetTextColor(hdc, DutySubColor);
	TextOut(hdc, 7, 235, "●", 3);

	SetTextColor(hdc, StateColor);
	TextOut(hdc, 7, 310, "―", 3);
	SetTextColor(hdc, HTRMainColor);
	TextOut(hdc, 7, 335, "―", 3);
	SetTextColor(hdc, HTRSubColor);
	TextOut(hdc, 7, 360, "―", 3);
	SetTextColor(hdc, PressColor);
	TextOut(hdc, 7, 385, "●", 3);

	hdc2 = BeginPaint(hWnd, &ps);
	// メモリDCから画像を転送
	BitBlt(hdc2, 0, 28, WINDOW_WIDTH, WINDOW_HEIGTH, hdc, 0, 0, SRCCOPY);

	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);

	SelectObject(hdc, GetStockObject(SYSTEM_FONT));
	DeleteObject(hFont);

	EndPaint(hWnd, &ps);
}
