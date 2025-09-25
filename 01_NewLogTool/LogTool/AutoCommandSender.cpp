#include "stdafx.h"
#include "LogTool.h"
#include "Resource.h"
#include "CommDlg.h"
#include "shlwapi.h"
#include <stdio.h>
#include <windowsx.h>
#include <ShlObj.h>
#include <shellapi.h> //ドラッグ＆ドロップ用
#include <regex> //正規表現用


#define TIMER_ID_WAIT_TIME 1
#define TIMER_ID_LOG_TIME_OUT 2
#define STATIC_POS 10
#define FIRST_WINDOW_POS 10
#define COMB_POS 40
#define COMBBOX_SIZE 220
#define EDIT_POS COMB_POS + COMBBOX_SIZE
#define CTRL_NUM 200
#define W_SIZE 24
#define EDIT_SIZE 180
#define BUTTON_SIZE 32
#define COMB_SIZE_Y 260
#define NEXT_POS 30
#define INIT_COMB_NUM 5
#define STR_MAX_LEN 200
#define PATH_MAX_LEN 255
#define SUB_SETTING_NUM 10

extern void calculateData(char *buffer);
extern void sendText(char *buffer);
extern bool judgement(char *buffer);
extern void changeParamValue(char *buffer, unsigned char kind);

extern HWND hSyslogDlgWnd;
extern HWND hDlgAcs;
extern char logToolSettingPath[STR_MAX_LEN];
extern char szIniFilePrivate[];

INT_PTR CALLBACK waitSettingProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK autoCommandSenderProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK comboBoxSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

bool setSettingFileName(HWND hDlg, char* filePath, char *fileName, char *ext);
void selectAction(HWND hDlg, unsigned char index);
void lineAddtion(HWND hDlg);
void lineDelete(HWND hDlg);
void dispClear(HWND hDlg);
void dispSubSetting(HWND hDlg);
unsigned char getSubSettingPoint(unsigned char index);
void readMacro(HWND hDlg, HDROP hDro);
void screenUpdate(HWND hDlg);
void scrollUpdate(HWND hDlg);
void execute(HWND hDlg);
void stop(HWND hDlg);
void end(HWND hDlg);
void pause(HWND hDlg);
void sendCommand(HWND hDlg);
void waitTime(HWND hDlg);
void startLogMonitor(HWND hDlg);
void startLogMonitorRegex(HWND hDlg);
void timeOutSeting(HWND hDlg);
void LogMonitor(char* str, unsigned long time, double *param, unsigned char dataNum);
void LogMonitorNormal(char* str, unsigned long time, double *param, unsigned char dataNum);
void LogMonitorRegex(char* str, unsigned long time, double *param, unsigned char dataNum);
void detectEndLogCheck(char* str, unsigned long time, double *param, unsigned char dataNum);
void jumpPoint(HWND hDlg);
void saveTextlog(HWND hDlg);
void saveTcdlog(HWND hDlg);
void outputMessage(HWND hDlg);
void exeReadFile(HWND hDlg);
void logClear(HWND hDlg);
void outPutData(HWND hDlg);
void calculation(HWND hDlg);
void keepData(unsigned long keepTime, double *param, unsigned char dataNum);
void branch(HWND hDlg);
void externalTool(HWND hDlg);

void vScroll(HWND hDlg, WPARAM wp);
int getExePos(HWND hDlg, unsigned short num);
int getWinMaxPos(HWND hDlg);

enum eCB_VAL {
	ESEND_CMD,
	EWAIT_TIME,
	EJMP_POINT,
	EWAIT_LOG,
	EWAIT_REGLOG,
	ESAVE_TEXT,
	ESAVE_TCD,
	EDISP_MSG,
	EEXE_FILE,
	ELOG_CLEAR,
	EOUTPUT_DATA,
	ECALCULATION,
	EBRANCH,
	EEXTERNAL_TOOL,
	EWAIT_END,
	ECB_VAL_NUM
};

const char *cbValText[ECB_VAL_NUM] = {
	 "SEND_CMD"
	,"WAIT_TIME"
	,"JMP_POINT"
	,"WAIT_LOG"
	,"WAIT_REGLOG"
	,"SAVE_TEXT"
	,"SAVE_TCD"
	,"DISP_MSG"
	,"EXE_FILE"
	,"LOG_CLEAR"
	,"OUTPUT_DATA"
	,"CALCULATION"
	,"BRANCH"
	,"EXTERNAL_TOOL"
	,"WAIT_END"
};

enum eLOG_DETECT_TYPE {
	ELOG_DETECT_NULL,
	ELOG_DETECT_NORMAL,
	ELOG_DETECT_REGEX,
	ELOG_DETECT_NUM
};

enum eBRUSH_COLOR {
	EBRUSH_COLOR_GREEN,
	EBRUSH_COLOR_GRAY_L,
	EBRUSH_COLOR_WHITE,
	EBRUSH_COLOR_GRAY_D
};

enum eCTRL_WIN_TYPE {
	ECTRL_WIN_EDIT,
	ECTRL_WIN_CMMENT,
};

struct SUB_SETTING {
	unsigned char id;
	unsigned long timeOut;
	unsigned char timeType;
	unsigned char timeBraPoint;
	char detectEndLog[STR_MAX_LEN];
	bool isRegCheck;
	unsigned char detectLogBraPoint;
	bool commentDisp;
};

static SUB_SETTING subSetting[SUB_SETTING_NUM];
static unsigned short ctrlNum = INIT_COMB_NUM; 
static unsigned char exePoint = 0;
static unsigned char nextPoint = 0;
static unsigned char nextBraPoint = 0;
static bool isExe = false;
static bool isStepExe = false;
static HFONT hControlFont;
static HFONT hSubSetFont;
static HWND staticText[CTRL_NUM];
static HWND combo[CTRL_NUM];
static HWND edit[CTRL_NUM];
static HWND button[CTRL_NUM];
static HWND comment[CTRL_NUM];
static HWND subSetStaticText[SUB_SETTING_NUM];
static char autoCommandSenderSettingPath[STR_MAX_LEN];
static char autoCommandSenderFileName[STR_MAX_LEN];
static signed long scrollValue = 0;
static HWND hMyDlg;
static char waitLog[STR_MAX_LEN];
static eLOG_DETECT_TYPE detectType = ELOG_DETECT_NULL;
static SCROLLINFO scr;
static unsigned char subScenario = 0;
static unsigned char buttonId;
bool comBusyFlag = false;

void setLogToolSettingPath(HWND hDlg)
{
	GetPrivateProfileString(_T("PATH"), _T("ACS_SETTING_FILE"), logToolSettingPath, autoCommandSenderSettingPath, sizeof(autoCommandSenderSettingPath), logToolSettingPath);
	GetPrivateProfileString(_T("PATH"), _T("ACS_SETTING_FILE"), autoCommandSenderSettingPath, autoCommandSenderSettingPath, sizeof(autoCommandSenderSettingPath), szIniFilePrivate);
}

void keepInAction()
{
	sprintf_s(buf, "%slogToolState.ini",szGlPath);
	if (isExe == true) {
		WritePrivateProfileString(_T("AUTO_CMMMACD_SEND"), _T("IS_RUNNING"), _T("1"), buf);
	} else {
		WritePrivateProfileString(_T("AUTO_CMMMACD_SEND"), _T("IS_RUNNING"), _T("0"), buf);
	}
}

void readLogToolIni(HWND hDlg)
{
	unsigned short i;
	unsigned short j;

	for (i = 0;i < CTRL_NUM; i++) {
		sprintf_s(buf, "CMB_SETTING_%d", i);
		GetPrivateProfileString(_T("AUTO_CMMMACD_SEND"), _T(buf), _T(""), buf, sizeof(buf), autoCommandSenderSettingPath);
		for (j = 0; j < ECB_VAL_NUM; j++) {
			if (strncmp(buf, cbValText[j], strlen(cbValText[j])) == 0) {
				SendMessage(combo[i] , CB_SETCURSEL , j , 0);
				SendMessage(hDlg , WM_COMMAND , CBN_SELCHANGE<<16 , (LPARAM)combo[i]);
			}
		}
		sprintf_s(buf, "VAL_SETTING_%d", i);
		GetPrivateProfileString(_T("AUTO_CMMMACD_SEND"), _T(buf), _T(""), buf, sizeof(buf), autoCommandSenderSettingPath);
		if (edit[i] != NULL) {
			SendMessage(edit[i], WM_SETTEXT, 0, (LPARAM)buf);
		}
		sprintf_s(buf, "COMMENT_%d", i);
		GetPrivateProfileString(_T("AUTO_CMMMACD_SEND"), _T(buf), _T(""), buf, sizeof(buf), autoCommandSenderSettingPath);
		if (comment[i] != NULL) {
			if (buf[0] == NULL) {
				sprintf_s(buf, ".");
			}
			SendMessage(comment[i], WM_SETTEXT, 0, (LPARAM)buf);
		}
	}
	GetPrivateProfileString(_T("AUTO_CMMMACD_SEND"), "EXE_POINT", _T("0"), buf, sizeof(buf), autoCommandSenderSettingPath);
	exePoint = atoi(buf);

	for (i = 0; i < SUB_SETTING_NUM; i++) {
		sprintf_s(buf, "ID_%d", i);
		GetPrivateProfileString(_T("AUTO_CMMMACD_SEND_SUB"), buf, _T("255"), buf, sizeof(buf), autoCommandSenderSettingPath);
		subSetting[i].id = atoi(buf);
		sprintf_s(buf, "TIMEOUT_%d", i);
		GetPrivateProfileString(_T("AUTO_CMMMACD_SEND_SUB"), buf, _T("0"), buf, sizeof(buf), autoCommandSenderSettingPath);
		subSetting[i].timeOut = atoi(buf);
		sprintf_s(buf, "TIMEOUT_TYPE_%d", i);
		GetPrivateProfileString(_T("AUTO_CMMMACD_SEND_SUB"), buf, _T("0"), buf, sizeof(buf), autoCommandSenderSettingPath);
		subSetting[i].timeType = atoi(buf);
		sprintf_s(buf, "TIME_BRA_POINT_%d", i);
		GetPrivateProfileString(_T("AUTO_CMMMACD_SEND_SUB"), buf, _T("255"), buf, sizeof(buf), autoCommandSenderSettingPath);
		subSetting[i].timeBraPoint = atoi(buf);
		sprintf_s(buf, "DETECT_END_LOG_%d", i);
		GetPrivateProfileString(_T("AUTO_CMMMACD_SEND_SUB"), buf, _T(""), buf, sizeof(buf), autoCommandSenderSettingPath);
		sprintf_s(subSetting[i].detectEndLog, buf);
		sprintf_s(buf, "IS_REG_CHECK_%d", i);
		GetPrivateProfileString(_T("AUTO_CMMMACD_SEND_SUB"), buf, _T("0"), buf, sizeof(buf), autoCommandSenderSettingPath);
		subSetting[i].isRegCheck = (atoi(buf) == 1);
		sprintf_s(buf, "DETECT_LOG_BRA_POINT_%d", i);
		GetPrivateProfileString(_T("AUTO_CMMMACD_SEND_SUB"), buf, _T("255"), buf, sizeof(buf), autoCommandSenderSettingPath);
		subSetting[i].detectLogBraPoint = atoi(buf);
		sprintf_s(buf, "IS_COMMENT_DISP_%d", i);
		GetPrivateProfileString(_T("AUTO_CMMMACD_SEND_SUB"), buf, _T("1"), buf, sizeof(buf), autoCommandSenderSettingPath);
		subSetting[i].commentDisp = (atoi(buf) == 1);
	}
	dispSubSetting(hDlg);

	char name[100];
	GetPrivateProfileString(_T("AUTO_CMMMACD_SEND"), "FILE_NAME", _T(""), buf, sizeof(buf), autoCommandSenderSettingPath);
	sprintf_s(name, "Auto Command Sender - %s",buf);
	SendMessage(hDlg, WM_SETTEXT, 0, (LPARAM)name);
	strcpy_s(autoCommandSenderFileName, buf);
}

void writeLogToolIni(HWND hDlg)
{
	unsigned short i;
	LRESULT selNum;
	LRESULT textlen;
	char text[PATH_MAX_LEN];

	for (i = 0; i < ctrlNum; i++) {
		selNum = SendMessage(combo[i] , CB_GETCURSEL , 0 , 0);
		sprintf_s(buf, "CMB_SETTING_%d", i);
		if (selNum != -1) {
			WritePrivateProfileString(_T("AUTO_CMMMACD_SEND"), _T(buf), cbValText[selNum], autoCommandSenderSettingPath);
		} else {
			WritePrivateProfileString(_T("AUTO_CMMMACD_SEND"), _T(buf), NULL, autoCommandSenderSettingPath);
		}
		textlen = SendMessage(edit[i], WM_GETTEXT, sizeof(text), (LPARAM)text);
		sprintf_s(buf, "VAL_SETTING_%d", i);
		if (textlen != 0) {		
			WritePrivateProfileString(_T("AUTO_CMMMACD_SEND"), _T(buf), text, autoCommandSenderSettingPath);
		} else {
			WritePrivateProfileString(_T("AUTO_CMMMACD_SEND"), _T(buf), NULL, autoCommandSenderSettingPath);
		}
		textlen = SendMessage(comment[i], WM_GETTEXT, sizeof(text), (LPARAM)text);
		sprintf_s(buf, "COMMENT_%d", i);
		if (textlen != 0) {		
			WritePrivateProfileString(_T("AUTO_CMMMACD_SEND"), _T(buf), text, autoCommandSenderSettingPath);
		} else {
			WritePrivateProfileString(_T("AUTO_CMMMACD_SEND"), _T(buf), NULL, autoCommandSenderSettingPath);
		}
	}
	for (i = ctrlNum; i < CTRL_NUM; i++) {
		sprintf_s(buf, "CMB_SETTING_%d", i);
		WritePrivateProfileString(_T("AUTO_CMMMACD_SEND"), _T(buf), NULL, autoCommandSenderSettingPath);
		sprintf_s(buf, "VAL_SETTING_%d", i);
		WritePrivateProfileString(_T("AUTO_CMMMACD_SEND"), _T(buf), NULL, autoCommandSenderSettingPath);
		sprintf_s(buf, "COMMENT_%d", i);
		WritePrivateProfileString(_T("AUTO_CMMMACD_SEND"), _T(buf), NULL, autoCommandSenderSettingPath);
	}
	sprintf_s(buf, "%d", exePoint);
	WritePrivateProfileString(_T("AUTO_CMMMACD_SEND"), "EXE_POINT", buf, autoCommandSenderSettingPath);

	for (i = 0; i < SUB_SETTING_NUM; i++) {
		sprintf_s(buf, "ID_%d", i);
		sprintf_s(text, "%d", subSetting[i].id);
		WritePrivateProfileString(_T("AUTO_CMMMACD_SEND_SUB"), buf, (subSetting[i].id != 0xff ? text : NULL), autoCommandSenderSettingPath);
		sprintf_s(buf, "TIMEOUT_%d", i);
		sprintf_s(text, "%d", subSetting[i].timeOut);
		WritePrivateProfileString(_T("AUTO_CMMMACD_SEND_SUB"), buf, (subSetting[i].id != 0xff ? text : NULL), autoCommandSenderSettingPath);
		sprintf_s(buf, "TIMEOUT_TYPE_%d", i);
		sprintf_s(text, "%d", subSetting[i].timeType);
		WritePrivateProfileString(_T("AUTO_CMMMACD_SEND_SUB"), buf, (subSetting[i].id != 0xff ? text : NULL), autoCommandSenderSettingPath);
		sprintf_s(buf, "TIME_BRA_POINT_%d", i);
		sprintf_s(text, "%d", subSetting[i].timeBraPoint);
		WritePrivateProfileString(_T("AUTO_CMMMACD_SEND_SUB"), buf, (subSetting[i].id != 0xff ? text : NULL), autoCommandSenderSettingPath);
		sprintf_s(buf, "DETECT_END_LOG_%d", i);
		sprintf_s(text, "%s", subSetting[i].detectEndLog);
		WritePrivateProfileString(_T("AUTO_CMMMACD_SEND_SUB"), buf, (subSetting[i].id != 0xff ? text : NULL), autoCommandSenderSettingPath);
		sprintf_s(buf, "IS_REG_CHECK_%d", i);
		sprintf_s(text, "%d", subSetting[i].isRegCheck);
		WritePrivateProfileString(_T("AUTO_CMMMACD_SEND_SUB"), buf, (subSetting[i].id != 0xff ? text : NULL), autoCommandSenderSettingPath);
		sprintf_s(buf, "DETECT_LOG_BRA_POINT_%d", i);
		sprintf_s(text, "%d", subSetting[i].detectLogBraPoint);
		WritePrivateProfileString(_T("AUTO_CMMMACD_SEND_SUB"), buf, (subSetting[i].id != 0xff ? text : NULL), autoCommandSenderSettingPath);
		sprintf_s(buf, "IS_COMMENT_DISP_%d", i);
		sprintf_s(text, "%d", subSetting[i].commentDisp);
		WritePrivateProfileString(_T("AUTO_CMMMACD_SEND_SUB"), buf, (subSetting[i].id != 0xff ? text : NULL), autoCommandSenderSettingPath);
	}

	char name[100];
	WritePrivateProfileString(_T("AUTO_CMMMACD_SEND"), "FILE_NAME", autoCommandSenderFileName, autoCommandSenderSettingPath);
	sprintf_s(name, "Auto Command Sender - %s",autoCommandSenderFileName);
	SendMessage(hDlg, WM_SETTEXT, 0, (LPARAM)name);
}

bool setSettingFileName(HWND hDlg, char *filePath, char *fileName, char *ext)
{
	int lengh;
	char name[PATH_MAX_LEN];

	OPENFILENAME ofn;
	// 構造体に情報をセット
	ZeroMemory(&ofn, sizeof(ofn));			// 最初にゼロクリアしておく
	ofn.lStructSize = sizeof(ofn);			// 構造体のサイズ
	ofn.hwndOwner = hDlg;                   // コモンダイアログの親ウィンドウハンドル
	if (strncmp(ext, "log", 3) == 0) {
		ofn.lpstrFilter = "設定(*.log)\0*.log\0All files(*.*)\0*.*\0\0";
	} else if (strncmp(ext, "tcd", 3) == 0) {
		ofn.lpstrFilter = "設定(*.tcd)\0*.tcd\0All files(*.*)\0*.*\0\0";
	} else {
		ofn.lpstrFilter = "設定(*.ini)\0*.ini\0All files(*.*)\0*.*\0\0";
	}
				// ファイルの種類
	ofn.lpstrFile = buf;					// 選択されたファイル名(フルパス)を受け取る変数のアドレス
	ofn.lpstrFileTitle = name;				// 選択されたファイル名を受け取る変数のアドレス
	ofn.nMaxFile = sizeof(buf);				// lpstrFileに指定した変数のサイズ
	ofn.nMaxFileTitle = sizeof(name);		// lpstrFileTitleに指定した変数のサイズ
	ofn.Flags = OFN_OVERWRITEPROMPT;        // フラグ指定
	ofn.lpstrTitle = _T("ファイル設定");
	ofn.lpstrDefExt = ext;					// デフォルトのファイルの種類
											// 名前を付けて保存コモンダイアログを作成

	buf[0] = 0; //読み込む前にクリアしておかないと、キャンセルした場合にエーラになるため
	if(!GetOpenFileName(&ofn)) {
		if (buf[0] != 0) {
			MessageBox(NULL, TEXT("ファイル名が長すぎる可能性があります"),TEXT("エラー"), MB_ICONWARNING);
		}
		return false;
	}
	lengh = strlen(buf);
	if (lengh > STR_MAX_LEN) {
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

void createWindowFont()
{
	hControlFont = CreateFont(15, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH, "Meiryo UI" );
	hSubSetFont = CreateFont(11, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH, "ＭＳ ゴシック" );
}
void createExePosWindow(HWND hDlg, unsigned short index)
{
	sprintf_s(buf, "%d", index);
	staticText[index] = CreateWindow(TEXT("STATIC") , TEXT(buf), 
		WS_CHILD | WS_VISIBLE | SS_CENTER |SS_CENTERIMAGE | WS_BORDER |SS_NOTIFY, 
		STATIC_POS, FIRST_WINDOW_POS + scrollValue + (NEXT_POS * index), W_SIZE, W_SIZE, hDlg, GetMenu(hDlg), NULL, NULL);
	SendMessage(staticText[index], WM_SETFONT, (WPARAM)hControlFont, MAKELPARAM(TRUE, 0));
}

void createComboBox(HWND hDlg, unsigned short index)
{
	combo[index] = CreateWindow(TEXT("COMBOBOX"), NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWN | CBS_HASSTRINGS, COMB_POS , FIRST_WINDOW_POS + scrollValue + (NEXT_POS * index), COMBBOX_SIZE, COMB_SIZE_Y, hDlg, NULL, NULL, NULL);
	SendMessage(combo[index], WM_SETFONT, (WPARAM)hControlFont, MAKELPARAM(TRUE, 0));
	SendMessage(combo[index] , CB_ADDSTRING , 0 , (LPARAM)"指定したコマンド送信する");
	SendMessage(combo[index] , CB_ADDSTRING , 0 , (LPARAM)"指定した時間待つ[msec]");
	SendMessage(combo[index] , CB_ADDSTRING , 0 , (LPARAM)"指定したPointに移動");
	SendMessage(combo[index] , CB_ADDSTRING , 0 , (LPARAM)"指定したログが出るまで待つ");
	SendMessage(combo[index] , CB_ADDSTRING , 0 , (LPARAM)"指定したログ(正規表現)が出るまで待つ");
	SendMessage(combo[index] , CB_ADDSTRING , 0 , (LPARAM)"ログをテキストで保存");
	SendMessage(combo[index] , CB_ADDSTRING , 0 , (LPARAM)"ログをtcd保存");
	SendMessage(combo[index] , CB_ADDSTRING , 0 , (LPARAM)"メッセージボックスを表示");
	SendMessage(combo[index] , CB_ADDSTRING , 0 , (LPARAM)"ファイルを読み込んで実行");
	SendMessage(combo[index] , CB_ADDSTRING , 0 , (LPARAM)"ログ表示をクリア");
	SendMessage(combo[index] , CB_ADDSTRING , 0 , (LPARAM)"データのログ出力");
	SendMessage(combo[index] , CB_ADDSTRING , 0 , (LPARAM)"数値計算");
	SendMessage(combo[index] , CB_ADDSTRING , 0 , (LPARAM)"分岐処理");
	SendMessage(combo[index] , CB_ADDSTRING , 0 , (LPARAM)"外部ツールの起動");
	SendMessage(combo[index] , CB_ADDSTRING , 0 , (LPARAM)"終了");
	
	SetWindowSubclass(combo[index], comboBoxSubclassProc, 0, 0); // サブクラス化
}

void createButtonWindow(HWND hDlg, unsigned short index)
{
	LRESULT selNum;
	selNum = SendMessage(combo[index] , CB_GETCURSEL , 0 , 0);

	button[index] = CreateWindow(TEXT("BUTTON"), TEXT(""),WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, EDIT_POS+EDIT_SIZE-BUTTON_SIZE , FIRST_WINDOW_POS + scrollValue  + (NEXT_POS * index), BUTTON_SIZE ,W_SIZE, hDlg, NULL, NULL, NULL);
	SendMessage(button[index], WM_SETFONT, (WPARAM)hControlFont, MAKELPARAM(TRUE, 0));
}
void createControlWindow(HWND hDlg, unsigned short index)
{
	edit[index] =  CreateWindow(TEXT("EDIT"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER|ES_AUTOHSCROLL, EDIT_POS, FIRST_WINDOW_POS + scrollValue  + (NEXT_POS * index), EDIT_SIZE ,W_SIZE, hDlg, NULL, NULL, NULL);
	SendMessage(edit[index], WM_SETFONT, (WPARAM)hControlFont, MAKELPARAM(TRUE, 0));
	comment[index] = CreateWindow(TEXT("EDIT"), TEXT(""), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, EDIT_POS+EDIT_SIZE , FIRST_WINDOW_POS + scrollValue  + (NEXT_POS * index) + 8, EDIT_SIZE,W_SIZE, hDlg, NULL, NULL, NULL);
	SendMessage(comment[index], WM_SETFONT, (WPARAM)hControlFont, MAKELPARAM(TRUE, 0));
	SendMessage(comment[index], WM_SETTEXT, 0, (LPARAM)(""));
}

void changeWindowPos(HWND hDlg, unsigned short index, eCTRL_WIN_TYPE type, unsigned long moveSize)
{
	if (type == ECTRL_WIN_EDIT) {
		SetWindowPos(edit[index], HWND_BOTTOM,  EDIT_POS, FIRST_WINDOW_POS + scrollValue  + (NEXT_POS * index), EDIT_SIZE - moveSize, W_SIZE, SWP_SHOWWINDOW);
	} else if (type == ECTRL_WIN_CMMENT) {
		SetWindowPos(comment[index], HWND_BOTTOM,  EDIT_POS+EDIT_SIZE+moveSize , FIRST_WINDOW_POS + scrollValue  + (NEXT_POS * index) + 8, EDIT_SIZE,W_SIZE, SWP_SHOWWINDOW);
	}
}

size_t getSettingtext (unsigned char index)
{
	size_t length;
	char timeoutText[50];
	char logDetectText[200];
	char typeText[10];
	char reg[5];

	if (subSetting[index].timeType == 0) {
		sprintf_s(typeText, "msec");
	} else if (subSetting[index].timeType == 1) {
		sprintf_s(typeText, "sec");
	} else {
		sprintf_s(typeText, "min");
	}
	if (subSetting[index].timeOut == 0) {
		sprintf_s(timeoutText, " //timeout:None\n");
	} else {
		if (subSetting[index].timeBraPoint == 0xff) {
			sprintf_s(timeoutText, " //timeout:%d%s\n",subSetting[index].timeOut,typeText);
		} else {
			sprintf_s(timeoutText, " //timeout:%d%s_branch:%d\n",subSetting[index].timeOut,typeText,subSetting[index].timeBraPoint);
		}
	}

	if (strlen(subSetting[index].detectEndLog) == 0) {
		sprintf_s(logDetectText, " //lofDetect:None");
	} else {
		sprintf_s(reg, "");
		if (subSetting[index].isRegCheck) {
			sprintf_s(reg, "reg");
		}
		if (subSetting[index].detectLogBraPoint == 0xff) {
			sprintf_s(logDetectText, " //%slogDetect:[%s]",reg, subSetting[index].detectEndLog);
		} else {
			sprintf_s(logDetectText, " //%slogDetect:[%s] branch:%d",reg, subSetting[index].detectEndLog,subSetting[index].detectLogBraPoint);
		}
	}
	
	if (strlen(timeoutText) > strlen(logDetectText)){
		length = strlen(timeoutText);
	} else {
		length = strlen(logDetectText);
	}
	sprintf_s(buf, "%s%s",timeoutText,logDetectText);
	return length;
}
void dispSubSetting(HWND hDlg)
{
	unsigned char i;
	unsigned char subPoint;
	size_t length;

	for (i = 0; i < SUB_SETTING_NUM; i++) {
		DestroyWindow(subSetStaticText[i]);
	}

	for (i = 0; i < ctrlNum; i++) {
		subPoint = getSubSettingPoint(i);
		if ((subPoint != 0xff) && (subSetting[subPoint].commentDisp)) {
			length = getSettingtext(subPoint) * 6;
			changeWindowPos(hDlg, i, ECTRL_WIN_CMMENT, length);
			subSetStaticText[subPoint] = CreateWindow(TEXT("STATIC") , buf, 
				WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOTIFY, 
				EDIT_POS+EDIT_SIZE , FIRST_WINDOW_POS + scrollValue  + (NEXT_POS * i)+1, length,W_SIZE, hDlg, NULL, NULL, NULL);
			SendMessage(subSetStaticText[subPoint], WM_SETFONT, (WPARAM)hSubSetFont, MAKELPARAM(TRUE, 0));
		} else {
			changeWindowPos(hDlg, i, ECTRL_WIN_CMMENT, 0);
		}
	}
}

void lineAddtion(HWND hDlg)
{
	int i;
	LRESULT selNum;

	if (ctrlNum >= CTRL_NUM-1) {
		sprintf_s(buf, "%d個以上は設定できません",CTRL_NUM);
		MessageBox(NULL, buf,TEXT("エラー"), MB_ICONINFORMATION);
	}
	for (i = ctrlNum - 2; i >= exePoint; i--) {
		selNum = SendMessage(combo[i] , CB_GETCURSEL , 0 , 0);
		SendMessage(combo[i+1], CB_SETCURSEL, selNum, 0);
		PostMessage(hDlg , WM_COMMAND , CBN_SELCHANGE<<16 , (LPARAM)combo[i+1]);
		GetWindowText(edit[i], buf, sizeof(buf));
		SendMessage(edit[i+1], WM_SETTEXT, 0, (LPARAM)buf);
		GetWindowText(comment[i], buf, sizeof(buf));
		SendMessage(comment[i+1], WM_SETTEXT, 0, (LPARAM)buf);
	}
	SendMessage(combo[exePoint], CB_SETCURSEL, -1, 0);
	SendMessage(edit[exePoint], WM_SETTEXT, 0, (LPARAM)"");
	SendMessage(comment[exePoint], WM_SETTEXT, 0, (LPARAM)".");
	changeWindowPos(hDlg, exePoint, ECTRL_WIN_EDIT, 0);
	DestroyWindow(button[exePoint]);

	for (i = 0; i < SUB_SETTING_NUM; i++) {
		if ((subSetting[i].id >= exePoint) && (subSetting[i].id != 0xff)) {
			subSetting[i].id++;
		}
		if ((subSetting[i].timeBraPoint >= exePoint) && (subSetting[i].timeBraPoint != 0xff)) {
			subSetting[i].timeBraPoint++;
		}
		if ((subSetting[i].detectLogBraPoint >= exePoint) && (subSetting[i].detectLogBraPoint != 0xff)) {
			subSetting[i].detectLogBraPoint++;
		}
	}
	dispSubSetting(hDlg);
}
void lineDelete(HWND hDlg)
{
	int i;
	LRESULT selNum;
	unsigned char sunPoint;

	if (exePoint < ctrlNum - 1) {
		for (i = exePoint; i < ctrlNum - 1; i++) {
			selNum = SendMessage(combo[i+1] , CB_GETCURSEL , 0 , 0);
			SendMessage(combo[i], CB_SETCURSEL, selNum, 0);
			PostMessage(hDlg , WM_COMMAND , CBN_SELCHANGE<<16 , (LPARAM)combo[i]);
			GetWindowText(edit[i+1], buf, sizeof(buf));
			SendMessage(edit[i], WM_SETTEXT, 0, (LPARAM)buf);
			GetWindowText(comment[i+1], buf, sizeof(buf));
			SendMessage(comment[i], WM_SETTEXT, 0, (LPARAM)buf);
		}
		SendMessage(combo[ctrlNum - 1], CB_SETCURSEL, -1, 0);
		if (ctrlNum > 5) {
			DestroyWindow(staticText[ctrlNum - 1]);
			DestroyWindow(edit[ctrlNum - 1]);
			DestroyWindow(combo[ctrlNum - 1]);
			DestroyWindow(comment[ctrlNum - 1]);
			DestroyWindow(button[ctrlNum - 1]);
			ctrlNum--;
		}
	}

	sunPoint = getSubSettingPoint(exePoint);
	if (sunPoint != 0xff) {
		subSetting[sunPoint].id = 0xff;
	}
	for (i = 0; i < SUB_SETTING_NUM; i++) {
		if ((subSetting[i].id > exePoint) && (subSetting[i].id != 0xff)) {
			subSetting[i].id--;
		}
		if ((subSetting[i].timeBraPoint > exePoint) && (subSetting[i].timeBraPoint != 0xff)) {
			subSetting[i].timeBraPoint--;
		}
		if ((subSetting[i].detectLogBraPoint > exePoint) && (subSetting[i].detectLogBraPoint != 0xff)) {
			subSetting[i].detectLogBraPoint--;
		}
	}
	dispSubSetting(hDlg);
}

void dispClear(HWND hDlg)
{
	unsigned short i;
	
	ctrlNum = INIT_COMB_NUM;
	for (i = 0; i < INIT_COMB_NUM; i++) {
		SendMessage(combo[i] , CB_SETCURSEL , -1 , 0);
		SendMessage(edit[i], WM_SETTEXT, 0, (LPARAM)"");
		SendMessage(comment[i], WM_SETTEXT, 0, (LPARAM)"");
		changeWindowPos(hDlg, i, ECTRL_WIN_EDIT, 0);
		DestroyWindow(button[i]);
	}
	for (i = INIT_COMB_NUM; i < CTRL_NUM; i++) {
		DestroyWindow(staticText[i]);
		DestroyWindow(edit[i]);
		DestroyWindow(combo[i]);
		DestroyWindow(comment[i]);
		DestroyWindow(button[i]);
		RemoveWindowSubclass(combo[i], comboBoxSubclassProc, 0); // サブクラス化解除
	}
	for (i = 0; i < SUB_SETTING_NUM; i++) {
		subSetting[i].id = 0xff;
	}
	dispSubSetting(hDlg);
	ScrollWindow(hDlg, 0, -scrollValue, NULL, NULL);
	SetScrollPos(hDlg, SB_VERT, 0, TRUE);
	scrollValue = 0;
	scrollUpdate(hDlg);
}

void acsMove(HWND hDlg, unsigned otherPoint)
{
	LRESULT selNumNext;
	LRESULT selNumCurrent;
	char textCurrent[256];
	char textNext[256];
	unsigned char subPosCurrent;
	unsigned char subPosNext;

	selNumCurrent = SendMessage(combo[exePoint], CB_GETCURSEL, 0, 0);
	selNumNext = SendMessage(combo[otherPoint], CB_GETCURSEL, 0, 0);
	SendMessage(combo[exePoint], CB_SETCURSEL, selNumNext, 0);
	PostMessage(hDlg, WM_COMMAND, CBN_SELCHANGE << 16, (LPARAM)combo[exePoint]);
	SendMessage(combo[otherPoint], CB_SETCURSEL, selNumCurrent, 0);
	PostMessage(hDlg, WM_COMMAND, CBN_SELCHANGE << 16, (LPARAM)combo[otherPoint]);
	GetWindowText(edit[exePoint], textCurrent, sizeof(textCurrent));
	GetWindowText(edit[otherPoint], textNext, sizeof(textNext));
	SendMessage(edit[exePoint], WM_SETTEXT, 0, (LPARAM)textNext);
	SendMessage(edit[otherPoint], WM_SETTEXT, 0, (LPARAM)textCurrent);
	GetWindowText(comment[exePoint], textCurrent, sizeof(textCurrent));
	GetWindowText(comment[otherPoint], textNext, sizeof(textNext));
	SendMessage(comment[exePoint], WM_SETTEXT, 0, (LPARAM)textNext);
	SendMessage(comment[otherPoint], WM_SETTEXT, 0, (LPARAM)textCurrent);
	subPosCurrent = getSubSettingPoint(exePoint);
	subPosNext = getSubSettingPoint(otherPoint);
	if (subPosCurrent != 0xff) {
		subSetting[subPosCurrent].id = otherPoint;
	}
	if (subPosNext != 0xff) {
		subSetting[subPosNext].id = exePoint;
	}
	dispSubSetting(hDlg);
	exePoint = otherPoint;
	screenUpdate(hDlg);
	InvalidateRect(hDlg, NULL, FALSE);
}

void acsMoveUp(HWND hDlg)
{
	if (exePoint > 0) {
		acsMove(hDlg, exePoint - 1);
	}
}

void acsMoveDown(HWND hDlg)
{
	if (exePoint < ctrlNum - 1) {
		acsMove(hDlg, exePoint + 1);
	}
}

void selectAction(HWND hDlg, unsigned char index)
{
	unsigned char sunPoint;
	LRESULT selNum;

	selNum = SendMessage(combo[index] , CB_GETCURSEL , 0 , 0);
	DestroyWindow(button[index]);
	if ((selNum == EWAIT_LOG) || (selNum == EWAIT_REGLOG)) {
		changeWindowPos(hDlg, index, ECTRL_WIN_EDIT, BUTTON_SIZE);
		createButtonWindow(hDlg, index);
		SetWindowText(button[index], "設定");
	} else if ((selNum == ESAVE_TEXT) || (selNum == ESAVE_TCD) || (selNum == EEXE_FILE) ) {
		changeWindowPos(hDlg, index, ECTRL_WIN_EDIT, BUTTON_SIZE);
		createButtonWindow(hDlg, index);
		SetWindowText(button[index], "参照");
	} else {
		changeWindowPos(hDlg, index, ECTRL_WIN_EDIT, 0);
	}

	if ((selNum != EWAIT_LOG) && (selNum != EWAIT_REGLOG)) {
		sunPoint = getSubSettingPoint(index);
		if (sunPoint != 0xff) {
			subSetting[sunPoint].id = 0xff;
		}
	}
	dispSubSetting(hDlg);

	if (index == (ctrlNum - 1)) {
		if (ctrlNum < CTRL_NUM) {
			createExePosWindow(hDlg, ctrlNum);
			createControlWindow(hDlg, ctrlNum);
			createComboBox(hDlg, ctrlNum);
			ctrlNum++;
			scrollUpdate(hDlg);
		} else {
			sprintf_s(buf, "%d個以上は設定できません",CTRL_NUM);
			MessageBox(NULL, buf,TEXT("エラー"), MB_ICONINFORMATION);
		}
	}
}

void buttonAction(HWND hDlg, unsigned char index)
{
	LRESULT selNum;
	char path[PATH_MAX_LEN];
	char name[STR_MAX_LEN];

	buttonId = index;
	selNum = SendMessage(combo[index] , CB_GETCURSEL , 0 , 0);
	if ((selNum == EWAIT_LOG) || (selNum == EWAIT_REGLOG)) {
		DialogBox(NULL, MAKEINTRESOURCE(IDD_WAIT_SETTINT), hDlg, waitSettingProc);
		dispSubSetting(hDlg);
	} else if (selNum == ESAVE_TEXT) {
		if(setSettingFileName(hDlg, path, name,"log")) {
			SetWindowText(edit[index], path);
		}
	} else if (selNum == ESAVE_TCD) {
		if(setSettingFileName(hDlg, path, name,"tcd")) {
			SetWindowText(edit[index], path);
		}
	} else if (selNum == EEXE_FILE) {
		if(setSettingFileName(hDlg, path, name,"ini")) {
			SetWindowText(edit[index], path);
		}
	} else {
	}
}

void mouseWheel(HWND hDlg, signed long mouse)
{
	static int scrPos;

	mouse /= 1000000;
	if (mouse < 0) {
		if (getExePos(hDlg, ctrlNum - 1) + mouse < getWinMaxPos(hDlg)) {
			if (getExePos(hDlg, ctrlNum - 1) < getWinMaxPos(hDlg)) {
				mouse = 0;
			}
			else {
				mouse = getWinMaxPos(hDlg) - getExePos(hDlg, ctrlNum - 1);
			}
		}
	} else {
		if (getExePos(hDlg, 0) + mouse >= FIRST_WINDOW_POS) {
			mouse = FIRST_WINDOW_POS - getExePos(hDlg, 0);
		}
	}
	scrollValue += mouse;
	scrPos = GetScrollPos(hDlg, SB_VERT);
	scrPos -= mouse;
	SetScrollPos(hDlg, SB_VERT, scrPos, TRUE);
	ScrollWindow(hDlg, 0, mouse, NULL, NULL);
	UpdateWindow(hDlg);
}

unsigned char getSubSettingPoint(unsigned char index)
{
	unsigned char i;
	unsigned char ret = 0xff;

	for (i = 0; i < SUB_SETTING_NUM; i++) {
		if (subSetting[i].id == index) {
			ret = i;
			break;
		}
	}
	return ret;
}

void keepSubSetting(HWND hDlg, unsigned char index)
{
	char *endp;
	SUB_SETTING set = {
		 0xff//id
		,0//timeOut
		,0//timeType
		,0xff//timeBraPoint
		,""//detectEndLog
		,false//isRegCheck
		,0xff//detectLogBraPoint
		,false//commentDisp
	};

	int len;
	unsigned char subSettingPoint;
	unsigned char i;
	long value;
	LRESULT regCheck;

	//タイムアウトによる監視終了設定
	GetWindowText(GetDlgItem(hDlg, IDS_TIMEOUT), buf, sizeof(buf));
	len = strlen(buf);
	if (len != 0) {
		set.timeOut = strtol(buf, &endp, 10);		
		set.id = index;
		set.timeType = (char)SendMessage(GetDlgItem(hDlg, IDS_TIMEOUT_COMB) , CB_GETCURSEL , 0 , 0);
		GetWindowText(GetDlgItem(hDlg, IDS_TIMEOUT_BRANCH), buf, sizeof(buf));
		len = strlen(buf);
		if (len != 0) {
			value = strtol(buf, &endp, 10);
			if (*endp == '\0') {
				set.timeBraPoint = (unsigned char)value;
			}
		}
	}
	//ログ検出による監視終了設定
	GetWindowText(GetDlgItem(hDlg, IDS_DETECT_ENG_LOG), buf, sizeof(buf));
	len = strlen(buf);
	if (len != 0) {
		set.id = index;
		regCheck = SendMessage(GetDlgItem(hDlg, IDS_REG_CHECK) , BM_GETCHECK , 0 , 0);
		set.isRegCheck = (regCheck == 1);
		sprintf_s(set.detectEndLog, buf);
		GetWindowText(GetDlgItem(hDlg, IDS_DETECT_LOG_BRANCH), buf, sizeof(buf));
		len = strlen(buf);
		if (len != 0) {
			value = strtol(buf, &endp, 10);
			if (*endp == '\0') {
				set.detectLogBraPoint = (unsigned char)value;
			}
		}
	}
	set.commentDisp = (SendMessage(GetDlgItem(hDlg, IDA_RES_COMMENT_DISP) , BM_GETCHECK , 0 , 0) == 1 ? true : false);

	subSettingPoint = getSubSettingPoint(index);
	if ((set.id == index) && (subSettingPoint == 0xff)) {
		for (i = 0; i < SUB_SETTING_NUM; i++) {
			if (subSetting[i].id == 0xff) {
				subSettingPoint = i;
				break;
			}
		}
		if (subSettingPoint == 0xff) {
			sprintf_s(buf, "設定できるのは%d個までです", SUB_SETTING_NUM);
			MessageBox(NULL, buf,TEXT("情報"), MB_ICONINFORMATION);
			return;
		}
		
	}
	if (subSettingPoint != 0xff) {
		subSetting[subSettingPoint] = set;
	}
}

LRESULT CALLBACK comboBoxSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	if (uMsg == WM_MOUSEWHEEL) {
		mouseWheel(hDlgAcs, wParam);//親ウィンドウのスクロール
		return 0; // メッセージを無視する
	}
	return DefSubclassProc(hwnd, uMsg, wParam, lParam); // 他のメッセージはデフォルトの処理を行う
}

INT_PTR CALLBACK waitSettingProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND numWh;
	static INT_PTR hbr = (INT_PTR)(CreateSolidBrush(RGB(255,255,255)));
	unsigned char type;
	static unsigned char subSettingId;

	switch (message) {
		case WM_INITDIALOG:
			sprintf_s(buf, "%d", buttonId);
			numWh = CreateWindow(TEXT("STATIC") , TEXT(buf), 
					WS_CHILD | WS_VISIBLE | SS_CENTER |SS_CENTERIMAGE | WS_BORDER,STATIC_POS, FIRST_WINDOW_POS, W_SIZE, W_SIZE, hDlg, NULL, NULL, NULL);
			
			SendMessage(numWh, WM_SETFONT, (WPARAM)hControlFont, MAKELPARAM(TRUE, 0));
			SendMessage(GetDlgItem(hDlg, IDS_TIMEOUT_COMB), CB_ADDSTRING , 0 , (LPARAM)"ミリ秒");
			SendMessage(GetDlgItem(hDlg, IDS_TIMEOUT_COMB), CB_ADDSTRING , 0 , (LPARAM)"秒");
			SendMessage(GetDlgItem(hDlg, IDS_TIMEOUT_COMB), CB_ADDSTRING , 0 , (LPARAM)"分");

			subSettingId = getSubSettingPoint(buttonId);
			if (subSettingId == 0xff) {
				type = 0;
				SendMessage(GetDlgItem(hDlg, IDA_RES_COMMENT_DISP) , BM_SETCHECK , BST_CHECKED , 0);
			} else {
				type = subSetting[subSettingId].timeType;
				if (subSetting[subSettingId].timeOut != 0) {
					sprintf_s(buf, "%d", subSetting[subSettingId].timeOut);
					SetWindowText(GetDlgItem(hDlg, IDS_TIMEOUT), buf);
				}
				if (subSetting[subSettingId].timeBraPoint != 0xff) {
					sprintf_s(buf, "%d", subSetting[subSettingId].timeBraPoint);
					SetWindowText(GetDlgItem(hDlg, IDS_TIMEOUT_BRANCH), buf);
				}
				if (strlen(subSetting[subSettingId].detectEndLog) != 0) {
					sprintf_s(buf, "%s", subSetting[subSettingId].detectEndLog);
					SetWindowText(GetDlgItem(hDlg, IDS_DETECT_ENG_LOG), buf);
				}
				if (subSetting[subSettingId].detectLogBraPoint != 0xff) {
					sprintf_s(buf, "%d", subSetting[subSettingId].detectLogBraPoint);
					SetWindowText(GetDlgItem(hDlg, IDS_DETECT_LOG_BRANCH), buf);
				}
				if (subSetting[subSettingId].isRegCheck) {
					SendMessage(GetDlgItem(hDlg, IDS_REG_CHECK) , BM_SETCHECK , BST_CHECKED , 0);
				} else {
					SendMessage(GetDlgItem(hDlg, IDS_REG_CHECK) , BM_SETCHECK , BST_UNCHECKED , 0);
				}
				if (subSetting[subSettingId].commentDisp) {
					SendMessage(GetDlgItem(hDlg, IDA_RES_COMMENT_DISP) , BM_SETCHECK , BST_CHECKED , 0);
				} else {
					SendMessage(GetDlgItem(hDlg, IDA_RES_COMMENT_DISP) , BM_SETCHECK , BST_UNCHECKED , 0);
				}
			}
			SendMessage(GetDlgItem(hDlg, IDS_TIMEOUT_COMB), CB_SETCURSEL, type, 0);
			break;
		case WM_CTLCOLORSTATIC:
			if ((HWND)lParam == numWh) {
				SetBkMode( (HDC)wParam, TRANSPARENT);
				return hbr;
			}
			break;
		case WM_COMMAND:
			switch (wParam) {
				case IDCANCEL:
					EndDialog(hDlg, LOWORD(wParam));
					break;
				case IDOK:
					keepSubSetting(hDlg, buttonId);
					EndDialog(hDlg, LOWORD(wParam));
					break;
				case IDS_CLEAR:
					SetWindowText(GetDlgItem(hDlg, IDS_TIMEOUT), "");
					SetWindowText(GetDlgItem(hDlg, IDS_TIMEOUT_BRANCH), "");
					SetWindowText(GetDlgItem(hDlg, IDS_DETECT_ENG_LOG), "");
					SetWindowText(GetDlgItem(hDlg, IDS_DETECT_LOG_BRANCH), "");
					SendMessage(GetDlgItem(hDlg, IDS_REG_CHECK) , BM_SETCHECK , BST_UNCHECKED , 0);
					SendMessage(GetDlgItem(hDlg, IDS_TIMEOUT_COMB), CB_SETCURSEL, 0, 0);
					SendMessage(GetDlgItem(hDlg, IDA_RES_COMMENT_DISP) , BM_SETCHECK , BST_CHECKED , 0);
					break;
				default:
					break;
			}
		default:
			break;
	}

	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK autoCommandSenderProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	unsigned char i;
	INT_PTR hbr;
	static INT_PTR hbrBkgnd[4];
	static HDROP hDrop;
	HMENU hmenu;
	HMENU hSubmenu;
	POINT pt;
	UINT uState;
	static int scrPos;
	static int count = 0;

	switch (message) {
		case WM_INITDIALOG:
			DragAcceptFiles(hDlg, TRUE);		//ドロップ可否指定[True:Drop許可]
			createWindowFont();
			for (i = 0; i < ctrlNum; i++) {
				createExePosWindow(hDlg, i);
				createControlWindow(hDlg, i);
				createComboBox(hDlg, i);
			}
			setLogToolSettingPath(hDlg);
			readLogToolIni(hDlg);
			scrollUpdate(hDlg);
			InvalidateRect(hDlg, NULL, FALSE);
			UpdateWindow(hDlg);
			hbrBkgnd[EBRUSH_COLOR_GREEN] = (INT_PTR)(CreateSolidBrush(RGB(0, 255, 0)));
			hbrBkgnd[EBRUSH_COLOR_GRAY_L] = (INT_PTR)(CreateSolidBrush(RGB(200,200,200)));
			hbrBkgnd[EBRUSH_COLOR_WHITE] = (INT_PTR)(CreateSolidBrush(RGB(255,255,255)));
			hbrBkgnd[EBRUSH_COLOR_GRAY_D] = (INT_PTR)(CreateSolidBrush(RGB(240,240,240)));

			GetPrivateProfileString(_T("AUTO_START"), _T("AUTO"), _T("OFF"), buf, sizeof(buf), autoCommandSenderSettingPath);
			GetPrivateProfileString(_T("AUTO_COMMAND_SEND"), _T("AUTO_START"), buf, buf, sizeof(buf), szIniFilePrivate);
			
			if (strncmp(buf, "ON", 2) == 0) {
				hmenu = GetMenu(hDlg);
				CheckMenuItem(hmenu, ID_AUTO_START, MF_BYCOMMAND | MFS_CHECKED);
				PostMessage((HWND)hDlg, WM_COMMAND, ID_EXECUTE, 0);
			}
			break;
		case WM_SIZE:
			scrollUpdate(hDlg);
			break;
		case WM_TIMER:
			if (wParam == TIMER_ID_WAIT_TIME) {
				if (isStepExe && (subScenario == 0)) {
					isExe = false;
					isStepExe = false;
					comBusyFlag = false;
				}
				KillTimer(hDlg, TIMER_ID_WAIT_TIME);
				exePoint = nextPoint;
				if (isExe) {
					execute(hDlg);
				}
				InvalidateRect(hDlg, NULL, FALSE);
			} else if (wParam == TIMER_ID_LOG_TIME_OUT) {
				KillTimer(hDlg, TIMER_ID_LOG_TIME_OUT);
				detectType = ELOG_DETECT_NULL;
				exePoint = nextBraPoint;
				execute(hDlg);
			} else {
			}
			break;
		case WM_COMMAND:
			switch (wParam) {
				case IDCANCEL:
					stop(hDlg);
					exePoint = 0;
					ShowWindow(hDlg, SW_HIDE);
					break;
				case ID_STOP:
					stop(hDlg);
					break;
				case ID_EXECUTE:
					if (!isExe) {
						isExe = true;
						keepInAction();
						execute(hDlg);
					}
					break;
				case ID_PAUSE:
					pause(hDlg);
					break;
				case ID_CLEAR:
					dispClear(hDlg);
					break;
				case ID_ADDTION:
					lineAddtion(hDlg);
					break;
				case ID_DELETE:
					lineDelete(hDlg);
					break;
				case ID_MOVE_UP:
					acsMoveUp(hDlg);
					break;
				case ID_MOVE_DOWN:
					acsMoveDown(hDlg);
					break;
				case OTHER_NAME_SAVE:
					if (setSettingFileName(hDlg, autoCommandSenderSettingPath, autoCommandSenderFileName, "ini")) {
						WritePrivateProfileString(_T("PATH"), _T("ACS_SETTING_FILE"), autoCommandSenderSettingPath, logToolSettingPath);
						exePoint = 0;
						writeLogToolIni(hDlg);
					}
					break;
				case OVERWRITE_SAVE:
					exePoint = 0;
					writeLogToolIni(hDlg);
					break;
				case ID_FILE_READ:
					if (setSettingFileName(hDlg, autoCommandSenderSettingPath, autoCommandSenderFileName, "ini")) {
						WritePrivateProfileString(_T("PATH"), _T("ACS_SETTING_FILE"), autoCommandSenderSettingPath, logToolSettingPath);
						dispClear(hDlg);
						readLogToolIni(hDlg);
						InvalidateRect(hDlg, NULL, FALSE);
					}
					break;
				case ID_HIDDEN:
					ShowWindow(hDlg, SW_HIDE);
					break;
				case ID_AUTO_START:
					hmenu = GetMenu(hDlg);
					uState = GetMenuState(hmenu, ID_AUTO_START, MF_BYCOMMAND);
					if (uState & MFS_CHECKED) {
						CheckMenuItem(hmenu, ID_AUTO_START, MF_BYCOMMAND | MFS_UNCHECKED);
						WritePrivateProfileString(_T("AUTO_START"), _T("AUTO"), "OFF", autoCommandSenderSettingPath);
					} else {
						CheckMenuItem(hmenu, ID_AUTO_START, MF_BYCOMMAND | MFS_CHECKED);
						WritePrivateProfileString(_T("AUTO_START"), _T("AUTO"), "ON", autoCommandSenderSettingPath);
					}
					break;
				default:
					break;
			}
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				for (i = 0; i < ctrlNum; i++) {
					if (lParam == (LPARAM)combo[i]) {
						selectAction(hDlg, i);
					}
				}
			}
			for (i = 0; i < ctrlNum; i++) {
				if ((lParam == (LPARAM)button[i]) && (button[i] != NULL)) {
					LONG lStyle = GetWindowLong(button[i], GWL_STYLE); // ウィンドウのスタイルを取得
					if ((lStyle & BS_PUSHBUTTON) == BS_PUSHBUTTON) {
						buttonAction(hDlg, i);
					}
				}
			}
			for (i = 0; i < ctrlNum; i++) {
				if ((lParam == (LPARAM)staticText[i]) ) {
					if (isExe == false) {
						if (HIWORD(wParam) == STN_CLICKED) {
							exePoint = i;
							screenUpdate(hDlg);
							InvalidateRect(hDlg, NULL, FALSE);
						} else if (HIWORD(wParam) == STN_DBLCLK) {
							isExe = true;
							isStepExe = true;
							exePoint = i;
							execute(hDlg);
						}
					}
					break;
				}
			}
			break;
		case WM_CTLCOLOREDIT:
			for (i = 0; i < ctrlNum; i++) {
				if (lParam == (LPARAM)comment[i]) {
					SetBkMode( (HDC)wParam, TRANSPARENT);
					return hbrBkgnd[EBRUSH_COLOR_GRAY_D];
				}
			}
			break;
		case WM_VSCROLL:
			vScroll(hDlg, wParam);
			InvalidateRect(hDlg, NULL, FALSE);
			break;
		case WM_CTLCOLORSTATIC:
			SetBkMode( (HDC)wParam, TRANSPARENT);
			if ((isExe == true) && (lParam == (LPARAM)staticText[exePoint])) {
				hbr = hbrBkgnd[EBRUSH_COLOR_GREEN];
			} else {
				if (lParam == (LPARAM)staticText[exePoint]) {
					hbr = hbrBkgnd[EBRUSH_COLOR_GRAY_L];
				} else {
					hbr = hbrBkgnd[EBRUSH_COLOR_WHITE];
				}
			}
			for (i = 0; i < SUB_SETTING_NUM; i++) {
				if (lParam == (LPARAM)subSetStaticText[i]) {
					hbr = hbrBkgnd[EBRUSH_COLOR_GRAY_D];
				}
			}
			return hbr; //returnしたBrushを使用して処理をしてくれるうようになっている
		case WM_MOUSEWHEEL:
			mouseWheel(hDlg, wParam);
			break;
		case WM_DROPFILES:
			dispClear(hDlg);
			hDrop = (HDROP)wParam;
			DragQueryFile(hDrop, 0, (PTSTR)buf, 200);
			readMacro(hDlg, hDrop);
			DragFinish(hDrop);
			break;
		case WM_RBUTTONDOWN:
			hmenu = GetMenu(hDlg);
			pt.x = LOWORD(lParam);
			pt.y = HIWORD(lParam);
			hSubmenu = GetSubMenu(hmenu, 1);
			ClientToScreen(hDlg, &pt);
			TrackPopupMenu(hSubmenu , TPM_LEFTALIGN | TPM_BOTTOMALIGN, pt.x, pt.y, 0 , hDlg , NULL);
			break;
		case WM_DESTROY:
			break;
		default:
			break;
	}
	return (INT_PTR)FALSE;
}

void readMacro(HWND hDlg, HDROP hDro)
{
	FILE *fp;
	char p[256];
	bool isEnableCommand;
	char *ptr;
	char *ctx;//内部的に使用するので深く考えない
	unsigned char ctrlPoint = 0;
	bool warning = false;

	fopen_s(&fp, buf, "r");
	while (fgets(p, sizeof(p), fp) != NULL) {
		isEnableCommand = false;
		if (ctrlPoint >= CTRL_NUM) {
			break;
		} else {
			if (strncmp(p, "sendln", 6) == 0) {
				ptr = strtok_s(p, "'", &ctx);
				ptr = strtok_s(NULL, "'", &ctx);
				SendMessage(edit[ctrlPoint], WM_SETTEXT, 0, (LPARAM)ptr);
				SendMessage(combo[ctrlPoint] , CB_SETCURSEL , ESEND_CMD , 0);
				SendMessage(hDlg , WM_COMMAND , CBN_SELCHANGE<<16 , (LPARAM)combo[ctrlPoint]);
				isEnableCommand = true;
			} else if (strncmp(p, "mpause", 6) == 0) {
				ptr = strtok_s(p, " ", &ctx);
				ptr = strtok_s(NULL, " ", &ctx);
				SendMessage(combo[ctrlPoint] , CB_SETCURSEL , EWAIT_TIME , 0);
				SendMessage(edit[ctrlPoint], WM_SETTEXT, 0, (LPARAM)ptr);
				SendMessage(hDlg , WM_COMMAND , CBN_SELCHANGE<<16 , (LPARAM)combo[ctrlPoint]);
				isEnableCommand = true;
			} else if (strncmp(p, "messagebox", 6) == 0) {
				ptr = strtok_s(p, "'", &ctx);
				ptr = strtok_s(NULL, "'", &ctx);
				SendMessage(edit[ctrlPoint], WM_SETTEXT, 0, (LPARAM)ptr);
				SendMessage(combo[ctrlPoint] , CB_SETCURSEL , EDISP_MSG , 0);
				SendMessage(hDlg , WM_COMMAND , CBN_SELCHANGE<<16 , (LPARAM)combo[ctrlPoint]);
				isEnableCommand = true;
			} else {
				char* context; //strtok_sの内部で使用
				char* token;
				token = strtok_s(p, " ", &context);
				if ((token == NULL) || (strncmp(token, "\n", 1) == 0) ) {
				} else {
					token = strtok_s(p, "　", &context);
					if ((token == NULL) || (strncmp(token, "\n", 1) == 0) ) {
					} else {
						warning = true;
					}
				}
			}
		}
		if (isEnableCommand) {
			ctrlPoint++;
		}
	}
	if (warning) {
		MessageBox(NULL, TEXT("[sendln][mpause][messagebox]以外は無視します"),TEXT("情報"), MB_ICONINFORMATION);
	}
	fclose(fp);
}

void execute(HWND hDlg)
{
	HWND hId;
	LRESULT selNum;

	screenUpdate(hDlg);

	comBusyFlag = true;

	hId = combo[exePoint];
	selNum = SendMessage(hId , CB_GETCURSEL , 0 , 0);	
	switch (selNum) {
		case ESEND_CMD:
			sendCommand(hDlg);
			break;
		case EWAIT_TIME:
			waitTime(hDlg);
			break;
		case EJMP_POINT:
			jumpPoint(hDlg);
			break;
		case EWAIT_LOG:
			startLogMonitor(hDlg);
			break;
		case EWAIT_REGLOG:
			startLogMonitorRegex(hDlg);
			break;
		case ESAVE_TEXT:
			saveTextlog(hDlg);
			break;
		case ESAVE_TCD:
			saveTcdlog(hDlg);
			break;
		case EDISP_MSG:
			outputMessage(hDlg);
			break;
		case EEXE_FILE:
			exeReadFile(hDlg);
			break;
		case ELOG_CLEAR:
			logClear(hDlg);
			break;
		case EOUTPUT_DATA:
			outPutData(hDlg);
			break;
		case ECALCULATION:
			calculation(hDlg);
			break;
		case EBRANCH:
			branch(hDlg);
			break;
		case EEXTERNAL_TOOL:
			externalTool(hDlg);
			break;
		case EWAIT_END:
		default:
			end(hDlg);
			break;
	}

	if ((selNum == EWAIT_LOG) || (selNum == EWAIT_REGLOG) || (selNum == EWAIT_TIME) || (selNum == EWAIT_END)) {
		comBusyFlag = false;
	}
}

void vScroll(HWND hDlg, WPARAM wp)
{
	static int prePos = 0;
	int scrPos = GetScrollPos(hDlg, SB_VERT);
	int movePos = HIWORD(wp);
	int move;

	switch (LOWORD(wp)) {;
		case SB_LINEUP:
			if (getExePos(hDlg, 0) < FIRST_WINDOW_POS) {
				scrollValue++;
				scrPos--;
				SetScrollPos(hDlg, SB_VERT, scrPos, TRUE);
				ScrollWindow(hDlg, 0, 1, NULL, NULL);
			}
			break;
		case SB_LINEDOWN:
			if (getExePos(hDlg, ctrlNum - 1) > getWinMaxPos(hDlg)) {
				scrollValue--;
				scrPos++;
				SetScrollPos(hDlg, SB_VERT, scrPos, TRUE);
				ScrollWindow(hDlg, 0, -1, NULL, NULL);
			}
			break;
		case SB_PAGEUP:
			move = scr.nPage;
			if (getExePos(hDlg, 0) + move >= FIRST_WINDOW_POS) {
				move = FIRST_WINDOW_POS - getExePos(hDlg, 0);
			}
			scrollValue += move;
			scrPos -= move;
			SetScrollPos(hDlg, SB_VERT, scrPos, TRUE);
			ScrollWindow(hDlg, 0, move, NULL, NULL);
			break;
		case SB_PAGEDOWN:
			move =  (0 - scr.nPage);
			if (getExePos(hDlg, ctrlNum - 1) + move < getWinMaxPos(hDlg)) {
				move = getWinMaxPos(hDlg) - getExePos(hDlg, ctrlNum - 1);
			}
			scrollValue += move;
			scrPos -= move;
			SetScrollPos(hDlg, SB_VERT, scrPos, TRUE);
			ScrollWindow(hDlg, 0, move, NULL, NULL);
			break;
		case SB_THUMBPOSITION:
			scrPos = HIWORD(wp);
			SetScrollPos(hDlg, SB_VERT, scrPos, TRUE);
			break;
		case SB_THUMBTRACK:
			move = prePos - movePos;
			if (move < 0) {
				if (getExePos(hDlg, ctrlNum - 1) + move < getWinMaxPos(hDlg)) {
					move = getWinMaxPos(hDlg) - getExePos(hDlg, ctrlNum - 1);
				}
			} else {
				if (getExePos(hDlg, 0) + move >= FIRST_WINDOW_POS) {
					move = FIRST_WINDOW_POS - getExePos(hDlg, 0);
				}
			}
			ScrollWindow(hDlg, 0, move, NULL, NULL);
			scrPos = move;
			scrollValue += move;
			prePos = movePos;
			break;
		default:
			break;
	}
}

int getExePos(HWND hDlg, unsigned short num) 
{
	WINDOWINFO windowInfo;
	POINT winPos;

	GetWindowInfo(combo[num], &windowInfo);
	winPos.y = windowInfo.rcWindow.top;
	ScreenToClient(hDlg, &winPos);
	return winPos.y;
}

int getWinMaxPos(HWND hDlg)
{
	WINDOWINFO windowInfo;
	POINT winPos;

	GetWindowInfo(hDlg, &windowInfo);
	winPos.y = windowInfo.rcWindow.bottom;
	ScreenToClient(hDlg, &winPos);
	return winPos.y - 9 - 30;
}

void screenUpdate(HWND hDlg)
{
	int moveValue = 0;
	int exePos = getExePos(hDlg, exePoint);
	int winMaxPos = getWinMaxPos(hDlg);
	int scrPos;

	//実行Pointが画面上に消えそうな場合（座標10未満）上にスクロール
	if (exePos < 10) {
		moveValue = (10 - exePos);
		scrollValue += moveValue;
		ScrollWindow(hDlg, 0, 10 - exePos, NULL, NULL);
		scrPos = GetScrollPos(hDlg, SB_VERT);
		scrPos -= moveValue;
		SetScrollPos(hDlg, SB_VERT, scrPos, TRUE);
	}
	//実行Pointが画面下に消えそうな場合下にスクロール
	if (exePos >= winMaxPos) {
		moveValue = (winMaxPos - exePos);
		scrollValue += moveValue;
		ScrollWindow(hDlg, 0, moveValue, NULL, NULL);
		scrPos = GetScrollPos(hDlg, SB_VERT);
		scrPos -= moveValue;
		SetScrollPos(hDlg, SB_VERT, scrPos, TRUE);
	}

	//画面更新「WM_CTLCOLORSTATIC」メッセージが呼ばれ、実行中pointは緑に光る
	InvalidateRect(hDlg, NULL, FALSE);	
	UpdateWindow(hDlg);
}

void scrollUpdate(HWND hDlg)
{
	unsigned long wLength;
	unsigned long ctrlLength;
	WINDOWINFO windowInfo;

	int pos = GetScrollPos(hDlg, SB_VERT);
	int max = scr.nMax - scr.nPage + 1;
	int perCent = 0;
	if (max !=  0) {
		perCent = pos * 1000 / max;
	}

	GetWindowInfo(hDlg, &windowInfo);
	wLength = getWinMaxPos(hDlg);	
	ctrlLength = FIRST_WINDOW_POS + NEXT_POS * ctrlNum;
	scr.cbSize = sizeof(SCROLLINFO);
	scr.fMask = SIF_PAGE | SIF_RANGE |SIF_POS;
	scr.nMin = 0;
	scr.nMax = ctrlLength;
	scr.nPage = wLength;
	
	if (pos != 0) {
		scr.nPos = (scr.nMax - scr.nPage + 1) * perCent / 1000;
	} else {
		scr.nPos = 0;
	}
	SetScrollInfo(hDlg, SB_VERT, &scr, TRUE);
}

void externalStart(HWND hDlg)
{
	dispClear(hDlg);
	setLogToolSettingPath(hDlg);
	readLogToolIni(hDlg);
	SendMessage(hDlg, WM_COMMAND, ID_EXECUTE, 0);
}

void externalStop(HWND hDlg)
{
	SendMessage(hDlg, WM_COMMAND, ID_STOP, 0);
}

void sendCommand(HWND hDlg)
{
	GetWindowText(edit[exePoint], buf, sizeof(buf));
	changeParamValue(buf, INTEGER_VALUE);//文字列の中の数値を変換
	sendText(buf);
	nextPoint = exePoint + 1;
	SetTimer(hDlg, TIMER_ID_WAIT_TIME, 10, NULL);
}
void waitTime(HWND hDlg)
{
	unsigned short time;

	GetWindowText(edit[exePoint], buf, sizeof(buf));
	changeParamValue(buf, INTEGER_VALUE);//文字列の中の数値を変換
	time = atoi(buf);
	nextPoint = exePoint + 1;
	SetTimer(hDlg, TIMER_ID_WAIT_TIME, time, NULL);
}
void jumpPoint(HWND hDlg)
{
	GetWindowText(edit[exePoint], buf, sizeof(buf));
	changeParamValue(buf, INTEGER_VALUE);//文字列の中の数値を変換
	if (atoi(buf) < ctrlNum) {
		nextPoint = atoi(buf);
		SetTimer(hDlg, TIMER_ID_WAIT_TIME, 1, NULL);
	} else {
		MessageBox(NULL, TEXT("そのPointには移動できません"),TEXT("Pointエラー"), MB_ICONINFORMATION);
	}
}

void stop(HWND hDlg)
{
	if (subScenario > 0) {
		sprintf_s(autoCommandSenderSettingPath, "%s1.ini",szGlPath);
		dispClear(hDlg);
		readLogToolIni(hDlg);
		for (unsigned char i = 1; i <= subScenario; i++) {
			sprintf_s(autoCommandSenderSettingPath, "%s%d.ini",szGlPath, i);
			remove(autoCommandSenderSettingPath);
		}
		subScenario = 0;
	}
	isExe = false;
	comBusyFlag = false;
	detectType = ELOG_DETECT_NULL;
	nextPoint = 0;
	KillTimer(hDlg, TIMER_ID_WAIT_TIME);
	SetTimer(hDlg, TIMER_ID_WAIT_TIME, 50, NULL);
	setLogToolSettingPath(hDlg);
	keepInAction();
}

void end(HWND hDlg)
{
	if (subScenario > 0) {
		sprintf_s(autoCommandSenderSettingPath, "%s%d.ini",szGlPath,subScenario);
		dispClear(hDlg);
		readLogToolIni(hDlg);
		remove(autoCommandSenderSettingPath);
		nextPoint = exePoint + 1;
		KillTimer(hDlg, TIMER_ID_WAIT_TIME);
		SetTimer(hDlg, TIMER_ID_WAIT_TIME, 50, NULL);
		subScenario--;
	} else {
		isExe = false;
		comBusyFlag = false;
		detectType = ELOG_DETECT_NULL;
		nextPoint = exePoint;
		KillTimer(hDlg, TIMER_ID_WAIT_TIME);
		SetTimer(hDlg, TIMER_ID_WAIT_TIME, 50, NULL);
		setLogToolSettingPath(hDlg);
		keepInAction();
	}
}

void pause(HWND hDlg)
{
	isExe = false;
	detectType = ELOG_DETECT_NULL;
	nextPoint = exePoint;
	KillTimer(hDlg, TIMER_ID_WAIT_TIME);
	SetTimer(hDlg, TIMER_ID_WAIT_TIME, 50, NULL);
}

void startLogMonitor(HWND hDlg)
{
	GetWindowText(edit[exePoint], buf, sizeof(buf));
	hMyDlg = hDlg;
	detectType = ELOG_DETECT_NORMAL;
	if (strlen(buf) > STR_MAX_LEN) {
		MessageBox(NULL, TEXT("文字が長すぎます"),TEXT("エラー"), MB_ICONWARNING);
		return;
	}
	strcpy_s(waitLog, buf);

	timeOutSeting(hDlg);

}
void startLogMonitorRegex(HWND hDlg)
{
	GetWindowText(edit[exePoint], buf, sizeof(buf));
	hMyDlg = hDlg;
	detectType = ELOG_DETECT_REGEX;
	if (strlen(buf) > STR_MAX_LEN) {
		MessageBox(NULL, TEXT("文字が長すぎます"),TEXT("エラー"), MB_ICONWARNING);
		return;
	}
	strcpy_s(waitLog, buf);

	timeOutSeting(hDlg);
}

void timeOutSeting(HWND hDlg)
{
	unsigned char subSet;
	unsigned long time;
	subSet = getSubSettingPoint(exePoint);
	if (subSet != 0xff) {
		if (subSetting[subSet].timeOut != 0) {
			nextBraPoint = subSetting[subSet].timeBraPoint;
			if (nextBraPoint == 0xff) {
				nextBraPoint = exePoint + 1;
			}
			if (subSetting[subSet].timeType == 1) {
				time = subSetting[subSet].timeOut * 1000;
			} else if (subSetting[subSet].timeType == 2) {
				time = subSetting[subSet].timeOut * 1000 * 60;
			} else{
				time = subSetting[subSet].timeOut;
			}
			SetTimer(hDlg, TIMER_ID_LOG_TIME_OUT, time, NULL);
		}
	}
}

void LogMonitor(char *str, unsigned long time, double *param, unsigned char dataNum)
{
	if (detectType == ELOG_DETECT_NORMAL) {
		LogMonitorNormal(str, time, param, dataNum);
	} else if (detectType == ELOG_DETECT_REGEX){
		LogMonitorRegex(str, time, param, dataNum);
	} else {
	}

	if (detectType != ELOG_DETECT_NULL) {
		detectEndLogCheck(str, time, param, dataNum);
	}
}

void LogMonitorNormal(char *str, unsigned long time, double *param, unsigned char dataNum)
{
	char* readline;

	readline = strstr(str, waitLog);
	if(readline != NULL) {
		KillTimer(hMyDlg, TIMER_ID_LOG_TIME_OUT);
		SetTimer(hMyDlg, TIMER_ID_WAIT_TIME, 1, NULL);
		nextPoint = exePoint + 1;
		detectType = ELOG_DETECT_NULL;
		keepData(time, param, dataNum);
		comBusyFlag = true;
	}
}

void LogMonitorRegex(char *str, unsigned long time, double *param, unsigned char dataNum)
{
	std::regex regex(waitLog);
	if (std::regex_search(str, regex)) {
		KillTimer(hMyDlg, TIMER_ID_LOG_TIME_OUT);
		SetTimer(hMyDlg, TIMER_ID_WAIT_TIME, 1, NULL);
		nextPoint = exePoint + 1;
		detectType = ELOG_DETECT_NULL;
		keepData(time, param, dataNum);
		comBusyFlag = true;
	} 
}
void detectEndLogCheck(char *str, unsigned long time, double *param, unsigned char dataNum)
{
	unsigned char subSet;
	subSet = getSubSettingPoint(exePoint);
	if (subSet != 0xff) {
		if (strlen(subSetting[subSet].detectEndLog) != 0) {
			if (subSetting[subSet].isRegCheck) {
				std::regex regex(subSetting[subSet].detectEndLog);
				if (std::regex_search(str, regex)) {
					nextBraPoint = subSetting[subSet].detectLogBraPoint;
					if (nextBraPoint == 0xff) {
						nextBraPoint = exePoint + 1;
					}
					SetTimer(hMyDlg, TIMER_ID_LOG_TIME_OUT, 1, NULL);
					detectType = ELOG_DETECT_NULL;
					keepData(time, param, dataNum);
					comBusyFlag = true;
				} 
			} else {
				char* readline;
				readline = strstr(str, subSetting[subSet].detectEndLog);
				if(readline != NULL) {
					nextBraPoint = subSetting[subSet].detectLogBraPoint;
					if (nextBraPoint == 0xff) {
						nextBraPoint = exePoint + 1;
					}
					SetTimer(hMyDlg, TIMER_ID_LOG_TIME_OUT, 1, NULL);
					detectType = ELOG_DETECT_NULL;
					keepData(time, param, dataNum);
					comBusyFlag = true;
				}
			}
		}
	}
}

void saveTextlog(HWND hDlg)
{
	char fileName[STR_MAX_LEN] = "log.txt";

	SetCurrentDirectory(szGlPath);
	GetWindowText(edit[exePoint], fileName, sizeof(fileName));
	changeParamValue(fileName, INTEGER_VALUE);//文字列の中の数値を変換
	if (PathIsRelative(fileName)) {
		sprintf_s(buf, "%s%s", szGlPath, fileName);
	} else {
		strcpy_s(buf, fileName);
	}
	LogSave(hSyslogDlgWnd, buf);
	nextPoint = exePoint + 1;
	SetTimer(hDlg, TIMER_ID_WAIT_TIME, 50, NULL);
}

void saveTcdlog(HWND hDlg)
{
	char fileName[STR_MAX_LEN] = "log.tcd";

	SetCurrentDirectory(szGlPath);
	GetWindowText(edit[exePoint], fileName, sizeof(fileName));
	changeParamValue(fileName, INTEGER_VALUE);//文字列の中の数値を変換
	if (PathIsRelative(fileName)) {
		sprintf_s(buf, "%s%s", szGlPath, fileName);
	} else {
		strcpy_s(buf, fileName);
	}
	tcmSave(hSyslogDlgWnd, buf);
	nextPoint = exePoint + 1;
	SetTimer(hDlg, TIMER_ID_WAIT_TIME, 50, NULL);
}

void outputMessage(HWND hDlg)
{
	GetWindowText(edit[exePoint], buf, sizeof(buf));
	changeParamValue(buf, FLOAT_VALUE);//文字列の中の数値を変換
	MessageBox(NULL, TEXT(buf),TEXT("情報"), MB_ICONINFORMATION);
	nextPoint = exePoint + 1;
	SetTimer(hDlg, TIMER_ID_WAIT_TIME, 1, NULL);
}

void exeReadFile(HWND hDlg)
{
	subScenario++;
	sprintf_s(autoCommandSenderSettingPath, "%s%d.ini",szGlPath,subScenario);
	writeLogToolIni(hDlg);
	GetWindowText(edit[exePoint], buf, sizeof(buf));
	WritePrivateProfileString(_T("AUTO_CMMMACD_SEND"), _T("FILE_NAME"), autoCommandSenderFileName, autoCommandSenderSettingPath);
	if(PathIsRelative(buf)) {
		sprintf_s(autoCommandSenderSettingPath, "%s%s%s",szGlDrive,szGlPath,buf);
	} else {
		strcpy_s(autoCommandSenderSettingPath, buf);
	}
	dispClear(hDlg);
	readLogToolIni(hDlg);
	nextPoint = exePoint;
	SetTimer(hDlg, TIMER_ID_WAIT_TIME, 50, NULL);
}

void logClear(HWND hDlg)
{
	SendMessage(hSyslogDlgWnd, WM_COMMAND, IDOK, 0);
	nextPoint = exePoint + 1;
	SetTimer(hDlg, TIMER_ID_WAIT_TIME, 50, NULL);
}

void outPutData(HWND hDlg)
{
	static FILE *logfileP;
	char fileName[STR_MAX_LEN] = "logFile.csv";
	char logFilepath[PATH_MAX_LEN];
	char* paramStr;
	char logStr[STR_MAX_LEN];
	char logDat[STR_MAX_LEN];
	char* context; //strtok_sの内部で使用

	GetWindowText(edit[exePoint], buf, sizeof(buf));

	strcpy_s(logStr, buf);
	paramStr = strtok_s(buf, ",", &context);
	if (paramStr) {
		strcpy_s(logStr, paramStr);
		paramStr = strtok_s(NULL, ",", &context);
		if (paramStr) {
			strcpy_s(fileName, paramStr);
		}
	}
	
	if(PathIsRelative(fileName)) {
		sprintf_s(logFilepath, "%s%s",szGlPath,fileName);
	} else {
		strcpy_s(logFilepath, fileName);
	}
	
	fopen_s(&logfileP, logFilepath, "a");
	paramStr = strtok_s(logStr, " ", &context);
	while (paramStr != NULL) {
		strcpy_s(logDat, paramStr);
		changeParamValue(logDat, FLOAT_VALUE);//文字列の中の数値を変換
		fprintf(logfileP,"%s,",logDat);
		paramStr = strtok_s(NULL, " ", &context);
	}
	fprintf(logfileP,"\n");
	fclose(logfileP);
	nextPoint = exePoint + 1;
	SetTimer(hDlg, TIMER_ID_WAIT_TIME, 1, NULL);
}

void calculation(HWND hDlg)
{
	GetWindowText(edit[exePoint], buf, sizeof(buf));
	calculateData(buf);
	nextPoint = exePoint + 1;
	SetTimer(hDlg, TIMER_ID_WAIT_TIME, 1, NULL);
}

void keepData(unsigned long keepTime, double *param, unsigned char dataNum)
{
	char keepParamFormula[100];
	unsigned char i;
	sprintf_s(keepParamFormula, "[time]=%d",keepTime);
	calculateData(keepParamFormula);

	for (i = 0; i < dataNum; i++) {
		sprintf_s(keepParamFormula, "[param%d]=%lf", i+1, param[i]);
		calculateData(keepParamFormula);
	}
}

void branch(HWND hDlg)
{
	GetWindowText(edit[exePoint], buf, sizeof(buf));

	if (judgement(buf)) {
		nextPoint = atoi(buf);
	} else {
		nextPoint = exePoint + 1;
	}
	SetTimer(hDlg, TIMER_ID_WAIT_TIME, 1, NULL);
}

void externalTool(HWND hDlg)
{
	STARTUPINFO si = {};
	PROCESS_INFORMATION pi = {};
	char toolFile[PATH_MAX_LEN];
	
	GetWindowText(edit[exePoint], buf, sizeof(buf));
	changeParamValue(buf, INTEGER_VALUE);//文字列の中の数値を変換
	if(PathIsRelative(buf)) {
		sprintf_s(toolFile, "%s%s",szGlPath,buf);
	} else {
		strcpy_s(toolFile, buf);
	}
	CreateProcess(NULL, (LPSTR)toolFile, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	nextPoint = exePoint + 1;
	SetTimer(hDlg, TIMER_ID_WAIT_TIME, 1, NULL);
}

