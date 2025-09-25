// LogTool.cpp : �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
//

//===========================================================================================
//	Ver2.00	2015/10/16	�V�K�쐬�H
//===========================================================================================
//	Ver2.01	2016/01/30	M.Hashimoto
//		[USB���O�\��]Click������������
//			�����O)*.log��I����*.tcd�֕ϊ�����
//			������)���k�t�@�C��(*.gz)��I���ˉ𓀁�*.tcd�֕ϊ�
//				�@ *.log�I�����́A�����O���l*.log��*.tcd�ϊ����s��
//-------------------------------------------------------------------------------------------
//	Ver2.02	2016/02/02	M.Hashimoto
//		[USB���O�\��]Click������������
//			�E���k�t�@�C��������t�H���_�։𓀂���"dev_proc_edbg_all.log"��ۑ�
//			�E"dev_proc_edbg_all.log"�R�s�[��A�𓀃f�[�^�폜
//			�E�h���b�O���h���b�v�Ή�
//-------------------------------------------------------------------------------------------
//	Ver2.03	2016/02/03	M.Hashimoto
//		�h���b�O���h���b�v������[�e�L�X�g�ŕۑ�]Click��������g��
//-------------------------------------------------------------------------------------------
//	Ver2.04	2016/02/04	M.Hashimoto
//		1)*.tcd�y��*.log�̃f�t�H���g�t�@�C������*.tar.gz��*�̕������g�p����悤�C��
//		2)�𓀃t�H���_�̃g�b�v�t�H���_��"log"�Œ�ׁ̈AUSB Log���k�t�@�C���𓀏���(USBFileDecomp)�C��
//-------------------------------------------------------------------------------------------
//	Ver2.05	2016/02/08	M.Hashimoto
//		1)LogTool.exe�t�@�C���Ƀh���b�O���h���b�v�ŏ������s
//-------------------------------------------------------------------------------------------
//	Ver2.06	2016/02/08	M.Hashimoto
//		1)LogTool.exe�t�@�C���Ƀh���b�O���h���b�v�����ꍇ�A�����I�����ʕ���
//		2)���k�t�@�C�����������ꍇ�G���[�������Ă����׏C��
//		3)���O�t�@�C���̓��e��1�s��256�����ȏ�̏ꍇ�G���[�������Ă�����2048�����܂őΉ��\�ƂȂ�悤�C��
//-------------------------------------------------------------------------------------------
//	Ver2.07	2016/02/16	M.Hashimoto
//		1)Drop�ɂď����s���ہAdev_proc_edbg_all.log��EngSoft�̃o�[�W�����ɑΉ�����
//		  �e��ݒ�t�@�C�����ēǍ�����
//		  �������Ӂ���
//		  �eVer�ɑ΂���ݒ�t�@�C���́A[SettingData\2RB_1000.001.013\tool_public.ini]�̂悤��
//		  SettinData�t�H���_�̉��̊e�o�[�W�����P�ʂō쐬���ꂽ�t�H���_�Ɋi�[����Ă��鎖
//		  �t�@�C�������݂��Ȃ��ꍇ�A�f�t�H���g�ǂݍ��݂����t�@�C�����g�p����
//		2)[�o�[�W�������]�{�^��Click���ɕ\��������ʂ̃o�[�W��������ҏW
//		�@����ɔ����A����̈�A��VerUp��1.01�`�Ƃ��Ă������A2.01�`�ɏC��
//-------------------------------------------------------------------------------------------
//	Ver2.08	2016/02/16	M.Hashimoto
//		1)���k�t�@�C���ɋ󔒂��܂܂��ƁATar�R�}���h������ɓ��삵�Ȃ��s��Ή�
//-------------------------------------------------------------------------------------------
//	Ver2.09	2016/02/17	M.Hashimoto
//		1)�v���W�F�N�g�U�ւ��ł���悤�A�U�֐ݒ�INI�t�@�C���쐬
//-------------------------------------------------------------------------------------------
//	Ver2.10	2016/02/25	M.Hashimoto
//		1)�S�𓀁˃t�@�C���w��𓀂��s���悤�C��
//			�t�@�C���w��̉𓀗�)tar -xvzf log.tar.gz log/files/debug_file/dev_proc_edbg_all.log
//		2)Trim()��������strcpy_s�ł̃T�C�Y�w��ɂ��AgetLogSoftVer()�ɂ�RunTime�G���[����
//		�@Run-Time Check Failure #2 - Stack around the variable 'GetData' was corrupted.
//			���L�w��ɕύX
//			strcpy_s(s, 100, &s[i]);	�ˁ@strcpy_s(s, SOFTVER_BUFFER_SZ, &s[i]);
//===========================================================================================

#include "stdafx.h"
#include "LogTool.h"
#include "saveTcdFile.h"
#include <process.h>
#include <ShlObj.h>

#include <stdio.h>
#include <share.h>
#include <locale.h>

//strstr�֐��Astrcmp�֐����p�̈�
#include <string.h>
//
#include <windows.h>
#include <shlwapi.h>
#include <shellapi.h>

//_splitpath_s�֐����p�̈�
#include <stdlib.h>

#include "CommDlg.h"

// �O���[�o���ϐ�:
extern HINSTANCE hInst;					// ���݂̃C���^�[�t�F�C�X
HWND hBaseWindow;						// �x�[�X�E�C���h�E�ւ̃n���h��
HWND hSyslogDlgWnd;						// SYSLOG�_�C�A���O�{�b�N�X�̃n���h��

char syslog_file[_T_MAX_PATH];			// SYSLOG�t�@�C���p�X
char szIniFilePublic[_T_MAX_PATH];		// Ini�t�@�C���p�X
char szIniFilePrivate[_T_MAX_PATH];		// Ini�t�@�C���p�X
char visualAnalyzerIni[_T_MAX_PATH];	// Ini�t�@�C���p�X
char visualAnalyzerName[_T_MAX_PATH];	// �t�@�C����
char logToolSettingPath[_T_MAX_PATH];	// logTool�ݒ�t�@�C���̃p�X
char useCountUpPath[_T_MAX_PATH];		// ���p�󋵃J�E���g�A�b�v�p�X
char szPulicIniFileName[_T_MAX_PATH];	// public.ini�t�@�C����
char connectCheckFileName[_T_MAX_PATH];	// �T�[�o�[�ڑ��m�F�t�@�C���p�X
char countInfoFileName[_T_MAX_PATH];	// �J�E���g���t�@�C���p�X
char allCountInfoFileName[_T_MAX_PATH];	// �J�E���g���t�@�C���p�X
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
char tcmLogGroupNo;						//�^�C�~���O�`���[�g�̃��O�O���[�v�ԍ�
eUNIT_TYPE unit;

TCHAR szWindowsTempPath[_T_MAX_PATH];	// Windows�e���|�����p�X
TCHAR engnFileNameEntry1[64];			// USB���O���̃G���W�����O�t�@�C�������1
TCHAR engnFileNameEntry2[64];			// USB���O���̃G���W�����O�t�@�C�������2

// �R�}���h���C�������t�@�C����
// LogTool.exe�֒���Drop���A�R�}���h���C��������Drop���ꂽ�t�@�C����������
char szCmdLineFileName[_T_MAX_PATH];
char szCmdLine2FileName[_T_MAX_PATH];

OPENFILENAME ofn;
TCHAR filename_full[_T_MAX_PATH];		// �t�@�C����(�t���p�X)���󂯎��̈�
TCHAR filename[_T_MAX_PATH];			// �t�@�C�������󂯎��̈�

extern void terminalMain();
extern void terminalClose();
extern void logFileClose();
extern void logFileReOpen();
extern void writeUsbLogBin(unsigned char* logBuf, size_t len);
extern void setVisualAnalyzerMode(char* syslog_file);
// ���O���b�Z�[�W����
extern int createLogMessage(FILE*, unsigned long, short, char, char, char, char, char*, unsigned char*);

// ���O���b�Z�[�W�o�b�t�@
extern char line_mbuf[2048];

// ���O��
#define GET_LOG_LENGTH(dat) \
			(((((dat & 0xf0) >> 4) == 1) || (((dat & 0xf0) >> 4) == 2))? (1 * (dat & 0x0f)): \
			(((((dat & 0xf0) >> 4) == 4) || (((dat & 0xf0) >> 4) == 5))? (2 * (dat & 0x0f)): \
			(((((dat & 0xf0) >> 4) == 6) || (((dat & 0xf0) >> 4) == 7))? (4 * (dat & 0x0f)): 0)))

#ifndef null_ptr 
#define null_ptr 0 
#endif 

char DefaltDir[] = "SettingData\\";

char	TransferPJ[TRANFER_PJ_MAX][10][4];	//�U�փv���W�F�N�g
int		TransPJ_cnt;						//�U�փv���W�F�N�g����

// _tsplitpath_s�ďo�����̏��擾�p�o�b�t�@
_TCHAR szMyPath[_T_MAX_PATH];		// ���v���Z�X�p�X
_TCHAR szGlDrive[_T_MAX_DRIVE];
_TCHAR szGlPath[_T_MAX_DIR];
_TCHAR szGlFName[_T_MAX_FNAME];
_TCHAR szGlExt[_T_MAX_EXT];
// _tsplitpath_s(�������p)�ďo�����̏��擾�p�o�b�t�@
_TCHAR szGlDrive2[_T_MAX_DRIVE];
_TCHAR szGlPath2[_T_MAX_DIR];
_TCHAR szGlFName2[_T_MAX_FNAME];
_TCHAR szGlExt2[_T_MAX_EXT];

extern volatile bool isLogStartConvert;

//==================================================
//	Main�֐�(_tWinMain)
//--------------------------------------------------
//	<����>
//	hInstance		:���݂̃C���X�^���X�E�n���h��
//	hPrevInstance	:�ȑO�̃C���X�^���X�E�n���h��
//	lpCmdLine		:�R�}���h���C���̕�����
//	nCmdShow		:�E�C���h�E�̕\�����
//--------------------------------------------------
//	<�����T�v>
//	�E��d�N���h�~�m�F
//	�E�R�}���h���C�������m�F
//	�EEXE�փt�@�C��Drop���̏���
//	�E�A�v��������
//	�EWindow�C�x���g�Ď�����
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
	//�R�}���h���C���������w�肳��Ă���ꍇ�A�擾�����t�@�C���p�X�𕪊�
	GetArgFileName(lpCmdLine);

	//���s���̃v���Z�X�̃t���p�X�����擾
	GetModuleFileName(NULL, szMyPath, sizeof(szMyPath));
	// �t���p�X���𕪊�
	splitpath(szMyPath);
	sprintf_s(szMyPath, "%s%s", szGlDrive, szGlPath);
	if (szPulicIniFileName[0] != 0x00) {
		sprintf_s(szIniFilePublic, "%s%s%s", szGlDrive, szGlPath, szPulicIniFileName);
	} else {
		sprintf_s(szIniFilePublic, "%s%stool_public_target.ini", szGlDrive, szGlPath);
	}
	sprintf_s(szIniFilePrivate, "%s%stool_private.ini", szGlDrive, szGlPath);
	sprintf_s(logmsg_file, _T_MAX_PATH, "%sDebuggerLogMsg.mes", szMyPath);// ���b�Z�[�W��`�t�@�C��
	sprintf_s(visualAnalyzerIni, "%s%sVisualAnalyzer.ini", szGlDrive, szGlPath);
	GetPrivateProfileString(_T("PROJECT"), _T("CODE"), _T("XXX"), projectCode, sizeof(projectCode), szIniFilePrivate);
	sprintf_s(mutexName, sizeof(mutexName), "__LogTool__%s", projectCode);
	sprintf_s(captionName, sizeof(captionName), "Syslog�\���c�[��-%s",projectCode);
	SHGetSpecialFolderPath( NULL, logToolSettingPath, CSIDL_PERSONAL, 0 );//�}�C�h�L�������g�̃p�X
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
	// ��d�N���h�~
	HANDLE hMutex = CreateMutex(NULL, TRUE, mutexName);
	if(GetLastError() == ERROR_ALREADY_EXISTS) return FALSE;

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;
	setlocale(LC_ALL,"japanese"); 

	// �O���[�o������������������Ă��܂��B
	MyRegisterClass(hInstance);

	// �A�v���P�[�V�����̏����������s���܂�:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, NULL);
	_beginthreadex(NULL, 0, ThreadFunc, NULL, 0, NULL);

	// ���C�� ���b�Z�[�W ���[�v:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			// �R�}���h���C����USB���O�t�@�C�����w�肵�ċN�������ꍇ�ɕϊ������{����
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
//	<����>
//	hInstance	:
//--------------------------------------------------
//	<�����T�v>
//	�E�B���h�E �N���X��o�^
//--------------------------------------------------
//	<�R�����g>
//    ���̊֐�����юg�����́A'RegisterClassEx' �֐����ǉ����ꂽ
//    Windows 95 ���O�� Win32 �V�X�e���ƌ݊�������ꍇ�ɂ̂ݕK�v�ł��B
//    �A�v���P�[�V�������A�֘A�t����ꂽ
//    �������`���̏������A�C�R�����擾�ł���悤�ɂ���ɂ́A
//    ���̊֐����Ăяo���Ă��������B
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
//	<����>
//	hInstance	:
//	nCmdShow	:
//--------------------------------------------------
//	<�����T�v>
//	�C���X�^���X �n���h����ۑ����āA���C�� �E�B���h�E���쐬
//--------------------------------------------------
//	<�R�����g>
//	���̊֐��ŁA�O���[�o���ϐ��ŃC���X�^���X �n���h����ۑ����A
//	���C�� �v���O���� �E�B���h�E���쐬����ѕ\�����܂��B
//==================================================
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;
   	hInst = hInstance; // �O���[�o���ϐ��ɃC���X�^���X�������i�[���܂��B

	_TCHAR szDrive[_T_MAX_DRIVE];
	_TCHAR szDir[_T_MAX_DIR];
	_TCHAR szTemp[_T_MAX_FNAME];


//��������������O�t�@�C���f�[�^�擾���s�����ߊ֐���
/*
	int cnt = 0;
	TCHAR subsec[16];
	extern BOOL log_group_disp[];
	extern char defLogLvl;
*/
	// Windows�e���|�����p�X�擾(USB���O�𓀗p)
	GetTempPath(_T_MAX_PATH, szWindowsTempPath);



	// USB���O�t�@�C�����̃G���W�����O�t�@�C�������擾
	GetPrivateProfileString(_T("USBLOG_SETTING"), _T("ENGN_LOG_FILE1"),
		_T(USBLOG_DEFAULT_ENGINE_FILE), engnFileNameEntry1, sizeof(engnFileNameEntry1), szIniFilePublic);
	GetPrivateProfileString(_T("USBLOG_SETTING"), _T("ENGN_LOG_FILE2"),
		_T(USBLOG_DEFAULT_ENGINE_FILE), engnFileNameEntry2, sizeof(engnFileNameEntry2), szIniFilePublic);

	//---------------------------------------------------------
	//�v���W�F�N�g�U�֐ݒ�INI�t�@�C���Ǎ�
	//---------------------------------------------------------
	char	szSettingIniFile[_T_MAX_PATH];			//LogTool�ݒ�pINI�t�@�C��
	char	szKey[16];								//INI�t�@�C���Ǎ���Key�w��p
	char	szBuff[100];							//INI�t�@�C���擾���ʃo�b�t�@�p
	int		i = 0;
	int		j;

	char	*delim = ",\n";			//�f���~�^�i�����n����j�����ł̓J���}�Ɖ��s�R�[�h
	char	*ctx;					//�����I�Ɏg�p����̂Ő[���l���Ȃ� 
	char	*next;

	//�U�փv���W�F�N�g�ޔ�̈揉����
	for (i = 0; i < TRANFER_PJ_MAX; i++) {
		for (j = 0; j < 10; j++) {
			TransferPJ[i][j][0] = '\0';
		}
	}

	TransPJ_cnt = 0;								//�U�փv���W�F�N�g����������
	sprintf_s(szSettingIniFile, "%s%sLogToolSetting.ini", szGlDrive, szGlPath);
	for(i = 0; i < TRANFER_PJ_MAX; i++) {
		sprintf_s(szKey, _T("TRANS_PJ%02d"), i);
		//INI�t�@�C���f�[�^�擾(�Z�N�V����=[TRANSFER_PJ],�L�[��=TRANS_PJXX,�f�t�H���g�l="***")
		GetPrivateProfileString(_T("TRANSFER_PJ"), szKey, _T("***"), szBuff, sizeof(szBuff), szSettingIniFile);
		if(!memcmp(szBuff, "***", 3)) break;		//INI�t�@�C�����f�[�^�擾�ł��Ȃ������ꍇ�AFor���I��

		//�擾���ʂ��J���}��������
		j = 0;
		next = strtok_s(szBuff, delim, &ctx); 
		while(next){ 
			strcpy_s(TransferPJ[i][j], next);
			TransferPJ[i][j][3] = '\0';
			next = strtok_s(null_ptr, delim, &ctx); 
			j++;
		} 
		TransPJ_cnt++;								//�U�փv���W�F�N�g����
	}

//��������������O�t�@�C���f�[�^�擾���s�����ߏ������֐���
	//�ݒ�l�擾
	if (getSettingData() == FALSE) {
		MessageBox(NULL, TEXT("���s�����m�F���Ă��������B"), TEXT("���s���ُ�"), (MB_OK | MB_ICONERROR));
		return FALSE;
	}

	// �E�B���h�E�̍쐬�ʒu���v�Z����
	RECT g_windowPos;	// �E�B���h�E�̈ʒu
	g_windowPos.left = GetSystemMetrics( SM_CXSCREEN ) + 100;
	g_windowPos.top = GetSystemMetrics( SM_CYSCREEN ) + 100;
	g_windowPos.right = g_windowPos.left + WINDOW_WIDTH;
	g_windowPos.bottom = g_windowPos.top + WINDOW_HEIGHT;

	hWnd = CreateWindow(
				TEXT("SyslogTool"),			// �E�B���h�E�N���X��
				TEXT("SyslogTool"),				// �^�C�g���o�[�ɕ\�����镶����
				WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,	// �E�B���h�E�̎��
				g_windowPos.left,		// �E�B���h�E��\������ʒu�iX���W�j
				g_windowPos.top,		// �E�B���h�E��\������ʒu�iY���W�j
				WINDOW_WIDTH,			// �E�B���h�E�̕�
				WINDOW_HEIGHT,			// �E�B���h�E�̍���
				NULL,					// �e�E�B���h�E�̃E�B���h�E�n���h��
				NULL,					// ���j���[�n���h��
				hInstance,				// �C���X�^���X�n���h��
				NULL					// ���̑��̍쐬�f�[�^
			);

	if (!hWnd) {
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	hBaseWindow = hWnd;

	// �N����Ԃ�SYSLOG�_�C�A���O���o��
	SendMessage(hWnd, WM_COMMAND, IDM_DISP_SYSLOG, 0);
		
	//��ʕ\����ɁA�R�}���h���C�������t�@�C��������ꍇ�A�e�폈�����s
	if (strcmp(szCmdLineFileName, "")) {
		//�t�@�C���p�X�𕪊�
		splitpath(szCmdLineFileName);
		::_tcscpy_s(szDrive, _T_MAX_DRIVE * sizeof(_TCHAR), szGlDrive);
		::_tcscpy_s(szDir, _T_MAX_DIR * sizeof(_TCHAR), szGlPath);
		::_tcscpy_s(szTemp, _T_MAX_FNAME * sizeof(_TCHAR), szGlFName);

		if (!strcmp(szGlExt, ".gz")) {
			//�𓀏���
			if (!USBFileDecomp(szCmdLineFileName)) return TRUE;			//�𓀏���
			//�𓀌�̃t�@�C����Drop���ꂽ�t�@�C���Ƃ��Đݒ�
			memset(szCmdLineFileName, 0, sizeof(szCmdLineFileName));	//�R�}���h���C�������t�@�C�����ޔ�̈揉����
			strcpy_s(szCmdLineFileName, szDrive);
			strcat_s(szCmdLineFileName, szDir);
			strcat_s(szCmdLineFileName, engnFileNameEntry2);

			if(!PathFileExists(szCmdLineFileName)) {
				//�𓀌�̃t�@�C����Drop���ꂽ�t�@�C���Ƃ��Đݒ�
				memset(szCmdLineFileName, 0, sizeof(szCmdLineFileName));	//�R�}���h���C�������t�@�C�����ޔ�̈揉����
				strcpy_s(szCmdLineFileName, szDrive);
				strcat_s(szCmdLineFileName, szDir);
				strcat_s(szCmdLineFileName, engnFileNameEntry1);
			}
		} else {
			strcpy_s(szCmdLineFileName, szCmdLineFileName);				//�R�}���h���C��������ޔ�
		}
		
		//dev_proc_edbg_all.log��EngSoft�̃o�[�W�����ɑΉ�����INI�t�@�C�����ݒ�l�擾
		char SoftVer[SOFTVER_BUFFER_SZ];
		if (!getLogSoftVer(szCmdLineFileName, SoftVer)) return (INT_PTR)TRUE;	//dev_proc_edbg_all.log���\�t�g�o�[�W�����擾
		//�\�t�g�o�[�W�������擾�ł����ꍇ�A�擾�o�[�W�����ɉ������e��ݒ�t�@�C�����Ď擾
		if (strcmp(SoftVer, "")) {
			getSoftVerSettingData(SoftVer);										//�\�t�g�o�[�W�����Ή����擾����
		}

		//*.log��*.tcd�ϊ�����
		char TargetFileName[_T_MAX_PATH];
		//*.tcd�̃t�@�C�����̓h���b�v���ꂽ�t�@�C���̊g���q���O�̕������g�p
		char *FileName;
		char* p;
		FileName = strtok_s(szTemp, "." , &p);
		strcpy_s(TargetFileName, FileName);

		//*.tcd�t�@�C��������
		char targetFile[2048] = "";
		strcpy_s(targetFile, szDrive);
		strcat_s(targetFile, szDir);
		strcat_s(targetFile, TargetFileName);
		strcat_s(targetFile, ".tcd");
		if (!USBLogConvert(hSyslogDlgWnd, szCmdLineFileName, targetFile)) return TRUE;	//*.log��*.tcd�ϊ�����

		//���O�\���ۑ�����
		//���O�\���ۑ��p�t�@�C��������
		memset(targetFile, 0, sizeof(targetFile));
		strcpy_s(targetFile, szDrive);
		strcat_s(targetFile, szDir);
		strcat_s(targetFile, TargetFileName);
		strcat_s(targetFile, ".log");
		if (!LogSave(hSyslogDlgWnd, targetFile)) return (INT_PTR)TRUE;			//���O�\�����e�t�@�C���ۑ�����

		DestroyWindow(hSyslogDlgWnd);		//�_�C�A���OWindow�j��
		DestroyWindow(hWnd);				//MainWindow�j��
	}

	return TRUE;
}

//==================================================
//	SaveSyslogToFile()
//--------------------------------------------------
//	<����>
//	hDlg	: LogTool�_�C�A���O�ւ̃n���h��
//--------------------------------------------------
//	<�����T�v>
//	
//==================================================
INT_PTR SaveSyslogToFile(HWND hDlg)
{

	// �\���̂ɏ����Z�b�g
	ZeroMemory(&ofn, sizeof(ofn));			// �ŏ��Ƀ[���N���A���Ă���
	ofn.lStructSize = sizeof(ofn);			// �\���̂̃T�C�Y
	ofn.hwndOwner = hDlg;                   // �R�����_�C�A���O�̐e�E�B���h�E�n���h��
	ofn.lpstrFilter = _T("���O�t�@�C��(*.log)\0*.log\0All files(*.*)\0*.*\0\0"); // �t�@�C���̎��
	ofn.lpstrFile = filename_full;          // �I�����ꂽ�t�@�C����(�t���p�X)���󂯎��ϐ��̃A�h���X
	ofn.lpstrFileTitle = filename;          // �I�����ꂽ�t�@�C�������󂯎��ϐ��̃A�h���X
	ofn.nMaxFile = sizeof(filename_full);   // lpstrFile�Ɏw�肵���ϐ��̃T�C�Y
	ofn.nMaxFileTitle = sizeof(filename);   // lpstrFileTitle�Ɏw�肵���ϐ��̃T�C�Y
	ofn.Flags = OFN_OVERWRITEPROMPT;        // �t���O�w��
	ofn.lpstrTitle = _T("�ۑ��t�@�C����ݒ�");
	ofn.lpstrDefExt = _T("log");            // �f�t�H���g�̃t�@�C���̎��

	// ���O��t���ĕۑ��R�����_�C�A���O���쐬
	if(!GetOpenFileName(&ofn)) return (INT_PTR)TRUE;
	if(PathFileExists(filename_full)) {
		if (MessageBox(NULL, TEXT("�w��t�@�C�������݂��Ă��܂��B�㏑�����܂����H\n"), TEXT("�m�F"), MB_YESNO | MB_ICONQUESTION) == IDNO) {
			return (INT_PTR)TRUE;
		}
		if (GetFileAttributes(filename_full) & FILE_ATTRIBUTE_READONLY) {
			if (MessageBox(NULL, TEXT("�w��t�@�C����ReadOnly�t�@�C���ł��B�㏑�����܂����H\n"), TEXT("�m�F"), MB_YESNO | MB_ICONQUESTION) == IDNO) {
				return (INT_PTR)TRUE;
			}
			SetFileAttributes(filename_full, FILE_ATTRIBUTE_NORMAL);
		}
	}
	if (!LogSave(hDlg, filename_full)) return (INT_PTR)TRUE;		//���O�\�����e�t�@�C���ۑ�����

	return (INT_PTR)TRUE;
}

INT_PTR OpenSyslogTextApp (HWND hDlg)
{
	char filePathBuf[1024];	// �t�@�C���p�X(�t�@�C�����܂�)�p ������o�b�t�@

	sprintf_s(filePathBuf, sizeof(filePathBuf), "%s%s", szGlPath, "temp_log.log");		// �t�@�C���p�X����
	INT_PTR saveResult = LogSave(hDlg, filePathBuf);
	if (saveResult == TRUE) {
		// �e�L�X�g�G�f�B�^�ŊJ��
		HINSTANCE instance = ShellExecute(NULL, "open", filePathBuf, NULL, NULL, SW_SHOWNORMAL);
	} else {
		// �t�@�C�����J���Ȃ������ꍇ�A�������Ȃ�
	}
	return (INT_PTR)TRUE;
}

//==================================================
//	ascii2bin()
//--------------------------------------------------
//	<����>
//	src		: �A�X�L�[�R�[�h�o�b�t�@�ւ̃|�C���^
//	dst		: �o�C�i���ϊ���̊i�[�o�b�t�@�ւ̃|�C���^
//--------------------------------------------------
//	<�����T�v>
//	�A�X�L�[�R�[�h���o�C�i���ϊ�����
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
//	<����>
//	preTimeCount	:
//	timeCount		:
//--------------------------------------------------
//	<�����T�v>
//	
//==================================================
unsigned long getTcmDiffTime (unsigned long *preTimeCount, unsigned long timeCount)
{
	unsigned long diffTimeCount = 0;           // ���ԍ�

    // �������ԎZ�o
	if (timeCount < *preTimeCount) {
		diffTimeCount = (0xffffffff - (*preTimeCount - timeCount));
	} else {
		diffTimeCount = (timeCount - *preTimeCount);
	}

    // �O�񎞊ԍX�V
	*preTimeCount = timeCount;

	return diffTimeCount;
}

//==================================================
//	LoadUSBLogFile()
//--------------------------------------------------
//	<����>
//	hDlg	: LogTool�_�C�A���O�ւ̃n���h��
//--------------------------------------------------
//	<�����T�v>
//	
//==================================================
INT_PTR LoadUSBLogFile(HWND hDlg)
{

	static TCHAR tcm_filename_full[_T_MAX_PATH];   // �t�@�C����(�t���p�X)���󂯎��̈�

	//�����ŕϐ������������\���̐ݒ�͋󕶎��̐ݒ�����Ȃ��ƁA
	//�t�@�C���I���_�C�A���O���\������Ȃ�
	memset(filename_full, 0, sizeof(filename_full));	//�t�@�C�����t���p�X�ޔ�̈揉����
	memset(filename, 0, sizeof(filename));				//�t�@�C�����ޔ�̈揉����

	HWND lbox = GetDlgItem(hDlg, IDC_SYSLOG);

	//---------------------------------------------------------
	// �\���̂ɏ����Z�b�g(�Ǎ��p)
	ZeroMemory(&ofn, sizeof(ofn));			// �ŏ��Ƀ[���N���A���Ă���
	ofn.lStructSize = sizeof(ofn);			// �\���̂̃T�C�Y
	ofn.hwndOwner = hDlg;                   // �R�����_�C�A���O�̐e�E�B���h�E�n���h��
	ofn.lpstrFilter = _T("USB���O���k�t�@�C��(*.gz)\0*.gz\0�G���W���֌W��USB���O�t�@�C��\0*edbg*.log\0All files(*.*)\0*.*\0\0"); // �t�@�C���̎��
	ofn.lpstrFile = filename_full;          // �I�����ꂽ�t�@�C����(�t���p�X)���󂯎��ϐ��̃A�h���X
	ofn.lpstrFileTitle = filename;          // �I�����ꂽ�t�@�C�������󂯎��ϐ��̃A�h���X
	ofn.nMaxFile = sizeof(filename_full);   // lpstrFile�Ɏw�肵���ϐ��̃T�C�Y
	ofn.nMaxFileTitle = sizeof(filename);   // lpstrFileTitle�Ɏw�肵���ϐ��̃T�C�Y
	ofn.Flags = OFN_OVERWRITEPROMPT;        // �t���O�w��
	ofn.lpstrTitle = _T("USB���O�t�@�C�����w��");
	ofn.lpstrDefExt = _T("gz");            // �f�t�H���g�̃t�@�C���̎��

	// ���O��t���ĕۑ��R�����_�C�A���O���쐬(�Ǎ��p)
	if(!GetOpenFileName(&ofn)) return (INT_PTR)TRUE;

	//�I���t�@�C���p�X���t�H���_�𒊏o
	char targetDir[2048] = "";
//	char targetFile[] = "\\log\\files\\debug_file\\dev_proc_edbg_all.log";
	splitpath(filename_full);

	strcpy_s(targetDir, szGlDrive);
	strcat_s(targetDir, szGlPath);

	//---------------------------------------------------------
	// �\���̂ɏ����Z�b�g(�����p)
	ZeroMemory(&ofn, sizeof(ofn));			// �ŏ��Ƀ[���N���A���Ă���
	ofn.lStructSize = sizeof(ofn);			// �\���̂̃T�C�Y
	ofn.hwndOwner = hDlg;                   // �R�����_�C�A���O�̐e�E�B���h�E�n���h��
	ofn.lpstrFilter = _T("TCD�f�[�^�t�@�C��(*.tcd)\0*.tcd\0All files(*.*)\0*.*\0\0"); // �t�@�C���̎��
	ofn.lpstrFile = tcm_filename_full;      // �I�����ꂽ�t�@�C����(�t���p�X)���󂯎��ϐ��̃A�h���X
	ofn.lpstrFileTitle = filename;          // �I�����ꂽ�t�@�C�������󂯎��ϐ��̃A�h���X
	ofn.nMaxFile = sizeof(tcm_filename_full); // lpstrFile�Ɏw�肵���ϐ��̃T�C�Y
	ofn.nMaxFileTitle = sizeof(filename);   // lpstrFileTitle�Ɏw�肵���ϐ��̃T�C�Y
	ofn.Flags = OFN_OVERWRITEPROMPT;        // �t���O�w��
	ofn.lpstrTitle = _T("�ۑ��t�@�C����ݒ�");
	ofn.lpstrDefExt = _T("tcd");            // �f�t�H���g�̃t�@�C���̎��
	ofn.lpstrInitialDir = targetDir;		//�I�����ꂽ�ϊ����t�@�C���Ɠ���t�H���_�������\���ݒ�

	// ���O��t���ĕۑ��R�����_�C�A���O���쐬(�����p)
	if(!GetOpenFileName(&ofn)) return (INT_PTR)TRUE;
	
	//---------------------------------------------------------
	//���k�t�@�C��(*.gz)��I�������ꍇ�A�ȉ��̏��������{
	//�@�E�t�@�C����
	//	�E�𓀂��Đ������ꂽ"dev_proc_edbg_all.log"��I���t�@�C���Ƃ��Ĉ���
	if (!strcmp(szGlExt, ".gz")) {
		if (!USBFileDecomp(filename_full)) return (INT_PTR)TRUE;	//�𓀏���(�G���[�������I��)
		//�𓀌�̃R�s�[���ꂽLog�t�@�C����I���t�@�C���Ƃ��Đݒ�
		memset(filename_full, 0, sizeof(filename_full));
		strcpy_s(filename_full, targetDir);
		strcat_s(filename_full, engnFileNameEntry2);
		if(!PathFileExists(filename_full)) {
			//�𓀌�̃R�s�[���ꂽLog�t�@�C����I���t�@�C���Ƃ��Đݒ�
			memset(filename_full, 0, sizeof(filename_full));
			strcpy_s(filename_full, targetDir);
			strcat_s(filename_full, engnFileNameEntry1);
		}
	}
	//*.log��*.tcd�ϊ�����
	if (!USBLogConvert(hDlg, filename_full, tcm_filename_full)) return (INT_PTR)TRUE;

	//---------------------------------------------------------
	//�����I��
	return (INT_PTR)TRUE;

}

//==================================================
//	SaveLogBinFile()
//--------------------------------------------------
//	<����>
//	hDlg	: LogTool�_�C�A���O�ւ̃n���h��
//--------------------------------------------------
//	<�����T�v>
//	
//==================================================
INT_PTR SaveLogBinFile(HWND hDlg)
{
	// �\���̂ɏ����Z�b�g
	ZeroMemory(&ofn, sizeof(ofn));			// �ŏ��Ƀ[���N���A���Ă���
	ofn.lStructSize = sizeof(ofn);			// �\���̂̃T�C�Y
	ofn.hwndOwner = hDlg;                   // �R�����_�C�A���O�̐e�E�B���h�E�n���h��
	ofn.lpstrFilter = _T("SYSLOG�o�C�i���t�@�C��(*.bin)\0*.bin\0All files(*.*)\0*.*\0\0"); // �t�@�C���̎��
	ofn.lpstrFile = filename_full;          // �I�����ꂽ�t�@�C����(�t���p�X)���󂯎��ϐ��̃A�h���X
	ofn.lpstrFileTitle = filename;          // �I�����ꂽ�t�@�C�������󂯎��ϐ��̃A�h���X
	ofn.nMaxFile = sizeof(filename_full);   // lpstrFile�Ɏw�肵���ϐ��̃T�C�Y
	ofn.nMaxFileTitle = sizeof(filename);   // lpstrFileTitle�Ɏw�肵���ϐ��̃T�C�Y
	ofn.Flags = OFN_OVERWRITEPROMPT;        // �t���O�w��
	ofn.lpstrTitle = _T("�ۑ��t�@�C����ݒ�");
	ofn.lpstrDefExt = _T("bin");            // �f�t�H���g�̃t�@�C���̎��

	// ���O��t���ĕۑ��R�����_�C�A���O���쐬
	if(!GetOpenFileName(&ofn)) return (INT_PTR)TRUE;

	if(PathFileExists(filename_full)) {
		DeleteFile(filename_full);
	}
	logFileClose();

	// SYSLOG�t�@�C�����R�s�[
	CopyFile(syslog_file, filename_full, FALSE);

	logFileReOpen();
	return (INT_PTR)TRUE;
}

//==================================================
//	SaveTcmlogToFile()
//--------------------------------------------------
//	<����>
//	hDlg	: LogTool�_�C�A���O�ւ̃n���h��
//--------------------------------------------------
//	<�����T�v>
//	
//==================================================
INT_PTR SaveTcmlogToFile(HWND hDlg)
{
	// �\���̂ɏ����Z�b�g
	ZeroMemory(&ofn, sizeof(ofn));			// �ŏ��Ƀ[���N���A���Ă���
	ofn.lStructSize = sizeof(ofn);			// �\���̂̃T�C�Y
	ofn.hwndOwner = hDlg;                   // �R�����_�C�A���O�̐e�E�B���h�E�n���h��
	ofn.lpstrFilter = _T("TCD�f�[�^�t�@�C��(*.tcd)\0*.tcd\0All files(*.*)\0*.*\0\0"); // �t�@�C���̎��
	ofn.lpstrFile = filename_full;          // �I�����ꂽ�t�@�C����(�t���p�X)���󂯎��ϐ��̃A�h���X
	ofn.lpstrFileTitle = filename;          // �I�����ꂽ�t�@�C�������󂯎��ϐ��̃A�h���X
	ofn.nMaxFile = sizeof(filename_full);   // lpstrFile�Ɏw�肵���ϐ��̃T�C�Y
	ofn.nMaxFileTitle = sizeof(filename);   // lpstrFileTitle�Ɏw�肵���ϐ��̃T�C�Y
	ofn.Flags = OFN_OVERWRITEPROMPT;        // �t���O�w��
	ofn.lpstrTitle = _T("�ۑ��t�@�C����ݒ�");
	ofn.lpstrDefExt = _T("tcd");            // �f�t�H���g�̃t�@�C���̎��

	// ���O��t���ĕۑ��R�����_�C�A���O���쐬
	if(!GetOpenFileName(&ofn)) return (INT_PTR)TRUE;

	BOOL tcm_area = FALSE;

	if(PathFileExists(filename_full)) {
		if (MessageBox(NULL, TEXT("�w��t�@�C�������݂��Ă��܂��B�㏑�����܂����H\n"), TEXT("�m�F"), MB_YESNO | MB_ICONQUESTION) == IDNO) {
			return (INT_PTR)TRUE;
		}
		if (GetFileAttributes(filename_full) & FILE_ATTRIBUTE_READONLY) {
			if (MessageBox(NULL, TEXT("�w��t�@�C����ReadOnly�t�@�C���ł��B�㏑�����܂����H\n"), TEXT("�m�F"), MB_YESNO | MB_ICONQUESTION) == IDNO) {
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
		sprintf_s(msgBuf, "�t�@�C���I�[�v���Ɏ��s���܂���(err=%d)", result);
		MessageBox(NULL, TEXT(msgBuf), TEXT("�G���["), MB_OK | MB_ICONSTOP);
		return (INT_PTR)FALSE;
	}

	//���O�t�@�C������UClose
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
		// ���O�u���b�N�̐擪�܂œǂݔ�΂�
		if(fread(readBuf, 1, 1, rstream) < 1) {
			break;
		}

		unsigned char read4bit_chk = ((readBuf[0] & 0xf0) >> 4);

		// SYSLOG�u���b�N�Ȃ�1�u���b�N�ǂݔ�΂�
		if(read4bit_chk == 0x00 || read4bit_chk == 0x01) {
			// CRLF�`�F�b�N
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

			char err_lvl = (readBuf[0] & 0x78) >> 3;	// �G���[���x��
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

			char group_id = (readBuf[4] & 0xfc) >> 2;	// �O���[�vID
			if((group_id < 0) || (group_id > log_group_num)) {
				continue;
			}

			if(fread(&readBuf[5], 1, 2, rstream) < 2) {
				nowRcved = TRUE;
				break;
			}

			short log_no = ((readBuf[4] & 0x03) << 8) + readBuf[5];	// ���ONo
			char arg_type = (readBuf[6] & 0xf0) >> 4;	// �����^�C�v
			if((arg_type < 0) || (arg_type > 7)) {
				continue;
			}

			char arg_num = (readBuf[6] & 0x0f);			// �����̐�
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
		// TCM�u���b�N�łȂ���Γǂݔ�΂�
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

		// deviceType�擾
		unsigned char devType = (readBuf[0] >> 3) & 0x0f;
		// �^�C���X�^���v�擾
		unsigned long timeStamp = (((readBuf[0] & 0x07) << 24)
									+ (readBuf[1] << 16)
									+ (readBuf[2] <<  8)
									+ readBuf[3]);
		// �������Ԏ擾
		unsigned long diffTm = getTcmDiffTime (&tcm_preTime, timeStamp);
		// �f�[�^�擾
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
		// �V�i���I�̓p�����[�^�����id�ɂ���
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

		// ���X�g�ɓo�^
		char deviceId[9];
		sprintf_s(deviceId, 9, "%03x%04x%d", param, id, io);

		// onoff��3�r�b�g�������i�[�ł��Ȃ��̂�
		// ���̂��ׂĂ�1�Ȃ�-1�ƂȂ�B
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

	//���O�t�@�C�����ēxOpen
	logFileReOpen();
	return (INT_PTR)TRUE;
}

INT_PTR GetSettingFileName(HWND hDlg, int kind)
{
	// �\���̂ɏ����Z�b�g
	ZeroMemory(&ofn, sizeof(ofn));			// �ŏ��Ƀ[���N���A���Ă���
	ofn.lStructSize = sizeof(ofn);			// �\���̂̃T�C�Y
	ofn.hwndOwner = hDlg;                   // �R�����_�C�A���O�̐e�E�B���h�E�n���h��
	// �t�@�C���̎��
	if(kind == 0) {
		ofn.lpstrFilter = _T("���b�Z�[�W�t�@�C��(*.mes)\0*.mes\0All files(*.*)\0*.*\0\0");
		ofn.lpstrDefExt = _T("mes");           // �f�t�H���g�̃t�@�C���̎��
	}
	else {
		ofn.lpstrFilter = _T("SYSLOG�t�@�C��(*.bin)\0*.bin\0All files(*.*)\0*.*\0\0");
		ofn.lpstrDefExt = _T("bin");           // �f�t�H���g�̃t�@�C���̎��
	}
	ofn.lpstrFile = filename_full;         // �I�����ꂽ�t�@�C����(�t���p�X)���󂯎��ϐ��̃A�h���X
	ofn.lpstrFileTitle = filename;         // �I�����ꂽ�t�@�C�������󂯎��ϐ��̃A�h���X
	ofn.nMaxFile = sizeof(filename_full);  // lpstrFile�Ɏw�肵���ϐ��̃T�C�Y
	ofn.nMaxFileTitle = sizeof(filename);  // lpstrFileTitle�Ɏw�肵���ϐ��̃T�C�Y
	ofn.Flags = OFN_FILEMUSTEXIST;         // �t���O�w��
	ofn.lpstrTitle = _T("�t�@�C����I��"); // �R�����_�C�A���O�̃L���v�V����

	// ���O��t���ĕۑ��R�����_�C�A���O���쐬
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
//	�t�H���_�폜�֐�(Shell�g�pVersion)
//--------------------------------------------------
//	<����>
//	lpPathName	:�폜�Ώۃt�H���_�p�X
//--------------------------------------------------
//	<�����T�v>
//	�w�肳�ꂽ�t�H���_�ȉ��S�폜���s
//==================================================
BOOL DeleteDirectory( LPCTSTR lpPathName )
{
	// ���͒l�`�F�b�N
	if( NULL == lpPathName )	return FALSE;

	// �f�B���N�g�����̕ۑ��i�I�[��'\'���Ȃ��Ȃ�t����j
	TCHAR szDirectoryPathName[_T_MAX_PATH];
	_tcsncpy_s( szDirectoryPathName, _T_MAX_PATH, lpPathName, _TRUNCATE );
	if( '\\' != szDirectoryPathName[_tcslen(szDirectoryPathName) - 1] )
	{	// ��ԍŌ��'\'���Ȃ��Ȃ�t������B
		_tcsncat_s( szDirectoryPathName, _T_MAX_PATH, _T("\\"), _TRUNCATE );
	}

	// �f�B���N�g�����̃t�@�C�������p�̃t�@�C�����쐬
	TCHAR szFindFilePathName[_T_MAX_PATH];
	_tcsncpy_s( szFindFilePathName, _T_MAX_PATH, szDirectoryPathName, _TRUNCATE );
	_tcsncat_s( szFindFilePathName, _T_MAX_PATH, _T("*"), _TRUNCATE );

	// �f�B���N�g�����̃t�@�C�������J�n
	WIN32_FIND_DATA		fd;
	HANDLE hFind = FindFirstFile( szFindFilePathName, &fd );
	if( INVALID_HANDLE_VALUE == hFind )
	{	// �����Ώۃt�H���_�����݂��Ȃ��B
		return FALSE;
	}

	do
	{
		//if( '.' != fd.cFileName[0] )
		if( 0 != _tcscmp( fd.cFileName, _T(".") )		// �J�����g�t�H���_�u.�v��
		 && 0 != _tcscmp( fd.cFileName, _T("..") ) )	// �e�t�H���_�u..�v�́A�������X�L�b�v
		{
			TCHAR szFoundFilePathName[_T_MAX_PATH];
			_tcsncpy_s( szFoundFilePathName, _T_MAX_PATH, szDirectoryPathName, _TRUNCATE );
			_tcsncat_s( szFoundFilePathName, _T_MAX_PATH, fd.cFileName, _TRUNCATE );

			if( FILE_ATTRIBUTE_DIRECTORY & fd.dwFileAttributes )
			{	// �f�B���N�g���Ȃ�ċN�Ăяo���ō폜
				if( !DeleteDirectory( szFoundFilePathName ) )
				{
					FindClose( hFind );
					return FALSE;
				}
			}
			else
			{	// �t�@�C���Ȃ�Win32API�֐���p���č폜
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
//	USB Log���k�t�@�C���𓀏���
//--------------------------------------------------
//	<����>
//	filename_full		:�ϊ��Ώۃt�@�C��(*.gz)
//--------------------------------------------------
//	<�����T�v>
//	�w��t�@�C���̉𓀏������s���A�w��Log�t�@�C�������k�t�@�C����
//	����t�H���_�փR�s�[��A�𓀃f�[�^���폜����
//==================================================
INT_PTR USBFileDecomp(char *filename_full)
{
	char LogFile[_T_MAX_PATH] = "";					//���O�t�@�C����(�t���p�X)�ޔ�p
	char FileBuff[_T_MAX_PATH] = "";				//�t�@�C����(�t���p�X)�ޔ�p

	splitpath(filename_full);

	//���k�t�@�C���̃t�@�C�����ޔ�(�g���q�܂�)
	memset(filename, 0, sizeof(filename));		//�t�@�C�����ޔ�̈�(�O���[�o���ϐ�)������
	strcpy_s(filename, szGlFName);
	strcat_s(filename, szGlExt);

	//�������n�����߂ɕK�v�Ȍ^��錾
	HINSTANCE lib;								// DLL�̃n���h��
	int i = 0,ret = 0,ret2 = 0;					// �R�}���h���C���̌����ɗ��p
	char cmd[2048] = "",outstr[1000] = "";		// ���o�̓o�b�t�@�̐錾
	WORD getv = 0;								// �o�[�W�����擾���i�[

	// ���I�����N(DLL���Ȃ��Ă�����ł���A�v���P�[�V����)
	// �v���O�����ŗ��p����API�֐���typedef�Ő錾���܂��B
	// API�֐��̓���ɂ��Ă̓A�[�J�C�oDLL�̃e�L�X�g���Q�Ƃ��Ă��������B
	// �����ł̓o�[�W�����擾�Ə��ɑ���݂̂𗘗p���܂��B
	// �������͌^�݂̂ł��܂��܂���B
	typedef WORD(WINAPI *TARGETVERSION)(void);
	typedef int(WINAPI *TAR)(const HWND, LPCSTR, LPSTR , const DWORD);
	
	lib = LoadLibrary("TAR32.dll");			// LoadLibrary��DLL���J��

	// DLL�����������[�h���ꂽ���m�F
	if (lib == NULL) {
		//�t�@�C����������Ȃ��Ȃǂ�DLL���J���̂Ɏ��s�����ꍇ�A�G���[�\���������I��
		MessageBox(NULL, TEXT("ERROR:TAR32.DLL not Found on your system.\n"), TEXT("�G���["), MB_OK | MB_ICONSTOP);
		return (INT_PTR)FALSE;
	} else {
		// ���������ꍇ�AAPI�֐��ւ̃A�h���X���AGetProcAddress�ŃL���X�g
		// ������₷���悤�Ɋ֐��̖��O��ϐ��Ő錾
		TARGETVERSION TarGetVersion = (TARGETVERSION)GetProcAddress(lib,"TarGetVersion");
		TAR Tar = (TAR)GetProcAddress(lib,"Tar");
		
		//tar�R�}���h�̕ҏW
		//�t�@�C���p�X�ɋ󔒂��܂܂��ƁA�R�}���h���퓮�삵�Ȃ��ׁA
		//�t�@�C���p�X�̓_�u���N�H�e�[�V�����ň͂�

//1)�S�𓀁˃t�@�C���w��𓀂��s���悤�C��
//			�t�@�C���w��̉𓀗�)tar -xvzf log.tar.gz log/files/debug_file/dev_proc_edbg_all.log
/*
		strcat_s(cmd ,"-xvf ");			//tar32�𓀃R�}���h
		strcat_s(cmd ,"\"");			//���k�t�@�C���w���["]�ň͂�
		strcat_s(cmd ,filename_full);	//���k�t�@�C���w��
		strcat_s(cmd ,"\"");			//���k�t�@�C���w���["]�ň͂�
		strcat_s(cmd ," -o ");			//�W�J��̎w��I�v�V����
		strcat_s(cmd ,"\"");			//���k�t�@�C���w���["]�ň͂�
		strcat_s(cmd ,targetDir);		//�W�J��t�H���_(�𓀃t�@�C���͈��k�t�@�C���Ɠ���t�H���_�ɕۑ�)
		strcat_s(cmd ,"\"");			//���k�t�@�C���w���["]�ň͂�
*/
		memset(cmd, 0, sizeof(cmd));
		strcat_s(cmd ,"-xvf ");			//tar32�𓀃R�}���h
		strcat_s(cmd ,"\"");			//���k�t�@�C���w���["]�ň͂�
		strcat_s(cmd ,filename_full);	//���k�t�@�C���w��
		strcat_s(cmd ,"\"");			//���k�t�@�C���w���["]�ň͂�
		strcat_s(cmd ," -o ");			//�W�J��̎w��I�v�V����
		strcat_s(cmd ,"\"");			//���k�t�@�C���w���["]�ň͂�
		strcat_s(cmd ,szWindowsTempPath);	//�W�J��t�H���_(�𓀃t�@�C����Windows�e���|�����t�H���_�ɕۑ�)
		strcat_s(cmd ,"\"");			//���k�t�@�C���w���["]�ň͂�
		strcat_s(cmd ," log\\files\\debug_file\\");
		strcat_s(cmd ,engnFileNameEntry1);		//�t�@�C���w��

		ret = Tar(NULL, cmd, outstr ,1000);		// tar�R�}���h���s

		memset(cmd, 0, sizeof(cmd));
		strcat_s(cmd ,"-xvf ");			//tar32�𓀃R�}���h
		strcat_s(cmd ,"\"");			//���k�t�@�C���w���["]�ň͂�
		strcat_s(cmd ,filename_full);	//���k�t�@�C���w��
		strcat_s(cmd ,"\"");			//���k�t�@�C���w���["]�ň͂�
		strcat_s(cmd ," -o ");			//�W�J��̎w��I�v�V����
		strcat_s(cmd ,"\"");			//���k�t�@�C���w���["]�ň͂�
		strcat_s(cmd ,szWindowsTempPath);	//�W�J��t�H���_(�𓀃t�@�C����Windows�e���|�����t�H���_�ɕۑ�)
		strcat_s(cmd ,"\"");			//���k�t�@�C���w���["]�ň͂�
		strcat_s(cmd ," log\\files\\debug_file\\");
		strcat_s(cmd ,engnFileNameEntry2);		//�t�@�C���w��

		ret2 = Tar(NULL, cmd, outstr ,1000);		// tar�R�}���h���s

		FreeLibrary(lib);						// ���[�h����DLL���J��
		if (ret && ret2) {
			MessageBox(NULL, outstr, TEXT("ERROR�FTAR32"), MB_OK | MB_ICONSTOP);
			return (INT_PTR)FALSE;
		}
	}

	//�w�舳�k�t�@�C���Ɠ���t�H���_�ɉ𓀂���Ă���̂�
	//log�t�@�C���̃t���p�X�ҏW(�𓀃t�H���_�E�t�@�C�����Œ�)
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

	//�𓀃t�H���_��"dev_proc_edbg_all_tmp.log"���݂��邩�m�F
	if (!PathFileExists(LogFile)) {
		//�w�舳�k�t�@�C���Ɠ���t�H���_�ɉ𓀂���Ă���̂�
		//log�t�@�C���̃t���p�X�ҏW(�𓀃t�H���_�E�t�@�C�����Œ�)
		strcpy_s(LogFile, szWindowsTempPath);
		strcat_s(LogFile, "log\\files\\debug_file\\");
		strcat_s(LogFile, engnFileNameEntry1);

		//�𓀃t�H���_��"dev_proc_engine.log"���݂��邩�m�F
		if (!PathFileExists(LogFile)) {
			MessageBox(NULL, TEXT("ERROR:dev_proc_edbg_all.log not Found.\n"), TEXT("�G���["), MB_OK | MB_ICONSTOP);
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

	//�G���W�����O�t�@�C�������k�t�@�C���Ɠ����t�H���_�փR�s�[(���Ƀt�@�C�����݂���ꍇ�㏑��)
	CopyFile(LogFile, FileBuff, false);

	//�𓀃t�@�C�����폜
	memset(FileBuff, 0, sizeof(FileBuff));
	strcpy_s(FileBuff, szWindowsTempPath);
	strcat_s(FileBuff, "log");
	DeleteDirectory(FileBuff);

	return (INT_PTR)TRUE;

}

//==================================================
//	USB Log�J�n�I��������ݒ�̓ǂݍ���
//--------------------------------------------------
//	<����>		 �Ȃ�
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
//	USB Log�I�����f
//------------------------------------------------------------
//	<����>
//	checkStr:�t�@�C������ǂݍ��񂾃f�[�^
// 	<�߂�l>
//	EUSBLOG_NONE:�p���AEUSBLOG_TCM_END:tcm���O�I��
//�@EUSBLOG_LINE_END:��s�I���AEUSBLOG_ALL_END:USB���O�̏I��
//============================================================
eUSBLOG_END_TYPE logEndCheck(char* checkStr)
{
	eUSBLOG_END_TYPE ret = EUSBLOG_NONE;
	unsigned char i;
	
	if ((checkStr[0] == 0x0a) && (checkStr[1] == 0x00)) {
		ret = EUSBLOG_LINE_END;
	} else if (!memcmp("[Start] ", checkStr, strlen("[Start] "))) {
		// �R���g���[���t���s�v������`�F�b�N
		ret = EUSBLOG_LINE_END;
	} else if (!memcmp("[End] ", checkStr, strlen("[End] "))) {
		// �R���g���[���t���s�v������`�F�b�N
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
//	USB Log�t�@�C���u������
//--------------------------------------------------
//	<����>
//	hDlg				:�_�C�A���O�n���h��
//	filename_full		:�ϊ��Ώۃt�@�C��(*.log)
//	tcm_filename_full	:�ϊ���t�@�C��(*.tcd)
//--------------------------------------------------
//	<�����T�v>
//	�w�肳�ꂽ���O�t�@�C��(*.log)���^�C�~���O�`���[�g(*.tcd)��
//	�t�@�C���֕ϊ�����
//==================================================
INT_PTR USBLogConvert(HWND hDlg, char *filename_full, char *tcm_filename_full)
{

	char logrec[2048];
	unsigned char binbuffer[1024];						// logrec�o�b�t�@�̔����T�C�Y
	bool logEnd = false;
	HWND lbox = GetDlgItem(hDlg, IDC_SYSLOG);

	//---------------------------------------------------------
	//�t�@�C��Open����
	FILE* stream;
	fopen_s(&stream, filename_full, "r");				//Log�ϊ��Ώۃt�@�C����Read���[�h��Open

	// TCM
	unsigned long tcm_preTime = 0L;
	std::list<TCMLOG>listTcmLog;
	unsigned long totalTm = 0;

	FILE* wstream;
	//�����p�Ɏw�肳�ꂽ�t�@�C�����̑��݃`�F�b�N
	//���݂���ꍇ�A�w��t�@�C���폜
	if(PathFileExists(tcm_filename_full)) {
		DeleteFile(tcm_filename_full);
	}

	if(fopen_s(&wstream, tcm_filename_full, "a+") != 0) {	//�����p�t�@�C����Open
		fclose(stream);
		return (INT_PTR)FALSE;
	}

	//Log�t�@�C���Ǎ��E�ϊ�����
	readUsbLogPartitionStr();

	// �G���W�����O�J�n�ʒu�܂ŃV�[�N
	while(fgets(logrec, sizeof(logrec), stream)) {
		// �G���W�����O�J�n������`�F�b�N
		char checkBuf[USBLOG_PARTITION_BUF] = {0};
		memcpy(checkBuf, usbLogStartStrBuf, strlen(usbLogStartStrBuf));
		if (strstr(logrec, checkBuf)) {
			break;
		}
	}

	// �G���W�����O�Ǎ���
	size_t lineLen = 0;
	size_t cntLineLen = 0;
	size_t log_pkt_len = 0;
	bool syslogFlg = true;
	bool isTcmEnd = false;
	eUSBLOG_END_TYPE lineCheck;

	while(fgets(logrec, sizeof(logrec), stream)) {
		// �G���W�����O�I��������`�F�b�N
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
		// �Ǎ���0byte�͎��s��
		lineLen = ((strlen(logrec) - 1) / 2);	// ���s�R�[�h�������f�[�^�̔���
		if(lineLen <= 1) {
			continue;
		}
		cntLineLen = lineLen;

		// �Ǎ��o�b�t�@�̃f�[�^���o�C�i����
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
				// ���O�r���Ńp�P�b�g���s��
				// (header�������؂�Ă���ꍇ�͎��s����������
				// �������킹�����O���擾�̂��߂̑O����)
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
				// (�������킹�����O���擾�A�����Ȃ��ꍇ�͎��s������)
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
				// �G���[���x��
				char err_lvl = (binbuffer[pos] & 0x78) >> 3;
				if ((err_lvl < 0) || (err_lvl > 3)) {
					pos++;
					cntLineLen--;
					SendMessage(lbox, LB_ADDSTRING, 0, (LPARAM)"�s����logLevel\n");
					continue;
				}
				// �^�C���X�^���v
				unsigned long tim = ((unsigned long)((binbuffer[pos] & 0x07) << 24)
							+ (unsigned long)(binbuffer[pos+1] << 16)
							+ (unsigned long)(binbuffer[pos+2] <<  8)
							+ binbuffer[pos+3]);
				// �O���[�vID
				char group_id = (binbuffer[pos+4] & 0xfc) >> 2;
				if (group_id >= log_group_num) {
					pos++;
					cntLineLen--;
					SendMessage(lbox, LB_ADDSTRING, 0, (LPARAM)"�s����LogGroup\n");
					continue;
				}
				// ���OID
				unsigned short log_no = (unsigned short)(0x00ff & binbuffer[pos+5]);
				log_no |= (unsigned short)((binbuffer[pos+4] & 0x03) << 8);
				// �����^�C�v
				char arg_type = (binbuffer[pos+6] & 0xf0) >> 4;
				// �����̐�
				char arg_num = (binbuffer[pos+6] & 0x0f);
				char* msg = get_msgtbl(group_id, log_no, arg_num);
				if(msg == NULL) {
					pos++;
					cntLineLen--;
					SendMessage(lbox, LB_ADDSTRING, 0, (LPARAM)"�s����log\n");
					continue;
				}
				// ���O�����񐶐�
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
				// ���G���g���[�̈ʒu�ݒ�
				pos += (unsigned short)log_pkt_len;
				cntLineLen -= log_pkt_len;
			} else {
				// tcm
				// ���O�r���Ńp�P�b�g���s��
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
				if (!isTcmEnd) { // allInfo�ȍ~�̃f�[�^�̓^�C�~���O�`���[�g�̉�͂��s��Ȃ��B
					// deviceType�擾
					unsigned char devType = (binbuffer[pos] >> 3) & 0x0f;
					// �^�C���X�^���v�擾
					unsigned long timeStamp = (((binbuffer[pos] & 0x07) << 24)
												+ (binbuffer[pos+1] << 16)
												+ (binbuffer[pos+2] <<  8)
												+ binbuffer[pos+3]);
					// �������Ԏ擾
					unsigned long diffTm = getTcmDiffTime (&tcm_preTime, timeStamp);
					// �f�[�^�擾
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
					// �V�i���I�̓p�����[�^�����id�ɂ���
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

					// ���X�g�ɓo�^
					char deviceId[9];
					sprintf_s(deviceId, 9, "%03x%04x%d", param, id, io);
					// onoff��3�r�b�g�������i�[�ł��Ȃ��̂�
					// ���̂��ׂĂ�1�Ȃ�-1�ƂȂ�B
					char fixedOnoff = onoff;
					if(onoff == 7) {
						fixedOnoff = -1;
					}
					TCMLOG tcmLog = {deviceId, io, fixedOnoff, val, totalTm, param};
					listTcmLog.push_back(tcmLog);
				}
				// ���G���g���[�̈ʒu�ݒ�
				pos += 8;
				cntLineLen -= 8;
			}
			// �Ǎ�������̉�͊����Ŏ��s��
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
	//�t�@�C��Close����
	fclose(wstream);
	fclose(stream);

	return (INT_PTR)TRUE;

}

//==================================================
//	Log�\���ۑ�����
//--------------------------------------------------
//	<����>
//	hDlg				:�_�C�A���O�n���h��
//	filename_full		:�ۑ��t�@�C����(*.log)
//--------------------------------------------------
//	<�����T�v>
//	���O�\�����e���w�肳�ꂽ���O�t�@�C��(*.log)�ɕۑ�����
//==================================================
INT_PTR LogSave(HWND hDlg, char *filename_full)
{
	char logrec[2048];

	static HWND lbox = GetDlgItem(hDlg, IDC_SYSLOG);
	int logcnt = SendMessage(lbox, LB_GETCOUNT, 0, 0);
	if(logcnt == LB_ERR || logcnt <= 0) {
		sprintf_s(logrec, "���O�f�[�^��0���̂��߃t�@�C���X�V���܂���B");
		MessageBox(NULL, TEXT(logrec), TEXT("�I��"), MB_OK);
		return (INT_PTR)FALSE;
	}

	FILE* stream;
	errno_t result = fopen_s(&stream, filename_full, "w+");
	if (result != 0) {
		sprintf_s(logrec, "�t�@�C���I�[�v���Ɏ��s���܂���(err=%d)", result);
		MessageBox(NULL, TEXT(logrec), TEXT("�G���["), MB_OK | MB_ICONSTOP);
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
//	"dev_proc_edbg_all.log"�\�t�g�o�[�W�����擾����
//--------------------------------------------------
//	<����>
//	fileFullPath		:"dev_proc_edbg_all.log"�̃t���p�X
//	SoftVer				:�擾�\�t�g�o�[�W�����i�[�o�b�t�@
//--------------------------------------------------
//	<�����T�v>
//	"dev_proc_edbg_all.log"��ǂݍ��݁A�o�[�W��������ENG Soft
//	�ݒ�l���擾����
//==================================================
INT_PTR getLogSoftVer(char *fileFullPath, char *SoftVer)
{

	char	ReadData[1024];		//�ǂݍ��݃f�[�^(1�s��)
	char	GetData[SOFTVER_BUFFER_SZ];		//�擾�\�t�g�o�[�W����

	char	*delim = ":\n";		//�f���~�^�i�����n����j�����ł̓J���}�Ɖ��s�R�[�h
								//���s�R�[�h����Ȃ��ƁA����������̍ŏI���ڂɉ��s�R�[�h�����Ă��đz��O�̓�����N�����܂���...�B
	char	*ctx;				//�����I�Ɏg�p����̂Ő[���l���Ȃ� 
	char	*next;
	int		i;
	int		ret;

//	memset(SoftVer, 0, sizeof(SoftVer));	// pointer�T�C�Y(4byte)�ƂȂ邽��(���o�b�t�@�T�C�Y)
	memset(SoftVer, 0, SOFTVER_BUFFER_SZ);

	//---------------------------------------------------------
	//�t�@�C��Open����
	FILE* stream;
	fopen_s(&stream, fileFullPath, "r");				//"dev_proc_edbg_all.log"��Read���[�h��Open

	//EOF�܂Ńt�@�C���Ǎ�
	while(fgets(ReadData, sizeof(ReadData), stream)) {
		//"Eng Soft"������`�F�b�N
		if(!memcmp("Eng Soft", ReadData, 8)) {
			//�擾�������":"�ŕ���
			i = 0;
			next = strtok_s(ReadData, delim, &ctx); 
		    while(next){ 
		        next = strtok_s(null_ptr, delim, &ctx); 
				i++;
				if (i == 1) {
					//�����f�[�^��2���ڂɂ��đO��̋󔒍폜���A�߂�l�Ƃ��Đݒ�
					strcpy_s(GetData, next);
					ret = Trim(GetData);				//�󔒏���
					strcpy_s(SoftVer, SOFTVER_BUFFER_SZ, GetData);	//�擾�\�t�g�o�[�W�����i�[�o�b�t�@�ݒ�
					break;	//Loop������
				}
			} 
			break;			//Loop������
		}
	}

	//---------------------------------------------------------
	//�t�@�C��Close����
	fclose(stream);

	return (INT_PTR)TRUE;

}

//==================================================
//	�\�t�g�o�[�W�����Ή����擾����
//--------------------------------------------------
//	<����>
//	SoftVer				:�擾�Ώۃ\�t�g�o�[�W����
//--------------------------------------------------
//	<�����T�v>
//	�w�肳�ꂽ�\�t�g�o�[�W�����ɑ΂���e�ݒ�l���擾����
//	�w��t�H���_�Ɋe�ݒ�l�t�@�C�������݂��Ȃ��ꍇ�A�f�t�H���g��
//	�ݒ�l�t�@�C����Ǎ�
//==================================================
INT_PTR getSoftVerSettingData(char *SoftVer)
{

	int		cnt = 0;
	char	TargetDir[_T_MAX_PATH];		//�ݒ�l�擾�Ώۃf�B���N�g��
	char	FileChk[_T_MAX_PATH];		//�t�@�C�����݊m�F�p
	DWORD	dwRet;
	char	szMyPath[_T_MAX_PATH];		//�v���Z�X�p�X

	//---------------------------------------------------------
	//�ݒ�l�擾�Ώۃf�B���N�g���ҏW
	// ���s���̃v���Z�X�̃t���p�X�����擾
	dwRet = GetModuleFileName(NULL, szMyPath, sizeof(szMyPath));
	splitpath(szMyPath);
	strcpy_s(TargetDir, szGlDrive);
	strcat_s(TargetDir, szGlPath);
	strcat_s(TargetDir, DefaltDir);

//�\�t�g�o�[�W�����Ɋւ��镔���͌�قǕҏW����悤�C��
//	strcat_s(TargetDir, SoftVer);

//	strcat_s(TargetDir, "\\");

	//---------------------------------------------------------
	//�t�H���_���݊m�F
	//���݂��Ȃ��ꍇ�A�U�փv���W�F�N�g�ɑ΂���t�H���_�����݂��邩�m�F
	//�U�փv���W�F�N�g�ɑ΂���t�H���_�����݂����ꍇ�A���̃t�H���_����
	//�ݒ�l���擾����ׁA�ݒ�l�擾�Ώۃt�H���_��u��������
	int		i = 0;
	int		j = 0;
	char	szPRJ[5];					//�\�t�g�o�[�W�����̃v���W�F�N�g�R�[�h
	char	szVer[20];					//�\�t�g�o�[�W�����̃o�[�W������
	int		intExistFlg = 0;			//�U�փv���W�F�N�g���݊m�F�t���O[0:���ݖ�,1:���ݗL]
	char	szChkPath[_T_MAX_PATH];		//���݊m�F�Ώۃp�X

	memset(szPRJ, 0, sizeof(szPRJ));	//�ޔ�̈揉����
	strncpy_s(szPRJ, SoftVer, 3);		//�\�t�g�o�[�W�����̃v���W�F�N�g�R�[�h�ޔ�
	strncpy_s(szVer, SoftVer + 3, 15);	//SoftVer�̐擪+3�̈ʒu(PRJ������)����15�������R�s�[

	//���񏈗��Ώۂ̃\�t�g�o�[�W�����ɑ΂���U�֑Ώۃf�[�^�����݂��邩�m�F
	for (i = 0; i < TransPJ_cnt; i++) {
		for (j = 0; j < 10; j++) {
			if(!memcmp(TransferPJ[i][j], szPRJ, 3)) {
				intExistFlg = 1;
				break;
			}
		}
		if (intExistFlg == 1) break;
	}
	
	//�t�H���_���݊m�F
	//�t�H���_�����������ꍇ�A�ݒ�l�擾�Ώۃt�H���_��u��������Loop�I��
	if (intExistFlg == 1) {
		for (j = 0; j < 10; j++) {
			//�m�F�t�H���_�ւ̃p�X��ҏW
			memset(szChkPath, 0, sizeof(szChkPath));
			strcpy_s(szChkPath, TargetDir);
			strcat_s(szChkPath, TransferPJ[i][j]);
			strcat_s(szChkPath, szVer);
			if (PathIsDirectory(szChkPath)) {
				memset(TargetDir, 0, sizeof(TargetDir));		//�ޔ�̈揉����
				strcpy_s(TargetDir, szChkPath);
				break;
			}
		}
	}


	//---------------------------------------------------------
	//�ݒ�l�擾(�t�@�C���p�X�֘A)
	//tool_public.ini�̃p�X�ҏW
	if (szPulicIniFileName[0] != 0x00) {	//�R�}���h���C��������INI�t�@�C���w��̏ꍇ
		//�t�@�C�����݊m�F
		sprintf_s(FileChk, "%s\\%s", TargetDir, szPulicIniFileName);
		if(PathFileExists(FileChk)) {
			strcpy_s(szIniFilePublic, FileChk);
		}
	} else {
		//�t�@�C�����݊m�F
		memset(FileChk, 0, sizeof(FileChk));		//�ޔ�̈揉����
		strcpy_s(FileChk, TargetDir);
//		strcat_s(FileChk, "tool_public.ini");
		strcat_s(FileChk, "\\tool_public.ini");
		if(PathFileExists(FileChk)) {
			strcpy_s(szIniFilePublic, FileChk);		//�t�@�C�����ݎ��A�\�t�g�o�[�W�����Ή�INI�t�@�C�����g�p
		}
	}

	//tool_private.ini�̃p�X�ҏW
	//�t�@�C�����݊m�F
	memset(FileChk, 0, sizeof(FileChk));			//�ޔ�̈揉����
	strcpy_s(FileChk, TargetDir);
//	strcat_s(FileChk, "tool_private.ini");
	strcat_s(FileChk, "\\tool_private.ini");
	if(PathFileExists(FileChk)) {
		strcpy_s(szIniFilePrivate, FileChk);		//�t�@�C�����ݎ��A�\�t�g�o�[�W�����Ή�INI�t�@�C�����g�p
	}

	//DebuggerLogMsg.mes�̃p�X�ҏW
	//�t�@�C�����݊m�F
	memset(FileChk, 0, sizeof(FileChk));			//�ޔ�̈揉����
	strcpy_s(FileChk, TargetDir);
//	strcat_s(FileChk, "DebuggerLogMsg.mes");
	strcat_s(FileChk, "\\DebuggerLogMsg.mes");
	if(PathFileExists(FileChk)) {
		//�t�@�C�����ݎ��A�\�t�g�o�[�W�����Ή�DebuggerLogMsg.mes���g�p���A���b�Z�[�W�Ǘ��e�[�u���Đ���
		sprintf_s(logmsg_file, _T_MAX_PATH, "%s\\DebuggerLogMsg.mes", TargetDir);
		remove_msgtbl();		// ���O���b�Z�[�W�Ǘ��e�[�u���폜
		create_msgtbl();		// ���O���b�Z�[�W�Ǘ��e�[�u������
	}

	getSettingData();			//�ݒ�l�Ď擾

	return (INT_PTR)TRUE;

}

//==================================================
//	������̐擪�Ɩ����ɂ���󔒂��폜����
//--------------------------------------------------
//	<����>
//	param[in] s		:�Ώە�����
//	return			:�폜�����󔒂̐���Ԃ�
//==================================================
int Trim(char *s) {

	int i;
    int count = 0;

    /* ��|�C���^��? */
    if ( s == NULL ) { /* yes */
        return -1;
    }
    
    i = strlen(s);									/* �����񒷂��擾���� */
    while ( --i >= 0 && s[i] == ' ' ) count++;		/* �������珇�ɋ󔒂łȂ��ʒu��T�� */
    s[i+1] = '\0';									/* �I�[NULL������t������ */
    i = 0;
    while ( s[i] != '\0' && s[i] == ' ' ) i++;		/* �擪���珇�ɋ󔒂łȂ��ʒu��T�� */

	strcpy_s(s, SOFTVER_BUFFER_SZ, &s[i]);	// �ő�ł�GetData[]�̃T�C�Y���܂ŁB

	return i + count;

}

//==================================================
//	�ݒ�f�[�^�擾����
//--------------------------------------------------
//	<����>	�Ȃ�
//--------------------------------------------------
//	<�����T�v>
//	TCD�쐬�p�f�[�^�A���O�ϊ��f�[�^���擾����
//==================================================
INT_PTR getSettingData()
{
	#define INI_BUFF_LEN		(100)

	int		cnt = 0;					//INI�t�@�C���擾��Loop�񐔃J�E���g�p
	TCHAR	subsec[16];					//INI�t�@�C���擾Key���̕ҏW�p
	char	szBuff[INI_BUFF_LEN];		//INI�t�@�C���擾���ʃo�b�t�@�p

	extern	BOOL log_group_disp[];		//���O�\���E��\���敪�ޔ�p�z��
	extern	char defLogLvl;				//INI�擾���ʑޔ�p[�Z�N�V����:DEFAULT_SETTING, Key:LOG_LEVEL]

	//�ޔ�̈揉����
	defLogLvl = 0;						//INI�l�擾�̃f�t�H���g�l��"0"�Ȃ̂ŏ����l��0�ݒ肵�Ă���
	log_group_num = 0;					//���O�O���[�v��������

	//---------------------------------------------------------
	//teraterm���O�t�@�C��
	//---------------------------------------------------------
	GetPrivateProfileString(_T("FILE"), _T("SYSLOGFILE"), _T(DEFAULT_SYSLOG_FILE), syslog_file, sizeof(syslog_file), szIniFilePrivate);
	char full[_T_MAX_PATH];
	_fullpath(full, syslog_file, _T_MAX_PATH);
	strcpy_s(syslog_file, full);

	//---------------------------------------------------------
	//TCD�쐬���(�O���[�o���ϐ��錾��[saveTcdFile.cpp])
	//---------------------------------------------------------
	//�N���b�N������̎���
	msPerClock = (unsigned long)GetPrivateProfileInt(_T("TCD_SETTING"), _T("MSEC_PER_CLOCK"), 2, szIniFilePublic);
	//���߂̐M���Ǝ��Ԃ������������ꍇ�ɂ��炷����
	collisionDeleyTime = (unsigned long)GetPrivateProfileInt(_T("TCD_SETTING"), _T("COLLISION_DELAY_TIME"), 1, szIniFilePublic);
	//ini�t�@�C���ɏ�����Ă��鏇�Ƀ\�[�g���邩
	sortDevice = GetPrivateProfileInt(_T("TCD_SETTING"), _T("SORT_DEVICE"), 0, szIniFilePublic);

	//---------------------------------------------------------
	// ���O�O���[�v������
	//---------------------------------------------------------
	ZeroMemory(log_group, sizeof(char*)*LOGGRP_DEFNUM);
	for(cnt = 0; cnt < LOGGRP_DEFNUM; cnt++) {
		sprintf_s(subsec, _T("LG%02d"), cnt);
		//INI�t�@�C���f�[�^�擾(�Z�N�V����=[LOG_GROUP_STRING],�L�[��=LGXX,�f�t�H���g�l="***")
		GetPrivateProfileString(_T("LOG_GROUP_STRING"), subsec, _T("***"), szBuff, sizeof(szBuff), szIniFilePublic);
		if(!memcmp(szBuff, "***", 3)) break;		//INI�t�@�C�����f�[�^�擾�ł��Ȃ������ꍇ�A�����I��
		log_group[cnt] = (char*)malloc(INI_BUFF_LEN);
		strcpy_s(log_group[cnt], INI_BUFF_LEN, szBuff);
		log_group_disp[cnt] = TRUE;
		log_group_num++;
	}
	tcmLogGroupNo = log_group_num;
	log_group_disp[tcmLogGroupNo] = TRUE;
	//INI�t�@�C���f�[�^�擾(�Z�N�V����=[DEFAULT_SETTING],�L�[��=LOG_LEVEL,�f�t�H���g�l=0)
	defLogLvl = (char)GetPrivateProfileInt(_T("DEFAULT_SETTING"), _T("LOG_LEVEL"), 0, szIniFilePublic);

	return (INT_PTR)((log_group_num>0)? TRUE: FALSE);

}

void splitpath(TCHAR* fullpath)
{
	// �擾�����t�@�C���p�X�𕪊�
	_tsplitpath_s(fullpath,
					szGlDrive, _T_MAX_DRIVE,
					szGlPath, _T_MAX_DIR,
					szGlFName, _T_MAX_FNAME,
					szGlExt, _T_MAX_EXT);
}

void splitpath2(TCHAR* fullpath)
{
	// �擾�����t�@�C���p�X�𕪊�
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

	// �R�}���h���C����������R�s�[�i���̕������j�󂵂Ȃ����߁j
	char cmdline_copy[256 * 4] = { 0 };
	strncpy_s(cmdline_copy, lpCmdLine, sizeof(cmdline_copy) - 1);

	// �g�[�N�������p
	char* token1 = NULL;	// ��1����
	char* token2 = NULL;	// ��2����
	char* next_token = NULL;

	// �ŏ��̃g�[�N���i��1�����j
	token1 = strtok_s(cmdline_copy, " \t", &next_token);
	if (token1 != NULL) {
		splitpath(token1);
		// 1�Ԗڂ̈����� szPulicIniFileName �ɃR�s�[
		//�t�@�C��Drop���̓R�}���h���C��������INI�t�@�C���ȊO���ݒ肳���ꍇ��
		//����ׁA�����w�肳�ꂽ�t�@�C����INI�t�@�C���̂ݕϐ��ޔ�����
		if (!memcmp(szGlExt, ".ini", 3)) {
			strncpy_s(szPulicIniFileName, token1, sizeof(szPulicIniFileName) - 1);
			szPulicIniFileName[sizeof(szPulicIniFileName) - 1] = '\0';
		}
	}
	// 2�Ԗڂ̃g�[�N���i��2�����j
	token2 = strtok_s(NULL, " \t", &next_token);
	if (token2 != NULL) {
		splitpath2(token2);
		// 2�Ԗڂ̈��������݂���̂ŕϐ��ɃR�s�[
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

	// �R�}���h���C�������N����̕ϊ���
	if (strcmp(szCmdLine2FileName, "")) {
		splitpath2(fullpath);
		::_tcscpy_s(szDrive, _T_MAX_DRIVE * sizeof(_TCHAR), szGlDrive2);
		::_tcscpy_s(szPath, _T_MAX_DIR * sizeof(_TCHAR), szGlPath2);
		::_tcscpy_s(szFName, _T_MAX_FNAME * sizeof(_TCHAR), szGlFName2);
		::_tcscpy_s(szExt, _T_MAX_EXT * sizeof(_TCHAR), szGlExt2);
	} else { // Drop������
		//�擾�����t�@�C���p�X�𕪊�
		splitpath(fullpath);
		::_tcscpy_s(szDrive, _T_MAX_DRIVE * sizeof(_TCHAR), szGlDrive);
		::_tcscpy_s(szPath, _T_MAX_DIR * sizeof(_TCHAR), szGlPath);
		::_tcscpy_s(szFName, _T_MAX_FNAME * sizeof(_TCHAR), szGlFName);
		::_tcscpy_s(szExt, _T_MAX_EXT * sizeof(_TCHAR), szGlExt);
	}
	memcpy(szFileName, fullpath, sizeof(szFileName));

	//�g���q��*.log or *.gz �ȊO�͏����ΏۊO
	if (!strcmp(szExt, ".gz") || !strcmp(szExt, ".log")) {
		if (!strcmp(szExt, ".gz")) {
			//�𓀏���
			if (!USBFileDecomp(szFileName)) return TRUE;			//�𓀏���
			//�𓀌�̃t�@�C����Drop���ꂽ�t�@�C���Ƃ��Đݒ�
			memset(szFileName, 0, sizeof(szFileName));	//�R�}���h���C�������t�@�C�����ޔ�̈揉����
			strcpy_s(szFileName, szDrive);
			strcat_s(szFileName, szPath);
			strcat_s(szFileName, engnFileNameEntry2);

			if (!PathFileExists(szFileName)) {
				//�𓀌�̃t�@�C����Drop���ꂽ�t�@�C���Ƃ��Đݒ�
				memset(szFileName, 0, sizeof(szFileName));	//�R�}���h���C�������t�@�C�����ޔ�̈揉����
				strcpy_s(szFileName, szDrive);
				strcat_s(szFileName, szPath);
				strcat_s(szFileName, engnFileNameEntry1);
			}
		}
		//dev_proc_edbg_all.log��EngSoft�̃o�[�W�����ɑΉ�����INI�t�@�C�����ݒ�l�擾
		getLogSoftVer(szFileName, SoftVer);	//dev_proc_edbg_all.log���\�t�g�o�[�W�����擾
		//�\�t�g�o�[�W�������擾�ł����ꍇ�A�擾�o�[�W�����ɉ������e��ݒ�t�@�C�����Ď擾
		if (strcmp(SoftVer, "")) {
			getSoftVerSettingData(SoftVer);										//�\�t�g�o�[�W�����Ή����擾����
		}

		memset(szFileName, 0, sizeof(szFileName));	//�R�}���h���C�������t�@�C�����ޔ�̈揉����
		strcpy_s(szFileName, szDrive);
		strcat_s(szFileName, szPath);
		strcat_s(szFileName, engnFileNameEntry2);
		//*.log��*.tcd�ϊ�����
		char TargetFileName[_T_MAX_PATH];
		//*.tcd�̃t�@�C�����̓h���b�v���ꂽ�t�@�C���̊g���q���O�̕������g�p
		char* FileName;
		char* p;
		FileName = strtok_s(szFName, ".", &p);
		strcpy_s(TargetFileName, FileName);

		//*.tcd�t�@�C��������
		char targetFile[2048] = "";
		strcpy_s(targetFile, szDrive);
		strcat_s(targetFile, szPath);
		strcat_s(targetFile, TargetFileName);
		strcat_s(targetFile, ".tcd");
		if (!USBLogConvert(hwnd, szFileName, targetFile)) return (INT_PTR)TRUE;	//*.log��*.tcd�ϊ�����

		//���O�\���ۑ�����
		//���O�\���ۑ��p�t�@�C��������
		memset(targetFile, 0, sizeof(targetFile));
		strcpy_s(targetFile, szDrive);
		strcat_s(targetFile, szPath);
		strcat_s(targetFile, TargetFileName);
		strcat_s(targetFile, ".log");
		if (!LogSave(hwnd, targetFile)) return (INT_PTR)TRUE;		//���O�\�����e�t�@�C���ۑ�����
	}
	return (INT_PTR)TRUE;
}