#pragma once

#include <string>
#include <list>

// TMCLog�̓��e
typedef struct {
	std::string deviceId; // �f�o�C�XID+I/O���
	unsigned char io;
	char onoff;
	unsigned short value;
	unsigned long time;
	unsigned char sceParam;
} TCMLOG;

// TCD�t�@�C����$signal��
typedef struct {
	std::string deviceId; // �f�o�C�XID+I/O���
	unsigned char io;
	char lastOnOff;
	unsigned long LastTime;
	std::list<std::string> signal;
	std::string deviceName;
	unsigned char sceParam;
} TCD_SIGNAL;

// TCD�t�@�C����$bus��
typedef struct {
	std::string deviceId; // �f�o�C�XID+I/O���
	unsigned char io;
	unsigned short lastValue;
	unsigned long LastTime;
	std::list<std::string> bus;
	std::string deviceName;
	unsigned char sceParam;
} TCD_BUS;

#define SENARIO_SAVE(x)			(((x & 0xfff0) >= 0x1000) && ((x & 0xfff0) < 0x5000)) || (((x & 0xfff0) >= 0x6000) && ((x & 0xfff0) < 0x7000))
#define SENARIO_SEPARETE(x)		((x >= 0x100) && (x < 0x500)) || ((x >= 0x600) && (x < 0x700))
#define SENARIO_SAVE_DPS(x)		(((x & 0xfff0) >= 0x1000) && ((x & 0xfff0) < 0x4000)) || (((x & 0xfff0) >= 0x6000) && ((x & 0xfff0) < 0x7000))
#define SENARIO_SEPARETE_DPS(x)	((x >= 0x100) && (x < 0x400)) || ((x >= 0x600) && (x < 0x700))
#define SENARIO_SAVE_DF(x)		(((x & 0xfff0) >= 0x1000) && ((x & 0xfff0) < 0x5000)) || (((x & 0xfff0) >= 0x8000) && ((x & 0xfff0) < 0x9000))
#define SENARIO_SEPARETE_DF(x)	((x >= 0x100) && (x < 0x500)) || ((x >= 0x800) && (x < 0x900))

extern unsigned long msPerClock;			// �N���b�N������̎���
extern unsigned long collisionDeleyTime;	// ���߂̐M���Ǝ��Ԃ������������ꍇ�ɂ��炷����
extern int sortDevice;						// ini�t�@�C���ɏ�����Ă��鏇�Ƀ\�[�g���邩


// �֐���`�v���g�^�C�v��`
extern void writeTcdFile(FILE* wstream, std::list<TCMLOG>listTcm, char *szIniFilePath);
extern bool senarioSeparete(unsigned short id);
extern bool senarioSave(unsigned short id);

// �֐���`�v���g�^�C�v��`
static void writeTcdHeader(FILE* wstream);
static void writeTcdSignals(FILE* wstream, std::list<TCMLOG>listTcm);
static void writeTcdBuses(FILE* wstream, std::list<TCMLOG>listTcm);
static void writeTcdSignal(FILE* wstream, TCD_SIGNAL tcdSignal, unsigned long minTime, unsigned long maxTime);
static void writeTcdBus(FILE* wstream, TCD_BUS tcdBus, unsigned long minTime, unsigned long maxTime);
static std::list<std::string> getSortOderList();
static std::string getDeviceName(std::string deviceId);
static std::list<TCD_SIGNAL> tcm2tcdSignal(std::list<TCMLOG>listTcm);
static std::list<TCD_BUS> tcm2tcdBus(std::list<TCMLOG>listTcm);
static unsigned long getMinTime(std::list<TCMLOG>listTcm);
static unsigned long getMaxTime(std::list<TCMLOG>listTcm);
static unsigned long getFixedTime(unsigned long minTime, unsigned long time);
