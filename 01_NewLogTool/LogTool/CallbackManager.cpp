#include "stdafx.h"
#include "LogTool.h"

#include <stdio.h>
#include <share.h>
#include <tchar.h>
#include <stdlib.h>
#include <windowsx.h>
#include <ShlObj.h>

//ドラッグ＆ドロップ用
#include <shellapi.h>
#pragma comment(lib, "shell32.lib")		//ライブラリ追加

#include <shlwapi.h>

#ifdef SYSLOG_MODELESS
BOOL SyslogDisp = FALSE;
extern bool isSynchroLogAndTcm;
extern bool isTimeStamp;

#endif

// 送信コマンドウインドウのdefaultプロシージャ
FARPROC orgWndProc;
// syslog表示Listboxウインドウのdefaultプロシージャ
FARPROC orgWndProc2;
// 現在のインターフェイス
HINSTANCE hInst;
// 最大ログ長(横ScrollBar出す出さないの判断用)
int max_logmsg_leng;
// 前回までのファイル位置
long prev_file_pos;
// Syslogファイル最終更新日時
SYSTEMTIME lastUpdateTime;
// 表示グループ全選択・解除モードフラグ
BOOL all_selchg_flg = TRUE;
// SYSLOG出力区間(非TCMモード中)
BOOL syslog_output = TRUE;
// 前回までのログ数
long prev_log_cnt = -1;
// ロググループ表示可否テーブル
BOOL log_group_disp[LOGGRP_DEFNUM];
HWND hDlgAcs;
HWND hDlgSearch;
volatile bool isTimeLogUpdate = false;				// 時間経過によるログ表示中フラグ
volatile bool isLogStartConvert = false;			// 引数起動でのログ変換開始フラグ

HWND logLevelDlgWnd;						// ログレベルダイアログのハンドル
// デフォルトログレベル
char defLogLvl = 0;
// ログメッセージ生成
extern int createLogMessage(FILE*, unsigned long, short, char, char, char, char, char*, unsigned char*);
// SYSLOGファイル保存
extern INT_PTR SaveSyslogToFile(HWND hDlg);
extern INT_PTR OpenSyslogTextApp(HWND hDlg);
// 設定ファイルパス設定
extern INT_PTR GetSettingFileName(HWND hDlg, int kind);
//定着ログ表示
extern void fixLogMain(HWND hDlg);

extern void visualAnalyzMain(HWND hDlg);
//タイミングチャート表示
extern void timingChartMain(HWND hWnd);
//text送信
extern void sendText(char* buffer);
//binファイルを閉じる
extern void logFileClose();
//binファイルを再度開く
extern void logFileReOpen();
//binファイルを開く
extern void logFileOpen();
//LogTerminal.cppのlogCountをセット
extern void setlogCount(unsigned long value);
//LogTerminal.cppのlog表示更新
extern void updateLog();
//自動コマンド送信機能の外部からの起動
extern void externalStart(HWND hDlg);
//自動コマンド送信機能の外部からの停止
extern void externalStop(HWND hDlg);
//文字列の検索
extern void stringSearch(char *searchLine, unsigned char direction);
// 定着ロググラフのリセット
extern void allReset();
// 使用カウント
extern void useCount(char *func);
// SYSLOGダイアログボックスのハンドル
extern HWND hSyslogDlgWnd;
// ベースウインドウへのハンドル
extern HWND hBaseWindow;
// SYSLOGファイルパス
extern char syslog_file[_T_MAX_PATH];
extern char szIniFilePublic[_T_MAX_PATH];		// Iniファイルパス
extern char logToolSettingPath[_T_MAX_PATH];	// logTool設定ファイルのパス
extern char line_mbuf[];
// Iniファイルパス
extern char szIniFilePrivate[];
// ロググループテーブル
extern char* log_group[];
// ロググループ登録数
extern char log_group_num;
// teratemrm使用
extern bool isTeratermUse;
// ログ表示周期更新
extern bool islogTimerUpdate;
// タイミングチャートのロググループ番号
extern char tcmLogGroupNo;
// ログ表示中フラグ
extern volatile bool isLogUpdate;


const char *unitTypeText[EUNIT_TYPE_NUM] = {
	 "ENGINE"
	,"DPS_ENGINE"
	,"DF"
};

// ツールの終了処理
void FinishHandle()
{
	int cnt;

	// メッセージ定義テーブル解放
	remove_msgtbl();
	// ロググループ文字列テーブル解放
	for(cnt = 0; cnt < log_group_num; cnt++) {
		if(log_group[cnt]) free(log_group[cnt]);
	}
}

//
//  関数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:  メイン ウィンドウのメッセージを処理します。
//
//  WM_COMMAND	- アプリケーション メニューの処理
//  WM_PAINT	- メイン ウィンドウの描画
//  WM_DESTROY	- 中止メッセージを表示して戻る
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// 選択されたメニューの解析:
		switch (wmId)
		{
		// SYSLOG表示
		case IDM_DISP_SYSLOG:
#ifndef SYSLOG_MODELESS
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_SYSLOG), hWnd, DispSyslog);
#else
		    // 二重起動防止
			if(SyslogDisp) break;
			SyslogDisp = TRUE;
			hSyslogDlgWnd = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIALOG_SYSLOG), hWnd, DispSyslog);
			max_logmsg_leng = 0;
#endif
			break;
		// TCMファイル作成
		case IDM_CREATE_TCMLOG:
			SaveTcmlogToFile(hWnd);
			break;
		// ファイルパス設定
		case IDM_SETTING:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_SETTING), hWnd, Setting);
			break;
		// バージョン情報
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		// 終了
		case IDM_EXIT:
			FinishHandle();
			DestroyWindow(hWnd);
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: 描画コードをここに追加してください...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

BOOL getFileTimeStamp(
		const char *Path,
		SYSTEMTIME *lastWriteTime
	 )
{
    // ファイル情報
    WIN32_FIND_DATA findData;
    // ファイル時間
    FILETIME fileTime;
    // ファイル情報取得
    HANDLE hFile = FindFirstFile(Path, &findData);
    if (hFile == INVALID_HANDLE_VALUE)	// 失敗(不正パス)
    {
        return FALSE;
    }
    // ファイル検索ハンドル閉じる
    FindClose(hFile);
    // 更新日時
    FileTimeToLocalFileTime(&findData.ftLastWriteTime, &fileTime);
    FileTimeToSystemTime(&fileTime, lastWriteTime);
    return TRUE;
}

void CALLBACK disp_syslog_timerFunc(
		HWND hwnd,		// handle of window for timer messages 
		UINT uMsg,		// WM_TIMER message 
		UINT idEvent,	// timer identifier 
		DWORD dwTime	// current system time
	 )
{
	#define LOGSTR_LEN		(128)

	// 本関数はタイマーにより一定間隔で呼ばれるが、
	// 前のタイマーで呼ばれたものがまだ動いているときは、何もせずに終了する
	static BOOL isInProcessing = FALSE;
	if (isInProcessing) {
		return;
	}
	isInProcessing = TRUE;

	HWND lbox = GetDlgItem(hwnd, IDC_SYSLOG);

	FILE* stream;
	unsigned char readBuf[7];

	SYSTEMTIME lastWriteTime;
	getFileTimeStamp(syslog_file, &lastWriteTime);

	BOOL nowRcved = FALSE;
	long nowRcved_file_pos = 0L;

	lastUpdateTime = lastWriteTime;

	// ログ取り込み
	stream = _fsopen(syslog_file, "rb", _SH_DENYNO);
	if(stream == (FILE*)NULL) {
		prev_file_pos = 0;
		max_logmsg_leng = 0;
		isInProcessing = FALSE;
		return;
	}

	fseek(stream, prev_file_pos, SEEK_SET);	// 前回の位置まで移動

	while(1) {
		nowRcved_file_pos = ftell(stream);

		// ログブロックの先頭まで読み飛ばし
		if(fread(readBuf, 1, 1, stream) <= 0) {
			break;
		}

		unsigned char read4bit_chk = ((readBuf[0] & 0xf0) >> 4);
		if(read4bit_chk != 0x00 && read4bit_chk != 0x01) {
			// syslogではない
			if((!syslog_output) && (readBuf[0] & 0x80)) {
				// tcmログなら
				fread(readBuf, 1, 7, stream);
			}
			continue;
		}

		// CRLFチェック
		if(readBuf[0] == '\r') {
			if(fread(readBuf, 1, 1, stream) <= 0) {
				break;
			}
			if(readBuf[0] == '\n') {
				continue;
			}
			fseek(stream, -2L, SEEK_CUR);
		}
		else {
			fseek(stream, -1L, SEEK_CUR);
		}

		if(fread(&readBuf[0], 1, 1, stream) <= 0) {
			break;
		}

		char err_lvl = (readBuf[0] & 0x78) >> 3;	// エラーレベル
		if((err_lvl < 0) || (err_lvl > 3)) {
			continue;
		}

		if(fread(&readBuf[1], 1, 4, stream) < 4) {
			nowRcved = TRUE;
			break;
		}

		unsigned long tim = (((readBuf[0] & 0x07) << 24)
					+ (readBuf[1] << 16)
					+ (readBuf[2] <<  8)
					+ readBuf[3]);

		char group_id = (readBuf[4] & 0xfc) >> 2;	// グループID
		if((group_id < 0) || (group_id > log_group_num)) {
			fseek(stream, -4L, SEEK_CUR);
			continue;
		}

		if(fread(&readBuf[5], 1, 2, stream) < 2) {
			nowRcved = TRUE;
			break;
		}

		short log_no = ((readBuf[4] & 0x03) << 8) + readBuf[5];	// ログNo
		char arg_type = (readBuf[6] & 0xf0) >> 4;	// 引数タイプ
		if((arg_type < 0) || (arg_type > 7)) {
			fseek(stream, -6L, SEEK_CUR);
			continue;
		}

		char arg_num = (readBuf[6] & 0x0f);			// 引数の数
		if((arg_num < 0) || (arg_num > 15)) {
			fseek(stream, -6L, SEEK_CUR);
			continue;
		}

		char* msg = get_msgtbl(group_id, log_no, arg_num);

		// ログクリアコマンドを受けた場合
		if(group_id == LOGGRP_DEBUGGER && log_no == 11) {
			SendMessage(lbox, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
			max_logmsg_leng = 0;	// 最大ログ長クリア
			prev_log_cnt = -1;		// 前回ログ数
			ShowScrollBar(lbox, SB_HORZ, FALSE);
			continue;
		}

		// ログ文字列生成
		int max_len = createLogMessage(stream, tim, log_no, err_lvl, group_id, arg_type, arg_num, msg, NULL);

		// 該当メッセージなしの場合
		if(max_len == 0) {
			continue;
		}

		// 表示対象のロググループかのチェック
		if(log_group_disp[group_id] == FALSE) {
			continue;
		}

		// 表示対象のログレベルかのチェック
		if(defLogLvl == LOGLVL_ERROR) {
			if((err_lvl != LOGLVL_ERROR) && (err_lvl != LOGLVL_SYSTEM)) {
				continue;
			}
		} else if(defLogLvl == LOGLVL_WARNING) {
			if(err_lvl == LOGLVL_INFORMATION) {
				continue;
			}
		}

		if(max_len == -1) {
			nowRcved = TRUE;
			break;
		}
		if(max_logmsg_leng < max_len) {
			max_logmsg_leng = max_len;
		}

		// TCM→SYSLOG切り替え
		if((group_id == LOGGRP_DEBUGGER) && (log_no == LOGID_TCM_SYS)) {
			syslog_output = TRUE;
		}

		if(syslog_output) {
			// 画面出力
			if(group_id == LOGGRP_DEBUGGER && (log_no >= 1 && log_no <= 3)) {
				int from_idx = (log_no-1)*15;
				char logstr[LOGSTR_LEN];

				for(int cnt = from_idx; cnt <= (from_idx+14); cnt++) {
					if(log_group_num <= cnt) {
						break;
					}
					// 文字列先頭のログ連番分ずらした位置から出力する。
					sprintf_s(logstr, "%d:%s:%d\n", cnt, log_group[cnt], (line_mbuf[(cnt-from_idx)*2+6]-'0'));
					SendMessage(lbox, LB_ADDSTRING, 0, (LPARAM)logstr);
				}
			} else {
				char* visualLogOffset = strstr(line_mbuf, "VisualLog: ");
				if (visualLogOffset != NULL) {
					if (getVisualLogDisp()) {
						SendMessage(lbox, LB_ADDSTRING, 0, (LPARAM)line_mbuf);
					}
				} else {
					SendMessage(lbox, LB_ADDSTRING, 0, (LPARAM)line_mbuf);
				}
			}
		}

		// SYSLOG→TCM切り替え
		if((group_id == LOGGRP_DEBUGGER) && (log_no == LOGID_SYS_TCM)) {
			syslog_output = FALSE;
		}
	}

	long file_pos = ftell(stream);
	if(prev_file_pos != file_pos) {
		prev_file_pos = file_pos;
		if(nowRcved) {	// ログ途中でちょん切れファイルENDなのでfpはそのログ先頭に。
			prev_file_pos = nowRcved_file_pos;
		}
		int logcnt = SendMessage(lbox, LB_GETCOUNT, 0, 0);
		if(logcnt != LB_ERR && logcnt > 0 && prev_log_cnt != logcnt) {
			prev_log_cnt = logcnt;
			SendMessage(lbox, LB_SETCURSEL, (WPARAM)(logcnt-1), (LPARAM)0);
			setlogCount(logcnt);
		}
	}

	fclose(stream);

	// ListBox横幅以上のメッセージの場合のみ横ScrollBarを出す
	if(max_logmsg_leng >= 0x52) {
		SendMessage(lbox, LB_SETHORIZONTALEXTENT, (WPARAM)((max_logmsg_leng+1)*6), (LPARAM)0);
		ShowScrollBar(lbox, SB_HORZ, TRUE);
	}

	isInProcessing = FALSE;
	return;
}

int DispUsbLog(HWND hDlg)
{
	extern INT_PTR LoadUSBLogFile(HWND hDlg);
	int ret = LoadUSBLogFile(hDlg);
	return ret;
}

int SaveLogBin(HWND hDlg)
{
	extern INT_PTR SaveLogBinFile(HWND hDlg);
	int ret = SaveLogBinFile(hDlg);
	return ret;
}

// ツール開始後にリフレッシュするスレッド
DWORD WINAPI RefleshThread(LPVOID arg)
{
	LPCTSTR ptMsg = _T("　起動中につき少々お待ちください ...");
	HWND lbox = GetDlgItem((HWND)arg, IDC_SYSLOG);
	SendMessage(lbox, LB_ADDSTRING, (WPARAM)0, (LPARAM)_T(""));
	SendMessage(lbox, LB_ADDSTRING, (WPARAM)0, (LPARAM)ptMsg);

	Sleep(3000);
	SendMessage((HWND)arg, WM_COMMAND, IDREFLESH, 0);
	EnableWindow(GetDlgItem((HWND)arg, IDREFLESH), true);
	EnableWindow(GetDlgItem((HWND)arg, IDSAVE), true);
	EnableWindow(GetDlgItem((HWND)arg, IDTCMSAVE), true);
	EnableWindow(GetDlgItem((HWND)arg, IDOK), true);
	EnableWindow(GetDlgItem((HWND)arg, IDM_SAVE_LOGBIN), true);
	return 0;
}

// ListBoxにグループ名登録
void SetGroupName(HWND hDlg)
{
	HWND hGrpSel = GetDlgItem(hDlg, IDC_SELGRP);
	for (int i = 0; i < log_group_num; i++) {
		SendMessage(hGrpSel, LB_ADDSTRING, (WPARAM)0, (LPARAM)log_group[i]);
		SendMessage(hGrpSel, LB_SETSEL, (WPARAM)log_group_disp[i], (LPARAM)i);
	}
	SendMessage(hGrpSel, LB_ADDSTRING, (WPARAM)0, (LPARAM)"TCM");
	SendMessage(hGrpSel, LB_SETSEL, (WPARAM)log_group_disp[tcmLogGroupNo], (LPARAM)tcmLogGroupNo);
}

// 検索Windowのメッセージ ハンドラーです。
INT_PTR CALLBACK stringSearchWindow(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char buffer[100];
	static unsigned char direction = 0; //0:下 1:上
	
	switch (message) {
		case WM_INITDIALOG:
			SendMessage(GetDlgItem(hDlg, ID_SEARCH_DOWN) , BM_SETCHECK , (direction == 0 ? BST_CHECKED : BST_UNCHECKED) , 0);
			SendMessage(GetDlgItem(hDlg, ID_SEARCH_UP) , BM_SETCHECK , (direction == 1 ? BST_CHECKED : BST_UNCHECKED) , 0);
			SetFocus(GetDlgItem(hDlg, ID_STRING)); // フォーカスを合わせる
			break;
		case WM_COMMAND:
			switch (wParam) {
				case IDCANCEL:
					EndDialog(hDlg, LOWORD(FALSE));
					break;
				case IDOK:
					GetWindowText(GetDlgItem(hDlg, ID_STRING), buffer, sizeof(buffer));
					stringSearch(buffer, direction);
					break;
				default:
					break;
			}
			if (HIWORD(wParam) == EN_CHANGE) {
				if (GetKeyState(VK_RETURN) < 0) {
					GetWindowText(GetDlgItem(hDlg, ID_STRING), buffer, sizeof(buffer));
					stringSearch(buffer, direction);
				}
			}
			if (lParam == (LPARAM)GetDlgItem(hDlg, ID_SEARCH_UP)) {
				direction = 1;
			} else if (lParam == (LPARAM)GetDlgItem(hDlg, ID_SEARCH_DOWN)) {
				direction = 0;
			} else {
			}
			break;
		default:
			break;
	}
	return (INT_PTR)FALSE;
}

// syslog表示windowのメッセージ ハンドラーです。
INT_PTR CALLBACK syslogListBox(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char buffer[1024];
	int selectedIndex;
	HGLOBAL hg;
	PTSTR strMem;

	switch (message) {
		case WM_KEYDOWN:
			if (GetKeyState(VK_CONTROL) & 0x8000) {
				if (GetAsyncKeyState('C') & 0x8000) {
					selectedIndex = SendMessage(hDlg, LB_GETCURSEL, 0, 0);
					SendMessage(hDlg, LB_GETTEXT, selectedIndex, (LPARAM)buffer);
					if (OpenClipboard(hDlg)) {
						EmptyClipboard();
						hg = GlobalAlloc(GHND | GMEM_SHARE , 1024);
						strMem = (PTSTR)GlobalLock(hg);
						lstrcpy(strMem , buffer);
						GlobalUnlock(hg);
						SetClipboardData(CF_TEXT, hg);
						CloseClipboard();
					}
				}
				if (GetAsyncKeyState('F') & 0x8000) {
					WINDOWINFO windowInfo;
					GetWindowInfo(hSyslogDlgWnd, &windowInfo);
					MoveWindow(hDlgSearch, windowInfo.rcWindow.left + 200, windowInfo.rcWindow.top + 5, 500, 100, TRUE);
					ShowWindow(hDlgSearch, SW_SHOW);
				}
				if (GetAsyncKeyState('W') & 0x8000) {
					SendMessage(GetDlgItem(hSyslogDlgWnd, IDC_SEND_TEXT), WM_ACTIVATE, 1, 0);
				}
				if (GetAsyncKeyState('R') & 0x8000) {
					OpenSyslogTextApp(hSyslogDlgWnd);
				}
			}
			break;
		default:
			break;
	}
	return (CallWindowProc((WNDPROC)orgWndProc2, hDlg, message, wParam, lParam));
}

// コマンド送信Windowのメッセージ ハンドラーです。
INT_PTR CALLBACK sendtextBox(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (!(GetKeyState(VK_SHIFT) < 0)) {
		switch (message) {
			case WM_KEYUP:
			case WM_KEYDOWN:
				if (wParam == VK_RETURN) {
					return 0;
				}
				break;
			case WM_CHAR:
				if (wParam == VK_RETURN) {
					SendMessage((HWND)hSyslogDlgWnd, WM_COMMAND, IDSEND, 0);
					SendMessage(hDlg, WM_SETTEXT, 0, (LPARAM)"");
					return 0;
				}
				if (wParam == VK_MENU) {
					return 0;
				}
				break;
			case WM_LBUTTONUP:
			case WM_MOUSEMOVE:
				SendMessage((HWND)hSyslogDlgWnd, message, wParam, lParam);
				break;
			default:
				break;
	
		}
	}
	return (CallWindowProc((WNDPROC)orgWndProc, hDlg, message, wParam, lParam));
}
// 表示設定画面のメッセージ ハンドラーです。
INT_PTR CALLBACK dispSetting(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static WINDOWINFO windowInfo;
	int width;
	int height;
	char buf[10];

	switch (message) {
		case WM_INITDIALOG:
			GetWindowInfo(hSyslogDlgWnd, &windowInfo);
			MoveWindow(hDlg, windowInfo.rcWindow.right-14, windowInfo.rcWindow.top, 125, 475, TRUE);
			SetGroupName(hDlg);
			if(defLogLvl == LOGLVL_ERROR) {
				SendMessage(GetDlgItem(hDlg, ID_GRP_ERR), (UINT)BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)0);
			} else if(defLogLvl == LOGLVL_WARNING) {
				SendMessage(GetDlgItem(hDlg, ID_GRP_WARN), (UINT)BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)0);
			} else {
				SendMessage(GetDlgItem(hDlg, ID_GRP_INFO), (UINT)BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)0);
			}
			break;
		case WM_MOVING:
			GetWindowInfo(hSyslogDlgWnd, &windowInfo);
			width = windowInfo.rcWindow.right - windowInfo.rcWindow.left;
			height = windowInfo.rcWindow.bottom - windowInfo.rcWindow.top;
			GetWindowInfo(hDlg, &windowInfo);
			if (windowInfo.rcWindow.left > 30) {
				MoveWindow(hSyslogDlgWnd, windowInfo.rcWindow.left-width+15, windowInfo.rcWindow.top, width, height, TRUE);
			}
			break;
		case WM_COMMAND:
			switch (wParam) {
				case IDCANCEL:
					DestroyWindow(hDlg);
					break;
				case ID_GRP_ERR:	// ログレベル:ERR
					if(defLogLvl != LOGLVL_ERROR) {
						defLogLvl = LOGLVL_ERROR;
					}
					sprintf_s(buf, sizeof(buf), "%d", defLogLvl);
					WritePrivateProfileString(_T("LOGLEVEL"), _T("LOG_LEVEL"), buf, logToolSettingPath);
					SendMessage((HWND)hSyslogDlgWnd, WM_COMMAND, IDREFLESH, 0);
					break;
				case ID_GRP_WARN:	// ログレベル:WARN
					if(defLogLvl != LOGLVL_WARNING) {
						defLogLvl = LOGLVL_WARNING;
					}
					sprintf_s(buf, sizeof(buf), "%d", defLogLvl);
					WritePrivateProfileString(_T("LOGLEVEL"), _T("LOG_LEVEL"), buf, logToolSettingPath);
					SendMessage((HWND)hSyslogDlgWnd, WM_COMMAND, IDREFLESH, 0);
					break;
				case ID_GRP_INFO:	// ログレベル:INFO
					if(defLogLvl != LOGLVL_INFORMATION) {
						defLogLvl = LOGLVL_INFORMATION;
					}
					sprintf_s(buf, sizeof(buf), "%d", defLogLvl);
					WritePrivateProfileString(_T("LOGLEVEL"), _T("LOG_LEVEL"), buf, logToolSettingPath);
					SendMessage((HWND)hSyslogDlgWnd, WM_COMMAND, IDREFLESH, 0);
					break;
				case IDALLGRPSEL:
					all_selchg_flg = !all_selchg_flg;
					for (int i = 0; i <= log_group_num; i++) {
						SendMessage(GetDlgItem(hDlg, IDC_SELGRP), LB_SETSEL, (WPARAM)all_selchg_flg, (LPARAM)i);
						sprintf_s(buf, "LG%02d_SEL", i);
						log_group_disp[i] = SendMessage(GetDlgItem(hDlg, IDC_SELGRP), LB_GETSEL, i, (LPARAM)0);
						if (log_group_disp[i]) {
							WritePrivateProfileString(_T("LOGLEVEL"), _T(buf), "ON", logToolSettingPath);
						} else {
							WritePrivateProfileString(_T("LOGLEVEL"), _T(buf), "OFF", logToolSettingPath);
						}
					}
					return (INT_PTR)TRUE;
				default:
					break;
			}
			if (LOWORD(wParam) == IDC_SELGRP) {
				for (int i = 0; i <= log_group_num; i++) {
					BOOL dummy = log_group_disp[i];
					log_group_disp[i] = SendMessage(GetDlgItem(hDlg, IDC_SELGRP), LB_GETSEL, i, (LPARAM)0);
					if (dummy != log_group_disp[i]) {
						sprintf_s(buf, "LG%02d_SEL", i);
						if (log_group_disp[i]) {
							WritePrivateProfileString(_T("LOGLEVEL"), _T(buf), "ON", logToolSettingPath);
						} else {
							WritePrivateProfileString(_T("LOGLEVEL"), _T(buf), "OFF", logToolSettingPath);
						}
					}
				}
			}
			break;
		default:
			break;
	}
	return (INT_PTR)FALSE;
}
// SYSLOG画面のメッセージ ハンドラーです。
INT_PTR CALLBACK DispSyslog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	#define LIST_BOX_RIGHT 50
	#define LIST_BOX_BOTTOM 135
	#define LIST_BOX_LEFT 17
	#define LIST_BOX_TOP 60
	
	TCHAR iniBuf[30];
	TCHAR logGroupNum[10];
	POINT winPos;
	static WINDOWINFO windowInfo;
	static unsigned long sizeX;
	static unsigned long sizeY;
	static signed short mousePosX;
	static signed short mousePosY;
	static signed short clickPosY;
	static bool winSizeChange = false;
	static signed short sizeOffset = 0;

	HWND hList = GetDlgItem(hDlg, IDC_SYSLOG);
	HMENU hMenu = GetMenu(hDlg);
	int logcnt = 0;

	HANDLE hThread;
	DWORD dwThreadId;

	static HFONT hFont1;
	char sendBoxText[1024];

	//ドラッグ&ドロップしたファイルパス文字列
	_TCHAR strFilePath[_T_MAX_PATH];

	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		//表示
		GetPrivateProfileString(_T("LOG_TOOL"), _T("DISPLAY"), "ON", iniBuf, sizeof(iniBuf), szIniFilePrivate);
		if (strncmp(iniBuf, "ON", 2) == 0) {
			ShowWindow(hDlg, SW_SHOW);
		}
		GetPrivateProfileString(_T("DISP_SETTING"), _T("ACS_AUTO_DISP"), "OFF", iniBuf, sizeof(iniBuf), szIniFilePrivate);
		if (strncmp(iniBuf, "ON", 2) == 0) {
			PostMessage((HWND)hDlg, WM_COMMAND, IDM_COMMAND_SENDER, 0);
		}

		// メッセージテーブル生成
		create_msgtbl();
		// 表示グループ選択
		SetGroupName(hDlg);
		// 前回のログが残ることがあったため強制リフレッシュ起動
		hThread = CreateThread(
			NULL,			//セキュリティ属性
			0,				//スタックサイズ
			RefleshThread,	//スレッド関数
			hDlg,			//スレッド関数に渡す引数
			0,				//作成オプション
			&dwThreadId);	//スレッドID
		DragAcceptFiles(hDlg, TRUE);		//ドロップ可否指定[True:Drop許可]

		for (int i = 0; i <= log_group_num; i++) {
			sprintf_s(logGroupNum, _T("LG%02d_SEL"), i);
			GetPrivateProfileString(_T("LOGLEVEL"), logGroupNum, _T("ON"), iniBuf, sizeof(iniBuf), logToolSettingPath);
			if (strncmp(iniBuf, "ON", 2) == 0) {
				log_group_disp[i] = TRUE;
			} else {
				log_group_disp[i] = FALSE;
			}
		}
		GetPrivateProfileString(_T("LOGLEVEL"), _T("LOG_LEVEL"), _T("2"), iniBuf, sizeof(iniBuf), logToolSettingPath);
		defLogLvl = atoi(iniBuf);

		GetPrivateProfileString(_T("DISP_SETTING"), _T("LOG_TCM_SYNC"), _T("OFF"), iniBuf, sizeof(iniBuf), logToolSettingPath);
		if (strncmp(iniBuf, "ON", 2) == 0) {
			isSynchroLogAndTcm = true;
			CheckMenuItem(hMenu, IDM_CHECK_BOX, MF_BYCOMMAND | MFS_CHECKED);
		} else {
			isSynchroLogAndTcm = false;
			CheckMenuItem(hMenu, IDM_CHECK_BOX, MF_BYCOMMAND | MFS_UNCHECKED);
		}
		GetPrivateProfileString(_T("DISP_SETTING"), _T("TIME_STAMP"), _T("OFF"), iniBuf, sizeof(iniBuf), logToolSettingPath);
		if (strncmp(iniBuf, "ON", 2) == 0) {
			isTimeStamp = true;
			CheckMenuItem(hMenu, IDM_CHECK_TIME_STAMP, MF_BYCOMMAND | MFS_CHECKED);
		} else {
			isTimeStamp = false;
			CheckMenuItem(hMenu, IDM_CHECK_TIME_STAMP, MF_BYCOMMAND | MFS_UNCHECKED);
		}
		
		// サイズ
		GetWindowInfo(hDlg, &windowInfo);
		sizeY = windowInfo.rcWindow.bottom - windowInfo.rcWindow.top - LIST_BOX_BOTTOM;
		sizeX = windowInfo.rcWindow.right - windowInfo.rcWindow.left - LIST_BOX_RIGHT;
		SetWindowPos(GetDlgItem(hDlg, IDC_SYSLOG),HWND_BOTTOM,LIST_BOX_LEFT, LIST_BOX_TOP, sizeX, sizeY, SWP_SHOWWINDOW);
	
		orgWndProc = (FARPROC)GetWindowLong(GetDlgItem(hDlg, IDC_SEND_TEXT), GWL_WNDPROC);
		SetWindowLong(GetDlgItem(hDlg, IDC_SEND_TEXT),GWL_WNDPROC,(LONG)sendtextBox);
		orgWndProc2 = (FARPROC)GetWindowLong(GetDlgItem(hDlg, IDC_SYSLOG), GWL_WNDPROC);
		SetWindowLong(GetDlgItem(hDlg, IDC_SYSLOG),GWL_WNDPROC,(LONG)syslogListBox);

		GetPrivateProfileString(_T("DISP_SETTING"), _T("LOG_UPDATE"), _T("ALWAYS"), iniBuf, sizeof(iniBuf), logToolSettingPath);
		if (strncmp(iniBuf, "ALWAYS", 6) == 0) {
			islogTimerUpdate = false;
			CheckMenuItem(hMenu, IDM_LOGUPDATE_TIMER, MF_BYCOMMAND | MFS_UNCHECKED);
			CheckMenuItem(hMenu, IDM_LOGUPDATE_ALWAYS, MF_BYCOMMAND | MFS_CHECKED);
		} else {
			islogTimerUpdate = true;
			CheckMenuItem(hMenu, IDM_LOGUPDATE_ALWAYS, MF_BYCOMMAND | MFS_UNCHECKED);
			CheckMenuItem(hMenu, IDM_LOGUPDATE_TIMER, MF_BYCOMMAND | MFS_CHECKED);
		}
		SetTimer(hDlg, TIMER_ID_LOGUPDATE, UPDATE_TIMING, NULL);

		hDlgAcs = CreateDialog(hInst, MAKEINTRESOURCE(IDD_AUTO_COMMAND_SEND), hDlg, autoCommandSenderProc);
		hDlgSearch = CreateDialog(hInst, MAKEINTRESOURCE(IDD_STRING_SEARCH), hDlg, stringSearchWindow);

		GetPrivateProfileString(_T("PROJECT"), _T("UNIT"), _T("ENGINE"), buf, sizeof(buf), szIniFilePrivate);
		for (int i = 0; i < EUNIT_TYPE_NUM; i++) {
			if (strncmp(buf, unitTypeText[i], strlen(unitTypeText[i])) == 0) {
				unit = (eUNIT_TYPE)(i);
				break;
			}
		}
		DeleteMenu(hMenu, IDM_MACRO_START, MF_BYCOMMAND);
		DeleteMenu(hMenu, IDM_MACRO_STOP, MF_BYCOMMAND);
		if (unit != EUNIT_TYPE_ENGINE) {
			DeleteMenu(hMenu, IDM_DISP_FIXLOG, MF_BYCOMMAND);
		}
		useCount("LOGTOOL");
        break;
	case WM_SIZE:
		GetWindowInfo(hDlg, &windowInfo);
		sizeY = windowInfo.rcWindow.bottom - windowInfo.rcWindow.top - LIST_BOX_BOTTOM;
		sizeX = windowInfo.rcWindow.right - windowInfo.rcWindow.left - LIST_BOX_RIGHT;
		SetWindowPos(GetDlgItem(hDlg, IDC_SYSLOG),HWND_BOTTOM,LIST_BOX_LEFT, LIST_BOX_TOP + sizeOffset, sizeX, sizeY - sizeOffset, SWP_SHOWWINDOW);
		SetWindowPos(GetDlgItem(hDlg, IDSEND),HWND_BOTTOM,sizeX-35, 12, 51, 38, SWP_SHOWWINDOW);
		SetWindowPos(GetDlgItem(hDlg, IDC_SEND_TEXT),HWND_BOTTOM,17, 12, sizeX - 66, 35 + sizeOffset, SWP_SHOWWINDOW);
		GetWindowInfo(hDlg, &windowInfo);
		MoveWindow(logLevelDlgWnd, windowInfo.rcWindow.right-14, windowInfo.rcWindow.top, 125, 475, TRUE);
		return (INT_PTR)TRUE;
	case WM_MOUSEMOVE:	//マウスを動かしたとき
		mousePosX = (signed short)LOWORD(lParam);
		mousePosY = (signed short)HIWORD(lParam);
		if (winSizeChange) {
			sizeOffset += mousePosY - clickPosY;
			clickPosY = mousePosY;
			SetWindowPos(GetDlgItem(hDlg, IDC_SYSLOG),HWND_BOTTOM,LIST_BOX_LEFT, LIST_BOX_TOP + sizeOffset, sizeX, sizeY - sizeOffset, SWP_SHOWWINDOW);
			SetWindowPos(GetDlgItem(hDlg, IDSEND),HWND_BOTTOM,sizeX-35, 12, 51, 38, SWP_SHOWWINDOW);
			SetWindowPos(GetDlgItem(hDlg, IDC_SEND_TEXT),HWND_BOTTOM,17, 12, sizeX - 66, 35 + sizeOffset, SWP_SHOWWINDOW);
		}
		break;
	case WM_LBUTTONDOWN:
		GetWindowInfo(GetDlgItem(hDlg, IDC_SEND_TEXT), &windowInfo);
		winPos.y = windowInfo.rcWindow.bottom;
		winPos.x = windowInfo.rcWindow.left;
		ScreenToClient(hDlg, &winPos);
		if (mousePosY > winPos.y) {
			GetWindowInfo(GetDlgItem(hDlg, IDC_SEND_TEXT), &windowInfo);
			winPos.x = windowInfo.rcWindow.right;
			GetWindowInfo(GetDlgItem(hDlg, IDC_SYSLOG), &windowInfo);
			winPos.y = windowInfo.rcWindow.top;
			ScreenToClient(hDlg, &winPos);
			if (mousePosY < winPos.y) {
				winSizeChange = true;
				clickPosY = mousePosY;
			}
		}
		break;
	case WM_LBUTTONUP:
		if (winSizeChange) {
			winSizeChange = false;
		}
		break;
	case WM_MOVING:
		GetWindowInfo(hDlg, &windowInfo);
		MoveWindow(logLevelDlgWnd, windowInfo.rcWindow.right-14, windowInfo.rcWindow.top, 125, 475, TRUE);
		break;
	case WM_DRAWITEM:
#if 0		// Owner Draw = Fixed
		if((UINT)wParam==IDC_SYSLOG) {
			HBRUSH hBrush =  NULL;
			WCHAR strText[256];
			LPDRAWITEMSTRUCT _DrawItem=(LPDRAWITEMSTRUCT)lParam;

			// 描画対象の文字列取得
			hList = GetDlgItem(hDlg, IDC_SYSLOG);
			SendMessage(hList,LB_GETTEXT,_DrawItem->itemID,(LPARAM)&strText);

			// 選択状態かチェック
			if((_DrawItem->itemState) & (ODS_SELECTED)) {
				// ブラシ作成
				hBrush = CreateSolidBrush(RGB(255,0,0));
				// テキストのカラーを設定
				SetTextColor(_DrawItem->hDC,RGB(255,255,255));
				// テキストの背景色を指定
				SetBkColor(_DrawItem->hDC,RGB(255,0,0));
			}
			else {
				// ブラシ作成
				hBrush = CreateSolidBrush(RGB(255,255,255));
				// テキストのカラーを設定
				SetTextColor(_DrawItem->hDC,RGB(0,0,0));
				// テキストの背景色を指定
				SetBkColor(_DrawItem->hDC,RGB(255,255,255));
			}

			// バックグラウンドを塗りつぶす
			FillRect(_DrawItem->hDC,&_DrawItem->rcItem,hBrush);
			// ブラシ削除
			DeleteObject(hBrush);

			// テキストを描画       
			TextOut(_DrawItem->hDC,_DrawItem->rcItem.left+22,_DrawItem->rcItem.top+3,strText,_tcslen(strText));
		}
#endif
		break;

	//Dropイベント
	case WM_DROPFILES:
		DragQueryFile((HDROP)wParam, NULL, strFilePath, _T_MAX_PATH);
		DragFinish((HDROP)wParam);

		//解凍および変換処理を行う
		unzipConvertLogFile(hDlg, strFilePath);
		break;
	case WM_TIMER:
		if (wParam == TIMER_ID_LOGUPDATE) {
			if (islogTimerUpdate) {
				if (!isLogUpdate) {
					isTimeLogUpdate = true;
					updateLog();
					isTimeLogUpdate = false;
				}
			}
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			SendMessage(hList, LB_RESETCONTENT, 0, 0);
			logFileClose();
			setlogCount(0);
			logFileOpen();
			return (INT_PTR)TRUE;
		case IDCANCEL:
		case IDM_LOG_END:
			remove_msgtbl();
			KillTimer(hDlg, 1);
#ifndef SYSLOG_MODELESS
			EndDialog(hDlg, LOWORD(wParam));
#else
			ShowWindow(hDlg, SW_HIDE);
			SyslogDisp = FALSE;
#endif
			SendMessage(hBaseWindow, WM_COMMAND, IDM_EXIT, 0);
			return (INT_PTR)TRUE;
		case IDSAVE:
			return SaveSyslogToFile(hDlg);
		case IDTCMSAVE:
			return SaveTcmlogToFile(hDlg);
		case ID_OPEN_TEXT_APP:
			return OpenSyslogTextApp(hDlg);
		case IDSEND:
			GetDlgItemText(hDlg, IDC_SEND_TEXT, sendBoxText, sizeof(sendBoxText));
			sendText(sendBoxText);
			return (INT_PTR)TRUE;
		case IDREFLESH:
			KillTimer(hDlg, 1);
			SendMessage(hList, LB_RESETCONTENT, 0, 0);

			ZeroMemory(&lastUpdateTime, sizeof(SYSTEMTIME));
			prev_file_pos = 0L;		// 前回までのファイル位置クリア
			max_logmsg_leng = 0;	// 最大ログ長クリア
			prev_log_cnt = -1;		// 前回ログ数
			if (isTeratermUse) {
				SetTimer(hDlg, 1, UPDATE_TIMING, disp_syslog_timerFunc);	// タイマーのセット
			} else {
				logFileClose();
				allReset();
				disp_syslog_timerFunc(hDlg, message, wParam, lParam);
				logFileReOpen();
				isLogStartConvert = true;
			}
			return (INT_PTR)TRUE;
		case IDM_SAVE_LOGBIN:
			KillTimer(hDlg, 1);
			SaveLogBin(hDlg);
			if (isTeratermUse) {
				SetTimer(hDlg, 1, UPDATE_TIMING, disp_syslog_timerFunc);	// タイマーのセット
			}
			return (INT_PTR)TRUE;
		case IDM_CHECK_BOX:
			{
				UINT uState = GetMenuState(hMenu, IDM_CHECK_BOX, MF_BYCOMMAND);
				if (uState & MFS_CHECKED) {
					CheckMenuItem(hMenu, IDM_CHECK_BOX, MF_BYCOMMAND | MFS_UNCHECKED);
					isSynchroLogAndTcm = false;
					WritePrivateProfileString(_T("DISP_SETTING"), _T("LOG_TCM_SYNC"), "OFF", logToolSettingPath);
				} else {
					CheckMenuItem(hMenu, IDM_CHECK_BOX, MF_BYCOMMAND | MFS_CHECKED);
					isSynchroLogAndTcm = true;
					WritePrivateProfileString(_T("DISP_SETTING"), _T("LOG_TCM_SYNC"), "ON", logToolSettingPath);
				}
			}
			break;
		case IDM_CHECK_TIME_STAMP:
			{
				UINT uState = GetMenuState(hMenu, IDM_CHECK_TIME_STAMP, MF_BYCOMMAND);
				if (uState & MFS_CHECKED) {
					CheckMenuItem(hMenu, IDM_CHECK_TIME_STAMP, MF_BYCOMMAND | MFS_UNCHECKED);
					WritePrivateProfileString(_T("DISP_SETTING"), _T("TIME_STAMP"), "OFF", logToolSettingPath);
					isTimeStamp = false;
				} else {
					CheckMenuItem(hMenu, IDM_CHECK_TIME_STAMP, MF_BYCOMMAND | MFS_CHECKED);
					WritePrivateProfileString(_T("DISP_SETTING"), _T("TIME_STAMP"), "ON", logToolSettingPath);
					isTimeStamp = true;
				}
			}
			break;
		case IDM_LOGUPDATE_ALWAYS:
			CheckMenuItem(hMenu, IDM_LOGUPDATE_TIMER, MF_BYCOMMAND | MFS_UNCHECKED);
			CheckMenuItem(hMenu, IDM_LOGUPDATE_ALWAYS, MF_BYCOMMAND | MFS_CHECKED);
			WritePrivateProfileString(_T("DISP_SETTING"), _T("LOG_UPDATE"), "ALWAYS", logToolSettingPath);
			islogTimerUpdate = false;
			break;
		case IDM_LOGUPDATE_TIMER:
			CheckMenuItem(hMenu, IDM_LOGUPDATE_ALWAYS, MF_BYCOMMAND | MFS_UNCHECKED);
			CheckMenuItem(hMenu, IDM_LOGUPDATE_TIMER, MF_BYCOMMAND | MFS_CHECKED);
			WritePrivateProfileString(_T("DISP_SETTING"), _T("LOG_UPDATE"), "TIMER", logToolSettingPath);
			islogTimerUpdate = true;
			break;
		case IDM_DISP_VERINFO:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hDlg, About);
			break;
		case IDM_DISP_FIXLOG:
			fixLogMain(hDlg);
			useCount("FIXLOG");
			return (INT_PTR)TRUE;
		case IDM_VISUAL_ANALYZER:
			visualAnalyzMain(hDlg);
			break;
		case IDM_TIMING_CHART:
			timingChartMain(hDlg);
			useCount("TIMING_CHART");
			return (INT_PTR)TRUE;
		case IDM_DOWNLOAD:
			if (TRUE == DialogBox(hInst, MAKEINTRESOURCE(IDD_DL_PASSWORD), hDlg, passWord)) {
				CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIALOG_DL), hDlg, download);
				useCount("DOWNLOAD");
			}
			return (INT_PTR)TRUE;
		case IDM_COM_SETTING:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_COM_SETTING), hDlg, comSetting);
			useCount("COM_SETTING");
			return (INT_PTR)TRUE;
		case IDM_DISP_SETTING:
			logLevelDlgWnd = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DISP_SETTING), hDlg, dispSetting);
			useCount("DISP_SETTING");
			return (INT_PTR)TRUE;
		case IDM_COMMAND_SENDER:
			ShowWindow(hDlgAcs, SW_SHOW);
			useCount("AUTO_CMMMACD_SEND");
			return (INT_PTR)TRUE;
		case IDM_DATA_SETTING:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_USE_COUNT), hDlg, dateSetting);
			return (INT_PTR)TRUE;
		case IDM_MACRO_START:
			if (hDlgAcs != NULL) {
				externalStart(hDlgAcs);
			}
			return (INT_PTR)TRUE;
		case IDM_MACRO_STOP:
			if (hDlgAcs != NULL) {
				externalStop(hDlgAcs);
			}
			return (INT_PTR)TRUE;
		default:
			break;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

// 設定ダイアログのメッセージ ハンドラーです。
INT_PTR CALLBACK Setting(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	TCHAR buf[1024];

	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		SetWindowText(GetDlgItem(hDlg, IDC_EDIT_SYSLOG_FIL), syslog_file);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			GetWindowText(GetDlgItem(hDlg, IDC_EDIT_SYSLOG_FIL), buf, sizeof(buf));
			WritePrivateProfileString(_T("FILE"), _T("SYSLOGFILE"), syslog_file, szIniFilePrivate);
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDFSEL1)
		{
			return GetSettingFileName(hDlg, 0);
		}
		else if (LOWORD(wParam) == IDFSEL2)
		{
			return GetSettingFileName(hDlg, 1);
		}
		break;
	}
	return (INT_PTR)FALSE;
}

// データ設定の ハンドラーです。
INT_PTR CALLBACK dateSetting(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
		case WM_INITDIALOG:
			SetWindowText(GetDlgItem(hDlg, ID_USE_COUNT_URL), useCountUpPath);
			return (INT_PTR)TRUE;
		case WM_COMMAND:
			switch (wParam) {
				case IDCANCEL:
					EndDialog(hDlg, LOWORD(FALSE));
					break;
				case IDOK:
					GetWindowText(GetDlgItem(hDlg, ID_USE_COUNT_URL), useCountUpPath, sizeof(useCountUpPath));
					if (useCountUpPath[0] == 0) {
						WritePrivateProfileString(_T("FILE"), _T("CONNECT_PATH"), NULL, logToolSettingPath);
						GetPrivateProfileString(_T("FILE"), _T("CONNECT_PATH"), logToolSettingPath, useCountUpPath, sizeof(useCountUpPath), szIniFilePrivate);
					} else {
						WritePrivateProfileString(_T("FILE"), _T("CONNECT_PATH"), useCountUpPath, logToolSettingPath);
					}
					EndDialog(hDlg, LOWORD(FALSE));
					break;
				default: 
					break;
			}
			break;
	}
	return (INT_PTR)FALSE;
}

// バージョン情報ボックスのメッセージ ハンドラーです。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hTextMsgFile = GetDlgItem(hDlg, IDC_MSG_VERSION);

	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		SetWindowText(hTextMsgFile, logmsg_mkdate);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
