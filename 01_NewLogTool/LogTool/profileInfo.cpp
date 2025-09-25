#include <stdio.h>
#include "stdafx.h"
#include "profileInfo.h"

paperSize dispPaperSize[PAPER_SIZE_NUM] = {
	{"カセットなし",			0x00},
	{"封筒Monarch",				0x01},
	{"Business",				0x02},
	{"封筒DL",					0x03},
	{"封筒C5",					0x04},
	{"7.25×10.5",				0x05},
	{"8.5×11",					0x06},
	{"8.5×14",					0x07},
	{"A4T",						0x08},
	{"B5T",						0x09},
	{"A3T",						0x0a},
	{"B4T",						0x0b},
	{"11×17",					0x0c},
	{"A5T",						0x0d},
	{"A6T",						0x0e},
	{"B6T",						0x0f},
	{"封筒 Comm#9",				0x10},
	{"封筒 Comm#6-3/4",			0x11},
	{"B5(ISO)",					0x12},
	{"CUSTOM",					0x13},
	{"封筒 C4",					0x1e},
	{"はがき(縦)",				0x1f},
	{"往復はがき(縦)148x200",	0x20},
	{"8.5x13(OficioⅡ)",		0x21},
	{"Special1(216x310)",		0x22},
	{"Special2(216x305)",		0x23},
	{"A3Wide",					0x24},
	{"LDGWide",					0x25},
	{"12x18",					0x26},
	{"8開",						0x27},
	{"16開T",					0x28},
	{"11x15(CPF)",				0x29},
	{"MexicanOficio(216x340)",	0x2a},
	{"13x19(13inch)",			0x2b},
	{"SRA3",					0x2c},
	{"封筒 角形2号",			0x2d},
	{"封筒 長形3号",			0x2e},
	{"封筒 洋長3号",			0x2f},
	{"封筒 長形4号",			0x30},
	{"13x19.2(inch)",			0x31},
	{"5.5x8.5",					0x32},
	{"FOLIO",					0x33},
	{"封筒 洋形2号",			0x34},
	{"封筒 洋形4号",			0x35},
	{"L size",					0x36},
	{"Name card",				0x37},
	{"Card",					0x38},
	{"11x8.5",					0x86},
	{"A4Y",						0x88},
	{"B5Y",						0x89},
	{"A5Y",						0x8d},
	{"16開Y",					0xa8},
	{"Statement横",				0xb2},
	{"カセットサイズエラー",	0xff}
};

mediaType dispMediaType[MEDIA_TYPE_NUM] = {
	{"Plain",				1},
	{"Transparency",		2},	
	{"Preprinted",			3},
	{"Labels",				4},
	{"Bond",				5},
	{"Recycled",			6},
	{"Vellum",				7},
	{"Rough紙",				8},
	{"Letterhead",			9},
	{"Color",				10},
	{"Prepunched",			11},
	{"Envelope",			12},
	{"Cardstock",			13},
	{"Coated",				14},
	{"2ndSide",				15},
	{"Thick",				16},
	{"HighQuality",			17},
	{"Index",				18},
	{"Custom1",				21},
	{"Custom2",				22},
	{"Custom3",				23},
	{"Custom4",				24},
	{"Custom5",				25},
	{"Custom6",				26},
	{"Custom7",				27},
	{"Custom8",				28}
};

paperWeigth dispPaperWeigth[PAPER_WEIGTH_NUM] = {
	{"Thin",		0},
	{"Normal1",		1},
	{"Normal2",		2},
	{"Normal3",		3},
	{"Heavy1",		4},
	{"Heavy2",		5},
	{"Heavy3",		6},
	{"Heavy4",		7},
	{"Heavy5",		8},
	{"ExtraHeavy",	9}
};

colerMode dispColorMode[COLOR_MODE_NUM] = {
	{"Mono",		0x01},
	{"Color",		0x0f}
};

printSpeed dispPrintSpeed[PRINT_SPEED_NUM] = {
	{"Speed Normal",		0},
	{"Speed 3/4",			1},
	{"Speed 1/2",			2},
	{"Speed 1/4",			3},
	{"Speed Full",			4},
};

paperFeed dispPaperFeed[PAPER_FEED_NUM] = {
	{"MPF",					0},
	{"Cass1",				1},
	{"Cass2",				2},
	{"Cass3",				3},
	{"Cass4",				4},
	{"Cass5",				5},
	{"Cass6",				6},
	{"Du",				 0xff},
};
paperSide dispPaperSide[PAPER_SIDE_NUM] = {
	{"1Side",				0},
	{"2Side",				1},
};
