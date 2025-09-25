// LogTool.cpp : アプリケーションのエントリ ポイントを定義します。
//

//===========================================================================================
//	Ver2.00	2015/10/16	新規作成？
//===========================================================================================
//	Ver2.01	2016/01/30	M.Hashimoto
//		[USBログ表示]Click時処理を改造
//			改造前)*.logを選択⇒*.tcdへ変換する
//			改造後)圧縮ファイル(*.gz)を選択⇒解凍⇒*.tcdへ変換
//				　 *.log選択時は、改造前同様*.log⇒*.tcd変換を行う
//-------------------------------------------------------------------------------------------
//	Ver2.02	2016/02/02	M.Hashimoto
//		[USBログ表示]Click時処理を改造
//			・圧縮ファイルがあるフォルダへ解凍した"dev_proc_edbg_all.log"を保存
//			・"dev_proc_edbg_all.log"コピー後、解凍データ削除
//			・ドラッグ＆ドロップ対応
//-------------------------------------------------------------------------------------------
//	Ver2.03	2016/02/03	M.Hashimoto
//		ドラッグ＆ドロップ処理に[テキストで保存]Click時処理を組込
//-------------------------------------------------------------------------------------------
//	Ver2.04	2016/02/04	M.Hashimoto
//		1)*.tcd及び*.logのデフォルトファイル名を*.tar.gzの*の部分を使用するよう修正
//		2)解凍フォルダのトップフォルダは"log"固定の為、USB Log圧縮ファイル解凍処理(USBFileDecomp)修正
//-------------------------------------------------------------------------------------------
//	Ver2.05	2016/02/08	M.Hashimoto
//		1)LogTool.exeファイルにドラッグ＆ドロップで処理実行
//-------------------------------------------------------------------------------------------
//	Ver2.06	2016/02/08	M.Hashimoto
//		1)LogTool.exeファイルにドラッグ＆ドロップした場合、処理終了後画面閉じる
//		2)圧縮ファイル名が長い場合エラー発生していた為修正
//		3)ログファイルの内容が1行が256文字以上の場合エラー発生していた為2048文字まで対応可能となるよう修正
//-------------------------------------------------------------------------------------------
//	Ver2.07	2016/02/16	M.Hashimoto
//		1)Dropにて処理行う際、dev_proc_edbg_all.logのEngSoftのバージョンに対応した
//		  各種設定ファイルを再読込する
//		  ■■注意■■
//		  各Verに対する設定ファイルは、[SettingData\2RB_1000.001.013\tool_public.ini]のように
//		  SettinDataフォルダの下の各バージョン単位で作成されたフォルダに格納されている事
//		  ファイルが存在しない場合、デフォルト読み込みされるファイルを使用する
//		2)[バージョン情報]ボタンClick時に表示される画面のバージョン情報を編集
//		　これに伴い、今回の一連のVerUpを1.01～としていたが、2.01～に修正
//-------------------------------------------------------------------------------------------
//	Ver2.08	2016/02/16	M.Hashimoto
//		1)圧縮ファイルに空白が含まれると、Tarコマンドが正常に動作しない不具合対応
//-------------------------------------------------------------------------------------------
//	Ver2.09	2016/02/17	M.Hashimoto
//		1)プロジェクト振替ができるよう、振替設定INIファイル作成
//-------------------------------------------------------------------------------------------
//	Ver2.10	2016/02/25	M.Hashimoto
//		1)全解凍⇒ファイル指定解凍を行うよう修正
//			ファイル指定の解凍例)tar -xvzf log.tar.gz log/files/debug_file/dev_proc_edbg_all.log
//		2)Trim()処理内のstrcpy_sでのサイズ指定により、getLogSoftVer()にてRunTimeエラー発生
//		　Run-Time Check Failure #2 - Stack around the variable 'GetData' was corrupted.
//			下記指定に変更
//			strcpy_s(s, 100, &s[i]);	⇒　strcpy_s(s, SOFTVER_BUFFER_SZ, &s[i]);
//===========================================================================================

#include "stdafx.h"
#include "LogTool.h"
#include "saveTcdFile.h"
#include <process.h>
#include <ShlObj.h>

#include <stdio.h>
#include <share.h>
#include <locale.h>

//strstr関数、strcmp関数利用の為
#include <string.h>
//
#include <windows.h>
#include <shlwapi.h>
#include <shellapi.h>

//_splitpath_s関数利用の為
#include <stdlib.h>

#include "CommDlg.h"

// グローバル変数:
extern HINSTANCE hInst;					// 現在のインターフェイス
HWND hBaseWindow;						// ベースウインドウへのハンドル
HWND hSyslogDlgWnd;						// SYSLOGダイアログボックスのハンドル

char syslog_file[_T_MAX_PATH];			// SYSLOGファイルパス
char szIniFilePublic[_T_MAX_PATH];		// Iniファイルパス
char szIniFilePrivate[_T_MAX_PATH];		// Iniファイルパス
char visualAnalyzerIni[_T_MAX_PATH];	// Iniファイルパス
char visualAnalyzerName[_T_MAX_PATH];	// ファイル名
char logToolSettingPath[_T_MAX_PATH];	// logTool設定ファイルのパス
char useCountUpPath[_T_MAX_PATH];		// 利用状況カウントアップパス
char szPulicIniFileName[_T_MAX_PATH];	// public.iniファイル名
char connectCheckFileName[_T_MAX_PATH];	// サーバー接続確認ファイルパス
char countInfoFileName[_T_MAX_PATH];	// カウント情報ファイルパス
char allCountInfoFileName[_T_MAX_PATH];	// カウント情報ファイルパス
char captionName[_T_MAX_PATH];
char buf[256];
char usbLogStartStrBuf[USBLOG_PARTITION_BUF] = { 0 };
char usbLogEndStrBuf[LOG_PARTITION_NUM][USBLOG_PARTITION_BUF] = { 0 };
char usbLogSepStrBuf[LOG_PARTITION_NUM][USBLOG_PARTITION_BUF] = { 0 };
char usbLogTcmEndStrBuf[LOG_PARTITION_NUM][USBLOG_PARTITION_BUF] = { 0 };
unsigned char usblogEndStrNum;
unsigned char usblogSepStrNum;
unsigned char usblogTcmEndStrNum;
char isConnectCountInfoFile = 0;
char tcmLogGroupNo;						//タイミングチャートのロググループ番号
eUNIT_TYPE unit;

TCHAR szWindowsTempPath[_T_MAX_PATH];	// Windowsテンポラリパス
TCHAR engnFileNameEntry1[64];			// USBログ内のエンジンログファイル名候補1
TCHAR engnFileNameEntry2[64];			// USBログ内のエンジンログファイル名候補2

// コマンドライン引数ファイル名
// LogTool.exeへ直接Drop時、コマンドライン引数にDropされたファイル名が入る
char szCmdLineFileName[_T_MAX_PATH];
char szCmdLine2FileName[_T_MAX_PATH];

OPENFILENAME ofn;
TCHAR filename_full[_T_MAX_PATH];		// ファイル名(フルパス)を受け取る領域
TCHAR filename[_T_MAX_PATH];			// ファイル名を受け取る領域

extern void terminalMain();
extern void terminalClose();
extern void logFileClose();
extern void logFileReOpen();
extern void writeUsbLogBin(unsigned char* logBuf, size_t len);
extern void setVisualAnalyzerMode(char* syslog_file);
// ログメッセージ生成
extern int createLogMessage(FILE*, unsigned long, short, char, char, char, char, char*, unsigned char*);

// ログメッセージバッファ
extern char line_mbuf[2048];

// ログ長
#define GET_LOG_LENGTH(dat) \
			(((((dat & 0xf0) >> 4) == 1) || (((dat & 0xf0) >> 4) == 2))? (1 * (dat & 0x0f)): \
			(((((dat & 0xf0) >> 4) == 4) || (((dat & 0xf0) >> 4) == 5))? (2 * (dat & 0x0f)): \
			(((((dat & 0xf0) >> 4) == 6) || (((dat & 0xf0) >> 4) == 7))? (4 * (dat & 0x0f)): 0)))

#ifndef null_ptr 
#define null_ptr 0 
#endif 

char DefaltDir[] = "SettingData\\";

char	TransferPJ[TRANFER_PJ_MAX][10][4];	//振替プロジェクト
int		TransPJ_cnt;						//振替プロジェクト件数

// _tsplitpath_s呼出し時の情報取得用バッファ
_TCHAR szMyPath[_T_MAX_PATH];		// 自プロセスパス
_TCHAR szGlDrive[_T_MAX_DRIVE];
_TCHAR szGlPath[_T_MAX_DIR];
_TCHAR szGlFName[_T_MAX_FNAME];
_TCHAR szGlExt[_T_MAX_EXT];
// _tsplitpath_s(第二引数用)呼出し時の情報取得用バッファ
_TCHAR szGlDrive2[_T_MAX_DRIVE];
_TCHAR szGlPath2[_T_MAX_DIR];
_TCHAR szGlFName2[_T_MAX_FNAME];
_TCHAR szGlExt2[_T_MAX_EXT];

extern volatile bool isLogStartConvert;

//==================================================
//	Main関数(_tWinMain)
//--------------------------------------------------
//	<引数>
//	hInstance		:現在のインスタンス・ハンドル
//	hPrevInstance	:以前のインスタンス・ハンドル
//	lpCmdLine		:コマンドラインの文字列
//	nCmdShow		:ウインドウの表示状態
//--------------------------------------------------
//	<処理概要>
//	・二重起動防止確認
//	・コマンドライン引数確認
//	・EXEへファイルDrop時の処理
//	・アプリ初期化
//	・Windowイベント監視処理
//==================================================
unsigned __stdcall ThreadFunc(void *p)
{
	terminalMain();
	return 0;
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
	char projectCode[100];
	char mutexName[256];
	char connectFlag[5];
	//コマンドライン引数が指定されている場合、取得したファイルパスを分割
	GetArgFileName(lpCmdLine);

	//実行中のプロセスのフルパス名を取得
	GetModuleFileName(NULL, szMyPath, sizeof(szMyPath));
	// フルパス名を分割
	splitpath(szMyPath);
	sprintf_s(szMyPath, "%s%s", szGlDrive, szGlPath);
	if (szPulicIniFileName[0] != 0x00) {
		sprintf_s(szIniFilePublic, "%s%s%s", szGlDrive, szGlPath, szPulicIniFileName);
	} else {
		sprintf_s(szIniFilePublic, "%s%stool_public_target.ini", szGlDrive, szGlPath);
	}
	sprintf_s(szIniFilePrivate, "%s%stool_private.ini", szGlDrive, szGlPath);
	sprintf_s(logmsg_file, _T_MAX_PATH, "%sDebuggerLogMsg.mes", szMyPath);// メッセージ定義ファイル
	sprintf_s(visualAnalyzerIni, "%s%sVisualAnalyzer.ini", szGlDrive, szGlPath);
	GetPrivateProfileString(_T("PROJECT"), _T("CODE"), _T("XXX"), projectCode, sizeof(projectCode), szIniFilePrivate);
	sprintf_s(mutexName, sizeof(mutexName), "__LogTool__%s", projectCode);
	sprintf_s(captionName, sizeof(captionName), "Syslog表示ツール-%s",projectCode);
	SHGetSpecialFolderPath( NULL, logToolSettingPath, CSIDL_PERSONAL, 0 );//マイドキュメントのパス
	sprintf_s(logToolSettingPath, sizeof(logToolSettingPath), "%s\\logTool_%s.ini", logToolSettingPath, projectCode);
	GetPrivateProfileString(_T("FILE"), _T("CONNECT_PATH"), logToolSettingPath, useCountUpPath, sizeof(useCountUpPath), szIniFilePrivate);
	GetPrivateProfileString(_T("FILE"), _T("CONNECT_PATH"), useCountUpPath, useCountUpPath, sizeof(useCountUpPath), logToolSettingPath);
	sprintf_s(connectCheckFileName, sizeof(connectCheckFileName), "%s\\CONNECT_CHECK.ini", useCountUpPath);
	GetPrivateProfileString(_T("CHECK"), _T("CONNECT"), _T("0"), connectFlag, sizeof(connectFlag), connectCheckFileName);
	isConnectCountInfoFile = atoi(connectFlag);
	if (isConnectCountInfoFile == 1) {
		sprintf_s(allCountInfoFileName, sizeof(allCountInfoFileName), "%s\\LOG_TOOL_COUNT_INFO.ini", useCountUpPath);
		sprintf_s(countInfoFileName, sizeof(countInfoFileName), "%s\\LOG_TOOL_COUNT_INFO_%s.ini", useCountUpPath,projectCode);
	} else {
		sprintf_s(countInfoFileName, sizeof(countInfoFileName), "%s", logToolSettingPath);
	}
	GetPrivateProfileString(_T("PATH"), _T("VISUAL_ANALYZER"), visualAnalyzerIni, visualAnalyzerIni, sizeof(visualAnalyzerIni), logToolSettingPath);
	GetPrivateProfileString(_T("PATH"), _T("VISUAL_ANALYZER"), visualAnalyzerIni, visualAnalyzerIni, sizeof(visualAnalyzerIni), szIniFilePrivate);
	SetCurrentDirectory(szGlPath);
	_fullpath(visualAnalyzerIni, visualAnalyzerIni, _T_MAX_PATH);
	// 二重起動防止
	HANDLE hMutex = CreateMutex(NULL, TRUE, mutexName);
	if(GetLastError() == ERROR_ALREADY_EXISTS) return FALSE;

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;
	setlocale(LC_ALL,"japanese"); 

	// グローバル文字列を初期化しています。
	MyRegisterClass(hInstance);

	// アプリケーションの初期化を実行します:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, NULL);
	_beginthreadex(NULL, 0, ThreadFunc, NULL, 0, NULL);

	// メイン メッセージ ループ:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			// コマンドラインにUSBログファイルを指定して起動した場合に変換を実施する
			if (strcmp(szCmdLine2FileName, "")) {
				if (isLogStartConvert) {
					unzipConvertLogFile(hSyslogDlgWnd, szCmdLine2FileName);
					isLogStartConvert = false;
					break;
				}
			}
		}
	}

	ReleaseMutex(hMutex);
	logFileClose();
	terminalClose();
	return (int) msg.wParam;
}

//==================================================
//	MyRegisterClass()
//--------------------------------------------------
//	<引数>
//	hInstance	:
//--------------------------------------------------
//	<処理概要>
//	ウィンドウ クラスを登録
//--------------------------------------------------
//	<コメント>
//    この関数および使い方は、'RegisterClassEx' 関数が追加された
//    Windows 95 より前の Win32 システムと互換させる場合にのみ必要です。
//    アプリケーションが、関連付けられた
//    正しい形式の小さいアイコンを取得できるようにするには、
//    この関数を呼び出してください。
//==================================================
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LOGTOOL));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= TEXT("SyslogTool");
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//==================================================
//	InitInstance()
//--------------------------------------------------
//	<引数>
//	hInstance	:
//	nCmdShow	:
//--------------------------------------------------
//	<処理概要>
//	インスタンス ハンドルを保存して、メイン ウィンドウを作成
//--------------------------------------------------
//	<コメント>
//	この関数で、グローバル変数でインスタンス ハンドルを保存し、
//	メイン プログラム ウィンドウを作成および表示します。
//==================================================
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;
   	hInst = hInstance; // グローバル変数にインスタンス処理を格納します。

	_TCHAR szDrive[_T_MAX_DRIVE];
	_TCHAR szDir[_T_MAX_DIR];
	_TCHAR szTemp[_T_MAX_FNAME];


//他処理からもログファイルデータ取得を行うため関数化
/*
	int cnt = 0;
	TCHAR subsec[16];
	extern BOOL log_group_disp[];
	extern char defLogLvl;
*/
	// Windowsテンポラリパス取得(USBログ解凍用)
	GetTempPath(_T_MAX_PATH, szWindowsTempPath);



	// USBログファイル内のエンジンログファイル名候補取得
	GetPrivateProfileString(_T("USBLOG_SETTING"), _T("ENGN_LOG_FILE1"),
		_T(USBLOG_DEFAULT_ENGINE_FILE), engnFileNameEntry1, sizeof(engnFileNameEntry1), szIniFilePublic);
	GetPrivateProfileString(_T("USBLOG_SETTING"), _T("ENGN_LOG_FILE2"),
		_T(USBLOG_DEFAULT_ENGINE_FILE), engnFileNameEntry2, sizeof(engnFileNameEntry2), szIniFilePublic);

	//---------------------------------------------------------
	//プロジェクト振替設定INIファイル読込
	//---------------------------------------------------------
	char	szSettingIniFile[_T_MAX_PATH];			//LogTool設定用INIファイル
	char	szKey[16];								//INIファイル読込時Key指定用
	char	szBuff[100];							//INIファイル取得結果バッファ用
	int		i = 0;
	int		j;

	char	*delim = ",\n";			//デリミタ（複数渡せる）ここではカンマと改行コード
	char	*ctx;					//内部的に使用するので深く考えない 
	char	*next;

	//振替プロジェクト退避領域初期化
	for (i = 0; i < TRANFER_PJ_MAX; i++) {
		for (j = 0; j < 10; j++) {
			TransferPJ[i][j][0] = '\0';
		}
	}

	TransPJ_cnt = 0;								//振替プロジェクト件数初期化
	sprintf_s(szSettingIniFile, "%s%sLogToolSetting.ini", szGlDrive, szGlPath);
	for(i = 0; i < TRANFER_PJ_MAX; i++) {
		sprintf_s(szKey, _T("TRANS_PJ%02d"), i);
		//INIファイルデータ取得(セクション=[TRANSFER_PJ],キー名=TRANS_PJXX,デフォルト値="***")
		GetPrivateProfileString(_T("TRANSFER_PJ"), szKey, _T("***"), szBuff, sizeof(szBuff), szSettingIniFile);
		if(!memcmp(szBuff, "***", 3)) break;		//INIファイルよりデータ取得できなかった場合、For文終了

		//取得結果をカンマ分割する
		j = 0;
		next = strtok_s(szBuff, delim, &ctx); 
		while(next){ 
			strcpy_s(TransferPJ[i][j], next);
			TransferPJ[i][j][3] = '\0';
			next = strtok_s(null_ptr, delim, &ctx); 
			j++;
		} 
		TransPJ_cnt++;								//振替プロジェクト件数
	}

//他処理からもログファイルデータ取得を行うため処理を関数化
	//設定値取得
	if (getSettingData() == FALSE) {
		MessageBox(NULL, TEXT("実行環境を確認してください。"), TEXT("実行環境異常"), (MB_OK | MB_ICONERROR));
		return FALSE;
	}

	// ウィンドウの作成位置を計算する
	RECT g_windowPos;	// ウィンドウの位置
	g_windowPos.left = GetSystemMetrics( SM_CXSCREEN ) + 100;
	g_windowPos.top = GetSystemMetrics( SM_CYSCREEN ) + 100;
	g_windowPos.right = g_windowPos.left + WINDOW_WIDTH;
	g_windowPos.bottom = g_windowPos.top + WINDOW_HEIGHT;

	hWnd = CreateWindow(
				TEXT("SyslogTool"),			// ウィンドウクラス名
				TEXT("SyslogTool"),				// タイトルバーに表示する文字列
				WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,	// ウィンドウの種類
				g_windowPos.left,		// ウィンドウを表示する位置（X座標）
				g_windowPos.top,		// ウィンドウを表示する位置（Y座標）
				WINDOW_WIDTH,			// ウィンドウの幅
				WINDOW_HEIGHT,			// ウィンドウの高さ
				NULL,					// 親ウィンドウのウィンドウハンドル
				NULL,					// メニューハンドル
				hInstance,				// インスタンスハンドル
				NULL					// その他の作成データ
			);

	if (!hWnd) {
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	hBaseWindow = hWnd;

	// 起動一番にSYSLOGダイアログを出力
	SendMessage(hWnd, WM_COMMAND, IDM_DISP_SYSLOG, 0);
		
	//画面表示後に、コマンドライン引数ファイルがある場合、各種処理実行
	if (strcmp(szCmdLineFileName, "")) {
		//ファイルパスを分割
		splitpath(szCmdLineFileName);
		::_tcscpy_s(szDrive, _T_MAX_DRIVE * sizeof(_TCHAR), szGlDrive);
		::_tcscpy_s(szDir, _T_MAX_DIR * sizeof(_TCHAR), szGlPath);
		::_tcscpy_s(szTemp, _T_MAX_FNAME * sizeof(_TCHAR), szGlFName);

		if (!strcmp(szGlExt, ".gz")) {
			//解凍処理
			if (!USBFileDecomp(szCmdLineFileName)) return TRUE;			//解凍処理
			//解凍後のファイルをDropされたファイルとして設定
			memset(szCmdLineFileName, 0, sizeof(szCmdLineFileName));	//コマンドライン引数ファイル名退避領域初期化
			strcpy_s(szCmdLineFileName, szDrive);
			strcat_s(szCmdLineFileName, szDir);
			strcat_s(szCmdLineFileName, engnFileNameEntry2);

			if(!PathFileExists(szCmdLineFileName)) {
				//解凍後のファイルをDropされたファイルとして設定
				memset(szCmdLineFileName, 0, sizeof(szCmdLineFileName));	//コマンドライン引数ファイル名退避領域初期化
				strcpy_s(szCmdLineFileName, szDrive);
				strcat_s(szCmdLineFileName, szDir);
				strcat_s(szCmdLineFileName, engnFileNameEntry1);
			}
		} else {
			strcpy_s(szCmdLineFileName, szCmdLineFileName);				//コマンドライン引数を退避
		}
		
		//dev_proc_edbg_all.logのEngSoftのバージョンに対応したINIファイルより設定値取得
		char SoftVer[SOFTVER_BUFFER_SZ];
		if (!getLogSoftVer(szCmdLineFileName, SoftVer)) return (INT_PTR)TRUE;	//dev_proc_edbg_all.logよりソフトバージョン取得
		//ソフトバージョンが取得できた場合、取得バージョンに応じた各種設定ファイルを再取得
		if (strcmp(SoftVer, "")) {
			getSoftVerSettingData(SoftVer);										//ソフトバージョン対応情報取得処理
		}

		//*.log⇒*.tcd変換処理
		char TargetFileName[_T_MAX_PATH];
		//*.tcdのファイル名はドロップされたファイルの拡張子より前の部分を使用
		char *FileName;
		char* p;
		FileName = strtok_s(szTemp, "." , &p);
		strcpy_s(TargetFileName, FileName);

		//*.tcdファイル名生成
		char targetFile[2048] = "";
		strcpy_s(targetFile, szDrive);
		strcat_s(targetFile, szDir);
		strcat_s(targetFile, TargetFileName);
		strcat_s(targetFile, ".tcd");
		if (!USBLogConvert(hSyslogDlgWnd, szCmdLineFileName, targetFile)) return TRUE;	//*.log⇒*.tcd変換処理

		//ログ表示保存処理
		//ログ表示保存用ファイル名生成
		memset(targetFile, 0, sizeof(targetFile));
		strcpy_s(targetFile, szDrive);
		strcat_s(targetFile, szDir);
		strcat_s(targetFile, TargetFileName);
		strcat_s(targetFile, ".log");
		if (!LogSave(hSyslogDlgWnd, targetFile)) return (INT_PTR)TRUE;			//ログ表示内容ファイル保存処理

		DestroyWindow(hSyslogDlgWnd);		//ダイアログWindow破棄
		DestroyWindow(hWnd);				//MainWindow破棄
	}

	return TRUE;
}

//==================================================
//	SaveSyslogToFile()
//--------------------------------------------------
//	<引数>
//	hDlg	: LogToolダイアログへのハンドラ
//--------------------------------------------------
//	<処理概要>
//	
//==================================================
INT_PTR SaveSyslogToFile(HWND hDlg)
{

	// 構造体に情報をセット
	ZeroMemory(&ofn, sizeof(ofn));			// 最初にゼロクリアしておく
	ofn.lStructSize = sizeof(ofn);			// 構造体のサイズ
	ofn.hwndOwner = hDlg;                   // コモンダイアログの親ウィンドウハンドル
	ofn.lpstrFilter = _T("ログファイル(*.log)\0*.log\0All files(*.*)\0*.*\0\0"); // ファイルの種類
	ofn.lpstrFile = filename_full;          // 選択されたファイル名(フルパス)を受け取る変数のアドレス
	ofn.lpstrFileTitle = filename;          // 選択されたファイル名を受け取る変数のアドレス
	ofn.nMaxFile = sizeof(filename_full);   // lpstrFileに指定した変数のサイズ
	ofn.nMaxFileTitle = sizeof(filename);   // lpstrFileTitleに指定した変数のサイズ
	ofn.Flags = OFN_OVERWRITEPROMPT;        // フラグ指定
	ofn.lpstrTitle = _T("保存ファイルを設定");
	ofn.lpstrDefExt = _T("log");            // デフォルトのファイルの種類

	// 名前を付けて保存コモンダイアログを作成
	if(!GetOpenFileName(&ofn)) return (INT_PTR)TRUE;
	if(PathFileExists(filename_full)) {
		if (MessageBox(NULL, TEXT("指定ファイルが存在しています。上書きしますか？\n"), TEXT("確認"), MB_YESNO | MB_ICONQUESTION) == IDNO) {
			return (INT_PTR)TRUE;
		}
		if (GetFileAttributes(filename_full) & FILE_ATTRIBUTE_READONLY) {
			if (MessageBox(NULL, TEXT("指定ファイルはReadOnlyファイルです。上書きしますか？\n"), TEXT("確認"), MB_YESNO | MB_ICONQUESTION) == IDNO) {
				return (INT_PTR)TRUE;
			}
			SetFileAttributes(filename_full, FILE_ATTRIBUTE_NORMAL);
		}
	}
	if (!LogSave(hDlg, filename_full)) return (INT_PTR)TRUE;		//ログ表示内容ファイル保存処理

	return (INT_PTR)TRUE;
}

INT_PTR OpenSyslogTextApp (HWND hDlg)
{
	char filePathBuf[1024];	// ファイルパス(ファイル名含む)用 文字列バッファ

	sprintf_s(filePathBuf, sizeof(filePathBuf), "%s%s", szGlPath, "temp_log.log");		// ファイルパス生成
	INT_PTR saveResult = LogSave(hDlg, filePathBuf);
	if (saveResult == TRUE) {
		// テキストエディタで開く
		HINSTANCE instance = ShellExecute(NULL, "open", filePathBuf, NULL, NULL, SW_SHOWNORMAL);
	} else {
		// ファイルが開けなかった場合、何もしない
	}
	return (INT_PTR)TRUE;
}

//==================================================
//	ascii2bin()
//--------------------------------------------------
//	<引数>
//	src		: アスキーコードバッファへのポインタ
//	dst		: バイナリ変換後の格納バッファへのポインタ
//--------------------------------------------------
//	<処理概要>
//	アスキーコードをバイナリ変換する
//==================================================
void ascii2bin(unsigned char* src, unsigned char* dst)
{
	int src_loc = 0;
	int dst_loc = 0;
	char bin = 0;

	while((src[src_loc] != 0x00) && (src[src_loc+1] != 0x00)) {
		bin = 0;
		if((src[src_loc] >= '0') && (src[src_loc] <= '9')) {
			bin = ((src[src_loc] - '0') << 4);
		} else if((src[src_loc] >= 'A') && (src[src_loc] <= 'F')) {
			bin = ((src[src_loc] - 'A' + 0x0a) << 4);
		} else if((src[src_loc] >= 'a') && (src[src_loc] <= 'f')) {
			bin = ((src[src_loc] - 'a' + 0x0a) << 4);
		}
		src_loc++;
		if((src[src_loc] >= '0') && (src[src_loc] <= '9')) {
			bin |= (src[src_loc] - '0');
		} else if((src[src_loc] >= 'A') && (src[src_loc] <= 'F')) {
			bin |= (src[src_loc] - 'A' + 0x0a);
		} else if((src[src_loc] >= 'a') && (src[src_loc] <= 'f')) {
			bin |= (src[src_loc] - 'a' + 0x0a);
		}
		src_loc++;
		dst[dst_loc] = bin;
		dst_loc++;
	}
}

//==================================================
//	TimeCount Difference functions
//--------------------------------------------------
//	<引数>
//	preTimeCount	:
//	timeCount		:
//--------------------------------------------------
//	<処理概要>
//	
//==================================================
unsigned long getTcmDiffTime (unsigned long *preTimeCount, unsigned long timeCount)
{
	unsigned long diffTimeCount = 0;           // 時間差

    // 差分時間算出
	if (timeCount < *preTimeCount) {
		diffTimeCount = (0xffffffff - (*preTimeCount - timeCount));
	} else {
		diffTimeCount = (timeCount - *preTimeCount);
	}

    // 前回時間更新
	*preTimeCount = timeCount;

	return diffTimeCount;
}

//==================================================
//	LoadUSBLogFile()
//--------------------------------------------------
//	<引数>
//	hDlg	: LogToolダイアログへのハンドラ
//--------------------------------------------------
//	<処理概要>
//	
//==================================================
INT_PTR LoadUSBLogFile(HWND hDlg)
{

	static TCHAR tcm_filename_full[_T_MAX_PATH];   // ファイル名(フルパス)を受け取る領域

	//ここで変数を初期化し構造体設定は空文字の設定をしないと、
	//ファイル選択ダイアログが表示されない
	memset(filename_full, 0, sizeof(filename_full));	//ファイル名フルパス退避領域初期化
	memset(filename, 0, sizeof(filename));				//ファイル名退避領域初期化

	HWND lbox = GetDlgItem(hDlg, IDC_SYSLOG);

	//---------------------------------------------------------
	// 構造体に情報をセット(読込用)
	ZeroMemory(&ofn, sizeof(ofn));			// 最初にゼロクリアしておく
	ofn.lStructSize = sizeof(ofn);			// 構造体のサイズ
	ofn.hwndOwner = hDlg;                   // コモンダイアログの親ウィンドウハンドル
	ofn.lpstrFilter = _T("USBログ圧縮ファイル(*.gz)\0*.gz\0エンジン関係のUSBログファイル\0*edbg*.log\0All files(*.*)\0*.*\0\0"); // ファイルの種類
	ofn.lpstrFile = filename_full;          // 選択されたファイル名(フルパス)を受け取る変数のアドレス
	ofn.lpstrFileTitle = filename;          // 選択されたファイル名を受け取る変数のアドレス
	ofn.nMaxFile = sizeof(filename_full);   // lpstrFileに指定した変数のサイズ
	ofn.nMaxFileTitle = sizeof(filename);   // lpstrFileTitleに指定した変数のサイズ
	ofn.Flags = OFN_OVERWRITEPROMPT;        // フラグ指定
	ofn.lpstrTitle = _T("USBログファイルを指定");
	ofn.lpstrDefExt = _T("gz");            // デフォルトのファイルの種類

	// 名前を付けて保存コモンダイアログを作成(読込用)
	if(!GetOpenFileName(&ofn)) return (INT_PTR)TRUE;

	//選択ファイルパスよりフォルダを抽出
	char targetDir[2048] = "";
//	char targetFile[] = "\\log\\files\\debug_file\\dev_proc_edbg_all.log";
	splitpath(filename_full);

	strcpy_s(targetDir, szGlDrive);
	strcat_s(targetDir, szGlPath);

	//---------------------------------------------------------
	// 構造体に情報をセット(書込用)
	ZeroMemory(&ofn, sizeof(ofn));			// 最初にゼロクリアしておく
	ofn.lStructSize = sizeof(ofn);			// 構造体のサイズ
	ofn.hwndOwner = hDlg;                   // コモンダイアログの親ウィンドウハンドル
	ofn.lpstrFilter = _T("TCDデータファイル(*.tcd)\0*.tcd\0All files(*.*)\0*.*\0\0"); // ファイルの種類
	ofn.lpstrFile = tcm_filename_full;      // 選択されたファイル名(フルパス)を受け取る変数のアドレス
	ofn.lpstrFileTitle = filename;          // 選択されたファイル名を受け取る変数のアドレス
	ofn.nMaxFile = sizeof(tcm_filename_full); // lpstrFileに指定した変数のサイズ
	ofn.nMaxFileTitle = sizeof(filename);   // lpstrFileTitleに指定した変数のサイズ
	ofn.Flags = OFN_OVERWRITEPROMPT;        // フラグ指定
	ofn.lpstrTitle = _T("保存ファイルを設定");
	ofn.lpstrDefExt = _T("tcd");            // デフォルトのファイルの種類
	ofn.lpstrInitialDir = targetDir;		//選択された変換元ファイルと同一フォルダを初期表示設定

	// 名前を付けて保存コモンダイアログを作成(書込用)
	if(!GetOpenFileName(&ofn)) return (INT_PTR)TRUE;
	
	//---------------------------------------------------------
	//圧縮ファイル(*.gz)を選択した場合、以下の処理を実施
	//　・ファイル解凍
	//	・解凍して生成された"dev_proc_edbg_all.log"を選択ファイルとして扱う
	if (!strcmp(szGlExt, ".gz")) {
		if (!USBFileDecomp(filename_full)) return (INT_PTR)TRUE;	//解凍処理(エラー発生時終了)
		//解凍後のコピーされたLogファイルを選択ファイルとして設定
		memset(filename_full, 0, sizeof(filename_full));
		strcpy_s(filename_full, targetDir);
		strcat_s(filename_full, engnFileNameEntry2);
		if(!PathFileExists(filename_full)) {
			//解凍後のコピーされたLogファイルを選択ファイルとして設定
			memset(filename_full, 0, sizeof(filename_full));
			strcpy_s(filename_full, targetDir);
			strcat_s(filename_full, engnFileNameEntry1);
		}
	}
	//*.log⇒*.tcd変換処理
	if (!USBLogConvert(hDlg, filename_full, tcm_filename_full)) return (INT_PTR)TRUE;

	//---------------------------------------------------------
	//処理終了
	return (INT_PTR)TRUE;

}

//==================================================
//	SaveLogBinFile()
//--------------------------------------------------
//	<引数>
//	hDlg	: LogToolダイアログへのハンドラ
//--------------------------------------------------
//	<処理概要>
//	
//==================================================
INT_PTR SaveLogBinFile(HWND hDlg)
{
	// 構造体に情報をセット
	ZeroMemory(&ofn, sizeof(ofn));			// 最初にゼロクリアしておく
	ofn.lStructSize = sizeof(ofn);			// 構造体のサイズ
	ofn.hwndOwner = hDlg;                   // コモンダイアログの親ウィンドウハンドル
	ofn.lpstrFilter = _T("SYSLOGバイナリファイル(*.bin)\0*.bin\0All files(*.*)\0*.*\0\0"); // ファイルの種類
	ofn.lpstrFile = filename_full;          // 選択されたファイル名(フルパス)を受け取る変数のアドレス
	ofn.lpstrFileTitle = filename;          // 選択されたファイル名を受け取る変数のアドレス
	ofn.nMaxFile = sizeof(filename_full);   // lpstrFileに指定した変数のサイズ
	ofn.nMaxFileTitle = sizeof(filename);   // lpstrFileTitleに指定した変数のサイズ
	ofn.Flags = OFN_OVERWRITEPROMPT;        // フラグ指定
	ofn.lpstrTitle = _T("保存ファイルを設定");
	ofn.lpstrDefExt = _T("bin");            // デフォルトのファイルの種類

	// 名前を付けて保存コモンダイアログを作成
	if(!GetOpenFileName(&ofn)) return (INT_PTR)TRUE;

	if(PathFileExists(filename_full)) {
		DeleteFile(filename_full);
	}
	logFileClose();

	// SYSLOGファイルをコピー
	CopyFile(syslog_file, filename_full, FALSE);

	logFileReOpen();
	return (INT_PTR)TRUE;
}

//==================================================
//	SaveTcmlogToFile()
//--------------------------------------------------
//	<引数>
//	hDlg	: LogToolダイアログへのハンドラ
//--------------------------------------------------
//	<処理概要>
//	
//==================================================
INT_PTR SaveTcmlogToFile(HWND hDlg)
{
	// 構造体に情報をセット
	ZeroMemory(&ofn, sizeof(ofn));			// 最初にゼロクリアしておく
	ofn.lStructSize = sizeof(ofn);			// 構造体のサイズ
	ofn.hwndOwner = hDlg;                   // コモンダイアログの親ウィンドウハンドル
	ofn.lpstrFilter = _T("TCDデータファイル(*.tcd)\0*.tcd\0All files(*.*)\0*.*\0\0"); // ファイルの種類
	ofn.lpstrFile = filename_full;          // 選択されたファイル名(フルパス)を受け取る変数のアドレス
	ofn.lpstrFileTitle = filename;          // 選択されたファイル名を受け取る変数のアドレス
	ofn.nMaxFile = sizeof(filename_full);   // lpstrFileに指定した変数のサイズ
	ofn.nMaxFileTitle = sizeof(filename);   // lpstrFileTitleに指定した変数のサイズ
	ofn.Flags = OFN_OVERWRITEPROMPT;        // フラグ指定
	ofn.lpstrTitle = _T("保存ファイルを設定");
	ofn.lpstrDefExt = _T("tcd");            // デフォルトのファイルの種類

	// 名前を付けて保存コモンダイアログを作成
	if(!GetOpenFileName(&ofn)) return (INT_PTR)TRUE;

	BOOL tcm_area = FALSE;

	if(PathFileExists(filename_full)) {
		if (MessageBox(NULL, TEXT("指定ファイルが存在しています。上書きしますか？\n"), TEXT("確認"), MB_YESNO | MB_ICONQUESTION) == IDNO) {
			return (INT_PTR)TRUE;
		}
		if (GetFileAttributes(filename_full) & FILE_ATTRIBUTE_READONLY) {
			if (MessageBox(NULL, TEXT("指定ファイルはReadOnlyファイルです。上書きしますか？\n"), TEXT("確認"), MB_YESNO | MB_ICONQUESTION) == IDNO) {
				return (INT_PTR)TRUE;
			}
			SetFileAttributes(filename_full, FILE_ATTRIBUTE_NORMAL);
		}
		DeleteFile(filename_full);
	}
	tcmSave(hDlg, filename_full);

	return (INT_PTR)TRUE;
}

INT_PTR tcmSave(HWND hDlg, char *fileName)
{
	unsigned char readBuf[sizeof(long) * 15 + 2];
	char msgBuf[64];
	FILE* wstream;
	unsigned long tcm_preTime = 0L;
	errno_t result = fopen_s(&wstream, fileName, "a+");

	if (result != 0) {
		sprintf_s(msgBuf, "ファイルオープンに失敗しました(err=%d)", result);
		MessageBox(NULL, TEXT(msgBuf), TEXT("エラー"), MB_OK | MB_ICONSTOP);
		return (INT_PTR)FALSE;
	}

	//ログファイルを一旦Close
	logFileClose();
	FILE* rstream = _fsopen(syslog_file, "rb", _SH_DENYNO);
	if(!rstream) {
		fclose(wstream);
		return (INT_PTR)FALSE;
	}

	BOOL nowRcved = FALSE;
	unsigned long totalTm = 0;

	std::list<TCMLOG>listTcmLog;
	while(1) {
		// ログブロックの先頭まで読み飛ばし
		if(fread(readBuf, 1, 1, rstream) < 1) {
			break;
		}

		unsigned char read4bit_chk = ((readBuf[0] & 0xf0) >> 4);

		// SYSLOGブロックなら1ブロック読み飛ばす
		if(read4bit_chk == 0x00 || read4bit_chk == 0x01) {
			// CRLFチェック
			if(readBuf[0] == '\r') {
				if(fread(readBuf, 1, 1, rstream) < 1) {
					nowRcved = TRUE;
					break;
				}
				if(readBuf[0] == '\n') {
					continue;
				}
				fseek(rstream, -2L, SEEK_CUR);
			}
			else {
				fseek(rstream, -1L, SEEK_CUR);
			}

			if(fread(&readBuf[0], 1, 1, rstream) < 1) {
				nowRcved = TRUE;
				break;
			}

			char err_lvl = (readBuf[0] & 0x78) >> 3;	// エラーレベル
			if((err_lvl < 0) || (err_lvl > 3)) {
				continue;
			}

			if(fread(&readBuf[1], 1, 4, rstream) < 4) {
				nowRcved = TRUE;
				break;
			}

			unsigned long tim = (((readBuf[0] & 0x07) << 24)
						+ (readBuf[1] << 16)
						+ (readBuf[2] <<  8)
						+ readBuf[3]);

			char group_id = (readBuf[4] & 0xfc) >> 2;	// グループID
			if((group_id < 0) || (group_id > log_group_num)) {
				continue;
			}

			if(fread(&readBuf[5], 1, 2, rstream) < 2) {
				nowRcved = TRUE;
				break;
			}

			short log_no = ((readBuf[4] & 0x03) << 8) + readBuf[5];	// ログNo
			char arg_type = (readBuf[6] & 0xf0) >> 4;	// 引数タイプ
			if((arg_type < 0) || (arg_type > 7)) {
				continue;
			}

			char arg_num = (readBuf[6] & 0x0f);			// 引数の数
			if((arg_num < 0) || (arg_num > 15)) {
				continue;
			}

			char* msg = get_msgtbl(group_id, log_no, arg_num);
			if(msg == NULL) {
				continue;
			}

			if(arg_type > 0 && arg_num > 0) {
				switch(arg_type) {
					case 1:		// signed char
					case 2:		// unsigned char
						if(fread(readBuf, sizeof(char), arg_num, rstream) < (size_t)arg_num) {
							nowRcved = TRUE;
						}
						break;
					case 4:		// signed short
					case 5:		// unsigned short
						if(fread(readBuf, sizeof(short), arg_num, rstream) < (size_t)arg_num) {
							nowRcved = TRUE;
						}
						break;
					case 6:		// signed long
					case 7:		// unsigned long
						if(fread(readBuf, sizeof(long), arg_num, rstream) < (size_t)arg_num) {
							nowRcved = TRUE;
						}
						break;
				}

				if(nowRcved) {
					break;
				}
			}
			continue;
		}
		// TCMブロックでなければ読み飛ばす
		else if(read4bit_chk != 0x08 && read4bit_chk != 0x09
			&& read4bit_chk != 0x0a && read4bit_chk != 0x0c) {
				continue;
		}

		if(nowRcved) {
			break;
		}

		fseek(rstream, -1L, SEEK_CUR);
		if(fread(readBuf, 1, 8, rstream) < 8) {
			break;
		}

		// deviceType取得
		unsigned char devType = (readBuf[0] >> 3) & 0x0f;
		// タイムスタンプ取得
		unsigned long timeStamp = (((readBuf[0] & 0x07) << 24)
									+ (readBuf[1] << 16)
									+ (readBuf[2] <<  8)
									+ readBuf[3]);
		// 差分時間取得
		unsigned long diffTm = getTcmDiffTime (&tcm_preTime, timeStamp);
		// データ取得
		unsigned short id = (unsigned short)(readBuf[4] << 3) + ((readBuf[5] >> 5) & 7);
		unsigned char io = (readBuf[5] >> 4) & 1;
		unsigned char onoff = (readBuf[5] >> 1) & 7;
		unsigned char valInvalid = readBuf[5] & 1;
		unsigned short val = (unsigned short)(readBuf[6] << 8) + readBuf[7];
		unsigned long data = (unsigned long)(val << 16)
								+ (unsigned long)(valInvalid << 15)
								+ (unsigned long)(onoff << 12)
								+ (unsigned long)(io << 11)
								+ (unsigned long)id;
//		fprintf(wstream, "%08x,%d\n", data, diffTm);

		totalTm = totalTm + diffTm;
		unsigned char param = 0;
		// シナリオはパラメータ部を別idにする
		if (senarioSeparete(id)) {
			if (val >= TCM_PAGEID_DIGIT) {
				if (unit == EUNIT_TYPE_DPS_ENGINE) {
					param = ((val >> 8) & (0xff));
					val = (val & 0xff);
				} else {
					param = val / 1000;
					val -= (param * 1000);
				}
			}
		}

		// リストに登録
		char deviceId[9];
		sprintf_s(deviceId, 9, "%03x%04x%d", param, id, io);

		// onoffは3ビット分しか格納できないので
		// そのすべてが1なら-1となる。
		char fixedOnoff = onoff;
		if(onoff == 7) {
			fixedOnoff = -1;
		}

		TCMLOG tcmLog = {deviceId, io, fixedOnoff, val, totalTm, param};
		listTcmLog.push_back(tcmLog);
	}

	writeTcdFile(wstream, listTcmLog, szIniFilePublic);

	fclose(rstream);
	fclose(wstream);

	//ログファイルを再度Open
	logFileReOpen();
	return (INT_PTR)TRUE;
}

INT_PTR GetSettingFileName(HWND hDlg, int kind)
{
	// 構造体に情報をセット
	ZeroMemory(&ofn, sizeof(ofn));			// 最初にゼロクリアしておく
	ofn.lStructSize = sizeof(ofn);			// 構造体のサイズ
	ofn.hwndOwner = hDlg;                   // コモンダイアログの親ウィンドウハンドル
	// ファイルの種類
	if(kind == 0) {
		ofn.lpstrFilter = _T("メッセージファイル(*.mes)\0*.mes\0All files(*.*)\0*.*\0\0");
		ofn.lpstrDefExt = _T("mes");           // デフォルトのファイルの種類
	}
	else {
		ofn.lpstrFilter = _T("SYSLOGファイル(*.bin)\0*.bin\0All files(*.*)\0*.*\0\0");
		ofn.lpstrDefExt = _T("bin");           // デフォルトのファイルの種類
	}
	ofn.lpstrFile = filename_full;         // 選択されたファイル名(フルパス)を受け取る変数のアドレス
	ofn.lpstrFileTitle = filename;         // 選択されたファイル名を受け取る変数のアドレス
	ofn.nMaxFile = sizeof(filename_full);  // lpstrFileに指定した変数のサイズ
	ofn.nMaxFileTitle = sizeof(filename);  // lpstrFileTitleに指定した変数のサイズ
	ofn.Flags = OFN_FILEMUSTEXIST;         // フラグ指定
	ofn.lpstrTitle = _T("ファイルを選択"); // コモンダイアログのキャプション

	// 名前を付けて保存コモンダイアログを作成
	if(!GetOpenFileName(&ofn)) return (INT_PTR)FALSE;

	if(kind == 0) {
		SetWindowText(GetDlgItem(hDlg, IDC_EDIT_LOGMES_FILE), filename_full);
	}
	else {
		SetWindowText(GetDlgItem(hDlg, IDC_EDIT_SYSLOG_FIL), filename_full);
	}

	return (INT_PTR)TRUE;
}

//==================================================
//	フォルダ削除関数(Shell使用Version)
//--------------------------------------------------
//	<引数>
//	lpPathName	:削除対象フォルダパス
//--------------------------------------------------
//	<処理概要>
//	指定されたフォルダ以下全削除実行
//==================================================
BOOL DeleteDirectory( LPCTSTR lpPathName )
{
	// 入力値チェック
	if( NULL == lpPathName )	return FALSE;

	// ディレクトリ名の保存（終端に'\'がないなら付ける）
	TCHAR szDirectoryPathName[_T_MAX_PATH];
	_tcsncpy_s( szDirectoryPathName, _T_MAX_PATH, lpPathName, _TRUNCATE );
	if( '\\' != szDirectoryPathName[_tcslen(szDirectoryPathName) - 1] )
	{	// 一番最後に'\'がないなら付加する。
		_tcsncat_s( szDirectoryPathName, _T_MAX_PATH, _T("\\"), _TRUNCATE );
	}

	// ディレクトリ内のファイル走査用のファイル名作成
	TCHAR szFindFilePathName[_T_MAX_PATH];
	_tcsncpy_s( szFindFilePathName, _T_MAX_PATH, szDirectoryPathName, _TRUNCATE );
	_tcsncat_s( szFindFilePathName, _T_MAX_PATH, _T("*"), _TRUNCATE );

	// ディレクトリ内のファイル走査開始
	WIN32_FIND_DATA		fd;
	HANDLE hFind = FindFirstFile( szFindFilePathName, &fd );
	if( INVALID_HANDLE_VALUE == hFind )
	{	// 走査対象フォルダが存在しない。
		return FALSE;
	}

	do
	{
		//if( '.' != fd.cFileName[0] )
		if( 0 != _tcscmp( fd.cFileName, _T(".") )		// カレントフォルダ「.」と
		 && 0 != _tcscmp( fd.cFileName, _T("..") ) )	// 親フォルダ「..」は、処理をスキップ
		{
			TCHAR szFoundFilePathName[_T_MAX_PATH];
			_tcsncpy_s( szFoundFilePathName, _T_MAX_PATH, szDirectoryPathName, _TRUNCATE );
			_tcsncat_s( szFoundFilePathName, _T_MAX_PATH, fd.cFileName, _TRUNCATE );

			if( FILE_ATTRIBUTE_DIRECTORY & fd.dwFileAttributes )
			{	// ディレクトリなら再起呼び出しで削除
				if( !DeleteDirectory( szFoundFilePathName ) )
				{
					FindClose( hFind );
					return FALSE;
				}
			}
			else
			{	// ファイルならWin32API関数を用いて削除
				if( !DeleteFile( szFoundFilePathName ) )
				{
					FindClose( hFind );
					return FALSE;
				}
			}
		}
	} while( FindNextFile( hFind, &fd ) );

	FindClose( hFind );

	return RemoveDirectory( lpPathName );
}

//==================================================
//	USB Log圧縮ファイル解凍処理
//--------------------------------------------------
//	<引数>
//	filename_full		:変換対象ファイル(*.gz)
//--------------------------------------------------
//	<処理概要>
//	指定ファイルの解凍処理を行い、指定Logファイルを圧縮ファイルと
//	同一フォルダへコピー後、解凍データを削除する
//==================================================
INT_PTR USBFileDecomp(char *filename_full)
{
	char LogFile[_T_MAX_PATH] = "";					//ログファイル名(フルパス)退避用
	char FileBuff[_T_MAX_PATH] = "";				//ファイル名(フルパス)退避用

	splitpath(filename_full);

	//圧縮ファイルのファイル名退避(拡張子含む)
	memset(filename, 0, sizeof(filename));		//ファイル名退避領域(グローバル変数)初期化
	strcpy_s(filename, szGlFName);
	strcat_s(filename, szGlExt);

	//文字列を渡すために必要な型を宣言
	HINSTANCE lib;								// DLLのハンドル
	int i = 0,ret = 0,ret2 = 0;					// コマンドラインの結合に利用
	char cmd[2048] = "",outstr[1000] = "";		// 入出力バッファの宣言
	WORD getv = 0;								// バージョン取得を格納

	// 動的リンク(DLLがなくても動作できるアプリケーション)
	// プログラムで利用するAPI関数をtypedefで宣言します。
	// API関数の動作についてはアーカイバDLLのテキストを参照してください。
	// ここではバージョン取得と書庫操作のみを利用します。
	// 仮引数は型のみでかまいません。
	typedef WORD(WINAPI *TARGETVERSION)(void);
	typedef int(WINAPI *TAR)(const HWND, LPCSTR, LPSTR , const DWORD);
	
	lib = LoadLibrary("TAR32.dll");			// LoadLibraryでDLLを開く

	// DLLが正しくロードされたか確認
	if (lib == NULL) {
		//ファイルが見つからないなどでDLLを開くのに失敗した場合、エラー表示し処理終了
		MessageBox(NULL, TEXT("ERROR:TAR32.DLL not Found on your system.\n"), TEXT("エラー"), MB_OK | MB_ICONSTOP);
		return (INT_PTR)FALSE;
	} else {
		// 見つかった場合、API関数へのアドレスを、GetProcAddressでキャスト
		// 分かりやすいように関数の名前を変数で宣言
		TARGETVERSION TarGetVersion = (TARGETVERSION)GetProcAddress(lib,"TarGetVersion");
		TAR Tar = (TAR)GetProcAddress(lib,"Tar");
		
		//tarコマンドの編集
		//ファイルパスに空白が含まれると、コマンド正常動作しない為、
		//ファイルパスはダブルクォテーションで囲む

//1)全解凍⇒ファイル指定解凍を行うよう修正
//			ファイル指定の解凍例)tar -xvzf log.tar.gz log/files/debug_file/dev_proc_edbg_all.log
/*
		strcat_s(cmd ,"-xvf ");			//tar32解凍コマンド
		strcat_s(cmd ,"\"");			//圧縮ファイル指定を["]で囲む
		strcat_s(cmd ,filename_full);	//圧縮ファイル指定
		strcat_s(cmd ,"\"");			//圧縮ファイル指定を["]で囲む
		strcat_s(cmd ," -o ");			//展開先の指定オプション
		strcat_s(cmd ,"\"");			//圧縮ファイル指定を["]で囲む
		strcat_s(cmd ,targetDir);		//展開先フォルダ(解凍ファイルは圧縮ファイルと同一フォルダに保存)
		strcat_s(cmd ,"\"");			//圧縮ファイル指定を["]で囲む
*/
		memset(cmd, 0, sizeof(cmd));
		strcat_s(cmd ,"-xvf ");			//tar32解凍コマンド
		strcat_s(cmd ,"\"");			//圧縮ファイル指定を["]で囲む
		strcat_s(cmd ,filename_full);	//圧縮ファイル指定
		strcat_s(cmd ,"\"");			//圧縮ファイル指定を["]で囲む
		strcat_s(cmd ," -o ");			//展開先の指定オプション
		strcat_s(cmd ,"\"");			//圧縮ファイル指定を["]で囲む
		strcat_s(cmd ,szWindowsTempPath);	//展開先フォルダ(解凍ファイルはWindowsテンポラリフォルダに保存)
		strcat_s(cmd ,"\"");			//圧縮ファイル指定を["]で囲む
		strcat_s(cmd ," log\\files\\debug_file\\");
		strcat_s(cmd ,engnFileNameEntry1);		//ファイル指定

		ret = Tar(NULL, cmd, outstr ,1000);		// tarコマンド実行

		memset(cmd, 0, sizeof(cmd));
		strcat_s(cmd ,"-xvf ");			//tar32解凍コマンド
		strcat_s(cmd ,"\"");			//圧縮ファイル指定を["]で囲む
		strcat_s(cmd ,filename_full);	//圧縮ファイル指定
		strcat_s(cmd ,"\"");			//圧縮ファイル指定を["]で囲む
		strcat_s(cmd ," -o ");			//展開先の指定オプション
		strcat_s(cmd ,"\"");			//圧縮ファイル指定を["]で囲む
		strcat_s(cmd ,szWindowsTempPath);	//展開先フォルダ(解凍ファイルはWindowsテンポラリフォルダに保存)
		strcat_s(cmd ,"\"");			//圧縮ファイル指定を["]で囲む
		strcat_s(cmd ," log\\files\\debug_file\\");
		strcat_s(cmd ,engnFileNameEntry2);		//ファイル指定

		ret2 = Tar(NULL, cmd, outstr ,1000);		// tarコマンド実行

		FreeLibrary(lib);						// ロードしたDLLを開放
		if (ret && ret2) {
			MessageBox(NULL, outstr, TEXT("ERROR：TAR32"), MB_OK | MB_ICONSTOP);
			return (INT_PTR)FALSE;
		}
	}

	//指定圧縮ファイルと同一フォルダに解凍されているので
	//logファイルのフルパス編集(解凍フォルダ・ファイル名固定)
	if (strcmp(szCmdLine2FileName, "")) {
		strcpy_s(LogFile, szWindowsTempPath);
		strcat_s(LogFile, "log\\files\\debug_file\\");
		strcat_s(LogFile, engnFileNameEntry1);
		strcpy_s(FileBuff, szGlDrive);
		strcat_s(FileBuff, szGlPath);
		strcat_s(FileBuff, engnFileNameEntry1);
		CopyFile(LogFile, FileBuff, false);
	}
	strcpy_s(LogFile, szWindowsTempPath);
	strcat_s(LogFile, "log\\files\\debug_file\\");
	strcat_s(LogFile, engnFileNameEntry2);

	//解凍フォルダに"dev_proc_edbg_all_tmp.log"存在するか確認
	if (!PathFileExists(LogFile)) {
		//指定圧縮ファイルと同一フォルダに解凍されているので
		//logファイルのフルパス編集(解凍フォルダ・ファイル名固定)
		strcpy_s(LogFile, szWindowsTempPath);
		strcat_s(LogFile, "log\\files\\debug_file\\");
		strcat_s(LogFile, engnFileNameEntry1);

		//解凍フォルダに"dev_proc_engine.log"存在するか確認
		if (!PathFileExists(LogFile)) {
			MessageBox(NULL, TEXT("ERROR:dev_proc_edbg_all.log not Found.\n"), TEXT("エラー"), MB_OK | MB_ICONSTOP);
			return (INT_PTR)FALSE;
		} else {
			strcpy_s(FileBuff, szGlDrive);
			strcat_s(FileBuff, szGlPath);
			strcat_s(FileBuff, engnFileNameEntry1);
		}
	} else {
		strcpy_s(FileBuff, szGlDrive);
		strcat_s(FileBuff, szGlPath);
		strcat_s(FileBuff, engnFileNameEntry2);
	}

	//エンジンログファイルを圧縮ファイルと同じフォルダへコピー(既にファイル存在する場合上書き)
	CopyFile(LogFile, FileBuff, false);

	//解凍ファイルを削除
	memset(FileBuff, 0, sizeof(FileBuff));
	strcpy_s(FileBuff, szWindowsTempPath);
	strcat_s(FileBuff, "log");
	DeleteDirectory(FileBuff);

	return (INT_PTR)TRUE;

}

//==================================================
//	USB Log開始終了文字列設定の読み込み
//--------------------------------------------------
//	<引数>		 なし
//
//==================================================
void readUsbLogPartitionStr()
{
	int cnt;
	
	GetPrivateProfileString(_T("USBLOG_PARTITION"), _T("USBLOG_START"), _T("***"), usbLogStartStrBuf, sizeof(usbLogStartStrBuf), szIniFilePublic);
	for(cnt = 0; cnt < LOG_PARTITION_NUM; cnt++) {
		sprintf_s(buf, "USBLOG_END_%d", cnt);
		GetPrivateProfileString(_T("USBLOG_PARTITION"), buf, _T("***"), usbLogEndStrBuf[cnt], sizeof(usbLogEndStrBuf[cnt]), szIniFilePublic);
		if(!memcmp(usbLogEndStrBuf[cnt], "***", strlen(usbLogEndStrBuf[cnt]))) {
			break;
		}
	}
	usblogEndStrNum = cnt;

	for (cnt = 0; cnt < LOG_PARTITION_NUM; cnt++) {
		sprintf_s(buf, "SEPARATION_%d", cnt);
		GetPrivateProfileString(_T("USBLOG_PARTITION"), buf, _T("***"), usbLogSepStrBuf[cnt], sizeof(usbLogSepStrBuf[cnt]), szIniFilePublic);
		if (!memcmp(usbLogSepStrBuf[cnt], "***", strlen(usbLogSepStrBuf[cnt]))) {
			break;
		}
	}
	usblogSepStrNum = cnt;
	
	for (cnt = 0; cnt < LOG_PARTITION_NUM; cnt++) {
		sprintf_s(buf, "TCMLOG_END_%d", cnt);
		GetPrivateProfileString(_T("USBLOG_PARTITION"), buf, _T("***"), usbLogTcmEndStrBuf[cnt], sizeof(usbLogTcmEndStrBuf[cnt]), szIniFilePublic);
		if (!memcmp(usbLogTcmEndStrBuf[cnt], "***", strlen(usbLogTcmEndStrBuf[cnt]))) {
			break;
		}
	}
	usblogTcmEndStrNum = cnt;
}

//============================================================
//	USB Log終了判断
//------------------------------------------------------------
//	<引数>
//	checkStr:ファイルから読み込んだデータ
// 	<戻り値>
//	EUSBLOG_NONE:継続、EUSBLOG_TCM_END:tcmログ終了
//　EUSBLOG_LINE_END:一行終了、EUSBLOG_ALL_END:USBログの終了
//============================================================
eUSBLOG_END_TYPE logEndCheck(char* checkStr)
{
	eUSBLOG_END_TYPE ret = EUSBLOG_NONE;
	unsigned char i;
	
	if ((checkStr[0] == 0x0a) && (checkStr[1] == 0x00)) {
		ret = EUSBLOG_LINE_END;
	} else if (!memcmp("[Start] ", checkStr, strlen("[Start] "))) {
		// コントローラ付加不要文字列チェック
		ret = EUSBLOG_LINE_END;
	} else if (!memcmp("[End] ", checkStr, strlen("[End] "))) {
		// コントローラ付加不要文字列チェック
		ret = EUSBLOG_LINE_END;
	} else 	if (!memcmp(" edbg: finish getting log completely", checkStr, strlen(" edbg: finish getting log completely"))) {
		ret = EUSBLOG_ALL_END;
	} else {
	}
	
	if (ret == EUSBLOG_NONE) {
		for (i = 0; i < usblogEndStrNum; i++) {
			if (!memcmp(usbLogEndStrBuf[i], checkStr, strlen(usbLogEndStrBuf[i]))) {
				ret = EUSBLOG_ALL_END;
				break;
			}
		}
	}
	if (ret == EUSBLOG_NONE) {
		for (i = 0; i < usblogSepStrNum; i++) {
			if (!memcmp(usbLogSepStrBuf[i], checkStr, strlen(usbLogSepStrBuf[i]))) {
				ret = EUSBLOG_LINE_END;
				break;
			}
		}
	}
	if (ret == EUSBLOG_NONE) {
		for (i = 0; i < usblogTcmEndStrNum; i++) {
			if (!memcmp(usbLogTcmEndStrBuf[i], checkStr, strlen(usbLogTcmEndStrBuf[i]))) {
				ret = EUSBLOG_TCM_END;
				break;
			}
		}
	}
	return ret;
}

//==================================================
//	USB Logファイル置換処理
//--------------------------------------------------
//	<引数>
//	hDlg				:ダイアログハンドラ
//	filename_full		:変換対象ファイル(*.log)
//	tcm_filename_full	:変換後ファイル(*.tcd)
//--------------------------------------------------
//	<処理概要>
//	指定されたログファイル(*.log)をタイミングチャート(*.tcd)の
//	ファイルへ変換する
//==================================================
INT_PTR USBLogConvert(HWND hDlg, char *filename_full, char *tcm_filename_full)
{

	char logrec[2048];
	unsigned char binbuffer[1024];						// logrecバッファの半分サイズ
	bool logEnd = false;
	HWND lbox = GetDlgItem(hDlg, IDC_SYSLOG);

	//---------------------------------------------------------
	//ファイルOpen処理
	FILE* stream;
	fopen_s(&stream, filename_full, "r");				//Log変換対象ファイルをReadモードでOpen

	// TCM
	unsigned long tcm_preTime = 0L;
	std::list<TCMLOG>listTcmLog;
	unsigned long totalTm = 0;

	FILE* wstream;
	//書込用に指定されたファイル名の存在チェック
	//存在する場合、指定ファイル削除
	if(PathFileExists(tcm_filename_full)) {
		DeleteFile(tcm_filename_full);
	}

	if(fopen_s(&wstream, tcm_filename_full, "a+") != 0) {	//書込用ファイルをOpen
		fclose(stream);
		return (INT_PTR)FALSE;
	}

	//Logファイル読込・変換処理
	readUsbLogPartitionStr();

	// エンジンログ開始位置までシーク
	while(fgets(logrec, sizeof(logrec), stream)) {
		// エンジンログ開始文字列チェック
		char checkBuf[USBLOG_PARTITION_BUF] = {0};
		memcpy(checkBuf, usbLogStartStrBuf, strlen(usbLogStartStrBuf));
		if (strstr(logrec, checkBuf)) {
			break;
		}
	}

	// エンジンログ読込み
	size_t lineLen = 0;
	size_t cntLineLen = 0;
	size_t log_pkt_len = 0;
	bool syslogFlg = true;
	bool isTcmEnd = false;
	eUSBLOG_END_TYPE lineCheck;

	while(fgets(logrec, sizeof(logrec), stream)) {
		// エンジンログ終了文字列チェック
		lineCheck = logEndCheck(logrec);
		if (lineCheck == EUSBLOG_ALL_END) {
			break;
		} else if (lineCheck == EUSBLOG_LINE_END) {
			SendMessage(lbox, LB_ADDSTRING, 0, (LPARAM)logrec);
			continue;
		} else if (lineCheck == EUSBLOG_TCM_END) {
			isTcmEnd = true;
			continue;
		} else {
		}
		// 読込長0byteは次行へ
		lineLen = ((strlen(logrec) - 1) / 2);	// 改行コード抜いたデータの半分
		if(lineLen <= 1) {
			continue;
		}
		cntLineLen = lineLen;

		// 読込バッファのデータをバイナリ化
		ascii2bin((unsigned char*)logrec, binbuffer);

		unsigned char lineEnd = 0x00;
		unsigned short pos = 0;

		if(((binbuffer[pos] & 0x80) == 0) || (isTcmEnd == true)) {
			syslogFlg = true;		// syslog
		} else {
			syslogFlg = false;		// tcm
			if (isTcmEnd) {
				break;
			}
		}

		while (!lineEnd) {
			// syslog
			if(syslogFlg) {
				// ログ途中でパケット改行時
				// (headerがちょん切れている場合は次行を結合して
				// 引数合わせたログ長取得のための前準備)
				if (cntLineLen < 7) {
					memcpy(binbuffer, &binbuffer[pos], cntLineLen);
					if(!fgets(logrec, sizeof(logrec), stream)) {
						logEnd = true;
						break;
					}
					lineCheck = logEndCheck(logrec);
					if (lineCheck == EUSBLOG_ALL_END) {
						logEnd = true;
						break;
					} else if (lineCheck == EUSBLOG_LINE_END) {
						SendMessage(lbox, LB_ADDSTRING, 0, (LPARAM)logrec);
						lineEnd = 0x01;
						continue;
					} else {
					}
					ascii2bin((unsigned char*)logrec, &binbuffer[cntLineLen]);
					lineLen = ((strlen(logrec) - 1) / 2) + cntLineLen;
					cntLineLen = lineLen;
					pos = 0;
				}
				// (引数合わせたログ長取得、満たない場合は次行を結合)
				log_pkt_len = 7 + GET_LOG_LENGTH(binbuffer[pos+6]);
				if (cntLineLen < log_pkt_len) {
					memcpy(binbuffer, &binbuffer[pos], cntLineLen);
					if(!fgets(logrec, sizeof(logrec), stream)) {
						logEnd = true;
						break;
					}
					lineCheck = logEndCheck(logrec);
					if (lineCheck == EUSBLOG_ALL_END) {
						logEnd = true;
						break;
					} else if (lineCheck == EUSBLOG_LINE_END) {
						SendMessage(lbox, LB_ADDSTRING, 0, (LPARAM)logrec);
						lineEnd = 0x01;
						continue;
					} else {
					}
					ascii2bin((unsigned char*)logrec, &binbuffer[cntLineLen]);
					lineLen = ((strlen(logrec) - 1) / 2) + cntLineLen;
					cntLineLen = lineLen;
					pos = 0;
				}
				// エラーレベル
				char err_lvl = (binbuffer[pos] & 0x78) >> 3;
				if ((err_lvl < 0) || (err_lvl > 3)) {
					pos++;
					cntLineLen--;
					SendMessage(lbox, LB_ADDSTRING, 0, (LPARAM)"不明なlogLevel\n");
					continue;
				}
				// タイムスタンプ
				unsigned long tim = ((unsigned long)((binbuffer[pos] & 0x07) << 24)
							+ (unsigned long)(binbuffer[pos+1] << 16)
							+ (unsigned long)(binbuffer[pos+2] <<  8)
							+ binbuffer[pos+3]);
				// グループID
				char group_id = (binbuffer[pos+4] & 0xfc) >> 2;
				if (group_id >= log_group_num) {
					pos++;
					cntLineLen--;
					SendMessage(lbox, LB_ADDSTRING, 0, (LPARAM)"不明なLogGroup\n");
					continue;
				}
				// ログID
				unsigned short log_no = (unsigned short)(0x00ff & binbuffer[pos+5]);
				log_no |= (unsigned short)((binbuffer[pos+4] & 0x03) << 8);
				// 引数タイプ
				char arg_type = (binbuffer[pos+6] & 0xf0) >> 4;
				// 引数の数
				char arg_num = (binbuffer[pos+6] & 0x0f);
				char* msg = get_msgtbl(group_id, log_no, arg_num);
				if(msg == NULL) {
					pos++;
					cntLineLen--;
					SendMessage(lbox, LB_ADDSTRING, 0, (LPARAM)"不明なlog\n");
					continue;
				}
				// ログ文字列生成
				int max_len = createLogMessage(NULL, tim, log_no, err_lvl, group_id, arg_type, arg_num, msg, &binbuffer[pos+7]);

				writeUsbLogBin(&binbuffer[pos],log_pkt_len);
				char* visualLogOffset = strstr(line_mbuf, "VisualLog: ");
				if (visualLogOffset != NULL) {
					if (getVisualLogDisp()) {
						SendMessage(lbox, LB_ADDSTRING, 0, (LPARAM)line_mbuf);
					}
				} else {
					SendMessage(lbox, LB_ADDSTRING, 0, (LPARAM)line_mbuf);
				}
				// 次エントリーの位置設定
				pos += (unsigned short)log_pkt_len;
				cntLineLen -= log_pkt_len;
			} else {
				// tcm
				// ログ途中でパケット改行時
				if (cntLineLen < 8) {
					memcpy(binbuffer, &binbuffer[pos], cntLineLen);
					if(!fgets(logrec, sizeof(logrec), stream)) {
						logEnd = true;
						break;
					}
					lineCheck = logEndCheck(logrec);
					if (lineCheck == EUSBLOG_ALL_END) {
						logEnd = true;
						break;
					} if (lineCheck == EUSBLOG_LINE_END) {
						SendMessage(lbox, LB_ADDSTRING, 0, (LPARAM)logrec);
						lineEnd = true;
						break;
					} else if (lineCheck == EUSBLOG_TCM_END) {
						isTcmEnd = true;
						break;
					} else {
					}
					ascii2bin((unsigned char*)logrec, &binbuffer[cntLineLen]);
					lineLen = ((strlen(logrec) - 1) / 2) + cntLineLen;
					cntLineLen = lineLen;
					pos = 0;
				}
				if (!isTcmEnd) { // allInfo以降のデータはタイミングチャートの解析を行わない。
					// deviceType取得
					unsigned char devType = (binbuffer[pos] >> 3) & 0x0f;
					// タイムスタンプ取得
					unsigned long timeStamp = (((binbuffer[pos] & 0x07) << 24)
												+ (binbuffer[pos+1] << 16)
												+ (binbuffer[pos+2] <<  8)
												+ binbuffer[pos+3]);
					// 差分時間取得
					unsigned long diffTm = getTcmDiffTime (&tcm_preTime, timeStamp);
					// データ取得
					unsigned short id = (unsigned short)(binbuffer[pos+4] << 3) + ((binbuffer[pos+5] >> 5) & 7);
					unsigned char io = (binbuffer[pos+5] >> 4) & 1;
					unsigned char onoff = (binbuffer[pos+5] >> 1) & 7;
					unsigned char valInvalid = binbuffer[pos+5] & 1;
					unsigned short val = (unsigned short)(binbuffer[pos+6] << 8) + binbuffer[pos+7];
					unsigned long data = (unsigned long)(val << 16)
											+ (unsigned long)(valInvalid << 15)
											+ (unsigned long)(onoff << 12)
											+ (unsigned long)(io << 11)
											+ (unsigned long)id;

					totalTm = totalTm + diffTm;
					unsigned char param = 0;
					// シナリオはパラメータ部を別idにする
					if (senarioSeparete(id)) {
						if (val >= TCM_PAGEID_DIGIT) {
							if (unit == EUNIT_TYPE_DPS_ENGINE) {
								param = ((val >> 8) & (0xff));
								val = (val & 0xff);
							} else {
								param = val / 1000;
								val -= (param * 1000);
							}	
						}
					}

					// リストに登録
					char deviceId[9];
					sprintf_s(deviceId, 9, "%03x%04x%d", param, id, io);
					// onoffは3ビット分しか格納できないので
					// そのすべてが1なら-1となる。
					char fixedOnoff = onoff;
					if(onoff == 7) {
						fixedOnoff = -1;
					}
					TCMLOG tcmLog = {deviceId, io, fixedOnoff, val, totalTm, param};
					listTcmLog.push_back(tcmLog);
				}
				// 次エントリーの位置設定
				pos += 8;
				cntLineLen -= 8;
			}
			// 読込文字列の解析完了で次行へ
			if (pos >= lineLen) {
				lineEnd = 0x01;
			}
		}
		if (true == logEnd) {
			break;
		}
	}

	writeTcdFile(wstream, listTcmLog, szIniFilePublic);

	logFileClose();
	setVisualAnalyzerMode(syslog_file);
	//---------------------------------------------------------
	//ファイルClose処理
	fclose(wstream);
	fclose(stream);

	return (INT_PTR)TRUE;

}

//==================================================
//	Log表示保存処理
//--------------------------------------------------
//	<引数>
//	hDlg				:ダイアログハンドラ
//	filename_full		:保存ファイル名(*.log)
//--------------------------------------------------
//	<処理概要>
//	ログ表示内容を指定されたログファイル(*.log)に保存する
//==================================================
INT_PTR LogSave(HWND hDlg, char *filename_full)
{
	char logrec[2048];

	static HWND lbox = GetDlgItem(hDlg, IDC_SYSLOG);
	int logcnt = SendMessage(lbox, LB_GETCOUNT, 0, 0);
	if(logcnt == LB_ERR || logcnt <= 0) {
		sprintf_s(logrec, "ログデータが0件のためファイル更新しません。");
		MessageBox(NULL, TEXT(logrec), TEXT("終了"), MB_OK);
		return (INT_PTR)FALSE;
	}

	FILE* stream;
	errno_t result = fopen_s(&stream, filename_full, "w+");
	if (result != 0) {
		sprintf_s(logrec, "ファイルオープンに失敗しました(err=%d)", result);
		MessageBox(NULL, TEXT(logrec), TEXT("エラー"), MB_OK | MB_ICONSTOP);
		return (INT_PTR)FALSE;
	}

	for(int i = 0; i < logcnt; i++) {
		SendMessage(lbox, LB_GETTEXT, i, (LPARAM)logrec);
		fputs(logrec, stream);
	}

	fclose(stream);
	return (INT_PTR)TRUE;

}

//==================================================
//	"dev_proc_edbg_all.log"ソフトバージョン取得処理
//--------------------------------------------------
//	<引数>
//	fileFullPath		:"dev_proc_edbg_all.log"のフルパス
//	SoftVer				:取得ソフトバージョン格納バッファ
//--------------------------------------------------
//	<処理概要>
//	"dev_proc_edbg_all.log"を読み込み、バージョン情報のENG Soft
//	設定値を取得する
//==================================================
INT_PTR getLogSoftVer(char *fileFullPath, char *SoftVer)
{

	char	ReadData[1024];		//読み込みデータ(1行分)
	char	GetData[SOFTVER_BUFFER_SZ];		//取得ソフトバージョン

	char	*delim = ":\n";		//デリミタ（複数渡せる）ここではカンマと改行コード
								//改行コード入れないと、分割文字列の最終項目に改行コードがついてきて想定外の動作を起こしました...。
	char	*ctx;				//内部的に使用するので深く考えない 
	char	*next;
	int		i;
	int		ret;

//	memset(SoftVer, 0, sizeof(SoftVer));	// pointerサイズ(4byte)となるため(≠バッファサイズ)
	memset(SoftVer, 0, SOFTVER_BUFFER_SZ);

	//---------------------------------------------------------
	//ファイルOpen処理
	FILE* stream;
	fopen_s(&stream, fileFullPath, "r");				//"dev_proc_edbg_all.log"をReadモードでOpen

	//EOFまでファイル読込
	while(fgets(ReadData, sizeof(ReadData), stream)) {
		//"Eng Soft"文字列チェック
		if(!memcmp("Eng Soft", ReadData, 8)) {
			//取得文字列を":"で分割
			i = 0;
			next = strtok_s(ReadData, delim, &ctx); 
		    while(next){ 
		        next = strtok_s(null_ptr, delim, &ctx); 
				i++;
				if (i == 1) {
					//分割データの2項目について前後の空白削除し、戻り値として設定
					strcpy_s(GetData, next);
					ret = Trim(GetData);				//空白除去
					strcpy_s(SoftVer, SOFTVER_BUFFER_SZ, GetData);	//取得ソフトバージョン格納バッファ設定
					break;	//Loop抜ける
				}
			} 
			break;			//Loop抜ける
		}
	}

	//---------------------------------------------------------
	//ファイルClose処理
	fclose(stream);

	return (INT_PTR)TRUE;

}

//==================================================
//	ソフトバージョン対応情報取得処理
//--------------------------------------------------
//	<引数>
//	SoftVer				:取得対象ソフトバージョン
//--------------------------------------------------
//	<処理概要>
//	指定されたソフトバージョンに対する各設定値を取得する
//	指定フォルダに各設定値ファイルが存在しない場合、デフォルトの
//	設定値ファイルを読込
//==================================================
INT_PTR getSoftVerSettingData(char *SoftVer)
{

	int		cnt = 0;
	char	TargetDir[_T_MAX_PATH];		//設定値取得対象ディレクトリ
	char	FileChk[_T_MAX_PATH];		//ファイル存在確認用
	DWORD	dwRet;
	char	szMyPath[_T_MAX_PATH];		//プロセスパス

	//---------------------------------------------------------
	//設定値取得対象ディレクトリ編集
	// 実行中のプロセスのフルパス名を取得
	dwRet = GetModuleFileName(NULL, szMyPath, sizeof(szMyPath));
	splitpath(szMyPath);
	strcpy_s(TargetDir, szGlDrive);
	strcat_s(TargetDir, szGlPath);
	strcat_s(TargetDir, DefaltDir);

//ソフトバージョンに関する部分は後ほど編集するよう修正
//	strcat_s(TargetDir, SoftVer);

//	strcat_s(TargetDir, "\\");

	//---------------------------------------------------------
	//フォルダ存在確認
	//存在しない場合、振替プロジェクトに対するフォルダが存在するか確認
	//振替プロジェクトに対するフォルダが存在した場合、そのフォルダから
	//設定値を取得する為、設定値取得対象フォルダを置き換える
	int		i = 0;
	int		j = 0;
	char	szPRJ[5];					//ソフトバージョンのプロジェクトコード
	char	szVer[20];					//ソフトバージョンのバージョン部
	int		intExistFlg = 0;			//振替プロジェクト存在確認フラグ[0:存在無,1:存在有]
	char	szChkPath[_T_MAX_PATH];		//存在確認対象パス

	memset(szPRJ, 0, sizeof(szPRJ));	//退避領域初期化
	strncpy_s(szPRJ, SoftVer, 3);		//ソフトバージョンのプロジェクトコード退避
	strncpy_s(szVer, SoftVer + 3, 15);	//SoftVerの先頭+3の位置(PRJ部除く)から15文字をコピー

	//今回処理対象のソフトバージョンに対する振替対象データが存在するか確認
	for (i = 0; i < TransPJ_cnt; i++) {
		for (j = 0; j < 10; j++) {
			if(!memcmp(TransferPJ[i][j], szPRJ, 3)) {
				intExistFlg = 1;
				break;
			}
		}
		if (intExistFlg == 1) break;
	}
	
	//フォルダ存在確認
	//フォルダが見つかった場合、設定値取得対象フォルダを置き換えてLoop終了
	if (intExistFlg == 1) {
		for (j = 0; j < 10; j++) {
			//確認フォルダへのパスを編集
			memset(szChkPath, 0, sizeof(szChkPath));
			strcpy_s(szChkPath, TargetDir);
			strcat_s(szChkPath, TransferPJ[i][j]);
			strcat_s(szChkPath, szVer);
			if (PathIsDirectory(szChkPath)) {
				memset(TargetDir, 0, sizeof(TargetDir));		//退避領域初期化
				strcpy_s(TargetDir, szChkPath);
				break;
			}
		}
	}


	//---------------------------------------------------------
	//設定値取得(ファイルパス関連)
	//tool_public.iniのパス編集
	if (szPulicIniFileName[0] != 0x00) {	//コマンドライン引数でINIファイル指定の場合
		//ファイル存在確認
		sprintf_s(FileChk, "%s\\%s", TargetDir, szPulicIniFileName);
		if(PathFileExists(FileChk)) {
			strcpy_s(szIniFilePublic, FileChk);
		}
	} else {
		//ファイル存在確認
		memset(FileChk, 0, sizeof(FileChk));		//退避領域初期化
		strcpy_s(FileChk, TargetDir);
//		strcat_s(FileChk, "tool_public.ini");
		strcat_s(FileChk, "\\tool_public.ini");
		if(PathFileExists(FileChk)) {
			strcpy_s(szIniFilePublic, FileChk);		//ファイル存在時、ソフトバージョン対応INIファイルを使用
		}
	}

	//tool_private.iniのパス編集
	//ファイル存在確認
	memset(FileChk, 0, sizeof(FileChk));			//退避領域初期化
	strcpy_s(FileChk, TargetDir);
//	strcat_s(FileChk, "tool_private.ini");
	strcat_s(FileChk, "\\tool_private.ini");
	if(PathFileExists(FileChk)) {
		strcpy_s(szIniFilePrivate, FileChk);		//ファイル存在時、ソフトバージョン対応INIファイルを使用
	}

	//DebuggerLogMsg.mesのパス編集
	//ファイル存在確認
	memset(FileChk, 0, sizeof(FileChk));			//退避領域初期化
	strcpy_s(FileChk, TargetDir);
//	strcat_s(FileChk, "DebuggerLogMsg.mes");
	strcat_s(FileChk, "\\DebuggerLogMsg.mes");
	if(PathFileExists(FileChk)) {
		//ファイル存在時、ソフトバージョン対応DebuggerLogMsg.mesを使用し、メッセージ管理テーブル再生成
		sprintf_s(logmsg_file, _T_MAX_PATH, "%s\\DebuggerLogMsg.mes", TargetDir);
		remove_msgtbl();		// ログメッセージ管理テーブル削除
		create_msgtbl();		// ログメッセージ管理テーブル生成
	}

	getSettingData();			//設定値再取得

	return (INT_PTR)TRUE;

}

//==================================================
//	文字列の先頭と末尾にある空白を削除する
//--------------------------------------------------
//	<引数>
//	param[in] s		:対象文字列
//	return			:削除した空白の数を返す
//==================================================
int Trim(char *s) {

	int i;
    int count = 0;

    /* 空ポインタか? */
    if ( s == NULL ) { /* yes */
        return -1;
    }
    
    i = strlen(s);									/* 文字列長を取得する */
    while ( --i >= 0 && s[i] == ' ' ) count++;		/* 末尾から順に空白でない位置を探す */
    s[i+1] = '\0';									/* 終端NULL文字を付加する */
    i = 0;
    while ( s[i] != '\0' && s[i] == ' ' ) i++;		/* 先頭から順に空白でない位置を探す */

	strcpy_s(s, SOFTVER_BUFFER_SZ, &s[i]);	// 最大でもGetData[]のサイズ分まで。

	return i + count;

}

//==================================================
//	設定データ取得処理
//--------------------------------------------------
//	<引数>	なし
//--------------------------------------------------
//	<処理概要>
//	TCD作成用データ、ログ変換データを取得する
//==================================================
INT_PTR getSettingData()
{
	#define INI_BUFF_LEN		(100)

	int		cnt = 0;					//INIファイル取得時Loop回数カウント用
	TCHAR	subsec[16];					//INIファイル取得Key名称編集用
	char	szBuff[INI_BUFF_LEN];		//INIファイル取得結果バッファ用

	extern	BOOL log_group_disp[];		//ログ表示・非表示区分退避用配列
	extern	char defLogLvl;				//INI取得結果退避用[セクション:DEFAULT_SETTING, Key:LOG_LEVEL]

	//退避領域初期化
	defLogLvl = 0;						//INI値取得のデフォルト値が"0"なので初期値は0設定しておく
	log_group_num = 0;					//ロググループ数初期化

	//---------------------------------------------------------
	//teratermログファイル
	//---------------------------------------------------------
	GetPrivateProfileString(_T("FILE"), _T("SYSLOGFILE"), _T(DEFAULT_SYSLOG_FILE), syslog_file, sizeof(syslog_file), szIniFilePrivate);
	char full[_T_MAX_PATH];
	_fullpath(full, syslog_file, _T_MAX_PATH);
	strcpy_s(syslog_file, full);

	//---------------------------------------------------------
	//TCD作成情報(グローバル変数宣言は[saveTcdFile.cpp])
	//---------------------------------------------------------
	//クロックあたりの時間
	msPerClock = (unsigned long)GetPrivateProfileInt(_T("TCD_SETTING"), _T("MSEC_PER_CLOCK"), 2, szIniFilePublic);
	//直近の信号と時間が同じだった場合にずらす時間
	collisionDeleyTime = (unsigned long)GetPrivateProfileInt(_T("TCD_SETTING"), _T("COLLISION_DELAY_TIME"), 1, szIniFilePublic);
	//iniファイルに書かれている順にソートするか
	sortDevice = GetPrivateProfileInt(_T("TCD_SETTING"), _T("SORT_DEVICE"), 0, szIniFilePublic);

	//---------------------------------------------------------
	// ロググループ文字列
	//---------------------------------------------------------
	ZeroMemory(log_group, sizeof(char*)*LOGGRP_DEFNUM);
	for(cnt = 0; cnt < LOGGRP_DEFNUM; cnt++) {
		sprintf_s(subsec, _T("LG%02d"), cnt);
		//INIファイルデータ取得(セクション=[LOG_GROUP_STRING],キー名=LGXX,デフォルト値="***")
		GetPrivateProfileString(_T("LOG_GROUP_STRING"), subsec, _T("***"), szBuff, sizeof(szBuff), szIniFilePublic);
		if(!memcmp(szBuff, "***", 3)) break;		//INIファイルよりデータ取得できなかった場合、処理終了
		log_group[cnt] = (char*)malloc(INI_BUFF_LEN);
		strcpy_s(log_group[cnt], INI_BUFF_LEN, szBuff);
		log_group_disp[cnt] = TRUE;
		log_group_num++;
	}
	tcmLogGroupNo = log_group_num;
	log_group_disp[tcmLogGroupNo] = TRUE;
	//INIファイルデータ取得(セクション=[DEFAULT_SETTING],キー名=LOG_LEVEL,デフォルト値=0)
	defLogLvl = (char)GetPrivateProfileInt(_T("DEFAULT_SETTING"), _T("LOG_LEVEL"), 0, szIniFilePublic);

	return (INT_PTR)((log_group_num>0)? TRUE: FALSE);

}

void splitpath(TCHAR* fullpath)
{
	// 取得したファイルパスを分割
	_tsplitpath_s(fullpath,
					szGlDrive, _T_MAX_DRIVE,
					szGlPath, _T_MAX_DIR,
					szGlFName, _T_MAX_FNAME,
					szGlExt, _T_MAX_EXT);
}

void splitpath2(TCHAR* fullpath)
{
	// 取得したファイルパスを分割
	_tsplitpath_s(fullpath,
					szGlDrive2, _T_MAX_DRIVE,
					szGlPath2, _T_MAX_DIR,
					szGlFName2, _T_MAX_FNAME,
					szGlExt2, _T_MAX_EXT);
}

void useCount(char *func)
{
	char szUserName[100];
	char numText[10];
	int useNum;
	DWORD dwSize = sizeof(szUserName)/sizeof(szUserName[0]);
	if (isConnectCountInfoFile == 1) {
		GetUserName(szUserName, &dwSize);
		GetPrivateProfileString(func, szUserName, _T("0"), numText, sizeof(numText), countInfoFileName);
		useNum = atoi(numText) + 1;
		sprintf_s(numText, "%d", useNum);
		WritePrivateProfileString(func, szUserName, numText, countInfoFileName);
		GetPrivateProfileString(func, szUserName, _T("0"), numText, sizeof(numText), allCountInfoFileName);
		useNum = atoi(numText) + 1;
		sprintf_s(numText, "%d", useNum);
		WritePrivateProfileString(func, szUserName, numText, allCountInfoFileName);
	}
}

INT_PTR GetArgFileName (const char *lpCmdLine)
{
	int ret = FALSE;
	if (lpCmdLine == NULL) return ret;

	// コマンドライン文字列をコピー（元の文字列を破壊しないため）
	char cmdline_copy[256 * 4] = { 0 };
	strncpy_s(cmdline_copy, lpCmdLine, sizeof(cmdline_copy) - 1);

	// トークン分割用
	char* token1 = NULL;	// 第1引数
	char* token2 = NULL;	// 第2引数
	char* next_token = NULL;

	// 最初のトークン（第1引数）
	token1 = strtok_s(cmdline_copy, " \t", &next_token);
	if (token1 != NULL) {
		splitpath(token1);
		// 1番目の引数を szPulicIniFileName にコピー
		//ファイルDrop時はコマンドライン引数にINIファイル以外が設定される場合が
		//ある為、引数指定されたファイルがINIファイルのみ変数退避する
		if (!memcmp(szGlExt, ".ini", 3)) {
			strncpy_s(szPulicIniFileName, token1, sizeof(szPulicIniFileName) - 1);
			szPulicIniFileName[sizeof(szPulicIniFileName) - 1] = '\0';
		}
	}
	// 2番目のトークン（第2引数）
	token2 = strtok_s(NULL, " \t", &next_token);
	if (token2 != NULL) {
		splitpath2(token2);
		// 2番目の引数が存在するので変数にコピー
		strncpy_s(szCmdLine2FileName, token2, sizeof(szCmdLine2FileName) - 1);
		szCmdLine2FileName[sizeof(szCmdLine2FileName) - 1] = '\0';
	}
	ret = TRUE;

	return ret;
}

INT_PTR unzipConvertLogFile (HWND hwnd, TCHAR* fullpath)
{
	_TCHAR szDrive[_T_MAX_DRIVE];
	_TCHAR szPath[_T_MAX_DIR];
	_TCHAR szFName[_T_MAX_FNAME];
	_TCHAR szExt[_T_MAX_EXT];
	char szFileName[_T_MAX_PATH];
	char SoftVer[SOFTVER_BUFFER_SZ];

	// コマンドライン引数起動後の変換時
	if (strcmp(szCmdLine2FileName, "")) {
		splitpath2(fullpath);
		::_tcscpy_s(szDrive, _T_MAX_DRIVE * sizeof(_TCHAR), szGlDrive2);
		::_tcscpy_s(szPath, _T_MAX_DIR * sizeof(_TCHAR), szGlPath2);
		::_tcscpy_s(szFName, _T_MAX_FNAME * sizeof(_TCHAR), szGlFName2);
		::_tcscpy_s(szExt, _T_MAX_EXT * sizeof(_TCHAR), szGlExt2);
	} else { // Drop処理時
		//取得したファイルパスを分割
		splitpath(fullpath);
		::_tcscpy_s(szDrive, _T_MAX_DRIVE * sizeof(_TCHAR), szGlDrive);
		::_tcscpy_s(szPath, _T_MAX_DIR * sizeof(_TCHAR), szGlPath);
		::_tcscpy_s(szFName, _T_MAX_FNAME * sizeof(_TCHAR), szGlFName);
		::_tcscpy_s(szExt, _T_MAX_EXT * sizeof(_TCHAR), szGlExt);
	}
	memcpy(szFileName, fullpath, sizeof(szFileName));

	//拡張子が*.log or *.gz 以外は処理対象外
	if (!strcmp(szExt, ".gz") || !strcmp(szExt, ".log")) {
		if (!strcmp(szExt, ".gz")) {
			//解凍処理
			if (!USBFileDecomp(szFileName)) return TRUE;			//解凍処理
			//解凍後のファイルをDropされたファイルとして設定
			memset(szFileName, 0, sizeof(szFileName));	//コマンドライン引数ファイル名退避領域初期化
			strcpy_s(szFileName, szDrive);
			strcat_s(szFileName, szPath);
			strcat_s(szFileName, engnFileNameEntry2);

			if (!PathFileExists(szFileName)) {
				//解凍後のファイルをDropされたファイルとして設定
				memset(szFileName, 0, sizeof(szFileName));	//コマンドライン引数ファイル名退避領域初期化
				strcpy_s(szFileName, szDrive);
				strcat_s(szFileName, szPath);
				strcat_s(szFileName, engnFileNameEntry1);
			}
		}
		//dev_proc_edbg_all.logのEngSoftのバージョンに対応したINIファイルより設定値取得
		getLogSoftVer(szFileName, SoftVer);	//dev_proc_edbg_all.logよりソフトバージョン取得
		//ソフトバージョンが取得できた場合、取得バージョンに応じた各種設定ファイルを再取得
		if (strcmp(SoftVer, "")) {
			getSoftVerSettingData(SoftVer);										//ソフトバージョン対応情報取得処理
		}

		memset(szFileName, 0, sizeof(szFileName));	//コマンドライン引数ファイル名退避領域初期化
		strcpy_s(szFileName, szDrive);
		strcat_s(szFileName, szPath);
		strcat_s(szFileName, engnFileNameEntry2);
		//*.log⇒*.tcd変換処理
		char TargetFileName[_T_MAX_PATH];
		//*.tcdのファイル名はドロップされたファイルの拡張子より前の部分を使用
		char* FileName;
		char* p;
		FileName = strtok_s(szFName, ".", &p);
		strcpy_s(TargetFileName, FileName);

		//*.tcdファイル名生成
		char targetFile[2048] = "";
		strcpy_s(targetFile, szDrive);
		strcat_s(targetFile, szPath);
		strcat_s(targetFile, TargetFileName);
		strcat_s(targetFile, ".tcd");
		if (!USBLogConvert(hwnd, szFileName, targetFile)) return (INT_PTR)TRUE;	//*.log⇒*.tcd変換処理

		//ログ表示保存処理
		//ログ表示保存用ファイル名生成
		memset(targetFile, 0, sizeof(targetFile));
		strcpy_s(targetFile, szDrive);
		strcat_s(targetFile, szPath);
		strcat_s(targetFile, TargetFileName);
		strcat_s(targetFile, ".log");
		if (!LogSave(hwnd, targetFile)) return (INT_PTR)TRUE;		//ログ表示内容ファイル保存処理
	}
	return (INT_PTR)TRUE;
}