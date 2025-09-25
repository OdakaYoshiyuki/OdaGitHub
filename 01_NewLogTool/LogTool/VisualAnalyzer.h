#ifndef VisualAnalyzer_h
#define VisualAnalyzer_h 1

#define TIMER_ID_1			1
#define TIMER_ID_2			2
#define TIMER_ID_3			3
#define TIMER_ID_4			4
#define WM_USER_REMTIME		1
#define WINDOW_WIDTH_VA		800
#define WINDOW_HEIGTH_VA	520
#define WINDOW_MDCH_W		1500
#define WINDOW_MDCH_H		900

#define ANALYZE_POINT_NUM	500
#define ANALYZE_BRANCH_NUM	100
#define POSITION_SWITCH_NUM	10
#define DEVICE_NUM			300
#define DEVICE_OPE_NUM		30
#define MOTION_PAGE_NUM		20
#define MOTION_SUB_NUM		10
#define MOTION_NUM			MOTION_PAGE_NUM + MOTION_SUB_NUM
#define HISTORY_BUF_NUM		30
#define MOTION_TIME			4
#define START_POINT_NUM		10
#define STR_LEN_MAX			100
#define FAIL_SAFE_MAX		1000
#define OPE_CHECK_NUM		2
#define START_PART_NUM		2

enum ePOINT_TYPE {
	 EPOINT_TYPE_NONE
	,EPOINT_TYPE_NEW_POINT
	,EPOINT_TYPE_ADD_POINT
	,EPOINT_TYPE_EDIT_POINT
	,EPOINT_TYPE_CONNECT_POINT
	,EPOINT_TYPE_SPLIT
	,EPOINT_TYPE_BRANCH
	,EPOINT_TYPE_SWITCH_BACK
	,EPOINT_TYPE_GROUPING
	,EPOINT_TYPE_ADD_START_POINT
	,EPOINT_TYPE_SINGLE_ROLLER
	,EPOINT_TYPE_PAIR_ROLLER
	,EPOINT_TYPE_SENSOR
	,EPOINT_TYPE_SOLENOID
	,EPOINT_TYPE_SIGNAL
	,EPOINT_TYPE_NONE_DEVICE
	,EPOINT_TYPE_DVICE_VIEW
};

enum eEDIT_STATE {
	 EEDIT_STATE_IDLE
	,EEDIT_STATE_SELECT
	,EEDIT_STATE_EDIT
};

enum eEDGE_POINT{
	 EEDGE_TOP
	,EEDGE_BOTTOM
};

enum eVACLICK_STATE {
	 EVA_MOUSE_UP
	,EVA_MOUSE_DOWN
};

enum eDEVICE_TYPE{
	 EDEV_SINGLE_ROLLER
	,EDEV_PAIR_ROLLER
	,EDEV_SOLENOID
	,EDEV_SENSOR
	,EDEV_SIGNAL
	,EDEV_NONE_DEVICE
};

enum eDEVICE_EDIT_TYPE{
	 EDEVICE_NONE
	,EDEVICE_ADD
	,EDEVICE_EDIT
	,EDEVICE_VIEW
};

enum eDRIVE_TYPE {
	 EDRIVE_TYPE_MOTOR
	,EDRIVE_TYPE_CLUCH
};

enum ePRIFILE_STATE {
	 EPRIFILE_IDLE
	,EPRIFILE_PAUSE
	,EPRIFILE_ACTIVE
	,EPRIFILE_WAIT
	,EPRIFILE_SPLIT
};

enum eDIRECTION {
	 ENONE_DIRECTION = 0
	,EFOWARD = 1
	,EREVERSE = -1
};

enum eLOG_READ_STATE {
	 ELOG_READ_NORMAL
	,ELOG_READ_ACTIVE
	,ELOG_READ_WAIT_TIMER
	,ELOG_READ_PAUSE
	,ELOG_READ_STOP
};

enum eDEVID_TYPE {
	 EDEVID_NONE
	,EDEVID_DEV
	,EDEVID_SOUCE
};

enum eCHECK_TYPE {
	 ECHECK_NONE
	,ECHECK_VALUE
	,ECHECK_DIR
};

enum eSTART_PART_EDIT{
	 ESTART_PART_NONE
	,ESTART_PART_ADD
	,ESTART_PART_EDIT
	,ESTART_PART_DELETE
};

enum eVALOG_TYPE{
	 EVALOG_NONE
	,EVALOG_INPUT
	,EVALOG_OUTPUT
	,EVALOG_START
};

enum ePROF_TYPE {
	 EPROF_NORMAL
	,EPROF_SPLIT
};

enum eTOOLBAR_VA_MENU {
	ECOMMAND_VA_SCALEUP = 1,	//ägëÂ
	ECOMMAND_VA_SCALEDOWN,		//èkè¨
	ECOMMAND_VA_PLAY,			//çƒê∂
	ECOMMAND_VA_PAUSE,			//àÍéûí‚é~
	ECOMMAND_VA_STOP,			//í‚é~
	ECOMMAND_VA_FRAME,			//ÉRÉ}ëóÇË

	ECOMMAND_VA_1,			
	ECOMMAND_VA_2,			
	ECOMMAND_VA_3,			
	ECOMMAND_VA_4,			
	ECOMMAND_VA_5,			
};

struct ANALYZE_POINT {
	double X;
	double Y;
	signed long texPostX;
	signed long texPostY;
	signed long texPostDisX;
	signed long texPostDisY;
	double distance;
	double distanceX;
	double distanceY;
	double dispDistance;
	unsigned short prePoint;
	unsigned short nextPoint;
	bool isConnect;
	bool isGroupe;
	bool isHideLength;
};

struct ANALYZE_BRANCH {
	unsigned short point;
	unsigned short nextPoint1;
	unsigned short nextPoint2;
};

struct POSITION_SWITCH {
	unsigned short point;
	unsigned short nextPoint;
};

struct CURRENT_BRANCH {
	unsigned short point;
	unsigned short nextPoint;
};

struct POSITION_INFO {
	unsigned short point;
	unsigned short nextPoint;
	double dis;
	eDIRECTION dir;
};

struct POSITION_HISTORY {
	unsigned short history[HISTORY_BUF_NUM];
	unsigned short topPos;
	unsigned short bottomPos;
	double distance[HISTORY_BUF_NUM];
};

struct DEVICE_INFO {
	unsigned short point;
	double disX;
	double disY;
	unsigned short size;
	char name[STR_LEN_MAX];
	signed short value;
	signed short preValue;
	signed short driveSouceDev;
	eDEVICE_TYPE type;
	eDRIVE_TYPE driveType;
	eDIRECTION dir;
	eDIRECTION preDir;
	bool notFirst;
};

struct DRIVE_SOUCE {
	char name[STR_LEN_MAX];
	unsigned short dev;
	signed short value;
	signed short preValue;
	eDIRECTION dir;
	eDIRECTION preDir;
	bool notFirst;
};

struct DEVICE_ID_INFO {
	eDEVID_TYPE type;
	signed short id;
};

struct DEVICE_OPERATION {
	char name[STR_LEN_MAX];
	DEVICE_ID_INFO dev[OPE_CHECK_NUM];
	LRESULT devSel[OPE_CHECK_NUM];
	char condition[OPE_CHECK_NUM][STR_LEN_MAX];
	char opeDevName[STR_LEN_MAX];
	signed short checkValue[OPE_CHECK_NUM];
	eCHECK_TYPE checkType[OPE_CHECK_NUM];
	eCOMPARISON_TYPE ope[OPE_CHECK_NUM];
	unsigned short opeValue;
	signed char opeIdDir;
	char opeValueStr[STR_LEN_MAX];
	char opeIdDirStr[STR_LEN_MAX];
	char operationFomura[STR_LEN_MAX];
	LRESULT opeDevSel;
	unsigned short stepNum;
	unsigned short stepTime;
	unsigned short stepCount;
	signed short stepValue;
	signed short stepNowValue;
	signed char stepDir;
	unsigned short remainingTime;
	unsigned long tentativeTime;
	bool timeWait;
};

struct START_INFO {
	unsigned short point;
	unsigned char startPart;
	unsigned char latestId;
	char name[STR_LEN_MAX];
	double X;
	double Y;
};

struct PRIFILE_INFO {
	double length;
	unsigned short startPart;
	unsigned char waitId;
	ePRIFILE_STATE state;
	ePROF_TYPE prfType;
	unsigned short mId;
};

struct START_PART{
	char name[STR_LEN_MAX];
	unsigned char partId;
};

struct CONVERT_VALOG{
	char name[STR_LEN_MAX];
	signed long val;
	signed char idDir;
	eVALOG_TYPE type;
	unsigned char startPart;
};
#endif
