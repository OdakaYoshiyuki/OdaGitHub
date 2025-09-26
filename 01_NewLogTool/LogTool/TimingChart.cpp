#include "stdafx.h"
#include "TimingChart.h"
#include "Resource.h"
#include "CommDlg.h"
#include <windows.h>
#include <Commctrl.h>
#include <stdio.h>
#include <shellapi.h>
#include <string.h>
#include "LogTool.h"
#include "saveTcdFile.h"

#define TCM_WINDOW_WIDTH 800
#define TCM_WINDOW_HEIGTH 500
#define TCM_DATA_BUF_NUM 500
#define TCM_DEVICE_NUM 20
#define TCM_TOOLBAR_HEIGTH 28
#define TCM_BORDER_POS_Y 50
#define TCM_INTERVAL 24
#define TCM_BUTTON 25
#define TCM_WIDTH_BUF_NUM 50
#define TCM_REVERSE_VALUE 7

extern void sendText(char* buffer);
extern void logFileOpen();
extern void logFileClose();
extern void createLog (unsigned char buf);

extern HINSTANCE hInst;
extern char logToolSettingPath[_T_MAX_PATH];
extern char szIniFilePublic[_T_MAX_PATH];		// Ini�t�@�C���p�X
extern char syslog_file[_T_MAX_PATH];

static LRESULT CALLBACK tcmWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK tcmSettingProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

static void deviceListDisp(HWND hDlg);
static void deviceReflect(HWND hDlg);
static void setTcmToolSettingPath(HWND hDlg);
static void writeTcmToolIni(HWND hDlg);
static void readTcmToolIni(HWND hDlg);
static void tcmButtonAction(HWND hDlg, unsigned char index);
static void deviceAddtion();
static void updateTcmWindow(HWND hWnd, HDC hdc);
static void dispDevTiming(HWND hWnd, HDC hdc);
static void dispTcmAxisScale(HWND hWnd, HDC hdc);
static void tcmDispTimeWidth(HWND hWnd, HDC hdc);
static void tcmCalculateNearestValue();
static void drawLeftArea(HWND hWnd, HDC hdc);
static signed long xPos(signed long timeCount);
static unsigned long yPos(unsigned short id, unsigned short onOff);
static signed long inverseXPos(unsigned long xPos);
static unsigned long pixelsMoved(unsigned long timeSec);
static void tcmDragScroll();
static void tcmMouseMove();
static void tcmLButtonDown();
static void tcmLButtonUp();
static bool isOnBorder();
static bool isOnTcm();
static bool isScenario(int id);
static float tcmConvertCountsToTime(signed long count);
static unsigned char selectFontSize(int areaSize, int strleng);
static unsigned long getWritePosX(unsigned long posX, unsigned long lastX, int strleng, unsigned char fontSize);
static unsigned long getWritePosY(unsigned long posY, unsigned char fontSize);
static void tcmRefresh();
static void tdmAllReset();
static void outPutTcdFile();

static HWND setButton[TCM_DEVICE_NUM];
static HFONT hNumFont;
static HFONT hNameFont;
static HFONT hTimeFont;
static HFONT hValueFont2;
static HFONT hValueFont[13];
static unsigned long timeScale = 20;
static signed long offSetTimePos = 0;
static unsigned long minCount = 0xffffffff;
static unsigned long maxCount = 0;
static unsigned long currentTimCount = 0;
static unsigned short deviceSetNum = TCM_DEVICE_NUM;
static unsigned long X_0 = 0;
static signed long moveOffSet = 0;			//�J�[�\���ړ��ɔ�����ʈړ��̃s�N�Z����
static signed long scaleDiff = 0;
static signed long analyzOffSet1 = 0;
static signed long analyzOffSet2 = 0;
static signed long scaleDiffCount = 0;	//�g��k���� �ύX�O��X_0�̃J�E���g�l
static int borderPosX;
static int mousePosX;
static int mousePosY;
static unsigned long cursorDownX = 0;		//���N���b�N�����Ƃ���X���W
static eTCM_CLICK_STATE tcmClickState = ETCM_MOUSE_UP;
static char tcmSettingPath[_T_MAX_PATH];
static char tcmSettingName[_T_MAX_PATH];
static unsigned char devButtonId;
static unsigned char selectPosId = 0xff;
static char filter1[30];
static char filter2[30];
static unsigned char tcmAnalyzState = ETCM_ANALYZ_NORMAL;
static bool  isWidthButtonOn = false;

static FILE *dummyFileP;
static FILE *tcmFileP;
bool isTcmRefresh;

struct TCM_LOG_BUF {
	unsigned long timeCount;
	unsigned long value;
	unsigned short id;
	unsigned short onOff;
};
struct TCM_LOG_DEV {
	signed long lastX;
	signed long lastY;
	unsigned long lastValue;
	bool isExist;
};
struct TCM_SETTING {
	unsigned long devId;
	unsigned short sceParam;
	bool isDispValue;
	bool isHexadecimal;
};

unsigned short tcmBufPos = 0;
bool tcmBufOverFlg;
TCM_LOG_BUF tcmLogBuf[TCM_DATA_BUF_NUM] = {0};
TCM_LOG_DEV tcmLogCood[TCM_DEVICE_NUM] = {0};

TCM_SETTING tcmSetting[TCM_DEVICE_NUM];
static char deviceName[TCM_DEVICE_NUM][256];

static unsigned long timeWidth1[TCM_WIDTH_BUF_NUM];
static unsigned long timeWidth2[TCM_WIDTH_BUF_NUM];
static unsigned short timeWidth1DevId[TCM_WIDTH_BUF_NUM];
static unsigned short timeWidth2DevId[TCM_WIDTH_BUF_NUM];
static float timeWidth3[TCM_WIDTH_BUF_NUM];
static unsigned long yPosTimeWidth[TCM_WIDTH_BUF_NUM];
static unsigned long posTimeWidth = 0;
static bool overTimeWidthFlg = false;

TBBUTTON tcmButton[] = {
	{0, ECOMMAND_TCM_SCALEUP,	TBSTATE_ENABLED, TBSTYLE_BUTTON,	0,0,0},
	{1, ECOMMAND_TCM_SCALEDOWN,	TBSTATE_ENABLED, TBSTYLE_BUTTON,	0,0,0},
	{2, ECOMMAND_TCM_TCMCOMMAND,TBSTATE_ENABLED, TBSTYLE_BUTTON,	0,0,0},
	{3, ECOMMAND_TCM_TIMEWIDTH,	TBSTATE_ENABLED, TBSTYLE_BUTTON,	0,0,0},
	{0, 0,						TBSTATE_ENABLED, TBSTYLE_SEP,		0,0,0},
	{4, ECOMMAND_TCM_RESET,		TBSTATE_ENABLED, TBSTYLE_BUTTON,	0,0,0},
	{5, ECOMMAND_TCM_ALLRESET,	TBSTATE_ENABLED, TBSTYLE_BUTTON,	0,0,0},
};

void setTcmToolSettingPath(HWND hWnd)
{
	GetPrivateProfileString(_T("PATH"), _T("TCM_FILE"), logToolSettingPath, tcmSettingPath, sizeof(tcmSettingPath), logToolSettingPath);
}

void readTcmToolIni(HWND hWnd)
{
	int i;
	char *p;

	GetPrivateProfileString(_T("PARAM"), _T("DEVICE_NUM"), "0", buf, sizeof(buf), tcmSettingPath);
	deviceSetNum = atoi(buf);
	GetPrivateProfileString(_T("PARAM"), _T("BORDER_POS"), "100", buf, sizeof(buf), tcmSettingPath);
	borderPosX = atoi(buf);

	for (i = 0; i < deviceSetNum; i++) {
		sprintf_s(buf, "NAME_%02d", i);
		GetPrivateProfileString(_T("DEVICE_NAME"), buf, "", deviceName[i], sizeof(deviceName[i]), tcmSettingPath);
		sprintf_s(buf, "ID_%02d", i);
		GetPrivateProfileString(_T("DEVICE_ID"), buf, "", buf, sizeof(buf), tcmSettingPath);
		tcmSetting[i].devId = strtol(buf, &p, 16);
		sprintf_s(buf, "DISP_VALUE_%02d", i);
		GetPrivateProfileString(_T("DISP_VALUE"), buf, "OFF", buf, sizeof(buf), tcmSettingPath);
		tcmSetting[i].isDispValue = (strcmp(buf, "ON") == 0);
		sprintf_s(buf, "HEXADECIMAL_%02d", i);
		GetPrivateProfileString(_T("HEXADECIMAL"), buf, "OFF", buf, sizeof(buf), tcmSettingPath);
		tcmSetting[i].isHexadecimal = (strcmp(buf, "ON") == 0);
		sprintf_s(buf, "PAGE_ID_%02d", i);
		GetPrivateProfileString(_T("PAGE_ID"), buf, "0", buf, sizeof(buf), tcmSettingPath);
		tcmSetting[i].sceParam = atoi(buf);
	}
	for (i = deviceSetNum; i < TCM_DEVICE_NUM; i++) {
		sprintf_s(deviceName[i], "");
		tcmSetting[i].devId = 0;
	}
	if (selectPosId >= (deviceSetNum - 1)) {
		selectPosId = 0xff;
	}
	GetPrivateProfileString(_T("SETTING"), "FILE_NAME", _T(""), tcmSettingName, sizeof(tcmSettingName), tcmSettingPath);
	sprintf_s(buf, "TimgChart - %s",tcmSettingName);
	SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)buf);
}
void writeTcmToolIni(HWND hWnd)
{
	int i;
	char str[10];
	sprintf_s(buf, "%0d", deviceSetNum);
	WritePrivateProfileString(_T("PARAM"), _T("DEVICE_NUM"), buf, tcmSettingPath);
	sprintf_s(buf, "%0d", borderPosX);
	WritePrivateProfileString(_T("PARAM"), _T("BORDER_POS"), buf, tcmSettingPath);

	for (i = 0; i < deviceSetNum; i++) {
		sprintf_s(buf, "NAME_%02d", i);
		WritePrivateProfileString(_T("DEVICE_NAME"), buf, deviceName[i], tcmSettingPath);
		sprintf_s(buf, "ID_%02d", i);
		sprintf_s(str, "%04x", tcmSetting[i].devId);
		WritePrivateProfileString(_T("DEVICE_ID"), buf, str, tcmSettingPath);
		sprintf_s(buf, "DISP_VALUE_%02d", i);
		WritePrivateProfileString(_T("DISP_VALUE"), buf, tcmSetting[i].isDispValue ? _T("ON") :_T("OFF"), tcmSettingPath);
		sprintf_s(buf, "HEXADECIMAL_%02d", i);
		WritePrivateProfileString(_T("HEXADECIMAL"), buf, tcmSetting[i].isHexadecimal ? _T("ON") : _T("OFF"), tcmSettingPath);
		sprintf_s(buf, "PAGE_ID_%02d", i);
		sprintf_s(str, "%02d", tcmSetting[i].sceParam);
		WritePrivateProfileString(_T("PAGE_ID"), buf, str, tcmSettingPath);
	}
	WritePrivateProfileString(_T("SETTING"), "FILE_NAME", tcmSettingName, tcmSettingPath);
	sprintf_s(buf, "TimgChart - %s",tcmSettingName);
	SendMessage(hWnd, WM_SETTEXT, 0, (LPARAM)buf);
}

bool setSettingIniFileName(HWND hDlg, char *filePath, char *fileName)
{
	int lengh;
	char name[_T_MAX_PATH];

	OPENFILENAME ofn;
	// �\���̂ɏ����Z�b�g
	ZeroMemory(&ofn, sizeof(ofn));			// �ŏ��Ƀ[���N���A���Ă���
	ofn.lStructSize = sizeof(ofn);			// �\���̂̃T�C�Y
	ofn.hwndOwner = hDlg;                   // �R�����_�C�A���O�̐e�E�B���h�E�n���h��
	ofn.lpstrFilter = "�ݒ�(*.ini)\0*.ini\0All files(*.*)\0*.*\0\0";

	// �t�@�C���̎��
	ofn.lpstrFile = buf;					// �I�����ꂽ�t�@�C����(�t���p�X)���󂯎��ϐ��̃A�h���X
	ofn.lpstrFileTitle = name;				// �I�����ꂽ�t�@�C�������󂯎��ϐ��̃A�h���X
	ofn.nMaxFile = sizeof(buf);				// lpstrFile�Ɏw�肵���ϐ��̃T�C�Y
	ofn.nMaxFileTitle = sizeof(name);		// lpstrFileTitle�Ɏw�肵���ϐ��̃T�C�Y
	ofn.Flags = OFN_OVERWRITEPROMPT;        // �t���O�w��
	ofn.lpstrTitle = _T("�t�@�C���ݒ�");
	ofn.lpstrDefExt = _T("ini");            // �f�t�H���g�̃t�@�C���̎��
											// ���O��t���ĕۑ��R�����_�C�A���O���쐬

	buf[0] = 0; //�ǂݍ��ޑO�ɃN���A���Ă����Ȃ��ƁA�L�����Z�������ꍇ�ɃG�[���ɂȂ邽��
	if(!GetOpenFileName(&ofn)) {
		if (buf[0] != 0) {
			MessageBox(NULL, TEXT("�t�@�C��������������\��������܂�"),TEXT("�G���["), MB_ICONWARNING);
		}
		return false;
	}
	lengh = strlen(buf);
	if (lengh > _T_MAX_PATH) {
		MessageBox(NULL, TEXT("�t�@�C�������������܂�"),TEXT("�G���["), MB_ICONWARNING);
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

void tcmButtonAction(HWND hWnd, unsigned char index)
{
	devButtonId = index;
	selectPosId = index;
	DialogBox(NULL, MAKEINTRESOURCE(IDD_TCM_SETTING), hWnd, tcmSettingProc);
}

void createTcmFont()
{
	hNumFont = CreateFont(15, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH, "Meiryo UI" );
	hNameFont = CreateFont(15, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH, NULL );
	hTimeFont = CreateFont(13, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH, NULL );
	hValueFont2 = CreateFont(12, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH, "�l�r �S�V�b�N" );
	for (int i = 0; i < 13; i++) {
		hValueFont[i] = CreateFont(i, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH, "�l�r �S�V�b�N" );
	}
}

void createTcmSetButtonWindow(HWND hWnd)
{
	int i;
	for (i = 0; i < TCM_DEVICE_NUM; i++) {
		setButton[i] = CreateWindow(TEXT("BUTTON"), TEXT(""),WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 0 , TCM_TOOLBAR_HEIGTH + TCM_BORDER_POS_Y + i * (TCM_INTERVAL), TCM_BUTTON ,TCM_INTERVAL, hWnd, NULL, NULL, NULL);
		SendMessage(setButton[i], WM_SETFONT, (WPARAM)hNumFont, MAKELPARAM(TRUE, 0));
		sprintf_s(buf, "%d", i);
		SendMessage(setButton[i], WM_SETTEXT, 0, (LPARAM)buf);
	}
}

void deviceAddtion()
{
	int i;
	
	for (i = TCM_DEVICE_NUM - 2; i >= devButtonId; i--) {
		tcmSetting[i + 1] = tcmSetting[i];
		sprintf_s(deviceName[i + 1], "%s", deviceName[i]);
	}
	sprintf_s(deviceName[devButtonId], "");
	if (deviceSetNum < TCM_DEVICE_NUM - 1) {
		deviceSetNum++;
	}
}

void deviceDelete()
{
	int i;

	for (i = devButtonId; i < TCM_DEVICE_NUM - 1; i++) {
		tcmSetting[i] = tcmSetting[i + 1];
		sprintf_s(deviceName[i], "%s", deviceName[i + 1]);
	}
	sprintf_s(deviceName[TCM_DEVICE_NUM - 1], "");
	if (selectPosId >= (deviceSetNum - 1)) {
		selectPosId = 0xff;
	}
	if (deviceSetNum > 0) {
		deviceSetNum--;
	}
}

void deviceMoveUp(HWND hDlg, HWND numWh)
{
	TCM_SETTING dummyDevInfo;

	if (devButtonId > 0) {
		dummyDevInfo = tcmSetting[devButtonId - 1];
		tcmSetting[devButtonId - 1] = tcmSetting[devButtonId];
		tcmSetting[devButtonId] = dummyDevInfo;
		strcpy_s(buf, deviceName[devButtonId - 1]);
		strcpy_s(deviceName[devButtonId - 1], deviceName[devButtonId]);
		strcpy_s(deviceName[devButtonId], buf);
		devButtonId--;
		selectPosId--;
		sprintf_s(buf, "%d", devButtonId);
		if (numWh != NULL) {
			SendMessage(numWh, WM_SETTEXT, 0, (LPARAM)buf);
		}
	}
}

void deviceMoveDown(HWND hDlg, HWND numWh)
{
	TCM_SETTING dummyDevInfo;

	if (devButtonId < TCM_DEVICE_NUM - 1) {
		dummyDevInfo = tcmSetting[devButtonId +1];
		tcmSetting[devButtonId + 1] = tcmSetting[devButtonId];
		tcmSetting[devButtonId] = dummyDevInfo;
		strcpy_s(buf, deviceName[devButtonId + 1]);
		strcpy_s(deviceName[devButtonId + 1], deviceName[devButtonId]);
		strcpy_s(deviceName[devButtonId], buf);
		devButtonId++;
		selectPosId++;
		sprintf_s(buf, "%d", devButtonId);
		SendMessage(numWh, WM_SETTEXT, 0, (LPARAM)buf);
		if (devButtonId > deviceSetNum) {
			deviceSetNum = devButtonId;
		}
	}
}
void timingChartMain(HWND hWnd)
{
	HWND hwnd;
	WNDCLASS winc;

	winc.style = CS_HREDRAW | CS_VREDRAW;
	winc.lpfnWndProc = tcmWndProc;
	winc.cbClsExtra = winc.cbWndExtra = 0;
	winc.hInstance = hInst;
	winc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winc.hCursor = LoadCursor(NULL, IDC_ARROW);
	winc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	winc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU3);
	winc.lpszClassName = TEXT("timgChart");

	UnregisterClass(winc.lpszClassName, winc.hInstance);
	if (!RegisterClass(&winc)) return;

	hwnd = CreateWindow(
		TEXT("timgChart"),
		TEXT("TimgChart"),						// �^�C�g���o�[�ɕ\�����镶����
		WS_OVERLAPPEDWINDOW | WS_EX_COMPOSITED,	// �E�B���h�E�̎��
		50,										// �E�B���h�E��\������ʒu�iX���W�j
		50,										// �E�B���h�E��\������ʒu�iY���W�j
		TCM_WINDOW_WIDTH,						// �E�B���h�E�̕�
		TCM_WINDOW_HEIGTH,						// �E�B���h�E�̍���
		NULL,									// �e�E�B���h�E�̃E�B���h�E�n���h��
		NULL,									// ���j���[�n���h��
		hInst,									// �C���X�^���X�n���h��
		NULL									// ���̑��̍쐬�f�[�^
	);

	if (hwnd == NULL) {
		return;
	}
	ShowWindow(hwnd, SW_SHOW);
}

INT_PTR CALLBACK tcmSettingProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND numWh;

	switch (message) {
		case WM_INITDIALOG:
			sprintf_s(buf, "%d", devButtonId);
			numWh = CreateWindow(TEXT("STATIC") , TEXT(buf), WS_CHILD | WS_VISIBLE | SS_CENTER |SS_CENTERIMAGE | WS_BORDER,10, 10, TCM_BUTTON, TCM_BUTTON, hDlg, NULL, NULL, NULL);
			SendMessage(numWh, WM_SETFONT, (WPARAM)hNumFont, MAKELPARAM(TRUE, 0));

			SendMessage(GetDlgItem(hDlg, ID_COMB2), CB_ADDSTRING , 0 , (LPARAM)"ON or OFF");
			SendMessage(GetDlgItem(hDlg, ID_COMB2), CB_ADDSTRING , 0 , (LPARAM)"�ݒ�l");
			SendMessage(GetDlgItem(hDlg, ID_FILTER_TEXT_1), WM_SETTEXT, 0, (LPARAM)filter1);
			SendMessage(GetDlgItem(hDlg, ID_FILTER_TEXT_2), WM_SETTEXT, 0, (LPARAM)filter2);
			sprintf_s(buf, "%d", tcmSetting[devButtonId].sceParam);
			SendMessage(GetDlgItem(hDlg, ID_SCE_PARAM), WM_SETTEXT, 0, (LPARAM)buf);
			SendMessage(GetDlgItem(hDlg, ID_COMB2), CB_SETCURSEL , (tcmSetting[devButtonId].isDispValue ? 1 : 0) , 0);
			SendMessage(GetDlgItem(hDlg, ID_CHECK), BM_SETCHECK, (tcmSetting[devButtonId].isHexadecimal == 1 ? BST_CHECKED : BST_UNCHECKED), 0);
			deviceListDisp(hDlg);
			if (deviceName[devButtonId][0] != 0) {
				sprintf_s(buf, "%05x",tcmSetting[devButtonId].devId);
				GetPrivateProfileString(_T("DEVICE_NAME"), buf, "0", buf, sizeof(buf), szIniFilePublic);
				SendMessage(GetDlgItem(hDlg, ID_COMB1), WM_SETTEXT, 0, (LPARAM)buf);
			}
			break;
		case WM_COMMAND:
			switch (wParam) {
				case IDCANCEL:
					EndDialog(hDlg, LOWORD(wParam));
					if (selectPosId >= (deviceSetNum - 1)) {
						selectPosId = 0xff;
					}
					break;
				case IDOK:
					if (deviceSetNum < devButtonId + 1) {
						deviceSetNum = devButtonId + 1;
					}
					deviceReflect(hDlg);
					EndDialog(hDlg, LOWORD(wParam));
					tcmRefresh();
					break;
				case ID_FILTER:
					deviceListDisp(hDlg);
					SendMessage(GetDlgItem(hDlg, ID_COMB1), CB_SHOWDROPDOWN, TRUE, 0);
					break;
				case ID_ADDTION:
					if (selectPosId != 0xff) {
						deviceAddtion();
						tcmRefresh();
					}
					break;
				case ID_DELETE:
					if (selectPosId != 0xff) {
						deviceDelete();
						tcmRefresh();
						SendMessage(GetDlgItem(hDlg, ID_COMB2), CB_SETCURSEL , (tcmSetting[devButtonId].isDispValue ? 1 : 0) , 0);
						sprintf_s(buf, "%05x",tcmSetting[devButtonId].devId);
						GetPrivateProfileString(_T("DEVICE_NAME"), buf, "0", buf, sizeof(buf), szIniFilePublic);
						SendMessage(GetDlgItem(hDlg, ID_COMB1), WM_SETTEXT, 0, (LPARAM)buf);
					}
					break;
				case ID_MOVE_UP:
					deviceMoveUp(hDlg, numWh);
					tcmRefresh();
					break;
				case ID_MOVE_DOWN:
					deviceMoveDown(hDlg, numWh);
					tcmRefresh();
					break;
				default:
					break;
			}
			break;
		case WM_DESTROY:
			DestroyWindow(numWh);
			break;
		default:
			break;
	}
	return (INT_PTR)FALSE;
}
void deviceListDisp(HWND hDlg)
{
	FILE *devFile;
	char* devStr;
	char* context; //strtok_s�̓����Ŏg�p
	bool devList = false;


	GetWindowText(GetDlgItem(hDlg, ID_FILTER_TEXT_1), filter1, sizeof(filter1));
	GetWindowText(GetDlgItem(hDlg, ID_FILTER_TEXT_2), filter2, sizeof(filter2));

	SendMessage(GetDlgItem(hDlg, ID_COMB1), CB_RESETCONTENT, 0, 0);
	fopen_s(&devFile, szIniFilePublic, "r");
	while (fgets(buf, 256, devFile) != NULL) {
		if (strcmp(buf, "[DEVICE_NAME]\n") == 0) {
			devList = true;
			continue;
		}
		if (devList == true) {
			devStr = strtok_s(buf, "=", &context);
			if (devStr != NULL) {
				devStr = strtok_s(NULL, "=", &context);
				if (devStr != NULL) {
					if ((strstr(devStr,filter1) != NULL) && (strstr(devStr,filter2) != NULL)) {
						devStr[strlen(devStr) - 1] = NULL;
						SendMessage(GetDlgItem(hDlg, ID_COMB1), CB_ADDSTRING , 0 , (LPARAM)devStr);
					}
				}
			}
		}
	}
	fclose(devFile);
}

void deviceReflect(HWND hDlg)
{
	FILE *devFile;
	char* context; //strtok_s�̓����Ŏg�p
	char devStr[256];
	char* devNum;
	char* devName;
	char* p;
	int param;
	LRESULT selNum;
	
	GetWindowText(GetDlgItem(hDlg, ID_COMB1), devStr, sizeof(devStr));
		
	fopen_s(&devFile, szIniFilePublic, "r");
	while (fgets(buf, 256, devFile) != NULL) {
		if (strstr(buf, devStr) != NULL) {
			devNum = strtok_s(buf, "=\r\n", &context);
			if (devNum != NULL) {
				devName = strtok_s(NULL, "=r\n", &context);
				if (devName != NULL) {
					if (strstr(devStr, devName) != NULL) {
						break;
					}
				}
			}
		}
	}
	fclose(devFile);
	
	tcmSetting[devButtonId].devId = strtol(devNum, &p, 16);
	if (isScenario(tcmSetting[devButtonId].devId)){
		GetWindowText(GetDlgItem(hDlg, ID_SCE_PARAM), buf, sizeof(buf));
		param = atoi(buf);
		tcmSetting[devButtonId].sceParam = param;
	}

	selNum = SendMessage(GetDlgItem(hDlg, ID_COMB2), CB_GETCURSEL , 0 , 0);
	if (selNum) {
		tcmSetting[devButtonId].isDispValue = true;
		if (isScenario(tcmSetting[devButtonId].devId)){
			sprintf_s(deviceName[devButtonId], "%s[%02d] �ݒ�l", devStr, param);
		} else {
			sprintf_s(deviceName[devButtonId], "%s �ݒ�l", devStr);
		}
	} else {
		tcmSetting[devButtonId].isDispValue = false;
		sprintf_s(deviceName[devButtonId], devStr);
	}

	if (SendMessage(GetDlgItem(hDlg, ID_CHECK), BM_GETCHECK, 0, 0) == BST_CHECKED) {
		tcmSetting[devButtonId].isHexadecimal = 1;
	} else {
		tcmSetting[devButtonId].isHexadecimal = 0;
	}
}

LRESULT CALLBACK tcmWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	static HDC hmdc;
	static HBITMAP hBitmap;
	static HWND hwndToolbar;
	static HBITMAP hToolBitmap;
	int i;

	switch (message) {
		case WM_CREATE:
			SetTimer(hWnd, TCM_TIMER_ID, 100, NULL);
			hdc = GetDC(hWnd);							//�f�o�C�X�R���e�L�X�g�̎擾
			hmdc = CreateCompatibleDC(hdc);				//�E�B���h�E�̃f�o�C�X�R���e�L�X�g�Ɋ֘A�t����ꂽ������DC���쐬
			hBitmap = CreateCompatibleBitmap(hdc, 2000, 1800);// �E�B���h�E�̃f�o�C�X�R���e�L�X�g�ƌ݊��̂���r�b�g�}�b�v���쐬
			SelectObject(hmdc, hBitmap);				// ������DC�Ńr�b�g�}�b�v��I��
			ReleaseDC(hWnd, hdc);						// �f�o�C�X�R���e�L�X�g�̉��	
			InitCommonControls();
			hToolBitmap = LoadBitmap(GetModuleHandle(0),MAKEINTRESOURCE(IDR_TOOLBAR2));
			hwndToolbar = CreateToolbarEx( hWnd, WS_CHILD | WS_VISIBLE, 0, ECOMMAND_NUM,
				NULL, (UINT_PTR)hToolBitmap,
				tcmButton, ECOMMAND_NUM , 0, 0, 0, 0, sizeof(TBBUTTON)
			);
			DragAcceptFiles(hWnd, TRUE);
			setTcmToolSettingPath(hWnd);
			readTcmToolIni(hWnd);
			createTcmFont();
			createTcmSetButtonWindow(hWnd);
			break;
		case WM_TIMER:
			if (wParam == TCM_TIMER_ID) {
				InvalidateRect(hWnd, NULL, FALSE);
			}
			break;
		case WM_LBUTTONDOWN:
			tcmLButtonDown();
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case WM_LBUTTONUP:								//���N���b�N�𗣂����Ƃ�
			tcmLButtonUp();
			break;
		case WM_MOUSEMOVE:	//�}�E�X�𓮂������Ƃ�
			mousePosX = (signed short)LOWORD(lParam);
			mousePosY = (signed short)HIWORD(lParam);
			tcmMouseMove();
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case WM_PAINT:
			updateTcmWindow(hWnd, hmdc);
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case ECOMMAND_TCM_SCALEUP:		//�g��{�^��
					scaleDiffCount = inverseXPos(X_0);
					if ((timeScale > 1) && (timeScale < 30)) {
						timeScale -= 1;
					} else if (timeScale >= 30) {
						timeScale -= 10;
					} else {
						timeScale = 1;
					}
					scaleDiff -= (signed long)xPos(scaleDiffCount) - (X_0);
					break;
				case ECOMMAND_TCM_SCALEDOWN:	//�k���{�^��
					scaleDiffCount = inverseXPos(X_0);
					if (timeScale < 30){
						timeScale += 1;
					} else {
						timeScale += 10;
					}
					scaleDiff -= (signed long)xPos(scaleDiffCount) - (X_0);
					if (offSetTimePos - moveOffSet - scaleDiff< 0) {
						scaleDiff = offSetTimePos - moveOffSet;
					}
					break;
				case ECOMMAND_TCM_TCMCOMMAND:
					sendText("tcm");
					break;
				case ECOMMAND_TCM_TIMEWIDTH:
					if (isWidthButtonOn == false) {
						isWidthButtonOn = true;
						tcmAnalyzState = ETCM_ANALYZ_WIDTH;
						SendMessage(hwndToolbar, TB_SETSTATE, ECOMMAND_TCM_TIMEWIDTH, MAKELPARAM(TBSTATE_ENABLED | TBSTATE_PRESSED, 0));
					} else {
						isWidthButtonOn = false;
						tcmAnalyzState = ETCM_ANALYZ_NORMAL;
						SendMessage(hwndToolbar, TB_SETSTATE, ECOMMAND_TCM_TIMEWIDTH, MAKELPARAM(TBSTATE_ENABLED, 0));
					}
					break;
				case ECOMMAND_TCM_RESET:
					posTimeWidth = 0;
					overTimeWidthFlg = false;
					break;
				case ECOMMAND_TCM_ALLRESET:
					tdmAllReset();
					break;
				case OTHER_NAME_SAVE:
					if (setSettingIniFileName(hWnd,tcmSettingPath, tcmSettingName)) {
						WritePrivateProfileString(_T("PATH"), _T("TCM_FILE"), tcmSettingPath, logToolSettingPath);
						writeTcmToolIni(hWnd);
					}
					break;
				case OVERWRITE_SAVE:
					writeTcmToolIni(hWnd);
					break;
				case ID_FILE_READ:
					if (setSettingIniFileName(hWnd,tcmSettingPath, tcmSettingName)) {
						WritePrivateProfileString(_T("PATH"), _T("TCM_FILE"), tcmSettingPath, logToolSettingPath);
						readTcmToolIni(hWnd);
						tcmRefresh();
					}
					break;
				case ID_ADDTION:
					deviceAddtion();
					tcmRefresh();
					break;
				case ID_DELETE:
					deviceDelete();
					tcmRefresh();
					break;
				case ID_OPEN_STC:
					outPutTcdFile();
					break;
				case ID_MOVE_UP:
					deviceMoveUp(NULL, NULL);
					tcmRefresh();
					break;
				case ID_MOVE_DOWN:
					deviceMoveDown(NULL, NULL);
					tcmRefresh();
					break;
				default:
					break;
			}
			for (i = 0; i < TCM_DEVICE_NUM; i++) {
				if ((lParam == (LPARAM)setButton[i]) && (setButton[i] != NULL)) {
					LONG lStyle = GetWindowLong(setButton[i], GWL_STYLE); // �E�B���h�E�̃X�^�C�����擾
					if ((lStyle & BS_PUSHBUTTON) == BS_PUSHBUTTON) {
						tcmButtonAction(hWnd, i);
					}
				}
			}
			break;
		case WM_DESTROY:
			DeleteDC(hmdc);								// ������DC�̍폜
			DeleteObject(hBitmap);						// �r�b�g�}�b�v �I�u�W�F�N�g�̍폜
			DeleteObject(hNumFont);
			DeleteObject(hNameFont);
			DeleteObject(hTimeFont);
			DeleteObject(hValueFont2);
			for (int i = 0; i < 13; i++) {
				DeleteObject(hValueFont[i]);
			}
			for (i = 0; i < TCM_DEVICE_NUM; i++) {
				DestroyWindow(setButton[i]);
				setButton[i] = NULL;
			}
		default:
			break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void tcmLogInfo(unsigned long timeCount, unsigned short id, unsigned short onOff, unsigned long value)
{
	bool isTimeUpdate = false;
	int i;

	if (tcmFileP == NULL) {
		sprintf_s(buf, "%s%s",szGlPath,"tcm.txt");
		fopen_s(&tcmFileP, buf, "w");//���O�t�@�C��
	} 
	currentTimCount = timeCount;
	if (id != 0x7ff0) {
		isTimeUpdate = true;
	}
	
	for (i = 0; i < deviceSetNum; i++) {
		if (tcmSetting[i].devId == id) {
			if (isScenario(id)) {
				int param = value / TCM_PAGEID_DIGIT;
				if (tcmSetting[i].sceParam == param) {
					tcmLogBuf[tcmBufPos].value = value % TCM_PAGEID_DIGIT;
					tcmLogBuf[tcmBufPos].id = i;
					tcmLogBuf[tcmBufPos].timeCount = timeCount;
					tcmLogBuf[tcmBufPos].onOff = onOff;
					tcmBufPos++;
				}
			} else {
				tcmLogBuf[tcmBufPos].id = i;
				tcmLogBuf[tcmBufPos].timeCount = timeCount;
				tcmLogBuf[tcmBufPos].onOff = onOff;
				tcmLogBuf[tcmBufPos].value = value;
				tcmBufPos++;	
			}
			if (tcmBufPos >= TCM_DATA_BUF_NUM) {
				tcmBufPos = 0;
				tcmBufOverFlg = true;
			}
		}
	}

	if (isTimeUpdate) {
		if ((timeCount / timeScale) + scaleDiff >= (600)) {
			offSetTimePos = (timeCount / timeScale) + scaleDiff - (600);
		}
		if (timeCount < minCount) {
			minCount = timeCount;
		} 
		if (timeCount > maxCount) {
			maxCount = timeCount;
		}
	}

	if ((tcmFileP != NULL) && (id != 0x7ff0) && (isTcmRefresh == false)) {
		char tcmBuf[256];
		sprintf_s(tcmBuf, sizeof(tcmBuf), "%d,%d,%d,%d\n", timeCount, id, onOff, value);
		fprintf(tcmFileP, tcmBuf);
	}
}

void updateTcmWindow(HWND hWnd, HDC hdc)
{
	PAINTSTRUCT ps;
	HBRUSH hBrush;
	static WINDOWINFO windowInfo;
	int winWidth;
	int winHeight;
	HDC hdcW = BeginPaint(hWnd, &ps);

	// window���A�����擾
	GetWindowInfo(hWnd, &windowInfo);
	winWidth = windowInfo.rcWindow.right - windowInfo.rcWindow.left;
	winHeight = windowInfo.rcWindow.bottom - windowInfo.rcWindow.top;

	//LTGRAY�u���V�̍쐬&�I��
	HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0xff, 0xff, 0xff));
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
	SelectObject(hdc, hPen);
	hBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
	SelectObject(hdc, hBrush);

	//DC����U�����ɂ���
	Rectangle(hdc, 0, 0, winWidth, winHeight);
	
	//�����̈��������DC�ɕ`��
	drawLeftArea(hWnd, hdc);
	// ������DC����摜��]��
	BitBlt(hdcW, 0, TCM_TOOLBAR_HEIGTH-1, borderPosX, winHeight, hdc, 0, 0, SRCCOPY);

	//DC����U�����ɂ���
	SelectObject(hdc, hBrush);
	Rectangle(hdc, 0, 0, winWidth, winHeight);
	//���̕`��
	dispTcmAxisScale(hWnd, hdc);

	//�^�C�~���O�`���[�g��ʂ�������DC�ɕ`��
	dispDevTiming(hWnd, hdc);

	//���J�[�\���̕`��
	tcmDispTimeWidth(hWnd, hdc);

	// ������DC����摜��]��
	BitBlt(hdcW, borderPosX, TCM_TOOLBAR_HEIGTH-1, winWidth, winHeight, hdc, 0, 0, SRCCOPY);

	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);

	//�c�[���o�[�̉�����
	hPen = CreatePen(PS_SOLID, 1, RGB(0xF0, 0xF0, 0xF0));
	hOldPen = (HPEN)SelectObject(hdcW, hPen);
	hBrush = (HBRUSH)(CreateSolidBrush(RGB(0xf0,0xF0,0xF0)));
	SelectObject(hdcW, hBrush);
	Rectangle(hdcW, TCM_WINDOW_WIDTH - 30, 1, winWidth, TCM_TOOLBAR_HEIGTH);
	DeleteObject(hBrush);

	SelectObject(hdcW, hOldPen);
	DeleteObject(hPen);
	EndPaint(hWnd, &ps);

}

//�f�o�C�XOnOff���l�ω��^�C�~���O�`��
void dispDevTiming(HWND hWnd, HDC hdc)
{
	unsigned long i;
	HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0x00, 0x00, 0x00));
	HPEN hPen2 = CreatePen(PS_SOLID, 1, RGB(0x10, 0x50, 0xff));
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
	unsigned long num;
	unsigned long startPos;
	unsigned long pos;
	unsigned short dev = 0;
	unsigned long devTime;
	unsigned short length;
	unsigned char fontSize;
	unsigned long wPosX;
	unsigned long wPosY;

	num = (tcmBufOverFlg == true ? TCM_DATA_BUF_NUM : tcmBufPos);
	startPos = (tcmBufOverFlg == true ? tcmBufPos : 0);
	for (i = 0; i < deviceSetNum; i++) {
		tcmLogCood[i].isExist = false;
		tcmLogCood[i].lastX = xPos(0);
		tcmLogCood[i].lastY = yPos((unsigned char)i, 0);
	}
	SelectObject(hdc, hValueFont2);

	for (i = 0; i < num; i++) {
		pos = (startPos + i) % TCM_DATA_BUF_NUM;		
		dev = tcmLogBuf[pos].id;
		devTime = tcmLogBuf[pos].timeCount;
		if (tcmLogCood[dev].isExist == true) {
			SelectObject(hdc, hPen);
		} else {
			SelectObject(hdc, hPen2);
		}
		if (tcmSetting[dev].isDispValue == true) {
			if (tcmLogCood[dev].isExist == false) {
				MoveToEx(hdc, tcmLogCood[dev].lastX, yPos(dev, 0), NULL);
				LineTo(hdc, xPos(devTime), yPos(dev, 0));
				MoveToEx(hdc,xPos(devTime), yPos(dev, TCM_REVERSE_VALUE), NULL);
				LineTo(hdc, xPos(devTime), yPos(dev, 1));
			} else {
				MoveToEx(hdc, tcmLogCood[dev].lastX, yPos(dev, 1), NULL);
				LineTo(hdc, xPos(devTime), yPos(dev, 1));
				LineTo(hdc, xPos(devTime), yPos(dev, TCM_REVERSE_VALUE));
				LineTo(hdc, tcmLogCood[dev].lastX, yPos(dev, TCM_REVERSE_VALUE));
				if (tcmSetting[dev].isHexadecimal == true) {
					length = sprintf_s(buf, sizeof(buf), "%04x", tcmLogCood[dev].lastValue);
				} else {
					length = sprintf_s(buf, sizeof(buf), "%d", tcmLogCood[dev].lastValue);
				}
				//���l�̕`��
				fontSize = selectFontSize(xPos(devTime) - tcmLogCood[dev].lastX, length);
				if (fontSize != 0) {
					SelectObject(hdc, hValueFont[fontSize]);
					wPosX = getWritePosX(xPos(devTime), tcmLogCood[dev].lastX, length, fontSize);
					wPosY = getWritePosY(yPos(dev, 0), fontSize);
					TextOut(hdc, wPosX, wPosY, buf, length);
				}
			}
			tcmLogCood[dev].lastX = xPos(devTime);
			tcmLogCood[dev].lastY = yPos(dev, 1);
			tcmLogCood[dev].isExist = true;
			tcmLogCood[dev].lastValue = tcmLogBuf[pos].value;
		} else {
			MoveToEx(hdc, tcmLogCood[dev].lastX, tcmLogCood[dev].lastY, NULL);
			LineTo(hdc, xPos(devTime), tcmLogCood[dev].lastY);
			LineTo(hdc, xPos(devTime), yPos(dev, tcmLogBuf[pos].onOff));
			tcmLogCood[dev].lastX = xPos(devTime);
			tcmLogCood[dev].lastY = yPos(dev, tcmLogBuf[pos].onOff);
			tcmLogCood[dev].isExist = true;
		}
	}

	//���݂̃|�C���g�܂Ő�����
	for (dev = 0; dev < deviceSetNum; dev++) {
		if (tcmLogCood[dev].isExist == true) {
			SelectObject(hdc, hPen);
		} else {
			SelectObject(hdc, hPen2);
		}
		if (tcmSetting[dev].isDispValue == true) {//�ݒ�l�̏ꍇ
			if (tcmLogCood[dev].isExist == false) {
				MoveToEx(hdc, tcmLogCood[dev].lastX, yPos(dev, 0), NULL);
				LineTo(hdc,  xPos(currentTimCount), yPos(dev, 0));
			} else {							
				MoveToEx(hdc, tcmLogCood[dev].lastX, yPos(dev, 1), NULL);
				LineTo(hdc,  xPos(currentTimCount), yPos(dev, 1));
				LineTo(hdc,  xPos(currentTimCount), yPos(dev, TCM_REVERSE_VALUE));
				LineTo(hdc,  tcmLogCood[dev].lastX, yPos(dev, TCM_REVERSE_VALUE));
				if (tcmSetting[dev].isHexadecimal == true) {
					length = sprintf_s(buf, sizeof(buf), "%04x", tcmLogCood[dev].lastValue);
				} else {
					length = sprintf_s(buf, sizeof(buf), "%d", tcmLogCood[dev].lastValue);
				}
				//���l�̕`��
				fontSize = selectFontSize(xPos(currentTimCount) - tcmLogCood[dev].lastX, length);
				if (fontSize != 0) {
					SelectObject(hdc, hValueFont[fontSize]);
					wPosX = getWritePosX(xPos(currentTimCount), tcmLogCood[dev].lastX, length, fontSize);
					wPosY = getWritePosY(yPos(dev, 0), fontSize);
					TextOut(hdc, wPosX, wPosY, buf, length);
				}
			}
		} else {//ON OFF�̏ꍇ
			MoveToEx(hdc, tcmLogCood[dev].lastX, tcmLogCood[dev].lastY, NULL);
			LineTo(hdc,  xPos(currentTimCount), tcmLogCood[dev].lastY);
		}
	}

	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);
	DeleteObject(hPen2);
}

// ���̈�̕`��
void drawLeftArea(HWND hWnd, HDC hdc)
{
	int i;
	HPEN hPen1 = CreatePen(PS_SOLID, 1, RGB(0xC0, 0xC0, 0xC0));
	HPEN hPen2 = CreatePen(PS_SOLID, 1, RGB(0x00, 0x00, 0x00));
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen1);
	HBRUSH hBrush;

	//�I���ʒu�̂ݐF��ς���
	hBrush = (HBRUSH)(CreateSolidBrush(RGB(0xF0,0xF0,0xF0)));
	SelectObject(hdc, hBrush);
	if (selectPosId != 0xff) {
		Rectangle(hdc, TCM_BUTTON - 1, TCM_BORDER_POS_Y + TCM_INTERVAL * selectPosId, borderPosX + 1, TCM_BORDER_POS_Y + TCM_INTERVAL * (selectPosId+1) + 1 );
	}
	//����
	for (i = 0; i <TCM_DEVICE_NUM; i++) {
		MoveToEx(hdc, 2, TCM_BORDER_POS_Y + TCM_INTERVAL * (i + 1), NULL);
		LineTo(hdc, borderPosX, TCM_BORDER_POS_Y + TCM_INTERVAL * (i + 1));
	}
	(HPEN)SelectObject(hdc, hPen2);
	//��ԏ�̉���
	MoveToEx(hdc,0,TCM_BORDER_POS_Y, NULL);
	LineTo(hdc, borderPosX, TCM_BORDER_POS_Y);

	SelectObject(hdc, hOldPen);
	DeleteObject(hPen1);
	DeleteObject(hPen2);
	DeleteObject(hBrush);
	
	SetBkMode(hdc, TRANSPARENT);
	SelectObject(hdc, hNameFont);
	for (i = 0; i <= deviceSetNum; i++) {
		TextOut(hdc, TCM_BUTTON + 2, TCM_BORDER_POS_Y + TCM_INTERVAL * i + 5, deviceName[i], strlen(deviceName[i]));
	}
}

//�I���ʒu�ύX
void changeSelectPos()
{
	int i;
	int curPosX = mousePosX - TCM_TOOLBAR_HEIGTH;
	int curPosY = mousePosY - TCM_TOOLBAR_HEIGTH;

	selectPosId = 0xff;
	if ((curPosX > TCM_BUTTON) && (curPosX < borderPosX)) {
		for (i = 0; i < deviceSetNum; i++) {
			if ((curPosY > TCM_BORDER_POS_Y + TCM_INTERVAL * i) && (curPosY < TCM_BORDER_POS_Y + TCM_INTERVAL * (i + 1))) {
				selectPosId = i;
				devButtonId = i;
			}
		}
	}	
}

//�����̖ڐ��̕\��
void dispTcmAxisScale(HWND hWnd, HDC hdc)
{
	// window���A�����擾
	static WINDOWINFO windowInfo;
	int winWidth;
	int winHeight;
	int i;
	HPEN hPen1 = CreatePen(PS_SOLID, 1, RGB(0xC0, 0xC0, 0xC0));
	HPEN hPen2 = CreatePen(PS_SOLID, 1, RGB(0x00, 0x00, 0x00));
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen1);

	GetWindowInfo(hWnd, &windowInfo);
	winWidth = windowInfo.rcWindow.right - windowInfo.rcWindow.left;
	winHeight = windowInfo.rcWindow.bottom - windowInfo.rcWindow.top;

	for (i = 0; i <deviceSetNum; i++) {
		MoveToEx(hdc, 2, TCM_BORDER_POS_Y + TCM_INTERVAL * (i + 1), NULL);
		LineTo(hdc, winWidth, TCM_BORDER_POS_Y + TCM_INTERVAL * (i + 1));
	}
	(HPEN)SelectObject(hdc, hPen2);
	MoveToEx(hdc,0,TCM_BORDER_POS_Y, NULL);
	LineTo(hdc, winWidth, TCM_BORDER_POS_Y);

	//�c��
	MoveToEx(hdc, 0, TCM_BORDER_POS_Y - 10, NULL);
	LineTo(hdc, 0, winHeight);
	MoveToEx(hdc, 1, TCM_BORDER_POS_Y - 10, NULL);
	LineTo(hdc, 1, winHeight);


	//����(���Ԏ��j
	unsigned short length;
	char str[10];
	signed long LinePos;
	static unsigned short xAxisScale = 5;	//���Ԏ��̐ݒ�
	int lineNum;

	SelectObject(hdc, hTimeFont);
	xAxisScale = ((unsigned short)(timeScale - 1) / 50 + 1) * 1;	//�ꃁ�����̎���
	lineNum = (signed long)(winWidth) / (pixelsMoved(xAxisScale));	//���̐�
	for (i = 0; i <= lineNum; i++) {
		LinePos =  (i * xAxisScale) * 500 / timeScale;	//���̍��W
		if (LinePos >= 0){
			MoveToEx(hdc, LinePos, TCM_BORDER_POS_Y, NULL);
			LineTo(hdc, LinePos, TCM_BORDER_POS_Y - 5);
			if ( ((i % 10) == 0) && (i != 0)) {
				length = sprintf_s(str, sizeof(str), "%ds", xAxisScale * i);
				LineTo(hdc, LinePos, TCM_BORDER_POS_Y - 10);
				if (lineNum >= 15) {
					TextOut(hdc, LinePos - 5, TCM_BORDER_POS_Y - 25, str, length);
				}
			}
		}
	}

	//����15�ȉ��ɂȂ�����100msec�P�ʂɂ���
	if (lineNum < 15) {
		for (i = 0;i < (lineNum + 1) * 10 ;i++) {
			LinePos = (100 * i / 2 / timeScale);
			MoveToEx(hdc, LinePos, TCM_BORDER_POS_Y, NULL);
			LineTo(hdc, LinePos, TCM_BORDER_POS_Y - 5);
			if ( ((i % 10) == 0) && (i != 0)) {
				LineTo(hdc, LinePos, TCM_BORDER_POS_Y - 10);
				length = sprintf_s(str, sizeof(str), "%dms", 10*i);
				TextOut(hdc, LinePos - 5, TCM_BORDER_POS_Y - 25, str, length);
			}
		}
	}

	SelectObject(hdc, hOldPen);
	DeleteObject(hPen1);
	DeleteObject(hPen2);
}

//���J�[�\���{�^���̏���
void tcmDispTimeWidth(HWND hWnd, HDC hdc)
{
	unsigned long i;
	HPEN hPen = CreatePen(PS_SOLID, 1, RGB(100, 100, 100));
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
	float resultTime;
	unsigned short length;
	char str[10];
	unsigned long textNum = (overTimeWidthFlg == true ? TCM_WIDTH_BUF_NUM : posTimeWidth);
	SelectObject(hdc, hValueFont[12]);

	if (tcmAnalyzState == ETCM_ANALYZ_WIDTH) {
		MoveToEx(hdc, xPos(timeWidth1[posTimeWidth]), TCM_WINDOW_HEIGTH, NULL);
		LineTo(hdc, xPos(timeWidth1[posTimeWidth]), 20);
	} else if (tcmAnalyzState == ETCM_ANALYZ_WIDTH_LINE) {
		MoveToEx(hdc, xPos(timeWidth1[posTimeWidth]), TCM_WINDOW_HEIGTH, NULL);
		LineTo(hdc, xPos(timeWidth1[posTimeWidth]), 20);
		MoveToEx(hdc, xPos(timeWidth1[posTimeWidth]), mousePosY - TCM_TOOLBAR_HEIGTH, NULL);
		LineTo(hdc, xPos(timeWidth2[posTimeWidth]), mousePosY - TCM_TOOLBAR_HEIGTH);
		MoveToEx(hdc, xPos(timeWidth2[posTimeWidth]), TCM_WINDOW_HEIGTH, NULL);
		LineTo(hdc, xPos(timeWidth2[posTimeWidth]), 20);
		if (timeWidth1[posTimeWidth] >= timeWidth2[posTimeWidth]) {
			resultTime = tcmConvertCountsToTime(timeWidth1[posTimeWidth] - timeWidth2[posTimeWidth]);
		} else {
			resultTime = tcmConvertCountsToTime(timeWidth2[posTimeWidth] - timeWidth1[posTimeWidth]);
		}
		length = sprintf_s(str, sizeof(str), "%.3fs", resultTime);
		TextOut(hdc, xPos((unsigned long)timeWidth3[posTimeWidth]) - 15, yPosTimeWidth[posTimeWidth] - 15, str, length);
	}
	for (i = 0; i < textNum; i++) {
		if (i != posTimeWidth) {
			//X���W�̔�r�i���Ԃ̕��̌v�Z�j
			if (timeWidth1[i] >= timeWidth2[i]) {
				resultTime = tcmConvertCountsToTime(timeWidth1[i] - timeWidth2[i]);
			} else {
				resultTime = tcmConvertCountsToTime(timeWidth2[i] - timeWidth1[i]);
			}
			timeWidth3[i] = (float)(timeWidth1[i] + timeWidth2[i]) / 2;
			//Y���W�̔�r�i�l�̍����̔�r�Ɛ������������j
			MoveToEx(hdc, xPos(timeWidth1[i]), yPosTimeWidth[i], NULL);
			LineTo(hdc, xPos(timeWidth2[i]), yPosTimeWidth[i]);

			MoveToEx(hdc, xPos(timeWidth1[i]), yPos(timeWidth1DevId[i], 0), NULL);
			if (yPosTimeWidth[i] >= yPos(timeWidth1DevId[i], 0)) {
				LineTo(hdc, xPos(timeWidth1[i]), yPosTimeWidth[i] + 10);
			} else {
				LineTo(hdc, xPos(timeWidth1[i]), yPosTimeWidth[i] - 10);
			}
			MoveToEx(hdc, xPos(timeWidth2[i]), yPos(timeWidth2DevId[i], 0), NULL);
			if (yPosTimeWidth[i] >= yPos(timeWidth2DevId[i], 0)) {
				LineTo(hdc, xPos(timeWidth2[i]), yPosTimeWidth[i] + 10);
			} else {
				LineTo(hdc, xPos(timeWidth2[i]), yPosTimeWidth[i] - 10);
			}
			length = sprintf_s(str, sizeof(str), "%.3fs", resultTime);
			TextOut(hdc, xPos((unsigned long)timeWidth3[i]) - 15, yPosTimeWidth[i] - 15, str, length);
		}
	}

	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);
}

//�h���b�N�h���b�v�ŃX�N���[��
void tcmDragScroll()
{
	analyzOffSet2 += analyzOffSet1;
	if (offSetTimePos - moveOffSet - scaleDiff < 0) {
		analyzOffSet2 = offSetTimePos - scaleDiff;
	}
	analyzOffSet1 = 0;
	moveOffSet = analyzOffSet1 + analyzOffSet2;
}

//�}�E�X�����������̏���
void tcmMouseMove()
{
	if (isOnBorder()) {
		SetCursor(LoadCursor(NULL, IDC_SIZEWE));
	} else {
		SetCursor(LoadCursor(NULL, IDC_ARROW));
	}
	if (tcmClickState == ETCM_MOUSE_DOWN_BORDER) {
		borderPosX = mousePosX;
	}

	if ((tcmClickState == ETCM_MOUSE_DOWN_MOVE) || (tcmClickState == ETCM_MOUSE_DOWN_MOVE)) {
		moveOffSet = (mousePosX - cursorDownX);
		analyzOffSet1 = (mousePosX - cursorDownX);
		moveOffSet = analyzOffSet1 + analyzOffSet2;
		//�}�E�X�̓����������̏ꍇ�AEANALYZ_MOUSE_MOVE�Ɉڍs���Ȃ�
		if ((-10 <= analyzOffSet1) && (analyzOffSet1 <= 1)) {
		} else {
			tcmClickState = ETCM_MOUSE_DOWN_MOVE;
		}
	}

	switch (tcmAnalyzState) {
		case ETCM_ANALYZ_WIDTH:
		case ETCM_ANALYZ_WIDTH_LINE:
			tcmCalculateNearestValue();
			break;
		default:
			break;
	}

	if (tcmClickState == ETCM_MOUSE_DOWN) {
		changeSelectPos();
	}
}

//���N���b�N���������Ƃ��̏���
void tcmLButtonDown()
{
	if (isOnBorder()) {
		tcmClickState = ETCM_MOUSE_DOWN_BORDER;
		SetCursor(LoadCursor(NULL, IDC_SIZEWE));
	} else {
		if (isOnTcm()) {
			cursorDownX = mousePosX;
			tcmClickState = ETCM_MOUSE_DOWN_MOVE;
		} else {
			tcmClickState = ETCM_MOUSE_DOWN;
		}
		SetCursor(LoadCursor(NULL, IDC_ARROW));
	}

	changeSelectPos();
}

//���N���b�N�𗣂����Ƃ��̏���
void tcmLButtonUp()
{
	if (isOnBorder()) {
		SetCursor(LoadCursor(NULL, IDC_SIZEWE));
	} else {
		SetCursor(LoadCursor(NULL, IDC_ARROW));
	}
	tcmDragScroll();
	tcmClickState = ETCM_MOUSE_UP;

	switch (tcmAnalyzState) {
		case ETCM_ANALYZ_WIDTH:
			tcmCalculateNearestValue();
			tcmAnalyzState = ETCM_ANALYZ_WIDTH_LINE;
			break;
		case ETCM_ANALYZ_WIDTH_LINE:
			posTimeWidth++;
			if (posTimeWidth >= TCM_WIDTH_BUF_NUM) {
				posTimeWidth = 0;
				overTimeWidthFlg = true;
			}
			tcmCalculateNearestValue();
			tcmAnalyzState = ETCM_ANALYZ_WIDTH;
			break;
		default:
			break;
	}
}

//�J�[�\���ɋ߂��l���v�Z����֐�
void tcmCalculateNearestValue()
{
	unsigned long minNum;
	signed long diffX;

	diffX = mousePosX - xPos(tcmLogBuf[0].timeCount) - borderPosX;
	minNum = diffX * diffX;
	for (unsigned long i = 0; i < TCM_DATA_BUF_NUM; i++) {
		diffX = mousePosX - xPos(tcmLogBuf[i].timeCount) - borderPosX;
		if (minNum >= (unsigned long)(diffX * diffX)) {
			minNum = diffX * diffX;
			if (tcmAnalyzState == ETCM_ANALYZ_WIDTH) {
				timeWidth1[posTimeWidth] = tcmLogBuf[i].timeCount;
				timeWidth1DevId[posTimeWidth] = tcmLogBuf[i].id;
			} else if (tcmAnalyzState == ETCM_ANALYZ_WIDTH_LINE) {
				timeWidth2[posTimeWidth] = tcmLogBuf[i].timeCount;
				timeWidth2DevId[posTimeWidth] = tcmLogBuf[i].id;
				timeWidth3[posTimeWidth] = (float)(timeWidth1[posTimeWidth] + tcmLogBuf[i].timeCount) / 2;
				
			} else {
			}
		}
		yPosTimeWidth[posTimeWidth] = mousePosY - TCM_TOOLBAR_HEIGTH;
	}
}

unsigned char selectFontSize(int areaSize, int strleng)
{
	unsigned char ret;
	int i;

	//�f�t�H���g�t�H���g�T�C�Y12 �̎�1����6��dot�Ƃ���
	//(areaSize < strleng * 6.0�j�ł̓t�H���g�T�C�Y11
	//(areaSize < strleng * 5.5�j�ł̓t�H���g�T�C�Y10
	for (i = 0; i < 12; i++) {
		if (areaSize < ((float)strleng * float(10 + (i * 5)) / 10)) {
			break;
		}
	}
	ret = i;
	return ret;
}

unsigned long getWritePosX(unsigned long posX, unsigned long lastX, int strleng, unsigned char fontSize)
{
	unsigned long ret;

	if (fontSize > 10) {
		ret = (posX + lastX) / 2 - (strleng * 6 / 2) + 2;
	} else if (fontSize > 8) {
		ret = (posX + lastX) / 2 - (strleng * 5 / 2) + 1;
	} else if (fontSize > 6) {
		ret = (posX + lastX) / 2 - (strleng * 4 / 2) + 1;
	} else if (fontSize > 4) {
		ret = (posX + lastX) / 2 - (strleng * 3 / 2);
	} else if (fontSize > 2) {
		ret = (posX + lastX) / 2 - (strleng * 3 / 4) - 1;
	} else {
		ret = (posX + lastX) / 2 - (strleng * 3 / 8);
	}
	return ret;
}

unsigned long getWritePosY(unsigned long posY, unsigned char fontSize)
{
	int ret;
	if (fontSize > 10) {
		ret = posY - 5;
	} else if (fontSize > 8) {
		ret = posY - 4;
	} else if (fontSize > 6) {
		ret = posY - 3;
	} else if (fontSize > 4) {
		ret = posY - 2;
	} else if (fontSize > 2) {
		ret = posY - 1;
	} else {
		ret = posY;
	}
	return ret;
}

//�S�����{�^��
void tdmAllReset()
{
	tcmBufPos = 0;
	tcmBufOverFlg = false;
	offSetTimePos = 0;
	timeScale = 20;
	analyzOffSet1 = 0;
	analyzOffSet2 = 0;
	moveOffSet = 0;			//�J�[�\���ړ��ɔ�����ʈړ��̃s�N�Z����
	scaleDiffCount = 0;		//�g��k���� �ύX�O�̒��S�̃J�E���g�l
	scaleDiff = 0;
	tcmAnalyzState = ETCM_ANALYZ_NORMAL;
}

//���O���t���b�V��
void tcmRefresh()
{
	char *ptr;
	char *ctx;//�����I�Ɏg�p����̂Ő[���l���Ȃ�
	unsigned long timeCount;
	unsigned short id;
	unsigned short onOff;
	unsigned long value;

	if (tcmFileP != NULL) {
		tcmBufPos = 0;
		tcmBufOverFlg = false;
		offSetTimePos = 0;
		fclose(tcmFileP);
		sprintf_s(buf, "%s%s",szGlPath,"tcm.txt");
		fopen_s(&tcmFileP, buf, "r");//���O�t�@�C��
		isTcmRefresh = true;
		while (fgets(buf, sizeof(buf), tcmFileP) != NULL) {
			ptr = strtok_s(buf, ",\r\n", &ctx);
			if (ptr == NULL) continue;
			timeCount = atoi(ptr);
			ptr = strtok_s(NULL, ",\r\n", &ctx);
			if (ptr == NULL) continue;
			id = atoi(ptr);
			ptr = strtok_s(NULL, ",\r\n", &ctx);
			if (ptr == NULL) continue;
			onOff = atoi(ptr);
			ptr = strtok_s(NULL, ",\r\n", &ctx);
			if (ptr == NULL) continue;
			value = atoi(ptr);
			tcmLogInfo(timeCount,id, onOff, value);
		}
		tcmLogInfo(maxCount,0x7ff0, 0, 0);

		fclose(tcmFileP);
		sprintf_s(buf, "%s%s",szGlPath,"tcm.txt");
		fopen_s(&tcmFileP, buf, "a");//���O�t�@�C��
		isTcmRefresh = false;
	}
}

void outPutTcdFile()
{
	STARTUPINFO si = {};
	PROCESS_INFORMATION pi = {};
	char fileName[256];
	char valueStr[20];
	char *ptr;
	char *ctx;//�����I�Ɏg�p����̂Ő[���l���Ȃ�
	unsigned long preTime = 0;
	unsigned long readTime = 0;
	unsigned short sameWrapCount = 0;
	unsigned short id;
	unsigned short onOff;
	unsigned long value;

	unsigned char dev;
	unsigned char io;
	char onOffChar = 'Z';

	if (tcmFileP == NULL) {
		return;
	}
	fclose(tcmFileP);
	sprintf_s(buf, sizeof(buf), "%s%s",szGlPath,"tcm.txt");
	fopen_s(&tcmFileP, buf, "r");//���O�t�@�C��
	
	sprintf_s(buf, sizeof(buf), "%s%s",szGlPath,"dummy.tcd");
	fopen_s(&dummyFileP, buf, "w");//���O�t�@�C��
	
	// �w�b�_��
	fprintf(dummyFileP, "$version, 2\n");
	fprintf(dummyFileP, "$timeunit, ms\n");
	fprintf(dummyFileP, "$timeperdot, 10.000000\n");
	fprintf(dummyFileP, "$grid, 0.00001\n");
	fprintf(dummyFileP, "$edgeangle, 90\n");
	fprintf(dummyFileP, "\n\n");
	
	for (dev = 0; dev < deviceSetNum; dev++) {
		if (tcmSetting[dev].isDispValue == true) {//�ݒ�l�̏ꍇ
			io = (tcmSetting[dev].devId % 10);
			if (io == 0) {
				fprintf(dummyFileP, "$bus,\t\t%s,\tIN, 90\n\tZ=0,\t", deviceName[dev]);
			} else {
				fprintf(dummyFileP, "$bus,\t\t%s,\tOUT, 16, 10, 90\n\tZ=0,\t", deviceName[dev]);
			}
		} else {//OnOff�̏ꍇ
			if (io == 0) {
				fprintf(dummyFileP, "$signal,\t%s,\tIN, 90\n\tZ,\t", deviceName[dev]);
			} else {
				fprintf(dummyFileP, "$signal,\t%s,\tOUT, 90\n\tZ,\t", deviceName[dev]);
			}
		}
		fseek(tcmFileP, 0, SEEK_SET);
		sameWrapCount = 0;
		while (fgets(buf, sizeof(buf), tcmFileP) != NULL) {
			ptr = strtok_s(buf, ",\r\n", &ctx);
			readTime = atoi(ptr) * msPerClock;
			ptr = strtok_s(NULL, ",\r\n", &ctx);
			id = atoi(ptr);
			ptr = strtok_s(NULL, ",\r\n", &ctx);
			onOff = atoi(ptr);
			ptr = strtok_s(NULL, ",\r\n", &ctx);
			value = atoi(ptr);
			if (tcmSetting[dev].devId == id) {
				if (readTime == preTime) {
					sameWrapCount++;
				} else {
					sameWrapCount = 0;
				}
				preTime = readTime;
				if (tcmSetting[dev].isDispValue == true) {//�ݒ�l�̏ꍇ
					if (isScenario(id)) {
						if (tcmSetting[dev].sceParam == (int)(value / TCM_PAGEID_DIGIT)) {
							if (sameWrapCount == 0) {
								fprintf(dummyFileP, "%d=%d,\t", value % TCM_PAGEID_DIGIT, readTime);
							} else {
								fprintf(dummyFileP, "%d=%d.%02d,\t", value % TCM_PAGEID_DIGIT, readTime, sameWrapCount);
							}
						}
					} else {
						if (tcmSetting[dev].isHexadecimal == true) {
							sprintf_s(valueStr, sizeof(valueStr), "%04x", value);
						} else {
							sprintf_s(valueStr, sizeof(valueStr), "%d", value);
						}
						if (sameWrapCount == 0) {
							fprintf(dummyFileP, "%s=%d,\t", valueStr, readTime);
						} else {
							fprintf(dummyFileP, "%s=%d.%02d,\t", valueStr, readTime, sameWrapCount);
						}
						
					}
				} else {
					if (value == TCM_REVERSE_VALUE) {
						onOffChar = 'L';
					} else if (value == 0) {
						onOffChar = 'Z';
					} else {
						onOffChar = 'H';
					}
					if (sameWrapCount == 0) {
						fprintf(dummyFileP, "%c=%d,\t", onOffChar, readTime);
					} else {
						fprintf(dummyFileP, "%c=%d.%02d,\t", onOffChar, readTime, sameWrapCount);
					}
					
				}
			}
		}
		if (tcmSetting[dev].isDispValue == true) {//�ݒ�l�̏ꍇ
			fprintf(dummyFileP, " %d=%d, \t", value, maxCount * 2);
		} else {
			fprintf(dummyFileP, "%c=%d,", onOffChar, maxCount * 2);
		}
		fprintf(dummyFileP, "\n");
	}
	fclose(tcmFileP);
	sprintf_s(buf, sizeof(buf), "%s%s",szGlPath,"tcm.txt");
	fopen_s(&tcmFileP, buf, "a");
	fclose(dummyFileP);

	sprintf_s(fileName, sizeof(fileName), "%s%s", szGlPath, "dummy.tcd");
	HINSTANCE instance = ShellExecute(NULL, "open", fileName, NULL, NULL, SW_SHOWNORMAL);
}

//�}�E�X�����E���ォ�ǂ���
bool isOnBorder()
{
	bool ret;
	if ((mousePosX >= (borderPosX - 2)) && (mousePosX <= (borderPosX + 2)) && (mousePosY > TCM_BORDER_POS_Y)) {
		ret = true;
	} else {
		ret = false;
	}
	return ret;
}

//�}�E�X���^�C�~���O�`���[�g�ォ�ǂ���
bool isOnTcm()
{
	bool ret;
	if ((mousePosX > (borderPosX + 2)) && (mousePosY > TCM_BORDER_POS_Y)) {
		ret = true;
	} else {
		ret = false;
	}
	return ret;
}

//x���W(���Ԏ��j�̌v�Z������֐�
signed long xPos(signed long timeCount)
{
	signed long xPosition = X_0 + (timeCount / (signed long)timeScale) - offSetTimePos + moveOffSet + scaleDiff;

	return xPosition;
}

//y���W(���Ԏ��j�̌v�Z������֐�
unsigned long yPos(unsigned short id, unsigned short onOff)
{
	unsigned long yPosition;
	if (onOff == TCM_REVERSE_VALUE) {
		yPosition = (TCM_BORDER_POS_Y) + (TCM_INTERVAL / 2) + (TCM_INTERVAL * id) + 7;
	} else if (onOff == 0) {
		yPosition = (TCM_BORDER_POS_Y) + (TCM_INTERVAL / 2) + (TCM_INTERVAL * id);
	} else {
		yPosition = (TCM_BORDER_POS_Y) + (TCM_INTERVAL / 2) + (TCM_INTERVAL * id) - 7;
	}
	return yPosition;
}

//x���W���J�E���g�l�ɕϊ�����֐�
signed long inverseXPos(unsigned long xPos)
{
	unsigned long xCount = (xPos - X_0 + offSetTimePos - moveOffSet - scaleDiff) * timeScale;

	return xCount;
}


//�w�肵���b�������s�N�Z�������v�Z����֐�
unsigned long pixelsMoved(unsigned long timeSec)
{
	unsigned long pixels = 500 * timeSec / timeScale;

	return pixels;
}

bool isScenario(int id) 
{
	if (senarioSeparete(id >> 4) ) {
		return true;
	} else {
		return false;
	}
}

//�J�E���g�l�����Ԃɕϊ�����֐�
float tcmConvertCountsToTime(signed long count)
{
	float time;
	
	time = ((float)(count * msPerClock) / 1000);

	return time;
}
