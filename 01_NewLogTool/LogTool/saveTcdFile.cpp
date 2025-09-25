#include "stdafx.h"
#include "saveTcdFile.h"
#include "LogTool.h"

// グローバル変数
char *iniFile;				// Iniファイルパス
unsigned long msPerClock;			// クロックあたりの時間
unsigned long collisionDeleyTime;	// 直近の信号と時間が同じだった場合にずらす時間
bool isSynchroLogAndTcm = false;
int sortDevice;						// iniファイルに書かれている順にソートするか

void writeTcdFile(FILE* wstream, std::list<TCMLOG>listTcm, char *szIniFilePath)
{
	// 書き込みべきログがなければ何もしない
	if (listTcm.size() == 0) {
		return;
	}

	iniFile = szIniFilePath;
	writeTcdHeader(wstream);
	writeTcdSignals(wstream, listTcm);
	writeTcdBuses(wstream, listTcm);
}


// iniファイルからデバイス名定義順を取得する
std::list<std::string> getSortOderList()
{
	std::list<std::string> sortOrderList;
	void* wkBuff = NULL;
	TCHAR* devcd_key;

	// iniファイル読み込み
	int devcd_key_size = GetPrivateProfileInt(_T("TCD_SETTING"), _T("WK_BUFF_SIZE"), 0, iniFile);
	if (devcd_key_size < 2) {
		devcd_key_size = 4096;	//データ取れない、0が指定されているなど対策
	}
	wkBuff = malloc(devcd_key_size);
	devcd_key = (TCHAR*)wkBuff;
	GetPrivateProfileString(_T("DEVICE_NAME"), NULL, _T(""), devcd_key, devcd_key_size, iniFile);	// 戻り値がdevcd_key_size-2なら失敗している可能性あり

	// 取得したキー名をリストに登録する
	while (*devcd_key != NULL) {
		sortOrderList.push_back(devcd_key);
		devcd_key += lstrlen(devcd_key) + 1;	// ポインタを 文字数＋NULL の分だけ進める
	}

	free(wkBuff);

	return sortOrderList;
}

// デバイスIDからデバイス名を取得する
std::string getDeviceName(std::string deviceId)
{
	// iniファイルから読み込み
	TCHAR readBuf[256];
	char devId[6];
	int val = std::stoi(deviceId, nullptr, 16);

	sprintf_s(devId, 6, "%05x", val & 0xfffff);
	GetPrivateProfileString(_T("DEVICE_NAME"), devId, devId, readBuf, sizeof(readBuf) - 5, iniFile);
	// シナリオはパラメータ部を別idとする
	if (senarioSave(val)) {
		sprintf_s(readBuf, sizeof(readBuf), "%s[%02d]", readBuf, (val >> 20));
	}
	return readBuf;
}

// TCMログから時間の最小値取得
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

// TCMログから時間の最大値取得
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

// タイムチャート上に表示する時間を取得
unsigned long getFixedTime(unsigned long minTime, unsigned long time){
	unsigned long fixedTime;

	fixedTime = (time - minTime) * msPerClock;

	return fixedTime;
}

// tcmlogからtcdファイルを作成するために必要な情報を生成する。
std::list<TCD_SIGNAL> tcm2tcdSignal(std::list<TCMLOG>listTcm)
{
	std::list<TCD_SIGNAL> listTcdSignal; // TCDシグナル部

	std::list<TCMLOG>::iterator tcmIterator;
	unsigned long minTime = getMinTime(listTcm);
	std::list<TCD_SIGNAL>::iterator tcdSignalIterator;

	// TCDシグナル部に変換
	tcmIterator = listTcm.begin();
	tcmIterator++; // EngineLogViewerは初めの値を無視しているのでそれにあわせる。
	for(; tcmIterator != listTcm.end(); tcmIterator++){

		// デバイス情報が未格納の場合。新たに追加する
		// デバイス情報がすでにリストの中にあるか
		for(tcdSignalIterator = listTcdSignal.begin(); tcdSignalIterator != listTcdSignal.end(); tcdSignalIterator++) {
			if(tcdSignalIterator->deviceId == tcmIterator->deviceId &&
				tcdSignalIterator->io == tcmIterator->io){
				break;
			}
		}
		// ない場合は追加
		if (tcdSignalIterator == listTcdSignal.end()) {
			// デバイス名取得
			std::string deviceName = getDeviceName(tcmIterator->deviceId);

			std::list<std::string> listSignal; // シグナルのリスト
			std::string initialValue = "Z";
			listSignal.push_back(initialValue);
			TCD_SIGNAL newDevice = {tcmIterator->deviceId, tcmIterator->io, tcmIterator->onoff, 0, listSignal, deviceName};

			listTcdSignal.push_back(newDevice);
		}

		// にシグナル情報を追加すべきデバイス情報を検索
		tcdSignalIterator = listTcdSignal.begin();
		for(; tcdSignalIterator != listTcdSignal.end(); tcdSignalIterator++) {
			if(tcdSignalIterator->deviceId == tcmIterator->deviceId &&
				tcdSignalIterator->io == tcmIterator->io){
				break;
			}
		}

		// シグナル情報を作成
		char buffer[16];
		unsigned long fixedTime = getFixedTime(minTime, tcmIterator->time);
		// 直近の時間と同じなら信号が発生していることがわかるようずらす
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

// tcmlogからtcdファイルを作成するために必要な情報を生成する。
std::list<TCD_BUS> tcm2tcdBus(std::list<TCMLOG>listTcm)
{
	std::list<TCD_BUS> listTcdBus; // TCDシグナル部

	std::list<TCMLOG>::iterator tcmIterator;
	unsigned long minTime = getMinTime(listTcm);
	std::list<TCD_BUS>::iterator tcdBusIterator;

	// TCDシグナル部に変換
	tcmIterator = listTcm.begin();
	tcmIterator++; // EngineLogViewerは初めの値を無視しているのでそれにあわせる。
	for(; tcmIterator != listTcm.end(); tcmIterator++){
		// IO=0かつval=0でないときだけ抽出
		if(!(tcmIterator->io==0 && tcmIterator->value==0)){

			// デバイス情報が未格納の場合。新たに追加する
			// デバイス情報がすでにリストの中にあるか
			for(tcdBusIterator=listTcdBus.begin(); tcdBusIterator!=listTcdBus.end(); tcdBusIterator++) {
				if(tcdBusIterator->deviceId == tcmIterator->deviceId &&
				   tcdBusIterator->io == tcmIterator->io){
						break;
				}
			}
			// ない場合は追加
			if (tcdBusIterator == listTcdBus.end()) {
				// デバイス名取得
				std::string deviceName = getDeviceName(tcmIterator->deviceId);
				std::list<std::string> listBus; // バス情報のリスト
				TCD_BUS newDevice = {tcmIterator->deviceId, tcmIterator->io, tcmIterator->value, 0, listBus, deviceName};
				listTcdBus.push_back(newDevice);
			}

			// バス情報を追加すべきデバイス情報を検索
			for(tcdBusIterator=listTcdBus.begin(); tcdBusIterator!=listTcdBus.end(); tcdBusIterator++) {
				if((tcdBusIterator->deviceId == tcmIterator->deviceId) &&
				   (tcdBusIterator->io == tcmIterator->io)) {
						break;
				}
			}

			// バス情報を作成
			char buffer[16];
			unsigned long fixedTime = getFixedTime(minTime, tcmIterator->time);
			// 直近の時間と同じなら信号が発生していることがわかるようずらす
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

// tcdファイルのヘッダ部を書き込む
void writeTcdHeader(FILE* wstream)
{
	// ヘッダ部
	fprintf(wstream, "$version, 2\n");
	fprintf(wstream, "$timeunit, ms\n");
	fprintf(wstream, "$timeperdot, 10.000000\n");
	fprintf(wstream, "$grid, 0.00001\n");
	fprintf(wstream, "$edgeangle, 90\n");
	fprintf(wstream, "\n\n");
}

// tcdファイルの$signal群を書き込む
void writeTcdSignals(FILE* wstream, std::list<TCMLOG>listTcm)
{
	std::list<TCD_SIGNAL> listTcdSignal = tcm2tcdSignal(listTcm);
	unsigned long minTime = getMinTime(listTcm);
	unsigned long maxTime = getMaxTime(listTcm);

	// TCMログに出てきた順番で書き込む
	if(sortDevice == 0) {
		std::list<TCD_SIGNAL>::iterator tcdSignalIterator;
		for(tcdSignalIterator = listTcdSignal.begin(); tcdSignalIterator != listTcdSignal.end(); tcdSignalIterator++){
			writeTcdSignal(wstream, *tcdSignalIterator, getMinTime(listTcm),getMaxTime(listTcm));
		}
		return;
	}

	// iniファイルのデバイス名定義順に書き込む
	// iniファイルからデバイス名定義順を取得する
	std::list<std::string> sortOderList = getSortOderList();

	std::list<std::string>::iterator sortOderIterator = sortOderList.begin();
	for(; sortOderIterator!=sortOderList.end(); sortOderIterator++){

		// 一致するものを探して追加していく
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

// tcdファイルの$signal部を書き込む
void writeTcdSignal(FILE* wstream, TCD_SIGNAL tcdSignal, unsigned long minTime, unsigned long maxTime)
{
	// 1行目
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

	// 2行目
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


// tcdファイルの$bus部を書き込む
void writeTcdBuses(FILE* wstream, std::list<TCMLOG>listTcm)
{
	std::list<TCD_BUS> listTcdBus = tcm2tcdBus(listTcm);
	unsigned long minTime = getMinTime(listTcm);
	unsigned long maxTime = getMaxTime(listTcm);

	// TCMログに出てきた順番で書き込む
	if(sortDevice == 0) {
		std::list<TCD_BUS>::iterator tcdBusIterator;
		for(tcdBusIterator = listTcdBus.begin(); tcdBusIterator != listTcdBus.end(); tcdBusIterator++){
			writeTcdBus(wstream, *tcdBusIterator, minTime, maxTime);
		}
		return ;
	}

	// iniファイルのデバイス名定義順に書き込む
	// iniファイルからデバイス名定義順を取得する
	std::list<std::string> sortOderList = getSortOderList();

	std::list<std::string>::iterator sortOderIterator = sortOderList.begin();
	for(; sortOderIterator!=sortOderList.end(); sortOderIterator++){

		// 一致するものを探して追加していく
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
	// 1行目
	std::string io;
	if ( tcdBus.io > 0) {
		io = "OUT";
	}
	else{
		io = "IN";
	}
	fprintf(wstream, "$bus,\t\t%s 設定値,\t%s, 16, 10, 90\n", tcdBus.deviceName.c_str(), io.c_str());

	// 2行目
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

