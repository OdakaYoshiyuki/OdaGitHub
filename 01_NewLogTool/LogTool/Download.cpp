#include "stdafx.h"
#include "LogTool.h"
#include "Resource.h"
#include <stdio.h>
#include <share.h>
#include <tchar.h>
#include <stdlib.h>
#include <windowsx.h>
#include <CommCtrl.h>
#include <shellapi.h> //ドラッグ＆ドロップ用
#include <ShlObj.h>

#define PB_MIN 0
#define PB_MAX 100
#define PB_STEP 5
#define SEND_BLOCK_BYTE 1024
#define ERASE_TIMER_OUT 1
#define ID_DOWNLOAD_FILE_SEND 0xff

enum eDL_STATE{
	 EDL_STATE_IDLE
	,EDL_STATE_BOOT_JUMP
	,EDL_STATE_BOOT_ELASE
	,EDL_STATE_BOOT_DOWNLOAD
};

extern char logToolSettingPath[];
extern HANDLE mHComPort;
extern HWND hSyslogDlgWnd;
extern HWND hComDlgWnd;
extern bool isBinaryLogData;
extern unsigned char sendReturnCode;
extern unsigned char receiveReturnCode; 
extern void sendText(char *buffer);

static char dlFileName[200];
static bool eraseEndFlag = false;
static unsigned short eraseTime;
static FILE *dlFile;
static unsigned long fileSize;
static unsigned long sendByte = 0;
static unsigned long sendCount = 0;

void downloadStandby(HWND hDlg, HDROP hDrop, HWND hProg);
void downloadFileSend(HWND hDlg, HDROP hDrop, HWND hPro);
void eraseEnd();

// パスワード
INT_PTR CALLBACK passWord(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char buff[100];
	static char settingInipath[256];

	switch (message) {
		case WM_INITDIALOG:
			GetPrivateProfileString(_T("DOWNLOAD"), _T("PASS_WORD"), _T(""), buff, sizeof(buff), logToolSettingPath);
			SendMessage(GetDlgItem(hDlg, ID_PASS_WORD), WM_SETTEXT, 0, (LPARAM)buff);
			break;
		case WM_COMMAND:
			switch (wParam) {
				case IDCANCEL:
					EndDialog(hDlg, LOWORD(FALSE));
					break;
				case IDOK:
					GetDlgItemText(hDlg, ID_PASS_WORD, buff, sizeof(buff));
					if (strncmp(buff, "ld external", 11) == 0) {
						WritePrivateProfileString(_T("DOWNLOAD"), _T("PASS_WORD"), "ld external", logToolSettingPath);
						EndDialog(hDlg, LOWORD(TRUE));
					} else {
						SendMessage(GetDlgItem(hDlg, ID_PASS_WORD), WM_SETTEXT, 0, (LPARAM)"");
					}
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
	return (INT_PTR)FALSE;
}
// ダウンロード
INT_PTR CALLBACK download(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HDROP hDrop;
	static HWND hProg;
	static unsigned char downloadState;
	static bool currentLogMode;
	static unsigned char currentSendReturnCode;
	static unsigned char currentReceiveReturnCode;
	static char logModeBuf[10];
	static char sReturnCodeBuf[10];
	static char rReturnCodeBuf[10];
	unsigned char data;
	
	UNREFERENCED_PARAMETER(lParam);
	switch (message) {
		case WM_INITDIALOG:
			DragAcceptFiles(hDlg, TRUE);		//ドロップ可否指定[True:Drop許可]
			hProg = GetDlgItem(hDlg, IDC_PROGRESS);
			SendMessage(hProg, PBM_SETRANGE, (WPARAM)0, MAKELPARAM(PB_MIN, PB_MAX));
			SendMessage(hProg, PBM_SETSTEP, (WPARAM)PB_STEP, 0);
			EnableMenuItem(GetMenu(hSyslogDlgWnd), IDM_DOWNLOAD, MF_DISABLED);
			currentLogMode = isBinaryLogData;
			currentSendReturnCode = sendReturnCode;
			currentReceiveReturnCode = receiveReturnCode;
			GetPrivateProfileString(_T("COM_SETTING"), _T("SEND_RETURN_CODE"), _T("CR+LF"), sReturnCodeBuf, sizeof(sReturnCodeBuf), logToolSettingPath);
			GetPrivateProfileString(_T("COM_SETTING"), _T("RCV_RETURN_CODE"), _T("CR+LF"), rReturnCodeBuf, sizeof(rReturnCodeBuf), logToolSettingPath);
			GetPrivateProfileString(_T("COM_SETTING"), _T("LOG_MODE"), _T("BINARY"), logModeBuf, sizeof(logModeBuf), logToolSettingPath);
			downloadState = EDL_STATE_IDLE;
			break;
		case WM_DROPFILES:
			hDrop = (HDROP)wParam;
			DragQueryFile(hDrop, 0, (PTSTR)dlFileName, 200);
			SendMessage(GetDlgItem(hDlg, ID_DISP_MSG), WM_SETTEXT, 0, (LPARAM)"ダウンロード中");
			downloadStandby(hDlg, hDrop, hProg);
			break;
		case WM_TIMER:
			if (wParam == ERASE_TIMER_OUT) {
				if (eraseEndFlag) {
					SendMessage(GetDlgItem(hDlg, ID_DISP_MSG), WM_SETTEXT, 0, (LPARAM)"ファイルをドラッグ＆ドロップ");
					KillTimer(hDlg, 1);
				}
			}
			break;
		case WM_COMMAND:
			switch (wParam) {
				case IDCANCEL:
					EndDialog(hDlg, LOWORD(wParam));
					EnableWindow(GetDlgItem(hSyslogDlgWnd, IDSEND), TRUE);
					EnableMenuItem(GetMenu(hSyslogDlgWnd), IDM_DOWNLOAD, MF_ENABLED);
					DestroyWindow(hDlg);
					break;
				case ID_BOOT:
					if (downloadState == EDL_STATE_IDLE) {
						sendText("boot");	
						downloadState = EDL_STATE_BOOT_JUMP;
						SendMessage(GetDlgItem(hDlg, ID_DISP_MSG), WM_SETTEXT, 0, (LPARAM)"Eraseボタンでイレースします");
						isBinaryLogData = false;
						receiveReturnCode = LIN_FEED_CODE_CRLF;
						sendReturnCode = LIN_FEED_CODE_CR;
						WritePrivateProfileString(_T("COM_SETTING"), _T("LOG_MODE"), "ASCII", logToolSettingPath);
						WritePrivateProfileString(_T("COM_SETTING"), _T("RCV_RETURN_CODE"), "CR+LF", logToolSettingPath);
						WritePrivateProfileString(_T("COM_SETTING"), _T("SEND_RETURN_CODE"), "CR", logToolSettingPath);
						SendMessage(GetDlgItem(hComDlgWnd, ID_COMB1), CB_SETCURSEL, 2, 0); //CR+LFを選択
						SendMessage(GetDlgItem(hComDlgWnd, ID_COMB2), CB_SETCURSEL, 0, 0); //CRを選択
						SendMessage(GetDlgItem(hComDlgWnd, ID_BINARY), BM_SETCHECK,  BST_UNCHECKED, 0);
						SendMessage(GetDlgItem(hComDlgWnd, ID_ASCII), BM_SETCHECK,  BST_CHECKED, 0);
					}
					break;
				case ID_ERASE:
					if (downloadState == EDL_STATE_BOOT_JUMP) {
						sendText("ld external");
						SendMessage(GetDlgItem(hDlg, ID_DISP_MSG), WM_SETTEXT, 0, (LPARAM)"イレース中");
						eraseEndFlag = false;
						SetTimer(hDlg, ERASE_TIMER_OUT, 100, NULL);
						EnableWindow(GetDlgItem(hDlg, ID_BOOT), FALSE);
						EnableWindow(GetDlgItem(hDlg, ID_ERASE), FALSE);
						EnableWindow(GetDlgItem(hSyslogDlgWnd, IDSEND), FALSE);
					}
					break;
				case ID_DOWNLOAD_FILE_SEND:
					downloadFileSend(hDlg, hDrop, hProg);
					break;
				default:
					break;
			}
			break;
		case WM_DESTROY:
			isBinaryLogData = currentLogMode;
			sendReturnCode = currentSendReturnCode;
			receiveReturnCode = currentReceiveReturnCode;
			WritePrivateProfileString(_T("COM_SETTING"), _T("LOG_MODE"), logModeBuf, logToolSettingPath);
			WritePrivateProfileString(_T("COM_SETTING"), _T("RCV_RETURN_CODE"), rReturnCodeBuf, logToolSettingPath);
			WritePrivateProfileString(_T("COM_SETTING"), _T("SEND_RETURN_CODE"), sReturnCodeBuf, logToolSettingPath);
			SendMessage(GetDlgItem(hComDlgWnd, ID_BINARY), BM_SETCHECK, (isBinaryLogData ? BST_CHECKED : BST_UNCHECKED), 0);
			SendMessage(GetDlgItem(hComDlgWnd, ID_ASCII), BM_SETCHECK, (!isBinaryLogData ? BST_CHECKED : BST_UNCHECKED), 0);
			data = (receiveReturnCode == (LIN_FEED_CODE_CRLF) ? 2 : ((receiveReturnCode == LIN_FEED_CODE_LF) ? 1 : 0));
			SendMessage(GetDlgItem(hComDlgWnd, ID_COMB1), CB_SETCURSEL, data, 0); // CR or LF or CF+LF
			data = (sendReturnCode == (LIN_FEED_CODE_CRLF) ? 2 : ((sendReturnCode == LIN_FEED_CODE_LF) ? 1 : 0));
			SendMessage(GetDlgItem(hComDlgWnd, ID_COMB2), CB_SETCURSEL, data, 0); // CR or LF or CF+LF
			break;
		default:
			break;
	}
	return (INT_PTR)FALSE;
}


//ダウンロード準備
void downloadStandby(HWND hDlg, HDROP hDrop, HWND hProg)
{
	fpos_t pos;
	sendCount = 0;
	sendByte = 0;
	fopen_s(&dlFile, dlFileName, "rb");
	fseek(dlFile, 0L, SEEK_END);
	fgetpos(dlFile, &pos);
	fseek(dlFile, 0, SEEK_SET);
	fileSize = int(pos);
	SendMessage(hProg, PBM_SETPOS, PB_MIN, 0);
	PostMessage((HWND)hDlg, WM_COMMAND, ID_DOWNLOAD_FILE_SEND, 0);
}

//ダウンロードファイルデータの送信
void downloadFileSend(HWND hDlg, HDROP hDrop, HWND hProg)
{
	unsigned long Num;
	unsigned long i = 0;
	char data[SEND_BLOCK_BYTE];
	DWORD writeBytes;

	if (sendCount < (fileSize / SEND_BLOCK_BYTE)) {
		Num = SEND_BLOCK_BYTE;
		for (i = 0; i < Num; i++) {
			data[i] = fgetc(dlFile);
		}
		WriteFile(mHComPort, (BYTE*)data, Num, &writeBytes, NULL);
		sendByte += SEND_BLOCK_BYTE;
		if (sendByte >= (fileSize / 20)) {
			SendMessage(hProg, PBM_STEPIT, 0, 0);
			sendByte = 0;
		}
		sendCount++;
		PostMessage((HWND)hDlg, WM_COMMAND, ID_DOWNLOAD_FILE_SEND, 0);
	} else if (fileSize % SEND_BLOCK_BYTE != 0) {
		Num = (fileSize % SEND_BLOCK_BYTE);
		for (i = 0; i < Num; i++) {
			data[i] = fgetc(dlFile);
		}
		WriteFile(mHComPort, (BYTE*)data, Num, &writeBytes, NULL);

		SendMessage(hProg, PBM_SETPOS, PB_MAX, 0);
		SendMessage(GetDlgItem(hDlg, ID_DISP_MSG), WM_SETTEXT, 0, (LPARAM)"完了");
		DragFinish(hDrop);
		fclose(dlFile);
	} else {
		SendMessage(hProg, PBM_SETPOS, PB_MAX, 0);
		SendMessage(GetDlgItem(hDlg, ID_DISP_MSG), WM_SETTEXT, 0, (LPARAM)"完了");
		DragFinish(hDrop);
		fclose(dlFile);
	}
}

void eraseEnd() 
{
	eraseEndFlag = true;
}


