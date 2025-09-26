#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <windows.h>
#include <ShlObj.h>
#include <time.h>

#include "resource.h"
#include "LogTool.h"
#include "saveTcdFile.h"
#include "VisualAnalyzer.h"

#define LOG_KEEP_NUM 50
#define LOG_BUF_MARGIN 50
#define TCM_TYPE_TIME 4
#define TCM_TYPE_ETC 8


enum eCONNECT {
	 ECONNECT_IDLE
	,ECONNECT_CONNECT
	,ECONNECT_RETRY
};

extern HWND hSyslogDlgWnd;
extern HWND hDlgSearch;
extern char line_mbuf[2048];
extern char syslog_file[_T_MAX_PATH];
extern _TCHAR szIniFilePrivate[_T_MAX_DIR];
extern char szIniFilePublic[_T_MAX_PATH];		// Iniファイルパス
extern char logToolSettingPath[_T_MAX_PATH];	// logTool設定ファイルのパス
extern char captionName[_T_MAX_PATH];
extern BOOL log_group_disp[];
extern char defLogLvl;
extern char tcmLogGroupNo;
extern bool comBusyFlag;
extern volatile bool isTimeLogUpdate;
 
void createLog (unsigned char buf);
void createAsciiLog (unsigned char buf);
void paramConversion (unsigned char argNum, unsigned char argType, unsigned char *log, double *param);
extern int createLogMessage(FILE*, unsigned long, short, char, char, char, char, char*, unsigned char*);// ログメッセージ生成
extern char* get_msgtbl(char group_id, int index, char arg_num);
extern void eraseEnd();
extern void receiveFixLog(char *buffer);
extern void LogMonitor(char *str, unsigned long time, double *param, unsigned char dataNum);
extern void tcmLogInfo(unsigned long timeCount, unsigned short id, unsigned short onOff, unsigned long value);
extern void visualAnalyzeLogInfo(char* form_msg);
extern void visualAnalyzeLogWait(char* form_msg);
extern bool updateVisualAnalyzeTime(unsigned long time);
extern bool getVisualLogDisp();
extern eLOG_READ_STATE getVaState();

FILE *fileP;
HANDLE mHComPort;
HANDLE mThreadId;
DCB dcb;
BOOL nBret;
int dstSocket;
HWND hComDlgWnd;

static TCHAR port[10];
static char logKeepBufmbuf[LOG_KEEP_NUM + LOG_BUF_MARGIN][1024];// 表示中に追加ログを受信してもいいようにバッファは+10持っておく
static char logKeepCount = 0;
static unsigned long baudRate = 38400;

char dispLine[1024];
bool isEngineReal = true;
bool isTeratermUse = false;
unsigned char sendReturnCode = LIN_FEED_CODE_CRLF;
unsigned char receiveReturnCode = LIN_FEED_CODE_CRLF; 
static unsigned long logCount = 0;
static unsigned char sendDelay = 5;
static eCONNECT comConnect = ECONNECT_IDLE;
bool isBinaryLogData = true;
bool isTimeStamp = false;
bool islogTimerUpdate = false;
double logParam[20]; //logにデータ数値
volatile bool isLogUpdate = false;

void createLog (unsigned char buf);
unsigned char getArgSize (unsigned char arg_type);
void logFileOpen();
void logFileReOpen();
void logFileClose();
void terminalMain();
void terminalClose();
void socket();
void sirial();
void readLogToolIni();
void setlogCount(unsigned long);
void timeStamp(char *buffer);
void dispLog();
void keepLog();
void updateLog();
bool checkVisualLog(bool isUpdateValog);

void readLogToolIni()
{
	TCHAR text[30];
	TCHAR defaultText[30];
	
	sprintf_s(defaultText, sizeof(defaultText), "%d", sendDelay);
	GetPrivateProfileString(_T("COM_SETTING"), _T("SEND_DELAY"), _T(defaultText), text, sizeof(text), logToolSettingPath);
	WritePrivateProfileString(_T("COM_SETTING"), _T("SEND_DELAY"), text, logToolSettingPath);
	sendDelay = atoi(text);

	sprintf_s(defaultText, sizeof(defaultText), "%d", baudRate);
	GetPrivateProfileString(_T("COM_SETTING"), _T("BAUD_RATE"), _T(defaultText), text, sizeof(text), logToolSettingPath);
	WritePrivateProfileString(_T("COM_SETTING"), _T("BAUD_RATE"), text, logToolSettingPath);
	baudRate = atoi(text);

	GetPrivateProfileString(_T("COM_SETTING"), _T("PORT"), _T("COM3"), port, sizeof(port), logToolSettingPath);
	GetPrivateProfileString(_T("TERMINAL"), _T("PORT"), _T(port), port, sizeof(port), szIniFilePrivate);
	WritePrivateProfileString(_T("COM_SETTING"), _T("PORT"), port, logToolSettingPath);
	WritePrivateProfileString(_T("TERMINAL"), _T("PORT"), port, szIniFilePrivate);

	GetPrivateProfileString(_T("COM_SETTING"), _T("RCV_RETURN_CODE"), _T("CR+LF"), text, sizeof(text), logToolSettingPath);
	WritePrivateProfileString(_T("COM_SETTING"), _T("RCV_RETURN_CODE"), text, logToolSettingPath);
	receiveReturnCode = ((strncmp(text, "CR+LF", 5) == 0) ? LIN_FEED_CODE_CRLF : ((strncmp(text, "LF", 2) == 0) ? LIN_FEED_CODE_LF : LIN_FEED_CODE_CR));

	GetPrivateProfileString(_T("COM_SETTING"), _T("SEND_RETURN_CODE"), _T("CR+LF"), text, sizeof(text), logToolSettingPath);
	WritePrivateProfileString(_T("COM_SETTING"), _T("SEND_RETURN_CODE"), text, logToolSettingPath);
	sendReturnCode = ((strncmp(text, "CR+LF", 5) == 0) ? LIN_FEED_CODE_CRLF : ((strncmp(text, "LF", 2) == 0) ? LIN_FEED_CODE_LF : LIN_FEED_CODE_CR));

	GetPrivateProfileString(_T("COM_SETTING"), _T("LOG_MODE"), _T("BINARY"), text, sizeof(text), logToolSettingPath);
	WritePrivateProfileString(_T("COM_SETTING"), _T("LOG_MODE"), text, logToolSettingPath);
	isBinaryLogData = (strncmp(text, "BINARY", 6) == 0);
}

// 通信設定
INT_PTR CALLBACK comSetting(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND hId;
	char text[10];
	unsigned char data;

	switch (message) {
		case WM_INITDIALOG:
			EnableMenuItem(GetMenu(hSyslogDlgWnd), IDM_COM_SETTING, MF_DISABLED);
			hComDlgWnd = hDlg;
			hId = GetDlgItem(hDlg, ID_SEND_DELAY);
			sprintf_s(text, sizeof(text), "%d", sendDelay);
			SendMessage(hId, WM_SETTEXT, 0, (LPARAM)text);
			hId = GetDlgItem(hDlg, ID_BAUD_RATE);
			sprintf_s(text, sizeof(text), "%d", baudRate);
			SendMessage(hId, WM_SETTEXT, 0, (LPARAM)text);
			hId = GetDlgItem(hDlg, ID_PORT);
			SendMessage(hId, WM_SETTEXT, 0, (LPARAM)port);
			hId = GetDlgItem(hDlg, ID_COMB1);
			SendMessage(hId , CB_ADDSTRING , 0 , (LPARAM)"CR");
			SendMessage(hId , CB_ADDSTRING , 0 , (LPARAM)"LF");
			SendMessage(hId , CB_ADDSTRING , 0 , (LPARAM)"CR+LF");
			data = (receiveReturnCode == (LIN_FEED_CODE_CRLF) ? 2 : ((receiveReturnCode == LIN_FEED_CODE_LF) ? 1 : 0));
			SendMessage(hId , CB_SETCURSEL , data , 0);// CR or LF or CF+LF
			hId = GetDlgItem(hDlg, ID_COMB2);
			SendMessage(hId , CB_ADDSTRING , 0 , (LPARAM)"CR");
			SendMessage(hId , CB_ADDSTRING , 0 , (LPARAM)"LF");
			SendMessage(hId , CB_ADDSTRING , 0 , (LPARAM)"CR+LF");
			data = (sendReturnCode == (LIN_FEED_CODE_CRLF) ? 2 : ((sendReturnCode == LIN_FEED_CODE_LF) ? 1 : 0));
			SendMessage(hId , CB_SETCURSEL , data , 0); // CR or LF or CF+LF
			hId = GetDlgItem(hDlg, ID_PORT);
			SendMessage(hId , CB_ADDSTRING , 0 , (LPARAM)"COM1");
			SendMessage(hId , CB_ADDSTRING , 0 , (LPARAM)"COM2");
			SendMessage(hId , CB_ADDSTRING , 0 , (LPARAM)"COM3");
			SendMessage(hId , CB_ADDSTRING , 0 , (LPARAM)"COM4");
			SendMessage(hId , CB_ADDSTRING , 0 , (LPARAM)"COM5");
			SendMessage(hId , CB_ADDSTRING , 0 , (LPARAM)"COM6");
			hId = GetDlgItem(hDlg, ID_BINARY);
			SendMessage(hId , BM_SETCHECK , (isBinaryLogData ? BST_CHECKED : BST_UNCHECKED) , 0);
			hId = GetDlgItem(hDlg, ID_ASCII);
			SendMessage(hId , BM_SETCHECK , (!isBinaryLogData ? BST_CHECKED : BST_UNCHECKED) , 0);
			SetFocus(hId); // フォーカスを合わせる
			break;
		case WM_COMMAND:
			switch (wParam) {
				case IDCANCEL:
					EndDialog(hDlg, LOWORD(wParam));
					DestroyWindow(hDlg);
					break;
				case IDOK:
					hId = GetDlgItem(hDlg, ID_SEND_DELAY);
					SendMessage(hId, WM_GETTEXT, 10, (LPARAM)text);
					sendDelay = atoi(text);
					WritePrivateProfileString(_T("COM_SETTING"), _T("SEND_DELAY"), text, logToolSettingPath);
					hId = GetDlgItem(hDlg, ID_BAUD_RATE);
					SendMessage(hId, WM_GETTEXT, 10, (LPARAM)text);
					baudRate = atoi(text);
					WritePrivateProfileString(_T("COM_SETTING"), _T("BAUD_RATE"), text, logToolSettingPath);
					hId = GetDlgItem(hDlg, ID_PORT);
					SendMessage(hId, WM_GETTEXT, 10, (LPARAM)text);
					sprintf_s(port, sizeof(port), "%s", text);
					WritePrivateProfileString(_T("COM_SETTING"), _T("PORT"), text, logToolSettingPath);
					WritePrivateProfileString(_T("TERMINAL"), _T("PORT"), port, szIniFilePrivate);
					hId = GetDlgItem(hDlg, ID_COMB1);
					SendMessage(hId, WM_GETTEXT, 10, (LPARAM)text);
					WritePrivateProfileString(_T("COM_SETTING"), _T("RCV_RETURN_CODE"), text, logToolSettingPath);
					receiveReturnCode = ((strncmp(text, "CR+LF", 5) == 0) ? LIN_FEED_CODE_CRLF : ((strncmp(text, "LF", 2) == 0) ? LIN_FEED_CODE_LF : LIN_FEED_CODE_CR));
					hId = GetDlgItem(hDlg, ID_COMB2);
					SendMessage(hId, WM_GETTEXT, 10, (LPARAM)text);
					WritePrivateProfileString(_T("COM_SETTING"), _T("SEND_RETURN_CODE"), text, logToolSettingPath);
					sendReturnCode  = ((strncmp(text, "CR+LF", 5) == 0) ? LIN_FEED_CODE_CRLF : ((strncmp(text, "LF", 2) == 0) ? LIN_FEED_CODE_LF : LIN_FEED_CODE_CR));
					hId = GetDlgItem(hDlg, ID_BINARY);
					isBinaryLogData = (BST_CHECKED == SendMessage(hId, BM_GETCHECK, 0, 0) ? true : false);
					WritePrivateProfileString(_T("COM_SETTING"), _T("LOG_MODE"), (isBinaryLogData ? "BINARY" : "ASCII"), logToolSettingPath);
					EndDialog(hDlg, LOWORD(wParam));
					comConnect = ECONNECT_RETRY;
					break;
				default:
					break;
			}
			break;
		case WM_DESTROY:
			EnableMenuItem(GetMenu(hSyslogDlgWnd), IDM_COM_SETTING, MF_ENABLED);
			break;
		default:
			break;
	}
	return (INT_PTR)FALSE;
}

void socket ()
{
	/* IP アドレス、ポート番号、ソケット */
	char destination[80];
	char hostname[256];
	IN_ADDR inaddr;

	TCHAR portBuf[10];
	GetPrivateProfileString(_T("TERMINAL"), _T("PORT"), _T("9999"), portBuf, sizeof(portBuf), szIniFilePrivate);
	unsigned short port = atoi(portBuf);

	/* sockaddr_in 構造体 */
	struct sockaddr_in dstAddr;

	/************************************************************/

	/* Windows 独自の設定 */
	WSADATA data;
	WSAStartup(MAKEWORD(2, 0), &data);

	//ホスト名を取得する
	gethostname(hostname, sizeof(hostname));
	//ホスト名からIPアドレスを取得する
	HOSTENT* hostend = gethostbyname(hostname);
	memcpy(&inaddr, hostend->h_addr_list[0], 4);
	strcpy_s(destination, inet_ntoa(inaddr));

	/* 相手先アドレスの入力 */
	printf("Connect to ? : (name or IP address) ");
	//	scanf_s("%s", destination);

	/* sockaddr_in 構造体のセット */
	memset(&dstAddr, 0, sizeof(dstAddr));
	dstAddr.sin_port = htons(port);
	dstAddr.sin_family = AF_INET;
	dstAddr.sin_addr.s_addr = inet_addr(destination);

	/* ソケット生成 */
	dstSocket = socket(AF_INET, SOCK_STREAM, 0);

	/* 接続 */
	printf("Trying to connect to %s: \n", destination);
	connect(dstSocket, (struct sockaddr *) &dstAddr, sizeof(dstAddr));

	char buffer[1];
	while (1) {
		if (comBusyFlag == false) {
			int numrcv;
			numrcv = recv(dstSocket, buffer, 1, 0);
			if((numrcv == 0) || (numrcv == -1)) {
				break;
			}
			createLog(buffer[0]);
		}
	}

	/* Windows 独自の設定 */
	closesocket(dstSocket);
	WSACleanup();
	fclose(fileP);
}

void sirial ()
{
	BYTE buffer[10];
	char str[100];
	DWORD receiveNum;
	BOOL Ret;
	COMMTIMEOUTS timeout;

	//LogTool.iniファイルから設定読み込み
	readLogToolIni();

	// 指定ポートを開く
	mHComPort = CreateFile(port, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(mHComPort == INVALID_HANDLE_VALUE) {
		comConnect = ECONNECT_IDLE;
		sprintf_s(str, sizeof(str), "%s - 未接続",captionName);
		SendMessage(hSyslogDlgWnd, WM_SETTEXT, 0, (LPARAM)str);
	} else {
		comConnect = ECONNECT_CONNECT;
	}
	//送受信バッファ初期化
	if (comConnect == ECONNECT_CONNECT) { 
		Ret = SetupComm(mHComPort, 1024, 1024);
		if(Ret == FALSE) {
			CloseHandle(mHComPort);
			comConnect = ECONNECT_IDLE;
			sprintf_s(str, sizeof(str), "%s - 未接続",captionName);
			SendMessage(hSyslogDlgWnd, WM_SETTEXT, 0, (LPARAM)str);
		}
	}
	if (comConnect == ECONNECT_CONNECT) { 
		Ret = PurgeComm(mHComPort, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
		if(Ret == FALSE) {
			CloseHandle(mHComPort);
			comConnect = ECONNECT_IDLE;
			sprintf_s(str, sizeof(str), "%s - 未接続",captionName);
			SendMessage(hSyslogDlgWnd, WM_SETTEXT, 0, (LPARAM)str);
		}
	}
	if (comConnect == ECONNECT_CONNECT) { 
		timeout.ReadIntervalTimeout = 10;
		timeout.ReadTotalTimeoutMultiplier = 10;
		timeout.ReadTotalTimeoutConstant = 10;
		Ret = SetCommTimeouts(mHComPort, &timeout);
		if(Ret == FALSE) {
			printf("SetCommTimeouts failed.\n");
			CloseHandle(mHComPort);
			comConnect = ECONNECT_IDLE;
			sprintf_s(str, sizeof(str), "%s - 未接続",captionName);
			SendMessage(hSyslogDlgWnd, WM_SETTEXT, 0, (LPARAM)str);
		}
	}

	// ポートのボーレート、パリティ等を設定
	if ((mHComPort != INVALID_HANDLE_VALUE) && (comConnect == ECONNECT_CONNECT)) {
		GetCommState( mHComPort, &dcb );
		dcb.BaudRate = baudRate;
		dcb.fParity = NOPARITY;
		dcb.StopBits = ONESTOPBIT;
		dcb.ByteSize = 8;
		nBret = SetCommState(mHComPort,  &dcb);
		sprintf_s(str, sizeof(str), "%s - %s",captionName, port);
		SendMessage(hSyslogDlgWnd, WM_SETTEXT, 0, (LPARAM)str);
	}

	while (1) {
		if (comConnect == ECONNECT_CONNECT) {
			if ((comBusyFlag == false) && (isLogUpdate == false)) {
				ReadFile(mHComPort, buffer, 1, &receiveNum, NULL);
				if (receiveNum > 0) {
					if (isBinaryLogData) {
						createLog(buffer[0]);
					} else {
						createAsciiLog(buffer[0]);
					}
				}
			}
		} else if (comConnect == ECONNECT_RETRY)  {
			break;
		}
	}
	if (mHComPort != NULL) {
		CloseHandle(mHComPort);
	}
}

void terminalMain ()
{
	TCHAR buf[128];
	errno_t result;

	GetPrivateProfileString(_T("TERMINAL"), _T("terminal"), _T("logTool"), buf, sizeof(buf), szIniFilePrivate);
	if (strncmp(buf, "teraterm", 8) == 0) {
		isTeratermUse = true;
		EnableWindow(GetDlgItem(hSyslogDlgWnd, IDSEND), FALSE);
		EnableWindow(GetDlgItem(hSyslogDlgWnd, IDC_SEND_TEXT), FALSE);
		EnableMenuItem(GetMenu(hSyslogDlgWnd), IDM_DOWNLOAD, MF_DISABLED);
		EnableMenuItem(GetMenu(hSyslogDlgWnd), IDM_COM_SETTING, MF_DISABLED);
		return;
	}

	GetPrivateProfileString(_T("TERMINAL"), _T("Engine"), _T("Real"), buf, sizeof(buf), szIniFilePrivate);
	if (strncmp(buf, "Real", 4) == 0) {
		isEngineReal = true;
		result = fopen_s(&fileP, syslog_file, "wb");
		while (1) {
			sirial();
		}
	} else if (strncmp(buf, "Virtual", 7) == 0)  {
		isEngineReal = false;
		result = fopen_s(&fileP, syslog_file, "wb");
		EnableMenuItem(GetMenu(hSyslogDlgWnd), IDM_DOWNLOAD, MF_DISABLED);
		EnableMenuItem(GetMenu(hSyslogDlgWnd), IDM_COM_SETTING, MF_DISABLED);
		sprintf_s(buf, sizeof(buf), "%s",captionName);
		SendMessage(hSyslogDlgWnd, WM_SETTEXT, 0, (LPARAM)buf);
		socket();
	} else {
		isTeratermUse = true;
		EnableWindow(GetDlgItem(hSyslogDlgWnd, IDSEND), FALSE);
		EnableWindow(GetDlgItem(hSyslogDlgWnd, IDC_SEND_TEXT), FALSE);
		EnableMenuItem(GetMenu(hSyslogDlgWnd), IDM_DOWNLOAD, MF_DISABLED);
		EnableMenuItem(GetMenu(hSyslogDlgWnd), IDM_COM_SETTING, MF_DISABLED);
	}
}

void sendText(char *buffer)
{
	unsigned short i = 0;

	if (isTeratermUse == true) {
		return;
	}
	DWORD writeBytes;
	if (isEngineReal) {
		while(1) {
			char *c = buffer + i;
			if (strncmp(c, "\0", 1) == 0) {
				if (sendReturnCode & LIN_FEED_CODE_CR) {
					WriteFile(mHComPort, "\r", 1, &writeBytes, NULL);
					Sleep(sendDelay);
				}
				if (sendReturnCode & LIN_FEED_CODE_LF) {
					WriteFile(mHComPort, "\n", 1, &writeBytes, NULL);
					Sleep(sendDelay);
				}
				break;
			} else {
				WriteFile(mHComPort, (BYTE*)c, 1, &writeBytes, NULL);
				Sleep(sendDelay);
				i++;
			}
		}
	} else {
		while(1) {
			char *c = buffer + i;
			if (strncmp(c, "\0", 1) == 0) {
				send(dstSocket, "\r", 1, 0);
				Sleep(5);
				send(dstSocket, "\n", 1, 0);
				Sleep(5);
				break;
			} else {
				send(dstSocket, c, 1, 0);
				Sleep(5);
				i++;
			}
		}
	}
}

void logFileOpen ()
{
	if (isTeratermUse == true) {
		return;
	}
	fopen_s(&fileP, syslog_file, "wb");
}

void logFileReOpen ()
{
	if (isTeratermUse == true) {
		return;
	}
	fopen_s(&fileP, syslog_file, "ab");
}

void logFileClose ()
{
	if (isTeratermUse == true) {
		return;
	}
	fclose(fileP);
}

void setlogCount (unsigned long value)
{
	logCount = value;
}

void terminalClose ()
{
	if (isTeratermUse == true) {
		return;
	}
	if (isEngineReal) {
		if (mHComPort != NULL) {
			CloseHandle(mHComPort);
		}
	} else {
		closesocket(dstSocket);
		WSACleanup();
	}
}

void createAsciiLog (unsigned char buf)
{
	unsigned short i;
	static char log[256];
	static unsigned short logPos = 0;
	char code;//改行コード

	if(receiveReturnCode & LIN_FEED_CODE_CR) {
		code = 0x0d;
		if (buf == 0x0a) {
			return;
		}
	} else {
		code = 0x0a;
	}

	if (buf == '\t') { //タブはスペース（0x20）×4
		log[logPos++] = 0x20;
		log[logPos++] = 0x20;
		log[logPos++] = 0x20;
		log[logPos++] = 0x20;
	} else {
		log[logPos++] = buf;
	}

	if (buf == code) {
		logPos = 0;		
		timeStamp(log);
		dispLog();
		LogMonitor((char*)log, 0, logParam, 1);
		if (strncmp((char*)log, "erase end", 9) == 0) {
			eraseEnd();
		}
		receiveFixLog((char*)log);
		for (i = 0; i < 256; i++) {
			log[i] = 0;
		}
	}
	fwrite(&buf, sizeof(unsigned char), 1, fileP);
}

void createLog (unsigned char buf)
{
	static unsigned char pos = 0;
	static unsigned char log[256];
	static char err_lvl;
	static unsigned long tim;
	static char group_id;
	static short log_no;
	static char arg_type;
	static char arg_num;
	static unsigned char argSize = 1;
	static bool tcmMode = false;
	char* msg;
	bool isUpdateValog;

	log[pos] = buf;
	pos++;

	if (pos == 1) {
		if(log[0] & 0x80) {
			tcmMode = true;
		} else {
			// TCM→SYSLOG切り替え
			tcmMode = false;
			err_lvl = (log[0] & 0x78) >> 3;	// エラーレベル
			if((err_lvl < 0) || (err_lvl > 3)) {
				pos = 0;
			}
		}
	} else if ((pos == 4) && (!tcmMode)){
		tim = (((log[0] & 0x07) << 24) + (log[1] << 16) + (log[2] << 8) + log[3]);
	} else if ((pos == 5) && (!tcmMode)) {
		group_id = (log[4] & 0xfc) >> 2;	// グループID
	} else if ((pos == 6) && (!tcmMode)) {
		log_no = ((log[4] & 0x03) << 8) + log[5];	// ログNo
	} else if ((pos == 7) && (!tcmMode)) {
		arg_type = (log[6] & 0xf0) >> 4;	// 引数タイプ
		arg_num = (log[6] & 0x0f);			// 引数の数
		argSize = getArgSize(arg_type);
		if (argSize == 0xff) {
			pos = 0;
		} else {
			msg = get_msgtbl(group_id, log_no, arg_num);
			if(msg == NULL) {
				pos = 0;
				sprintf_s(line_mbuf, "不明なログ");
				timeStamp(line_mbuf);
				dispLog();
			}
		}
	} else {
	}
	
	if (tcmMode == false) {
		if((pos - 7) == (arg_num * argSize)){
			//VisualAnalyzerの時間更新
			isUpdateValog = updateVisualAnalyzeTime(tim * msPerClock);

			// 表示対象のロググループかのチェック
			if(log_group_disp[group_id] == FALSE) {
				//表示はしないが、ログメッセージに変換はする
				msg = get_msgtbl(group_id, log_no, arg_num);
				createLogMessage(NULL, tim, log_no, err_lvl, group_id, arg_type, arg_num, msg, &log[7]);
				paramConversion(arg_num, arg_type, &log[7], logParam); //paramにデータを格納
				LogMonitor(msg, tim * msPerClock, logParam, arg_num);
				checkVisualLog(isUpdateValog);
			} else if ((defLogLvl == LOGLVL_ERROR) && ((err_lvl != LOGLVL_ERROR) && (err_lvl != LOGLVL_SYSTEM))) {
				//表示はしないが、ログメッセージに変換はする
				msg = get_msgtbl(group_id, log_no, arg_num);
				createLogMessage(NULL, tim, log_no, err_lvl, group_id, arg_type, arg_num, msg, &log[7]);
				paramConversion(arg_num, arg_type, &log[7], logParam); //paramにデータを格納
				LogMonitor(msg, tim * msPerClock, logParam, arg_num);
				checkVisualLog(isUpdateValog);
			} else if ((defLogLvl == LOGLVL_WARNING) && (err_lvl == LOGLVL_INFORMATION)) {
				//表示はしないが、ログメッセージに変換はする
				msg = get_msgtbl(group_id, log_no, arg_num);
				createLogMessage(NULL, tim, log_no, err_lvl, group_id, arg_type, arg_num, msg, &log[7]);
				paramConversion(arg_num, arg_type, &log[7], logParam); //paramにデータを格納
				LogMonitor(msg, tim * msPerClock, logParam, arg_num);
				checkVisualLog(isUpdateValog);
			} else {
				if ((group_id == 3) && ((log_no >= 1) && (log_no <= 3)) ) {
					int from_idx = (log_no-1)*15;
					HWND lbox = GetDlgItem(hSyslogDlgWnd, IDC_SYSLOG);

					for(int cnt = from_idx; cnt <= (from_idx+14); cnt++) {
						if(log_group_num <= cnt) {
							break;
						}
						// 文字列先頭のログ連番分ずらした位置から出力する。
						sprintf_s(dispLine, "%d:%s:%d\n", cnt, log_group[cnt], log[7+cnt-from_idx]);
						dispLog();
					}
				} else {
					HWND lbox = GetDlgItem(hSyslogDlgWnd, IDC_SYSLOG);
					msg = get_msgtbl(group_id, log_no, arg_num);
					int max_len = createLogMessage(NULL, tim, log_no, err_lvl, group_id, arg_type, arg_num, msg, &log[7]);
					
					if(checkVisualLog(isUpdateValog) == false) {
						timeStamp(line_mbuf);
						dispLog();
					}
					paramConversion(arg_num, arg_type, &log[7], logParam); //paramにデータを格納
					LogMonitor(line_mbuf, tim * msPerClock, logParam, arg_num);
					// ListBox横幅以上のメッセージの場合のみ横ScrollBarを出す
					if(max_len >= 0x52) {
						SendMessage(lbox, LB_SETHORIZONTALEXTENT, (WPARAM)((max_len+1)*6), (LPARAM)0);
						ShowScrollBar(lbox, SB_HORZ, TRUE);
					}
				}
			}
			pos = 0;
		}
	} else {
		if (pos == 8) {
			//タイミングチャートモードの場合の表示
			HWND lbox = GetDlgItem(hSyslogDlgWnd, IDC_SYSLOG);
			unsigned short id = (unsigned short)(log[4] << 3) + ((log[5] >> 5) & 7);
			unsigned char type = (log[0] & 0x78)>>3;
			unsigned char io = (log[5] >> 4) & 1;
			unsigned short val = (unsigned short)((log[6] << 8) | log[7]);
			unsigned long time = ((0x07 & log[0]) << 24) | (log[1]<<16) | (log[2]<<8) | log[3];
			unsigned char onOff = (log[5] & 0x0f) >> 1;
			char readBuf[128];
			char dispBuf[128];
			char deviceId[6];
			char value[64];
			char param[64];
			pos = 0;
			sprintf_s(deviceId, 6, "%04x%d", id, io);
			GetPrivateProfileString(_T("DEVICE_NAME"), deviceId, deviceId, readBuf, sizeof(readBuf) - 5, szIniFilePublic);
			if (senarioSeparete(id)) {
				sprintf_s(value, " %d", val % 1000);
				sprintf_s(param, " [%02d]", val/1000);
				strcat_s(readBuf, param);
				strcat_s(readBuf, value);
			} else {
				sprintf_s(value, " %d %d",onOff, val);
				strcat_s(readBuf, value);
			}
			sprintf_s(dispBuf, "%09d %s",time,readBuf);
			strcat_s(dispBuf, "\n");
			int max_len = createLogMessage(NULL, tim, log_no, err_lvl, group_id, arg_type, arg_num, dispBuf, &log[7]);
			if((log_group_disp[tcmLogGroupNo] == TRUE) && (type != TCM_TYPE_TIME)) {
				timeStamp(line_mbuf);
				dispLog();
			}
			LogMonitor(line_mbuf, time * msPerClock, logParam, 1);
			// ListBox横幅以上のメッセージの場合のみ横ScrollBarを出す
			if(max_len >= 0x52) {
				SendMessage(lbox, LB_SETHORIZONTALEXTENT, (WPARAM)((max_len+1)*6), (LPARAM)0);
				ShowScrollBar(lbox, SB_HORZ, TRUE);
			}
			tcmLogInfo(time, (id<<4) | io, onOff, val);
		}
	}

	if (getVaState() == ELOG_READ_NORMAL) {
		fwrite(&buf, sizeof(unsigned char), 1, fileP);
	}
}

bool checkVisualLog(bool isUpdateValog)
{
	bool ret = false;
	char* visualLogOffset = strstr(line_mbuf, "VisualLog: ");

	if (visualLogOffset != NULL) {
		if (isUpdateValog) {
			visualAnalyzeLogInfo(visualLogOffset + strlen("VisualLog: "));
		} else {
			visualAnalyzeLogWait(visualLogOffset + strlen("VisualLog: "));
		}
		if (getVisualLogDisp()) {
			timeStamp(line_mbuf);
			dispLog();
		}
		ret = true;
	}
	
	return ret;
}

void dispLog()
{
	if (islogTimerUpdate) {
		keepLog();
	} else {
		keepLog();
		updateLog();
	}
}
void keepLog()
{
	strcpy_s(logKeepBufmbuf[logKeepCount], dispLine);
	if (logKeepCount < LOG_KEEP_NUM + LOG_BUF_MARGIN) {
		logKeepCount++;
	}
	if (logKeepCount >= LOG_KEEP_NUM) {
		if (!isTimeLogUpdate) {
			isLogUpdate = true;
			updateLog();
			isLogUpdate = false;
		}
	}
}

void updateLog()
{
	HWND lbox = GetDlgItem(hSyslogDlgWnd, IDC_SYSLOG);
	if (logKeepCount > 0) {
		for (int i = 0;i < logKeepCount;i++) {
			SendMessage(lbox, LB_ADDSTRING, 0, (LPARAM)logKeepBufmbuf[i]);
			logCount++;
		}
		logKeepCount = 0;
		if (IsWindowVisible(hDlgSearch) == NULL) {//検索ウィンドウが表示されていなければカーソル位置を更新
			SendMessage(lbox, LB_SETCURSEL, (WPARAM)(logCount - 1), (LPARAM)0);
		}
	}
}

unsigned char getArgSize (unsigned char arg_type)
{
	unsigned char ret = 1;

	switch(arg_type) {
		case 0:		// 引数無し
			ret = 0;
			break;
		case 1:		// signed char
		case 2:		// unsigned char
			ret = 1;
			break;
		case 4:		// signed short
		case 5:		// unsigned short
			ret = 2;
			break;
		case 6:		// signed long
		case 7:		// unsigned long
			ret = 4;
			break;
		default:
			ret = 0xff;
			break;
	}

	return ret;
}

void timeStamp(char *buffer)
{
	time_t timer = time(NULL);
	struct tm date;
	localtime_s(&date, &timer);

	unsigned short hour = date.tm_hour;
	unsigned short min = date.tm_min;
	unsigned short sec = date.tm_sec;

	if (isTimeStamp) {
		sprintf_s(dispLine, sizeof(dispLine), "%02d:%02d:%02d %s",hour,min,sec,buffer);
	} else {
		sprintf_s(dispLine, sizeof(dispLine), "%s",buffer);
	}
	
}

void paramConversion (unsigned char argNum, unsigned char argType, unsigned char *log, double *param)
{
	unsigned char size = getArgSize(argType);
	unsigned char i;
	signed char offset = (size - 1);
	unsigned char pos = 0;
	unsigned long data;
	
	if (argNum > 0x0f) {
		argNum = 0x0f;
	}
	for (i=0; i < argNum; i++) {
		data = 0;
		for (offset = (size - 1); offset >= 0; offset--) {
			data |= log[pos++] << (offset * 8);
		}
		if (argType == 1) {
			param[i] = (signed char)data;
		} else if (argType == 4) {
			param[i] = (signed short)data;
		} else if (argType == 6) {
			param[i] = (signed long)data;
		} else {
			param[i] = data;
		}
	}
}

void stringSearch(char *searchLine, unsigned char direction)
{
	HWND lbox = GetDlgItem(hSyslogDlgWnd, IDC_SYSLOG);
	char buffer[1024];
	char* readline;
	char end;
	int pos;
	int listCount;

	for (int i = 0; i < 2; i++) {
		end = searchLine[strlen(searchLine) - 1];
		if ((end == '\r') || (end == '\n')) {
			searchLine[strlen(searchLine) - 1] = '\0';
		}
	}


	pos = SendMessage(lbox, LB_GETCURSEL, 0, 0);
	listCount = SendMessage(lbox, LB_GETCOUNT, 0, 0);
	if (direction == 0) {
		while (pos < listCount) {
			pos++;
			SendMessage(lbox, LB_GETTEXT, pos, (LPARAM)buffer);
			readline = strstr(buffer, searchLine);
			if(readline != NULL) {
				SendMessage(lbox, LB_SETCURSEL, (WPARAM)(pos), (LPARAM)0);
				break;
			}
		}
	} else {
		while (pos > 0) {
			pos--;
			SendMessage(lbox, LB_GETTEXT, pos, (LPARAM)buffer);
			readline = strstr(buffer, searchLine);
			if(readline != NULL) {
				SendMessage(lbox, LB_SETCURSEL, (WPARAM)(pos), (LPARAM)0);
				break;
			}
		}
	}
}

void dispToolDbgLog(char* valog)
{
	HWND lbox = GetDlgItem(hSyslogDlgWnd, IDC_SYSLOG);

	SendMessage(lbox, LB_ADDSTRING, 0, (LPARAM)valog);
	SendMessage(lbox, LB_SETCURSEL, (WPARAM)(logCount), (LPARAM)0);
	logCount++;
}

void writeUsbLogBin(unsigned char* logBuf, size_t len)
{
	for (unsigned short i = 0; i < len; i++ ) {
		fwrite(logBuf + i, sizeof(unsigned char), 1, fileP);
	}
	logCount++;
}
