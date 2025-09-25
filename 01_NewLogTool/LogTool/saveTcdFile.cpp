#include "stdafx.h"
#include "saveTcdFile.h"
#include "LogTool.h"

// �O���[�o���ϐ�
char *iniFile;				// Ini�t�@�C���p�X
unsigned long msPerClock;			// �N���b�N������̎���
unsigned long collisionDeleyTime;	// ���߂̐M���Ǝ��Ԃ������������ꍇ�ɂ��炷����
bool isSynchroLogAndTcm = false;
int sortDevice;						// ini�t�@�C���ɏ�����Ă��鏇�Ƀ\�[�g���邩

void writeTcdFile(FILE* wstream, std::list<TCMLOG>listTcm, char *szIniFilePath)
{
	// �������ׂ݂����O���Ȃ���Ή������Ȃ�
	if (listTcm.size() == 0) {
		return;
	}

	iniFile = szIniFilePath;
	writeTcdHeader(wstream);
	writeTcdSignals(wstream, listTcm);
	writeTcdBuses(wstream, listTcm);
}


// ini�t�@�C������f�o�C�X����`�����擾����
std::list<std::string> getSortOderList()
{
	std::list<std::string> sortOrderList;
	void* wkBuff = NULL;
	TCHAR* devcd_key;

	// ini�t�@�C���ǂݍ���
	int devcd_key_size = GetPrivateProfileInt(_T("TCD_SETTING"), _T("WK_BUFF_SIZE"), 0, iniFile);
	if (devcd_key_size < 2) {
		devcd_key_size = 4096;	//�f�[�^���Ȃ��A0���w�肳��Ă���ȂǑ΍�
	}
	wkBuff = malloc(devcd_key_size);
	devcd_key = (TCHAR*)wkBuff;
	GetPrivateProfileString(_T("DEVICE_NAME"), NULL, _T(""), devcd_key, devcd_key_size, iniFile);	// �߂�l��devcd_key_size-2�Ȃ玸�s���Ă���\������

	// �擾�����L�[�������X�g�ɓo�^����
	while (*devcd_key != NULL) {
		sortOrderList.push_back(devcd_key);
		devcd_key += lstrlen(devcd_key) + 1;	// �|�C���^�� �������{NULL �̕������i�߂�
	}

	free(wkBuff);

	return sortOrderList;
}

// �f�o�C�XID����f�o�C�X�����擾����
std::string getDeviceName(std::string deviceId)
{
	// ini�t�@�C������ǂݍ���
	TCHAR readBuf[256];
	char devId[6];
	int val = std::stoi(deviceId, nullptr, 16);

	sprintf_s(devId, 6, "%05x", val & 0xfffff);
	GetPrivateProfileString(_T("DEVICE_NAME"), devId, devId, readBuf, sizeof(readBuf) - 5, iniFile);
	// �V�i���I�̓p�����[�^�����id�Ƃ���
	if (senarioSave(val)) {
		sprintf_s(readBuf, sizeof(readBuf), "%s[%02d]", readBuf, (val >> 20));
	}
	return readBuf;
}

// TCM���O���玞�Ԃ̍ŏ��l�擾
unsigned long getMinTime(std::list<TCMLOG>listTcm)
{
	std::list<TCMLOG>::iterator tcmIterator = listTcm.begin();
	unsigned long minTime;

	if (isSynchroLogAndTcm) {
		minTime = 0;
	} else {
		minTime = tcmIterator->time;
		for (; tcmIterator != listTcm.end(); tcmIterator++) {
			if (tcmIterator->time < minTime) {
				minTime = tcmIterator->time;
			}
		}
	
		if (minTime >= 40) {
			minTime = minTime - 40;
		} else {
			minTime = 0;
		}
	}

	return minTime;
}

// TCM���O���玞�Ԃ̍ő�l�擾
unsigned long getMaxTime(std::list<TCMLOG>listTcm)
{
	std::list<TCMLOG>::iterator tcmIterator = listTcm.begin();
	unsigned long maxTime = tcmIterator->time;

	for(; tcmIterator!=listTcm.end(); tcmIterator++) {
		if(tcmIterator->time > maxTime) {
			maxTime = tcmIterator->time;
		}
	}

	return maxTime;
}

// �^�C���`���[�g��ɕ\�����鎞�Ԃ��擾
unsigned long getFixedTime(unsigned long minTime, unsigned long time){
	unsigned long fixedTime;

	fixedTime = (time - minTime) * msPerClock;

	return fixedTime;
}

// tcmlog����tcd�t�@�C�����쐬���邽�߂ɕK�v�ȏ��𐶐�����B
std::list<TCD_SIGNAL> tcm2tcdSignal(std::list<TCMLOG>listTcm)
{
	std::list<TCD_SIGNAL> listTcdSignal; // TCD�V�O�i����

	std::list<TCMLOG>::iterator tcmIterator;
	unsigned long minTime = getMinTime(listTcm);
	std::list<TCD_SIGNAL>::iterator tcdSignalIterator;

	// TCD�V�O�i�����ɕϊ�
	tcmIterator = listTcm.begin();
	tcmIterator++; // EngineLogViewer�͏��߂̒l�𖳎����Ă���̂ł���ɂ��킹��B
	for(; tcmIterator != listTcm.end(); tcmIterator++){

		// �f�o�C�X��񂪖��i�[�̏ꍇ�B�V���ɒǉ�����
		// �f�o�C�X��񂪂��łɃ��X�g�̒��ɂ��邩
		for(tcdSignalIterator = listTcdSignal.begin(); tcdSignalIterator != listTcdSignal.end(); tcdSignalIterator++) {
			if(tcdSignalIterator->deviceId == tcmIterator->deviceId &&
				tcdSignalIterator->io == tcmIterator->io){
				break;
			}
		}
		// �Ȃ��ꍇ�͒ǉ�
		if (tcdSignalIterator == listTcdSignal.end()) {
			// �f�o�C�X���擾
			std::string deviceName = getDeviceName(tcmIterator->deviceId);

			std::list<std::string> listSignal; // �V�O�i���̃��X�g
			std::string initialValue = "Z";
			listSignal.push_back(initialValue);
			TCD_SIGNAL newDevice = {tcmIterator->deviceId, tcmIterator->io, tcmIterator->onoff, 0, listSignal, deviceName};

			listTcdSignal.push_back(newDevice);
		}

		// �ɃV�O�i������ǉ����ׂ��f�o�C�X��������
		tcdSignalIterator = listTcdSignal.begin();
		for(; tcdSignalIterator != listTcdSignal.end(); tcdSignalIterator++) {
			if(tcdSignalIterator->deviceId == tcmIterator->deviceId &&
				tcdSignalIterator->io == tcmIterator->io){
				break;
			}
		}

		// �V�O�i�������쐬
		char buffer[16];
		unsigned long fixedTime = getFixedTime(minTime, tcmIterator->time);
		// ���߂̎��ԂƓ����Ȃ�M�����������Ă��邱�Ƃ��킩��悤���炷
		if(fixedTime <= tcdSignalIterator->LastTime) {
			fixedTime = tcdSignalIterator->LastTime + collisionDeleyTime;
		}

		sprintf_s(buffer, 16, "%d", fixedTime);
		std::string signal;

		if(tcmIterator->onoff > 0) {
			signal = "H=";
		}
		else if (tcmIterator->onoff == 0) {
			signal = "Z=";
		}
		else {
			signal = "L=";
		}
		signal.append(buffer);


		tcdSignalIterator->signal.push_back(signal);
		tcdSignalIterator->lastOnOff = tcmIterator->onoff;
		tcdSignalIterator->LastTime = fixedTime;
	}

	return listTcdSignal;
}

// tcmlog����tcd�t�@�C�����쐬���邽�߂ɕK�v�ȏ��𐶐�����B
std::list<TCD_BUS> tcm2tcdBus(std::list<TCMLOG>listTcm)
{
	std::list<TCD_BUS> listTcdBus; // TCD�V�O�i����

	std::list<TCMLOG>::iterator tcmIterator;
	unsigned long minTime = getMinTime(listTcm);
	std::list<TCD_BUS>::iterator tcdBusIterator;

	// TCD�V�O�i�����ɕϊ�
	tcmIterator = listTcm.begin();
	tcmIterator++; // EngineLogViewer�͏��߂̒l�𖳎����Ă���̂ł���ɂ��킹��B
	for(; tcmIterator != listTcm.end(); tcmIterator++){
		// IO=0����val=0�łȂ��Ƃ��������o
		if(!(tcmIterator->io==0 && tcmIterator->value==0)){

			// �f�o�C�X��񂪖��i�[�̏ꍇ�B�V���ɒǉ�����
			// �f�o�C�X��񂪂��łɃ��X�g�̒��ɂ��邩
			for(tcdBusIterator=listTcdBus.begin(); tcdBusIterator!=listTcdBus.end(); tcdBusIterator++) {
				if(tcdBusIterator->deviceId == tcmIterator->deviceId &&
				   tcdBusIterator->io == tcmIterator->io){
						break;
				}
			}
			// �Ȃ��ꍇ�͒ǉ�
			if (tcdBusIterator == listTcdBus.end()) {
				// �f�o�C�X���擾
				std::string deviceName = getDeviceName(tcmIterator->deviceId);
				std::list<std::string> listBus; // �o�X���̃��X�g
				TCD_BUS newDevice = {tcmIterator->deviceId, tcmIterator->io, tcmIterator->value, 0, listBus, deviceName};
				listTcdBus.push_back(newDevice);
			}

			// �o�X����ǉ����ׂ��f�o�C�X��������
			for(tcdBusIterator=listTcdBus.begin(); tcdBusIterator!=listTcdBus.end(); tcdBusIterator++) {
				if((tcdBusIterator->deviceId == tcmIterator->deviceId) &&
				   (tcdBusIterator->io == tcmIterator->io)) {
						break;
				}
			}

			// �o�X�����쐬
			char buffer[16];
			unsigned long fixedTime = getFixedTime(minTime, tcmIterator->time);
			// ���߂̎��ԂƓ����Ȃ�M�����������Ă��邱�Ƃ��킩��悤���炷
			if(fixedTime <= tcdBusIterator->LastTime) {
				fixedTime = tcdBusIterator->LastTime + collisionDeleyTime;
			}

			sprintf_s(buffer, 16, "%d=%d", tcmIterator->value, fixedTime);

			tcdBusIterator->bus.push_back(buffer);
			tcdBusIterator->lastValue = tcmIterator->value;
			tcdBusIterator->LastTime = fixedTime;
		}
	}

	return listTcdBus;
}

// tcd�t�@�C���̃w�b�_������������
void writeTcdHeader(FILE* wstream)
{
	// �w�b�_��
	fprintf(wstream, "$version, 2\n");
	fprintf(wstream, "$timeunit, ms\n");
	fprintf(wstream, "$timeperdot, 10.000000\n");
	fprintf(wstream, "$grid, 0.00001\n");
	fprintf(wstream, "$edgeangle, 90\n");
	fprintf(wstream, "\n\n");
}

// tcd�t�@�C����$signal�Q����������
void writeTcdSignals(FILE* wstream, std::list<TCMLOG>listTcm)
{
	std::list<TCD_SIGNAL> listTcdSignal = tcm2tcdSignal(listTcm);
	unsigned long minTime = getMinTime(listTcm);
	unsigned long maxTime = getMaxTime(listTcm);

	// TCM���O�ɏo�Ă������Ԃŏ�������
	if(sortDevice == 0) {
		std::list<TCD_SIGNAL>::iterator tcdSignalIterator;
		for(tcdSignalIterator = listTcdSignal.begin(); tcdSignalIterator != listTcdSignal.end(); tcdSignalIterator++){
			writeTcdSignal(wstream, *tcdSignalIterator, getMinTime(listTcm),getMaxTime(listTcm));
		}
		return;
	}

	// ini�t�@�C���̃f�o�C�X����`���ɏ�������
	// ini�t�@�C������f�o�C�X����`�����擾����
	std::list<std::string> sortOderList = getSortOderList();

	std::list<std::string>::iterator sortOderIterator = sortOderList.begin();
	for(; sortOderIterator!=sortOderList.end(); sortOderIterator++){

		// ��v������̂�T���Ēǉ����Ă���
		std::list<TCD_SIGNAL>::iterator tcdSignalIterator = listTcdSignal.begin();
		for(; tcdSignalIterator!=listTcdSignal.end(); tcdSignalIterator++){
			char devId[6];
			int val = std::stoi(tcdSignalIterator->deviceId, nullptr, 16);
			sprintf_s(devId, 6, "%05x", val & 0xfffff);
			if(sortOderIterator->compare(devId) == 0) {
				writeTcdSignal(wstream, *tcdSignalIterator, minTime, maxTime);
			}
		}
	}
}

// tcd�t�@�C����$signal������������
void writeTcdSignal(FILE* wstream, TCD_SIGNAL tcdSignal, unsigned long minTime, unsigned long maxTime)
{
	// 1�s��
	std::string io;
	if ( tcdSignal.io > 0) {
		io = "OUT";
	}
	else{
		io = "IN";
	}

	fprintf(wstream, "$signal,\t%s,\t%s, 90\n", tcdSignal.deviceName.c_str(), io.c_str());

	std::list<std::string>::iterator signalIterator;
	std::list<std::string> signalList =  tcdSignal.signal;

	// 2�s��
	for(signalIterator = signalList.begin(); signalIterator !=  signalList.end(); signalIterator++){
		fprintf(wstream, "\t%s,", signalIterator->c_str() );
	}

	char maxTimeBuffer[16];
	sprintf_s(maxTimeBuffer, 16, "%d", (maxTime - minTime + 40) * msPerClock);
	std::string signal;

	if(tcdSignal.lastOnOff > 0) {
		signal = "H=";
	}
	else if (tcdSignal.lastOnOff == 0) {
		signal = "Z=";
	}
	else {
		signal = "L=";
	}
	signal.append(maxTimeBuffer);
	fprintf(wstream, "\t%s,", signal.c_str() );

	fprintf(wstream, "\n");
}


// tcd�t�@�C����$bus������������
void writeTcdBuses(FILE* wstream, std::list<TCMLOG>listTcm)
{
	std::list<TCD_BUS> listTcdBus = tcm2tcdBus(listTcm);
	unsigned long minTime = getMinTime(listTcm);
	unsigned long maxTime = getMaxTime(listTcm);

	// TCM���O�ɏo�Ă������Ԃŏ�������
	if(sortDevice == 0) {
		std::list<TCD_BUS>::iterator tcdBusIterator;
		for(tcdBusIterator = listTcdBus.begin(); tcdBusIterator != listTcdBus.end(); tcdBusIterator++){
			writeTcdBus(wstream, *tcdBusIterator, minTime, maxTime);
		}
		return ;
	}

	// ini�t�@�C���̃f�o�C�X����`���ɏ�������
	// ini�t�@�C������f�o�C�X����`�����擾����
	std::list<std::string> sortOderList = getSortOderList();

	std::list<std::string>::iterator sortOderIterator = sortOderList.begin();
	for(; sortOderIterator!=sortOderList.end(); sortOderIterator++){

		// ��v������̂�T���Ēǉ����Ă���
		std::list<TCD_BUS>::iterator tcdBusIterator = listTcdBus.begin();
		for(; tcdBusIterator!=listTcdBus.end(); tcdBusIterator++){

			char devId[6];
			int val = std::stoi(tcdBusIterator->deviceId, nullptr, 16);
			sprintf_s(devId, 6, "%05x", val & 0xfffff);
			if(sortOderIterator->compare(devId) == 0) {
				writeTcdBus(wstream, *tcdBusIterator, minTime, maxTime);
			}
		}
	}
}

void writeTcdBus(FILE* wstream, TCD_BUS tcdBus, unsigned long minTime, unsigned long maxTime)
{
	// 1�s��
	std::string io;
	if ( tcdBus.io > 0) {
		io = "OUT";
	}
	else{
		io = "IN";
	}
	fprintf(wstream, "$bus,\t\t%s �ݒ�l,\t%s, 16, 10, 90\n", tcdBus.deviceName.c_str(), io.c_str());

	// 2�s��
	std::list<std::string>::iterator busIterator;
	std::list<std::string> busList =  tcdBus.bus;

	fprintf(wstream, "\tZ=0,");
	for(busIterator = busList.begin(); busIterator !=  busList.end(); busIterator++){
		fprintf(wstream, "\t%s,", busIterator->c_str() );
	}

	char maxTimeBuffer[16];
	sprintf_s(maxTimeBuffer, 16, "%d=%d", tcdBus.lastValue, (maxTime - minTime + 40) * msPerClock);
	fprintf(wstream, "%s,", maxTimeBuffer);

	fprintf(wstream, "\n");
}

bool senarioSeparete(unsigned short id)
{
	if (unit == EUNIT_TYPE_ENGINE) {
		return SENARIO_SEPARETE(id);
	} else if (unit == EUNIT_TYPE_DPS_ENGINE) {
		return SENARIO_SEPARETE_DPS(id);
	} else if (unit == EUNIT_TYPE_DF) {
		return SENARIO_SEPARETE_DF(id);
	} else {
		return SENARIO_SEPARETE(id);
	}
}

bool senarioSave(unsigned short id)
{
	if (unit == EUNIT_TYPE_ENGINE) {
		return SENARIO_SAVE(id);
	} else if (unit == EUNIT_TYPE_DPS_ENGINE) {
		return SENARIO_SAVE_DPS(id);
	} else if (unit == EUNIT_TYPE_DF) {
		return SENARIO_SAVE_DF(id);
	} else {
		return SENARIO_SAVE(id);
	}
}

