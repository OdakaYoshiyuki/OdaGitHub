#include "stdafx.h"
#include "LogTool.h"

#include <stdio.h>
#include <windows.h>

// ���O���b�Z�[�W�t�@�C���p�X
char logmsg_file[_T_MAX_PATH];
// ���O���b�Z�[�W�t�@�C����������	
char logmsg_mkdate[32];
// ���b�Z�[�W�Ǘ��e�[�u���ւ̃|�C���^
MESSTR** mestbl;

// �x�[�X�E�C���h�E�ւ̃n���h��
extern HWND hBaseWindow;

// ���O���b�Z�[�W�Ǘ��e�[�u������
void create_msgtbl()
{
	FILE* stream;
	char* ctx;
	char* delim = ",";	// �f���~�^
	int group_id = -1;
	int msg_id = -1;
	char* msg = NULL;
	MESSTR* ms = NULL;
	char line[256];

	if(log_group_num == 0) {
		MessageBox(NULL, TEXT("���s�����m�F���Ă��������B"), TEXT("���s���ُ�"), MB_OK | MB_ICONSTOP);
		SendMessage(hBaseWindow, WM_COMMAND, IDM_EXIT, 0);
		return;
	}

	//[DebuggerLogMsg.mes]�t�@�C����Ǎ����[�h��Open
	errno_t err = fopen_s(&stream, logmsg_file, "r");
	if(err != 0) {
		MessageBox(NULL, TEXT("���b�Z�[�W��`�t�@�C����������܂���B"), TEXT("�G���["), MB_OK | MB_ICONSTOP);
		SendMessage(hBaseWindow, WM_COMMAND, IDM_EXIT, 0);
		return;
	}

	// ���O���b�Z�[�W�t�@�C����������
	fgets(line, sizeof(line), stream);		//[DebuggerLogMsg.mes]�t�@�C����1�s�Ǎ�
	strcpy_s(logmsg_mkdate, &line[13]);		//�擪14��������ϐ��ɑޔ�

	mestbl = (MESSTR**)calloc(log_group_num, sizeof(MESSTR*));
	for(int i = 0; i < log_group_num; i++) {
		mestbl[i] = (MESSTR*)calloc(1, sizeof(MESSTR));
	}

	//EOF�ɂȂ�܂ŁA1�s�ÂǍ�
	while(fgets(line, sizeof(line), stream)) {
		if (line[0] == '/') {
			continue;
		}
		group_id = -1;
		msg_id = -1;
		msg = NULL;
		//�ǂݍ��񂾃f�[�^��delim(",")�ŕ���
		char* next = strtok_s(line, delim, &ctx);
		//�����������ڐ���Loop
		while(next){
			if(group_id == -1) {
				group_id = atoi(next);
			}
			else if(msg_id == -1) {
				msg_id = atoi(next);
			}
			else {
				if(ctx[0] == 0x00) {
					msg = (char*)malloc(strlen(next)+2);
					strcpy_s(msg, strlen(next)+2, next);
				}
				else {
					int leng = strlen(next)+strlen(ctx);
					msg = (char*)malloc(leng+2);
					sprintf_s(msg, leng+2, "%s,%s", next, ctx);
				}
				break;
			}
	        next = strtok_s(NULL, delim, &ctx); 
		} 

		ms = mestbl[group_id];
		if(ms) {
			ms->mes_entry++;
			ms->mesID = (int**)realloc(ms->mesID, sizeof(int*)*ms->mes_entry);
			ms->mesID[ms->mes_entry-1] = (int*)malloc(sizeof(int));
			*(ms->mesID[ms->mes_entry-1]) = msg_id;
			ms->mestbl = (char**)realloc(ms->mestbl, sizeof(char*)*ms->mes_entry);
			ms->mestbl[ms->mes_entry-1] = (char*)malloc(strlen(msg)+4);
			strcpy_s(ms->mestbl[ms->mes_entry-1], strlen(msg)+2, msg);
		}
		free(msg);				//���������
	}
	fclose(stream);				//�t�@�C��Close
}

// ���O���b�Z�[�W�ւ̃|�C���^�擾
char* get_msgtbl(char group_id, int index, char arg_num)
{
	char *logMsgCpoy;
	unsigned char count = 0; 

	if (group_id >= log_group_num) {
		return NULL;
	}
	MESSTR* ms = mestbl[group_id];
	for(int i=0; i<ms->mes_entry; i++) {
		if(*(ms->mesID[i]) != index) {
			continue;
		}
		logMsgCpoy = ms->mestbl[i];
		for (unsigned int j = 0; j < strlen(logMsgCpoy); j++) {
			if (logMsgCpoy[j] == '%') {//�����̐��`�F�b�N
				count++;
			}
		}
		if (count == arg_num) {
			return (ms->mestbl[i]);
		} else {
			return NULL;
		}
	}
	return NULL;
}

// ���O���b�Z�[�W�Ǘ��e�[�u���폜
void remove_msgtbl()
{
	if(!mestbl) return;		// ���ɉ���ς�

	for(int i = 0; i < log_group_num; i++) {
		MESSTR* ms = mestbl[i];
		if(ms->mes_entry <= 0) continue;

		for(int i=0; i<ms->mes_entry; i++) {
			if(ms->mestbl[i]) {
				free(ms->mestbl[i]);
				ms->mestbl[i] = NULL;
			}
			if(ms->mesID[i]) {
				free(ms->mesID[i]);
				ms->mesID[i] = NULL;
			}
		}
		if(ms->mestbl) {
			free(ms->mestbl);
			ms->mestbl = NULL;
		}
		if(ms->mesID) {
			free(ms->mesID);
			ms->mesID = NULL;
		}
		ms->mes_entry = 0;
		free(ms);
	}

	free(mestbl);
	mestbl = NULL;
}
