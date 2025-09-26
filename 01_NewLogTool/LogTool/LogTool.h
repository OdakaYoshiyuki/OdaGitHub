#pragma once

#include "resource.h"
#include "config.h"

// ���b�Z�[�W�Ǘ��e�[�u���\���̒�`
typedef struct {
	int mes_entry;		// ���b�Z�[�W�o�^��
	int** mesID;		// ���b�Z�[�WID�e�[�u���ւ̃|�C���^
	char** mestbl;		// ���b�Z�[�W�e�[�u���ւ̃|�C���^
} MESSTR;

// �֐��̐錾
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
#define WINDOW_WIDTH  (300)		// �E�B���h�E�̕�
#define WINDOW_HEIGHT (100)		// �E�B���h�E�̍���

#define TIMER_ID_LOGUPDATE 2

#define LIN_FEED_CODE_CR	0x01
#define LIN_FEED_CODE_LF	0x02
#define LIN_FEED_CODE_CRLF	LIN_FEED_CODE_LF|LIN_FEED_CODE_CR

#define SYSLOG_MODELESS			// SYSLOG�_�C�A���O�����[�h���X�ŏo��

// Teraterm���O�p�X
// (tool.ini�̒�`���Q�Ƃ���邽�߂��̒�`�͐G��K�v�Ȃ�)
#define DEFAULT_SYSLOG_FILE		"c:\\test_syslog.bin"

#define SOFTVER_BUFFER_SZ		32

// ���O���x���C���f�b�N�X��`
#define LOGLVL_ERROR		0
#define LOGLVL_WARNING		1
#define LOGLVL_INFORMATION	2
#define LOGLVL_SYSTEM		3

// USB���O �G���W�����O���܂܂��f�t�H���g�t�@�C������`
#define USBLOG_DEFAULT_ENGINE_FILE		"dev_proc_engine.log"

// �t�@�C���p�X������o�b�t�@�T�C�Y��`
#define _T_MAX_PATH  (_MAX_PATH  * (sizeof(wchar_t) / sizeof(_TCHAR)))
#define _T_MAX_DRIVE (_MAX_DRIVE)
#define _T_MAX_DIR   (_MAX_DIR   * (sizeof(wchar_t) / sizeof(_TCHAR)))
#define _T_MAX_FNAME (_MAX_FNAME * (sizeof(wchar_t) / sizeof(_TCHAR)))
#define _T_MAX_EXT   (_MAX_EXT   * (sizeof(wchar_t) / sizeof(_TCHAR)))

// �ϐ��ԊҎ��̎��
#define INTEGER_VALUE	0
#define FLOAT_VALUE		1

// USB���OLog�I�����f������o�b�t�@��
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

// ���O���b�Z�[�W�Ǘ��e�[�u������
extern void create_msgtbl();
// ���O���b�Z�[�W�ւ̃|�C���^�擾
extern char* get_msgtbl(char group_id, int index, char arg_num);
// ���O���b�Z�[�W�Ǘ��e�[�u���폜
void remove_msgtbl();
// �t�@�C���p�X�������ʏ���
void splitpath(TCHAR* fullpath);
// Visual���O�̕\����
extern bool getVisualLogDisp();

// ���O���b�Z�[�W�t�@�C���p�X
extern char logmsg_file[];
// ���O�O���[�v�e�[�u��
extern char* log_group[];
extern char log_group_num;
// ���O���b�Z�[�W�t�@�C����������	
extern char logmsg_mkdate[];
// �t�@�C���p�X������o�b�t�@��`
extern _TCHAR szGlDrive[];
extern _TCHAR szGlPath[];
extern _TCHAR szGlFName[];
extern _TCHAR szGlExt[];
extern char useCountUpPath[_T_MAX_PATH];
// USB���O���̃G���W�����O�t�@�C�������
extern TCHAR engnFileNameEntry1[];
extern TCHAR engnFileNameEntry2[];
//�C�ӂ̃o�b�t�@
extern char buf[256];
//���j�b�g����
extern eUNIT_TYPE unit;
