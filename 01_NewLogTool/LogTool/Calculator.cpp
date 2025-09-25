#include "stdafx.h"
#include <stdio.h>
#include "LogTool.h"


#define SPLIT_NUM		8
#define CALC_PARAM_NUM	100
#define STR_MAX_LEN		256
#define CALC_PARAM_LEN	20

double calculation(double param1, double param2, char operatorChr);
double numericalCalculation(char *formula);
double* dataConversion(char* dataStr);
void fillInSpace(char *str);
void strInitialize(char *str, size_t size);
void strChange(char *str, char *insertStr, size_t size);
void formulaConversion(char *formula);
void calculateFromLeft(char *formula);
void priorityCalculation(char *formula);
bool firstCalculation(char *formula);
bool isValue(char c, bool isFirst = false);
bool isOperator(char c);
bool isPriorityOperator(char c);
eCOMPARISON_TYPE isComparisoOperator(char *c);
bool comparison(double tada1, double data2, eCOMPARISON_TYPE opeId);

struct paramStrData {
	char str[CALC_PARAM_LEN];
	double data;
};

paramStrData calcParam[CALC_PARAM_NUM];


void calculateData(char *buffer)
{
	char* paramStr;
	char* formula;
	char* context; //strtok_sの内部で使用
	double* param;

	fillInSpace(buffer); // スペース削除
	if (buffer[0] == NULL) {
		return;
	}
	paramStr = strtok_s(buffer, "=", &context);
	if (paramStr != NULL) {
		formula = strtok_s(NULL, "=", &context);
	} else {
		return;
	}
	param = dataConversion(paramStr);
	if (formula != NULL) {
		*param = numericalCalculation(formula);
	}
}

double numericalCalculation(char *formula)
{
	double result;

	// 変数を数値に変換
	formulaConversion(formula);

	//()内の計算
	while (firstCalculation(formula)) {}

	//掛け算、割り算
	priorityCalculation(formula);

	//左から準に計算
	calculateFromLeft(formula);

	//値に変換
	result = atof(formula);

	return result;
}

//計算式の中の変数を数値に変換
void formulaConversion(char *formula)
{
	char *endp;
	char formulaCopy[STR_MAX_LEN];
	char paramStr[STR_MAX_LEN];
	char valueStr[STR_MAX_LEN];
	double* value;
	unsigned char paramStrPos = 0;
	size_t pos = 0;
	size_t formulaLen = strlen(formula);
	size_t paramStrLen = 0;
	size_t valueLen;
	size_t newLen;

	strInitialize(paramStr, sizeof(paramStr));
	for (pos = 0; pos <= formulaLen; pos++) {
		if (isValue(formula[pos])) {
			paramStr[paramStrPos++] = formula[pos];
			paramStrLen++;
		} else {
			if (paramStrLen != 0) {
				strtod(paramStr, &endp);
				if (*endp != '\0') {
					strcpy_s(formulaCopy, &formula[pos]);
					value = dataConversion(paramStr);
					valueLen = sprintf_s(valueStr, "%f",*value);
					strChange(&formula[pos - paramStrLen], valueStr, valueLen);
					strChange(&formula[pos - paramStrLen + valueLen], formulaCopy, strlen(formulaCopy));
					newLen = formulaLen - paramStrLen + valueLen;
					if (newLen < formulaLen) {
						strInitialize(&formula[newLen], formulaLen - newLen);
					}
					formulaLen = newLen;
					pos = pos - paramStrLen + valueLen;
				}
				paramStrLen = 0;
				paramStrPos = 0;
				strInitialize(paramStr, sizeof(paramStr));
			}
		}
	}
}


//()内の計算
bool firstCalculation(char *formula)
{
	char subFormulaStr[STR_MAX_LEN];
	char formulaCopy[STR_MAX_LEN];
	unsigned char subFormulaStrPos = 0;
	bool flag = false;
	size_t pos = 0;
	size_t startPos = 0;
	size_t formulaLen = strlen(formula);


	strInitialize(subFormulaStr, sizeof(subFormulaStr));
	for (pos = 0; pos < formulaLen; pos++) {
		if (formula[pos] == '(') {
			startPos = pos;
			subFormulaStrPos = 0;
			strInitialize(subFormulaStr, sizeof(subFormulaStr));
			flag = true;
		} else if (formula[pos] == ')') {
			strcpy_s(formulaCopy, &formula[pos + 1]);
			priorityCalculation(subFormulaStr);
			calculateFromLeft(subFormulaStr);
			strChange(&formula[startPos], subFormulaStr, strlen(subFormulaStr));
			strChange(&formula[startPos + strlen(subFormulaStr)], formulaCopy, strlen(formulaCopy));
			return true;
		} else if (flag == true) {
			subFormulaStr[subFormulaStrPos++] = formula[pos];
		} else {
		}
	}
	return false;
}


//掛け算割り算を先に計算
void priorityCalculation(char *formula)
{
	char valueStr[STR_MAX_LEN];
	char formulaCopy[STR_MAX_LEN];
	unsigned char valueStrPos = 0; 
	char oper = ' ';
	double param[2];
	double calcresult;
	size_t formulaLen = strlen(formula);
	size_t pos = 0;
	size_t valueLen;
	size_t newLen;
	size_t param1Pos = 0;

	strInitialize(valueStr, sizeof(valueStr));
	for (pos = 0; pos <= formulaLen; pos++) {
		if (isValue(formula[pos], ((pos != 0) && (valueStrPos == 0)))) {
			valueStr[valueStrPos++] = formula[pos];
		} else {
			if (oper != ' ') {
				strcpy_s(formulaCopy, &formula[pos]);
				param[1] = atof(valueStr);
				calcresult = calculation(param[0], param[1], oper);
				valueLen = sprintf_s(valueStr, "%f",calcresult);
				strChange(&formula[param1Pos], valueStr, valueLen);
				strChange(&formula[param1Pos + valueLen], formulaCopy, strlen(formulaCopy));
				newLen = param1Pos + valueLen + strlen(formulaCopy);
				if (newLen < formulaLen) {
					strInitialize(&formula[newLen], formulaLen - newLen);
				}
				formulaLen = strlen(formula);
				pos = param1Pos + valueLen;
				oper = ' ';
				param[0] = calcresult;
			}
			if (isPriorityOperator(formula[pos])) {
				param[0] = atof(valueStr);
				oper = formula[pos];
				param1Pos = pos - strlen(valueStr);
			}
			strInitialize(valueStr, sizeof(valueStr));
			valueStrPos = 0;
		}
	}
}

//左から順に計算
void calculateFromLeft(char *formula)
{
	size_t pos = 0;
	size_t formulaLen = strlen(formula);
	size_t valueLen;
	size_t param1Pos = 0;
	size_t newLen;
	char valueStr[STR_MAX_LEN];
	unsigned char valueStrPos = 0;
	double param[2] = {0};
	char oper = ' ';
	char formulaCopy[STR_MAX_LEN];
	double calcresult;

	strInitialize(valueStr, sizeof(valueStr));
	for (pos = 0; pos <= formulaLen; pos++) {
		if (isValue(formula[pos], ((pos != 0) && (valueStrPos == 0)))) {
			valueStr[valueStrPos++] = formula[pos];
		} else {
			if (oper != ' ') {
				strcpy_s(formulaCopy, &formula[pos]);
				param[1] = atof(valueStr);
				calcresult = calculation(param[0], param[1], oper);
				valueLen = sprintf_s(valueStr, "%f",calcresult);
				strChange(&formula[param1Pos], valueStr, valueLen);
				strChange(&formula[param1Pos + valueLen], formulaCopy, strlen(formulaCopy));
				newLen = valueLen + strlen(formulaCopy);
				if (newLen < formulaLen) {
					strInitialize(&formula[newLen], formulaLen - newLen);
				}
				formulaLen = newLen;
				pos = param1Pos + valueLen;
				param[0] = calcresult;
			} else {	
				if (valueStrPos != 0) {
					param[0] = atof(valueStr);
					param1Pos = pos - strlen(valueStr);
				}
			}
			if (isOperator(formula[pos])) {
				oper = formula[pos];
			}
			strInitialize(valueStr, sizeof(valueStr));
			valueStrPos = 0;
		}
	}
}

//文字列を数値に変換
double* dataConversion(char* dataStr)
{
	unsigned char i;
	size_t len;
	double* result = 0;

	for (i = 0; i < CALC_PARAM_NUM; i++) {
		if (calcParam[i].str[0] == NULL) {
			//存在しなければ新たに登録
			strcpy_s(calcParam[i].str, dataStr);
		}
		len = strlen(dataStr);
		if (len < strlen(calcParam[i].str)) {
			len = strlen(calcParam[i].str);
		}
		if(strncmp(dataStr, calcParam[i].str, len) == 0){
			result = &calcParam[i].data;
			break;
		}
	}
	return result;
}

//数値の計算
double calculation(double param1, double param2, char operatorChr)
{
	double result = 0;

	switch (operatorChr) {
		case '+':
			result = param1 + param2;
			break;
		case '-':
			result = param1 - param2;
			break;
		case '*':
			result = param1 * param2;
			break;
		case '/':
			if (param2 != 0) {
				result = param1 / param2;
			}
			break;
		case '%':
			if (param2 != 0) {
				result = (int)param1 % (int)param2;
			}
			break;
		default:
			break;
	}

	return result;
}

//文字列の初期化
void strInitialize(char *str, size_t size)
{
	unsigned short i;

	for (i = 0;i < size; i++) {
		str[i] = NULL;
	}
}
//文字列の変換
void strChange(char *str, char *insertStr, size_t size)
{
	unsigned short i;

	for (i = 0;i < size; i++) {
		str[i] = insertStr[i];
	}
	str[size] = NULL;
}
//数値かどうかの判定
bool isValue(char c, bool isFirst)
{
	unsigned char i;
	char delim[SPLIT_NUM+1] = "+-*/%()";

	if ((c == '-') && (isFirst)) {
		return true;
	}
	for (i = 0; i < SPLIT_NUM; i++) {
		if (c == delim[i]) {
			return false;
		}
	}
	return true;
}

//掛け算割り算演算子かどうかの判定
bool isPriorityOperator(char c)
{
	unsigned char i;
	char operatorChr[4] = "*/%";

	for (i = 0; i < 4; i++) {
		if (c == operatorChr[i]) {
			return true;
		}
	}
	return false;
}

//演算子かどうかの判定
bool isOperator(char c)
{
	unsigned char i;
	char operatorChr[6] = "+-*/%";

	for (i = 0; i < 5; i++) {
		if (c == operatorChr[i]) {
			return true;
		}
	}
	return false;
}

//スペースの削除
void fillInSpace(char *str)
{
	unsigned char i;
	for (i = 0;i < strlen(str); i++) {
		if (str[i] == ' ') {
			strChange(&str[i], &str[i+1],(strlen(str) - i - 1));
		}
	}
	str[i] = NULL;
}


bool judgement(char *formula)
{
	size_t pos = 0;
	size_t pos1 = 0; 
	size_t formulaLen = strlen(formula);
	char formula1[STR_MAX_LEN];
	char formula2[STR_MAX_LEN];
	double data1;
	double data2;
	eCOMPARISON_TYPE opeId;
	eCOMPARISON_TYPE ope = ECOMPARISON_NONE;
	unsigned char opeLen;
	bool flag = false;
	bool ret = true;

	fillInSpace(formula); // スペース削除
	for (pos = 0; pos <= formulaLen; pos++) {
		opeId = isComparisoOperator(&formula[pos]); 
		if (opeId != ECOMPARISON_NONE) {
			if (!flag) {
				strncpy_s(formula1, formula, pos);
				data1 = numericalCalculation(formula1);
				strInitialize(formula1, sizeof(formula1));
				pos1 = pos;
				ope = opeId;
				flag = true;
				if ((ope == ECOMPARISON_LESS_EQUAL) || (ope == ECOMPARISON_LARGER_EQUAL) || (ope == ECOMPARISON_NOT_EQUAL)) {
					pos++;
				}
			} else {
				if ((ope == ECOMPARISON_LESS_EQUAL) || (ope == ECOMPARISON_LARGER_EQUAL) || (ope == ECOMPARISON_NOT_EQUAL)) {
					opeLen = 2;
				} else {
					opeLen = 1;
				}
				strncpy_s(formula2, &formula[pos1 + opeLen], pos - pos1 - opeLen);
				data2 = numericalCalculation(formula2);
				if (!comparison(data1, data2, ope)) {
					ret = false;
				}
				data1 = data2;
				pos1 = pos;
				ope = opeId;
				strcpy_s(formula1, formula2);
				strInitialize(formula2, sizeof(formula2));
				if (opeId == ECOMPARISON_END) {
					strcpy_s(formula1, &formula[pos+1]);
					strChange(formula, formula1, strlen(formula1));
					numericalCalculation(formula);
					strInitialize(formula1, sizeof(formula2));
					break;
				}
			}
		}
	}
	return ret;
}

eCOMPARISON_TYPE isComparisoOperator(char *c)
{
	eCOMPARISON_TYPE ret = ECOMPARISON_NONE;

	switch (*c) {
		case '<':
			ret = ECOMPARISON_LESS;
			if (c[1] == '=') {
				ret = ECOMPARISON_LESS_EQUAL;
			}
			break;
		case '>':
			ret = ECOMPARISON_LARGER;
			if (c[1] == '=') {
				ret = ECOMPARISON_LARGER_EQUAL;
			}
			break;
		case '=':
			ret = ECOMPARISON_EQUAL;
			break;
		case ',':
			ret = ECOMPARISON_END;
			break;
		case '!':
			if (c[1] == '=') {
				ret = ECOMPARISON_NOT_EQUAL;
			}
			break;
		default:
			ret = ECOMPARISON_NONE;
			break;
	}
	return ret;
}

bool comparison(double data1, double data2, eCOMPARISON_TYPE opeId)
{
	bool ret = false;

	switch (opeId) {
		case ECOMPARISON_LESS:
			ret = (data1 < data2);
			break;
		case ECOMPARISON_LESS_EQUAL:
			ret = (data1 <= data2);
			break;
		case ECOMPARISON_EQUAL:
			ret = (data1 == data2);
			break;
		case ECOMPARISON_LARGER:
			ret = (data1 > data2);
			break;
		case ECOMPARISON_LARGER_EQUAL:
			ret = (data1 >= data2);
			break;
		case ECOMPARISON_NOT_EQUAL:
			ret = (data1 != data2);
			break;
		default:
			break;
	}
	return ret;
}

//変数として登録された文字列のみを数値に変換
void changeParamValue(char *buffer, unsigned char kind)
{
	char* context; //strtok_sの内部で使用
	char* str;
	unsigned char i;
	size_t len;
	char retStr[STR_MAX_LEN]="";

	if (buffer[0] == 0) {
		return;
	}

	str = strtok_s(buffer, " ", &context);
	while (str != NULL) {
		for (i = 0; i < CALC_PARAM_NUM; i++) {
			len = strlen(str);
			if (len < strlen(calcParam[i].str)) {
				len = strlen(calcParam[i].str);
			}
			if(strncmp(str, calcParam[i].str, len) == 0){
				char dummy[STR_MAX_LEN];
				if (kind == INTEGER_VALUE) {
					sprintf_s(dummy, "%d",(int)calcParam[i].data);
				} else {
					sprintf_s(dummy, "%lf",calcParam[i].data);
				}
				strcat_s(retStr, dummy);
				strcat_s(retStr, " ");
				break;
			} else if (calcParam[i].str[0] == NULL) {
				strcat_s(retStr, str);
				strcat_s(retStr, " ");
				break;
			} else {
			}
		}
		str = strtok_s(NULL, " ", &context);
	}
	len = strlen(retStr);
	if (len != 0) {
		retStr[len - 1] = NULL;
		strChange(buffer, retStr, strlen(retStr));
	}
}

