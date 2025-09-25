#include <windows.h>
#include "stdafx.h"
#include <Commctrl.h>
#include <stdio.h>
#include "FixLog.h"
#include "profileInfo.h"
#include "Resource.h"
#include <shellapi.h>
#include <string.h>

#pragma comment(lib,"comctl32.lib")

#define TIMER_ID			1
#define TEMP_BUF_NUM		1000
#define INFO_BUF_NUM		200
#define DATA_BUF_NUM		20
#define WINDOW_WIDTH		800
#define WINDOW_HEIGTH		520
#define STATE_SCALE			25			//Stateの値をSTATE_SCALEの数だけ倍にする
#define	TIME_COUNT_50000	50000
#define CLICK_STATE_ADJ		10
#define USBLOG_DEV_NUM		6			//TH1,TH2,TH3,DSub,TH5,Duty

extern bool isTeratermUse;
extern HINSTANCE hInst;
extern void usageGuideMain(HWND hWnd);
extern void sendText(char* buffer);

void fixLogMain(HWND hDlg);
void fixLogInfo(char* form_msg, unsigned long timeCount, unsigned long value);
LRESULT CALLBACK fixWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void updateWindow(HWND hWnd, HDC hdc);
void dispAxisScale(HDC hdc);
void dispAxis(HWND hWndA, HDC hdc);
void dispTh1(HDC hdc);
void dispTh2(HDC hdc);
void dispTh3(HDC hdc);
void dispTh4(HDC hdc);
void dispTh5(HDC hdc);
void dispCtl0(HWND hWnd, HDC hdc);
void dispCtl1(HWND hWnd,HDC hdc);
void dispInfo(HDC hdc);
void dispDuty(HDC hdc);
void dispDutySub(HDC hdc);
void dispState(HWND hWnd, HDC hdc);
void dispHTRMain(HWND hWnd, HDC hdc);
void dispHTRSub(HWND hWnd, HDC hdc);
void dispPress(HDC hdc);
void dispIhCore(HDC hdc);
void dispCrrentTemp(HDC hdc);
void dispYData(HDC hdc);
void dispHorizonalLine(HDC hdc);
void dispVerticalLine(HDC hdc);
void dispTimeWidth(HDC hdc);
void dispPrintProfileInfo(HDC hdc);
void dispAnyLog(HDC hdc);

unsigned long xPos(unsigned long time);
unsigned long inverseXPos(unsigned long xPos);
unsigned long yPos(unsigned long value ,unsigned char axis);
unsigned long inverseYPos(unsigned long yPos);
unsigned long xAxis(unsigned long time);
unsigned long pixelsMoved(unsigned long timeSec);
float convertCountsToTime(signed long count);
void allReset();
void autoScroll();
void dragScroll();
void mouseMove();
void lButtonDown();
void lButtonUp(HDROP hDrop);
void calculateNearestValue();
void dispAllButton(HDROP hDrop);		//画面尺度を全体表示に合わせる
void readFile(HDROP hDrop);
void readAllFile(HDROP hDrop, HDC hdc, HWND hWnd);
void drawAllFile(char* form_msg, unsigned long timeCount, unsigned long value, HDC hdc);
void selectRange(HDC hdc, HWND hWnd);

static unsigned long X_0 = 40, Y_0 = 340;
static unsigned long timeTh1[TEMP_BUF_NUM];
static unsigned long tempTh1[TEMP_BUF_NUM];
static unsigned long posTh1 = 0;
static bool overTh1Flg = false;
static unsigned long timeTh2[TEMP_BUF_NUM];
static unsigned long tempTh2[TEMP_BUF_NUM];
static unsigned long posTh2 = 0;
static bool overTh2Flg = false;
static unsigned long timeTh3[TEMP_BUF_NUM];
static unsigned long tempTh3[TEMP_BUF_NUM];
static unsigned long posTh3 = 0;
static bool overTh3Flg = false;
static unsigned long timeTh4[TEMP_BUF_NUM];
static unsigned long tempTh4[TEMP_BUF_NUM];
static unsigned long posTh4 = 0;
static bool overTh4Flg = false;
static unsigned long timeTh5[TEMP_BUF_NUM];
static unsigned long tempTh5[TEMP_BUF_NUM];
static unsigned long posTh5 = 0;
static bool overTh5Flg = false;

static unsigned long timeCtl1[DATA_BUF_NUM];
static unsigned long tempCtl1[DATA_BUF_NUM];
static unsigned long posCtl1 = 0;
static bool overCtl1Flg = false;
static unsigned long timeCtl0[DATA_BUF_NUM];
static unsigned long tempCtl0[DATA_BUF_NUM];
static unsigned long posCtl0 = 0;
static bool overCtl0Flg = false;

static unsigned long timeInfo[INFO_BUF_NUM];
static unsigned long valueInfo[INFO_BUF_NUM];
static unsigned long posInfo = 0;
static bool overInfoFlg = false;

static unsigned long timeDuty[TEMP_BUF_NUM];
static unsigned long valueDuty[TEMP_BUF_NUM];
static unsigned long posDuty = 0;
static bool overDutyFlg = false;
static unsigned long timeDutySub[TEMP_BUF_NUM];
static unsigned long valueDutySub[TEMP_BUF_NUM];
static unsigned long posDutySub = 0;
static bool overDutySubFlg = false;

static unsigned long timeState[DATA_BUF_NUM];
static unsigned long valueState[DATA_BUF_NUM];
static unsigned long posState = 0;
static bool overStateFlg = false;

static unsigned long timeHTRMain[DATA_BUF_NUM];
static unsigned long valueHTRMain[DATA_BUF_NUM];
static unsigned long posHTRMain = 0;
static bool overHTRMainFlg = false;
static unsigned long timeHTRSub[DATA_BUF_NUM];
static unsigned long valueHTRSub[DATA_BUF_NUM];
static unsigned long posHTRSub = 0;
static bool overHTRSubFlg = false;

static unsigned long timePress[DATA_BUF_NUM];
static unsigned long valuePress[DATA_BUF_NUM];
static unsigned long posPress = 0;
static bool overPressFlg = false;

static unsigned long timeIhCore[DATA_BUF_NUM];
static unsigned long valueIhCore[DATA_BUF_NUM];
static unsigned long posIhCore = 0;
static bool overIhCoreFlg = false;

static unsigned long timePaSize[DATA_BUF_NUM];
static unsigned long paperSize[DATA_BUF_NUM];
static unsigned long posPaSize = 0;
static bool overPaSizeFlg = false;

static unsigned long timeMediaType[DATA_BUF_NUM];
static unsigned long mediaType[DATA_BUF_NUM];
static unsigned long posMediaType = 0;
static bool overMediaTypeFlg = false;

static unsigned long timeColorMode[DATA_BUF_NUM];
static unsigned long colorMode[DATA_BUF_NUM];
static unsigned long posColorMode = 0;
static bool overColorModeFlg = false;

static unsigned long timePaWeigth[DATA_BUF_NUM];
static unsigned long paperWeigth[DATA_BUF_NUM];
static unsigned long posPaWeight = 0;
static bool overPaWeightFlg = false;

static unsigned long timePaSpeed[DATA_BUF_NUM];
static unsigned long printSpeed[DATA_BUF_NUM];
static unsigned long posPaSpeed = 0;
static bool overPaSpeedFlg = false;

static unsigned long timePaFeed[DATA_BUF_NUM];
static unsigned long paperFpeed[DATA_BUF_NUM];
static unsigned long posPaFeed = 0;
static bool overPaFeedFlg = false;

static unsigned long timePaSide[DATA_BUF_NUM];
static unsigned long printSide[DATA_BUF_NUM];
static unsigned long posPaSide = 0;
static bool overPaSideFlg = false;

static unsigned long timeAnyLog[DATA_BUF_NUM];
static unsigned long valueAnyLog[DATA_BUF_NUM];
static char wordAnyLog[DATA_BUF_NUM][20];
static unsigned long posAnyLog = 0;
static bool overAnyFlg = false;

static unsigned long timeYData1[DATA_BUF_NUM];
static unsigned long valueYData1[DATA_BUF_NUM];
static unsigned long timeYData2[DATA_BUF_NUM];
static unsigned long valueYData2[DATA_BUF_NUM];
static unsigned char valueYAxis[DATA_BUF_NUM];
static unsigned long posYData = 0;
static bool overYDataFlg = false;

static unsigned long timeWidth1[DATA_BUF_NUM];
static unsigned long valueTimeWidth1[DATA_BUF_NUM];
static unsigned long timeWidth2[DATA_BUF_NUM];
static unsigned long valueTimeWidth2[DATA_BUF_NUM];
static float timeWidth3[DATA_BUF_NUM];
static unsigned long yPosTimeWidth[DATA_BUF_NUM];
static unsigned char valueTimeWidthAxis[DATA_BUF_NUM];
static unsigned long posTimeWidth = 0;
static bool overTimeWidthFlg = false;

static unsigned long valueHorizonal[DATA_BUF_NUM];
static unsigned char valueHorizonalAxis[DATA_BUF_NUM];
static unsigned long posHorizonal = 0;
static bool overHorizonalFlg = false;

static unsigned long timeVertical[DATA_BUF_NUM];
static unsigned long posVertical = 0;
static bool overVerticalFlg = false;

static unsigned long offSetTimePos = 0;
static unsigned long offSetPlus = 0;
static unsigned long timeScale = 20;

static unsigned long cursorDownX = 0;		//左クリックしたときのX座標
static signed long analyzOffSet1 = 0;
static signed long analyzOffSet2 = 0;
static signed long moveOffSet = 0;			//カーソル移動に伴う画面移動のピクセル数
static signed long mousePosX;
static signed long mousePosY;
static unsigned long movingY = 0;			//MouseMoveしているときのY座標
static unsigned long movingX = 0;			//MouseMoveしているときのX座標
static unsigned long scaleDiffCount = 0;	//拡大縮小時 変更前の中心のカウント値
static signed long scaleDiff = 0;

FILE *fp;
static char fileName[200];
static unsigned long minCount = 0xffffffff;
static unsigned long maxCount = 0;
static bool firstLogFlg[EANALYZ_LOG_NUM] = {true, true, true, true, true, true, true, true, true, true, true, true, true, true, true};
static unsigned long timeLogFile[EANALYZ_LOG_NUM] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static unsigned long valueLogFile[EANALYZ_LOG_NUM] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static unsigned long firstReadLogTime = 0xffffffff;
static bool clickFlg = false;
static bool clickMoveFlg = false;

unsigned char itemNum = EANALYZ_LOG_INFO; 
static eANALYZ_STATE analyzState = EANALYZ_NORMAL;
static eANALYZ_LOG_STATE logFileAnalyzState = EANALYZ_IDLE;
static eANALYZ_CLICK_STATE clickState = EANALYZ_MOUSE_UP;

										  //TH1   TH2    TH3    DSub   TH5    Duty
static bool usbLogExist[USBLOG_DEV_NUM] = {false, false, false, false, false, false};
struct USB_LOG_INFO_STR{
	char logStr[10];
	char valStr[10];
	char timeStr[10];
	bool result;
};
struct USB_LOG_TEMP_STR{
	char valStr[USBLOG_DEV_NUM][10];
	char timeStr[10];
	bool result;
};

static struct{
	unsigned short dispID;
	unsigned short choiceID;
	unsigned long bufNum;
	bool dispFlag;
	unsigned long *time;
	unsigned long *value;
	unsigned char axis;
} analyzItem[EANALYZ_LOG_NUM] = {
	{IDM_DISP_BOX_TH1,		IDM_CHOICE_BOX_TH1,		TEMP_BUF_NUM,	true,	timeTh1,	tempTh1,	1},
	{IDM_DISP_BOX_TH2,		IDM_CHOICE_BOX_TH2,		TEMP_BUF_NUM,	true,	timeTh2,	tempTh2,	1},
	{IDM_DISP_BOX_TH3,		IDM_CHOICE_BOX_TH3,		TEMP_BUF_NUM,	true,	timeTh3,	tempTh3,	1},
	{IDM_DISP_BOX_TH4,		IDM_CHOICE_BOX_TH4,		TEMP_BUF_NUM,	true,	timeTh4,	tempTh4,	1},
	{IDM_DISP_BOX_TH5,		IDM_CHOICE_BOX_TH5,		TEMP_BUF_NUM,	true,	timeTh5,	tempTh5,	1},
	{IDM_DISP_BOX_CTL0,		IDM_CHOICE_BOX_CTL0,	DATA_BUF_NUM,	true,	timeCtl0,	tempCtl0,	1},
	{IDM_DISP_BOX_CTL1,		IDM_CHOICE_BOX_CTL1,	DATA_BUF_NUM,	true,	timeCtl1,	tempCtl1,	1},
	{IDM_DISP_BOX_INFO,		IDM_CHOICE_BOX_INFO,	INFO_BUF_NUM,	true,	timeInfo,	valueInfo,	1},
	{IDM_DISP_BOX_DUTY,		IDM_CHOICE_BOX_DUTY,	TEMP_BUF_NUM,	true,	timeDuty,	valueDuty,	1},
	{IDM_DISP_BOX_DUTYSUB,	IDM_CHOICE_BOX_DUTYSUB,	TEMP_BUF_NUM,	true,	timeDutySub,valueDutySub,1},
	{IDM_DISP_BOX_STATE,	IDM_CHOICE_BOX_STATE,	DATA_BUF_NUM,	true,	timeState,	valueState,	2},
	{IDM_DISP_BOX_HTRMAIN,	IDM_CHOICE_BOX_HTRMAIN,	DATA_BUF_NUM,	true,	timeHTRMain,valueHTRMain,2},
	{IDM_DISP_BOX_HTRSUB,	IDM_CHOICE_BOX_HTRSUB,	DATA_BUF_NUM,	true,	timeHTRSub,	valueHTRSub,2},
	{IDM_DISP_BOX_PRESS,	IDM_CHOICE_BOX_PRESS,	DATA_BUF_NUM,	true,	timePress,	valuePress,	2},
	{IDM_DISP_BOX_IHCORE,	IDM_CHOICE_BOX_IHCORE,	DATA_BUF_NUM,	true,	timeIhCore,	valueIhCore,2},
};

static struct{
	unsigned long *pos;
	bool *flg;
} itemReset[EANALYZ_LOG_NUM + EANALYZ_TOOL_NUM + EANALYZ_PRINT_PROFILE_NUM] = {
	{&posTh1,		&overTh1Flg},
	{&posTh2,		&overTh2Flg},
	{&posTh3,		&overTh3Flg},
	{&posTh4,		&overTh4Flg},
	{&posTh5,		&overTh5Flg},
	{&posCtl0,		&overCtl0Flg},
	{&posCtl1,		&overCtl1Flg},
	{&posInfo,		&overInfoFlg},
	{&posDuty,		&overDutyFlg},
	{&posDutySub,	&overDutySubFlg},
	{&posState,		&overStateFlg},
	{&posHTRMain,	&overHTRSubFlg},
	{&posHTRSub,	&overHTRMainFlg},
	{&posPress,		&overPressFlg},
	{&posIhCore,	&overIhCoreFlg},
	{&posYData,		&overYDataFlg},
	{&posTimeWidth,	&overTimeWidthFlg},
	{&posHorizonal,	&overHorizonalFlg},
	{&posVertical,	&overVerticalFlg},
	{&posPaSize,	&overPaSizeFlg},
	{&posMediaType,	&overMediaTypeFlg},
	{&posColorMode,	&overColorModeFlg},
	{&posPaWeight,	&overPaWeightFlg},
	{&posPaSpeed,	&overPaSpeedFlg},
	{&posPaFeed,	&overPaFeedFlg},
	{&posPaSide,	&overPaSideFlg},
	{&posAnyLog,	&overAnyFlg}
};

void fixLogMain(HWND hDlg)
{
	HWND hwnd;
	WNDCLASS winc;

	winc.style = CS_HREDRAW | CS_VREDRAW;
	winc.lpfnWndProc = fixWndProc;
	winc.cbClsExtra = winc.cbWndExtra = 0;
	winc.hInstance = hInst;
	winc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winc.hCursor = LoadCursor(NULL, IDC_ARROW);
	winc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	winc.lpszMenuName = MAKEINTRESOURCE(IDR_ANALYZER);
	winc.lpszClassName = TEXT("fixlog");

	UnregisterClass(winc.lpszClassName, winc.hInstance);
	if (!RegisterClass(&winc)) return;

	hwnd = CreateWindow(
		TEXT("fixlog"),
		TEXT("定着Analyzer"),									// タイトルバーに表示する文字列
		(WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX) | WS_EX_COMPOSITED,	// ウィンドウの種類
		0,														// ウィンドウを表示する位置（X座標）
		0,														// ウィンドウを表示する位置（Y座標）
		WINDOW_WIDTH,											// ウィンドウの幅
		WINDOW_HEIGTH,											// ウィンドウの高さ
		NULL,													// 親ウィンドウのウィンドウハンドル
		NULL,													// メニューハンドル
		hInst,													// インスタンスハンドル
		NULL													// その他の作成データ
	);

	if (hwnd == NULL) {
		return;
	}
	ShowWindow(hwnd, SW_SHOW);
}

TBBUTTON tbButton[] = {
	{0, ECOMMAND_SCALEUP,	TBSTATE_ENABLED, TBSTYLE_BUTTON,		0,0,0},
	{1, ECOMMAND_SCALEDOWN,	TBSTATE_ENABLED, TBSTYLE_BUTTON,		0,0,0},
	{2, ECOMMAND_HELP,		TBSTATE_ENABLED, TBSTYLE_BUTTON,		0,0,0},
	{3, ECOMMAND_FIXINSERT,	TBSTATE_ENABLED, TBSTYLE_BUTTON,		0,0,0},
	{4, ECOMMAND_DISPALL,	TBSTATE_ENABLED, TBSTYLE_BUTTON,		0,0,0},
	{0, 0,					TBSTATE_ENABLED, TBSTYLE_SEP,			0,0,0},
	{5, ECOMMAND_MOVE,		TBSTATE_ENABLED, TBSTYLE_CHECKGROUP,	0,0,0},
	{6, ECOMMAND_TIMEWIDTH,	TBSTATE_ENABLED, TBSTYLE_CHECKGROUP,	0,0,0},
	{7, ECOMMAND_YDATA,		TBSTATE_ENABLED, TBSTYLE_CHECKGROUP,	0,0,0},
	{8, ECOMMAND_HORIZONAL,	TBSTATE_ENABLED, TBSTYLE_CHECKGROUP,	0,0,0},
	{9, ECOMMAND_VERTICAL,	TBSTATE_ENABLED, TBSTYLE_CHECKGROUP,	0,0,0},
	{0, 0,					TBSTATE_ENABLED, TBSTYLE_SEP,			0,0,0},
	{10,ECOMMAND_RESET,		TBSTATE_ENABLED, TBSTYLE_BUTTON,		0,0,0},
	{11,ECOMMAND_ALLRESET,	TBSTATE_ENABLED, TBSTYLE_BUTTON,		0,0,0}
};

BOOL CALLBACK EnumWndProc(HWND hWnd, LPARAM lParam)
{
	char buff[256] = "";
	char buff2[256] = "Tera Term VT";
	static HWND teraHwnd;

	GetWindowText(hWnd, buff, sizeof(buff));//ウィンドウ名を取得
	unsigned short length = sprintf_s(buff, sizeof(buff), "%s", buff);

	if (strstr(buff, buff2) != NULL) {
		//		teraHwnd = FindWindow(NULL, buff);
		SetForegroundWindow(hWnd);
		keybd_event('F', 0, 0, 0);
		keybd_event('F', 0, KEYEVENTF_KEYUP, 0);
		keybd_event('I', 0, 0, 0);
		keybd_event('I', 0, KEYEVENTF_KEYUP, 0);
		keybd_event('X', 0, 0, 0);
		keybd_event('X', 0, KEYEVENTF_KEYUP, 0);
		keybd_event(VK_RETURN, 0, 0, 0);                //enterキー押す
		keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0);  //enterキー離す
		printf("%s\n", buff);
	}
	return true;
}


LRESULT CALLBACK fixWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	static HDC hmdc;
	static HDROP hDrop;
	static HBITMAP hBitmap;
	static HWND hwndToolbar;
	static HBITMAP hToolBitmap;
	static HMENU hMenu[EANALYZ_LOG_NUM];
	static UINT uState[EANALYZ_LOG_NUM];
	static HMENU cMenu[EANALYZ_LOG_NUM];
	static UINT cState[EANALYZ_LOG_NUM];

	switch (message) {
		case WM_CREATE:
			SetTimer(hWnd, TIMER_ID, 100, NULL);
			hdc = GetDC(hWnd);							//デバイスコンテキストの取得
			hmdc = CreateCompatibleDC(hdc);				//ウィンドウのデバイスコンテキストに関連付けられたメモリDCを作成
			hBitmap = CreateCompatibleBitmap(hdc, WINDOW_WIDTH, WINDOW_HEIGTH);// ウィンドウのデバイスコンテキストと互換のあるビットマップを作成
			SelectObject(hmdc, hBitmap);				// メモリDCでビットマップを選択
			ReleaseDC(hWnd, hdc);						// デバイスコンテキストの解放	
			InitCommonControls();
			hToolBitmap = LoadBitmap(GetModuleHandle(0),MAKEINTRESOURCE(IDR_TOOLBAR1));
			hwndToolbar = CreateToolbarEx(
				hWnd, WS_CHILD | WS_VISIBLE, 0, 14,
				NULL, (UINT_PTR)hToolBitmap,
				tbButton, 14, 0, 0, 0, 0, sizeof(TBBUTTON)
			);
			DragAcceptFiles(hWnd, TRUE);
			break;
		case WM_TIMER:
			if (wParam == TIMER_ID) {
				if (analyzState != EANALYZ_ALL_PAINT) {
					autoScroll();
					InvalidateRect(hWnd, NULL, FALSE);
				}
			}
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				//TOOLBAR_COMMAND
				case ECOMMAND_SCALEUP:				//拡大ボタン
					scaleDiffCount = inverseXPos(WINDOW_WIDTH / 2);
					if ((timeScale > 1) && (timeScale < 30)) {
						timeScale -= 1;
					} else if (timeScale >= 30) {
						timeScale -= 10;
					} else {
						timeScale = 1;
					}
					scaleDiff -= (signed long)xPos(scaleDiffCount) - (WINDOW_WIDTH / 2);
					break;
				case ECOMMAND_SCALEDOWN:			//縮小ボタン
					scaleDiffCount = inverseXPos(WINDOW_WIDTH / 2);
					if (timeScale < 30){
						timeScale += 1;
					} else {
						timeScale += 10;
					}
					scaleDiff -= (signed long)xPos(scaleDiffCount) - (WINDOW_WIDTH / 2);
					if ((signed long)offSetTimePos - (signed long)moveOffSet - (signed long)scaleDiff< 0) {
						scaleDiff = (signed long)offSetTimePos - (signed long)moveOffSet;
					}
					break;
				case ECOMMAND_FIXINSERT:				//Fix入力ボタン
					if (isTeratermUse) {
						EnumWindows(EnumWndProc, (LPARAM)NULL);
					} else {
						sendText("fix");
					}
					break;
				case ECOMMAND_DISPALL:					//全体表示ボタン
					if (hDrop != NULL) {
						if (logFileAnalyzState == EANALYZ_IDLE) {
							allReset();
							readFile(hDrop);
							logFileAnalyzState = EANALYZ_DROP_FILE;
						}
					}
					dispAllButton(hDrop);
					InvalidateRect(hWnd, NULL, FALSE);
					break;
				case ECOMMAND_TIMEWIDTH:				//幅カーソルボタン
					if ((analyzState != EANALYZ_ALL_PAINT) || (logFileAnalyzState != EANALYZ_SELECT_RANGE)) {
						analyzState = EANALYZ_TIME_WIDTH;
					}
					break;
				case ECOMMAND_MOVE:						//十字ボタン
					if ((analyzState != EANALYZ_ALL_PAINT) || (logFileAnalyzState != EANALYZ_SELECT_RANGE)) {
						analyzState = EANALYZ_NORMAL;
					}
					break;
				case ECOMMAND_YDATA:					//Ydata表示ボタン
					if ((analyzState != EANALYZ_ALL_PAINT) || (logFileAnalyzState != EANALYZ_SELECT_RANGE)) {
						analyzState = EANALYZ_WATCH_YDATA;
					}
					break;
				case ECOMMAND_HORIZONAL:				//横線ボタン
					if ((analyzState != EANALYZ_ALL_PAINT) || (logFileAnalyzState != EANALYZ_SELECT_RANGE)) {
						analyzState = EANALYZ_HORIZONAL;
					}
					break;
				case ECOMMAND_VERTICAL:					//縦線ボタン
					if ((analyzState != EANALYZ_ALL_PAINT) || (logFileAnalyzState != EANALYZ_SELECT_RANGE)) {
						analyzState = EANALYZ_VERTICAL;
					}
					break;
				case ECOMMAND_RESET:					//Resetボタン
					switch (analyzState) {
						case EANALYZ_TIME_WIDTH:
						case EANALYZ_TIME_WIDTH_LINE:
							analyzState = EANALYZ_TIME_WIDTH;
							posTimeWidth = 0;
							overTimeWidthFlg = false;
							break;
						case EANALYZ_WATCH_YDATA:
						case EANALYZ_WATCH_YDATA_LINE:
							analyzState = EANALYZ_WATCH_YDATA;
							posYData = 0;
							overYDataFlg = false;
							break;
						case EANALYZ_HORIZONAL:
							posHorizonal = 0;
							overHorizonalFlg = false;
							break;
						case EANALYZ_VERTICAL:
							posVertical = 0;
							overVerticalFlg = false;
							break;
						default:
							break;
					}
					InvalidateRect(hWnd, NULL, FALSE);
					break;
				case ECOMMAND_ALLRESET:
					allReset();
					InvalidateRect(hWnd, NULL, FALSE);
					break;
				//MENUBAR & TOOLBAR_COMMAND
				case ECOMMAND_HELP:						//凡例ボタン・メニュー
				case IDM_ANALYS_LEGEND:
					usageGuideMain(hWnd);
					break;
				//MENUBAR_COMMAND
				case IDM_DISP_BOX_TH1:
				case IDM_DISP_BOX_TH2:
				case IDM_DISP_BOX_TH3:
				case IDM_DISP_BOX_TH4:
				case IDM_DISP_BOX_TH5:
				case IDM_DISP_BOX_CTL0:
				case IDM_DISP_BOX_CTL1:
				case IDM_DISP_BOX_INFO:
				case IDM_DISP_BOX_DUTY:
				case IDM_DISP_BOX_DUTYSUB:
				case IDM_DISP_BOX_STATE:
				case IDM_DISP_BOX_HTRMAIN:
				case IDM_DISP_BOX_HTRSUB:
				case IDM_DISP_BOX_PRESS:
				case IDM_DISP_BOX_IHCORE:
					for(int i = 0; i <= EANALYZ_LOG_NUM;i++){
						if (analyzItem[i].dispID == LOWORD(wParam)) {
							hMenu[i] = GetMenu(hWnd);
							uState[i] = GetMenuState(hMenu[i], analyzItem[i].dispID, MF_BYCOMMAND);
							if (uState[i] & MFS_CHECKED) {
								CheckMenuItem(hMenu[i], analyzItem[i].dispID, MF_BYCOMMAND | MFS_UNCHECKED);
								analyzItem[i].dispFlag = false;
							} else {
								CheckMenuItem(hMenu[i], analyzItem[i].dispID, MF_BYCOMMAND | MFS_CHECKED);
								analyzItem[i].dispFlag = true;
							}
							break;
						}
					}
					break;
				case IDM_CHOICE_BOX_TH1:
				case IDM_CHOICE_BOX_TH2:
				case IDM_CHOICE_BOX_TH3:
				case IDM_CHOICE_BOX_TH4:
				case IDM_CHOICE_BOX_TH5:
				case IDM_CHOICE_BOX_CTL0:
				case IDM_CHOICE_BOX_CTL1:
				case IDM_CHOICE_BOX_INFO:
				case IDM_CHOICE_BOX_DUTY:
				case IDM_CHOICE_BOX_DUTYSUB:
				case IDM_CHOICE_BOX_STATE:
				case IDM_CHOICE_BOX_HTRMAIN:
				case IDM_CHOICE_BOX_HTRSUB:
				case IDM_CHOICE_BOX_PRESS:
				case IDM_CHOICE_BOX_IHCORE:
					for (int j = 0;j <= EANALYZ_LOG_NUM;j++) {
						if(analyzItem[j].choiceID != LOWORD(wParam)){
							cMenu[j] = GetMenu(hWnd);
							cState[j] = GetMenuState(cMenu[j], analyzItem[j].choiceID, MF_BYCOMMAND);
							CheckMenuItem(cMenu[j], analyzItem[j].choiceID, MF_BYCOMMAND | MFS_UNCHECKED);
						}
					}
					for (int i = 0; i <= EANALYZ_LOG_NUM; i++) {
						if (analyzItem[i].choiceID == LOWORD(wParam)) {
							itemNum = i;
							cMenu[i] = GetMenu(hWnd);
							cState[i] = GetMenuState(cMenu[i], analyzItem[i].choiceID, MF_BYCOMMAND);
							CheckMenuItem(cMenu[i], analyzItem[i].choiceID, MF_BYCOMMAND | MFS_CHECKED);
						}
					}		
					break;
				default:
					break;
			}
			break;
		case WM_PAINT:
			if (analyzState == EANALYZ_ALL_PAINT) {
				if (logFileAnalyzState == EANALYZ_DROP_FILE) {
					readAllFile(hDrop, hmdc, hWnd);
				} else if (logFileAnalyzState == EANALYZ_SELECT_RANGE) {
					selectRange(hmdc, hWnd);
				} else {
				}
			} else {
				updateWindow(hWnd, hmdc);
			}
			break;
		case WM_LBUTTONDOWN:
			lButtonDown();
			break;
		case WM_MOUSEMOVE:	//マウスを動かしたとき
			mousePosX = (signed short)LOWORD(lParam);
			mousePosY = (signed short)HIWORD(lParam);
			if (mousePosX < 0) {
				movingX = 0;
			} else {
				movingX = mousePosX;
			}				
			movingY = mousePosY;
			mouseMove();
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case WM_LBUTTONUP:								//左クリックを離したとき
			lButtonUp(hDrop);
			break;
		case WM_DROPFILES:								//ファイルをドロップしたとき
			hDrop = (HDROP)wParam;
			DragQueryFile(hDrop, 0, (PTSTR)fileName, 200);
			allReset();
			readFile(hDrop);
			logFileAnalyzState = EANALYZ_DROP_FILE;
			DragFinish(hDrop);
			break;
		case WM_DESTROY:
			DeleteDC(hmdc);								// メモリDCの削除
			DeleteObject(hBitmap);						// ビットマップ オブジェクトの削除
#ifdef LOG_MODE
			PostQuitMessage(0);
#endif
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void fixLogInfo(char* form_msg, unsigned long timeCount, unsigned long value)
{
	bool isTimeUpdate = false;

	if (strncmp(form_msg, "TH1", 3) == 0) {
		timeTh1[posTh1] = timeCount;
		tempTh1[posTh1] = value;
		posTh1++;
		if (posTh1 >= TEMP_BUF_NUM) {
			posTh1 = 0;
			overTh1Flg = true;
		}
		isTimeUpdate = true;
	} else if (strncmp(form_msg, "TH2", 3) == 0) {
		timeTh2[posTh2] = timeCount;
		tempTh2[posTh2] = value;
		posTh2++;
		if (posTh2 >= TEMP_BUF_NUM) {
			posTh2 = 0;
			overTh2Flg = true;
		}
		isTimeUpdate = true;
	} else if (strncmp(form_msg, "TH3", 3) == 0) {
		timeTh3[posTh3] = timeCount;
		tempTh3[posTh3] = value;
		posTh3++;
		if (posTh3 >= TEMP_BUF_NUM) {
			posTh3 = 0;
			overTh3Flg = true;
		}
	} else if (strncmp(form_msg, "TH4", 3) == 0) {
		timeTh4[posTh4] = timeCount;
		tempTh4[posTh4] = value;
		posTh4++;
		if (posTh4 >= TEMP_BUF_NUM) {
			posTh4 = 0;
			overTh4Flg = true;
		}
	} else if (strncmp(form_msg, "TH5", 3) == 0) {
		timeTh5[posTh5] = timeCount;
		tempTh5[posTh5] = value;
		posTh5++;
		if (posTh5 >= TEMP_BUF_NUM) {
			posTh5 = 0;
			overTh5Flg = true;
		}
	} else if (strncmp(form_msg, "Ctl1", 4) == 0) {
		timeCtl1[posCtl1] = timeCount;
		tempCtl1[posCtl1] = value;
		posCtl1++;
		if (posCtl1 >= DATA_BUF_NUM) {
			posCtl1 = 0;
			overCtl1Flg = true;
		}
	} else if (strncmp(form_msg, "Ctl0", 4) == 0) {
		timeCtl0[posCtl0] = timeCount;
		tempCtl0[posCtl0] = value;
		posCtl0++;
		if (posCtl0 >= DATA_BUF_NUM) {
			posCtl0 = 0;
			overCtl0Flg = true;
		}
	} else if (strncmp(form_msg, "Info", 4) == 0) {
		timeInfo[posInfo] = timeCount;
		valueInfo[posInfo] = value;
		posInfo++;
		if (posInfo >= INFO_BUF_NUM) {
			posInfo = 0;
			overInfoFlg = true;
		}
	} else if (strncmp(form_msg, "Duty", 4) == 0) {
		timeDuty[posDuty] = timeCount;
		valueDuty[posDuty] = value;
		posDuty++;
		if (posDuty >= TEMP_BUF_NUM) {
			posDuty = 0;
			overDutyFlg = true;
		}
	} else if (strncmp(form_msg, "DSub", 4) == 0) {
		timeDutySub[posDutySub] = timeCount;
		valueDutySub[posDutySub] = value;
		posDutySub++;
		if (posDutySub >= TEMP_BUF_NUM) {
			posDutySub = 0;
			overDutySubFlg = true;
		}
	} else if (strncmp(form_msg, "Sta", 3) == 0) {
		timeState[posState] = timeCount;
		valueState[posState] = value;
		posState++;
		if (posState >= DATA_BUF_NUM) {
			posState = 0;
			overStateFlg = true;
		}
	} else if (strncmp(form_msg, "Alg0", 4) == 0) {
		timeHTRMain[posHTRMain] = timeCount;
		valueHTRMain[posHTRMain] = value;
		posHTRMain++;
		if (posHTRMain >= DATA_BUF_NUM) {
			posHTRMain = 0;
			overHTRMainFlg = true;
		}
	} else if (strncmp(form_msg, "Alg1", 4) == 0) {
		timeHTRSub[posHTRSub] = timeCount;
		valueHTRSub[posHTRSub] = value;
		posHTRSub++;
		if (posHTRSub >= DATA_BUF_NUM) {
			posHTRSub = 0;
			overHTRSubFlg = true;
		}
	} else if (strncmp(form_msg, "Pres", 4) == 0) {
		timePress[posPress] = timeCount;
		valuePress[posPress] = value;
		posPress++;
		if (posPress >= DATA_BUF_NUM) {
			posPress = 0;
			overPressFlg = true;
		}
	} else if (strncmp(form_msg, "Core", 4) == 0) {
		timeIhCore[posIhCore] = timeCount;
		valueIhCore[posIhCore] = value;
		posIhCore++;
		if (posIhCore >= DATA_BUF_NUM) {
			posIhCore = 0;
			overIhCoreFlg = true;
		}
	} else if (strncmp(form_msg, "FS:PaSize", 9) == 0) {
		timePaSize[posPaSize] = timeCount;
		paperSize[posPaSize] = value;
		posPaSize++;
		if (posPaSize >= DATA_BUF_NUM) {
			posPaSize = 0;
			overPaSizeFlg = true;
		}
	} else if (strncmp(form_msg, "FS:MediaType", 12) == 0) {
		timeMediaType[posMediaType] = timeCount;
		mediaType[posMediaType] = value;
		posMediaType++;
		if (posMediaType >= DATA_BUF_NUM) {
			posMediaType = 0;
			overMediaTypeFlg = true;
		}
	} else if (strncmp(form_msg, "FS:ColorMode", 12) == 0) {
		timeColorMode[posColorMode] = timeCount;
		colorMode[posColorMode] = value;
		posColorMode++;
		if (posColorMode >= DATA_BUF_NUM) {
			posColorMode = 0;
			overColorModeFlg = true;
		}
	} else if (strncmp(form_msg, "FS:PaWeigth", 11) == 0) {
		timePaWeigth[posPaWeight] = timeCount;
		paperWeigth[posPaWeight] = value;
		posPaWeight++;
		if (posPaWeight >= DATA_BUF_NUM) {
			posPaWeight = 0;
			overPaWeightFlg = true;
		}
	} else if (strncmp(form_msg, "FS:PaSpeed", 10) == 0) {
		timePaSpeed[posPaSpeed] = timeCount;
		printSpeed[posPaSpeed] = value;
		posPaSpeed++;
		if (posPaSpeed >= DATA_BUF_NUM) {
			posPaSpeed = 0;
			overPaSpeedFlg = true;
		}
	} else if (strncmp(form_msg, "FS:PaFeed", 9) == 0) {
		timePaFeed[posPaFeed] = timeCount;
		paperFpeed[posPaFeed] = value;
		posPaFeed++;
		if (posPaFeed >= DATA_BUF_NUM) {
			posPaFeed = 0;
			overPaFeedFlg = true;
		}
	} else if (strncmp(form_msg, "FS:PaSide", 9) == 0) {
		timePaSide[posPaSide] = timeCount;
		printSide[posPaSide] = value;
		posPaSide++;
		if (posPaSide >= DATA_BUF_NUM) {
			posPaSide = 0;
			overPaSideFlg = true;
		}
	} else if (strncmp(form_msg, "FS:LOG:", 7) == 0) {
		strcpy_s(wordAnyLog[posAnyLog], &form_msg[7]);
		timeAnyLog[posAnyLog] = timeCount;
		valueAnyLog[posAnyLog] = value;
		posAnyLog++;
		if (posAnyLog >= DATA_BUF_NUM) {
			posAnyLog = 0;
			overAnyFlg = true;
		}
	} else {
	}

	if (isTimeUpdate) {
		if ((timeCount / timeScale) + scaleDiff >= 700) {
			offSetTimePos = (timeCount / timeScale) + scaleDiff - 700;
		}
		if (timeCount < minCount) {
			minCount = timeCount;
		} 
		if (timeCount > maxCount) {
			maxCount = timeCount;
		}
	}
}

void updateWindow(HWND hWnd, HDC hdc)
{
	HDC hdc1;
	HDC hdc2;
	PAINTSTRUCT ps;
	HBRUSH hBrush;

	//LTGRAYブラシの作成&選択
	HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0xc0, 0xc0, 0xc0));
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
	SelectObject(hdc, hPen);
	hBrush = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	SelectObject(hdc, hBrush);
	Rectangle(hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGTH);

	//現在のツールバーの画像をメモリDCに転送
	hdc1 = GetDC(hWnd);
	BitBlt(hdc, 0, 0, WINDOW_WIDTH, 28, hdc1, 0, 0, SRCCOPY);
	ReleaseDC(hWnd, hdc1);

	SetBkMode(hdc, TRANSPARENT);

	dispAxisScale(hdc);
	dispAxis(hWnd, hdc);
	if (analyzItem[EANALYZ_LOG_TH1].dispFlag == true) {
		dispTh1(hdc);
	}
	if (analyzItem[EANALYZ_LOG_TH2].dispFlag == true) {
		dispTh2(hdc);
	}
	if (analyzItem[EANALYZ_LOG_TH3].dispFlag == true) {
		dispTh3(hdc);
	}
	if (analyzItem[EANALYZ_LOG_TH4].dispFlag == true) {
		dispTh4(hdc);
	}
	if (analyzItem[EANALYZ_LOG_TH5].dispFlag == true) {
		dispTh5(hdc);
	}
	if (analyzItem[EANALYZ_LOG_CTL0].dispFlag == true) {
		dispCtl0(hWnd, hdc);
	}
	if (analyzItem[EANALYZ_LOG_CTL1].dispFlag == true) {
		dispCtl1(hWnd, hdc);
	}
	if (analyzItem[EANALYZ_LOG_DUTY].dispFlag == true) {
		dispDuty(hdc);
	}
	if (analyzItem[EANALYZ_LOG_DUTYSUB].dispFlag == true) {
		dispDutySub(hdc);
	}
	if (analyzItem[EANALYZ_LOG_STATE].dispFlag == true) {
		dispState(hWnd, hdc);
	}
	if (analyzItem[EANALYZ_LOG_HTRMAIN].dispFlag == true) {
		dispHTRMain(hWnd, hdc);
	}
	if (analyzItem[EANALYZ_LOG_HTRSUB].dispFlag == true) {
		dispHTRSub(hWnd, hdc);
	}
	if (analyzItem[EANALYZ_LOG_PRESS].dispFlag == true) {
		dispPress(hdc);
	}
	if (analyzItem[EANALYZ_LOG_IHCORE].dispFlag == true) {
		dispIhCore(hdc);
	}
	if (analyzItem[EANALYZ_LOG_INFO].dispFlag == true) {
		dispInfo(hdc);
	}

	dispCrrentTemp(hdc);
	dispYData(hdc);
	dispHorizonalLine(hdc);
	dispVerticalLine(hdc);
	dispTimeWidth(hdc);
	dispPrintProfileInfo(hdc);
	dispAnyLog(hdc);
	hdc2 = BeginPaint(hWnd, &ps);
	// メモリDCから画像を転送
	BitBlt(hdc2, 0, 0, WINDOW_WIDTH, WINDOW_HEIGTH, hdc, 0, 0, SRCCOPY);

	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);

	EndPaint(hWnd, &ps);
}

//軸線の目盛の表示
void dispAxisScale(HDC hdc)
{
	HFONT hFont = CreateFont(17, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, VARIABLE_PITCH, NULL);
	SelectObject(hdc, hFont);

	//縦軸
	TextOut(hdc, 13, Y_0 - 50, " 50", 3);
	TextOut(hdc, 13, Y_0 - 100, "100", 3);
	TextOut(hdc, 13, Y_0 - 150, "150", 3);
	TextOut(hdc, 13, Y_0 - 200, "200", 3);
	TextOut(hdc, 13, Y_0 - 250, "250", 3);

	HPEN hPen = CreatePen(PS_DOT, 1, RGB(255, 255, 255));
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

	MoveToEx(hdc, 0, Y_0 - 50, NULL);
	LineTo(hdc, 800, Y_0 - 50);
	MoveToEx(hdc, 0, Y_0 - 100, NULL);
	LineTo(hdc, 800, Y_0 - 100);
	MoveToEx(hdc, 0, Y_0 - 150, NULL);
	LineTo(hdc, 800, Y_0 - 150);
	MoveToEx(hdc, 0, Y_0 - 200, NULL);
	LineTo(hdc, 800, Y_0 - 200);
	MoveToEx(hdc, 0, Y_0 - 250, NULL);
	LineTo(hdc, 800, Y_0 - 250);

	//横軸(時間軸）
	signed long i;
	unsigned short length;
	char str[10];
	signed long LinePos;
	signed long offSetSum = offSetTimePos - moveOffSet - scaleDiff;
	static unsigned short xAxisScale = 5;	//時間軸の設定

	xAxisScale = ((unsigned short)(timeScale - 1) / 50 + 1) * 5;
	for (i = 0; i <= (signed long)((WINDOW_WIDTH - X_0) / (pixelsMoved(xAxisScale))); i++) {
		LinePos = ((signed long)(convertCountsToTime(offSetSum * timeScale) / xAxisScale) * xAxisScale + (i * xAxisScale));

		if (LinePos >= 0){
			MoveToEx(hdc, xAxis(LinePos), Y_0, NULL);
			LineTo(hdc, xAxis(LinePos), 0);
			length = sprintf_s(str, sizeof(str), "%d", LinePos);
			TextOut(hdc, xAxis(LinePos), Y_0, str, length);
		} else {
		}
	}
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);
	DeleteObject(hFont);
}

//軸線の表示
void dispAxis(HWND hWnd, HDC hdc)
{
	HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
	HFONT hFont = CreateFont(12, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, VARIABLE_PITCH, NULL);
	SelectObject(hdc, hFont);

	MoveToEx(hdc, 0, Y_0, NULL);
	LineTo(hdc, WINDOW_WIDTH, Y_0);
	MoveToEx(hdc, X_0, 0, NULL);
	LineTo(hdc, X_0, WINDOW_HEIGTH);

	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);
	DeleteObject(hFont);
}

//Th1の描画関数
void dispTh1(HDC hdc)
{
	unsigned long i;
	HPEN hPen = CreatePen(PS_SOLID, 2, Th1Color);
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
	unsigned long startPos = (overTh1Flg == true ? posTh1 : 0);
	unsigned long num = (overTh1Flg == true ? TEMP_BUF_NUM : posTh1);
	unsigned long next; 

	MoveToEx(hdc, xPos(timeTh1[startPos]), yPos(tempTh1[startPos], 1), NULL);
	for (i = 0; i < num; i++) {
		next = ((startPos + i) % TEMP_BUF_NUM);
		LineTo(hdc, xPos(timeTh1[next]), yPos(tempTh1[next],1));
	}
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);
}

//Th2の描画関数
void dispTh2(HDC hdc)
{
	unsigned long i;
	HPEN hPen = CreatePen(PS_SOLID, 2, Th2Color);
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
	unsigned long startPos = (overTh2Flg == true ? posTh2 : 0);
	unsigned long num = (overTh2Flg == true ? TEMP_BUF_NUM : posTh2);
	unsigned long next;

	MoveToEx(hdc, xPos(timeTh2[startPos]), yPos(tempTh2[startPos], 1), NULL);
	for (i = 0; i < num; i++) {
		next = ((startPos + i) % TEMP_BUF_NUM);
		LineTo(hdc, xPos(timeTh2[next]), yPos(tempTh2[next], 1));
	}
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);
}

//Th3の描画関数
void dispTh3(HDC hdc)
{
	unsigned long i;
	HPEN hPen = CreatePen(PS_SOLID, 2, Th3Color);
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
	unsigned long startPos = (overTh3Flg == true ? posTh3 : 0);
	unsigned long num = (overTh3Flg == true ? TEMP_BUF_NUM : posTh3);
	unsigned long next;

	MoveToEx(hdc, xPos(timeTh3[startPos]), yPos(tempTh3[startPos], 1), NULL);
	for (i = 0; i < num; i++) {
		next = ((startPos + i) % TEMP_BUF_NUM);
		LineTo(hdc, xPos(timeTh3[next]), yPos(tempTh3[next], 1));
	}
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);
}

//Th4の描画関数
void dispTh4(HDC hdc)
{
	unsigned long i;
	HPEN hPen = CreatePen(PS_SOLID, 1, Th4Color);
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
	unsigned long startPos = (overTh4Flg == true ? posTh4 : 0);
	unsigned long num = (overTh4Flg == true ? TEMP_BUF_NUM : posTh4);
	unsigned long next;

	MoveToEx(hdc, xPos(timeTh4[startPos]), yPos(tempTh4[startPos],1), NULL);
	for (i = 0; i < num; i++) {
		next = ((startPos + i) % TEMP_BUF_NUM);
		LineTo(hdc, xPos(timeTh4[next]), yPos(tempTh4[next], 1));
	}
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);
}

//Th5の描画関数
void dispTh5(HDC hdc)
{
	unsigned long i;
	HPEN hPen = CreatePen(PS_SOLID, 2, Th5Color);
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
	unsigned long startPos = (overTh5Flg == true ? posTh5 : 0);
	unsigned long num = (overTh5Flg == true ? TEMP_BUF_NUM : posTh5);
	unsigned long next;

	MoveToEx(hdc, xPos(timeTh5[startPos]), yPos(tempTh5[startPos],1), NULL);
	for (i = 0; i < num; i++) {
		next = ((startPos + i) % TEMP_BUF_NUM);
		LineTo(hdc, xPos(timeTh5[next]), yPos(tempTh5[next], 1));
	}
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);
}

//目標温度の表示
void dispCtl0(HWND hWnd, HDC hdc)
{
	unsigned long i;
	HPEN hPen = CreatePen(PS_SOLID, 2, Ctl0Color);
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
	unsigned long num = (overCtl0Flg == true ? DATA_BUF_NUM : posCtl0);
	unsigned long startPos = (overCtl0Flg == true ? posCtl0 : 0);
	unsigned long pos;
	unsigned long next;

	if (num > 0) {
		MoveToEx(hdc, xPos(timeCtl0[startPos]), yPos(tempCtl0[startPos], 1), NULL);
		pos = startPos;
		for (i = 1; i < num; i++) {
			next = (startPos + i) % DATA_BUF_NUM;
			LineTo(hdc, xPos(timeCtl0[next]), yPos(tempCtl0[pos], 1));
			LineTo(hdc, xPos(timeCtl0[next]), yPos(tempCtl0[next], 1));
			pos = next;
		}
		LineTo(hdc, WINDOW_WIDTH, yPos(tempCtl0[pos], 1));
	}
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);
}

//目標温度の表示
void dispCtl1(HWND hWnd, HDC hdc)
{
	unsigned long i;
	HPEN hPen = CreatePen(PS_SOLID, 2, Ctl1Color);
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
	unsigned long num = (overCtl1Flg == true ? DATA_BUF_NUM : posCtl1);
	unsigned long startPos = (overCtl1Flg == true ? posCtl1 : 0);
	unsigned long pos;
	unsigned long next;
	
	if (num > 0) {
		MoveToEx(hdc, xPos(timeCtl1[startPos]), yPos(tempCtl1[startPos], 1), NULL);
		pos = startPos;
		for (i = 1; i < num; i++) {
			next = (startPos + i) % DATA_BUF_NUM;
			LineTo(hdc, xPos(timeCtl1[next]), yPos(tempCtl1[pos], 1));
			LineTo(hdc, xPos(timeCtl1[next]), yPos(tempCtl1[next], 1));
			pos = next;
		}
		LineTo(hdc, WINDOW_WIDTH, yPos(tempCtl1[pos], 1));
	}	
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);
}

//Infoの描画関数
void dispInfo(HDC hdc)
{
	unsigned long i;
	unsigned short logPos = 0;
	unsigned short dispNum = 5;//１列に表示させるInfoの数値の数

	HPEN hPen = CreatePen(PS_SOLID, 7, InfoColor);
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
	unsigned long infoNum = (overInfoFlg == true ? INFO_BUF_NUM : posInfo);

	for(i = 0;i < infoNum;i++){
		Ellipse(hdc, xPos(timeInfo[i]) - 1, yPos(valueInfo[i], 1) - 1, xPos(timeInfo[i]) + 1, yPos(valueInfo[i], 1) + 1);
	}
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);
}

//Dutyの描画関数
void dispDuty(HDC hdc)
{
	unsigned long i;
	HPEN hPen = CreatePen(PS_SOLID, 6, DutyColor);
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
	unsigned long num = (overDutyFlg == true ? TEMP_BUF_NUM : posDuty);

	for (i = 0; i < num; i++) {
		Ellipse(hdc, xPos(timeDuty[i]) - 1, yPos(valueDuty[i], 1) - 1, xPos(timeDuty[i]) + 1, yPos(valueDuty[i], 1) + 1);
	}
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);
}

//DutySubの描画関数
void dispDutySub(HDC hdc)
{
	unsigned long i;
	HPEN hPen = CreatePen(PS_SOLID, 3, DutySubColor);
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
	unsigned long num = (overDutySubFlg == true ? TEMP_BUF_NUM : posDutySub);

	for (i = 0; i < num; i++) {
		Ellipse(hdc, xPos(timeDutySub[i]) - 1, yPos(valueDutySub[i], 1) - 1, xPos(timeDutySub[i]) + 1, yPos(valueDutySub[i], 1) + 1);
	}
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);
}

//状態の表示
void dispState(HWND hWnd, HDC hdc)
{
	unsigned long i;
	HPEN hPen = CreatePen(PS_SOLID, 2, StateColor);
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
	unsigned long num = (overStateFlg == true ? DATA_BUF_NUM : posState);
	unsigned long startPos = (overStateFlg == true ? posState : 0);
	unsigned long pos;
	unsigned long next;

	if (num > 0) {
		MoveToEx(hdc, xPos(timeState[startPos]), yPos(valueState[startPos],2), NULL);
		pos = startPos;
		for (i = 1; i < num; i++) {
			next = (startPos + i) % DATA_BUF_NUM;
			LineTo(hdc, xPos(timeState[next]), yPos(valueState[pos], 2));
			LineTo(hdc, xPos(timeState[next]), yPos(valueState[next], 2));
			pos = next;
		}
		LineTo(hdc, WINDOW_WIDTH, yPos(valueState[pos], 2));
	}
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);
}

//メインヒーターの状態の表示
void dispHTRMain(HWND hWnd, HDC hdc)
{
	unsigned long i;
	HPEN hPen = CreatePen(PS_SOLID, 1, HTRMainColor);
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
	unsigned long num = (overHTRMainFlg == true ? DATA_BUF_NUM : posHTRMain);
	unsigned long startPos = (overHTRMainFlg == true ? posHTRMain : 0);
	unsigned long pos;
	unsigned long next;

	if (num > 0) {
		MoveToEx(hdc, xPos(timeHTRMain[startPos]), yPos(valueHTRMain[startPos], 2), NULL);
		pos = startPos;
		for (i = 1; i < num; i++) {
			next = (startPos + i) % DATA_BUF_NUM;
			LineTo(hdc, xPos(timeHTRMain[next]), yPos(valueHTRMain[pos], 2));
			LineTo(hdc, xPos(timeHTRMain[next]), yPos(valueHTRMain[next], 2));
			pos = next;
		}
		LineTo(hdc, WINDOW_WIDTH, yPos(valueHTRMain[pos], 2));
	}
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);
}

//サブヒーターの状態の表示
void dispHTRSub(HWND hWnd, HDC hdc)
{
	unsigned long i;
	HPEN hPen = CreatePen(PS_SOLID, 2, HTRSubColor);
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
	unsigned long num = (overHTRSubFlg == true ? DATA_BUF_NUM : posHTRSub);
	unsigned long startPos = (overHTRSubFlg == true ? posHTRSub : 0);
	unsigned long pos;
	unsigned long next;

	if (num > 0) {
		MoveToEx(hdc, xPos(timeHTRSub[startPos]), yPos(valueHTRSub[startPos], 2), NULL);
		pos = startPos;
		for (i = 1; i < num; i++) {
			next = (startPos + i) % DATA_BUF_NUM;
			LineTo(hdc, xPos(timeHTRSub[next]), yPos(valueHTRSub[pos], 2));
			LineTo(hdc, xPos(timeHTRSub[next]), yPos(valueHTRSub[next], 2));
			pos = next;
		}
		LineTo(hdc, WINDOW_WIDTH, yPos(valueHTRSub[pos], 2));
	}
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);
}

//圧解除制御の状態表示
void dispPress(HDC hdc)
{
	unsigned long i;
	HPEN hPen = CreatePen(PS_SOLID, 7, PressColor);
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
	unsigned long num = (overPressFlg == true ? DATA_BUF_NUM : posPress);

	for (i = 0; i < num; i++) {
		Ellipse(hdc, xPos(timePress[i]) - 1, yPos(valuePress[i], 2) - 1, xPos(timePress[i]) + 1, yPos(valuePress[i], 2) + 1);
	}
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);
}

//IHCore動作
void dispIhCore(HDC hdc)
{
	unsigned long i;
	HPEN hPen = CreatePen(PS_SOLID, 7, IhCoreColor);
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
	unsigned long num = (overIhCoreFlg == true ? DATA_BUF_NUM : posIhCore);

	for (i = 0; i < num; i++) {
		Ellipse(hdc, xPos(timeIhCore[i]) - 1, yPos(valueIhCore[i], 2) - 1, xPos(timeIhCore[i]) + 1, yPos(valueIhCore[i], 2) + 1);
	}
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);
}

//現在温度の表示
void dispCrrentTemp(HDC hdc)
{
	#define THERM_NUM 5
	unsigned short length;
	char str[10];
	unsigned long pos;
	unsigned long yPosTh[THERM_NUM];
	const bool* overFlag[THERM_NUM] = {&overTh1Flg, &overTh2Flg, &overTh3Flg, &overTh4Flg, &overTh5Flg}; 
	const unsigned long* currentPos[THERM_NUM] = {&posTh1, &posTh2, &posTh3, &posTh4, &posTh5};
	const unsigned long* currentTemp[THERM_NUM] = {&tempTh1[0], &tempTh2[0], &tempTh3[0], &tempTh4[0], &tempTh5[0]};
	const unsigned long* currentTime[THERM_NUM] = {&timeTh1[0], &timeTh2[0], &timeTh3[0], &timeTh4[0], &timeTh5[0]};
	const unsigned long colorTh[THERM_NUM] = {Th1Color, Th2Color, Th3Color, Th4Color, Th5Color};
	const unsigned char dispFlagNum[THERM_NUM] = {EANALYZ_LOG_TH1, EANALYZ_LOG_TH2, EANALYZ_LOG_TH3, EANALYZ_LOG_TH4, EANALYZ_LOG_TH5};
	const unsigned char thNum[THERM_NUM] = {1, 2, 3, 4, 5};
	unsigned char jun[THERM_NUM] = {0, 1, 2, 3, 4};
	unsigned long temp[THERM_NUM] = {0xffff, 0xffff, 0xffff, 0xffff, 0xffff};
	unsigned char jn;
	unsigned char i;
	unsigned char j;

	for (i = 0; i < THERM_NUM; i++) {
		pos = (*overFlag[i] == true ? (*currentPos[i] == 0 ? TEMP_BUF_NUM - 1 : *currentPos[i] - 1) : (*currentPos[i] == 0 ? TEMP_BUF_NUM : *currentPos[i] - 1) );
		if (pos != TEMP_BUF_NUM) {
			temp[i] = currentTemp[i][pos];
		}
	}

	//温度が小さい順を決める
	for (j = 0; j < THERM_NUM - 1; j++) {
		for (i = j + 1; i < THERM_NUM; i++) {
			if (temp[jun[j]] > temp[i]) {
				jn = jun[j];
				jun[j] = jun[i];
				jun[i] = jn;
			}
		}
	}
	for (j = 0; j < THERM_NUM; j++) {
		i = jun[j];
		pos = (*overFlag[i] == true ? (*currentPos[i] == 0 ? TEMP_BUF_NUM - 1 : *currentPos[i] - 1) : (*currentPos[i] == 0 ? TEMP_BUF_NUM : *currentPos[i] - 1));
		if (pos != TEMP_BUF_NUM) {			
			length = sprintf_s(str, sizeof(str), "TH%d:%d", thNum[i], currentTemp[i][pos]);
			SetTextColor(hdc, colorTh[i]);
			yPosTh[j] = currentTemp[i][pos];
			if (j != 0) {
				if (currentTemp[i][pos] < yPosTh[j - 1] + 10) {
					yPosTh[j] = yPosTh[j - 1] + 10;
				}
			}
			if (analyzItem[dispFlagNum[i]].dispFlag == true) {
				TextOut(hdc, xPos(currentTime[i][pos]), yPos(yPosTh[j], 1), str, length);
			}
		}
	}
	SetTextColor(hdc, RGB(0, 0, 0));
}

//値表示ボタンの処理
void dispYData(HDC hdc)
{
	unsigned short length;
	char str[5];
	unsigned long i;
	unsigned long textNum = (overYDataFlg == true ? DATA_BUF_NUM : posYData);

	HFONT hFont = CreateFont(10, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, VARIABLE_PITCH, NULL);
	SelectObject(hdc, hFont);
	HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
	
	if (analyzState == EANALYZ_WATCH_YDATA) {
		MoveToEx(hdc, xPos(timeYData1[posYData]), yPos(valueYData1[posYData], valueYAxis[posYData]), NULL);
		LineTo(hdc, xPos(timeYData1[posYData]) - 5, yPos(valueYData1[posYData], valueYAxis[posYData]) - 15);
		length = sprintf_s(str, sizeof(str), "%d", valueYData1[posYData]);
		TextOut(hdc, xPos(timeYData1[posYData]) - 15, yPos(valueYData1[posYData], valueYAxis[posYData]) - 30, str, length);
	} else if (analyzState == EANALYZ_WATCH_YDATA_LINE) {
		MoveToEx(hdc, xPos(timeYData1[posYData]), yPos(valueYData1[posYData], valueYAxis[posYData]), NULL);
		LineTo(hdc, movingX, movingY);
		length = sprintf_s(str, sizeof(str), "%d", valueYData1[posYData]);
		if (yPos(valueYData1[posYData], valueYAxis[posYData]) <= movingY) {
			TextOut(hdc, movingX - 10, movingY + 6, str, length);
		} else {
			TextOut(hdc, movingX - 10, movingY - 10, str, length);
		}	
	} else {
	}
	for (i = 0; i < textNum; i++) {
		if (i != posYData) {
			MoveToEx(hdc, xPos(timeYData1[i]), yPos(valueYData1[i], valueYAxis[i]), NULL);
			LineTo(hdc, xPos(timeYData2[i]), yPos(valueYData2[i], 1));
			length = sprintf_s(str, sizeof(str), "%d", valueYData1[i]);
			if (yPos(valueYData1[i], valueYAxis[i]) <= yPos(valueYData2[i], 1)) {
				TextOut(hdc, xPos(timeYData2[i]) - 5, yPos(valueYData2[i], 1) + 6, str, length);
			} else {
				TextOut(hdc, xPos(timeYData2[i]) - 5, yPos(valueYData2[i], 1) - 10, str, length);
			}
		}
	}
	SelectObject(hdc, GetStockObject(SYSTEM_FONT));
	DeleteObject(hFont);
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);
}

//縦線ボタンの処理
void dispVerticalLine(HDC hdc)
{
	unsigned long i;
	unsigned long textNum = (overVerticalFlg == true ? DATA_BUF_NUM : posVertical);

	HPEN hPen = CreatePen(PS_SOLID, 1, RGB(100, 100, 100));
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

	if (analyzState == EANALYZ_VERTICAL) {
		MoveToEx(hdc, xPos(timeVertical[posVertical]), 0, NULL);
		LineTo(hdc, xPos(timeVertical[posVertical]), Y_0);
	} else {
	}
	for (i = 0; i < textNum; i++) {
		if (i != posVertical) {
			MoveToEx(hdc, xPos(timeVertical[i]), 0, NULL);
			LineTo(hdc, xPos(timeVertical[i]), Y_0);
		}	
	}
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);
}

//横線ボタンの処理
void dispHorizonalLine(HDC hdc)
{
	unsigned long i;
	unsigned long textNum = (overHorizonalFlg == true ? DATA_BUF_NUM : posHorizonal);

	HPEN hPen = CreatePen(PS_SOLID, 1, RGB(100, 100, 100));
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

	if (analyzState == EANALYZ_HORIZONAL) {
		MoveToEx(hdc, 0, yPos(valueHorizonal[posHorizonal], valueHorizonalAxis[posHorizonal] = analyzItem[itemNum].axis), NULL);
		LineTo(hdc, WINDOW_WIDTH, yPos(valueHorizonal[posHorizonal], valueHorizonalAxis[posHorizonal] = analyzItem[itemNum].axis));
	} else {
	}
	for (i = 0; i < textNum; i++) {
		if (i != posHorizonal){
			MoveToEx(hdc, 0, yPos(valueHorizonal[i], valueHorizonalAxis[i]), NULL);
			LineTo(hdc, WINDOW_WIDTH, yPos(valueHorizonal[i], valueHorizonalAxis[i]));
		}
	}
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);
}

//幅カーソルボタンの処理
void dispTimeWidth(HDC hdc)
{
	unsigned long i;
	float resultTime;
	unsigned long textNum = (overTimeWidthFlg == true ? DATA_BUF_NUM : posTimeWidth);
	unsigned short length;
	char str[10];
	
	HFONT hFont = CreateFont(10, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, VARIABLE_PITCH, NULL);
	SelectObject(hdc, hFont);
	HPEN hPen = CreatePen(PS_SOLID, 1, RGB(100, 100, 100));
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

	if (analyzState == EANALYZ_TIME_WIDTH) {
		MoveToEx(hdc, xPos(timeWidth1[posTimeWidth]), WINDOW_HEIGTH, NULL);
		LineTo(hdc, xPos(timeWidth1[posTimeWidth]), 20);
	} else if (analyzState == EANALYZ_TIME_WIDTH_LINE) {
		MoveToEx(hdc, xPos(timeWidth1[posTimeWidth]), WINDOW_HEIGTH, NULL);
		LineTo(hdc, xPos(timeWidth1[posTimeWidth]), 20);
		MoveToEx(hdc, xPos(timeWidth1[posTimeWidth]), movingY, NULL);
		LineTo(hdc, xPos(timeWidth2[posTimeWidth]), movingY);
		MoveToEx(hdc, xPos(timeWidth2[posTimeWidth]), WINDOW_HEIGTH, NULL);
		LineTo(hdc, xPos(timeWidth2[posTimeWidth]), 20);
		if (timeWidth1[posTimeWidth] >= timeWidth2[posTimeWidth]) {
			resultTime = convertCountsToTime(timeWidth1[posTimeWidth] - timeWidth2[posTimeWidth]);
		} else {
			resultTime = convertCountsToTime(timeWidth2[posTimeWidth] - timeWidth1[posTimeWidth]);
		}
		length = sprintf_s(str, sizeof(str), "%.2fs", resultTime);
		TextOut(hdc, xPos((unsigned long)timeWidth3[posTimeWidth]) - 10, yPosTimeWidth[posTimeWidth] - 15, str, length);
	} else {
	}
	for (i = 0; i < textNum; i++) {
		if (i != posTimeWidth) {
			//X座標の比較（時間の幅の計算）
			if (timeWidth1[i] >= timeWidth2[i]) {
				resultTime = convertCountsToTime(timeWidth1[i] - timeWidth2[i]);
			} else {
				resultTime = convertCountsToTime(timeWidth2[i] - timeWidth1[i]);
			}
			timeWidth3[i] = (float)(timeWidth1[i] + timeWidth2[i]) / 2;

			//Y座標の比較（値の高さの比較と線を引く処理）
			MoveToEx(hdc, xPos(timeWidth1[i]), yPosTimeWidth[i], NULL);
			LineTo(hdc, xPos(timeWidth2[i]), yPosTimeWidth[i]);
			MoveToEx(hdc, xPos(timeWidth1[i]), yPos(valueTimeWidth1[i], valueTimeWidthAxis[i]), NULL);
			if (yPosTimeWidth[i] >= yPos(valueTimeWidth1[i], valueTimeWidthAxis[i])) {
				LineTo(hdc, xPos(timeWidth1[i]), yPosTimeWidth[i] + 10);
			} else {
				LineTo(hdc, xPos(timeWidth1[i]), yPosTimeWidth[i] - 10);
			}
			MoveToEx(hdc, xPos(timeWidth2[i]), yPos(valueTimeWidth2[i], valueTimeWidthAxis[i]), NULL);
			if (yPosTimeWidth[i] >= yPos(valueTimeWidth2[i], valueTimeWidthAxis[i])) {
				LineTo(hdc, xPos(timeWidth2[i]), yPosTimeWidth[i] + 10);
			} else {
				LineTo(hdc, xPos(timeWidth2[i]), yPosTimeWidth[i] - 10);
			}
			length = sprintf_s(str, sizeof(str), "%.2fs", resultTime);
			TextOut(hdc, xPos((unsigned long)timeWidth3[i]) - 10, yPosTimeWidth[i] - 15, str, length);
		}
	}	
	SelectObject(hdc, GetStockObject(SYSTEM_FONT));
	DeleteObject(hFont);
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);
}

//紙の情報の表示
void dispPrintProfileInfo(HDC hdc)
{
	HPEN hPen = CreatePen(PS_SOLID, 6, RGB(0, 0, 0));
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
	HFONT hFont = CreateFont(12, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, VARIABLE_PITCH, NULL);
	SelectObject(hdc, hFont);
	unsigned short length;
	char str[20];
	unsigned long paperSizeNum = (overPaSizeFlg == true ? DATA_BUF_NUM : posPaSize);
	unsigned long mediaTypeNum = (overMediaTypeFlg == true ? DATA_BUF_NUM : posMediaType);
	unsigned long paperWeightNum = (overPaWeightFlg == true ? DATA_BUF_NUM : posPaWeight);
	unsigned long colorModeNum = (overColorModeFlg == true ? DATA_BUF_NUM : posColorMode);
	unsigned long printSpeedNum = (overPaSpeedFlg == true ? DATA_BUF_NUM : posPaSpeed);
	unsigned long paperFeedNum = (overPaFeedFlg == true ? DATA_BUF_NUM : posPaFeed);
	unsigned long paperSideNum = (overPaSideFlg == true ? DATA_BUF_NUM : posPaSide);
	unsigned long dispPos;

	//紙のサイズ
	dispPos = Y_0 + 23;
	for (unsigned long i = 0; i < paperSizeNum; i++) {
		for (int j = 0; j < PAPER_SIZE_NUM; j++) {
			if (paperSize[i] == dispPaperSize[j].paperSizeId) {
				length = sprintf_s(str, sizeof(str), "%s", dispPaperSize[j].paperSizeName);
				break;
			}
		}
		TextOut(hdc, xPos(timePaSize[i]) + 5, dispPos - 5, str, length);
		Ellipse(hdc, xPos(timePaSize[i]) + 1, dispPos - 1, xPos(timePaSize[i]) - 1, dispPos + 1);
	}

	//紙の種類 メディアタイプ
	dispPos = Y_0 + 35;
	for (unsigned long i = 0; i < mediaTypeNum; i++) {
		for (int j = 0; j < MEDIA_TYPE_NUM; j++) {
			if (mediaType[i] == dispMediaType[j].mediaTypeId) {
				length = sprintf_s(str, sizeof(str), "%s", dispMediaType[j].mediaTypeName);
				break;
			}
		}
		TextOut(hdc, xPos(timeMediaType[i]) + 5, dispPos - 6, str, length);
		Ellipse(hdc, xPos(timeMediaType[i]) + 1, dispPos - 1, xPos(timeMediaType[i]) - 1, dispPos + 1);
	}

	//紙の重さ
	dispPos = Y_0 + 47;
	for (unsigned long i = 0; i < paperWeightNum; i++) {
		for (int j = 0; j < PAPER_WEIGTH_NUM; j++) {
			if (paperWeigth[i] == dispPaperWeigth[j].paperWeigthId) {
				length = sprintf_s(str, sizeof(str), "%s", dispPaperWeigth[j].paperWeigthName);
				break;
			}
		}
		TextOut(hdc, xPos(timePaWeigth[i]) + 5, dispPos - 7, str, length);
		Ellipse(hdc, xPos(timePaWeigth[i]) + 1, dispPos - 1, xPos(timePaWeigth[i]) - 1, dispPos + 1);
	}

	//カラーモード
	dispPos = Y_0 + 59;
	for (unsigned long i = 0; i < colorModeNum; i++) {
		for (int j = 0; j < COLOR_MODE_NUM; j++) {
			if (colorMode[i] == dispColorMode[j].colorModeId) {
				length = sprintf_s(str, sizeof(str), "%s", dispColorMode[j].colorModeName);
				break;
			}
		}
		TextOut(hdc, xPos(timeColorMode[i]) + 5, dispPos - 7, str, length);
		Ellipse(hdc, xPos(timeColorMode[i]) + 1, dispPos - 1, xPos(timeColorMode[i]) - 1, dispPos + 1);
	}
	//紙の印刷速度
	dispPos = Y_0 + 71;
	for (unsigned long i = 0; i < printSpeedNum; i++) {
		for (int j = 0; j < PRINT_SPEED_NUM; j++) {
			if (printSpeed[i] == dispPrintSpeed[j].printSpeedId) {
				length = sprintf_s(str, sizeof(str), "%s", dispPrintSpeed[j].printSpeedName);
				break;
			}
		}
		TextOut(hdc, xPos(timePaSpeed[i]) + 5, dispPos - 7, str, length);
		Ellipse(hdc, xPos(timePaSpeed[i]) + 1, dispPos - 1, xPos(timePaSpeed[i]) - 1, dispPos + 1);
	}
	//給紙段
	dispPos = Y_0 + 83;
	for (unsigned long i = 0; i < paperFeedNum; i++) {
		for (int j = 0; j < PAPER_FEED_NUM; j++) {
			if (paperFpeed[i] == dispPaperFeed[j].paperFeedId) {
				length = sprintf_s(str, sizeof(str), "%s", dispPaperFeed[j].paperFeedName);
				break;
			}
		}
		TextOut(hdc, xPos(timePaFeed[i]) + 5, dispPos - 7, str, length);
		Ellipse(hdc, xPos(timePaFeed[i]) + 1, dispPos - 1, xPos(timePaFeed[i]) - 1, dispPos + 1);
	}
	//片面両面
	dispPos = Y_0 + 95;
	for (unsigned long i = 0; i < paperSideNum; i++) {
		for (int j = 0; j < PAPER_SIDE_NUM; j++) {
			if (printSide[i] == dispPaperSide[j].paperSideId) {
				length = sprintf_s(str, sizeof(str), "%s", dispPaperSide[j].paperSideName);
				break;
			}
		}
		TextOut(hdc, xPos(timePaSide[i]) + 5, dispPos - 7, str, length);
		Ellipse(hdc, xPos(timePaSide[i]) + 1, dispPos - 1, xPos(timePaSide[i]) - 1, dispPos + 1);
	}

	SelectObject(hdc, GetStockObject(SYSTEM_FONT));
	DeleteObject(hFont);
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);
}

//任意のログの表示
void dispAnyLog(HDC hdc)
{
	HPEN hPen = CreatePen(PS_SOLID, 6, RGB(0, 0, 0));
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
	HFONT hFont = CreateFont(12, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, VARIABLE_PITCH, NULL);
	SelectObject(hdc, hFont);

	unsigned long anyLogNum = (overAnyFlg == true ? DATA_BUF_NUM : posAnyLog);
	unsigned short length;
	char str[20];

	//任意のログ
	for (unsigned long i = 0; i < anyLogNum; i++) {
		length = sprintf_s(str, sizeof(str), "%s", wordAnyLog[i]);
		TextOut(hdc, xPos(timeAnyLog[i]) + 5,yPos(valueAnyLog[i], 1) - 5, str, length);
		Ellipse(hdc, xPos(timeAnyLog[i]) + 1, yPos(valueAnyLog[i], 1) - 1, xPos(timeAnyLog[i]) - 1, yPos(valueAnyLog[i], 1) + 1);
	}

	SelectObject(hdc, GetStockObject(SYSTEM_FONT));
	DeleteObject(hFont);
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);
}

//全消しボタン
void allReset()
{
	for (int i = 0; i < EANALYZ_LOG_NUM + EANALYZ_TOOL_NUM + EANALYZ_PRINT_PROFILE_NUM; i++) {
		*(itemReset[i].pos) = 0;
		*(itemReset[i].flg) = false;
	}
	offSetTimePos = 0;
	offSetPlus = 0;

	timeScale = 20;
	analyzOffSet1 = 0;
	analyzOffSet2 = 0;
	moveOffSet = 0;	//カーソル移動に伴う画面移動のピクセル数
	scaleDiffCount = 0;		//拡大縮小時 変更前の中心のカウント値
	scaleDiff = 0;

	maxCount = 0;
	minCount = 0xffffffff;

	for (char i = 0; i < EANALYZ_LOG_NUM;i++) {
		firstLogFlg[i] = true;
	}

	analyzState = EANALYZ_NORMAL;
	logFileAnalyzState = EANALYZ_IDLE;
}

//自動スクロール機能
void autoScroll()
{
	if ((analyzState == EANALYZ_TIME_WIDTH_LINE) && (clickState == EANALYZ_MOUSE_UP)) {
		if ((movingX > 700) && (movingX <= WINDOW_WIDTH)) {
			analyzOffSet2 -= 10;
			moveOffSet = analyzOffSet1 + analyzOffSet2;
		} else if ((movingX >= 0) && (movingX < 100)) {
			if ((signed long)offSetTimePos - (moveOffSet + 10) < 0) {
				moveOffSet = offSetTimePos;
			} else {
				analyzOffSet2 += 10;
				moveOffSet = analyzOffSet1 + analyzOffSet2;
			}
		}
	}
}

//ドラックドロップでスクロール
void dragScroll()
{
	analyzOffSet2 += analyzOffSet1;
	if ((signed long)offSetTimePos - moveOffSet - scaleDiff < 0) {
		analyzOffSet2 = offSetTimePos - scaleDiff;
	}
	analyzOffSet1 = 0;
	moveOffSet = analyzOffSet1 + analyzOffSet2;
}

//マウスが動いているときの処理
void mouseMove()
{
	if (clickState != EANALYZ_MOUSE_UP) {
		analyzOffSet1 = (movingX - cursorDownX);
		moveOffSet = analyzOffSet1 + analyzOffSet2;
		//マウスの動きが少しの場合、EANALYZ_MOUSE_MOVEに移行しない
		if ((-CLICK_STATE_ADJ <= analyzOffSet1) && (analyzOffSet1 <= CLICK_STATE_ADJ)) {
		} else {
			clickState = EANALYZ_MOUSE_MOVE;
		}
	}
	switch (analyzState) {
		case EANALYZ_WATCH_YDATA:
		case EANALYZ_TIME_WIDTH:
		case EANALYZ_TIME_WIDTH_LINE:
		case EANALYZ_VERTICAL:
		case EANALYZ_HORIZONAL:
			calculateNearestValue();
			break;
		default:
			break;
	}
}

//左クリックを押したときの処理
void lButtonDown()
{
	clickState = EANALYZ_MOUSE_DOWN;
	if (analyzState != EANALYZ_ALL_PAINT) {
		cursorDownX = movingX;
	}	
}

//左クリックを離したときの処理
void lButtonUp(HDROP hDrop)
{
	dragScroll();
	if (clickState == EANALYZ_MOUSE_MOVE) {
		clickState = EANALYZ_MOUSE_UP;
		return;
	}
	clickState = EANALYZ_MOUSE_UP;
	switch (analyzState){
		case EANALYZ_WATCH_YDATA:
			analyzState = EANALYZ_WATCH_YDATA_LINE;
			break;
		case EANALYZ_WATCH_YDATA_LINE:
			//値をバッファに溜める
			timeYData2[posYData] = inverseXPos(movingX);
			valueYData2[posYData] = inverseYPos(movingY);
			posYData++;
			if (posYData >= DATA_BUF_NUM) {
				posYData = 0;
				overYDataFlg = true;
			}
			calculateNearestValue();
			analyzState = EANALYZ_WATCH_YDATA;
			break;
		case EANALYZ_VERTICAL:
			//値をバッファに溜める
			calculateNearestValue();
			posVertical++;
			if (posVertical >= DATA_BUF_NUM) {
				posVertical = 0;
				overVerticalFlg = true;
			}
			break;
		case EANALYZ_HORIZONAL:
			//値をバッファに溜める
			calculateNearestValue();
			posHorizonal++;
			if (posHorizonal >= DATA_BUF_NUM) {
				posHorizonal = 0;
				overHorizonalFlg = true;
			}
			break;
		case EANALYZ_TIME_WIDTH:
			calculateNearestValue();
			analyzState = EANALYZ_TIME_WIDTH_LINE;
			break;
		case EANALYZ_TIME_WIDTH_LINE:
			posTimeWidth++;
			if (posTimeWidth >= DATA_BUF_NUM) {
				posTimeWidth = 0;
				overTimeWidthFlg = true;
			}
			calculateNearestValue();
			analyzState = EANALYZ_TIME_WIDTH;
			break;
		case EANALYZ_ALL_PAINT:
			if (logFileAnalyzState == EANALYZ_SELECT_RANGE) {
				if (movingX >= X_0) {
					firstReadLogTime = inverseXPos(movingX);
				} else {
					firstReadLogTime = 0;
				}				
				allReset();
				logFileAnalyzState = EANALYZ_FILE_RELOAD;
				readFile(hDrop);
				dispAllButton(hDrop);
				logFileAnalyzState = EANALYZ_IDLE;
			}
			break;
		default:
			break;
	}
}

//USBログの温度情報を、すでに存在する温度ログのバッファーに入れる（表示時刻順になるように、間に埋め込んでいく）
void changeTempLog(USB_LOG_TEMP_STR logTemp, unsigned long time)
{
	#define next(x) (x == TEMP_BUF_NUM - 1 ? 0 : x + 1)
	#define prev(x) (x == 0 ? TEMP_BUF_NUM - 1 : x - 1)
	static struct{
		bool *overFlg;
		unsigned long *thPos;
		unsigned long *timeTh;
		unsigned long *tempTh;
	} tempLog[USBLOG_DEV_NUM] = {
		{&overTh1Flg,		&posTh1,	timeTh1,	tempTh1		},
		{&overTh2Flg,		&posTh2,	timeTh2,	tempTh2		},
		{&overTh3Flg,		&posTh3,	timeTh3,	tempTh3		},
		{&overDutySubFlg,	&posDutySub,timeDutySub,valueDutySub},
		{&overTh5Flg,		&posTh5,	timeTh5,	tempTh5		},
		{&overDutyFlg,		&posDuty,	timeDuty,	valueDuty	}
	};

	int pos;
	int i;
	int j;
	int k;
	int startPos;
	int num;
	
	for (k = 0; k < USBLOG_DEV_NUM; k++) {
		if (usbLogExist[k] == true) {
			startPos = (*(tempLog[k].overFlg) == true ? *(tempLog[k].thPos) : 0);
			num = (*(tempLog[k].overFlg) == true ? TEMP_BUF_NUM : *(tempLog[k].thPos));
			if (*(tempLog[k].overFlg) && time < tempLog[k].timeTh[startPos]) {
				//一番古いデータより前の時間の物は無視
				return;
			}
			if (*(tempLog[k].overFlg)) {
				for (i = 0, pos = startPos; i < num; i++, pos = next(pos) ) {
					if (time < tempLog[k].timeTh[pos]) {
						break;
					}
				}
				if (pos != *(tempLog[k].thPos)) {
					for (j = prev(*(tempLog[k].thPos)); j != pos; j = prev(j)) {
						tempLog[k].timeTh[next(j)] = tempLog[k].timeTh[j];
						tempLog[k].tempTh[next(j)] = tempLog[k].tempTh[j];
					}
				}
				tempLog[k].timeTh[pos] = time; 
				tempLog[k].tempTh[pos] = strtol(logTemp.valStr[k], NULL, 16);	
			} else {
				if (num != 0) {
					for (pos = 0; pos < (int)(*(tempLog[k].thPos)); pos++) {
						if (time < tempLog[k].timeTh[pos]) {
							break;
						}
					}
					for (j = *(tempLog[k].thPos) - 1; j >= pos; j--) {
						tempLog[k].timeTh[j + 1] = tempLog[k].timeTh[j];
						tempLog[k].tempTh[j + 1] = tempLog[k].tempTh[j];
					}
				} else {
					pos = *(tempLog[k].thPos);
				}
				tempLog[k].timeTh[pos] = time; 
				tempLog[k].tempTh[pos] = strtol(logTemp.valStr[k], NULL, 16);
			}
			(*(tempLog[k].thPos))++;
			if (*(tempLog[k].thPos) >= TEMP_BUF_NUM) {
				*(tempLog[k].thPos) = 0;
				*(tempLog[k].overFlg) = true;
			}
		}
	}
	
}

//ログデータをグラフ化できるログフォーマットに変換する
void changeLog(unsigned long token, unsigned long value, unsigned long time)
{
	static char str[18][100] = {
		"",
		"TH1",
		"TH2",
		"TH3",
		"Duty",
		"TH5",
		"Ctl0",
		"Ctl1", 
		"Ctl2",
		"Alg0",
		"Alg1",
		"Alg2",
		"Sta",
		"Info",
		"core",
		"Pres",
		"Icpu"
	};
	fixLogInfo(str[token], time, value);
}

//infomationログの文字列を分離する
USB_LOG_INFO_STR separationInfoLog(char *line)
{
	char delim[] = " \r\n";
	char* context; //strtok_sの内部で使用
	char* token;
	char* valueStr1;
	char* valueStr2;
	USB_LOG_INFO_STR logInfo;
	logInfo.result = false;

	token = strtok_s(line, delim, &context);
	if (token != NULL) {
		sprintf_s(logInfo.logStr, sizeof(logInfo.logStr), "%s", token);
		if (strlen(token) == 2) {
			valueStr1 = strtok_s(NULL, delim, &context);
			if ((valueStr1 != NULL) && (strlen(valueStr1) == 2)) {
				sprintf_s(logInfo.valStr, sizeof(logInfo.valStr), "%s", valueStr1);
				valueStr2 = strtok_s(NULL, delim, &context);
				if (valueStr2 != NULL) {
					sprintf_s(logInfo.timeStr, sizeof(logInfo.timeStr), "%s", valueStr2);
					logInfo.result = true;
				}
			}
		} else if (strlen(token) == 8) {
			sprintf_s(logInfo.timeStr, sizeof(logInfo.timeStr), "%s", token);
		}
	}
	return logInfo;
}

//温度情報ログの文字列を分離する
USB_LOG_TEMP_STR  separationTempLog(char *line)
{
	char delim[] = " \r\n";
	char* context; //strtok_sの内部で使用
	char* token;
	USB_LOG_TEMP_STR logTemp;
	logTemp.result = false;

	token = strtok_s(line, delim, &context);
	if ((token != NULL) && (strlen(token) == 2)) {
		sprintf_s(logTemp.valStr[0], sizeof(logTemp.valStr[0]), "%s", token);
		token = strtok_s(NULL, delim, &context);
		if ((token != NULL) && (strlen(token) == 2)) {
			sprintf_s(logTemp.valStr[1], sizeof(logTemp.valStr[1]), "%s", token);
			token = strtok_s(NULL, delim, &context);
			if ((token != NULL) && (strlen(token) == 2)) {
				sprintf_s(logTemp.valStr[2], sizeof(logTemp.valStr[2]), "%s", token);
				token = strtok_s(NULL, delim, &context);
				if ((token != NULL) && (strlen(token) == 2)) {
					sprintf_s(logTemp.valStr[3], sizeof(logTemp.valStr[3]), "%s", token);
					token = strtok_s(NULL, delim, &context);
					if ((token != NULL) && (strlen(token) == 2)) {
						sprintf_s(logTemp.valStr[4], sizeof(logTemp.valStr[4]), "%s", token);
						token = strtok_s(NULL, delim, &context);
						if ((token != NULL) && (strlen(token) == 2)) {
							sprintf_s(logTemp.valStr[5], sizeof(logTemp.valStr[5]), "%s", token);
							logTemp.result = true;
						}
					}
				}
			}
		}
	} else if ((token != NULL) && (strlen(token) == 8)) {
		sprintf_s(logTemp.timeStr, sizeof(logTemp.timeStr), "%s", token);
	} else {
	}
	return logTemp;
}

//USBログの定着ログを読み込む
bool readFixUsbLog(char *line)
{
	char* logBuf;
	static unsigned char offset = 0;
	char delim[] = " \r\n";
	char* context; //strtok_sの内部で使用
	char* token;
	bool ret = false;
	static unsigned long logTime = 0;
	static unsigned long preLogTime = 0;
	static unsigned long readByte = 0;
	static unsigned long tempReadCount = 0;
	static unsigned long startTempTime = 0;
	static unsigned long usbLogTimeInfo = 0;
	static unsigned long usbLogTimeTemp = 0;
	static unsigned short usbLogTimeCarryOverNum = 0;
	static unsigned short usbLogReadStep = 0;
	
	USB_LOG_INFO_STR logInfo;
	USB_LOG_TEMP_STR logTemp;
	
	if (usbLogReadStep == 1) {
		readByte += strlen(line) + 1 ;
		logInfo = separationInfoLog(line + offset);
		logTime = strtol(logInfo.timeStr, NULL, 16);
		if (logInfo.result) {
			if (logTime < preLogTime) {
				usbLogTimeCarryOverNum++;//2バイトの時間情報が何回オーバーフローしたのかカウントする
			}
			preLogTime = logTime;
		} else {
			usbLogTimeCarryOverNum = ((logTime - preLogTime) >> 16) - usbLogTimeCarryOverNum;
			preLogTime = 0;
		}
	}
	if (usbLogReadStep == 2) {
		readByte += strlen(line) + 1 ;
		logTemp = separationTempLog(line + offset);
		if (logTemp.result) {
			tempReadCount++;//温度データが何秒分あったのかカウントする
			for (int i = 0; i < USBLOG_DEV_NUM; i++) {
				if (strtol(logTemp.valStr[i], NULL, 16) != 0) {
					usbLogExist[i] = true;
				}
			}
		} else {
			logTime = strtol(logTemp.timeStr, NULL, 16);
			startTempTime = 500; //1秒 1000 / 2 = システムカウント500
			if (tempReadCount > 0) {
				if (logTime > (tempReadCount - 1) * 1000 / 2) {
					startTempTime = (logTime) - ((tempReadCount - 1) * 1000 / 2);
				}
			}
			tempReadCount = 0;
		}
	}

	if (usbLogReadStep == 0) {
		logBuf = (strstr(line, "M0"));
		if (logBuf != NULL) {
			offset = 6;
		}
		logBuf = (strstr(line + offset, "fixlog"));
		if ((logBuf != NULL) && (strlen(line + offset) == 7)) {	//改行コード含めて7文字
			//初期化
			usbLogTimeCarryOverNum = 0;
			tempReadCount = 0;
			usbLogReadStep = 1;
			readByte = 0;
			preLogTime = 0;
			for (int i = 0; i < USBLOG_DEV_NUM; i++) {
				usbLogExist[i] = false; 
			}
		}
	} else if (usbLogReadStep == 1) {
		//8桁の最終ログ表示時間（システムカウンタ）の値を取得するまで、データは読み飛ばす
		logBuf = line + offset;
		token = strtok_s(logBuf, delim, &context);
		if (token != NULL) {
			if (strlen(token) == 8) {
				usbLogTimeInfo = strtol(token, NULL, 16);
				usbLogReadStep = 2;
			}
		}
	} else if (usbLogReadStep == 2) {
		//8桁の最終温度表示時間（システムカウンタ）の値を取得するまで、データは読み飛ばす
		logBuf = line + offset;
		token = strtok_s(logBuf, delim, &context);
		if (token != NULL) {
			if (strlen(token) == 8) {
				usbLogTimeTemp = strtol(token, NULL, 16);
				usbLogReadStep = 3;
				fseek(fp, -1 * readByte, SEEK_CUR);//もう一度、ログデータ表示開始位置までファイルポイントを移動
			}
		}
	} else if (usbLogReadStep == 3) {
		logBuf = line + offset;
		logInfo = separationInfoLog(line + offset);
		if (logInfo.result) {
			logTime = strtol(logInfo.timeStr, NULL, 16);
			logTime += (0x10000 * usbLogTimeCarryOverNum);
			if (logTime < preLogTime) {
				usbLogTimeCarryOverNum++;
			}
			preLogTime = logTime;
			changeLog(strtol(logInfo.logStr, NULL, 16), strtol(logInfo.valStr, NULL, 16), logTime);
			ret = true;
		} else {
			usbLogTimeInfo = strtol(logInfo.timeStr, NULL, 16);
			usbLogReadStep = 4;
		}
	} else if (usbLogReadStep == 4) {
		logTemp = separationTempLog(line + offset);
		if (logTemp.result) {
			changeTempLog(logTemp, startTempTime + tempReadCount * 1000 / 2);
			tempReadCount++;
		} else {
			usbLogReadStep = 0;
		}
		
	}
	return ret;
}

//ファイルを読む処理
void readFile(HDROP hDrop)
{
	//ファイルオープン
	char p[50];
	char* readline1;
	char* readline2;
	char* token;
	char* valueStr1;
	char* valueStr2;
	unsigned long value1;
	unsigned long value2;
	char delim[] = " ";
	char* context; //strtok_sの内部で使用

	fopen_s(&fp, fileName, "r");
	while (fgets(p, 50, fp) != NULL) {
		readline1 = (strstr(p, "[FSR] M"));
		readline2 = (strstr(p, "[FSR] "));
		if (readline1 != NULL) {
			readline1 += 12;				//Xaqro2.0 ＜[FSR] M0670 ＞が12文字
		} else if (readline2 != NULL) {
			readline1 = readline2 + 6;		//Xaqro1.1 ＜[FSR] ＞が6文字
		} else if (strncmp(p, "\t\t", 2) == 0) {
			readline1 = p + 2 ;				//Xaqro1.0 \t\tのあとに定着ログ
		} else {
			readline1 = p;
		}
		if (readFixUsbLog(readline1)) {
			continue;
		}
		token = strtok_s(readline1, delim, &context);
		if (token != NULL) {
			valueStr1 = strtok_s(NULL, delim, &context);
			if (valueStr1 != NULL) {
				valueStr2 = strtok_s(NULL, delim, &context);
				if (valueStr2 != NULL) {
					value1 = atoi(valueStr1);
					value2 = atol(valueStr2);
					if (logFileAnalyzState == EANALYZ_FILE_RELOAD) {
						if ((value1 >= firstReadLogTime) && ((overTh1Flg != true) || (overTh2Flg != true))) {
							fixLogInfo(token, value1, value2);
						}
					} else {
						fixLogInfo(token, value1, value2);
					}	
				}
			}
		}
	}
	
	fclose(fp);
}

//全体表示ボタンを押したときの処理
void dispAllButton(HDROP hDrop)
{
	if (maxCount >= minCount) {
		if (logFileAnalyzState == EANALYZ_DROP_FILE) {
			if ((overTh1Flg == true) || (overTh2Flg == true)) {
				analyzState = EANALYZ_ALL_PAINT;
			} else {
				logFileAnalyzState = EANALYZ_IDLE;
			}
		}
		timeScale = ((maxCount - minCount) / 750) + 1;
		offSetTimePos = minCount / timeScale;
		moveOffSet = 0;
		analyzOffSet1 = 0;
		analyzOffSet2 = 0;
		scaleDiff = 0;
	}
}

//fileの中身を見る
void readAllFile(HDROP hDrop, HDC hdc, HWND hWnd)
{
	HDC hdc1;
	HDC hdc2;
	PAINTSTRUCT ps;
	HBRUSH hBrush;

	//LTGRAYブラシの作成&選択
	HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0xc0, 0xc0, 0xc0));
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
	SelectObject(hdc, hPen);
	hBrush = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	SelectObject(hdc, hBrush);
	Rectangle(hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGTH);

	//現在のツールバーの画像をメモリDCに転送
	hdc1 = GetDC(hWnd);
	BitBlt(hdc, 0, 0, WINDOW_WIDTH, 28, hdc1, 0, 0, SRCCOPY);
	ReleaseDC(hWnd, hdc1);

	SetBkMode(hdc, TRANSPARENT);

	dispAxisScale(hdc);
	dispAxis(hWnd, hdc);

	//ファイルオープン
	char p[50];
	char* readline1;
	char* readline2;
	char* token;
	char* valueStr1;
	char* valueStr2;
	unsigned long value1;
	unsigned long value2;
	char delim[] = " ";
	char* context; //strtok_sの内部で使用

	fopen_s(&fp, fileName, "r");
	while (fgets(p, 50, fp) != NULL) {
		readline1 = (strstr(p, "[FSR] M"));
		readline2 = (strstr(p, "[FSR] "));
		if (readline1 != NULL) {
			readline1 += 12;				//Xaqro2.0 ＜[FSR] M0670 ＞が12文字
		} else if (readline2 != NULL) {
			readline1 = readline2 + 6;		//Xaqro1.1 ＜[FSR] ＞が6文字
		} else if (strncmp(p, "\t\t", 2) == 0) {
			readline1 = p + 2 ;				//Xaqro1.0 \t\tのあとに定着ログ
		} else {
			readline1 = p;
		}
		token = strtok_s(readline1, delim, &context);
		if (token != NULL) {
			valueStr1 = strtok_s(NULL, delim, &context);
			if (valueStr1 != NULL) {
				valueStr2 = strtok_s(NULL, delim, &context);
				if (valueStr2 != NULL) {
					value1 = atoi(valueStr1);
					value2 = atol(valueStr2);
					drawAllFile(token, value1, value2, hdc);
				}
			}
		}
	}
	//右端まで描画
	if (timeLogFile[EANALYZ_LOG_CTL1] != 0) {
		HPEN hPen5 = CreatePen(PS_SOLID, 2, Ctl1Color);
		HPEN hOldPen5 = (HPEN)SelectObject(hdc, hPen5);
		MoveToEx(hdc, xPos(timeLogFile[EANALYZ_LOG_CTL1]), yPos(valueLogFile[EANALYZ_LOG_CTL1], 1), NULL);
		LineTo(hdc, WINDOW_WIDTH, yPos(valueLogFile[EANALYZ_LOG_CTL1], 1));
		SelectObject(hdc, hOldPen5);
		DeleteObject(hPen5);
	}
	if (timeLogFile[EANALYZ_LOG_CTL0] != 0) {
		HPEN hPen6 = CreatePen(PS_SOLID, 2, Ctl0Color);
		HPEN hOldPen6 = (HPEN)SelectObject(hdc, hPen6);
		MoveToEx(hdc, xPos(timeLogFile[EANALYZ_LOG_CTL0]), yPos(valueLogFile[EANALYZ_LOG_CTL0], 1), NULL);
		LineTo(hdc, WINDOW_WIDTH, yPos(valueLogFile[EANALYZ_LOG_CTL0], 1));
		SelectObject(hdc, hOldPen6);
		DeleteObject(hPen6);
	}
	if (timeLogFile[EANALYZ_LOG_STATE] != 0) {
		HPEN hPen10 = CreatePen(PS_SOLID, 2, StateColor);
		HPEN hOldPen10 = (HPEN)SelectObject(hdc, hPen10);
		MoveToEx(hdc, xPos(timeLogFile[EANALYZ_LOG_STATE]), yPos(valueLogFile[EANALYZ_LOG_STATE], 2), NULL);
		LineTo(hdc, WINDOW_WIDTH, yPos(valueLogFile[EANALYZ_LOG_STATE], 2));
		SelectObject(hdc, hOldPen10);
		DeleteObject(hPen10);
	}
	if (timeLogFile[EANALYZ_LOG_HTRMAIN] != 0) {
		HPEN hPen11 = CreatePen(PS_SOLID, 1, HTRMainColor);
		HPEN hOldPen11 = (HPEN)SelectObject(hdc, hPen11);
		MoveToEx(hdc, xPos(timeLogFile[EANALYZ_LOG_HTRMAIN]), yPos(valueLogFile[EANALYZ_LOG_HTRMAIN], 2), NULL);
		LineTo(hdc, WINDOW_WIDTH, yPos(valueLogFile[EANALYZ_LOG_HTRMAIN], 2));
		SelectObject(hdc, hOldPen11);
		DeleteObject(hPen11);
	}
	if (timeLogFile[EANALYZ_LOG_HTRSUB] != 0) {
		HPEN hPen12 = CreatePen(PS_SOLID, 1, HTRMainColor);
		HPEN hOldPen12 = (HPEN)SelectObject(hdc, hPen12);
		MoveToEx(hdc, xPos(timeLogFile[EANALYZ_LOG_HTRSUB]), yPos(valueLogFile[EANALYZ_LOG_HTRSUB], 2), NULL);
		LineTo(hdc, WINDOW_WIDTH, yPos(valueLogFile[EANALYZ_LOG_HTRSUB], 2));
		SelectObject(hdc, hOldPen12);
		DeleteObject(hPen12);
	}
	fclose(fp);

	//ペンを削除
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);

	// メモリDCからDC2に画像を転送
	hdc2 = BeginPaint(hWnd, &ps);
	BitBlt(hdc2, 0, 0, WINDOW_WIDTH, WINDOW_HEIGTH, hdc, 0, 0, SRCCOPY);
	EndPaint(hWnd, &ps);

	logFileAnalyzState = EANALYZ_SELECT_RANGE;
}

//readAllFile()から取得した値を描画する
void drawAllFile(char* form_msg, unsigned long timeCount, unsigned long value, HDC hdc)
{
	if (strncmp(form_msg, "TH1", 3) == 0) {
		HPEN hPen1 = CreatePen(PS_SOLID, 2, Th1Color);
		HPEN hOldPen1 = (HPEN)SelectObject(hdc, hPen1);
		if (firstLogFlg[EANALYZ_LOG_TH1] == true) {
			MoveToEx(hdc, xPos(timeCount), yPos(value, 1), NULL);
			firstLogFlg[EANALYZ_LOG_TH1] = false;
		} else {
			MoveToEx(hdc, xPos(timeLogFile[EANALYZ_LOG_TH1]), yPos(valueLogFile[EANALYZ_LOG_TH1], 1), NULL);
		}
		LineTo(hdc, xPos(timeCount), yPos(value, 1));
		timeLogFile[EANALYZ_LOG_TH1] = timeCount;
		valueLogFile[EANALYZ_LOG_TH1] = value;

		SelectObject(hdc, hOldPen1);
		DeleteObject(hPen1);
	} else if (strncmp(form_msg, "TH2", 3) == 0) {
		HPEN hPen2 = CreatePen(PS_SOLID, 2, Th2Color);
		HPEN hOldPen2 = (HPEN)SelectObject(hdc, hPen2);
		if (firstLogFlg[EANALYZ_LOG_TH2] == true) {
			MoveToEx(hdc, xPos(timeCount), yPos(value, 1), NULL);
			firstLogFlg[EANALYZ_LOG_TH2] = false;
		} else {
			MoveToEx(hdc, xPos(timeLogFile[EANALYZ_LOG_TH2]), yPos(valueLogFile[EANALYZ_LOG_TH2], 1), NULL);
		}
		LineTo(hdc, xPos(timeCount), yPos(value, 1));
		timeLogFile[EANALYZ_LOG_TH2] = timeCount;
		valueLogFile[EANALYZ_LOG_TH2] = value;

		SelectObject(hdc, hOldPen2);
		DeleteObject(hPen2);
	} else if (strncmp(form_msg, "TH3", 3) == 0) {
		HPEN hPen3 = CreatePen(PS_SOLID, 2, Th3Color);
		HPEN hOldPen3 = (HPEN)SelectObject(hdc, hPen3);
		if (firstLogFlg[EANALYZ_LOG_TH3] == true) {
			MoveToEx(hdc, xPos(timeCount), yPos(value, 1), NULL);
			firstLogFlg[EANALYZ_LOG_TH3] = false;
		} else {
			MoveToEx(hdc, xPos(timeLogFile[EANALYZ_LOG_TH3]), yPos(valueLogFile[EANALYZ_LOG_TH3], 1), NULL);
		}
		LineTo(hdc, xPos(timeCount), yPos(value, 1));
		timeLogFile[EANALYZ_LOG_TH3] = timeCount;
		valueLogFile[EANALYZ_LOG_TH3] = value;
		SelectObject(hdc, hOldPen3);
		DeleteObject(hPen3);
	} else if (strncmp(form_msg, "TH4", 3) == 0) {
		HPEN hPen4 = CreatePen(PS_SOLID, 1, Th4Color);
		HPEN hOldPen4 = (HPEN)SelectObject(hdc, hPen4);
		if (firstLogFlg[EANALYZ_LOG_TH4] == true) {
			MoveToEx(hdc, xPos(timeCount), yPos(value, 1), NULL);
			firstLogFlg[EANALYZ_LOG_TH4] = false;
		} else {
			MoveToEx(hdc, xPos(timeLogFile[EANALYZ_LOG_TH4]), yPos(valueLogFile[EANALYZ_LOG_TH4], 1), NULL);
		}
		LineTo(hdc, xPos(timeCount), yPos(value, 1));
		timeLogFile[EANALYZ_LOG_TH4] = timeCount;
		valueLogFile[EANALYZ_LOG_TH4] = value;
		SelectObject(hdc, hOldPen4);
		DeleteObject(hPen4);
	} else if (strncmp(form_msg, "TH5", 3) == 0) {
		HPEN hPen4 = CreatePen(PS_SOLID, 2, Th5Color);
		HPEN hOldPen4 = (HPEN)SelectObject(hdc, hPen4);
		if (firstLogFlg[EANALYZ_LOG_TH5] == true) {
			MoveToEx(hdc, xPos(timeCount), yPos(value, 1), NULL);
			firstLogFlg[EANALYZ_LOG_TH5] = false;
		} else {
			MoveToEx(hdc, xPos(timeLogFile[EANALYZ_LOG_TH5]), yPos(valueLogFile[EANALYZ_LOG_TH5], 1), NULL);
		}
		LineTo(hdc, xPos(timeCount), yPos(value, 1));
		timeLogFile[EANALYZ_LOG_TH5] = timeCount;
		valueLogFile[EANALYZ_LOG_TH5] = value;
		SelectObject(hdc, hOldPen4);
		DeleteObject(hPen4);
	} else if (strncmp(form_msg, "Ctl1", 4) == 0) {
		HPEN hPen5 = CreatePen(PS_SOLID, 2, Ctl1Color);
		HPEN hOldPen5 = (HPEN)SelectObject(hdc, hPen5);
		if (firstLogFlg[EANALYZ_LOG_CTL1] == true) {
			MoveToEx(hdc, xPos(timeCount), yPos(value, 1), NULL);
			firstLogFlg[EANALYZ_LOG_CTL1] = false;
		} else {
			MoveToEx(hdc, xPos(timeLogFile[EANALYZ_LOG_CTL1]), yPos(valueLogFile[EANALYZ_LOG_CTL1], 1), NULL);
			LineTo(hdc, xPos(timeCount), yPos(valueLogFile[EANALYZ_LOG_CTL1], 1));
			LineTo(hdc, xPos(timeCount), yPos(value, 1));
		}
		timeLogFile[EANALYZ_LOG_CTL1] = timeCount;
		valueLogFile[EANALYZ_LOG_CTL1] = value;
		SelectObject(hdc, hOldPen5);
		DeleteObject(hPen5);
	} else if (strncmp(form_msg, "Ctl0", 4) == 0) {
		HPEN hPen6 = CreatePen(PS_SOLID, 2, Ctl0Color);
		HPEN hOldPen6 = (HPEN)SelectObject(hdc, hPen6);
		if (firstLogFlg[EANALYZ_LOG_CTL0] == true) {
			MoveToEx(hdc, xPos(timeCount), yPos(value, 1), NULL);
			firstLogFlg[EANALYZ_LOG_CTL0] = false;
		} else {
			MoveToEx(hdc, xPos(timeLogFile[EANALYZ_LOG_CTL0]), yPos(valueLogFile[EANALYZ_LOG_CTL0], 1), NULL);
			LineTo(hdc, xPos(timeCount), yPos(valueLogFile[EANALYZ_LOG_CTL0], 1));
			LineTo(hdc, xPos(timeCount), yPos(value, 1));
		}
		timeLogFile[EANALYZ_LOG_CTL0] = timeCount;
		valueLogFile[EANALYZ_LOG_CTL0] = value;
		SelectObject(hdc, hOldPen6);
		DeleteObject(hPen6);
	} else if (strncmp(form_msg, "Info", 4) == 0) {
		HPEN hPen7 = CreatePen(PS_SOLID, 6, InfoColor);
		HPEN hOldPen7 = (HPEN)SelectObject(hdc, hPen7);
		Ellipse(hdc, xPos(timeCount) - 1, yPos(value, 1) - 1, xPos(timeCount) + 1, yPos(value, 1) + 1);
		SelectObject(hdc, hOldPen7);
		DeleteObject(hPen7);
	} else if (strncmp(form_msg, "Duty", 4) == 0) {
		HPEN hPen8 = CreatePen(PS_SOLID, 5, DutyColor);
		HPEN hOldPen8 = (HPEN)SelectObject(hdc, hPen8);
		Ellipse(hdc, xPos(timeCount) - 1, yPos(value, 1) - 1, xPos(timeCount) + 1, yPos(value, 1) + 1);
		SelectObject(hdc, hOldPen8);
		DeleteObject(hPen8);
	} else if (strncmp(form_msg, "DSub", 4) == 0) {
		HPEN hPen9 = CreatePen(PS_SOLID, 2, DutySubColor);
		HPEN hOldPen9 = (HPEN)SelectObject(hdc, hPen9);
		Ellipse(hdc, xPos(timeCount) - 1, yPos(value, 1) - 1, xPos(timeCount) + 1, yPos(value, 1) + 1);
		SelectObject(hdc, hOldPen9);
		DeleteObject(hPen9);
	} else if (strncmp(form_msg, "Sta", 3) == 0) {
		HPEN hPen10 = CreatePen(PS_SOLID, 2, StateColor);
		HPEN hOldPen10 = (HPEN)SelectObject(hdc, hPen10);
		if (firstLogFlg[EANALYZ_LOG_STATE] == true) {
			MoveToEx(hdc, xPos(timeCount), yPos(value, 2), NULL);
			firstLogFlg[EANALYZ_LOG_STATE] = false;
		} else {
			MoveToEx(hdc, xPos(timeLogFile[EANALYZ_LOG_STATE]), yPos(valueLogFile[EANALYZ_LOG_STATE], 2), NULL);
			LineTo(hdc, xPos(timeCount), yPos(valueLogFile[EANALYZ_LOG_STATE], 2));
			LineTo(hdc, xPos(timeCount), yPos(value, 2));
		}
		timeLogFile[EANALYZ_LOG_STATE] = timeCount;
		valueLogFile[EANALYZ_LOG_STATE] = value;
		SelectObject(hdc, hOldPen10);
		DeleteObject(hPen10);
	} else if (strncmp(form_msg, "Alg0", 4) == 0) {
		HPEN hPen11 = CreatePen(PS_SOLID, 1, HTRMainColor);
		HPEN hOldPen11 = (HPEN)SelectObject(hdc, hPen11);
		if (firstLogFlg[EANALYZ_LOG_HTRMAIN] == true) {
			MoveToEx(hdc, xPos(timeCount), yPos(value, 2), NULL);
			firstLogFlg[EANALYZ_LOG_HTRMAIN] = false;
		} else {
			MoveToEx(hdc, xPos(timeLogFile[EANALYZ_LOG_HTRMAIN]), yPos(valueLogFile[EANALYZ_LOG_HTRMAIN], 2), NULL);
			LineTo(hdc, xPos(timeCount), yPos(valueLogFile[EANALYZ_LOG_HTRMAIN], 2));
			LineTo(hdc, xPos(timeCount), yPos(value, 2));
		}
		timeLogFile[EANALYZ_LOG_HTRMAIN] = timeCount;
		valueLogFile[EANALYZ_LOG_HTRMAIN] = value;
		SelectObject(hdc, hOldPen11);
		DeleteObject(hPen11);
	} else if (strncmp(form_msg, "Alg1", 4) == 0) {
		HPEN hPen12 = CreatePen(PS_SOLID, 2, HTRSubColor);
		HPEN hOldPen12 = (HPEN)SelectObject(hdc, hPen12);
		if (firstLogFlg[EANALYZ_LOG_HTRSUB] == true) {
			MoveToEx(hdc, xPos(timeCount), yPos(value, 2), NULL);
			firstLogFlg[EANALYZ_LOG_HTRSUB] = false;
		} else {
			MoveToEx(hdc, xPos(timeLogFile[EANALYZ_LOG_HTRSUB]), yPos(valueLogFile[EANALYZ_LOG_HTRSUB], 2), NULL);
			LineTo(hdc, xPos(timeCount), yPos(valueLogFile[EANALYZ_LOG_HTRSUB],2));
			LineTo(hdc, xPos(timeCount), yPos(value, 2));
		}
		timeLogFile[EANALYZ_LOG_HTRSUB] = timeCount;
		valueLogFile[EANALYZ_LOG_HTRSUB] = value;
		SelectObject(hdc, hOldPen12);
		DeleteObject(hPen12);
	} else if (strncmp(form_msg, "Pres", 4) == 0) {
		HPEN hPen13 = CreatePen(PS_SOLID, 6, PressColor);
		HPEN hOldPen13 = (HPEN)SelectObject(hdc, hPen13);
		Ellipse(hdc, xPos(timeCount) - 1, yPos(value, 2) - 1, xPos(timeCount) + 1, yPos(value, 2) + 1);
		SelectObject(hdc, hOldPen13);
		DeleteObject(hPen13);
	} else {
	}
}

//カーソル+5000カウント分を選択する関数
void selectRange(HDC hdc, HWND hWnd)
{
	HDC hdc1;
	PAINTSTRUCT ps;

	hdc1 = BeginPaint(hWnd, &ps);
	HPEN hPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
	HPEN hOldPen = (HPEN)SelectObject(hdc1, hPen);
	
	//現在のツールバーの画像をメモリDCに転送
	BitBlt(hdc1, 0, 0, WINDOW_WIDTH, WINDOW_HEIGTH, hdc, 0, 0, SRCCOPY);
	SetBkMode(hdc1, TRANSPARENT);
	
	unsigned long movingXPos = (movingX >= X_0 ? inverseXPos(movingX) : 0);

	MoveToEx(hdc1, xPos(movingXPos), Y_0 - 310, NULL);
	LineTo(hdc1, xPos(movingXPos), Y_0 + 118);
	LineTo(hdc1, xPos(movingXPos + TIME_COUNT_50000), Y_0 + 118);
	LineTo(hdc1, xPos(movingXPos + TIME_COUNT_50000), Y_0 - 310);
	LineTo(hdc1, xPos(movingXPos), Y_0 - 310);

	SelectObject(hdc1, hOldPen);
	DeleteObject(hPen);
}

//カーソルに近い値を計算する関数
void calculateNearestValue()
{
	unsigned long minNum;
	signed long diffX;
	signed long diffY;

	switch (analyzState) {
		case EANALYZ_WATCH_YDATA:
		case EANALYZ_WATCH_YDATA_LINE:
			diffX = (movingX - xPos(analyzItem[itemNum].time[0]));
			diffY = (movingY - yPos(analyzItem[itemNum].value[0], analyzItem[itemNum].axis));
			minNum = (diffX * diffX) + (diffY * diffY);
			for (unsigned long i = 0; i < analyzItem[itemNum].bufNum; i++) {
				diffX = (movingX - xPos(analyzItem[itemNum].time[i]));
				diffY = (movingY - yPos(analyzItem[itemNum].value[i], analyzItem[itemNum].axis));
				if (minNum >= (unsigned long) ((diffX * diffX) + (diffY * diffY))) {
					minNum = (diffX * diffX) + (diffY * diffY);
					timeYData1[posYData] = analyzItem[itemNum].time[i];
					valueYData1[posYData] = analyzItem[itemNum].value[i];
					valueYAxis[posYData] = analyzItem[itemNum].axis;
				}
			}
			break;
		case EANALYZ_VERTICAL:
			diffX = movingX - xPos(analyzItem[itemNum].time[0]);
			minNum = diffX * diffX;
			for (unsigned long i = 0; i < analyzItem[itemNum].bufNum; i++) {
				diffX = movingX - xPos(analyzItem[itemNum].time[i]);
				if (minNum >= (unsigned long)(diffX * diffX)) {
					minNum = diffX * diffX;
					timeVertical[posVertical] = analyzItem[itemNum].time[i];
				}
			}
			break;
		case EANALYZ_HORIZONAL:
			diffY = movingY - yPos(analyzItem[itemNum].value[0], analyzItem[itemNum].axis);
			minNum = diffY * diffY;
			for (unsigned long i = 0; i < analyzItem[itemNum].bufNum; i++) {
				diffY = movingY - yPos(analyzItem[itemNum].value[i], analyzItem[itemNum].axis);
				if (minNum >= (unsigned long)(diffY * diffY)) {
					minNum = diffY * diffY;
					valueHorizonal[posHorizonal] = analyzItem[itemNum].value[i];
					valueHorizonalAxis[posHorizonal] = analyzItem[itemNum].axis;
				}
			}
			break;
		case EANALYZ_TIME_WIDTH:
			diffX = movingX - xPos(analyzItem[itemNum].time[0]);
			minNum = diffX * diffX;
			for (unsigned long i = 0; i < analyzItem[itemNum].bufNum; i++) {
				diffX = movingX - xPos(analyzItem[itemNum].time[i]);
				if (minNum >= (unsigned long)(diffX * diffX)) {
					minNum = diffX * diffX;
					timeWidth1[posTimeWidth] = analyzItem[itemNum].time[i];
					valueTimeWidth1[posTimeWidth] = analyzItem[itemNum].value[i];
					valueTimeWidthAxis[posTimeWidth] = analyzItem[itemNum].axis;
				}
			}
			break;
		case EANALYZ_TIME_WIDTH_LINE:
			diffX = movingX - xPos(analyzItem[itemNum].time[0]);
			minNum = diffX * diffX;
			for (unsigned long i = 0; i < analyzItem[itemNum].bufNum; i++) {
				diffX = movingX - xPos(analyzItem[itemNum].time[i]);
				if (minNum >= (unsigned long)(diffX * diffX)) {
					minNum = diffX * diffX;
					timeWidth2[posTimeWidth] = analyzItem[itemNum].time[i];
					valueTimeWidth2[posTimeWidth] = analyzItem[itemNum].value[i];
					timeWidth3[posTimeWidth] = (float)(timeWidth1[posTimeWidth] + analyzItem[itemNum].time[i]) / 2;
				}
			}
			yPosTimeWidth[posTimeWidth] = movingY;
			break;
		default:
			break;
	}
}

// アスキーコードモードのログを受け取った際の処理
void receiveFixLog(char *buffer)
{
	char delim[] = " ";
	char* context; //strtok_sの内部で使用
	char* token;
	char* valueStr1;
	char* valueStr2;
	char* valueStr3;
	unsigned long value1;
	unsigned long value2;

	token = strtok_s(buffer, delim, &context);
	valueStr1 = strtok_s(NULL, delim, &context);
	if (valueStr1 != NULL) {
		valueStr2 = strtok_s(NULL, delim, &context);
		if (valueStr2 != NULL) {
			value1 = atoi(valueStr1);
			value2 = atol(valueStr2);
			valueStr3 = strtok_s(NULL, delim, &context);
			if (valueStr3 == NULL) {
				fixLogInfo(token, value1, value2);
			}
		}
	}
}
//x座標(時間軸）の計算をする関数
unsigned long xPos(unsigned long time)
{
	unsigned long xPosition = X_0 + (time / timeScale) - offSetTimePos + moveOffSet + scaleDiff;

	return xPosition;
}

//x座標をカウント値に変換する関数
unsigned long inverseXPos(unsigned long xPos)
{
	unsigned long xTime = (xPos - X_0 + offSetTimePos - moveOffSet - scaleDiff) * timeScale;

	return xTime;
}

//Y座標(1軸目）を計算する関数
unsigned long yPos(unsigned long value, unsigned char axis)
{
	unsigned long yPosition = 0;
	if (axis == 1){
		yPosition = Y_0 - value;
	} else if (axis == 2) {
		yPosition = Y_0 - (value * STATE_SCALE);
	} else {
	}
	return yPosition;
}

//Y座標を値に変換する関数
unsigned long inverseYPos(unsigned long yPos)
{
	unsigned long value = Y_0 - yPos;

	return value;
}

//時間軸の目盛を表示する場所を計算する関数
unsigned long xAxis(unsigned long timeSec)
{
	return xPos(500 * timeSec);
}

//指定した秒数が何ピクセル分か計算する関数
unsigned long pixelsMoved(unsigned long timeSec)
{
	unsigned long pixels = 500 * timeSec / timeScale;

	return pixels;
}

//カウント値を時間に変換する関数
float convertCountsToTime(signed long count)
{
	float time = ((float)count / 500);

	return time;
}
