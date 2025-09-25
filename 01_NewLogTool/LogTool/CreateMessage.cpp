#include "stdafx.h"
#include "LogTool.h"

#include <stdio.h>

// ログメッセージバッファ
char line_mbuf[2048];
char line_head[2048];
// ロググループテーブル
char* log_group[LOGGRP_DEFNUM];
// ロググループ登録数
char log_group_num = 0;

extern void fixLogInfo(char* form_msg,unsigned long timeCount, unsigned long value);

char CHAR_CHK(char cd)
{
	if((cd >= ' ') && ('~' >= cd)) {
		return cd;
	}
	else {
		return '.';
	}
}

int createLogMessage(FILE* stream,			// ログファイルへのポインタ
					unsigned long tim,		// ログタイムスタンプ
					short log_no,			// ログNo
					char err_lvl,			// エラーレベル
					char group_id,			// グループID
					char arg_type,			// 引数タイプ
					char arg_num,			// 引数の数
					char* form_msg,			// ログメッセージへのポインタ
					unsigned char* usb_log_arg)	// 引数へのポインタ(USBログ時のみ)
{
	// 任意ログメッセージ出力用定義
	#define MAX_PARAM			(15 * sizeof(long))	// 最大パラメータ数
	#define SPACE_CODE			' '					// スペース文字コード

	char arg_buf_base[64];

	char* pt1;
	unsigned char* pt2;
	short* pt3;
	unsigned short* pt4;
	long* pt5;
	unsigned long* pt6;

	char swp;
	char erlvl[] = {'E', 'W', 'I', 'S'};

	// (デバッガ、USBログ以外)タイムスタンプ等表示
	if((group_id != LOGGRP_DEBUGGER) && (group_id != LOGGRP_USBLOG)) {
		sprintf_s(line_head, 1024, "%09ld [%c] [%s] ", tim, erlvl[err_lvl], log_group[group_id]);
	}

	// 引数読み出す
	if(arg_type > 0 && arg_num > 0) {
		switch(arg_type) {
		case 1:		// signed char
			pt1 = (char*)arg_buf_base;
			for(int i=0; i<arg_num; i++, pt1++) {
				if(stream) {
					if(fread(pt1, sizeof(char), 1, stream) <= 0) {
						return -1;
					}
				} else {
					memcpy(pt1, &usb_log_arg[i], 1);
				}
			}
			pt1 = (char*)arg_buf_base;
			break;
		case 2:		// unsigned char
			pt2 = (unsigned char*)arg_buf_base;
			for(int i=0; i<arg_num; i++, pt2++) {
				if(stream) {
					if(fread(pt2, sizeof(char), 1, stream) <= 0) {
						return -1;
					}
				} else {
					memcpy(pt2, &usb_log_arg[i], 1);
				}
			}
			pt2 = (unsigned char*)arg_buf_base;
			break;
		case 4:		// signed short
			pt3 = (short*)arg_buf_base;
			for(int i=0; i<arg_num; i++, pt3++) {
				if(stream) {
					if(fread(pt3, sizeof(short), 1, stream) <= 0) {
						return -1;
					}
				} else {
					memcpy(pt3, &usb_log_arg[i*2], 2);
				}
				swp = *((unsigned char*)pt3);
				*((unsigned char*)pt3) = *((unsigned char*)pt3+1);
				*((unsigned char*)pt3+1) = swp;
			}
			pt3 = (short*)arg_buf_base;
			break;
		case 5:		// unsigned short
			pt4 = (unsigned short*)arg_buf_base;
			for(int i=0; i<arg_num; i++, pt4++) {
				if(stream) {
					if(fread(pt4, sizeof(short), 1, stream) <= 0) {
						return -1;
					}
				} else {
					memcpy(pt4, &usb_log_arg[i*2], 2);
				}
				swp = *((unsigned char*)pt4);
				*((unsigned char*)pt4) = *((unsigned char*)pt4+1);
				*((unsigned char*)pt4+1) = swp;
			}
			pt4 = (unsigned short*)arg_buf_base;
			break;
		case 6:		// signed long
			pt5 = (long*)arg_buf_base;
			for(int i=0; i<arg_num; i++, pt5++) {
				if(stream) {
					if(fread(pt5, sizeof(long), 1, stream) <= 0) {
						return -1;
					}
				} else {
					memcpy(pt5, &usb_log_arg[i*4], 4);
				}
				swp = *((unsigned char*)pt5);
				*((unsigned char*)pt5) = *((unsigned char*)pt5+3);
				*((unsigned char*)pt5+3) = swp;
				swp = *((unsigned char*)pt5+1);
				*((unsigned char*)pt5+1) = *((unsigned char*)pt5+2);
				*((unsigned char*)pt5+2) = swp;
			}
			pt5 = (long*)arg_buf_base;
			break;
		case 7:		// unsigned long
			pt6 = (unsigned long*)arg_buf_base;
			for(int i=0; i<arg_num; i++, pt6++) {
				if(stream) {
					if(fread(pt6, sizeof(long), 1, stream) <= 0) {
						return -1;
					}
				} else {
					memcpy(pt6, &usb_log_arg[i*4], 4);
				}
				swp = *((unsigned char*)pt6);
				*((unsigned char*)pt6) = *((unsigned char*)pt6+3);
				*((unsigned char*)pt6+3) = swp;
				swp = *((unsigned char*)pt6+1);
				*((unsigned char*)pt6+1) = *((unsigned char*)pt6+2);
				*((unsigned char*)pt6+2) = swp;
			}
			pt6 = (unsigned long*)arg_buf_base;
			break;
		}
	}

	if(form_msg == NULL) {
		return 0;
	}

	switch(arg_num) {
	case 0:
		sprintf_s(line_mbuf, 1024, form_msg);
		break;
	case 1:
		if(arg_type == 1) {
			sprintf_s(line_mbuf, 1024, form_msg, pt1[0]);
		}
		else if(arg_type == 2) {
			sprintf_s(line_mbuf, 1024, form_msg, pt2[0]);
		}
		else if(arg_type == 4) {
			sprintf_s(line_mbuf, 1024, form_msg, pt3[0]);
		}
		else if(arg_type == 5) {
			sprintf_s(line_mbuf, 1024, form_msg, pt4[0]);
		}
		else if(arg_type == 6) {
			sprintf_s(line_mbuf, 1024, form_msg, pt5[0]);
		}
		else if(arg_type == 7) {
			sprintf_s(line_mbuf, 1024, form_msg, pt6[0]);
		}
		break;
	case 2:
		if(arg_type == 1) {
			sprintf_s(line_mbuf, 1024, form_msg, pt1[0], pt1[1]);
		}
		else if(arg_type == 2) {
			sprintf_s(line_mbuf, 1024, form_msg, pt2[0], pt2[1]);
		}
		else if(arg_type == 4) {
			sprintf_s(line_mbuf, 1024, form_msg, pt3[0], pt3[1]);
		}
		else if(arg_type == 5) {
			sprintf_s(line_mbuf, 1024, form_msg, pt4[0], pt4[1]);
		}
		else if(arg_type == 6) {
			sprintf_s(line_mbuf, 1024, form_msg, pt5[0], pt5[1]);
		}
		else if(arg_type == 7) {
			sprintf_s(line_mbuf, 1024, form_msg, pt6[0], pt6[1]);
			fixLogInfo(&form_msg[6], pt6[0], pt6[1]);
		}
		break;
	case 3:
		if(arg_type == 1) {
			sprintf_s(line_mbuf, 1024, form_msg, pt1[0], pt1[1], pt1[2]);
		}
		else if(arg_type == 2) {
			sprintf_s(line_mbuf, 1024, form_msg, pt2[0], pt2[1], pt2[2]);
		}
		else if(arg_type == 4) {
			sprintf_s(line_mbuf, 1024, form_msg, pt3[0], pt3[1], pt3[2]);
		}
		else if(arg_type == 5) {
			sprintf_s(line_mbuf, 1024, form_msg, pt4[0], pt4[1], pt4[2]);
		}
		else if(arg_type == 6) {
			sprintf_s(line_mbuf, 1024, form_msg, pt5[0], pt5[1], pt5[2]);
		}
		else if(arg_type == 7) {
			sprintf_s(line_mbuf, 1024, form_msg, pt6[0], pt6[1], pt6[2]);
		}
		break;
	case 4:
		if(arg_type == 1) {
			sprintf_s(line_mbuf, 1024, form_msg, pt1[0], pt1[1], pt1[2], pt1[3]);
		}
		else if(arg_type == 2) {
			sprintf_s(line_mbuf, 1024, form_msg, pt2[0], pt2[1], pt2[2], pt2[3]);
		}
		else if(arg_type == 4) {
			sprintf_s(line_mbuf, 1024, form_msg, pt3[0], pt3[1], pt3[2], pt3[3]);
		}
		else if(arg_type == 5) {
			sprintf_s(line_mbuf, 1024, form_msg, pt4[0], pt4[1], pt4[2], pt4[3]);
		}
		else if(arg_type == 6) {
			sprintf_s(line_mbuf, 1024, form_msg, pt5[0], pt5[1], pt5[2], pt5[3]);
		}
		else if(arg_type == 7) {
			sprintf_s(line_mbuf, 1024, form_msg, pt6[0], pt6[1], pt6[2], pt6[3]);
		}
		break;
	case 5:
		if(arg_type == 1) {
			sprintf_s(line_mbuf, 1024, form_msg, pt1[0], pt1[1], pt1[2], pt1[3], pt1[4]);
		}
		else if(arg_type == 2) {
			sprintf_s(line_mbuf, 1024, form_msg, pt2[0], pt2[1], pt2[2], pt2[3], pt2[4]);
		}
		else if(arg_type == 4) {
			sprintf_s(line_mbuf, 1024, form_msg, pt3[0], pt3[1], pt3[2], pt3[3], pt3[4]);
		}
		else if(arg_type == 5) {
			sprintf_s(line_mbuf, 1024, form_msg, pt4[0], pt4[1], pt4[2], pt4[3], pt4[4]);
		}
		else if(arg_type == 6) {
			sprintf_s(line_mbuf, 1024, form_msg, pt5[0], pt5[1], pt5[2], pt5[3], pt5[4]);
		}
		else if(arg_type == 7) {
			sprintf_s(line_mbuf, 1024, form_msg, pt6[0], pt6[1], pt6[2], pt6[3], pt6[4]);
		}
		break;
	case 6:
		if(arg_type == 1) {
			sprintf_s(line_mbuf, 1024, form_msg, pt1[0], pt1[1], pt1[2], pt1[3], pt1[4], pt1[5]);
		}
		else if(arg_type == 2) {
			sprintf_s(line_mbuf, 1024, form_msg, pt2[0], pt2[1], pt2[2], pt2[3], pt2[4], pt2[5]);
		}
		else if(arg_type == 4) {
			sprintf_s(line_mbuf, 1024, form_msg, pt3[0], pt3[1], pt3[2], pt3[3], pt3[4], pt3[5]);
		}
		else if(arg_type == 5) {
			sprintf_s(line_mbuf, 1024, form_msg, pt4[0], pt4[1], pt4[2], pt4[3], pt4[4], pt4[5]);
		}
		else if(arg_type == 6) {
			sprintf_s(line_mbuf, 1024, form_msg, pt5[0], pt5[1], pt5[2], pt5[3], pt5[4], pt5[5]);
		}
		else if(arg_type == 7) {
			sprintf_s(line_mbuf, 1024, form_msg, pt6[0], pt6[1], pt6[2], pt6[3], pt6[4], pt6[5]);
		}
		break;
	case 7:
		if(arg_type == 1) {
			sprintf_s(line_mbuf, 1024, form_msg, pt1[0], pt1[1], pt1[2], pt1[3], pt1[4], pt1[5], pt1[6]);
		}
		else if(arg_type == 2) {
			sprintf_s(line_mbuf, 1024, form_msg, pt2[0], pt2[1], pt2[2], pt2[3], pt2[4], pt2[5], pt2[6]);
		}
		else if(arg_type == 4) {
			sprintf_s(line_mbuf, 1024, form_msg, pt3[0], pt3[1], pt3[2], pt3[3], pt3[4], pt3[5], pt3[6]);
		}
		else if(arg_type == 5) {
			sprintf_s(line_mbuf, 1024, form_msg, pt4[0], pt4[1], pt4[2], pt4[3], pt4[4], pt4[5], pt4[6]);
		}
		else if(arg_type == 6) {
			sprintf_s(line_mbuf, 1024, form_msg, pt5[0], pt5[1], pt5[2], pt5[3], pt5[4], pt5[5], pt5[6]);
		}
		else if(arg_type == 7) {
			sprintf_s(line_mbuf, 1024, form_msg, pt6[0], pt6[1], pt6[2], pt6[3], pt6[4], pt6[5], pt6[6]);
		}
		break;
	case 8:
		if(arg_type == 1) {
			sprintf_s(line_mbuf, 1024, form_msg, pt1[0], pt1[1], pt1[2], pt1[3], pt1[4], pt1[5], pt1[6], pt1[7]);
		}
		else if(arg_type == 2) {
			sprintf_s(line_mbuf, 1024, form_msg, pt2[0], pt2[1], pt2[2], pt2[3], pt2[4], pt2[5], pt2[6], pt2[7]);
		}
		else if(arg_type == 4) {
			sprintf_s(line_mbuf, 1024, form_msg, pt3[0], pt3[1], pt3[2], pt3[3], pt3[4], pt3[5], pt3[6], pt3[7]);
		}
		else if(arg_type == 5) {
			sprintf_s(line_mbuf, 1024, form_msg, pt4[0], pt4[1], pt4[2], pt4[3], pt4[4], pt4[5], pt4[6], pt4[7]);
		}
		else if(arg_type == 6) {
			sprintf_s(line_mbuf, 1024, form_msg, pt5[0], pt5[1], pt5[2], pt5[3], pt5[4], pt5[5], pt5[6], pt5[7]);
		}
		else if(arg_type == 7) {
			sprintf_s(line_mbuf, 1024, form_msg, pt6[0], pt6[1], pt6[2], pt6[3], pt6[4], pt6[5], pt6[6], pt6[7]);
		}
		break;
	case 9:
		if(arg_type == 1) {
			sprintf_s(line_mbuf, 1024, form_msg, pt1[0], pt1[1], pt1[2], pt1[3], pt1[4], pt1[5], pt1[6], pt1[7], pt1[8]);
		}
		else if(arg_type == 2) {
			sprintf_s(line_mbuf, 1024, form_msg, pt2[0], pt2[1], pt2[2], pt2[3], pt2[4], pt2[5], pt2[6], pt2[7], pt2[8]);
		}
		else if(arg_type == 4) {
			sprintf_s(line_mbuf, 1024, form_msg, pt3[0], pt3[1], pt3[2], pt3[3], pt3[4], pt3[5], pt3[6], pt3[7], pt3[8]);
		}
		else if(arg_type == 5) {
			sprintf_s(line_mbuf, 1024, form_msg, pt4[0], pt4[1], pt4[2], pt4[3], pt4[4], pt4[5], pt4[6], pt4[7], pt4[8]);
		}
		else if(arg_type == 6) {
			sprintf_s(line_mbuf, 1024, form_msg, pt5[0], pt5[1], pt5[2], pt5[3], pt5[4], pt5[5], pt5[6], pt5[7], pt5[8]);
		}
		else if(arg_type == 7) {
			sprintf_s(line_mbuf, 1024, form_msg, pt6[0], pt6[1], pt6[2], pt6[3], pt6[4], pt6[5], pt6[6], pt6[7], pt6[8]);
		}
		break;
	case 10:
		if(arg_type == 1) {
			sprintf_s(line_mbuf, 1024, form_msg, pt1[0], pt1[1], pt1[2], pt1[3], pt1[4], pt1[5], pt1[6], pt1[7], pt1[8], pt1[9]);
		}
		else if(arg_type == 2) {
			sprintf_s(line_mbuf, 1024, form_msg, pt2[0], pt2[1], pt2[2], pt2[3], pt2[4], pt2[5], pt2[6], pt2[7], pt2[8], pt2[9]);
		}
		else if(arg_type == 4) {
			sprintf_s(line_mbuf, 1024, form_msg, pt3[0], pt3[1], pt3[2], pt3[3], pt3[4], pt3[5], pt3[6], pt3[7], pt3[8], pt3[9]);
		}
		else if(arg_type == 5) {
			sprintf_s(line_mbuf, 1024, form_msg, pt4[0], pt4[1], pt4[2], pt4[3], pt4[4], pt4[5], pt4[6], pt4[7], pt4[8], pt4[9]);
		}
		else if(arg_type == 6) {
			sprintf_s(line_mbuf, 1024, form_msg, pt5[0], pt5[1], pt5[2], pt5[3], pt5[4], pt5[5], pt5[6], pt5[7], pt5[8], pt5[9]);
		}
		else if(arg_type == 7) {
			sprintf_s(line_mbuf, 1024, form_msg, pt6[0], pt6[1], pt6[2], pt6[3], pt6[4], pt6[5], pt6[6], pt6[7], pt6[8], pt6[9]);
		}
		break;
	case 11:
		if(arg_type == 1) {
			sprintf_s(line_mbuf, 1024, form_msg, pt1[0], pt1[1], pt1[2], pt1[3], pt1[4], pt1[5], pt1[6], pt1[7], pt1[8], pt1[9], pt1[10]);
		}
		else if(arg_type == 2) {
			sprintf_s(line_mbuf, 1024, form_msg, pt2[0], pt2[1], pt2[2], pt2[3], pt2[4], pt2[5], pt2[6], pt2[7], pt2[8], pt2[9], pt2[10]);
		}
		else if(arg_type == 4) {
			sprintf_s(line_mbuf, 1024, form_msg, pt3[0], pt3[1], pt3[2], pt3[3], pt3[4], pt3[5], pt3[6], pt3[7], pt3[8], pt3[9], pt3[10]);
		}
		else if(arg_type == 5) {
			sprintf_s(line_mbuf, 1024, form_msg, pt4[0], pt4[1], pt4[2], pt4[3], pt4[4], pt4[5], pt4[6], pt4[7], pt4[8], pt4[9], pt4[10]);
		}
		else if(arg_type == 6) {
			sprintf_s(line_mbuf, 1024, form_msg, pt5[0], pt5[1], pt5[2], pt5[3], pt5[4], pt5[5], pt5[6], pt5[7], pt5[8], pt5[9], pt5[10]);
		}
		else if(arg_type == 7) {
			sprintf_s(line_mbuf, 1024, form_msg, pt6[0], pt6[1], pt6[2], pt6[3], pt6[4], pt6[5], pt6[6], pt6[7], pt6[8], pt6[9], pt6[10]);
		}
		break;
	case 12:
		if(arg_type == 1) {
			sprintf_s(line_mbuf, 1024, form_msg, pt1[0], pt1[1], pt1[2], pt1[3], pt1[4], pt1[5], pt1[6], pt1[7], pt1[8], pt1[9], pt1[10], pt1[11]);
		}
		else if(arg_type == 2) {
			sprintf_s(line_mbuf, 1024, form_msg, pt2[0], pt2[1], pt2[2], pt2[3], pt2[4], pt2[5], pt2[6], pt2[7], pt2[8], pt2[9], pt2[10], pt2[11]);
		}
		else if(arg_type == 4) {
			sprintf_s(line_mbuf, 1024, form_msg, pt3[0], pt3[1], pt3[2], pt3[3], pt3[4], pt3[5], pt3[6], pt3[7], pt3[8], pt3[9], pt3[10], pt3[11]);
		}
		else if(arg_type == 5) {
			sprintf_s(line_mbuf, 1024, form_msg, pt4[0], pt4[1], pt4[2], pt4[3], pt4[4], pt4[5], pt4[6], pt4[7], pt4[8], pt4[9], pt4[10], pt4[11]);
		}
		else if(arg_type == 6) {
			sprintf_s(line_mbuf, 1024, form_msg, pt5[0], pt5[1], pt5[2], pt5[3], pt5[4], pt5[5], pt5[6], pt5[7], pt5[8], pt5[9], pt5[10], pt5[11]);
		}
		else if(arg_type == 7) {
			sprintf_s(line_mbuf, 1024, form_msg, pt6[0], pt6[1], pt6[2], pt6[3], pt6[4], pt6[5], pt6[6], pt6[7], pt6[8], pt6[9], pt6[10], pt6[11]);
		}
		break;
	case 13:
		if(arg_type == 1) {
			sprintf_s(line_mbuf, 1024, form_msg, pt1[0], pt1[1], pt1[2], pt1[3], pt1[4], pt1[5], pt1[6], pt1[7], pt1[8], pt1[9], pt1[10], pt1[11], pt1[12]);
		}
		else if(arg_type == 2) {
			sprintf_s(line_mbuf, 1024, form_msg, pt2[0], pt2[1], pt2[2], pt2[3], pt2[4], pt2[5], pt2[6], pt2[7], pt2[8], pt2[9], pt2[10], pt2[11], pt2[12]);
		}
		else if(arg_type == 4) {
			sprintf_s(line_mbuf, 1024, form_msg, pt3[0], pt3[1], pt3[2], pt3[3], pt3[4], pt3[5], pt3[6], pt3[7], pt3[8], pt3[9], pt3[10], pt3[11], pt3[12]);
		}
		else if(arg_type == 5) {
			sprintf_s(line_mbuf, 1024, form_msg, pt4[0], pt4[1], pt4[2], pt4[3], pt4[4], pt4[5], pt4[6], pt4[7], pt4[8], pt4[9], pt4[10], pt4[11], pt4[12]);
		}
		else if(arg_type == 6) {
			sprintf_s(line_mbuf, 1024, form_msg, pt5[0], pt5[1], pt5[2], pt5[3], pt5[4], pt5[5], pt5[6], pt5[7], pt5[8], pt5[9], pt5[10], pt5[11], pt5[12]);
		}
		else if(arg_type == 7) {
			sprintf_s(line_mbuf, 1024, form_msg, pt6[0], pt6[1], pt6[2], pt6[3], pt6[4], pt6[5], pt6[6], pt6[7], pt6[8], pt6[9], pt6[10], pt6[11], pt6[12]);
		}
		break;
	case 14:
		if(arg_type == 1) {
			sprintf_s(line_mbuf, 1024, form_msg, pt1[0], pt1[1], pt1[2], pt1[3], pt1[4], pt1[5], pt1[6], pt1[7], pt1[8], pt1[9], pt1[10], pt1[11], pt1[12], pt1[13]);
		}
		else if(arg_type == 2) {
			sprintf_s(line_mbuf, 1024, form_msg, pt2[0], pt2[1], pt2[2], pt2[3], pt2[4], pt2[5], pt2[6], pt2[7], pt2[8], pt2[9], pt2[10], pt2[11], pt2[12], pt2[13]);
		}
		else if(arg_type == 4) {
			sprintf_s(line_mbuf, 1024, form_msg, pt3[0], pt3[1], pt3[2], pt3[3], pt3[4], pt3[5], pt3[6], pt3[7], pt3[8], pt3[9], pt3[10], pt3[11], pt3[12], pt3[13]);
		}
		else if(arg_type == 5) {
			sprintf_s(line_mbuf, 1024, form_msg, pt4[0], pt4[1], pt4[2], pt4[3], pt4[4], pt4[5], pt4[6], pt4[7], pt4[8], pt4[9], pt4[10], pt4[11], pt4[12], pt4[13]);
		}
		else if(arg_type == 6) {
			sprintf_s(line_mbuf, 1024, form_msg, pt5[0], pt5[1], pt5[2], pt5[3], pt5[4], pt5[5], pt5[6], pt5[7], pt5[8], pt5[9], pt5[10], pt5[11], pt5[12], pt5[13]);
		}
		else if(arg_type == 7) {
			sprintf_s(line_mbuf, 1024, form_msg, pt6[0], pt6[1], pt6[2], pt6[3], pt6[4], pt6[5], pt6[6], pt6[7], pt6[8], pt6[9], pt6[10], pt6[11], pt6[12], pt6[13]);
		}
		break;
	case 15:
		if(arg_type == 1) {
			sprintf_s(line_mbuf, 1024, form_msg, pt1[0], pt1[1], pt1[2], pt1[3], pt1[4], pt1[5], pt1[6], pt1[7], pt1[8], pt1[9], pt1[10], pt1[11], pt1[12], pt1[13], pt1[14]);
		}
		else if(arg_type == 2) {
			sprintf_s(line_mbuf, 1024, form_msg, pt2[0], pt2[1], pt2[2], pt2[3], pt2[4], pt2[5], pt2[6], pt2[7], pt2[8], pt2[9], pt2[10], pt2[11], pt2[12], pt2[13], pt2[14]);
		}
		else if(arg_type == 4) {
			sprintf_s(line_mbuf, 1024, form_msg, pt3[0], pt3[1], pt3[2], pt3[3], pt3[4], pt3[5], pt3[6], pt3[7], pt3[8], pt3[9], pt3[10], pt3[11], pt3[12], pt3[13], pt3[14]);
		}
		else if(arg_type == 5) {
			sprintf_s(line_mbuf, 1024, form_msg, pt4[0], pt4[1], pt4[2], pt4[3], pt4[4], pt4[5], pt4[6], pt4[7], pt4[8], pt4[9], pt4[10], pt4[11], pt4[12], pt4[13], pt4[14]);
		}
		else if(arg_type == 6) {
			sprintf_s(line_mbuf, 1024, form_msg, pt5[0], pt5[1], pt5[2], pt5[3], pt5[4], pt5[5], pt5[6], pt5[7], pt5[8], pt5[9], pt5[10], pt5[11], pt5[12], pt5[13], pt5[14]);
		}
		else if(arg_type == 7) {
			sprintf_s(line_mbuf, 1024, form_msg, pt6[0], pt6[1], pt6[2], pt6[3], pt6[4], pt6[5], pt6[6], pt6[7], pt6[8], pt6[9], pt6[10], pt6[11], pt6[12], pt6[13], pt6[14]);
		}
		break;
	}

	// [特殊処理]
	// デバッガ、メモリダンプ
	// (メモリダンプの最後に文字を出力)
	if((group_id == LOGGRP_DEBUGGER && log_no == 5) ||
	   (group_id == LOGGRP_USBLOG && log_no == 3)) {
		char chrbuf[64];
		sprintf_s(chrbuf, 64, "  * %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c *\n",
			CHAR_CHK((pt4[2] >> 8) & 0xff), CHAR_CHK(pt4[2] & 0xff),
			CHAR_CHK((pt4[3] >> 8) & 0xff), CHAR_CHK(pt4[3] & 0xff),
			CHAR_CHK((pt4[4] >> 8) & 0xff), CHAR_CHK(pt4[4] & 0xff),
			CHAR_CHK((pt4[5] >> 8) & 0xff), CHAR_CHK(pt4[5] & 0xff),
			CHAR_CHK((pt4[6] >> 8) & 0xff), CHAR_CHK(pt4[6] & 0xff),
			CHAR_CHK((pt4[7] >> 8) & 0xff), CHAR_CHK(pt4[7] & 0xff),
			CHAR_CHK((pt4[8] >> 8) & 0xff), CHAR_CHK(pt4[8] & 0xff),
			CHAR_CHK((pt4[9] >> 8) & 0xff), CHAR_CHK(pt4[9] & 0xff));
		line_mbuf[strlen(line_mbuf) - 1] = 0x00; //最後の改行を削除してから後に文字列の追加
		strcat_s(line_mbuf, chrbuf);

	// 任意のログメッセージを出力
	} else if((group_id == LOGGRP_SYSTEM) && (log_no == 1)) {
		unsigned char* src = (unsigned char*)pt6;
		unsigned char* dst = (unsigned char*)line_mbuf;
		unsigned char* last_spc_pos = NULL;
		for (unsigned char pos = 0; pos < MAX_PARAM; pos++, src++, dst++) {
			*dst = *src;
			if (*dst == SPACE_CODE) {
				if (*(dst - 1) != SPACE_CODE) {	// スペースが連続したら先頭スペースのみ。
					last_spc_pos = dst;
				}
			} else if (*dst != 0x00) {
				last_spc_pos = NULL;
			} else {
			}
		}
		// スペース分はTrimする
		if (last_spc_pos) {
			dst = last_spc_pos;
		}
		// 行末/改行コードセット
		*dst = 0x00;
		strcat_s(line_mbuf, "\n");
	} else {
	}

	// デバッガ以外はタイムスタンプ等表示
	if((group_id != LOGGRP_DEBUGGER) &&  (group_id != LOGGRP_USBLOG)) {
		strcat_s(line_head, line_mbuf);
		strcpy_s(line_mbuf, line_head);
	}

	int msg_len = strlen(line_mbuf);
	return msg_len;
}
