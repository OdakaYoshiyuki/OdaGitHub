//紙のサイズ用

#define PAPER_SIZE_NUM 54
#define MEDIA_TYPE_NUM 26
#define PAPER_WEIGTH_NUM 10
#define COLOR_MODE_NUM 2
#define PRINT_SPEED_NUM 5
#define PAPER_FEED_NUM 8
#define PAPER_SIDE_NUM 2

struct paperSize
{
	const char* paperSizeName;
	unsigned char paperSizeId;
};

struct mediaType
{
	const char* mediaTypeName;
	unsigned char mediaTypeId;
};

struct paperWeigth
{
	const char* paperWeigthName;
	unsigned char paperWeigthId;
};

struct colerMode
{
	const char* colorModeName;
	unsigned char colorModeId;
};

struct printSpeed
{
	const char* printSpeedName;
	unsigned char printSpeedId;
};

struct paperFeed
{
	const char* paperFeedName;
	unsigned char paperFeedId;
};
struct paperSide
{
	const char* paperSideName;
	unsigned char paperSideId;
};


extern struct paperSize dispPaperSize[PAPER_SIZE_NUM];
extern struct mediaType dispMediaType[MEDIA_TYPE_NUM];
extern struct paperWeigth dispPaperWeigth[PAPER_WEIGTH_NUM];
extern struct colerMode dispColorMode[COLOR_MODE_NUM];
extern struct printSpeed dispPrintSpeed[PRINT_SPEED_NUM];
extern struct paperFeed dispPaperFeed[PAPER_FEED_NUM];
extern struct paperSide dispPaperSide[PAPER_SIDE_NUM];
