// ロググループ最大登録数
#define LOGGRP_DEFNUM	128

// LogMessageManagementSheet_core.xlsmのロググループの定義と一致させること。
#define LOGGRP_SYSTEM		0x00
#define LOGGRP_DEBUGGER		0x03
#define LOGGRP_USBLOG		0x26

#define LOGID_SYS_TCM		7
#define LOGID_TCM_SYS		8

//タイミングチャートのシナリオのPAGEIDになる桁
#define TCM_PAGEID_DIGIT 1000

// teratermログチェック間隔(ms)
#define UPDATE_TIMING	500

//Ver2.09 Insert Start
// 振替プロジェクトコード登録数最大値
#define TRANFER_PJ_MAX	128
//Ver2.09 Insert End
