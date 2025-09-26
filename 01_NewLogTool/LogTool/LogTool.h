#pragma once

#include "resource.h"
#include "config.h"

// メッセージ管理テーブル構造体定義
typedef struct {
	int mes_entry;		// メッセージ登録数
	int** mesID;		// メッセージIDテーブルへのポインタ
	char** mestbl;		// メッセージテーブルへのポインタ
} MESSTR;

// 関数の宣言
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	DispSyslog(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	Setting(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	dateSetting(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	download(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	passWord(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	comSetting(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	autoCommandSenderProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR				SaveTcmlogToFile(HWND hDlg);
BOOL				DeleteDirectory(LPCTSTR lpPathName);
INT_PTR				USBFileDecomp(char *);
INT_PTR				USBLogConvert(HWND, char *, char *);
INT_PTR				LogSave(HWND, char *);
INT_PTR				tcmSave(HWND, char *);
INT_PTR				getLogSoftVer(char *, char *);
INT_PTR				getSoftVerSettingData(char *);
int					Trim(char *s);
INT_PTR				getSettingData();
INT_PTR				GetArgFileName(const char *lpCmdLine);
INT_PTR				unzipConvertLogFile(HWND hwnd, TCHAR* fullpath);

#define MAX_LOADSTRING 100
#define WINDOW_WIDTH  (300)		// ウィンドウの幅
#define WINDOW_HEIGHT (100)		// ウィンドウの高さ

#define TIMER_ID_LOGUPDATE 2

#define LIN_FEED_CODE_CR	0x01
#define LIN_FEED_CODE_LF	0x02
#define LIN_FEED_CODE_CRLF	LIN_FEED_CODE_LF|LIN_FEED_CODE_CR

#define SYSLOG_MODELESS			// SYSLOGダイアログをモードレスで出力

// Teratermログパス
// (tool.iniの定義が参照されるためこの定義は触る必要なし)
#define DEFAULT_SYSLOG_FILE		"c:\\test_syslog.bin"

#define SOFTVER_BUFFER_SZ		32

// ログレベルインデックス定義
#define LOGLVL_ERROR		0
#define LOGLVL_WARNING		1
#define LOGLVL_INFORMATION	2
#define LOGLVL_SYSTEM		3

// USBログ エンジンログが含まれるデフォルトファイル名定義
#define USBLOG_DEFAULT_ENGINE_FILE		"dev_proc_engine.log"

// ファイルパス文字列バッファサイズ定義
#define _T_MAX_PATH  (_MAX_PATH  * (sizeof(wchar_t) / sizeof(_TCHAR)))
#define _T_MAX_DRIVE (_MAX_DRIVE)
#define _T_MAX_DIR   (_MAX_DIR   * (sizeof(wchar_t) / sizeof(_TCHAR)))
#define _T_MAX_FNAME (_MAX_FNAME * (sizeof(wchar_t) / sizeof(_TCHAR)))
#define _T_MAX_EXT   (_MAX_EXT   * (sizeof(wchar_t) / sizeof(_TCHAR)))

// 変数返還時の種別
#define INTEGER_VALUE	0
#define FLOAT_VALUE		1

// USBログLog終了判断文字列バッファ数
#define USBLOG_PARTITION_BUF	64
#define LOG_PARTITION_NUM		10

enum eUNIT_TYPE {
	EUNIT_TYPE_ENGINE,
	EUNIT_TYPE_DPS_ENGINE,
	EUNIT_TYPE_DF,
	EUNIT_TYPE_PF,
	EUNIT_TYPE_PH,
	EUNIT_TYPE_SFRG,
	EUNIT_TYPE_MOTOR_CPU,
	EUNIT_TYPE_NUM
};
enum eUSBLOG_END_TYPE {
	EUSBLOG_NONE,
	EUSBLOG_TCM_END,
	EUSBLOG_LINE_END,
	EUSBLOG_ALL_END,

	EUSBLOG_END_TYPE_NUM
};

enum eCOMPARISON_TYPE {
	 ECOMPARISON_NONE
	,ECOMPARISON_LESS
	,ECOMPARISON_LESS_EQUAL
	,ECOMPARISON_EQUAL
	,ECOMPARISON_LARGER
	,ECOMPARISON_LARGER_EQUAL
	,ECOMPARISON_NOT_EQUAL
	,ECOMPARISON_END
};

// ログメッセージ管理テーブル生成
extern void create_msgtbl();
// ログメッセージへのポインタ取得
extern char* get_msgtbl(char group_id, int index, char arg_num);
// ログメッセージ管理テーブル削除
void remove_msgtbl();
// ファイルパス分割共通処理
void splitpath(TCHAR* fullpath);
// Visualログの表示可否
extern bool getVisualLogDisp();

// ログメッセージファイルパス
extern char logmsg_file[];
// ロググループテーブル
extern char* log_group[];
extern char log_group_num;
// ログメッセージファイル生成日時	
extern char logmsg_mkdate[];
// ファイルパス文字列バッファ定義
extern _TCHAR szGlDrive[];
extern _TCHAR szGlPath[];
extern _TCHAR szGlFName[];
extern _TCHAR szGlExt[];
extern char useCountUpPath[_T_MAX_PATH];
// USBログ内のエンジンログファイル名候補
extern TCHAR engnFileNameEntry1[];
extern TCHAR engnFileNameEntry2[];
//任意のバッファ
extern char buf[256];
//ユニット識別
extern eUNIT_TYPE unit;
