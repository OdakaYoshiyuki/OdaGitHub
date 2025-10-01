#include <windows.h>
#include "stdafx.h"
#include <Commctrl.h>
#include <stdio.h>
#include <iostream>
#include <shlwapi.h>
#include "FixLog.h"
#include "LogTool.h"
#include "profileInfo.h"
#include "Resource.h"
#include "VisualAnalyzer.h"
#include <shellapi.h>
#include <string.h>
#include <math.h>
#include <commdlg.h>  // OPENFILENAME の定義と共通ダイアログ API のため

#pragma comment(lib,"comctl32.lib")

extern void fillInSpace(char *str);
extern bool comparison(double data1, double data2, eCOMPARISON_TYPE opeId);
extern void calculateData(char *buffer);
extern double numericalCalculation(char* dataStr);
extern void useCount(char *func);
extern char logToolSettingPath[];
void dispToolDbgLog(char* valog);

extern HINSTANCE hInst;
extern char visualAnalyzerIni[_T_MAX_PATH];	// Iniファイルパス
extern char visualAnalyzerName[_T_MAX_PATH];	// Iniファイルパス
extern char szIniFilePublic[_T_MAX_PATH];	// Iniファイルパス

char* readVaIniDataNum(char* section, char* key);
char* readVaIniData(char* section, char* keyName, int num);
void writeVaIniDataNum(char* section, char* key, int value);
void writeVaIniDataInt(char* section, char* keyName, int num, int value);
void writeVaIniDataFloat(char* section, char* keyName, int num, double value);
void writeVaIniDataStr(char* section, char* keyName, int num, char* name);
LRESULT CALLBACK visualAnalyzWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void updateAnalyzeWindow(HWND hWnd, HDC hdc);
void dispPoint(HWND hWnd, HDC hdc);
void dispLine(HWND hWnd, HDC hdc);
void dispSelectPoint(HWND hWnd, HDC hdc);
void dispBranch(HWND hWnd, HDC hdc);
void dispDevice(HWND hWnd, HDC hdc);
void dispStartPoint(HWND hWnd, HDC hdc);
void dispText(HWND hWnd, HDC hdc);
void dispInfomation(HWND hWnd, HDC hdc);
void dispMotion(HWND hWnd, HDC hdc);
void readDevSetting(HWND hDlg);
void motionEnd(unsigned char id);
void setEditPos();
void connectPoint (unsigned short ePos);
void normalPoint (unsigned short ePos);
void moveOn(unsigned char id, double moveDisTop, double moveDisBottom);
void proceedToPoint(unsigned char id, eEDGE_POINT edge, unsigned short point);
void updateTopPoint(unsigned char id, double dis);
void updateMotion(unsigned long time);
void motionReset();
void operationCheck(char* devName);
void updateOperation(unsigned short opeId);
void visualAnalyzeLogInfo(char* form_msg);
void updateTimeDisp();
void motionIintial(unsigned short point, unsigned char id);
bool isLastMotion(unsigned short id);
void startAnalyze(unsigned char startPart, double length, unsigned char id);
void foldPaper(unsigned char startPart);
void vaMouseMove();
void vaLButtonUp();
void editStart(HWND hDlg);
void devEditStart(HWND hDlg);
void staPartEditStart(HWND hDlg);
void addOperationList(HWND hDlg);
void addStartPartList(HWND hDlg);
void setCondition(unsigned short opeId);
void calcConnectDistance();
unsigned short getNearPoint();
unsigned short getPointNum();
unsigned short getDeviceNum();
unsigned char getWaitNum(unsigned char id);
signed short checkSwitchPoint(unsigned short point);
signed short getDeviceId(unsigned short point);
signed short getBranchId(unsigned short point);
signed short getStartPartId(unsigned short point);
double calcDistance (double x1, double y1, double x2, double y2);
void pointSwitch(unsigned short swichPoint);
unsigned short getNextTopPoint(unsigned short point);
unsigned short getNextBottomPoint(unsigned char id);
void updateOffset();
void movePen(HDC hdc, double x, double y);
void writeLine(HDC hdc, double x, double y);
void writePoint(HDC hdc, double x, double y);
void writeText(HDC hdc, double x, double y, char *str, long length);
double convertOffsetPointX(double x);
double convertOffsetPointY(double y);
double inConvertOffsetPointY(double y);
double inConvertOffsetPointX(double x);
void deviceLogReceptionSimulation(HWND hDlg);
void startLogReceptionSimulation(HWND hDlg);
void settingOperation(HWND hDlg);
void readOperationSetting(HWND hDlg);
bool updateVisualAnalyzeTime(unsigned long time);
void updateBottomPointFoward(unsigned char id);
void updateBottomPointReverse(unsigned char id);
void updateBottomPointSwitch(unsigned char id);
void checkDirChange(unsigned short dev, signed short val, eDIRECTION dir);
signed short getSpeed(unsigned short dev);
DEVICE_ID_INFO getDevId(char *name);
void fileReadStart();
void fileReadExecute();
void deviceLogAnalyze(char* deviceStr, signed short val, signed char id);
void pageCancel();
bool checkFoldPoint(unsigned short stPos, unsigned short id);
bool isFold(unsigned char sPart);

static HWND vaHwnd;
static HWND timeWindw;
static HWND devSetHDlg;
static HFONT hTimelFont;
static FILE* fileP;
static char fileName[200];
static signed long mousePosX;
static signed long mousePosY;
static ePOINT_TYPE editType = EPOINT_TYPE_NONE;
static eEDIT_STATE editState = EEDIT_STATE_IDLE;
static double editDistance = 100;
static bool isHideLength = false;
static unsigned char editSartPart = 0;
static unsigned short deviceSize = 30;
static char deviceName[STR_LEN_MAX];
static char driveSroceName[STR_LEN_MAX];
static char startPartName[STR_LEN_MAX];
static unsigned short editPos = 0;
static unsigned short splitPos = 0;
static unsigned short textPos = 0;
static unsigned short editDev = 0;
static unsigned short editSp = 0;
static unsigned short selectPos = 0;
static unsigned short operationPos = 0;
static unsigned short pointNum = 0;
static unsigned short connectPos = 0;
static unsigned short branchNum = 0;
static unsigned short switchBackNum = 0;
static unsigned short deviceNum = 0;
static unsigned short startPartNum = 0;
static unsigned short driveSouceNum = 0;
static unsigned short operationNum = 0;
static POSITION_INFO topPosiInfo[MOTION_NUM];
static POSITION_INFO bottomPosiInfo[MOTION_NUM];
static bool isDispText = true;
static bool isDispLog = false;
static bool isDispInfo = false;
static double dispScale = 1;
static signed short moveOffSetX;
static signed short moveOffSetY;
static signed short movingOffSetX;
static signed short movingOffSetY;
static signed short cursorDownX;
static signed short cursorDownY;
static eVACLICK_STATE vaClickState = EVA_MOUSE_UP;
static eDRIVE_TYPE driveType;
static eDEVICE_TYPE deviceType;
static unsigned long crrunetTime = 0;
static eDEVICE_EDIT_TYPE devEditType;
static eSTART_PART_EDIT staPartEditType;
static signed short driveSouceDev = -1;
static eLOG_READ_STATE logReadState = ELOG_READ_NORMAL;
static unsigned long difTime;
static bool pauseFlag = false;
static char readIniStr[256];
static unsigned short duStartPartId;
static char vaLogFormMmsg[256];
static bool isLogWait = false;
static unsigned long rememberTime;
static unsigned short operationId;
static unsigned long fastForwardTime = 0;

static bool isActive[MOTION_NUM];
static double initOverLength[MOTION_NUM];
static POSITION_HISTORY topPosHistory[MOTION_NUM];
static PRIFILE_INFO profileInfo[MOTION_NUM];

static ANALYZE_POINT analyzePoint[ANALYZE_POINT_NUM];
static ANALYZE_BRANCH analyzeBranch[ANALYZE_BRANCH_NUM];
static CURRENT_BRANCH currentBranch[ANALYZE_BRANCH_NUM];
static POSITION_SWITCH positionSwitch[POSITION_SWITCH_NUM];
static DEVICE_INFO devicePoint[DEVICE_NUM];
static START_INFO startPoint[START_POINT_NUM];
static DRIVE_SOUCE driveSouce[DEVICE_NUM];
static DEVICE_OPERATION deviceOperation[DEVICE_OPE_NUM];
static char vaLogBuf[100][256];
static int dataCount = 0;
static int dispCount = 0;
static bool firstTime = true;

static unsigned char settingPatternIndex = 0;
static unsigned char selectPatternIndex = 0;
static char* iniFilePath;

const START_PART startPartTable[] = {
	{"MPF",		0},
	{"CASS1",	1},
	{"CASS2",	2},
	{"CASS3",	3},
	{"CASS4",	4},
	{"CASS5",	5},
	{"CASS6",	6},
	{"CASS7",	7},
	{"CASS8",	10},
	{"BULKFD",	8},
	{"ENV",		9},
	{"OPTMPF",	11},
	{"INS1",	12},
	{"INS2",	13},
	{"DU",		0xff},
	{"DF-In",	0},
	{"DF-Fold1",	0xfe},
	{"DF-Fold2",	0xfd},
};

void outputLog ()
{
	int i;

	for (i = dispCount; i < dataCount; i++) {
		dispToolDbgLog(vaLogBuf[i]);
		dispCount++;
	}
	//例
	//sprintf_s(vaLogBuf[dataCount], sizeof(vaLogBuf[dataCount]), "%d\n", data);
	//dataCount++;
	//sprintf_s(vaLogBuf[dataCount], sizeof(vaLogBuf[dataCount]), "moveDis=%f, speed=%f, time=%d, dev=%d, Point=%d, dis=%f\n", moveDis, speed, time, i, topPosiInfo[id].point, topPosiInfo[id].dis);
	//dataCount++;

}

void visualAnalyzMain(HWND hDlg)
{
	WNDCLASS winc;

	winc.style = CS_HREDRAW | CS_VREDRAW;
	winc.lpfnWndProc = visualAnalyzWndProc;
	winc.cbClsExtra = winc.cbWndExtra = 0;
	winc.hInstance = hInst;
	winc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winc.hCursor = LoadCursor(NULL, IDC_ARROW);
	winc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	winc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU4);
	winc.lpszClassName = TEXT("VisualAnalyzer");

	UnregisterClass(winc.lpszClassName, winc.hInstance);
	if (!RegisterClass(&winc)) return;

	vaHwnd = CreateWindow(
		TEXT("VisualAnalyzer"),									// 登録されたWindowウクラスの名前
		TEXT("VisualAnalyzer"),									// タイトルバーに表示する文字列
		WS_OVERLAPPEDWINDOW | WS_EX_COMPOSITED,	// ウィンドウの種類
		0,														// ウィンドウを表示する位置（X座標）
		0,														// ウィンドウを表示する位置（Y座標）
		WINDOW_WIDTH_VA,											// ウィンドウの幅
		WINDOW_HEIGTH_VA,											// ウィンドウの高さ
		NULL,													// 親ウィンドウのウィンドウハンドル
		NULL,													// メニューハンドル
		hInst,													// インスタンスハンドル
		NULL													// その他の作成データ
	);

	if (vaHwnd == NULL) {
		return;
	}
	ShowWindow(vaHwnd, SW_SHOW);
}

void createTimeDispWindow(HWND hwnd)
{
	timeWindw = CreateWindow(TEXT("STATIC"), "0.0000sec", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE | WS_BORDER | SS_NOTIFY, 265, 2 , 100, 25, hwnd, NULL, NULL, NULL);
	hTimelFont = CreateFont(15, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH, "Meiryo UI" );

	SendMessage(timeWindw, WM_SETFONT, (WPARAM)hTimelFont, MAKELPARAM(TRUE, 0));
}

TBBUTTON tbAnalyzeButton[] = {
	{0, ECOMMAND_VA_SCALEUP,	TBSTATE_ENABLED, TBSTYLE_BUTTON,		0,0,0},
	{1, ECOMMAND_VA_SCALEDOWN,	TBSTATE_ENABLED, TBSTYLE_BUTTON,		0,0,0},
	{2, ECOMMAND_VA_PLAY,		TBSTATE_ENABLED, TBSTYLE_BUTTON,		0,0,0},
	{3, ECOMMAND_VA_PAUSE,		TBSTATE_ENABLED, TBSTYLE_BUTTON,		0,0,0},
	{4, ECOMMAND_VA_STOP,		TBSTATE_ENABLED, TBSTYLE_BUTTON,		0,0,0},
	{5, ECOMMAND_VA_FRAME,		TBSTATE_ENABLED, TBSTYLE_BUTTON,		0,0,0},
	{0, 0,						TBSTATE_ENABLED, TBSTYLE_SEP,			0,0,0},
	{6, ECOMMAND_VA_1,			TBSTATE_ENABLED, TBSTYLE_BUTTON,		0,0,0},
	{7, ECOMMAND_VA_2,			TBSTATE_ENABLED, TBSTYLE_BUTTON,		0,0,0},
	{8, ECOMMAND_VA_3,			TBSTATE_ENABLED, TBSTYLE_BUTTON,		0,0,0},
	{9, ECOMMAND_VA_4,			TBSTATE_ENABLED, TBSTYLE_BUTTON,		0,0,0},
	{10,ECOMMAND_VA_5,			TBSTATE_ENABLED, TBSTYLE_BUTTON,		0,0,0}
};

TBBUTTON tbAnalyzeButtonTestMode[] = {
	{2,  ECOMMAND_VA_PLAY,		TBSTATE_ENABLED, TBSTYLE_BUTTON,		0,0,0},
	{3,  ECOMMAND_VA_PAUSE,		TBSTATE_ENABLED, TBSTYLE_BUTTON,		0,0,0},
	{4,  ECOMMAND_VA_STOP,		TBSTATE_ENABLED, TBSTYLE_BUTTON,		0,0,0},
	{5,  ECOMMAND_VA_FRAME,		TBSTATE_ENABLED, TBSTYLE_BUTTON,		0,0,0},
};

TBBUTTON tbAnalyzeButtonByPattern[] = {
	{6,  ECOMMAND_VA_1,			TBSTATE_ENABLED, TBSTYLE_CHECKGROUP,	0,0,0},
	{7,  ECOMMAND_VA_2,			TBSTATE_ENABLED, TBSTYLE_CHECKGROUP,	0,0,0},
	{8,  ECOMMAND_VA_3,			TBSTATE_ENABLED, TBSTYLE_CHECKGROUP,	0,0,0},
	{9,  ECOMMAND_VA_4,			TBSTATE_ENABLED, TBSTYLE_CHECKGROUP,	0,0,0},
	{10, ECOMMAND_VA_5,			TBSTATE_ENABLED, TBSTYLE_CHECKGROUP,	0,0,0}
};
void readVaIni(HWND hWnd, char* path)
{
	unsigned short i;
	iniFilePath = path;

	for (i = 0; i < ANALYZE_POINT_NUM; i++) {
		analyzePoint[i].isConnect = false;
	}
	pointNum = atoi(readVaIniDataNum("NUMBER", "POINT_NUM"));
	editPos = pointNum;
	branchNum = atoi(readVaIniDataNum("NUMBER", "BRANCH_NUM"));
	switchBackNum = atoi(readVaIniDataNum("NUMBER", "SWITCH_BACK_NUM"));
	deviceNum = atoi(readVaIniDataNum("NUMBER", "DEVICE_NUM"));
	editDev = deviceNum;
	driveSouceNum = atoi(readVaIniDataNum("NUMBER", "DEVICE_SOUCE_NUM"));
	startPartNum = atoi(readVaIniDataNum("NUMBER", "START_PART_NUM"));
	editSp = startPartNum;
	operationNum = atoi(readVaIniDataNum("NUMBER", "OPERATION_NUM"));

	for (i = 0; i < pointNum; i++) {
		analyzePoint[i].X = atof(readVaIniData("POINT", "X", i));
		analyzePoint[i].Y = atof(readVaIniData("POINT", "Y", i));
		analyzePoint[i].texPostX = atoi(readVaIniData("POINT", "TEXT_POS_X", i));
		analyzePoint[i].texPostY = atoi(readVaIniData("POINT", "TEXT_POS_Y", i));
		analyzePoint[i].distance = atof(readVaIniData("POINT", "DISTANCE", i));
		analyzePoint[i].distanceX = atof(readVaIniData("POINT", "DISTANCE_X", i));
		analyzePoint[i].distanceY = atof(readVaIniData("POINT", "DISTANCE_Y", i));
		analyzePoint[i].dispDistance = atof(readVaIniData("POINT", "DISP_DISTANCE", i));
		analyzePoint[i].prePoint = atoi(readVaIniData("POINT", "PREPOINT", i));
		analyzePoint[i].nextPoint = atoi(readVaIniData("POINT", "NEXTPOINT", i));
		analyzePoint[i].isConnect = atoi(readVaIniData("POINT", "IS_CONNECT", i)) == 1 ? true : false;
		analyzePoint[i].isGroupe = atoi(readVaIniData("POINT", "IS_GROUPE", i)) == 1 ? true : false;
		analyzePoint[i].isHideLength = atoi(readVaIniData("POINT", "IS_HIDE_LEN", i)) == 1 ? true : false;
		analyzePoint[i].texPostDisX = atoi(readVaIniData("POINT", "TEXT_POSDIS_X", i));
		analyzePoint[i].texPostDisY = atoi(readVaIniData("POINT", "TEXT_POSDIS_Y", i));
	}
	for (i = 0; i < branchNum; i++) {
		analyzeBranch[i].point = atoi(readVaIniData("BRANCH", "POINT", i));
		currentBranch[i].point = analyzeBranch[i].point;
		analyzeBranch[i].nextPoint1 = atoi(readVaIniData("BRANCH", "NEXTPOINT1", i));
		currentBranch[i].nextPoint = analyzeBranch[i].nextPoint1;
		analyzeBranch[i].nextPoint2 = atoi(readVaIniData("BRANCH", "NEXTPOINT2", i));
	}
	for (i = 0; i < switchBackNum; i++) {
		positionSwitch[i].point = atoi(readVaIniData("SWITCH_BACK", "POINT", i));
		positionSwitch[i].nextPoint = atoi(readVaIniData("SWITCH_BACK", "NEXT", i));
	}
	for (i = 0; i < deviceNum; i++) {
		devicePoint[i].type = (eDEVICE_TYPE)atoi(readVaIniData("DEVICE", "TYPE", i));
		strcpy_s(devicePoint[i].name, readVaIniData("DEVICE", "NAME", i));
		devicePoint[i].point = atoi(readVaIniData("DEVICE", "POINT", i));
		devicePoint[i].size = atoi(readVaIniData("DEVICE", "SIZE", i));
		devicePoint[i].driveSouceDev = atoi(readVaIniData("DEVICE", "SOUCE", i));
		devicePoint[i].disX = atof(readVaIniData("DEVICE", "DIS_X", i));
		devicePoint[i].disY = atof(readVaIniData("DEVICE", "DIS_Y", i));
		devicePoint[i].driveType = (eDRIVE_TYPE)atoi(readVaIniData("DEVICE", "DRIVE_TYPE", i));
	}

	for (i = 0; i < driveSouceNum; i++) {
		driveSouce[i].dev = atoi(readVaIniData("DRIVE_SOUCE", "ID", i));
		strcpy_s(driveSouce[i].name, readVaIniData("DRIVE_SOUCE", "NAME", i));
	}
	for (i = 0; i < startPartNum; i++) {
		startPoint[i].startPart = atoi(readVaIniData("START_PART", "ID", i));
		startPoint[i].point = atoi(readVaIniData("START_PART", "POINT", i));
		strcpy_s(startPoint[i].name, readVaIniData("START_PART", "NAME", i));
		startPoint[i].X = atof(readVaIniData("START_PART", "DIS_X", i));
		startPoint[i].Y = atof(readVaIniData("START_PART", "DIS_Y", i));
		if (startPoint[i].startPart == 0xff) {
			duStartPartId = i;
		}
	}
	for (i = 0; i < operationNum; i++) {
		strcpy_s(deviceOperation[i].name, readVaIniData("OPERATION", "NAME", i));
		deviceOperation[i].dev[0].id = atoi(readVaIniData("OPERATION", "DEV_ID1", i));
		deviceOperation[i].dev[0].type = (eDEVID_TYPE)atoi(readVaIniData("OPERATION", "DEV_TYPE1", i));
		deviceOperation[i].dev[1].id = atoi(readVaIniData("OPERATION", "DEV_ID2", i));
		deviceOperation[i].dev[1].type = (eDEVID_TYPE)atoi(readVaIniData("OPERATION", "DEV_TYPE2", i));
		strcpy_s(deviceOperation[i].condition[0], readVaIniData("OPERATION", "CONDITION1", i));
		strcpy_s(deviceOperation[i].condition[1], readVaIniData("OPERATION", "CONDITION2", i));
		strcpy_s(deviceOperation[i].opeDevName, readVaIniData("OPERATION", "OPE_DEV", i));
		strcpy_s(deviceOperation[i].opeValueStr, readVaIniData("OPERATION", "OPE_VALUE", i));
		strcpy_s(deviceOperation[i].opeIdDirStr, readVaIniData("OPERATION", "OPE_ID_DIR", i));
		deviceOperation[i].devSel[0] = atoi(readVaIniData("OPERATION", "DEV_SEL1", i));
		deviceOperation[i].devSel[1] = atoi(readVaIniData("OPERATION", "DEV_SEL2", i));
		deviceOperation[i].opeDevSel = atoi(readVaIniData("OPERATION", "OPE_DEV_SEL", i));
		deviceOperation[i].stepNum = atoi(readVaIniData("OPERATION", "STEP_NUM", i));
		deviceOperation[i].stepTime = atoi(readVaIniData("OPERATION", "STEP_TIME", i));
		strcpy_s(deviceOperation[i].operationFomura, readVaIniData("OPERATION", "OPE_FOMURA", i));
		setCondition(i);
	}

	GetPrivateProfileString(_T("VISUAL_ANALYZER"), _T("DISP_SCALE"), "1", readIniStr, sizeof(readIniStr), logToolSettingPath);
	dispScale = atof(readIniStr);
	GetPrivateProfileString(_T("VISUAL_ANALYZER"), _T("OFFSET_X"), "0", readIniStr, sizeof(readIniStr), logToolSettingPath);
	moveOffSetX = atoi(readIniStr);
	GetPrivateProfileString(_T("VISUAL_ANALYZER"), _T("OFFSET_Y"), "0", readIniStr, sizeof(readIniStr), logToolSettingPath);
	moveOffSetY = atoi(readIniStr);
}

void writeVaIni(HWND hWnd, char* path)
{
	unsigned short i;
	iniFilePath = path;

	writeVaIniDataNum("NUMBER", "POINT_NUM", pointNum);
	writeVaIniDataNum("NUMBER", "BRANCH_NUM", branchNum);
	writeVaIniDataNum("NUMBER", "SWITCH_BACK_NUM", switchBackNum);
	writeVaIniDataNum("NUMBER", "DEVICE_NUM", deviceNum);
	writeVaIniDataNum("NUMBER", "DEVICE_SOUCE_NUM", driveSouceNum);
	writeVaIniDataNum("NUMBER", "START_PART_NUM", startPartNum);
	writeVaIniDataNum("NUMBER", "OPERATION_NUM", operationNum);
	
	for (i = 0; i < pointNum; i++) {
		writeVaIniDataFloat("POINT", "X", i, analyzePoint[i].X);
		writeVaIniDataFloat("POINT", "Y", i, analyzePoint[i].Y);
		writeVaIniDataInt("POINT", "TEXT_POS_X", i, analyzePoint[i].texPostX);
		writeVaIniDataInt("POINT", "TEXT_POS_Y", i, analyzePoint[i].texPostY);
		writeVaIniDataFloat("POINT", "DISTANCE", i, analyzePoint[i].distance);
		writeVaIniDataFloat("POINT", "DISTANCE_X", i, analyzePoint[i].distanceX);
		writeVaIniDataFloat("POINT", "DISTANCE_Y", i, analyzePoint[i].distanceY);
		writeVaIniDataInt("POINT", "PREPOINT", i, analyzePoint[i].prePoint);
		writeVaIniDataInt("POINT", "NEXTPOINT", i, analyzePoint[i].nextPoint);
		if (analyzePoint[i].isConnect == true) writeVaIniDataInt("POINT", "IS_CONNECT", i, (int)analyzePoint[i].isConnect);
		if (analyzePoint[i].isGroupe == true) writeVaIniDataInt("POINT", "IS_GROUPE", i, (int)analyzePoint[i].isGroupe);
		if (analyzePoint[i].isHideLength == true) writeVaIniDataInt("POINT", "IS_HIDE_LEN", i, (int)analyzePoint[i].isHideLength);
		if (analyzePoint[i].texPostDisX != 0) writeVaIniDataInt("POINT", "TEXT_POSDIS_X", i, (int)analyzePoint[i].texPostDisX);
		if (analyzePoint[i].texPostDisY) writeVaIniDataInt("POINT", "TEXT_POSDIS_Y", i, (int)analyzePoint[i].texPostDisY);
		if (analyzePoint[i].dispDistance != 0) writeVaIniDataFloat("POINT", "DISP_DISTANCE", i, analyzePoint[i].dispDistance);
	}
	for (i = 0; i < branchNum; i++) {
		writeVaIniDataInt("BRANCH", "POINT", i, analyzeBranch[i].point);
		writeVaIniDataInt("BRANCH", "NEXTPOINT1", i, analyzeBranch[i].nextPoint1);
		writeVaIniDataInt("BRANCH", "NEXTPOINT2", i, analyzeBranch[i].nextPoint2);
	}
	for (i = 0; i < switchBackNum; i++) {
		writeVaIniDataInt("SWITCH_BACK", "POINT", i, positionSwitch[i].point);
		writeVaIniDataInt("SWITCH_BACK", "NEXT", i, positionSwitch[i].nextPoint);
	}

	for (i = 0; i < deviceNum; i++) {
		writeVaIniDataInt("DEVICE", "TYPE", i, devicePoint[i].type);
		writeVaIniDataStr("DEVICE", "NAME", i, devicePoint[i].name);
		writeVaIniDataInt("DEVICE", "POINT", i, devicePoint[i].point);
		writeVaIniDataInt("DEVICE", "SIZE", i, devicePoint[i].size);
		writeVaIniDataInt("DEVICE", "SOUCE", i, devicePoint[i].driveSouceDev);
		writeVaIniDataFloat("DEVICE", "DIS_X", i, devicePoint[i].disX);
		writeVaIniDataFloat("DEVICE", "DIS_Y", i, devicePoint[i].disY);
		writeVaIniDataInt("DEVICE", "DRIVE_TYPE", i, devicePoint[i].driveType);
	}
	for (i = 0; i < driveSouceNum; i++) {
		writeVaIniDataInt("DRIVE_SOUCE", "ID", i, driveSouce[i].dev);
		writeVaIniDataStr("DRIVE_SOUCE", "NAME", i, driveSouce[i].name);
	}
	for (i = 0; i < startPartNum; i++) {
		writeVaIniDataInt("START_PART", "ID", i, startPoint[i].startPart);
		writeVaIniDataInt("START_PART", "POINT", i, startPoint[i].point);
		writeVaIniDataStr("START_PART", "NAME", i, startPoint[i].name);
		writeVaIniDataFloat("START_PART", "DIS_X", i, startPoint[i].X);
		writeVaIniDataFloat("START_PART", "DIS_Y", i, startPoint[i].Y);
	}
	for (i = 0; i < operationNum; i++) {
		writeVaIniDataStr("OPERATION", "NAME", i, deviceOperation[i].name);
		writeVaIniDataInt("OPERATION", "DEV_ID1", i, deviceOperation[i].dev[0].id);
		writeVaIniDataInt("OPERATION", "DEV_TYPE1", i, deviceOperation[i].dev[0].type);
		writeVaIniDataInt("OPERATION", "DEV_ID2", i, deviceOperation[i].dev[1].id);
		writeVaIniDataInt("OPERATION", "DEV_TYPE2", i, deviceOperation[i].dev[1].type);
		writeVaIniDataStr("OPERATION", "CONDITION1", i, deviceOperation[i].condition[0]);
		writeVaIniDataStr("OPERATION", "CONDITION2", i, deviceOperation[i].condition[1]);
		writeVaIniDataStr("OPERATION", "OPE_DEV", i, deviceOperation[i].opeDevName);
		writeVaIniDataStr("OPERATION", "OPE_VALUE", i, deviceOperation[i].opeValueStr);
		writeVaIniDataStr("OPERATION", "OPE_ID_DIR", i, deviceOperation[i].opeIdDirStr);
		writeVaIniDataInt("OPERATION", "DEV_SEL1", i, deviceOperation[i].devSel[0]);
		writeVaIniDataInt("OPERATION", "DEV_SEL2", i, deviceOperation[i].devSel[1]);
		writeVaIniDataInt("OPERATION", "OPE_DEV_SEL", i, deviceOperation[i].opeDevSel);
		writeVaIniDataInt("OPERATION", "STEP_NUM", i, deviceOperation[i].stepNum);
		writeVaIniDataInt("OPERATION", "STEP_TIME", i, deviceOperation[i].stepTime);
		writeVaIniDataStr("OPERATION", "OPE_FOMURA", i, deviceOperation[i].operationFomura);
	}
}

void readPattern(HWND hWnd, char select)
{
	char text[_T_MAX_PATH];

	if (select == 0) {
		readVaIni(hWnd, visualAnalyzerIni);
	} else {
		SetCurrentDirectory(szGlPath);
		sprintf_s(text, sizeof(text), "INI_FILE_%d", select);
		GetPrivateProfileString("BY_PATTERN", text, "0", text, sizeof(text), visualAnalyzerIni);
		DWORD attr = GetFileAttributesA(text);
		if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY)){//存在するか確認
			readVaIni(hWnd, text);
		}
	}
}

void setBaseNumStr(HWND hDlg, unsigned char Num)
{
	char text[_T_MAX_PATH];

	sprintf_s(text, sizeof(text), "INI_FILE_%d", Num);
	GetPrivateProfileString(_T("BY_PATTERN"), text, "None", text, sizeof(text), visualAnalyzerIni);
	if (strncmp(text, "None", 4) == 0) {
		SetWindowText(GetDlgItem(hDlg, ID_VA_INI_NAME), text);
	} else {
		char* str = PathFindFileNameA(text);
		SetWindowText(GetDlgItem(hDlg, ID_VA_INI_NAME), str);
	}
}

void clearPattern(HWND hDlg, unsigned char Num)
{
	char text[_T_MAX_PATH];

	sprintf_s(text, sizeof(text), "INI_FILE_%d", Num);
	WritePrivateProfileString("BY_PATTERN", text, NULL, visualAnalyzerIni);
	readPattern(GetParent(hDlg), 0);
	setBaseNumStr(hDlg, Num);
}

bool checkDiffExist(unsigned char Num)
{
	bool ret = false;
	char text[100];

	sprintf_s(text, sizeof(text), "INI_FILE_%d", Num);
	GetPrivateProfileString(_T("BY_PATTERN"), text, "None", text, sizeof(text), visualAnalyzerIni);
	if (strncmp(text, "None", 4) != 0) {
		ret = true;
	}
	return ret;
}

bool setVaSettingIniFileName(HWND hDlg, char *filePath, char *fileName)
{
	int lengh;
	char name[_T_MAX_PATH];

	OPENFILENAME ofn;
	// 構造体に情報をセット
	ZeroMemory(&ofn, sizeof(ofn));			// 最初にゼロクリアしておく
	ofn.lStructSize = sizeof(ofn);			// 構造体のサイズ
	ofn.hwndOwner = hDlg;                   // コモンダイアログの親ウィンドウハンドル
	ofn.lpstrFilter = "設定(*.ini)\0*.ini\0All files(*.*)\0*.*\0\0";

	// ファイルの種類
	ofn.lpstrFile = buf;					// 選択されたファイル名(フルパス)を受け取る変数のアドレス
	ofn.lpstrFileTitle = name;				// 選択されたファイル名を受け取る変数のアドレス
	ofn.nMaxFile = sizeof(buf);				// lpstrFileに指定した変数のサイズ
	ofn.nMaxFileTitle = sizeof(name);		// lpstrFileTitleに指定した変数のサイズ
	ofn.Flags = OFN_OVERWRITEPROMPT;        // フラグ指定
	ofn.lpstrTitle = _T("ファイル設定");
	ofn.lpstrDefExt = _T("ini");            // デフォルトのファイルの種類
											// 名前を付けて保存コモンダイアログを作成

	buf[0] = 0; //読み込む前にクリアしておかないと、キャンセルした場合にエーラになるため
	if(!GetOpenFileName(&ofn)) {
		if (buf[0] != 0) {
			MessageBox(NULL, TEXT("ファイル名が長すぎる可能性があります"),TEXT("エラー"), MB_ICONWARNING);
		}
		return false;
	}
	lengh = strlen(buf);
	if (lengh > _T_MAX_PATH) {
		MessageBox(NULL, TEXT("ファイル名が長すぎます"),TEXT("エラー"), MB_ICONWARNING);
		return false;
	}

	for (unsigned long i=0; i <= strlen(buf); i++) {
		filePath[i] = buf[i];
	}
	for (unsigned long i=0; i <= strlen(name); i++) {
		fileName[i] = name[i];
	}
	return true;
}


INT_PTR CALLBACK visualAnalyzStartPointProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message) {
		case WM_INITDIALOG:
			addStartPartList(hDlg);
			break;
		case WM_COMMAND:
			switch (wParam) {
				case IDCANCEL:
					editState = EEDIT_STATE_IDLE;
					EndDialog(hDlg, LOWORD(wParam));
					break;
				case ID_ADAPT:
					staPartEditType = ESTART_PART_ADD;
					staPartEditStart(hDlg);
					break;
				case ID_EDIT:
					staPartEditType = ESTART_PART_EDIT;
					staPartEditStart(hDlg);
					break;
				case ID_DELETE:
					staPartEditType = ESTART_PART_DELETE;
					staPartEditStart(hDlg);
					break;
				case ID_TEST_SEND:
					startLogReceptionSimulation(hDlg);
					break;
				default:
					break;
			}
			break;
		case WM_DESTROY:
			break;
		default:
			break;
	}
	return (INT_PTR)FALSE;
}


INT_PTR CALLBACK visualAnalyzOperationSettingProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT selNum;

	switch (message) {
		case WM_INITDIALOG:
			addOperationList(hDlg);
			break;
		case WM_COMMAND:
			switch (wParam) {
				case IDCANCEL:
					editState = EEDIT_STATE_IDLE;
					EndDialog(hDlg, LOWORD(wParam));
					break;
				case ID_ADD_CONDITION:
					operationPos = operationNum;
					settingOperation(hDlg);
					break;
				case ID_EDIT_CONDITION:
					settingOperation(hDlg);
					break;
				default:
					break;
			}
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				if (lParam == (LPARAM)GetDlgItem(hDlg, ID_OPE_SELECT)) {
					selNum = SendMessage(GetDlgItem(hDlg, ID_OPE_SELECT), CB_GETCURSEL, 0, 0);
					operationPos = (unsigned short)selNum;
					readOperationSetting(hDlg);
				}
			}
			break;
		case WM_DESTROY:
			break;
		default:
			break;
	}
	return (INT_PTR)FALSE;
}


INT_PTR CALLBACK visualAnalyzDeviceSettingProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT selNum;
	unsigned short i;

	switch (message) {
		case WM_INITDIALOG:
			EnableWindow(GetDlgItem(hDlg, ID_DRIVE_TYPE), FALSE);
			EnableWindow(GetDlgItem(hDlg, ID_DRIVE_SOURCE), FALSE);
			break;
		case WM_COMMAND:
			switch (wParam) {
				case IDCANCEL:
					editState = EEDIT_STATE_IDLE;
					devEditType = EDEVICE_NONE;
					editDev = 0;
					EndDialog(hDlg, LOWORD(wParam));
					break;
				case IDOK:
					devEditType = EDEVICE_ADD;
					devEditStart(hDlg);
					break;
				case ID_EDIT:
					devEditType = EDEVICE_EDIT;
					devEditStart(hDlg);
					break;
				case ID_VIEW:
					devEditType = EDEVICE_VIEW;
					devSetHDlg = hDlg;
					devEditStart(hDlg);
					break;
				case ID_ROLLER1:
				case ID_ROLLER2:
					EnableWindow(GetDlgItem(hDlg, ID_DRIVE_TYPE), TRUE);
					EnableWindow(GetDlgItem(hDlg, ID_SIZE), TRUE);
					SendMessage(GetDlgItem(hDlg, ID_DRIVE_TYPE), CB_RESETCONTENT, 0, 0);
					SendMessage(GetDlgItem(hDlg, ID_DRIVE_TYPE), CB_ADDSTRING, 0, (LPARAM)"Motor");
					SendMessage(GetDlgItem(hDlg, ID_DRIVE_TYPE), CB_ADDSTRING, 0, (LPARAM)"Cluch");
					break; 
				case ID_SOLENOID:
				case ID_SENSOR:
					SendMessage(GetDlgItem(hDlg, ID_DRIVE_TYPE), WM_SETTEXT, 0, (LPARAM)"");
					SendMessage(GetDlgItem(hDlg, ID_DRIVE_TYPE), CB_RESETCONTENT, 0, 0);
					EnableWindow(GetDlgItem(hDlg, ID_DRIVE_TYPE), FALSE);
					EnableWindow(GetDlgItem(hDlg, ID_DRIVE_SOURCE), FALSE);
					EnableWindow(GetDlgItem(hDlg, ID_SIZE), FALSE);
					SendMessage(GetDlgItem(hDlg, ID_DRIVE_SOURCE), WM_SETTEXT, 0, (LPARAM)"");
					break;
				case ID_TEST_LOG_SEND:
					deviceLogReceptionSimulation(hDlg);
					break;
				default:
					break;
			}
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				if (lParam == (LPARAM)GetDlgItem(hDlg, ID_DRIVE_TYPE)) {
					selNum = SendMessage(GetDlgItem(hDlg, ID_DRIVE_TYPE), CB_GETCURSEL, 0, 0);
					if (selNum == EDRIVE_TYPE_CLUCH) {
						EnableWindow(GetDlgItem(hDlg, ID_DRIVE_SOURCE), TRUE);
						SendMessage(GetDlgItem(hDlg, ID_DRIVE_SOURCE), CB_RESETCONTENT, 0, 0);
						for (i = 0; i < driveSouceNum; i++) {
							SendMessage(GetDlgItem(hDlg, ID_DRIVE_SOURCE), CB_ADDSTRING, 0, (LPARAM)driveSouce[i].name);
						}
					} else {
						EnableWindow(GetDlgItem(hDlg, ID_DRIVE_SOURCE), FALSE);
						SendMessage(GetDlgItem(hDlg, ID_DRIVE_SOURCE), WM_SETTEXT, 0, (LPARAM)"");
					}
				}
			}
			break;
		case WM_DESTROY:
			break;
		default:
			break;
	}
	return (INT_PTR)FALSE;
}


INT_PTR CALLBACK visualAnalyzLineSettingProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND numWh;

	switch (message) {
		case WM_INITDIALOG:
			break;
		case WM_COMMAND:
			switch (wParam) {
				case IDCANCEL:
					editState = EEDIT_STATE_IDLE;
					EndDialog(hDlg, LOWORD(wParam));
					break;
				case IDOK:
					EndDialog(hDlg, LOWORD(wParam));
					break;
				case ID_ADAPT:
					editStart(hDlg);
					break;
				default:
					break;
			}
			break;
		case WM_DESTROY:
			break;
		default:
			break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK visualAnalyzTestModeProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char text[STR_LEN_MAX];
	int passTime;
	static bool play = false;
	static HWND hwndToolbar;
	static HBITMAP hToolBitmap;

	switch (message) {
		case WM_INITDIALOG:
				InitCommonControls();
				hToolBitmap = LoadBitmap(GetModuleHandle(0),MAKEINTRESOURCE(IDR_TOOLBAR3));
				hwndToolbar = CreateToolbarEx(
					hDlg, WS_CHILD | WS_VISIBLE, 0, 4,
					NULL, (UINT_PTR)hToolBitmap,
					tbAnalyzeButtonTestMode, 4, 0, 0, 0, 0, sizeof(TBBUTTON)
				);
				SetWindowText(GetDlgItem(hDlg, IDS_PASS_TIME), "50");
				SetFocus(GetDlgItem(hDlg, ECOMMAND_VA_FRAME));
			break;
		case WM_TIMER:
			if (wParam == TIMER_ID_1) {
				KillTimer(hDlg, wParam | TIMER_ID_1);
				if (play == true) {
					GetWindowText(GetDlgItem(hDlg, IDS_PASS_TIME), text, sizeof(text));
					passTime = atoi(text);
					updateVisualAnalyzeTime(crrunetTime + passTime);
					SetTimer(hDlg, TIMER_ID_1, passTime, NULL);
				}
			}
			break;
		case WM_COMMAND:
			switch (wParam) {
				case IDCANCEL:
					EndDialog(hDlg, LOWORD(wParam));
					break;
				case ECOMMAND_VA_FRAME:
					GetWindowText(GetDlgItem(hDlg, IDS_PASS_TIME), text, sizeof(text));
					passTime = atoi(text);
					updateVisualAnalyzeTime(crrunetTime + passTime);
					break;
				case ECOMMAND_VA_PLAY:
					if (play == false) {
						GetWindowText(GetDlgItem(hDlg, IDS_PASS_TIME), text, sizeof(text));
						passTime = atoi(text);
						play = true;
						updateVisualAnalyzeTime(crrunetTime + passTime);
						SetTimer(hDlg, TIMER_ID_1, passTime, NULL);
					}
					break;
				case ECOMMAND_VA_PAUSE:
					play = false;
					break;
				case ECOMMAND_VA_STOP:
					pageCancel();
					motionReset();
					play = false;
					break;
				default:
					break;
			}
			break;
		case WM_DESTROY:
			break;
		default:
			break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK visualAnalyzByPatternProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND hwndToolbar;
	static HBITMAP hToolBitmap;
	int state;
	char readIniFile[_T_MAX_PATH];
	char readIniName[_T_MAX_PATH];
	char relativepath[_T_MAX_PATH];
	char text[_T_MAX_PATH];

	switch (message) {
		case WM_INITDIALOG:
			InitCommonControls();
			hToolBitmap = LoadBitmap(GetModuleHandle(0),MAKEINTRESOURCE(IDR_TOOLBAR3));
			hwndToolbar = CreateToolbarEx(
				hDlg, WS_CHILD | WS_VISIBLE, 0, 5,
				NULL, (UINT_PTR)hToolBitmap,
				tbAnalyzeButtonByPattern, 5, 0, 0, 0, 0, sizeof(TBBUTTON)
			);
			settingPatternIndex = (selectPatternIndex == 0) ? 1 : selectPatternIndex;
			state = SendMessage(hwndToolbar, TB_GETSTATE, ECOMMAND_VA_1 + settingPatternIndex - 1, 0);
			state |= TBSTATE_CHECKED;
			SendMessage(hwndToolbar, TB_SETSTATE, ECOMMAND_VA_1 + settingPatternIndex - 1, (LPARAM)state);
			setBaseNumStr(hDlg, settingPatternIndex);
			break;
		case WM_COMMAND:
			switch (wParam) {
				case IDCANCEL:
					EndDialog(hDlg, LOWORD(wParam));
					break;
				case IDOK:
					EndDialog(hDlg, LOWORD(wParam));
					break;
				case ECOMMAND_VA_1:
				case ECOMMAND_VA_2:
				case ECOMMAND_VA_3:
				case ECOMMAND_VA_4:
				case ECOMMAND_VA_5:
					settingPatternIndex = wParam - ECOMMAND_VA_1 + 1;
					setBaseNumStr(hDlg, settingPatternIndex);
					break;
				case ID_VA_READ_FILE:
					if (setVaSettingIniFileName(hDlg,readIniFile, readIniName)) {
						sprintf_s(text, sizeof(text), "%s%s", szGlDrive, szGlPath);
						PathRelativePathToA(relativepath, text, FILE_ATTRIBUTE_DIRECTORY, readIniFile, FILE_ATTRIBUTE_NORMAL);
						sprintf_s(text, sizeof(text), "INI_FILE_%d", settingPatternIndex);
						WritePrivateProfileString("BY_PATTERN", text, relativepath, visualAnalyzerIni);
						readPattern(GetParent(hDlg), settingPatternIndex);
						setBaseNumStr(hDlg, settingPatternIndex);
					}
					break;
				case ID_PAT_CLEAR:
					clearPattern(hDlg, settingPatternIndex);
					break;
				default:
					break;
			}
			break;
		case WM_DESTROY:
			break;
		default:
			break;
	}
	return (INT_PTR)FALSE;
}

LRESULT CALLBACK visualAnalyzWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	static HDC hmdc;
	static HDROP hDrop;
	static HBITMAP hBitmap;
	static HWND hwndToolbar;
	static HBITMAP hToolBitmap;
	static HBRUSH hbr;
	unsigned char i;
	unsigned char timId;
	unsigned short timData;
	bool isSameTime;
	int buttonState;
	unsigned char select;
	char str[_T_MAX_PATH];

	switch (message) {
		case WM_CREATE:
			SetTimer(hWnd, TIMER_ID_1, 100, NULL);
			hdc = GetDC(hWnd);							//デバイスコンテキストの取得
			hmdc = CreateCompatibleDC(hdc);				//ウィンドウのデバイスコンテキストに関連付けられたメモリDCを作成
			hBitmap = CreateCompatibleBitmap(hdc, WINDOW_MDCH_W, WINDOW_MDCH_H);// ウィンドウのデバイスコンテキストと互換のあるビットマップを作成
			SelectObject(hmdc, hBitmap);				// メモリDCでビットマップを選択
			ReleaseDC(hWnd, hdc);						// デバイスコンテキストの解放	
			InitCommonControls();
			hToolBitmap = LoadBitmap(GetModuleHandle(0),MAKEINTRESOURCE(IDR_TOOLBAR3));
			hwndToolbar = CreateToolbarEx(
				hWnd, WS_CHILD | WS_VISIBLE, 0, 12,
				NULL, (UINT_PTR)hToolBitmap,
				tbAnalyzeButton, 12, 0, 0, 0, 0, sizeof(TBBUTTON)
			);
			DragAcceptFiles(hWnd, TRUE);
			if (isDispText) {
				CheckMenuItem(GetMenu(hWnd), ID_TEXT1, MF_BYCOMMAND | MFS_CHECKED);
			}
			if (isDispLog) {
				CheckMenuItem(GetMenu(hWnd), ID_LOG_DISP, MF_BYCOMMAND | MFS_CHECKED);
			}
			if (isDispInfo) {
				CheckMenuItem(GetMenu(hWnd), ID_INFOMATION, MF_BYCOMMAND | MFS_CHECKED);
			}
			hbr = CreateSolidBrush(RGB(255,255,255));
			createTimeDispWindow(hWnd);//時間表示window
			readVaIni(hWnd, visualAnalyzerIni);
			useCount("VISUAL_ANALYZER");
			break;
		case WM_TIMER:
			timId = wParam & 0x0f;
			timData = (wParam >> 8);
			if (timId == TIMER_ID_1) {
				InvalidateRect(hWnd, NULL, FALSE);
				for (i = 0; i < MOTION_NUM; i++) {
					if (profileInfo[i].state == EPRIFILE_ACTIVE) {
						if (isLastMotion(i)) {
							isActive[i] = true;
						}
						if (isActive[i]) {
							moveOn(i, MOTION_TIME, MOTION_TIME);
						}
					}
				}
				updateTimeDisp();
			} else if (timId == TIMER_ID_2) {
				KillTimer(hWnd, (timData << 8) | TIMER_ID_2);
				if (pauseFlag == false) {
					logReadState = ELOG_READ_ACTIVE;
					updateMotion(difTime);
					if (isLogWait == true) {
						visualAnalyzeLogInfo(vaLogFormMmsg);
					}
					fileReadExecute();
				} else {
					logReadState = ELOG_READ_PAUSE;
				}
			} else if (timId == TIMER_ID_3) {
				
			} else if (timId == TIMER_ID_4) {
				KillTimer(hWnd, (timData << 8) | TIMER_ID_4);
				if (logReadState != ELOG_READ_PAUSE ) {
					isSameTime = (rememberTime == deviceOperation[timData].tentativeTime);
					updateMotion(difTime);
					updateOperation(timData);
					logReadState = ELOG_READ_ACTIVE;
					if (isSameTime) {
						visualAnalyzeLogInfo(vaLogFormMmsg);
						fileReadExecute();
					} else {
						updateVisualAnalyzeTime(rememberTime);
					}
				}
			}
			break;
		case WM_CTLCOLORSTATIC:
			if (lParam == (LPARAM)timeWindw) {
				return (INT_PTR)hbr;
			}
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				//TOOLBAR_COMMAND
				case ECOMMAND_VA_SCALEUP:
					dispScale += 0.1;
					break;
				case ECOMMAND_VA_SCALEDOWN:
					dispScale -= 0.1;
					break;
				case ECOMMAND_VA_PLAY:
					pauseFlag = false;
					if (logReadState == ELOG_READ_PAUSE) {
						updateMotion(difTime);
						logReadState = ELOG_READ_ACTIVE;
						fileReadExecute();
					} else if (logReadState == ELOG_READ_STOP) {
						fileReadStart();
					} else {
					}
					break;
				case ECOMMAND_VA_PAUSE:
					pauseFlag = true;
					break;
				case ECOMMAND_VA_STOP:
					if (logReadState != ELOG_READ_NORMAL) {
						motionReset();
						logReadState = ELOG_READ_STOP;
					}
					break;
				case ECOMMAND_VA_FRAME:
					if (logReadState == ELOG_READ_PAUSE) {
						if (operationId == 0xffff) {
							updateMotion(difTime);
							if (isLogWait == true) {
								visualAnalyzeLogInfo(vaLogFormMmsg);
							}
							logReadState = ELOG_READ_ACTIVE;
							fileReadExecute();
							logReadState = ELOG_READ_PAUSE;
						} else {
							isSameTime = (rememberTime == deviceOperation[operationId].tentativeTime);
							updateMotion(difTime);
							updateOperation(operationId);
							if (isSameTime) {
								visualAnalyzeLogInfo(vaLogFormMmsg);
								logReadState = ELOG_READ_ACTIVE;
								fileReadExecute();
								logReadState = ELOG_READ_PAUSE;
							} else {
								updateVisualAnalyzeTime(rememberTime);
							}
						}
					}
					break;
				case ECOMMAND_VA_1:
					outputLog();
				case ECOMMAND_VA_2:
				case ECOMMAND_VA_3:
				case ECOMMAND_VA_4:
				case ECOMMAND_VA_5:
					select = wParam - ECOMMAND_VA_1 + 1;
					if (checkDiffExist(select)) {
						selectPatternIndex = select;
						for (i = 0; i < 5; i++) {
							buttonState = SendMessage(hwndToolbar, TB_GETSTATE, LOWORD(ECOMMAND_VA_1 + i), 0);
							if (selectPatternIndex != (i + 1)) {
								buttonState &= ~TBSTATE_CHECKED;
								SendMessage(hwndToolbar, TB_SETSTATE, LOWORD(ECOMMAND_VA_1 + i), (LPARAM)buttonState);
							} else {
								if (buttonState & TBSTATE_CHECKED) {
									buttonState &= ~TBSTATE_CHECKED;
									selectPatternIndex = 0;
								} else {
									buttonState |= TBSTATE_CHECKED;
								}
								SendMessage(hwndToolbar, TB_SETSTATE, LOWORD(wParam), (LPARAM)buttonState);
							}
						}
						readPattern(hWnd, selectPatternIndex);
					}
					break;
				case IDM_ADD_POS:
					CreateDialog(hInst, MAKEINTRESOURCE(IDD_VISUAL_ANALYZER_LINE), hWnd, visualAnalyzLineSettingProc);
					break;
				case IDM_DEVICE:
					CreateDialog(hInst, MAKEINTRESOURCE(IDD_VISUAL_ANALYZER_DEVICE), hWnd, visualAnalyzDeviceSettingProc);
					break;
				case IDM_START_POINT:
					CreateDialog(hInst, MAKEINTRESOURCE(IDD_VISUAL_ANALYZER_START_PART), hWnd, visualAnalyzStartPointProc);
					break;
				case IDM_OPERATION_SETTING:
					CreateDialog(hInst, MAKEINTRESOURCE(IDD_VISUAL_ANALYZER_OPERATION), hWnd, visualAnalyzOperationSettingProc);
					break;
				case IDM_BY_PATTERN:
					CreateDialog(hInst, MAKEINTRESOURCE(IDD_VISUAL_ANALYZER_BY_PATTERN), hWnd, visualAnalyzByPatternProc);
					break;
				case IDM_TEST_MODE:
					CreateDialog(hInst, MAKEINTRESOURCE(IDD_VISUAL_ANALYZER_TEST_MODE), hWnd, visualAnalyzTestModeProc);
					break;
				case ID_DISTANCE_VALUE:
					if (isDispText) {
						CheckMenuItem(GetMenu(hWnd), ID_DISTANCE_VALUE, MF_BYCOMMAND | MFS_UNCHECKED);
					} else {
						CheckMenuItem(GetMenu(hWnd), ID_DISTANCE_VALUE, MF_BYCOMMAND | MFS_CHECKED);
					}
					isDispText = !isDispText;
					break;
				case ID_LOG_DISP:
					if (isDispLog) {
						CheckMenuItem(GetMenu(hWnd), ID_LOG_DISP, MF_BYCOMMAND | MFS_UNCHECKED);
					} else {
						CheckMenuItem(GetMenu(hWnd), ID_LOG_DISP, MF_BYCOMMAND | MFS_CHECKED);
					}
					isDispLog = !isDispLog;
					break;
				case ID_INFOMATION:
					if (isDispInfo) {
						CheckMenuItem(GetMenu(hWnd), ID_INFOMATION, MF_BYCOMMAND | MFS_UNCHECKED);
					} else {
						CheckMenuItem(GetMenu(hWnd), ID_INFOMATION, MF_BYCOMMAND | MFS_CHECKED);
					}
					isDispInfo = !isDispInfo;
					break;
				case OVERWRITE_SAVE:
					sprintf_s(str, sizeof(str), "INI_FILE_%d", selectPatternIndex);
					GetPrivateProfileString("BY_PATTERN", str, visualAnalyzerIni, str, sizeof(str), visualAnalyzerIni);
					writeVaIni(hWnd, str);
					break;
				case ID_VA_FILE_READ:
					if (setVaSettingIniFileName(hWnd,visualAnalyzerIni, visualAnalyzerName)) {
						WritePrivateProfileString(_T("PATH"), _T("VISUAL_ANALYZER"), visualAnalyzerIni, logToolSettingPath);
						readVaIni(hWnd, visualAnalyzerIni);
					}
					break;
				case ID_VA_OTHER_NAME_SAVE:
					if (setVaSettingIniFileName(hWnd,visualAnalyzerIni, visualAnalyzerName)) {
						WritePrivateProfileString(_T("PATH"), _T("VISUAL_ANALYZER"), visualAnalyzerIni, logToolSettingPath);
						writeVaIni(hWnd, visualAnalyzerIni);
					}
					break;
				default:
					break;
			}
			break;
		case WM_USER:
			switch (LOWORD(wParam)) {
				case WM_USER_REMTIME:
					isSameTime = (rememberTime == deviceOperation[lParam].tentativeTime);
					updateMotion(difTime);
					updateOperation((unsigned short)lParam);
					if (isSameTime) {
						visualAnalyzeLogInfo(vaLogFormMmsg);
					} else {
						updateVisualAnalyzeTime(rememberTime);
					}
					break;
				default:
					break;
			}
			break;
		case WM_PAINT:
			updateAnalyzeWindow(hWnd, hmdc);
			break;
		case WM_LBUTTONDOWN:
			cursorDownX = (signed short)mousePosX;
			cursorDownY = (signed short)mousePosY;
			vaClickState = EVA_MOUSE_DOWN;
			break;
		case WM_MOUSEMOVE:	//マウスを動かしたとき
			mousePosX = (signed short)LOWORD(lParam);
			mousePosY = (signed short)HIWORD(lParam);
			vaMouseMove();
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case WM_LBUTTONUP:								//左クリックを離したとき	
			vaClickState = EVA_MOUSE_UP;
			InvalidateRect(hWnd, NULL, FALSE);
			vaLButtonUp();
			break;
		case WM_DROPFILES:								//ファイルをドロップしたとき
			hDrop = (HDROP)wParam;
			DragQueryFile(hDrop, 0, (PTSTR)fileName, 256);
			fileReadStart();
			DragFinish(hDrop);
			break;
		case WM_DESTROY:
			DeleteDC(hmdc);								// メモリDCの削除
			DeleteObject(hBitmap);						// ビットマップ オブジェクトの削除
			DeleteObject(hbr);							// ブラシを削除
			DeleteObject(hTimelFont);
			DestroyWindow(timeWindw);

			char text[STR_LEN_MAX];
			sprintf_s(text, sizeof(text), "%f", dispScale);
			WritePrivateProfileString(_T("VISUAL_ANALYZER"), _T("DISP_SCALE"), text, logToolSettingPath);
			sprintf_s(text, sizeof(text), "%d", moveOffSetX);
			WritePrivateProfileString(_T("VISUAL_ANALYZER"), _T("OFFSET_X"), text, logToolSettingPath);
			sprintf_s(text, sizeof(text), "%d", moveOffSetY);
			WritePrivateProfileString(_T("VISUAL_ANALYZER"), _T("OFFSET_Y"), text, logToolSettingPath);
#ifdef LOG_MODE
			PostQuitMessage(0);
#endif
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void readDevSetting(HWND hDlg)
{
	unsigned short dev = getDeviceId(selectPos);
	char str[STR_LEN_MAX];

	if (dev != -1) {
		sprintf_s(str, sizeof(str), "%d", devicePoint[dev].size);
		SendMessage(GetDlgItem(hDlg, ID_SIZE), WM_SETTEXT, 0, (LPARAM)str);
		SendMessage(GetDlgItem(hDlg, ID_NAME), WM_SETTEXT, 0, (LPARAM)devicePoint[dev].name);
		sprintf_s(str, sizeof(str), "%d", devicePoint[dev].value);
		SendMessage(GetDlgItem(hDlg, ID_TEST_NAME), WM_SETTEXT, 0, (LPARAM)devicePoint[dev].name);
		SendMessage(GetDlgItem(hDlg, ID_TEST_VALUE), WM_SETTEXT, 0, (LPARAM)str);
		sprintf_s(str, sizeof(str), "%d", devicePoint[dev].dir);
		SendMessage(GetDlgItem(hDlg, ID_TEST_ID), WM_SETTEXT, 0, (LPARAM)str);
		SendMessage(GetDlgItem(hDlg, ID_ROLLER1), BM_SETCHECK, (devicePoint[dev].type == EDEV_SINGLE_ROLLER ? BST_CHECKED : BST_UNCHECKED), 0);
		SendMessage(GetDlgItem(hDlg, ID_ROLLER2), BM_SETCHECK, (devicePoint[dev].type == EDEV_PAIR_ROLLER ? BST_CHECKED : BST_UNCHECKED), 0);
		SendMessage(GetDlgItem(hDlg, ID_SOLENOID), BM_SETCHECK, (devicePoint[dev].type == EDEV_SOLENOID ? BST_CHECKED : BST_UNCHECKED), 0);
		SendMessage(GetDlgItem(hDlg, ID_SENSOR), BM_SETCHECK, (devicePoint[dev].type == EDEV_SENSOR ? BST_CHECKED : BST_UNCHECKED), 0);
		SendMessage(GetDlgItem(hDlg, ID_SIGNAL), BM_SETCHECK, (devicePoint[dev].type == EDEV_SIGNAL ? BST_CHECKED : BST_UNCHECKED), 0);
		SendMessage(GetDlgItem(hDlg, ID_NONE_DEVICE), BM_SETCHECK, (devicePoint[dev].type == EDEV_NONE_DEVICE ? BST_CHECKED : BST_UNCHECKED), 0);

		if ((devicePoint[dev].type == EDEV_SINGLE_ROLLER) || (devicePoint[dev].type == EDEV_PAIR_ROLLER)) {
			EnableWindow(GetDlgItem(hDlg, ID_DRIVE_TYPE), TRUE);
			SendMessage(GetDlgItem(hDlg, ID_DRIVE_TYPE), CB_RESETCONTENT, 0, 0);
			SendMessage(GetDlgItem(hDlg, ID_DRIVE_TYPE), CB_ADDSTRING, 0, (LPARAM)"Motor");
			SendMessage(GetDlgItem(hDlg, ID_DRIVE_TYPE), CB_ADDSTRING, 0, (LPARAM)"Cluch");
			SendMessage(GetDlgItem(hDlg, ID_DRIVE_TYPE), CB_SETCURSEL, devicePoint[dev].driveType, 0);
			if (devicePoint[dev].driveType == EDRIVE_TYPE_CLUCH) {
				EnableWindow(GetDlgItem(hDlg, ID_DRIVE_SOURCE), TRUE);
				SendMessage(hDlg, WM_COMMAND, CBN_SELCHANGE << 16, (LPARAM)GetDlgItem(hDlg, ID_DRIVE_TYPE));
				if (devicePoint[dev].driveSouceDev != -1) {	
					SendMessage(GetDlgItem(hDlg, ID_DRIVE_SOURCE), CB_SETCURSEL, devicePoint[dev].driveSouceDev, 0);
				}
			} else {
				EnableWindow(GetDlgItem(hDlg, ID_DRIVE_SOURCE), FALSE);
			}
		} else {
			EnableWindow(GetDlgItem(hDlg, ID_DRIVE_SOURCE), FALSE);
			EnableWindow(GetDlgItem(hDlg, ID_DRIVE_TYPE), FALSE);
			SendMessage(GetDlgItem(hDlg, ID_DRIVE_TYPE), WM_SETTEXT, 0, (LPARAM)"");
			SendMessage(GetDlgItem(hDlg, ID_DRIVE_SOURCE), WM_SETTEXT, 0, (LPARAM)"");
		}
	}
}

void addDriveSouceList(char* newName, unsigned short dev)
{
	unsigned short i;
	size_t strLen;
	size_t addStrLen;

	for (i = 0; i < driveSouceNum; i++) {
		strLen = strlen(driveSouce[i].name);
		addStrLen = strlen(newName);
		if (strLen == addStrLen) {
			if (strncmp(driveSouce[i].name, newName, strLen) == 0) {
				break;
			}
		}
	}
	if (i == driveSouceNum) {
		strcpy_s(driveSouce[driveSouceNum].name, newName);
		driveSouce[driveSouceNum].dev = dev;
		driveSouceNum++;
	}
}

void deviceLogAnalyze(char* deviceStr, signed short val, signed char id)
{
	char* readline;
	unsigned short i;
	unsigned short bra;

	for (i = 0; i < deviceNum; i++) {
		readline = strstr(deviceStr, devicePoint[i].name);
		if(readline != NULL) {
			if (strlen(devicePoint[i].name) == strlen(deviceStr)) {
				devicePoint[i].preValue = devicePoint[i].value;
				devicePoint[i].value = val;
				switch (devicePoint[i].type) {
					case EDEV_SENSOR:
						if (devicePoint[i].value != 0) {
							proceedToPoint(id, EEDGE_TOP, devicePoint[i].point);
							updateBottomPointFoward(id);
						} else {
							proceedToPoint(id, EEDGE_BOTTOM, devicePoint[i].point);
						}
						break;
					case EDEV_PAIR_ROLLER:
					case EDEV_SINGLE_ROLLER:
						if (devicePoint[i].value != 0) {
							if ((devicePoint[i].dir != ENONE_DIRECTION) && (devicePoint[i].dir != (eDIRECTION)id)) {
								checkDirChange(i, val, (eDIRECTION)id);
							}
							devicePoint[i].preDir = devicePoint[i].dir;
							devicePoint[i].dir = (eDIRECTION)id;
						}
						break;
					case EDEV_SOLENOID:
						bra = getBranchId(devicePoint[i].point);
						if (bra != -1) {
							if (devicePoint[i].value != 0) {
								currentBranch[bra].nextPoint = analyzeBranch[bra].nextPoint2;
								signed short spId = getStartPartId(analyzeBranch[bra].nextPoint2);
								if (spId != -1) {
									foldPaper(startPoint[(unsigned char)spId].startPart);
								}
							} else {
								currentBranch[bra].nextPoint = analyzeBranch[bra].nextPoint1;
							}
						}
						break;
					default:
						break;
				}
			}
		}
	}
	for (i = 0; i < driveSouceNum; i++) {
		readline = strstr(deviceStr, driveSouce[i].name);
		if (readline != NULL) {
			if (strlen(driveSouce[i].name) == strlen(deviceStr)) {
				driveSouce[i].preValue = driveSouce[i].value;
				driveSouce[i].value = val;
				driveSouce[i].preDir = driveSouce[i].dir;
				driveSouce[i].dir = (eDIRECTION)id;
			}
		}
	}

}

bool isFold(unsigned char sPart)
{
	if ((sPart == 0xfe) || (sPart == 0xfd)) {
		return true;
	} else {
		return false;
	}

}
bool checkFoldPoint(unsigned short stPos, unsigned short id)
{
	unsigned short fPoint = analyzePoint[startPoint[stPos].point].prePoint;
	unsigned short pos = 0;
	bool ret = false;

	pos = topPosHistory[id].topPos;
	while (pos != topPosHistory[id].bottomPos) {
		if (topPosHistory[id].history[pos] == fPoint) {
			ret = true;
			break;
		}
		pos = (pos != 0) ? pos - 1 : HISTORY_BUF_NUM - 1;
	}
	return ret;
}

unsigned short getSubId()
{
	unsigned short i;

	for (i = MOTION_PAGE_NUM; i < MOTION_NUM; i++) {
		if (profileInfo[i].state == EPRIFILE_IDLE) {
			return i;
		}
	}
	return 0xffff;
}

void changeProfileInfo(unsigned short stPos, unsigned char id)
{
	unsigned short historyBuf[100] = { 0 };
	double distanceBuf[100] = { 0 };
	unsigned char historyBufPos = 0;
	unsigned short pos;
	unsigned short staPoint = startPoint[stPos].point;
	unsigned short braPoint = analyzePoint[staPoint].prePoint;
	unsigned short topNextPoint = topPosiInfo[id].nextPoint;
	unsigned short bottomNextPoint = bottomPosiInfo[id].nextPoint;

	double topPosiDisBuf = topPosiInfo[id].dis;
	double bottomPosiDisBuf = bottomPosiInfo[id].dis;
	double len1 = 0;
	double len2 = 0;
	unsigned short id2;

	id2 = getSubId();
	if (id2 == 0xffff) {
		return;
	}
	pos = topPosHistory[id].bottomPos;
	while (1) {
		historyBuf[historyBufPos] = topPosHistory[id].history[pos];
		distanceBuf[historyBufPos] = topPosHistory[id].distance[pos];
		historyBufPos++;
		if (pos == topPosHistory[id].topPos) {
			break;
		} else {
			pos++;
			pos = (pos != HISTORY_BUF_NUM) ? pos : 0;
		}
	}
	pos = 0;
	len1 = analyzePoint[analyzePoint[historyBuf[pos]].nextPoint].distance - bottomPosiInfo[id].dis;
	while (1) {
		pos++;
		if (historyBuf[pos] != braPoint) {
			len1 += analyzePoint[analyzePoint[historyBuf[pos]].nextPoint].distance;
		} else {
			break;
		}
	}
	len2 = profileInfo[id].length - len1;
	motionEnd(id);
	pos = 0;
	while (1) {
		topPosHistory[id].history[pos] = historyBuf[pos];
		topPosHistory[id].distance[pos] = distanceBuf[pos];
		if (historyBuf[pos] == braPoint) {
			pos++;
			break;
		} else {
			pos++;
		}
	}
	topPosHistory[id].history[pos] = staPoint;
	topPosHistory[id].distance[pos] = analyzePoint[staPoint].distance;
	topPosHistory[id].topPos = pos;
	topPosHistory[id].bottomPos = 0;
	bottomPosiInfo[id].point = historyBuf[0];
	bottomPosiInfo[id].dis = bottomPosiDisBuf;
	bottomPosiInfo[id].dir = EFOWARD;
	bottomPosiInfo[id].nextPoint = bottomNextPoint;
	topPosiInfo[id].dis = 0;
	topPosiInfo[id].point = staPoint;
	topPosiInfo[id].nextPoint = getNextTopPoint(topPosiInfo[id].point);
	profileInfo[id].state = EPRIFILE_SPLIT;
	profileInfo[id].waitId = 0xff;
	profileInfo[id].length = len1;
	profileInfo[id].mId = id;
	
	pos = 0;
	while (1) {
		topPosHistory[id2].history[pos] = historyBuf[historyBufPos - 1 - pos];
		topPosHistory[id2].distance[pos] = distanceBuf[historyBufPos - pos];
		if (topPosHistory[id2].history[pos] == braPoint) {
			pos++;
			break;
		} else {
			pos++;
		}
	}

	topPosHistory[id2].history[pos] = staPoint;
	topPosHistory[id2].distance[pos] = analyzePoint[staPoint].distance;
	topPosHistory[id2].topPos = pos;
	topPosHistory[id2].bottomPos = 0;
	topPosiInfo[id2].dis = 0;
	topPosiInfo[id2].point = staPoint;
	topPosiInfo[id2].nextPoint = getNextTopPoint(topPosiInfo[id2].point);
	topPosiInfo[id2].dir = EFOWARD;
	bottomPosiInfo[id2].dir = EREVERSE;
	bottomPosiInfo[id2].point = historyBuf[historyBufPos - 1];
	bottomPosiInfo[id2].dis = topPosiDisBuf;
	bottomPosiInfo[id2].nextPoint = topNextPoint;
	profileInfo[id2].state = EPRIFILE_SPLIT;
	profileInfo[id2].waitId = 0xff;
	profileInfo[id2].length = len2;
	profileInfo[id2].mId = id;

	if (profileInfo[id].length > profileInfo[id2].length) {
		profileInfo[id].prfType = EPROF_NORMAL;
		profileInfo[id2].prfType = EPROF_SPLIT;
	} else {
		profileInfo[id2].prfType = EPROF_NORMAL;
		profileInfo[id].prfType = EPROF_SPLIT;
	}
}

void foldPaper(unsigned char startPart)
{
	unsigned char i;
	unsigned short point = 0;
	unsigned short stPos = 0;

	for (i = 0; i < startPartNum; i++) {
		if (startPoint[i].startPart == startPart) {
			point = startPoint[i].point;
			stPos = i;
			break;
		}
	}
	for (i = 0; i < MOTION_NUM; i++) {
		if (profileInfo[i].state == EPRIFILE_ACTIVE) {
			if (checkFoldPoint(stPos, i)) {
				changeProfileInfo(stPos, i);
			}
		}
	}
	for (i = 0; i < MOTION_NUM; i++) {
		if (profileInfo[i].state == EPRIFILE_SPLIT) {
			profileInfo[i].startPart = startPart;
			profileInfo[i].state = EPRIFILE_ACTIVE;
			if (bottomPosiInfo[i].dir == EFOWARD) {
				updateBottomPointFoward(i);
			} else {
				updateBottomPointSwitch(i);
			}
		}
	}
	
}

void startAnalyze(unsigned char startPart, double length, unsigned char id)
{
	unsigned short point = 0;
	unsigned short pos = 0;
	unsigned short stPos = 0;
	unsigned char i;

	if (id >= MOTION_NUM) {
		return;
	}
	if (profileInfo[id].state == EPRIFILE_ACTIVE) {
		return;//すでに搬送中の場合無視。詰みなおしの場合もあるため
	}
	for (i = 0; i < startPartNum; i++) {
		if (startPoint[i].startPart == startPart) {
			point = startPoint[i].point;
			stPos = i;
			break;
		}
	}
	profileInfo[id].startPart = startPart;
	profileInfo[id].length = length;
	profileInfo[id].waitId = 0xff;
	profileInfo[id].prfType = EPROF_NORMAL;
	profileInfo[id].mId = id;

	for (i = 0; i < MOTION_NUM; i++) {
		if ((profileInfo[i].state == EPRIFILE_ACTIVE) || (profileInfo[i].state == EPRIFILE_WAIT)) {
			pos = topPosHistory[i].topPos;
			while (pos != topPosHistory[i].bottomPos) {
				if (topPosHistory[i].history[pos] == point) {
					profileInfo[id].state = EPRIFILE_PAUSE;
					profileInfo[startPoint[stPos].latestId].waitId = id;
					break;
				}
				pos = (pos != 0) ? pos - 1 : HISTORY_BUF_NUM - 1;
			}
		}
	}
	startPoint[stPos].latestId = id;
	if (profileInfo[id].state == EPRIFILE_IDLE) {
		profileInfo[id].state = EPRIFILE_ACTIVE;
	}
	if (profileInfo[id].startPart == 0xff) {
		profileInfo[id].state = EPRIFILE_WAIT;
	}

	motionIintial(point, id);
	updateBottomPointFoward(id);
}

void motionIintial(unsigned short point, unsigned char id)
{
	if (profileInfo[id].startPart == 0xff) {
		topPosiInfo[id].point = point;
		topPosiInfo[id].nextPoint = getNextTopPoint(topPosiInfo[id].point);
		topPosiInfo[id].dir = EFOWARD;
		topPosiInfo[id].dis = 0;
		bottomPosiInfo[id] = topPosiInfo[id];
		topPosHistory[id].history[0] = analyzePoint[point].prePoint;
		topPosHistory[id].distance[0] = 0;
		topPosHistory[id].history[1] = point;
		topPosHistory[id].distance[1] = analyzePoint[point].distance;
		topPosHistory[id].topPos = 1;
		topPosHistory[id].bottomPos = 0;
		initOverLength[id] = 0;
	} else {
		topPosiInfo[id].point = point;
		topPosiInfo[id].nextPoint = getNextTopPoint(topPosiInfo[id].point);
		topPosiInfo[id].dir = EFOWARD;
		topPosiInfo[id].dis = 0;
		bottomPosiInfo[id].nextPoint = point;
		bottomPosiInfo[id].point = analyzePoint[point].prePoint;
		bottomPosiInfo[id].dir = EFOWARD;
		initOverLength[id] = profileInfo[id].length - analyzePoint[point].distance;
		topPosHistory[id].history[0] = analyzePoint[point].prePoint;
		topPosHistory[id].distance[0] = 0;
		topPosHistory[id].history[1] = point;
		topPosHistory[id].distance[1] = analyzePoint[point].distance;
		topPosHistory[id].topPos = 1;
		topPosHistory[id].bottomPos = 0;
	}
}

CONVERT_VALOG convertVisualAnalyzeLog (char* form_msg)
{
	CONVERT_VALOG ret;
	char devStr[256];
	char* context; //strtok_sの内部で使用
	char* token;
	unsigned short devId;
	signed short val = 0;
	unsigned char id = 0;
	unsigned char io = 0;
	unsigned char startPart = 0;
	signed char dir = 0;
	double length = 0;

	strcpy_s(devStr, form_msg);
	token = strtok_s(devStr, " ", &context);

	if (token != NULL) {
		if (strncmp(token, "Input", 6) == 0) {
			io = 0;
			token = strtok_s(NULL, " ", &context);
			if (token != NULL) {
				devId = atoi(token);
				token = strtok_s(NULL, " ", &context);
				if (token != NULL) {
					val = atoi(token);
					token = strtok_s(NULL, " ", &context);
					if (token != NULL) {
						id = atoi(token);
						sprintf_s(devStr, 6, "%04x%d", devId, io);
						GetPrivateProfileString(_T("DEVICE_NAME"), devStr, devStr, devStr, sizeof(devStr), szIniFilePublic);
						strcpy_s(ret.name, devStr);
						ret.val = val;
						ret.idDir = id;
						ret.type = EVALOG_INPUT;
					}
				}
			}
		}
		if (strncmp(token, "Output", 7) == 0) {
			io = 1;
			token = strtok_s(NULL, " ", &context);
			if (token != NULL) {
				devId = atoi(token);
				token = strtok_s(NULL, " ", &context);
				if (token != NULL) {
					val = atoi(token);
					token = strtok_s(NULL, " ", &context);
					if (token != NULL) {
						dir = atoi(token);
						sprintf_s(devStr, 6, "%04x%d", devId, io);
						GetPrivateProfileString(_T("DEVICE_NAME"), devStr, devStr, devStr, sizeof(devStr), szIniFilePublic);
						strcpy_s(ret.name, devStr);
						ret.val = val;
						ret.idDir = dir;
						ret.type = EVALOG_OUTPUT;
					}
				}
			}
		}
		if (strncmp(token, "Start", 6) == 0) {
			token = strtok_s(NULL, " ", &context);
			if (token != NULL) {
				startPart = atoi(token);
				token = strtok_s(NULL, " ", &context);
				if (token != NULL) {
					length = atoi(token) / 10;
					token = strtok_s(NULL, " ", &context);
					if (token != NULL) {
						id = atoi(token);
						strcpy_s(ret.name, startPoint[startPart].name);
						ret.val = (int)length;
						ret.idDir = id;
						ret.startPart = startPart;
						ret.type = EVALOG_START;
					}
				}
			}
		}
	}
	return ret;
}

void visualAnalyzeLogWait(char* form_msg)
{
	CONVERT_VALOG convert;
	strcpy_s(vaLogFormMmsg, form_msg);
	isLogWait = true;
	convert = convertVisualAnalyzeLog(form_msg);
	sprintf_s(&form_msg[0], 100, "%s %d %d\n",convert.name, convert.val, convert.idDir);
}

void visualAnalyzeLogInfo(char* form_msg)
{    
	isLogWait = false;
	CONVERT_VALOG cnvLog = convertVisualAnalyzeLog(form_msg);
		
	switch (cnvLog.type) {
		case EVALOG_INPUT:
			deviceLogAnalyze(cnvLog.name, (signed short)cnvLog.val, cnvLog.idDir);
			sprintf_s(&form_msg[0], 100, "%s %d %d\n",cnvLog.name, cnvLog.val, cnvLog.idDir);
			operationCheck(cnvLog.name);
			break;
		case EVALOG_OUTPUT:
			deviceLogAnalyze(cnvLog.name, (signed short)cnvLog.val, cnvLog.idDir);
			sprintf_s(&form_msg[0], 100, "%s %d %d\n", cnvLog.name, cnvLog.val, cnvLog.idDir);
			operationCheck(cnvLog.name);
			break;
		case EVALOG_START:
			startAnalyze(cnvLog.startPart, cnvLog.val, cnvLog.idDir);
			sprintf_s(&form_msg[0], 100, "%s %d %d\n",cnvLog.name, cnvLog.val, cnvLog.idDir);
			break;
		default:
			break;
	}
}

void deviceLogReceptionSimulation(HWND hDlg)
{
	char text[STR_LEN_MAX];
	signed short val;
	unsigned char id;
	GetWindowText(GetDlgItem(hDlg, ID_TEST_VALUE), text, sizeof(text));
	val = atoi(text);
	GetWindowText(GetDlgItem(hDlg, ID_TEST_ID), text, sizeof(text));
	id = atoi(text);
	GetWindowText(GetDlgItem(hDlg, ID_TEST_NAME), text, sizeof(text));
	deviceLogAnalyze(text, val, id);
	operationCheck(text);
}

void startLogReceptionSimulation(HWND hDlg)
{
	char text[STR_LEN_MAX];
	unsigned short len;
	unsigned char id;
	LRESULT selNum = SendMessage(GetDlgItem(hDlg, ID_START_PART_LIST), CB_GETCURSEL, 0, 0);

	GetWindowText(GetDlgItem(hDlg, ID_LENGTH), text, sizeof(text));
	len = atoi(text);
	GetWindowText(GetDlgItem(hDlg, ID_PAGE_ID), text, sizeof(text));
	id = atoi(text);
	sprintf_s(text, sizeof(text), "Start %d %d %d",startPartTable[selNum].partId, len, id);
	visualAnalyzeLogInfo(text);
}


void updateTimeDisp()
{
	char text[STR_LEN_MAX];
	double dispTime = (double)(crrunetTime - difTime) / 1000;;

	sprintf_s(text, sizeof(text), "%fsec", dispTime);
	SendMessage(timeWindw, WM_SETTEXT, 0, (LPARAM)(text));
}


void updateAnalyzeWindow(HWND hWnd, HDC hdc)
{
	HDC hdc1;
	HDC hdc2;
	PAINTSTRUCT ps;
	HBRUSH hBrush;
	static WINDOWINFO windowInfo;
	int winWidth;
	int winHeight;

	// window幅、高さ取得
	GetWindowInfo(hWnd, &windowInfo);
	winWidth = windowInfo.rcWindow.right - windowInfo.rcWindow.left;
	winHeight = windowInfo.rcWindow.bottom - windowInfo.rcWindow.top;

	//LTGRAYブラシの作成&選択
	HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0xc0, 0xc0, 0xc0));
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
	SelectObject(hdc, hPen);
	hBrush = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	SelectObject(hdc, hBrush);
	Rectangle(hdc, 0, 0, winWidth, winHeight);

	//現在のツールバーの画像をメモリDCに転送
	hdc1 = GetDC(hWnd);
	BitBlt(hdc, 0, 0, WINDOW_WIDTH_VA, 28, hdc1, 0, 0, SRCCOPY);
	ReleaseDC(hWnd, hdc1);
	//ツールバー延長分
	hBrush = (HBRUSH)(CreateSolidBrush(RGB(0xf0,0xF0,0xF0)));
	SelectObject(hdc, hBrush);
	Rectangle(hdc, WINDOW_WIDTH_VA - 17, 0, winWidth, 29);
	DeleteObject(hBrush);
	
	SetBkMode(hdc, TRANSPARENT);

	if (pointNum > 0) {
		if (editState == EEDIT_STATE_EDIT) {
			setEditPos();
			calcConnectDistance();
		}
		dispLine(hWnd, hdc);
	}
	dispPoint(hWnd, hdc);
	dispBranch(hWnd, hdc);
	dispDevice(hWnd, hdc);
	dispStartPoint(hWnd, hdc);
	dispMotion(hWnd, hdc);
	dispSelectPoint(hWnd, hdc);
	if (isDispText) {
		dispText(hWnd, hdc);
	}
	if (isDispInfo) {
		dispInfomation(hWnd, hdc);
	}

	hdc2 = BeginPaint(hWnd, &ps);
	// メモリDCから画像を転送
	BitBlt(hdc2, 0, 0, winWidth, winHeight, hdc, 0, 0, SRCCOPY);

	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);

	EndPaint(hWnd, &ps);
}

//マウスが動いているときの処理
void vaMouseMove()
{
	selectPos = getNearPoint();
	signed short dev;
	signed short sp;
	
	switch (editState) {
		case EEDIT_STATE_EDIT:
			switch (editType) {
				case EPOINT_TYPE_NEW_POINT:
					analyzePoint[editPos].X = inConvertOffsetPointX(mousePosX);
					analyzePoint[editPos].Y = inConvertOffsetPointY(mousePosY);
					break;
				case EPOINT_TYPE_EDIT_POINT:
					if (analyzePoint[editPos].prePoint == editPos) {
						analyzePoint[editPos].X = inConvertOffsetPointX(mousePosX);
						analyzePoint[editPos].Y = inConvertOffsetPointY(mousePosY);
					}
					break;
				case EPOINT_TYPE_GROUPING:
					analyzePoint[textPos].texPostDisX = (long)(inConvertOffsetPointX(mousePosX) - analyzePoint[textPos].X);
					analyzePoint[textPos].texPostDisY = (long)(inConvertOffsetPointY(mousePosY) - analyzePoint[textPos].Y);
					break;
				case EPOINT_TYPE_ADD_START_POINT:
					startPoint[editSp].X = inConvertOffsetPointX(mousePosX) - analyzePoint[startPoint[editSp].point].X;
					startPoint[editSp].Y = inConvertOffsetPointY(mousePosY) - analyzePoint[startPoint[editSp].point].Y;
					break;
				case EPOINT_TYPE_SINGLE_ROLLER:
				case EPOINT_TYPE_PAIR_ROLLER:
				case EPOINT_TYPE_SENSOR:
				case EPOINT_TYPE_SOLENOID:
				case EPOINT_TYPE_SIGNAL:
					devicePoint[editDev].disX = inConvertOffsetPointX(mousePosX) - analyzePoint[devicePoint[editDev].point].X;
					devicePoint[editDev].disY = inConvertOffsetPointY(mousePosY) - analyzePoint[devicePoint[editDev].point].Y;
				default:
					break;
			}
			break;
		case EEDIT_STATE_SELECT:
			switch (editType) {
				case EPOINT_TYPE_ADD_POINT:
				case EPOINT_TYPE_CONNECT_POINT:
				case EPOINT_TYPE_BRANCH:
				case EPOINT_TYPE_SWITCH_BACK:
					analyzePoint[editPos].prePoint = selectPos;
					break;
				case EPOINT_TYPE_SPLIT:
				case EPOINT_TYPE_GROUPING:
					break;
				case EPOINT_TYPE_SINGLE_ROLLER:
				case EPOINT_TYPE_PAIR_ROLLER:
				case EPOINT_TYPE_SENSOR:
				case EPOINT_TYPE_SOLENOID:
				case EPOINT_TYPE_SIGNAL:
					if (devEditType == EDEVICE_ADD) {
						editDev = deviceNum;
						devicePoint[editDev].point = selectPos;
					} else {
						dev = getDeviceId(selectPos);
						editDev = (dev != -1) ? dev : deviceNum;
					}
					break;
				case EPOINT_TYPE_DVICE_VIEW:
				case EPOINT_TYPE_NONE_DEVICE:
					editDev = getDeviceId(selectPos);
					break;
				case EPOINT_TYPE_ADD_START_POINT:
					if (staPartEditType == ESTART_PART_ADD) {
						editSp = startPartNum;
					} else {
						sp = getStartPartId(selectPos);
						editSp = (sp != -1) ? sp : startPartNum;
					}
				default:
					break;
			}
			break;
		default:
			break;
	}
	
	if (vaClickState == EVA_MOUSE_DOWN) {
		movingOffSetX = (signed short)(mousePosX - cursorDownX);
		movingOffSetY = (signed short)(mousePosY - cursorDownY);
	}
	
}


//左クリックを離したときの処理
void vaLButtonUp()
{
	switch (editState) {
		case EEDIT_STATE_SELECT:
			switch (editType) {
				case EPOINT_TYPE_ADD_POINT:
					editState = EEDIT_STATE_EDIT;
					break;
				case EPOINT_TYPE_EDIT_POINT:
					editPos = selectPos;
					editState = EEDIT_STATE_EDIT;
					if (analyzePoint[editPos].isConnect == true) {
						selectPos = analyzePoint[editPos].nextPoint;
					}
					if (editDistance == 0) {
						editDistance = analyzePoint[editPos].distance;
					}
					break;
				case EPOINT_TYPE_CONNECT_POINT:
					analyzePoint[editPos].isConnect = true;
					editState = EEDIT_STATE_EDIT;
					break;
				case EPOINT_TYPE_BRANCH:
					editState = EEDIT_STATE_EDIT;
					analyzeBranch[branchNum].point = selectPos;
					break;
				case EPOINT_TYPE_SWITCH_BACK:
					editState = EEDIT_STATE_EDIT;
					break;
				case EPOINT_TYPE_SPLIT:
					analyzePoint[editPos].prePoint = analyzePoint[selectPos].prePoint;
					analyzePoint[analyzePoint[selectPos].prePoint].nextPoint = editPos;
					analyzePoint[editPos].nextPoint = selectPos;
					analyzePoint[selectPos].prePoint = editPos;
					splitPos = selectPos;
					editState = EEDIT_STATE_EDIT;
					//ポイントの順番が昇順ではなくなるためポイントの数の回数分位置を再計算しておく
					for (int i = 0;i < editPos; i++) {
						setEditPos();
					}
					break;
				case EPOINT_TYPE_GROUPING:
					textPos = analyzePoint[selectPos].nextPoint;
					analyzePoint[selectPos].isGroupe = true;
					editState = EEDIT_STATE_EDIT;
					break;
				case EPOINT_TYPE_ADD_START_POINT:
					if (staPartEditType == ESTART_PART_DELETE) {
						if (startPoint[editSp].startPart == editSartPart) {
							strcpy_s(startPoint[editSp].name, "");
							startPoint[editSp].X = 0;
							startPoint[editSp].Y = 0;
							startPoint[editSp].point = 0;
							editState = EEDIT_STATE_IDLE;
						}
						break;
					}
					strcpy_s(startPoint[editSp].name, startPartName);
					startPoint[editSp].startPart = editSartPart;
					startPoint[editSp].X = inConvertOffsetPointX(mousePosX) - analyzePoint[selectPos].X;
					startPoint[editSp].Y = inConvertOffsetPointY(mousePosY) - analyzePoint[selectPos].Y;
					startPoint[editSp].point = selectPos;
					if (editSartPart == 0xff) {
						duStartPartId = editSp;
					}
					editState = EEDIT_STATE_EDIT;
					break;
				case EPOINT_TYPE_SINGLE_ROLLER:
				case EPOINT_TYPE_PAIR_ROLLER:
				case EPOINT_TYPE_SENSOR:
				case EPOINT_TYPE_SIGNAL:
					devicePoint[editDev].disX = inConvertOffsetPointX(mousePosX) - analyzePoint[devicePoint[editDev].point].X;
					devicePoint[editDev].disY = inConvertOffsetPointY(mousePosY) - analyzePoint[devicePoint[editDev].point].Y;
					devicePoint[editDev].size = deviceSize;
					devicePoint[editDev].driveType = driveType;
					devicePoint[editDev].driveSouceDev = driveSouceDev;
					strcpy_s(devicePoint[editDev].name, deviceName);
					devicePoint[editDev].type = deviceType;
					editState = EEDIT_STATE_EDIT;
					break;
				case EPOINT_TYPE_SOLENOID:
					devicePoint[editDev].driveSouceDev = -1;
					strcpy_s(devicePoint[editDev].name, deviceName);
					devicePoint[editDev].type = deviceType;
					editState = EEDIT_STATE_IDLE;
					if (devEditType == EDEVICE_ADD) {
						deviceNum++;
					}
					break;
				case EPOINT_TYPE_DVICE_VIEW:
					readDevSetting(devSetHDlg);
					editState = EEDIT_STATE_IDLE;
					break;
				case EPOINT_TYPE_NONE_DEVICE:
					devicePoint[editDev].type = deviceType;
					editState = EEDIT_STATE_IDLE;
					break;
				default:
					break;
			}
			break;
		case EEDIT_STATE_EDIT:
			if (analyzePoint[editPos].isConnect == true) {
				double preX = analyzePoint[analyzePoint[editPos].prePoint].X;
				double preY = analyzePoint[analyzePoint[editPos].prePoint].Y;
				double nextX = analyzePoint[editPos].X;
				double nextY = analyzePoint[editPos].Y;
				analyzePoint[editPos].distance = calcDistance(preX, preY, nextX, nextY);
				analyzePoint[editPos].dispDistance = editDistance;
			} else {
				analyzePoint[editPos].distance = editDistance;
			}
			switch (editType) {
				case EPOINT_TYPE_NEW_POINT:
					analyzePoint[editPos].X = inConvertOffsetPointX(mousePosX);
					analyzePoint[editPos].Y = inConvertOffsetPointY(mousePosY);
					analyzePoint[editPos].distance = 0;
					analyzePoint[editPos + 1].prePoint = editPos;
					analyzePoint[editPos].isHideLength = isHideLength;
					pointNum++;
					editPos++;
					editState = EEDIT_STATE_EDIT;
					editType = EPOINT_TYPE_ADD_POINT;
					break;
				case EPOINT_TYPE_ADD_POINT:
					if (analyzePoint[analyzePoint[editPos].prePoint].nextPoint == 0) {
						analyzePoint[analyzePoint[editPos].prePoint].nextPoint = editPos;
					}
					analyzePoint[editPos].isHideLength = isHideLength;
					pointNum++;
					editPos++;
					editState = EEDIT_STATE_IDLE;
					break;
				case EPOINT_TYPE_BRANCH:
					analyzeBranch[branchNum].nextPoint1 = analyzePoint[analyzeBranch[branchNum].point].nextPoint;
					analyzeBranch[branchNum].nextPoint2 = editPos;
					if (checkSwitchPoint(analyzeBranch[branchNum].point) != -1) {
						analyzeBranch[branchNum].nextPoint1 = checkSwitchPoint(analyzeBranch[branchNum].point);
					}
					currentBranch[branchNum].point = analyzeBranch[branchNum].point;
					currentBranch[branchNum].nextPoint = analyzeBranch[branchNum].nextPoint1;
					analyzePoint[editPos].isHideLength = isHideLength;
					branchNum++;
					pointNum++;
					editPos++;
					editState = EEDIT_STATE_IDLE;
					break;
				case EPOINT_TYPE_EDIT_POINT:
				case EPOINT_TYPE_GROUPING:
					analyzePoint[editPos].isHideLength = isHideLength;
					editPos = pointNum;
					editState = EEDIT_STATE_IDLE;
					break;
				case EPOINT_TYPE_CONNECT_POINT:
					analyzePoint[analyzePoint[editPos].prePoint].nextPoint = editPos;
					analyzePoint[editPos].isHideLength = isHideLength;
					pointNum++;
					editPos++;
					editState = EEDIT_STATE_IDLE;
					break;
				case EPOINT_TYPE_SPLIT:
					analyzePoint[editPos].isHideLength = isHideLength;
					pointNum++;
					editPos++;
					editState = EEDIT_STATE_IDLE;
					break;
				case EPOINT_TYPE_SWITCH_BACK:
					positionSwitch[switchBackNum].point = analyzePoint[editPos].prePoint;
					positionSwitch[switchBackNum].nextPoint = editPos;
					analyzePoint[editPos].isHideLength = isHideLength;
					switchBackNum++;
					pointNum++;
					editPos++;
					editState = EEDIT_STATE_IDLE;
					break;
				case EPOINT_TYPE_ADD_START_POINT:
					if (staPartEditType == ESTART_PART_ADD) {
						startPartNum++;
					}
					editState = EEDIT_STATE_IDLE;
					break;
				case EPOINT_TYPE_SINGLE_ROLLER:
				case EPOINT_TYPE_PAIR_ROLLER:
				case EPOINT_TYPE_SENSOR:
				case EPOINT_TYPE_SIGNAL:
					if (devicePoint[editDev].driveType == EDRIVE_TYPE_MOTOR) {
						addDriveSouceList(devicePoint[editDev].name, editDev);
					}
					if (devEditType == EDEVICE_ADD) {
						deviceNum++;
					}
					editState = EEDIT_STATE_IDLE;
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}

	updateOffset();
}

void dispPoint(HWND hWnd, HDC hdc)
{
	HPEN hPen = CreatePen(PS_SOLID, 6, RGB(0,255,0));
	HPEN hPen2 = CreatePen(PS_DOT, 1, RGB(255, 255, 255));
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
	unsigned short prePoint = analyzePoint[editPos].prePoint;
	unsigned short i;
	
	if (editState == EEDIT_STATE_EDIT) {
		writePoint(hdc, analyzePoint[prePoint].X, analyzePoint[prePoint].Y);
	} else if (editState == EEDIT_STATE_SELECT) {
		writePoint(hdc, analyzePoint[prePoint].X , analyzePoint[prePoint].Y);
	}

	SelectObject(hdc, hPen2);
	for (i = 0; i < pointNum; i++) {
		if (analyzePoint[i].isGroupe == false) {
			writePoint(hdc, analyzePoint[i].X, analyzePoint[i].Y);
		}
	}
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);
	DeleteObject(hPen2);
}

void dispSelectPoint(HWND hWnd, HDC hdc)
{
	HPEN hPen1 = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
	HPEN hPen2 = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen1);
	double posX = analyzePoint[selectPos].X;
	double posY = analyzePoint[selectPos].Y;
	unsigned short point;

	if ((editState == EEDIT_STATE_SELECT) || ((editState == EEDIT_STATE_EDIT) && (editType == EPOINT_TYPE_CONNECT_POINT))) {
		movePen(hdc, posX, posY);
		writeLine(hdc, posX - 5, posY - 15);
	}	
	if ((editType == EPOINT_TYPE_EDIT_POINT) && (editState == EEDIT_STATE_SELECT)) {
		SelectObject(hdc, hPen2);
		movePen(hdc, posX, posY);
		writeLine(hdc, analyzePoint[analyzePoint[selectPos].prePoint].X, analyzePoint[analyzePoint[selectPos].prePoint].Y);
	}
	if ((editType == EPOINT_TYPE_SPLIT) && (editState == EEDIT_STATE_SELECT)) {
		SelectObject(hdc, hPen2);
		movePen(hdc, posX, posY);
		writeLine(hdc, analyzePoint[analyzePoint[selectPos].prePoint].X, analyzePoint[analyzePoint[selectPos].prePoint].Y);
	}
	if ((editType == EPOINT_TYPE_GROUPING) && (editState == EEDIT_STATE_SELECT)) {
		SelectObject(hdc, hPen2);
		movePen(hdc, posX, posY);
		writeLine(hdc, analyzePoint[analyzePoint[selectPos].prePoint].X, analyzePoint[analyzePoint[selectPos].prePoint].Y);
		point = analyzePoint[selectPos].prePoint;
		while (analyzePoint[point].isGroupe) {
			writeLine(hdc, analyzePoint[analyzePoint[point].prePoint].X, analyzePoint[analyzePoint[point].prePoint].Y);
			point = analyzePoint[point].prePoint;
		}
		point = selectPos;
		movePen(hdc, posX, posY);
		while (analyzePoint[point].isGroupe) {
			writeLine(hdc, analyzePoint[analyzePoint[point].nextPoint].X, analyzePoint[analyzePoint[point].nextPoint].Y);
			point = analyzePoint[point].nextPoint;
		}
	}
	if (editState == EEDIT_STATE_SELECT) {
		char str[STR_LEN_MAX];
		if ((devEditType == EDEVICE_VIEW) && (getDeviceId(selectPos) != -1)) {
			long length = sprintf_s(str, sizeof(str), "%s", devicePoint[editDev].name);
			COLORREF oldTextColor;
			oldTextColor = SetTextColor(hdc, RGB(255, 0, 0));
			writeText(hdc, posX - 5, posY - 15, str, length);
			SetTextColor(hdc, oldTextColor);
		}
	}
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen1);
	DeleteObject(hPen2);
}

void setEditPos()
{
	unsigned short num = getPointNum();

	for (unsigned short i = 1; i < num; i++) {
		if (analyzePoint[i].isConnect == true) {
			connectPoint(i);
		} else {
			if (analyzePoint[i].prePoint != i){
				normalPoint(i);
			}
		}
	}
}

void dispLine(HWND hWnd, HDC hdc)
{
	HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	HPEN hPen2 = CreatePen(PS_DOT, 1, RGB(255, 255, 255));
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
	unsigned short num = getPointNum();

	for (unsigned short i = 1; i < num; i++) {
		(i == editPos ? SelectObject(hdc, hPen2) : SelectObject(hdc, hPen));
		movePen(hdc, analyzePoint[analyzePoint[i].prePoint].X, analyzePoint[analyzePoint[i].prePoint].Y);
		writeLine(hdc, analyzePoint[i].X, analyzePoint[i].Y);
	}
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);
	DeleteObject(hPen2);
}

unsigned short getPointNum()
{
	unsigned short num = pointNum;

	if (editState == EEDIT_STATE_EDIT) {
		switch (editType) {
			case EPOINT_TYPE_ADD_POINT:
			case EPOINT_TYPE_CONNECT_POINT:
			case EPOINT_TYPE_BRANCH:
			case EPOINT_TYPE_SWITCH_BACK:
			case EPOINT_TYPE_SPLIT:
				num = pointNum + 1;
				break;
			default:
				break;
		}
	}
	return num;
}

void dispText(HWND hWnd, HDC hdc)
{
	char str[STR_LEN_MAX];
	long length;
	HFONT hFont;
	COLORREF oldTextColor;
	double displayDistance;
	double errorRange;
	unsigned short point;
	hFont = CreateFont(15, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH, "Meiryo UI" );

	SelectObject(hdc, hFont);
	for (unsigned short i = 1; i < pointNum; i++) {
		if (analyzePoint[i].prePoint == i) {
			continue;
		}
		if (analyzePoint[i].isGroupe) {
			continue;
		}
		if (analyzePoint[analyzePoint[i].prePoint].isGroupe) {
			analyzePoint[i].texPostX = (signed long)(analyzePoint[i].X + analyzePoint[i].texPostDisX);
			analyzePoint[i].texPostY = (signed long)(analyzePoint[i].Y + analyzePoint[i].texPostDisY);
		} else {
			analyzePoint[i].texPostX = (signed long)(analyzePoint[i].X + analyzePoint[analyzePoint[i].prePoint].X) / 2;
			analyzePoint[i].texPostY = (signed long)(analyzePoint[i].Y + analyzePoint[analyzePoint[i].prePoint].Y) / 2;
		}
		displayDistance = analyzePoint[i].distance;
		errorRange = analyzePoint[i].distance - analyzePoint[i].dispDistance;
		oldTextColor = SetTextColor(hdc, RGB(0, 0, 0));
		if ((analyzePoint[i].dispDistance != 0)) {
			if ((errorRange > -0.5) && (errorRange < 0.5)) {
				displayDistance = analyzePoint[i].dispDistance;
			} else {
				oldTextColor = SetTextColor(hdc, RGB(255, 0, 0));
			}
		}
		point = analyzePoint[i].prePoint;
		while (analyzePoint[point].isGroupe) {
			displayDistance += analyzePoint[point].distance;
			point = analyzePoint[point].prePoint;
		}
		length = sprintf_s(str, sizeof(str), "%.1f", displayDistance);
		if (analyzePoint[i].isHideLength == false) {
			writeText(hdc, analyzePoint[i].texPostX, analyzePoint[i].texPostY, str, length);
		}
		SetTextColor(hdc, oldTextColor);
	}
	DeleteObject(hFont);
}

double calcTopBottomDisFoward(unsigned short id)
{
	double sum;
	unsigned short pos = topPosHistory[id].topPos;
	unsigned short failSafeCount = 0;

	sum = topPosiInfo[id].dis;
	while (topPosHistory[id].history[pos] != bottomPosiInfo[id].point) {
		sum += topPosHistory[id].distance[pos];
		pos = (pos != 0) ? pos - 1 : HISTORY_BUF_NUM - 1;
		failSafeCount++;
		if (failSafeCount > FAIL_SAFE_MAX) {
			break;
		}
	}
	sum -= bottomPosiInfo[id].dis;
	return sum;
}

double calcTopBottomDisReverse(unsigned short id)
{
	double sum;
	unsigned short next;
	unsigned short pos = topPosHistory[id].topPos;
	unsigned short failSafeCount = 0;

	sum = analyzePoint[topPosiInfo[id].nextPoint].distance - topPosiInfo[id].dis;
	pos = (pos != 0) ? pos - 1 : HISTORY_BUF_NUM - 1;
	while (topPosHistory[id].history[pos] != bottomPosiInfo[id].point) {
		next = analyzePoint[topPosHistory[id].history[pos]].nextPoint;
		sum += analyzePoint[next].distance;
		failSafeCount++;
		if (failSafeCount > FAIL_SAFE_MAX) {
			break;
		}
		pos = (pos != 0) ? pos - 1 : HISTORY_BUF_NUM - 1;
	}
	sum += bottomPosiInfo[id].dis;
	return sum;
}

double calcTopBottomDisSwitch(unsigned short id)
{
	double sum;
	unsigned short pos = topPosHistory[id].topPos;
	unsigned short failSafeCount = 0;

	sum = topPosiInfo[id].dis;
	while (topPosHistory[id].history[pos] != bottomPosiInfo[id].point) {
		sum += topPosHistory[id].distance[pos];
		pos = (pos != 0) ? pos - 1 : HISTORY_BUF_NUM - 1;
		failSafeCount++;
		if (failSafeCount > FAIL_SAFE_MAX) {
			break;
		}
	}
	sum += bottomPosiInfo[id].dis;
	return sum;
}

double calcTopBottomDis (unsigned short id)
{
	double ret  = 0;
	unsigned short pos = topPosHistory[id].topPos;
	unsigned short failSafeCount = 0;

	if (topPosiInfo[id].dir == EFOWARD) {
		if (bottomPosiInfo[id].dir == EFOWARD) {
			ret = calcTopBottomDisFoward(id);
		} else {
			ret = calcTopBottomDisSwitch(id);
		}
	} else {
		ret = calcTopBottomDisReverse(id);
	}
	return ret;
}

void dispInfomation(HWND hWnd, HDC hdc)
{
	char str[STR_LEN_MAX];
	long length;
	HFONT hFont;
	COLORREF oldTextColor;
	unsigned char i;
	unsigned char count;
	double plen;

	hFont = CreateFont(15, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH, "Meiryo UI" );
	oldTextColor = SetTextColor(hdc, RGB(255, 255, 255));
	SelectObject(hdc, hFont);
	length = sprintf_s(str, sizeof(str), "point:%d", selectPos);
	TextOut(hdc, 1, 30, str, length);

	count = 0;
	for (i = 0; i < MOTION_NUM; i++) {
		if (profileInfo[i].state != EPRIFILE_IDLE) {
			count++;
			plen = calcTopBottomDis(i);
			length = sprintf_s(str, sizeof(str), "page:%d top:%d+%.2f btm:%d+%.2f len=%.2f", i, topPosiInfo[i].point, topPosiInfo[i].dis, bottomPosiInfo[i].point, bottomPosiInfo[i].dis, plen);
			TextOut(hdc, 1, 30 + (10 * count), str, length);
		}
	}
	SetTextColor(hdc, oldTextColor);
	DeleteObject(hFont);
}

void dispStartPoint(HWND hWnd, HDC hdc)
{
	double textPosX;
	double textPosY;
	unsigned short num = startPartNum;
	char str[STR_LEN_MAX];
	long length;
	HFONT hFont;

	hFont = CreateFont(15, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH, "Meiryo UI" );
	HPEN hPen = CreatePen(PS_SOLID, 1,  RGB(100, 232, 232));
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
	SelectObject(hdc, hFont);

	if (editState == EEDIT_STATE_EDIT) {
		if (editType == EPOINT_TYPE_ADD_START_POINT) {
			num++;
		}
	}

	for (unsigned short i = 0; i < num; i++) {
		textPosX = analyzePoint[startPoint[i].point].X + startPoint[i].X;
		textPosY = analyzePoint[startPoint[i].point].Y + startPoint[i].Y;
		if ((startPoint[i].X == 0) && (startPoint[i].Y == 0)) {
			continue;
		}
		length = sprintf_s(str, sizeof(str), "%s", startPoint[i].name);
		writeText(hdc, textPosX - 5, textPosY - 10, str, length);
		movePen(hdc, textPosX , textPosY);
		writeLine(hdc,  analyzePoint[startPoint[i].point].X, analyzePoint[startPoint[i].point].Y);
	}
	DeleteObject(hFont);
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);
}

void dispBranch(HWND hWnd, HDC hdc)
{
	HPEN hPen1 = CreatePen(PS_SOLID, 3, RGB(0, 0, 0));
	HPEN hPen2 = CreatePen(PS_SOLID, 3, RGB(255, 255, 255));
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen1);

	for (unsigned short i = 0; i < branchNum; i++) {
		if ((editState == EEDIT_STATE_SELECT) && (editType == EPOINT_TYPE_SOLENOID) && (analyzeBranch[i].point == selectPos)) {
			(HPEN)SelectObject(hdc, hPen2);
		} else {
			(HPEN)SelectObject(hdc, hPen1);
		}
		movePen(hdc, analyzePoint[currentBranch[i].point].X, analyzePoint[currentBranch[i].point].Y);
		writeLine(hdc, analyzePoint[currentBranch[i].nextPoint].X, analyzePoint[currentBranch[i].nextPoint].Y);
	}

	SelectObject(hdc, hOldPen);
	DeleteObject(hPen1);
	DeleteObject(hPen2);
}

void dispDevice(HWND hWnd, HDC hdc)
{
	HPEN hPen;
	HPEN hPen2;
	unsigned short num = getDeviceNum();
	bool onOff = false;
	COLORREF devColor;
	unsigned char penSize;
	
	for (unsigned short i = 0; i < num; i++) {
		if (((devicePoint[i].type == EDEV_SINGLE_ROLLER) || (devicePoint[i].type == EDEV_PAIR_ROLLER)) && (devicePoint[i].driveType == EDRIVE_TYPE_CLUCH) && (devicePoint[i].driveSouceDev != -1)) {
			onOff = (devicePoint[i].value != 0) && (driveSouce[devicePoint[i].driveSouceDev].value != 0);
		} else {
			onOff = (devicePoint[i].value != 0);
		}
		devColor = (onOff == true) ? RGB(0, 0, 255) : RGB(30, 30, 30);

		if (devicePoint[i].type == EDEV_SENSOR) {
			penSize = 3;
		} else if (devicePoint[i].type == EDEV_SIGNAL) {
			if (devicePoint[i].value != 0) {
				penSize = 3;
			} else {
				penSize = 1;
				devColor = RGB(255, 255, 0);
			}
		} else {
			penSize = 1;
		}
		hPen = CreatePen(PS_SOLID, penSize, devColor);
		hPen2 = CreatePen(PS_DOT, (int)(devicePoint[i].size * dispScale), devColor);
		SelectObject(hdc, hPen);
		if (devicePoint[i].type == EDEV_PAIR_ROLLER) {
			movePen(hdc, analyzePoint[devicePoint[i].point].X - devicePoint[i].disX, analyzePoint[devicePoint[i].point].Y - devicePoint[i].disY);
			writeLine(hdc, analyzePoint[devicePoint[i].point].X + devicePoint[i].disX, analyzePoint[devicePoint[i].point].Y + devicePoint[i].disY);
			SelectObject(hdc, hPen2);
			writePoint(hdc, analyzePoint[devicePoint[i].point].X - devicePoint[i].disX, analyzePoint[devicePoint[i].point].Y - devicePoint[i].disY);
			writePoint(hdc, analyzePoint[devicePoint[i].point].X + devicePoint[i].disX, analyzePoint[devicePoint[i].point].Y + devicePoint[i].disY);
		} else if (devicePoint[i].type == EDEV_SINGLE_ROLLER) {
			movePen(hdc, analyzePoint[devicePoint[i].point].X , analyzePoint[devicePoint[i].point].Y);
			writeLine(hdc, analyzePoint[devicePoint[i].point].X + devicePoint[i].disX, analyzePoint[devicePoint[i].point].Y + devicePoint[i].disY);
			SelectObject(hdc, hPen2);
			writePoint(hdc, analyzePoint[devicePoint[i].point].X + devicePoint[i].disX, analyzePoint[devicePoint[i].point].Y + devicePoint[i].disY);
		} else if (devicePoint[i].type == EDEV_SENSOR) {
			movePen(hdc, analyzePoint[devicePoint[i].point].X - devicePoint[i].disX, analyzePoint[devicePoint[i].point].Y - devicePoint[i].disY);
			writeLine(hdc, analyzePoint[devicePoint[i].point].X + devicePoint[i].disX, analyzePoint[devicePoint[i].point].Y + devicePoint[i].disY);
		} else if (devicePoint[i].type == EDEV_SIGNAL) {
			movePen(hdc, analyzePoint[devicePoint[i].point].X , analyzePoint[devicePoint[i].point].Y);
			writeLine(hdc, analyzePoint[devicePoint[i].point].X + devicePoint[i].disX, analyzePoint[devicePoint[i].point].Y + devicePoint[i].disY);
		}
		DeleteObject(hPen);
		DeleteObject(hPen2);
	}
}

unsigned short getDeviceNum()
{
	unsigned short num = deviceNum;

	if (editState == EEDIT_STATE_EDIT) {
		switch (editType) {
			case EPOINT_TYPE_SINGLE_ROLLER:
			case EPOINT_TYPE_PAIR_ROLLER:
			case EPOINT_TYPE_SENSOR:
			case EPOINT_TYPE_SIGNAL:
				if (devEditType == EDEVICE_ADD) {
					num = deviceNum + 1;
				}
				break;
			default:
				break;
		}
	}
	return num;
}

unsigned short getTopVectorPoint(unsigned char id)
{
	unsigned short ret = 0;
	unsigned short j;

	if (topPosiInfo[id].nextPoint == 0) {
		ret = topPosiInfo[id].point;
	} else {
		if (analyzePoint[analyzePoint[topPosiInfo[id].point].nextPoint].isConnect) {
			ret = analyzePoint[topPosiInfo[id].point].nextPoint;
			for (j = 0; j < branchNum; j++) {
				if (topPosiInfo[id].point == currentBranch[j].point) {
					ret = currentBranch[j].nextPoint;
					break;
				}
			}
		} else {
			ret = topPosiInfo[id].nextPoint;
		}
	}
	return ret;
}

unsigned short getbottomVectorPoint(unsigned char id)
{
	unsigned short ret = 0;

	if (analyzePoint[analyzePoint[bottomPosiInfo[id].point].nextPoint].isConnect) {
		ret = analyzePoint[bottomPosiInfo[id].point].nextPoint;
	} else {
		ret = bottomPosiInfo[id].nextPoint;
	}
	return ret;
}

void dispMotion(HWND hWnd, HDC hdc)
{
	unsigned char i;
	signed long tPosX;
	signed long tPosY;
	signed long posX;
	signed long posY;
	unsigned short pos;
	unsigned short pre;
	unsigned short next;
	double rate;
	HPEN hPen = CreatePen(PS_SOLID, 4, RGB(0,255,0));
	HPEN hPen2 = CreatePen(PS_SOLID, 4, RGB(255,0,0));
	HPEN hPen3 = CreatePen(PS_SOLID, 4, RGB(200,200,200));
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
	COLORREF oldTextColor = SetTextColor(hdc, RGB(255, 255, 255));
	char str[STR_LEN_MAX];
	int length;
	
	for (i = 0; i < MOTION_NUM; i++) {
		if (profileInfo[i].state != EPRIFILE_IDLE) {
			if ((topPosiInfo[i].dis != 0) || (topPosiInfo[i].point != 0)) {
				SelectObject(hdc, hPen);
				next = getTopVectorPoint(i);
				rate = topPosiInfo[i].dis / analyzePoint[next].distance;
				tPosX = signed long(analyzePoint[topPosiInfo[i].point].X + analyzePoint[next].distanceX * rate);
				tPosY = signed long(analyzePoint[topPosiInfo[i].point].Y + analyzePoint[next].distanceY * rate);
				movePen(hdc, tPosX, tPosY);

				pos = topPosHistory[i].topPos;
				if (topPosiInfo[i].dir == EFOWARD) {
					while (topPosHistory[i].history[pos] != bottomPosiInfo[i].point) {
						pre = topPosHistory[i].history[pos];
						writeLine(hdc, analyzePoint[pre].X, analyzePoint[pre].Y);
						pos = (pos != 0) ? pos - 1 : HISTORY_BUF_NUM - 1;
					}
				} else {
					while (topPosHistory[i].history[pos] != bottomPosiInfo[i].point) {
						pos = (pos != 0) ? pos - 1 : HISTORY_BUF_NUM - 1;
						pre = topPosHistory[i].history[pos];
						writeLine(hdc, analyzePoint[pre].X, analyzePoint[pre].Y);
					}
				}
				if ((bottomPosiInfo[i].dir == EREVERSE) && (topPosiInfo[i].dir == EFOWARD)) {
					writeLine(hdc, analyzePoint[bottomPosiInfo[i].point].X, analyzePoint[bottomPosiInfo[i].point].Y);
				}
				next = analyzePoint[bottomPosiInfo[i].point].nextPoint;
				if (analyzePoint[next].isConnect == false) {
					next = (bottomPosiInfo[i].nextPoint == 0 ? bottomPosiInfo[i].point : bottomPosiInfo[i].nextPoint);
				}
				rate = bottomPosiInfo[i].dis / analyzePoint[next].distance;
				posX = signed long(analyzePoint[bottomPosiInfo[i].point].X + analyzePoint[next].distanceX * rate);
				posY = signed long(analyzePoint[bottomPosiInfo[i].point].Y + analyzePoint[next].distanceY * rate);
				writeLine(hdc, posX, posY);
				if (profileInfo[i].state == EPRIFILE_ACTIVE) {
					SelectObject(hdc, hPen2);
				} else {
					SelectObject(hdc, hPen3);
				}
				writePoint(hdc, tPosX, tPosY);
				if (profileInfo[i].prfType == EPROF_NORMAL) {
					length = sprintf_s(str, sizeof(str), "%d", profileInfo[i].mId);
					writeText(hdc, tPosX + getWaitNum(i) * 10, tPosY, str, length);
				}
			}
		}
	}

	SetTextColor(hdc, oldTextColor);
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);
	DeleteObject(hPen2);
	DeleteObject(hPen3);
}

void editStart(HWND hDlg)
{
	char text[STR_LEN_MAX];

	GetWindowText(GetDlgItem(hDlg, ID_DISTANCE), text, sizeof(text));
	editDistance = atof(text);
	isHideLength = (SendMessage(GetDlgItem(hDlg, ID_CHECK), BM_GETCHECK, 0, 0) == BST_CHECKED);

	if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, ID_NEW_POINT), BM_GETCHECK, 0, 0)) {
		editType = EPOINT_TYPE_NEW_POINT;
		editState = EEDIT_STATE_EDIT;
		analyzePoint[editPos].prePoint = editPos;
	} else if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, ID_ADD_POINT), BM_GETCHECK, 0, 0)) {
		editType = EPOINT_TYPE_ADD_POINT;
		editState = EEDIT_STATE_SELECT;
	} else if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, ID_CONNECT_POINT), BM_GETCHECK, 0, 0)) {
		editType = EPOINT_TYPE_CONNECT_POINT;
		editState = EEDIT_STATE_SELECT;
	} else if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, ID_EDIT_POINT), BM_GETCHECK, 0, 0)) {
		editType = EPOINT_TYPE_EDIT_POINT;
		editState = EEDIT_STATE_SELECT;
	} else if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, ID_BRANCH), BM_GETCHECK, 0, 0)) {
		editType = EPOINT_TYPE_BRANCH;
		editState = EEDIT_STATE_SELECT;
	} else if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, ID_SWITCH_BACK), BM_GETCHECK, 0, 0)) {
		editType = EPOINT_TYPE_SWITCH_BACK;
		editState = EEDIT_STATE_SELECT;
	} else if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, ID_SPLIT), BM_GETCHECK, 0, 0)) {
		editType = EPOINT_TYPE_SPLIT;
		editState = EEDIT_STATE_SELECT;
	} else if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, ID_GROUPING), BM_GETCHECK, 0, 0)) {
		editType = EPOINT_TYPE_GROUPING;
		editState = EEDIT_STATE_SELECT;
	} else {
		editType = EPOINT_TYPE_EDIT_POINT;
		editState = EEDIT_STATE_SELECT;
	}
}

void devEditStart(HWND hDlg)
{
	char text[STR_LEN_MAX];
	GetWindowText(GetDlgItem(hDlg, ID_SIZE), text, sizeof(text));
	deviceSize = atoi(text);
	GetWindowText(GetDlgItem(hDlg, ID_NAME), deviceName, sizeof(deviceName));
	GetWindowText(GetDlgItem(hDlg, ID_DRIVE_SOURCE), driveSroceName, sizeof(driveSroceName));
	driveType = (eDRIVE_TYPE)SendMessage(GetDlgItem(hDlg, ID_DRIVE_TYPE) , CB_GETCURSEL , 0 , 0);
	driveSouceDev = (signed short)SendMessage(GetDlgItem(hDlg, ID_DRIVE_SOURCE), CB_GETCURSEL, 0, 0);

	if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, ID_ROLLER1), BM_GETCHECK, 0, 0)) {
		editType = EPOINT_TYPE_SINGLE_ROLLER;
		editState = EEDIT_STATE_SELECT;
		deviceType = EDEV_SINGLE_ROLLER;
	} else if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, ID_ROLLER2), BM_GETCHECK, 0, 0)) {
		editType = EPOINT_TYPE_PAIR_ROLLER;
		editState = EEDIT_STATE_SELECT;
		deviceType = EDEV_PAIR_ROLLER;
	} else if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, ID_SENSOR), BM_GETCHECK, 0, 0)) {
		editType = EPOINT_TYPE_SENSOR;
		editState = EEDIT_STATE_SELECT;
		deviceType = EDEV_SENSOR;
	} else if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, ID_SOLENOID), BM_GETCHECK, 0, 0)) {
		editType = EPOINT_TYPE_SOLENOID;
		editState = EEDIT_STATE_SELECT;
		deviceType = EDEV_SOLENOID;
	} else if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, ID_SIGNAL), BM_GETCHECK, 0, 0)) {
		editType = EPOINT_TYPE_SIGNAL;
		editState = EEDIT_STATE_SELECT;
		deviceType = EDEV_SIGNAL;
	} else {
		editType = EPOINT_TYPE_NONE_DEVICE;
		editState = EEDIT_STATE_SELECT;
		deviceType = EDEV_NONE_DEVICE;
	}
	if (devEditType == EDEVICE_VIEW) {
		editType = EPOINT_TYPE_DVICE_VIEW;
		editState = EEDIT_STATE_SELECT;
	}
}

void staPartEditStart(HWND hDlg)
{
	LRESULT selNum = SendMessage(GetDlgItem(hDlg, ID_START_PART_LIST), CB_GETCURSEL, 0, 0);

	editSartPart = startPartTable[selNum].partId;
	strcpy_s(startPartName, startPartTable[selNum].name);
	editType = EPOINT_TYPE_ADD_START_POINT;
	editState = EEDIT_STATE_SELECT;
}

void addStartPartList(HWND hDlg)
{
	unsigned short tableSize = (sizeof(startPartTable) / sizeof(startPartTable[0]));
	unsigned char i;

	for (i = 0; i < tableSize; i++) {
		SendMessage(GetDlgItem(hDlg, ID_START_PART_LIST), CB_ADDSTRING, 0, (LPARAM)startPartTable[i].name);
	}
}

void addOperationList(HWND hDlg)
{
	unsigned short i;
	unsigned short j;
	char text[STR_LEN_MAX];
	int itemCount;
	char *addName;
	bool isAdd;

	SendMessage(GetDlgItem(hDlg, ID_OPE_DEV1), CB_ADDSTRING, 0, (LPARAM)"");
	SendMessage(GetDlgItem(hDlg, ID_OPE_DEV2), CB_ADDSTRING, 0, (LPARAM)"");
	SendMessage(GetDlgItem(hDlg, ID_OPE_DEV), CB_ADDSTRING, 0, (LPARAM)"");
	for (i = 0; i < deviceNum + driveSouceNum; i++) {
		addName = (i < deviceNum) ? devicePoint[i].name : driveSouce[i - deviceNum].name;
		itemCount = SendMessage(GetDlgItem(hDlg, ID_OPE_DEV1), CB_GETCOUNT, 0, 0);
		isAdd = true;
		for (j = 0; j < itemCount; j++) {
			SendMessage(GetDlgItem(hDlg, ID_OPE_DEV1), CB_GETLBTEXT, j, (LPARAM)text);
			char *readline = strstr(text, addName);
			if(readline != NULL) {
				if (strlen(addName) == strlen(text)) {
					isAdd = false;
				}
			}
		}
		if (isAdd) {
			SendMessage(GetDlgItem(hDlg, ID_OPE_DEV1), CB_ADDSTRING, 0, (LPARAM)addName);
			SendMessage(GetDlgItem(hDlg, ID_OPE_DEV2), CB_ADDSTRING, 0, (LPARAM)addName);
			SendMessage(GetDlgItem(hDlg, ID_OPE_DEV), CB_ADDSTRING, 0, (LPARAM)addName);
		}
	}
	for (i = 0; i < operationNum; i++) {
		SendMessage(GetDlgItem(hDlg, ID_OPE_SELECT), CB_ADDSTRING, 0, (LPARAM)deviceOperation[i].name);
	}
}

DEVICE_ID_INFO getDevId(char *name)
{
	DEVICE_ID_INFO ret ={EDEVID_NONE, -1};
	unsigned short i;
	char *line;

	for (i = 0; i < deviceNum; i++) {
		line = strstr(devicePoint[i].name, name);
		if (line != NULL) {
			if (strlen(devicePoint[i].name) == strlen(name)) {
				ret.id = i;
				ret.type = EDEVID_DEV;
				break;
			}
		}
	}
	if (ret.id == -1) {
		for (i = 0; i < driveSouceNum; i++) {
			line = strstr(driveSouce[i].name, name);
			if (line != NULL) {
				if (strlen(driveSouce[i].name) == strlen(name)) {
					ret.id = i;
					ret.type = EDEVID_SOUCE;
					break;
				}
			}
		}
	}
	return ret;
}

void operationCheck(char* devName)
{
	unsigned char i;
	unsigned char j;
	signed short subjectVal;
	signed short changeDevVal;
	signed short changeDevPreVal;
	eDIRECTION subjectDir;
	eDIRECTION changeDevDir;
	bool judge;
	DEVICE_ID_INFO dev = getDevId(devName);
	char opeValStr[100];
	char opeValDir[100];

	if (dev.id == -1) {
		return;
	} else if (dev.type == EDEVID_DEV) {
		if ((devicePoint[dev.id].value == devicePoint[dev.id].preValue) && (devicePoint[dev.id].dir == devicePoint[dev.id].preDir)) {
			if (devicePoint[dev.id].notFirst == true) {
				return;
			}
		}
		devicePoint[dev.id].notFirst = true;
		changeDevVal = devicePoint[dev.id].value;
		changeDevPreVal = devicePoint[dev.id].preValue;
		changeDevDir = devicePoint[dev.id].dir;
	} else {
		if (driveSouce[dev.id].value == driveSouce[dev.id].preValue) {
			if (driveSouce[dev.id].notFirst == true) {
				return;
			}
		}
		driveSouce[dev.id].notFirst = true;
		changeDevVal = driveSouce[dev.id].value;
		changeDevPreVal = driveSouce[dev.id].preValue;
	}

	for (i = 0; i < operationNum; i++) {
		if ((dev.type == deviceOperation[i].dev[0].type) && (dev.id == deviceOperation[i].dev[0].id)) {
		} else if ((dev.type == deviceOperation[i].dev[1].type) && (dev.id == deviceOperation[i].dev[1].id)) {
		} else {
			continue;
		}
		for (j = 0; j < OPE_CHECK_NUM; j++) {
			if (deviceOperation[i].dev[j].type == EDEVID_DEV) {
				subjectVal = devicePoint[deviceOperation[i].dev[j].id].value;
				subjectDir = devicePoint[deviceOperation[i].dev[j].id].dir;
			} else if (deviceOperation[i].dev[j].type == EDEVID_SOUCE) {
				subjectVal = driveSouce[deviceOperation[i].dev[j].id].value;
				subjectDir = EFOWARD;
			} else {
			}
			switch(deviceOperation[i].checkType[j]) {
				case ECHECK_VALUE:
					judge = comparison(subjectVal, deviceOperation[i].checkValue[j], deviceOperation[i].ope[j]);
					break;
				case ECHECK_DIR:
					judge = comparison(subjectDir, deviceOperation[i].checkValue[j], deviceOperation[i].ope[j]);
					break;
				case ECHECK_NONE:
				default:
					judge = true;
					break;
			}
			if (judge == false) {
				break;
			}
		}
		if (judge == true) {
			char formula[100];
			sprintf_s(formula, "[value]=%d",changeDevVal);
			calculateData(formula);
			sprintf_s(formula, "[dir]=%d",changeDevDir);
			calculateData(formula);
			if (deviceOperation[i].operationFomura[0] != NULL) {
				strcpy_s(formula, deviceOperation[i].operationFomura);
				calculateData(formula);
			}
			if (deviceOperation[i].stepTime != 0) {
				if (deviceOperation[i].stepNum == 0) {
					strcpy_s(opeValStr, deviceOperation[i].opeValueStr);
					strcpy_s(opeValDir, deviceOperation[i].opeIdDirStr);
					deviceOperation[i].opeValue = (unsigned short)numericalCalculation(opeValStr);
					deviceOperation[i].opeIdDir = (signed char)numericalCalculation(opeValDir);
					deviceOperation[i].tentativeTime = crrunetTime + deviceOperation[i].stepTime;
					deviceOperation[i].timeWait = true;
				} else {
					deviceOperation[i].stepValue = (changeDevVal - changeDevPreVal) / deviceOperation[i].stepNum;
					deviceOperation[i].stepCount = 1;
					deviceOperation[i].stepNowValue = changeDevPreVal + deviceOperation[i].stepValue;
					deviceOperation[i].stepDir = changeDevDir;
					deviceOperation[i].tentativeTime = crrunetTime + deviceOperation[i].stepTime;
					deviceOperation[i].opeValue = changeDevVal;
					deviceOperation[i].timeWait = true;
					deviceLogAnalyze(deviceOperation[i].opeDevName, deviceOperation[i].stepNowValue, deviceOperation[i].stepDir);
				}
			} else if ((deviceOperation[i].opeValueStr[0] != NULL) && (deviceOperation[i].opeIdDirStr[0] != NULL)) {
				strcpy_s(opeValStr, deviceOperation[i].opeValueStr);
				strcpy_s(opeValDir, deviceOperation[i].opeIdDirStr);
				deviceOperation[i].opeValue = (unsigned short)numericalCalculation(opeValStr);
				deviceOperation[i].opeIdDir = (signed char)numericalCalculation(opeValDir);
				deviceLogAnalyze(deviceOperation[i].opeDevName, deviceOperation[i].opeValue, deviceOperation[i].opeIdDir);
			} else {
			}
		}
	}
}

void setCondition(unsigned short opeId)
{
	char *check;
	unsigned char pos = 0;
	unsigned char i;

	for (i = 0; i < OPE_CHECK_NUM; i++) {
		fillInSpace(deviceOperation[opeId].condition[i]);
		check = deviceOperation[opeId].condition[i];
		if (strncmp(check, "value", 5) == 0) {
			deviceOperation[opeId].checkType[i] = ECHECK_VALUE;
			pos = 5;
		} else if (strncmp(check, "dir", 3) == 0) {
			deviceOperation[opeId].checkType[i] = ECHECK_DIR;
			pos = 3;
		} else {
		}
		if (pos != 0) {
			switch (check[pos]) {
				case '<':
					deviceOperation[opeId].ope[i] = ECOMPARISON_LESS;
					pos++;
					if (check[pos] == '=') {
						deviceOperation[opeId].ope[i] = ECOMPARISON_LESS_EQUAL;
						pos++;
					}
					break;
				case '>':
					deviceOperation[opeId].ope[i] = ECOMPARISON_LARGER;
					pos++;
					if (check[pos] == '=') {
						deviceOperation[opeId].ope[i] = ECOMPARISON_LARGER_EQUAL;
						pos++;
					}
					break;
				case '=':
					deviceOperation[opeId].ope[i] =  ECOMPARISON_EQUAL;
					pos++;
					break;
				case '!':
					deviceOperation[opeId].ope[i] =  ECOMPARISON_NOT_EQUAL;
					pos++;
					if (check[pos] == '=') {
						deviceOperation[opeId].ope[i] = ECOMPARISON_NOT_EQUAL;
						pos++;
					}
					break;
				default:
					break;
			}
			deviceOperation[opeId].checkValue[i] = atoi(&check[pos]);
		}
	}
}

void settingOperation(HWND hDlg)
{
	char name[STR_LEN_MAX];
	char dev1[STR_LEN_MAX];
	char dev2[STR_LEN_MAX];
	char con1[STR_LEN_MAX];
	char con2[STR_LEN_MAX];
	char dev[STR_LEN_MAX];
	char value[STR_LEN_MAX];
	char IdDir[STR_LEN_MAX];
	char stepNum[STR_LEN_MAX];
	char stepTime[STR_LEN_MAX];
	char opeFomura[STR_LEN_MAX];
	char text[STR_LEN_MAX] = {0};
	
	GetWindowText(GetDlgItem(hDlg, ID_OPE_NAME), name, sizeof(name));
	GetWindowText(GetDlgItem(hDlg, ID_OPE_DEV1), dev1, sizeof(dev1));
	GetWindowText(GetDlgItem(hDlg, ID_OPE_DEV2), dev2, sizeof(dev2));
	GetWindowText(GetDlgItem(hDlg, ID_CONDITION1), con1, sizeof(con1));
	GetWindowText(GetDlgItem(hDlg, ID_CONDITION2), con2, sizeof(con2));
	GetWindowText(GetDlgItem(hDlg, ID_OPE_DEV), dev, sizeof(dev));
	GetWindowText(GetDlgItem(hDlg, ID_OPE_VALUE), value, sizeof(value));
	GetWindowText(GetDlgItem(hDlg, ID_OPE_DIR), IdDir, sizeof(IdDir));
	GetWindowText(GetDlgItem(hDlg, ID_STEP_NUM), stepNum, sizeof(stepNum));
	GetWindowText(GetDlgItem(hDlg, ID_STEP_TIME), stepTime, sizeof(stepTime));
	GetWindowText(GetDlgItem(hDlg, ID_FORMULA), opeFomura, sizeof(opeFomura));

	if (name[0] == NULL) strcpy_s(text, "Input the operation name.");
	if ((dev1[0] == NULL) && (dev2[0] == NULL)) strcpy_s(text, "select the condition.");
	if ((con1[0] == NULL) && (con2[0] == NULL)) strcpy_s(text, "select the condition.");
	if (dev[0] == NULL) strcpy_s(text, "select the device");

	if (text[0] == NULL) {
		strcpy_s(deviceOperation[operationPos].name, name);
		deviceOperation[operationPos].dev[0] = getDevId(dev1);
		deviceOperation[operationPos].dev[1] = getDevId(dev2);
		strcpy_s(deviceOperation[operationPos].condition[0], con1);
		strcpy_s(deviceOperation[operationPos].condition[1], con2);
		strcpy_s(deviceOperation[operationPos].opeDevName, dev);
		strcpy_s(deviceOperation[operationPos].operationFomura, opeFomura);
		strcpy_s(deviceOperation[operationPos].opeValueStr, value);
		strcpy_s(deviceOperation[operationPos].opeIdDirStr, IdDir);
		deviceOperation[operationPos].stepNum = atoi(stepNum);
		deviceOperation[operationPos].stepTime = atoi(stepTime);
		deviceOperation[operationPos].devSel[0] = SendMessage(GetDlgItem(hDlg, ID_OPE_DEV1), CB_GETCURSEL, 0, 0);
		deviceOperation[operationPos].devSel[1] = SendMessage(GetDlgItem(hDlg, ID_OPE_DEV2), CB_GETCURSEL, 0, 0);
		deviceOperation[operationPos].opeDevSel = SendMessage(GetDlgItem(hDlg, ID_OPE_DEV), CB_GETCURSEL, 0, 0);
		setCondition(operationPos);
		if (operationPos == operationNum) {
			SendMessage(GetDlgItem(hDlg, ID_OPE_SELECT), CB_ADDSTRING, 0, (LPARAM)name);
			SendMessage(GetDlgItem(hDlg, ID_OPE_SELECT), CB_SETCURSEL, operationNum , 0);
			operationNum++;
			MessageBox(NULL, TEXT(("OK")), TEXT("infomation"), MB_ICONINFORMATION);
		} else {
			SendMessage(GetDlgItem(hDlg, ID_OPE_SELECT), CB_DELETESTRING, operationPos, 0);// 既存の項目を削除
			SendMessage(GetDlgItem(hDlg, ID_OPE_SELECT), CB_INSERTSTRING, operationPos, (LPARAM)name);// 再追加
			SendMessage(GetDlgItem(hDlg, ID_OPE_SELECT), CB_SETCURSEL, operationNum , 0);
			MessageBox(NULL, TEXT(("OK")), TEXT("infomation"), MB_ICONINFORMATION);
		}
		
	} else {
		MessageBox(NULL, TEXT(text), TEXT("infomation"), MB_ICONINFORMATION);
	}
}

void readOperationSetting(HWND hDlg)
{
	char text[STR_LEN_MAX];

	SendMessage(GetDlgItem(hDlg, ID_OPE_NAME), WM_SETTEXT, 0, (LPARAM)deviceOperation[operationPos].name);
	SendMessage(GetDlgItem(hDlg, ID_CONDITION1), WM_SETTEXT, 0, (LPARAM)deviceOperation[operationPos].condition[0]);
	SendMessage(GetDlgItem(hDlg, ID_CONDITION2), WM_SETTEXT, 0, (LPARAM)deviceOperation[operationPos].condition[1]);
	SendMessage(GetDlgItem(hDlg, ID_OPE_VALUE), WM_SETTEXT, 0, (LPARAM)deviceOperation[operationPos].opeValueStr);
	SendMessage(GetDlgItem(hDlg, ID_OPE_DIR), WM_SETTEXT, 0, (LPARAM)deviceOperation[operationPos].opeIdDirStr);
	SendMessage(GetDlgItem(hDlg, ID_OPE_DEV1), CB_SETCURSEL, deviceOperation[operationPos].devSel[0], 0);
	SendMessage(GetDlgItem(hDlg, ID_OPE_DEV2), CB_SETCURSEL, deviceOperation[operationPos].devSel[1], 0);
	SendMessage(GetDlgItem(hDlg, ID_OPE_DEV), CB_SETCURSEL, deviceOperation[operationPos].opeDevSel, 0);
	if (deviceOperation[operationPos].stepNum != 0) {
		sprintf_s(text, sizeof(text), "%d", deviceOperation[operationPos].stepNum);
		SendMessage(GetDlgItem(hDlg, ID_STEP_NUM), WM_SETTEXT, 0, (LPARAM)text);
	}
	if (deviceOperation[operationPos].stepTime != 0) {
		sprintf_s(text, sizeof(text), "%d", deviceOperation[operationPos].stepTime);
		SendMessage(GetDlgItem(hDlg, ID_STEP_TIME), WM_SETTEXT, 0, (LPARAM)text);
	}
	sprintf_s(text, sizeof(text), "%s", deviceOperation[operationPos].operationFomura);
	SendMessage(GetDlgItem(hDlg, ID_FORMULA), WM_SETTEXT, 0, (LPARAM)text);	
}

void updateOperation(unsigned short opeId)
{
	deviceOperation[opeId].stepCount++;
	if (deviceOperation[opeId].stepCount >= deviceOperation[opeId].stepNum) {
		deviceOperation[opeId].timeWait = false;
		deviceOperation[opeId].stepNowValue = deviceOperation[opeId].opeValue;
		deviceOperation[opeId].tentativeTime = 0;
	} else {
		deviceOperation[opeId].stepNowValue += deviceOperation[opeId].stepValue;
		deviceOperation[opeId].tentativeTime += deviceOperation[opeId].stepTime;
	}
	deviceLogAnalyze(deviceOperation[opeId].opeDevName, deviceOperation[opeId].stepNowValue, deviceOperation[opeId].stepDir);
}

unsigned short getNearPoint()
{
	unsigned short i;
	unsigned short ret = 0;
	double pointX;
	double pointY;
	double minDiff;
	double diff;
	
	pointX = (unsigned long)convertOffsetPointX(analyzePoint[0].X);
	pointY = (unsigned long)convertOffsetPointY(analyzePoint[0].Y);
	minDiff = calcDistance(pointX ,pointY, mousePosX, mousePosY);
	for (i = 1; i < pointNum; i++) {
		if (editState == EEDIT_STATE_EDIT) {
			if ((i == analyzePoint[editPos].prePoint) || (analyzePoint[i].isConnect == true)) {
				continue;
			}
		} else if ((editState == EEDIT_STATE_SELECT) && (editType != EPOINT_TYPE_EDIT_POINT)) {
			if (analyzePoint[i].isConnect == true) {
				continue;
			}
		} else {
		}
		if (analyzePoint[i].isConnect == true) {
			pointX = (analyzePoint[i].X + analyzePoint[analyzePoint[i].prePoint].X) / 2;
			pointY = (analyzePoint[i].Y + analyzePoint[analyzePoint[i].prePoint].Y) / 2;
		} else {
			pointX = analyzePoint[i].X;
			pointY = analyzePoint[i].Y;
		}
		pointX = (unsigned long)convertOffsetPointX(pointX);
		pointY = (unsigned long)convertOffsetPointY(pointY);
		diff = calcDistance(pointX, pointY, mousePosX, mousePosY);
		if (diff < minDiff) {
			minDiff = diff;
			ret = i;
		}
	}
	return ret;
}

void normalPoint (unsigned short ePos)
{
	double mouseDistance;
	double mouseDistanceX;
	double mouseDistanceY;
	double mPosX = inConvertOffsetPointX(mousePosX);
	double mPosY = inConvertOffsetPointY(mousePosY);

	if (ePos == editPos) {
		mouseDistanceX = mPosX - analyzePoint[analyzePoint[ePos].prePoint].X;
		mouseDistanceY = mPosY - analyzePoint[analyzePoint[ePos].prePoint].Y;
		mouseDistance = sqrt((mouseDistanceX * mouseDistanceX) + (mouseDistanceY * mouseDistanceY));
		analyzePoint[ePos].distanceX = mouseDistanceX * editDistance / mouseDistance;
		analyzePoint[ePos].distanceY = mouseDistanceY * editDistance / mouseDistance;
	}
	analyzePoint[ePos].X = analyzePoint[analyzePoint[ePos].prePoint].X + analyzePoint[ePos].distanceX;
	analyzePoint[ePos].Y = analyzePoint[analyzePoint[ePos].prePoint].Y + analyzePoint[ePos].distanceY;
}

void connectPoint (unsigned short ePos)
{
	if (ePos == editPos) {
		analyzePoint[ePos].nextPoint = analyzePoint[selectPos].nextPoint;
		analyzePoint[ePos].X = analyzePoint[selectPos].X;
		analyzePoint[ePos].Y = analyzePoint[selectPos].Y;
		analyzePoint[ePos].distanceX = analyzePoint[ePos].X - analyzePoint[analyzePoint[ePos].prePoint].X;
		analyzePoint[ePos].distanceY = analyzePoint[ePos].Y - analyzePoint[analyzePoint[ePos].prePoint].Y;
	} else {
		analyzePoint[ePos].X = analyzePoint[analyzePoint[analyzePoint[ePos].nextPoint].prePoint].X;
		analyzePoint[ePos].Y = analyzePoint[analyzePoint[analyzePoint[ePos].nextPoint].prePoint].Y;
	}
}

void calcConnectDistance()
{
	double preX;
	double preY;

	for (unsigned short i = 1; i <= pointNum; i++) {
		if (analyzePoint[i].isConnect == true) {
			preX = analyzePoint[analyzePoint[i].prePoint].X;
			preY = analyzePoint[analyzePoint[i].prePoint].Y;
			analyzePoint[i].distanceX = (double)analyzePoint[i].X - preX;
			analyzePoint[i].distanceY = (double)analyzePoint[i].Y - preY;
			analyzePoint[i].distance = calcDistance(preX, preY, analyzePoint[i].X, analyzePoint[i].Y);
		}
	}
}

void pointSwitch(unsigned short swichPoint)
{
	unsigned short btmPos;
	double curDis;
	unsigned short btmNext;
	unsigned char i;
	unsigned short historyBuf[HISTORY_BUF_NUM];
	unsigned short bPos;
	unsigned short tPos;
	unsigned short num;

	curDis = bottomPosiInfo[swichPoint].dis;
	bottomPosiInfo[swichPoint].dis = topPosiInfo[swichPoint].dis;
	topPosiInfo[swichPoint].dis = curDis;

	btmPos = bottomPosiInfo[swichPoint].point;
	bottomPosiInfo[swichPoint].point = topPosiInfo[swichPoint].point;
	topPosiInfo[swichPoint].point = btmPos;

	btmNext = bottomPosiInfo[swichPoint].nextPoint;
	bottomPosiInfo[swichPoint].nextPoint = topPosiInfo[swichPoint].nextPoint;
	topPosiInfo[swichPoint].nextPoint = btmNext;

	for (i = 0; i < HISTORY_BUF_NUM; i++) {
		historyBuf[i] = topPosHistory[swichPoint].history[i];
	}
	tPos = topPosHistory[swichPoint].topPos;
	bPos = topPosHistory[swichPoint].bottomPos;
	if (tPos == bPos) {
		num = 0;
	} else {
		num = (tPos > bPos ? tPos - bPos + 1 : HISTORY_BUF_NUM - tPos + bPos + 1);
	}
	for (i = 0; i < num; i++) {
		topPosHistory[swichPoint].history[bPos] = historyBuf[tPos];
		bPos = (bPos == HISTORY_BUF_NUM - 1 ? 0 : bPos + 1);
		tPos = (tPos == 0 ? HISTORY_BUF_NUM - 1 : tPos - 1);
	}
}

unsigned short getNextTopPoint(unsigned short point)
{
	unsigned short i;
	unsigned short ret;

	ret = analyzePoint[point].nextPoint;
	if (analyzePoint[ret].isConnect == true) {
		ret = analyzePoint[analyzePoint[ret].nextPoint].prePoint;
	}
	if (checkSwitchPoint(point) == -1) {
		for (i = 0; i < branchNum; i++) {
			if (point == currentBranch[i].point) {
				ret = currentBranch[i].nextPoint;
				break;
			}
		}
	}
	return ret;
}

unsigned short getNextBottomPoint(unsigned char id)
{
	unsigned short ret = 0;
	unsigned short bottomPos = (topPosHistory[id].bottomPos == HISTORY_BUF_NUM - 1 ? 0 : topPosHistory[id].bottomPos + 1);

	if (bottomPos == topPosHistory[id].topPos) {
		ret = topPosiInfo[id].nextPoint;
	} else {
		ret = topPosHistory[id].history[bottomPos == HISTORY_BUF_NUM - 1 ? 0 : bottomPos + 1];
	}
	return ret;
}

signed short checkSwitchPoint(unsigned short point)
{
	unsigned short i;
	signed short ret = -1;

	for (i = 0; i < switchBackNum; i++) {
		if (point == positionSwitch[i].point) {
			ret = positionSwitch[i].nextPoint;
		}
	}
	if (ret != -1) {
		for (i = 0; i < branchNum; i++) {
			if (point == currentBranch[i].point) {
				ret = currentBranch[i].nextPoint;
				break;
			}
		}
	}

	//@@
	/*
	for (i = 0; i < startPartNum; i++) {
		if (point == analyzePoint[startPoint[i].point].prePoint) {
			if (isFold(startPoint[i].startPart)) {
				ret = startPoint[i].point;
			}
		}
	}
	*/
	return ret;
}

bool isTopSwitchPos(unsigned char id, unsigned short switchPos)
{
	unsigned short topSwitPoint = checkSwitchPoint(topPosiInfo[id].point);
	unsigned short tPos = topPosHistory[id].topPos;
	unsigned short bPos = topPosHistory[id].bottomPos;
	unsigned char num = (tPos > bPos ? tPos - bPos + 1 : HISTORY_BUF_NUM - tPos + bPos + 1);
	bool ret = false;

	if (topPosiInfo[id].nextPoint == topSwitPoint) {
		ret = true;
	}
	for (unsigned char i = 0; i < num; i++) {
		if(topPosHistory[id].history[tPos] == switchPos){
			ret = true;
			break;
		}
		tPos = (tPos == 0 ? HISTORY_BUF_NUM - 1 : tPos - 1);
	}
	return ret;
}

signed short getDeviceId(unsigned short point)
{
	signed short ret = -1;
	unsigned short i = 0;

	for (i = 0; i < deviceNum; i++) {
		if (devicePoint[i].point == point) {
			ret = i;
			break;
		}
	}
	return ret;
}

signed short getStartPartId(unsigned short point)
{
	signed short ret = -1;
	unsigned short i = 0;

	for (i = 0; i < startPartNum; i++) {
		if (startPoint[i].point == point) {
			ret = i;
			break;
		}
	}
	return ret;
}

signed short getBranchId(unsigned short point)
{
	signed short ret = -1;
	unsigned short i = 0;

	for (i = 0; i < branchNum; i++) {
		if (analyzeBranch[i].point == point) {
			ret = i;
			break;
		}
	}
	return ret;
}

signed short getSpeed(unsigned short dev)
{
	signed short ret = 0;

	ret = devicePoint[dev].value;
	if (devicePoint[dev].driveType == EDRIVE_TYPE_CLUCH) {
		if (ret != 0) {
			if (devicePoint[dev].driveSouceDev != -1) {
				ret = driveSouce[devicePoint[dev].driveSouceDev].value;
			}
		}
	}
	return ret;
}

void checkDisProceedNextPoint (unsigned char id)
{
	unsigned short failSafeCount = 0;
	double distance;

	distance = analyzePoint[getTopVectorPoint(id)].distance;
	while ((topPosiInfo[id].nextPoint != 0) && (distance < topPosiInfo[id].dis)) {//次のpointまでの長さをすでに超えている
		topPosiInfo[id].point = topPosiInfo[id].nextPoint;
		topPosiInfo[id].dis = topPosiInfo[id].dis - distance;
		topPosiInfo[id].nextPoint = getNextTopPoint(topPosiInfo[id].point);
		topPosHistory[id].topPos = (topPosHistory[id].topPos + 1) % HISTORY_BUF_NUM;
		topPosHistory[id].history[topPosHistory[id].topPos] = topPosiInfo[id].point;
		topPosHistory[id].distance[topPosHistory[id].topPos] = distance;
		distance = analyzePoint[getTopVectorPoint(id)].distance;
		failSafeCount++;
		if (failSafeCount > FAIL_SAFE_MAX) {
			break;
		}
	}
}

void updateTopPoint(unsigned char id, double dis)
{
	enum eHOLD_TYPE{
		 EHOLD_NONE
		,EHOLD_SENSOR
		,EHOLD_ROOLER
	};
	double distance;
	signed short switchPos;
	signed short dev;
	eHOLD_TYPE holdType = EHOLD_NONE;
	unsigned short failSafeCount = 0;

	if (topPosiInfo[id].dir == EREVERSE) {
		if (topPosiInfo[id].dis > dis) {
			topPosiInfo[id].dis -= dis;
		} else {
			switchPos = checkSwitchPoint(topPosiInfo[id].point);
			if (switchPos == -1) {
				topPosiInfo[id].nextPoint = topPosiInfo[id].point != 0 ? topPosiInfo[id].point : topPosiInfo[id].nextPoint;
				topPosiInfo[id].point = topPosiInfo[id].point != 0 ? analyzePoint[topPosiInfo[id].point].prePoint : topPosiInfo[id].point;
				topPosiInfo[id].dis = topPosiInfo[id].dis - dis + analyzePoint[topPosiInfo[id].point].distance;
			} else {
				topPosiInfo[id].dis = (-1) * (topPosiInfo[id].dis - dis);
				topPosiInfo[id].dir = EFOWARD;
				if (topPosiInfo[id].nextPoint != switchPos) {
					topPosiInfo[id].nextPoint = switchPos;
				} else {
					topPosiInfo[id].nextPoint = analyzePoint[topPosiInfo[id].point].nextPoint;
				}
				checkDisProceedNextPoint(id);
			}
		}
	} else {
		topPosiInfo[id].dis += dis;
		if (topPosiInfo[id].nextPoint != 0) {

			distance = analyzePoint[getTopVectorPoint(id)].distance;
			while ((topPosiInfo[id].nextPoint != 0) && (distance < topPosiInfo[id].dis)) {
				dev = getDeviceId(topPosiInfo[id].nextPoint);
				if (dev != -1) {
					switch (devicePoint[dev].type) {
						case EDEV_SENSOR:
							if(devicePoint[dev].value == 0) {
								holdType = EHOLD_SENSOR;
							}
							break;
						case EDEV_SINGLE_ROLLER:
						case EDEV_PAIR_ROLLER:
							if(devicePoint[dev].value == 0) {
								holdType = EHOLD_ROOLER;
							}
							break;
						default:
							break;
					}
				}
				if (holdType == EHOLD_SENSOR) {
					topPosiInfo[id].dis = distance;
					break;
				} else if (holdType == EHOLD_ROOLER) {
					topPosiInfo[id].point = topPosiInfo[id].nextPoint;
					topPosiInfo[id].dis = 0;
					topPosiInfo[id].nextPoint = getNextTopPoint(topPosiInfo[id].point);
					topPosHistory[id].topPos = (topPosHistory[id].topPos + 1) % HISTORY_BUF_NUM;
					topPosHistory[id].history[topPosHistory[id].topPos] = topPosiInfo[id].point;
					topPosHistory[id].distance[topPosHistory[id].topPos] = distance;
					break;
				} else {
					topPosiInfo[id].point = topPosiInfo[id].nextPoint;
					topPosiInfo[id].dis = topPosiInfo[id].dis - distance;
					topPosiInfo[id].nextPoint = getNextTopPoint(topPosiInfo[id].point);
					topPosHistory[id].topPos = (topPosHistory[id].topPos + 1) % HISTORY_BUF_NUM;
					topPosHistory[id].history[topPosHistory[id].topPos] = topPosiInfo[id].point;
					topPosHistory[id].distance[topPosHistory[id].topPos] = distance;
					distance = analyzePoint[getTopVectorPoint(id)].distance;
					failSafeCount++;
					if (failSafeCount > FAIL_SAFE_MAX) {
						break;
					}
				}
			}
		}
	}
}

void updateBottomPoint(unsigned char id, double dis)
{
	double diatance;
	unsigned short pos = topPosHistory[id].bottomPos;

	if (initOverLength[id] != 0) {
		if (initOverLength[id] <= dis) {
			dis = dis - initOverLength[id];
			initOverLength[id] = 0;
		} else {
			initOverLength[id] -= dis;
			return;
		}
	}

	if (bottomPosiInfo[id].dir == EREVERSE) {
		if (bottomPosiInfo[id].dis > dis) {
			bottomPosiInfo[id].dis -= dis;
		}
	} else {
		bottomPosiInfo[id].dis += dis;
		if (bottomPosiInfo[id].nextPoint != 0) {
			diatance = analyzePoint[getbottomVectorPoint(id)].distance;
			if (diatance < bottomPosiInfo[id].dis) {
				if (topPosHistory[id].bottomPos != topPosHistory[id].topPos) {
					pos = (pos + 1) % HISTORY_BUF_NUM;
					bottomPosiInfo[id].point = topPosHistory[id].history[pos];
					bottomPosiInfo[id].dis = bottomPosiInfo[id].dis - diatance;
					topPosHistory[id].bottomPos = pos;
					if (topPosHistory[id].bottomPos != topPosHistory[id].topPos) {
						pos = (pos + 1) % HISTORY_BUF_NUM;
						bottomPosiInfo[id].nextPoint = topPosHistory[id].history[pos];
					} else {
						bottomPosiInfo[id].nextPoint = topPosiInfo[id].nextPoint;
					}
				}
			}
		}
	}
}
signed short checkDu(unsigned char id, unsigned short point)
{
	unsigned short i;
	signed short ret = -1;

	if (point == startPoint[duStartPartId].point) {
		for (i = 0; i < MOTION_NUM; i++) {
			if (profileInfo[i].state == EPRIFILE_WAIT) {
				if (profileInfo[i].startPart == 0xff) {
					if (profileInfo[i].waitId = 0xff) {
						ret = i;
						break;
					}
				}
			}
		}
	}
	return ret;
}

void moveOn(unsigned char id, double moveDisTop, double moveDisBottom) 
{
	unsigned short point = topPosiInfo[id].point;
	signed short waitId;

	//駆動によって先端が進
	updateTopPoint(id, moveDisTop);
	if (point != topPosiInfo[id].point) {
		waitId = checkDu(id, topPosiInfo[id].point);
		if (waitId != -1) {
			profileInfo[waitId].state = EPRIFILE_ACTIVE;
			topPosHistory[waitId] = topPosHistory[id];
			bottomPosiInfo[waitId] = bottomPosiInfo[id];
			topPosiInfo[waitId] = topPosiInfo[id];
			motionEnd(id);
			id = (unsigned char)waitId;
			if (profileInfo[waitId].waitId != 0xff) {
				if (profileInfo[profileInfo[waitId].waitId].state == EPRIFILE_WAIT) {
					profileInfo[waitId].waitId = 0xff;
				}
			}
		}
	}
	//駆動によって後端が進む
	updateBottomPoint(id, moveDisBottom);

	//用紙長さ以上にはならないので、駆動していなくても後端は進む
	point = bottomPosiInfo[id].point;
	if (topPosiInfo[id].dir == EFOWARD) {
		if (bottomPosiInfo[id].dir == EFOWARD) {
			updateBottomPointFoward(id);
		} else {
			updateBottomPointSwitch(id);
		}
	} else {
		updateBottomPointReverse(id);
	}

	if (topPosHistory[id].bottomPos >= 1) {
		if (profileInfo[id].waitId != 0xff) {
			profileInfo[profileInfo[id].waitId].state = EPRIFILE_ACTIVE;
			profileInfo[id].waitId = 0xff;
		}
	}
}
void proceedToPoint(unsigned char id, eEDGE_POINT edge, unsigned short point)
{
	bool isFirst = true;
	unsigned short checkPos;
	unsigned char i;
	double distance;

	if (edge == EEDGE_TOP) {
		distance = analyzePoint[getTopVectorPoint(id)].distance;
		while(topPosiInfo[id].point != point) {
			topPosiInfo[id].dis = 0;
			topPosHistory[id].topPos = (topPosHistory[id].topPos + 1) % HISTORY_BUF_NUM;
			topPosiInfo[id].point = isFirst ? topPosiInfo[id].nextPoint : getNextTopPoint(topPosiInfo[id].point);
			topPosiInfo[id].nextPoint = getNextTopPoint(topPosiInfo[id].point);
			isFirst = false;
			topPosHistory[id].history[topPosHistory[id].topPos] = topPosiInfo[id].point;
			topPosHistory[id].distance[topPosHistory[id].topPos] = distance;
			distance = analyzePoint[getTopVectorPoint(id)].distance;
			if (topPosiInfo[id].point == 0) {
				break;
			}
		}
		topPosiInfo[id].nextPoint = getNextTopPoint(topPosiInfo[id].point);
	} else {
		checkPos = topPosHistory[id].topPos;
		for (i = 0; i < HISTORY_BUF_NUM; i++) {
			if (topPosHistory[id].history[checkPos] == point) {
				bottomPosiInfo[id].point = point;
				bottomPosiInfo[id].nextPoint = analyzePoint[point].nextPoint;
				bottomPosiInfo[id].dis = 0;
				while(topPosHistory[id].history[topPosHistory[id].bottomPos] != point) {	
					topPosHistory[id].bottomPos = (topPosHistory[id].bottomPos + 1) % HISTORY_BUF_NUM;
					if ((topPosHistory[id].bottomPos == topPosHistory[id].topPos) || (topPosHistory[id].bottomPos == 0)) {
						break;
					}
				}
				break;
			}
			checkPos = (checkPos != 0) ? checkPos - 1 : HISTORY_BUF_NUM - 1;
			if (checkPos == topPosHistory[id].bottomPos) {
				break;
			}
		}
		
	}
}

void updateBottomPointFoward(unsigned char id)
{
	double length;
	unsigned short point;
	unsigned short pos = topPosHistory[id].topPos;
	unsigned short i;

	point = topPosiInfo[id].point;
	length = topPosiInfo[id].dis;
	while (length < profileInfo[id].length) {
		length += topPosHistory[id].distance[pos];
		pos = (pos != 0) ? pos - 1 : HISTORY_BUF_NUM - 1;
		point = topPosHistory[id].history[pos];
		if (pos == topPosHistory[id].bottomPos) {
			if (length < profileInfo[id].length) {
				return;
			} else {
				if (bottomPosiInfo[id].dis > (length - profileInfo[id].length)) {
					return;
				}
			}
				
		}
	}
	if (bottomPosiInfo[id].point != point) {
		bottomPosiInfo[id].point = point;
		while(topPosHistory[id].history[topPosHistory[id].bottomPos] != point) {
			topPosHistory[id].bottomPos = (topPosHistory[id].bottomPos + 1) % HISTORY_BUF_NUM;
		}
		if (topPosHistory[id].bottomPos != topPosHistory[id].topPos) {
			bottomPosiInfo[id].nextPoint = topPosHistory[id].history[(topPosHistory[id].bottomPos + 1) % HISTORY_BUF_NUM];
		} else {
			bottomPosiInfo[id].nextPoint = topPosiInfo[id].nextPoint;
		}
	}
	if (length >= profileInfo[id].length) {
		bottomPosiInfo[id].dis = length - profileInfo[id].length;
	}

	if ((bottomPosiInfo[id].nextPoint == 0) && (topPosiInfo[id].nextPoint == 0)) {
		motionEnd(id);
	}

	//折ポイントを過ぎたら短いほうは消す。
	for (i = 0; i < startPartNum; i++) {
		if (point == startPoint[i].point) {
			if (isFold(startPoint[i].startPart)) {
				if (profileInfo[id].prfType == EPROF_SPLIT) {
					motionEnd(id);
				}
			}
		}
	}
}

void updateBottomPointReverse(unsigned char id)
{
	double length;
	unsigned short point;
	unsigned short next;
	unsigned short pos = topPosHistory[id].topPos;

	point = topPosiInfo[id].point;
	if (topPosHistory[id].topPos == topPosHistory[id].bottomPos) {
		bottomPosiInfo[id].dis = topPosiInfo[id].dis + profileInfo[id].length;
		bottomPosiInfo[id].point = topPosiInfo[id].point;
		bottomPosiInfo[id].nextPoint = topPosiInfo[id].nextPoint;
	} else {
		pos = topPosHistory[id].topPos;
		pos = (pos != 0) ? pos - 1 : HISTORY_BUF_NUM - 1;
		point = topPosHistory[id].history[pos];
		length = analyzePoint[point].distance - topPosiInfo[id].dis;
		if (length > profileInfo[id].length) {
			topPosHistory[id].bottomPos = topPosHistory[id].topPos;
			bottomPosiInfo[id].dis = topPosiInfo[id].dis + profileInfo[id].length;
			bottomPosiInfo[id].point = topPosiInfo[id].point;
			bottomPosiInfo[id].nextPoint = topPosiInfo[id].nextPoint;
		} else if (pos == topPosHistory[id].bottomPos) {
			bottomPosiInfo[id].dis = profileInfo[id].length - length;
			bottomPosiInfo[id].point = point;
		} else {
			if (analyzePoint[point].nextPoint == 0) {
				bottomPosiInfo[id].dis = profileInfo[id].length - length;
				bottomPosiInfo[id].point = point;
				bottomPosiInfo[id].nextPoint = topPosiInfo[id].nextPoint;
			} else {
				while (length < profileInfo[id].length) {
					point = topPosHistory[id].history[pos];
					pos = (pos != 0) ? pos - 1 : HISTORY_BUF_NUM - 1;
					next = topPosHistory[id].history[pos];
					if (length + analyzePoint[next].distance < profileInfo[id].length) {
						length += analyzePoint[next].distance;
						if (pos == topPosHistory[id].bottomPos) {
							bottomPosiInfo[id].dis = profileInfo[id].length - length;
							break;
						}
					} else {
						bottomPosiInfo[id].dis = profileInfo[id].length - length;
						bottomPosiInfo[id].point = point;
						bottomPosiInfo[id].nextPoint = next;
						break;
					}
				}
			}
		}
	}
	
	while (topPosHistory[id].history[topPosHistory[id].bottomPos] != bottomPosiInfo[id].point) {
		topPosHistory[id].bottomPos = (topPosHistory[id].bottomPos + 1) % HISTORY_BUF_NUM;
	}
	if ((bottomPosiInfo[id].nextPoint == 0) && (topPosiInfo[id].nextPoint == 0)) {
		motionEnd(id);
	}
	if ((bottomPosiInfo[id].point == 0) && (bottomPosiInfo[id].nextPoint == 1) && (bottomPosiInfo[id].dis < 0)) {
		motionEnd(id);
	}
}
void updateBottomPointSwitch(unsigned char id)
{
	double length;
	unsigned short point;
	unsigned short next;
	unsigned short pos = topPosHistory[id].topPos;

	point = topPosiInfo[id].point;
	length = topPosiInfo[id].dis;

	if (length > profileInfo[id].length) {
		bottomPosiInfo[id].dis = topPosiInfo[id].dis - profileInfo[id].length;
		bottomPosiInfo[id].point = point;
		bottomPosiInfo[id].nextPoint = topPosiInfo[id].nextPoint;
		bottomPosiInfo[id].dir = EFOWARD;
	} else {
		while (checkSwitchPoint(point) == (-1)) {
			length += analyzePoint[point].distance;
			if (length > profileInfo[id].length) {
				bottomPosiInfo[id].dis = length - profileInfo[id].length;
				bottomPosiInfo[id].dir = EFOWARD;
				bottomPosiInfo[id].nextPoint = point;
				break;
			} else {
				pos = (pos != 0) ? pos - 1 : HISTORY_BUF_NUM - 1;
				point = topPosHistory[id].history[pos];
			}
		}
		if(bottomPosiInfo[id].dir == EREVERSE){
			if (point == bottomPosiInfo[id].point) {
				bottomPosiInfo[id].dis = profileInfo[id].length - length;
			} else {
				if (length + analyzePoint[topPosHistory[id].history[(pos != 0) ? pos - 1 : HISTORY_BUF_NUM - 1]].distance > profileInfo[id].length) {
					bottomPosiInfo[id].dis = profileInfo[id].length - length;
					bottomPosiInfo[id].nextPoint = topPosHistory[id].history[(pos != 0) ? pos - 1 : HISTORY_BUF_NUM - 1];
					bottomPosiInfo[id].point = point;
				} else {
					pos = (pos != 0) ? pos - 1 : HISTORY_BUF_NUM - 1;
					point = topPosHistory[id].history[pos];
					length += analyzePoint[point].distance;
					if (pos == topPosHistory[id].bottomPos) {
						bottomPosiInfo[id].dis = profileInfo[id].length - length;
						bottomPosiInfo[id].point = point;
					} else {
						while (length < profileInfo[id].length) {
							point = topPosHistory[id].history[pos];
							pos = (pos != 0) ? pos - 1 : HISTORY_BUF_NUM - 1;
							next = topPosHistory[id].history[pos];
							if (length + analyzePoint[next].distance < profileInfo[id].length) {
								length += analyzePoint[next].distance;
								if (pos == topPosHistory[id].bottomPos) {
									bottomPosiInfo[id].dis = profileInfo[id].length - length;
									break;
								}
							} else {
								bottomPosiInfo[id].dis = profileInfo[id].length - length;
								bottomPosiInfo[id].point = point;
								bottomPosiInfo[id].nextPoint = next;
								break;
							}
						}
					}
				}
			}
		}
	}


	while (topPosHistory[id].history[topPosHistory[id].bottomPos] != bottomPosiInfo[id].point) {
		topPosHistory[id].bottomPos = (topPosHistory[id].bottomPos + 1) % HISTORY_BUF_NUM;
	}
	if ((bottomPosiInfo[id].nextPoint == 0) && (topPosiInfo[id].nextPoint == 0)) {
		if (topPosiInfo[id].point == bottomPosiInfo[id].point) {
			motionEnd(id);
		}
	}
	if ((bottomPosiInfo[id].point == 0) && (bottomPosiInfo[id].nextPoint == 1) && (bottomPosiInfo[id].dis < 0)) {
		motionEnd(id);
	}
}

void checkDirChange(unsigned short devId, signed short val, eDIRECTION dir)
{
	unsigned short i;
	unsigned short point;
	unsigned short pos;
	signed short dev;
	unsigned short failSafeCount;

	for (i = 0; i < MOTION_NUM; i++) {
		if (profileInfo[i].state == EPRIFILE_ACTIVE) {
			if (topPosHistory[i].topPos == topPosHistory[i].bottomPos) {
				continue;
			} else {
				pos = (topPosHistory[i].bottomPos + 1) % HISTORY_BUF_NUM;
				point = topPosHistory[i].history[pos];
				failSafeCount = 0;
				while (failSafeCount < FAIL_SAFE_MAX) {
					failSafeCount++;
					dev = getDeviceId(point);
					if (dev != (-1)) {
						if ((devicePoint[dev].type == EDEV_SINGLE_ROLLER) || (devicePoint[dev].type == EDEV_PAIR_ROLLER)) {
							if (point == devicePoint[devId].point) {
								if (dir == EREVERSE) {
									if ((topPosiInfo[i].dir == EFOWARD) && (bottomPosiInfo[i].dir == EFOWARD)) {
										pointSwitch(i);
										topPosiInfo[i].dir = EREVERSE;
										bottomPosiInfo[i].dir = EREVERSE;
									} 	
									break;
								} else {
									if ((topPosiInfo[i].dir == EREVERSE) && (bottomPosiInfo[i].dir == EREVERSE)) {
										topPosiInfo[i].dir = EFOWARD;
										bottomPosiInfo[i].dir = EFOWARD;
									}
									if ((topPosiInfo[i].dir == EFOWARD) && (bottomPosiInfo[i].dir == EREVERSE)) {
										pointSwitch(i);
									}
									break;
								}
							} else {
								break;
							}
						}
					}
					if (topPosHistory[i].topPos == topPosHistory[i].bottomPos) {
						break;
					} else {
						pos = (pos + 1) % HISTORY_BUF_NUM;
						point = topPosHistory[i].history[pos];
					}
				}
				continue;
			}
		}
	}
}

void updateMotion(unsigned long time)
{
	unsigned short pos;
	unsigned short point;
	bool isTopConfirm = false;
	double moveDisTop;
	double moveDisBootom;
	double speed = 0;
	int id;
	int i;

	for (id = 0; id < MOTION_NUM; id++) {
		if (profileInfo[id].state == EPRIFILE_ACTIVE) {
			pos = topPosHistory[id].topPos;
			isTopConfirm = false;
			moveDisTop = 0;
			moveDisBootom = 0;
			speed = 0;
			while (pos != topPosHistory[id].bottomPos) {
				point = topPosHistory[id].history[pos];
				for (i = 0; i < deviceNum; i++) {
					if (point == devicePoint[i].point) {
						if ((devicePoint[i].type == EDEV_SINGLE_ROLLER) || (devicePoint[i].type == EDEV_PAIR_ROLLER)) {
							if (devicePoint[i].value != 0) {
								speed = getSpeed(i);
								if (isTopConfirm == false) {
									moveDisTop = speed * time / 1000;
								}
							}
							if (isTopConfirm) {
								moveDisBootom = speed * time / 1000;
							}
							isTopConfirm = true;
						}
					}
				}
				pos = (pos != 0) ? pos - 1 : HISTORY_BUF_NUM - 1;
			}
			if ((moveDisTop > 0) || (moveDisBootom > 0)) {
				moveOn(id, moveDisTop, moveDisBootom);
			}
		}
	}
}

void motionReset()
{
	unsigned char i;

	pauseFlag = false;
	firstTime = true;
	crrunetTime = 0;
	difTime = 0;
	if (fileP != NULL) {
		fclose(fileP);
	}
	for (i = 0; i < MOTION_NUM; i++) {
		if (profileInfo[i].state != EPRIFILE_IDLE) {
			motionEnd(i);
		}
	}
	for (i = 0; i < deviceNum; i++) {
		devicePoint[i].dir = ENONE_DIRECTION;
		devicePoint[i].value = 0;
	}
}

void pageCancel()
{
	for (unsigned char i = 0; i < MOTION_NUM; i++) {
		motionEnd(i);
	}
}

unsigned short checkOperationTime(unsigned long time)
{
	unsigned short ret = 0xffff;
	unsigned char i;
	unsigned long mimTime = time;

	for (i = 0; i < operationNum; i++) {
		if (deviceOperation[i].timeWait) {
			if (deviceOperation[i].tentativeTime <= mimTime) {
				mimTime = deviceOperation[i].tentativeTime;
				ret = i;
			}
		}
	}
	return ret; 
}

bool updateVisualAnalyzeTime(unsigned long time)
{
	unsigned long curTime = time;
	unsigned long waitTime;
	bool ret = true;

	if(curTime > crrunetTime)  {
		operationId = checkOperationTime(curTime);
		if (operationId != 0xffff) {
			difTime = deviceOperation[operationId].tentativeTime - crrunetTime;
		} else {
			difTime = curTime - crrunetTime;
		}
	} else {
		return ret;
	}
	rememberTime = curTime;
	waitTime = difTime;

	//早送り
	if ((fastForwardTime > curTime) || (firstTime == true)) {
		waitTime = 1;
	}
	firstTime = false;

#if 1
	if (logReadState == ELOG_READ_NORMAL) {
		if (operationId != 0xffff) {
			PostMessage(vaHwnd, WM_USER, WM_USER_REMTIME, operationId);
		} else {
			updateMotion(difTime);
		}
	} else if (logReadState == ELOG_READ_ACTIVE) {
		if (operationId != 0xffff) {
			curTime = deviceOperation[operationId].tentativeTime;
			logReadState = ELOG_READ_WAIT_TIMER;
			SetTimer(vaHwnd, (operationId << 8) | TIMER_ID_4, waitTime, NULL);
			ret = false;
		} else if (difTime > 0) {
			logReadState = ELOG_READ_WAIT_TIMER;
			SetTimer(vaHwnd, TIMER_ID_2, waitTime, NULL);
			ret = false;
		} else {
		}
	} else if (logReadState == ELOG_READ_PAUSE) {
		if (operationId != 0xffff) {
			curTime = deviceOperation[operationId].tentativeTime;
		}
		ret = false;
	}
#else
	if (operationId != 0xffff) {
		PostMessage(vaHwnd, WM_USER, WM_USER_REMTIME, operationId);
	} else {
		updateMotion(difTime);
	}
#endif
	crrunetTime = curTime;
	return ret;
}

void fileReadStart()
{
	errno_t result;

	if (fileName[0] == NULL) {
		return;
	}
	result = fopen_s(&fileP, fileName, "rb");
	logReadState = ELOG_READ_ACTIVE;
	fileReadExecute();

}
void fileReadExecute()
{
	extern void createLog (unsigned char buf);
	char c;

	while (fread(&c, sizeof(unsigned char), 1, fileP) == 1) {
		createLog(c);
		if (logReadState != ELOG_READ_ACTIVE) {
			break;
		}
	}
}

void motionEnd(unsigned char id)
{
	unsigned char i;

	isActive[id] = false;
	bottomPosiInfo[id].point = 0;
	topPosiInfo[id].point = 0;
	bottomPosiInfo[id].dis = 0;
	topPosiInfo[id].dis = 0;
	profileInfo[id].state = EPRIFILE_IDLE;
	for (i = 0; i < HISTORY_BUF_NUM; i++) {
		topPosHistory[id].history[i] = 0;
		topPosHistory[id].distance[i] = 0;
	}
}

bool isLastMotion(unsigned short id)
{
	bool ret = false;
	unsigned short pos = topPosHistory[id].topPos;
	unsigned short point = topPosHistory[id].history[pos];
	signed short dev;

	if (analyzePoint[point].nextPoint == 0) {
		if (profileInfo[id].state == EPRIFILE_ACTIVE) {
			ret = true;
			while (pos != topPosHistory[id].bottomPos) {
				point = topPosHistory[id].history[pos];
				dev = getDeviceId(point);
				if (dev != (-1)) {
					if ((devicePoint[dev].type == EDEV_SINGLE_ROLLER) || (devicePoint[dev].type == EDEV_PAIR_ROLLER)) {
						ret = false;
						break;
					}
				}
				pos = (pos != 0) ? pos - 1 : HISTORY_BUF_NUM - 1;
			}
		}
	}
	return ret;
}

unsigned char getWaitNum(unsigned char id)
{
	unsigned char i;
	unsigned char ret = 0;

	if (profileInfo[id].state == EPRIFILE_ACTIVE) {
		ret = 0;
	} else {
		while (ret < MOTION_NUM) {
			for (i = 0; i < MOTION_NUM; i++) {
				if (profileInfo[i].state != EPRIFILE_IDLE) {
					if (profileInfo[i].waitId == id) {
						id = i;
						ret++;
						break;
					}
				}
			}
			if (i == MOTION_NUM) {
				break;
			}
		}
	}
	return ret;
}

void updateOffset()
{
	moveOffSetX += movingOffSetX;
	moveOffSetY += movingOffSetY;
	movingOffSetX = 0;
	movingOffSetY = 0;
}

void movePen(HDC hdc, double x, double y)
{
	x = convertOffsetPointX(x);
	y = convertOffsetPointY(y);
	MoveToEx(hdc, (int)x, (int)y, NULL);
}

void writeLine(HDC hdc, double x, double y)
{
	x = convertOffsetPointX(x);
	y = convertOffsetPointY(y);
	LineTo(hdc, (int)x, (int)y);
}
void writePoint(HDC hdc, double x, double y)
{
	x = convertOffsetPointX(x);
	y = convertOffsetPointY(y);
	Ellipse(hdc, (int)x - 1, (int)y - 1, (int)x + 1, (int)y + 1);
}
void writeText(HDC hdc, double x, double y, char *str, long length)
{
	x = convertOffsetPointX(x);
	y = convertOffsetPointY(y);
	TextOut(hdc, (int)x, (int)y, str, length);
}

double convertOffsetPointX(double x)
{
	x *= dispScale;
	x += moveOffSetX + movingOffSetX;
	return x;
}
double convertOffsetPointY(double y)
{
	y *= dispScale;
	y += moveOffSetY + movingOffSetY;
	return y;
}

double inConvertOffsetPointX(double x)
{
	x -= (moveOffSetX + movingOffSetX);
	x /= dispScale;
	return x;
}
double inConvertOffsetPointY(double y)
{
	y -= (moveOffSetY + movingOffSetY);
	y /= dispScale;
	return y;
}

double calcDistance(double x1, double y1, double x2, double y2)
{
	return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

char* readVaIniDataNum(char* section, char* key)
{
	GetPrivateProfileString(section, key, "0", readIniStr, sizeof(readIniStr), iniFilePath);
	return readIniStr;
}

char* readVaIniData(char* section, char* keyName, int num)
{
	char key[256];

	sprintf_s(key, sizeof(key), "%s_%04d", keyName, num);
	GetPrivateProfileString(section, key, "0", readIniStr, sizeof(readIniStr), iniFilePath);
	return readIniStr;
}
void writeVaIniDataNum(char* section, char* key, int value)
{
	char str[256];

	sprintf_s(str, sizeof(str), "%d", value);
	WritePrivateProfileString(section, key, str, iniFilePath);
}

void writeVaIniDataInt(char* section, char* keyName, int num, int value)
{
	char key[256];
	char str[256];

	sprintf_s(key, sizeof(key), "%s_%04d", keyName, num);
	sprintf_s(str, sizeof(str), "%d",value);
	WritePrivateProfileString(section, key, str, iniFilePath);
}

void writeVaIniDataFloat(char* section, char* keyName, int num, double value)
{
	char key[256];
	char str[256];

	sprintf_s(key, sizeof(key), "%s_%04d", keyName, num);
	sprintf_s(str, sizeof(str), "%f",value);
	WritePrivateProfileString(section, key, str, iniFilePath);
}

void writeVaIniDataStr(char* section, char* keyName, int num, char* name)
{
	char key[256];

	sprintf_s(key, sizeof(key), "%s_%04d", keyName, num);
	WritePrivateProfileString(section, key, name, iniFilePath);
}

bool getVisualLogDisp()
{
	return isDispLog;
}

eLOG_READ_STATE getVaState()
{
	return logReadState;
}

void setVisualAnalyzerMode(char* syslog_file)
{
	strcpy_s(fileName, syslog_file);
	logReadState = ELOG_READ_STOP;
}


