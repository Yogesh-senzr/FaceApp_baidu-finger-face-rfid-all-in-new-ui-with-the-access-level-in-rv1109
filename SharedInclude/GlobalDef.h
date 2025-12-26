#ifndef GLOBALDEF_H
#define GLOBALDEF_H

#include <QtCore/QString>
#include <QtCore/QDateTime>
#include "BaiduFace/inc/interface/bface_types.h"


#define ISC_OK (0)
#define ISC_ERROR (-1)
#define ISC_DEVICE_CHECK_ERROR (-2)
#define ISC_DEVICE_CHECK_BYPASS (-3)
#define ISC_ERROR_EXIST (-4)
#define ISC_UPDATE_PERSON_NOT_EXIST (-5)
#define ISC_UPDATE_PERSON_DB_ERROR (-6)
#define ISC_NULL                 0L

#define ISC_TRUE (1)
#define ISC_FALSE (0)

#define DEVICE_MASTE          0
#define DEVICE_SLAVE          1

#define MAX_FACES (10)


#define FACE_CROP_PATH "/mnt/user/face_crop_image"
#define STRANGER (1)
#define NOT_STRANGER (2)
#define TOP_MESSAGE (1)
#define BOTTOM_MESSAGE (2)
#define ALARM_MESSAGE (3)
//#define FACE_FULL_COUNT	23000	//磁盘保留最大图片数,到最大值, 删除图片到 20000 条
//#define FACE_SHRINK_COUNT	20000	//磁盘保留最大图片数,到最大值, 删除图片到 20000 条

#define ZHANGJIAKOU //张家口后台启用
typedef enum _DOOR_OPEN_MODE
{
    ICCARD = 1,//刷卡
    SWIPING_FACE,//刷脸
    THERMOMETRY,//测温
    MASK,//口罩
    QRCODE,//二唯码
    IDCARD,//身份证
    PERSON_IDCARD,//人证比对
    QRCODE_LOCAL,//本地码,粤康证/深I您　
}DOOR_OPEN_MODE;

typedef enum _DEVICE_CALLBACK_TYPE
{
    DEVICE_CALLBACK_TYPE_STRANGER = 0,
    DEVICE_CALLBACK_TYPE_CARD_NUM,
    DEVICE_CALLBACK_TYPE_FACE_MATCH,
    DEVICE_CALLBACK_TYPE_FACETOOLS_CALLBACK,
    DEVICE_CALLBACK_TYPE_FACE_NOT_PERMISSION
} DEVICE_CALLBACK_TYPE;

#define FACEDB_REGIST_IMAGE "/mnt/user/facedb/img/"

/**
 * 摄像头支持的格式
 */
#define IF_PIXEL_FORMAT_YUV_SEMIPLANAR_420 (29)
#define IF_PIXEL_FORMAT_YVU_SEMIPLANAR_420 (26)
#define IF_PIXEL_FORMAT_YVU_SEMIPLANAR_422 (25)
#define IF_PIXEL_FORMAT_YUV_400 (40)

typedef struct
{
    char Key[64];
    char Value[64];
    void *Prev;
    void *Next;
} Config_S;

typedef enum _CORE_FACE_RECT_TYPE_EN
{
    CORE_FACE_RECT_TYPE_UNKNOW = 0,  //未知
    CORE_FACE_RECT_TYPE_MOVING, // 识别到人脸后，或跟随人脸移动状态
    CORE_FACE_RECT_TYPE_SEARCH,
    CORE_FACE_RECT_TYPE_REGIST,
    CORE_FACE_RECT_TYPE_POST,
    CORE_FACE_RECT_TYPE_MATCH, // 已匹配到人脸状态
    CORE_FACE_RECT_TYPE_DELETE,//删除人脸记录
} CORE_FACE_RECT_TYPE_EN;

typedef struct _CORE_FACE_RECT_S
{
    unsigned int nX;
    unsigned int nY;
    unsigned int nWidth;
    unsigned int nHeight;
    unsigned int nColor;
}CORE_FACE_RECT_S;

/**
 *  人脸数据结构体
 *
 *内部成员
 *	   unsigned char* pFaceFeature ：  人脸特征值，由算法库填入，默认是在系统启动后由数据入加载入此地址
 *	   unsigned int nFaceFeatureSize ：  pFaceFeature 大小
 *     CORE_FACE_RECT_S stFaceRect ：人脸具体坐标，对应于内存YUV数据坐标系，如根据isc_config.ini 配置，PREVIEW_YUV_SIZE = 576x1024 ，
 *     														stFaceRect就是对应在 576x1024内存YUV內人脸位置，GUI使用时，需要根据屏幕的具体分别率做转换
 *     CORE_FACE_RECT_TYPE_EN enFaceType ： 人脸类型，具体见上方描述
 *
 */
typedef struct _CORE_FACE_ATTR_INFO
{
    int gender;
    int age;
    float pose_pitch;
    float pose_yaw;
    float pose_roll;
    float quality;
    int liveness_ir;
    int image_color;
    int face_mask;     //大于0即为戴口罩，0不戴口罩
} CORE_FACE_ATTR_INFO;

/* R	R  R  R	 R	R  R  R  G  G  G  G  G  G  G  G  B  B  B  B  B  B  B  B */
#define		ASVL_PAF_RGB24_B8G8R8		0x201
/* X	X  X  X	 X	X  R  R  R  R  R  R  G  G  G  G  G  G  B  B  B  B  B  B */
#define		ASVL_PAF_RGB24_B6G6R6		0x202
/* X	X  X  X	 X	T  R  R  R  R  R  R  G  G  G  G  G  G  B  B  B  B  B  B */
#define		ASVL_PAF_RGB24_B6G6R6T		0x203
/* B  B  B  B  B  B  B  B  G  G  G  G  G  G  G  G  R	R  R  R	 R	R  R  R */
#define		ASVL_PAF_RGB24_R8G8B8		0x204
/* X	X  X  X	 X	X  B  B  B  B  B  B  G  G  G  G  G  G  R  R  R  R  R  R */
#define		ASVL_PAF_RGB24_R6G6B6		0x205

/* X	X  X  X	 X	X  X  X	 R	R  R  R	 R	R  R  R  G  G  G  G  G  G  G  G  B  B  B  B  B  B  B  B */
#define		ASVL_PAF_RGB32_B8G8R8		0x301
/* A	A  A  A	 A	A  A  A	 R	R  R  R	 R	R  R  R  G  G  G  G  G  G  G  G  B  B  B  B  B  B  B  B */
#define		ASVL_PAF_RGB32_B8G8R8A8		0x302
/* X	X  X  X	 X	X  X  X	 B  B  B  B  B  B  B  B  G  G  G  G  G  G  G  G  R	R  R  R	 R	R  R  R */
#define		ASVL_PAF_RGB32_R8G8B8		0x303
/* B    B  B  B  B  B  B  B  G  G  G  G  G  G  G  G  R  R  R  R  R  R  R  R  A	A  A  A  A	A  A  A */
#define		ASVL_PAF_RGB32_A8R8G8B8		0x304
/* A    A  A  A  A  A  A  A  B  B  B  B  B  B  B  B  G  G  G  G  G  G  G  G  R  R  R  R  R	R  R  R */
#define		ASVL_PAF_RGB32_R8G8B8A8		0x305

/*8 bit Y plane followed by 8 bit 2x2 subsampled UV planes*/
#define		ASVL_PAF_NV12				0x801
/*8 bit Y plane followed by 8 bit 2x2 subsampled VU planes*/
#define		ASVL_PAF_NV21				0x802
/*8 bit Y plane followed by 8 bit 2x2 subsampled U and V planes*/
#define		ASVL_PAF_I420				0x601
/*8 bit Y plane followed by 8 bit 1x2 subsampled U and V planes*/
#define		ASVL_PAF_I422V				0x602
/*8 bit Y plane followed by 8 bit 2x1 subsampled U and V planes*/
#define		ASVL_PAF_I422H				0x603
/*8 bit Y plane followed by 8 bit U and V planes*/
#define		ASVL_PAF_I444				0x604
/*8 bit Y plane followed by 8 bit 2x2 subsampled V and U planes*/
#define		ASVL_PAF_YV12				0x605
/*8 bit Y plane followed by 8 bit 1x2 subsampled V and U planes*/
#define		ASVL_PAF_YV16V				0x606
/*8 bit Y plane followed by 8 bit 2x1 subsampled V and U planes*/
#define		ASVL_PAF_YV16H				0x607
/*8 bit Y plane followed by 8 bit V and U planes*/
#define		ASVL_PAF_YV24				0x608
/*8 bit Y plane only*/
#define		ASVL_PAF_GRAY				0x701

/*Y0, U0, Y1, V0*/
#define		ASVL_PAF_YUYV				0x501
#define		ASVL_PAF_DEPTH_U16			0xc02

typedef	signed		char		MInt8;
typedef	unsigned	char		MUInt8;
typedef	signed		short		MInt16;
typedef	unsigned	short		MUInt16;
typedef signed		int			MInt32;
typedef unsigned	int			MUInt32;

#ifndef __MRECT__
#define __MRECT__
typedef struct __tag_rect
{
	MInt32 left;
	MInt32 top;
	MInt32 right;
	MInt32 bottom;
} MRECT, *PMRECT;
#endif

/*Define the image format space*/
typedef struct __tag_ASVL_OFFSCREEN
{
	MUInt32	u32PixelArrayFormat;
	MInt32	i32Width;
	MInt32	i32Height;
	MUInt8*	ppu8Plane[4];
	MInt32	pi32Pitch[4];
}ASVLOFFSCREEN, *LPASVLOFFSCREEN;

typedef struct _CORE_FACE_S
{
    unsigned char *pFaceFeature;//特征值
    unsigned int nFaceFeatureSize;//特征值大小
    unsigned int track_count;
    unsigned int crop_count;
    unsigned int croped;
    char crop_path[256];//char *crop_path;
    double similar;//图片质量
    unsigned long long track_id;//任务ID
    CORE_FACE_RECT_S stFaceRect;
    CORE_FACE_RECT_TYPE_EN enFaceType;
    CORE_FACE_ATTR_INFO attr_info;
    ASVLOFFSCREEN FaceOffscreen;
    char FaceImgPath[256];
    float catch_face_quality; //当前抓人脸图的质量值
    unsigned long long catch_face_track_id;//任务ID
} CORE_FACE_S;

typedef struct _Face_S
{
    CORE_FACE_S stCoreFaces[MAX_FACES];
#ifdef FIX_MOVE_FRAME_LOSS
    int nFixMovePrevFaceID;
#endif
    int nTrackID;
    long nSearchID;
    int nFaceFrameCounter;
    unsigned char *pRgbYuvData;
    int nRgbYuvWidth;
    int nRgbYuvHeight;
    unsigned char *pFaceFeature;
    int nFaceFeatureSize;
    int nState;
    bface::BoundingBox_t stBoundingBox;    
} Face_S;

/********************************** 人员信息函数集合 *********************************************/

#define Person_IDMethod_FACE (0)
#define Person_IDMethod_CARD (1)

typedef struct _Person_S
{
    char *pFeature;
    int nFeatureSize;

    long nPersonId; //数据库中唯一标识
    long nAID; // 用于和算法ID相匹配
    char szSex[8];
    char szName[64];
    char szIDCardNum[64];
    char szICCardNum[64];
    char szCreateTime[128];
    char szImage[256];
    unsigned char* rt_crop_data;
    int rt_crop_data_size;
    int rt_crop_width;
    int rt_crop_height;
    CORE_FACE_S* last_face_info;

    char szGids[64]; //对应组，可多个
    char szAids[64]; //对应权限，可多个
    char szUUID[128];
    int nPersonType;
    int nIDMethod; 
    char *pFingerprint;        // Pointer to fingerprint template data
    int nFingerprintSize;  
} Person_S;

typedef enum _CORE_MESSAGE
{
    CORE_MESSAGE_BASE = 1000,
    CORE_MESSAGE_FACE_MOVING,
    CORE_MESSAGE_FACE_MATCH,
    CORE_MESSAGE_FACE_DISMISS,
    CORE_CAMERA_CHN_GETCHNFRAME_FAIL,
    CORE_MESSAGE_STRANGER_ENTER,
    CORE_MESSAGE_CATCH_STRANGER_ENTER_IMG,
    CORE_MESSAGE_CARD_NUM,
    CORE_MESSAGE_SET_FACETOOLS_CALLBACK_URL,
    CORE_MESSAGE_SET_FACETOOLS_CALLBACK_PASSWORD,
    CORE_MESSAGE_REQUEST_FACETOOLS_CONNECT_PASSWORD,
    CORE_MESSAGE_REQUEST_FACETOOLS_CONNECT_NEED_COOKIE,
    CORE_MESSAGE_REQUEST_FACETOOLS_CONNECT_THREAD_DELAY,
    CORE_MESSAGE_PERSON_NOT_PERMISSION,
    CORE_MESSAGE_FACE_MATCH_IDCARD,
    CORE_MESSAGE_FACE_PASSWORD_KEY_BUTTON_OPEN_DOOR,
    CORE_MESSAGE_FACE_HEALTH_CODE,
    CORE_MESSAGE_FACE_HEALTH_CODE_IDCARD,
    CORE_MESSAGE_FACE_SHOW_TEXT,
    CORE_MESSAGE_FACE_OPEN_DOOR,
} CORE_MESSAGE;

typedef struct _UART_ATTR_S
{
    unsigned int nBaudRate; //串口波特率
    unsigned int RDBlock; //串口是否阻塞， 0为非阻塞， 1为阻塞
    unsigned int nAttr; //配置校验位 ，传入0为 无校验位，8位数据位，1位停止位，禁用CTS/RTS
    unsigned int mBlockData; //阻塞模式下，多少个字节返回
    unsigned int mBlockTime; //阻塞模式下，多长时间返回
} UART_ATTR_S;

typedef struct _Net_Message
{
    int nMsgType;
    char *szMsg;
    int nszMsgLen;
    Person_S *pPerson;
} Net_Message;

typedef struct _Group_S
{
    long nGid; //组唯一ID
    char szName[32];
    char szDesc[64];
    char szAids[64];
} Group_S;

typedef struct _Action_S
{
    long nAid; //行为唯一ID
    char szCode[64];
    char szName[64];
    char szDesc[256];
} Action_S;

typedef struct _DEVICE_S
{
    long nDid;  //设备ID
    char szName[64];
    char szMac[20];
    char szIp[30];
    char szSn[256];
    char szMsg[256];
} Device_S;

typedef struct record_person
{
    long nRid;
    int nIdentifyed;
    int face_mask;    //0不戴口罩，1戴口罩
    float temp_value;
    long nPersonId;
    char szUUID[128]; //数据库对外唯一标识
    int nPersonType;
    char szName[64];
    char szImage[256];
    char szCreateTime[64];
    char szGids[64]; //对应组，可多个
    char szAids[64]; //对应权限，可多个

    char szIDCardNum[64]; // 身份证号
    char szICCardNum[64]; //韦根卡号
    char szSex[8];
    char szMsg[256];    //客户特殊信息
} Record_penson;


//////////Audio/////////////
typedef enum _SOUND_MODE_E
{
    SOUND_MODE_MONO = 0,/*mono 单声道*/
    SOUND_MODE_STEREO = 1, /*stereo 双声道*/
} SOUND_MODE_E;

typedef enum _BIT_WIDTH_E
{
    BIT_WIDTH_8 = 0, /* 8bit width */
    BIT_WIDTH_16 = 1, /* 16bit width*/
    BIT_WIDTH_24 = 2, /* 24bit width*/
} BIT_WIDTH_E;

typedef enum _SAMPLE_RATE_E
{
    SAMPLE_RATE_8000 = 8000, /* 8K samplerate*/
    SAMPLE_RATE_12000 = 12000, /* 12K samplerate*/
    SAMPLE_RATE_11025 = 11025, /* 11.025K samplerate*/
    SAMPLE_RATE_16000 = 16000, /* 16K samplerate*/
    SAMPLE_RATE_22050 = 22050, /* 22.050K samplerate*/
    SAMPLE_RATE_24000 = 24000, /* 24K samplerate*/
    SAMPLE_RATE_32000 = 32000, /* 32K samplerate*/
    SAMPLE_RATE_44100 = 44100, /* 44.1K samplerate*/
    SAMPLE_RATE_48000 = 48000, /* 48K samplerate*/
    SAMPLE_RATE_64000 = 64000, /* 64K samplerate*/
    SAMPLE_RATE_96000 = 96000, /* 96K samplerate*/
    SAMPLE_RATE_BUTT,
} SAMPLE_RATE_E;

typedef struct _AUDIO_DEV_S
{
    BIT_WIDTH_E enBitwidth;
    SAMPLE_RATE_E enSampleRate;
    SOUND_MODE_E enSoundMode;
    int nAudioDevType;
    unsigned char *pAudioDataBuf;
    unsigned int nAudioDataBufSize;
} AUDIO_DEV_S;

/*
 * 信号强度 ，初略分为4个等级 ,分别为   无信号 - 信号强度弱 - 信号强度一般 - 信号强度高
 */
typedef enum _NET_STRENGTH_EN
{
    NET_STRENGTH_NONE, //无信号
    NET_STRENGTH_WEAK,  //信号强度弱
    NET_STRENGTH_NORMAL, //信号强度一般
    NET_STRENGTH_STRENGHT, //信号强度高
} NET_STRENGTH_EN;

typedef enum _NET_TYPE_EN
{
    NET_TYPE_BT,  //
    NET_TYPE_4G, //
    NET_TYPE_WIFI, //
    NET_TYPE_ETHENET, //
} NET_TYPE_EN;

typedef enum _NET_STATE_EN
{
    NET_TYPE_OPEN, //网络设备已打开，可正常使用状态
    NET_TYPE_CONNECTED, //设备已连接状态
    NET_TYPE_DISCONNECTED, //设备已连接状态
    NET_TYPE_CLOSE, //设备已关闭状态
} NET_STATE_EN;

typedef enum _NET_PASSWORD_TYPE_EN
{
    NET_PASSWORD_TYPE_NONE, //无加密
    NET_PASSWORD_TYPE_WPA_PSK, //WPA_PSK /WPA2_PSK 加密
} NET_PASSWORD_TYPE_EN;

typedef struct _NET_INFO_S
{
    NET_TYPE_EN enDevType;        //网络类型
    NET_STATE_EN enNetState;       //网络状态
    unsigned char szIpAddress[18]; //IP
} NET_INFO_S;

///////////////////// Video 模块编码接口 ///////////////////////////////////////////////////////////////////////////////////

typedef enum _VIDEO_ENCODER_PARAM_RC_E
{
    VIDEO_ENCODER_RC_CBR = 0,
    VIDEO_ENCODER_RC_VBR,
    VIDEO_ENCODER_RC_AVBR,
    VIDEO_ENCODER_RC_QVBR,
    VIDEO_ENCODER_RC_CVBR,
    VIDEO_ENCODER_RC_QPMAP,
    VIDEO_ENCODER_RC_FIXQP
} VIDEO_ENCODER_PARAM_RC_E;

/* the gop mode */
typedef enum _VIDEO_ENCODER_PARAM_GOP_MODE_E
{
    VIDEO_ENCODER_GOPMODE_NORMALP = 0, /* NORMALP */
    VIDEO_ENCODER_GOPMODE_DUALP = 1, /* DUALP;  Not support for Hi3556AV100 */
    VIDEO_ENCODER_GOPMODE_SMARTP = 2, /* SMARTP; Not support for Hi3556AV100 */
    VIDEO_ENCODER_GOPMODE_ADVSMARTP = 3, /* ADVSMARTP ; Only used for Hi3559AV100 */
    VIDEO_ENCODER_GOPMODE_BIPREDB = 4, /* BIPREDB ;Only used for Hi3559AV100/Hi3519AV100 */
    VIDEO_ENCODER_GOPMODE_LOWDELAYB = 5, /* LOWDELAYB; Not support */

    VIDEO_ENCODER_GOPMODE_BUTT,
} VIDEO_ENCODER_PARAM_GOP_MODE_E;

typedef enum _VIDEO_ENCODER_PARAM_EN
{
    VIDEO_ENCODER_PARAM_BIND_MAIN_STREAM, // 绑定 1080p主流
    VIDEO_ENCODER_PARAM_BIND_AI_STREAM, // 绑定 算法数据流，具体大小根据 PREVIEW_YUV_SIZE配置
    VIDEO_ENCODER_PARAM_BIND_MINOR_STREAM, // 绑定 扩展数据流，具体大小根据 MINOR_YUV_SIZE配置
    VIDEO_ENCODER_PARAM_BIND_VDEC_STREAM, // 绑定 解码数据流，具体大小根据 客户参入参数配置
    VIDEO_ENCODER_PARAM_SET_RC,
    VIDEO_ENCODER_PARAM_SET_GOP_MODE
} VIDEO_ENCODER_PARAM_EN;

typedef enum _VIDEO_ENCODER_DATA_TYPE_EN
{
    VIDEO_ENCODER_H264_UNKNOW, //
    VIDEO_ENCODER_H264_I_FRAME, //
    VIDEO_ENCODER_H264_B_FRAME, //
    VIDEO_ENCODER_H264_P_FRAME,
    VIDEO_ENCODER_H264_NALU_SEI, /* SEI types */
    VIDEO_ENCODER_H264_NALU_SPS, /* SPS types */
    VIDEO_ENCODER_H264_NALU_PPS, /* PPS types */
} VIDEO_ENCODER_DATA_TYPE_EN;

#define VIDEO_ENCODE_FAIL_UNKNOW 				(1000)
#define VIDEO_ENCODE_IS_NULL 						(VIDEO_ENCODE_FAIL_UNKNOW +1)
#define VIDEO_ENCODE_NOT_INIT 						(VIDEO_ENCODE_FAIL_UNKNOW +2)
#define VIDEO_ENCODE_ONE_FRAME_OK 			(VIDEO_ENCODE_FAIL_UNKNOW +3)

///////////////////// Sensor 模块接口 //////////////////////////////////////////////////////////////////////
typedef enum _SENSOR_TYPE_EN
{
    SENSOR_TYPE_UNKNOW, SENSOR_TYPE_CB1698, //温度传感器
    SENSOR_TYPE_RB32, //
    SENSOR_TYPE_HWD, // 海威达温度模块
    SENSOR_TYPE_SZD, // 神州盾温度模块
    SENSOR_TYPE_ZC, // 泽成温度模块
    SENSOR_TYPE_TH, // 铁虎测温模块
    SENSOR_TYPE_DKAY, //电科安研模块
    SENSOR_TYPE_DM20018, //DM20018模块
    SENSOR_TYPE_TJ, //铁甲模块
    SNSOR_TYPE_HTPA32_5,
    SNSOR_TYPE_MX15_IR,
    SNSOR_TYPE_HM,//海漫
} SENSOR_TYPE_EN;

typedef struct _PERMISSION_INFO_S
{
    char auty_type[64]; //"time_range" or "week_cycle"
    char auth_info[64]; //周循环 月循环信息；
    char startTime[64];
    char endTime[64];
} permission_Info;

typedef struct PERSONS_s
{
    // ALL EXISTING FIELDS (keep these exactly as they are)
    QByteArray feature;
    QString name;
    QString sex;
    QString idcard;
    QString iccard;
    QString uuid;
    int persontype;
    int personid;
    QString gids;
    QString pids;
    QString createtime;
    QString timeOfAccess;
    QString department;
    bool reader;
    QString strBase64;
    
    // ADD THESE FOUR NEW FIELDS TO THE EXISTING STRUCTURE:
    QString attendanceMode;  // New field for attendance mode (face, card, etc.)
    QString tenantId;        // New field for tenant ID (multi-tenant support)
    QString id;              // New field for additional ID 
    QString status;          // New field for status (active, inactive, etc.)
    int uploadStatus;
    QByteArray fingerprint; 
    int finger_id;
    int access_level;
} PERSONS_t;


//健康码信息
typedef struct HEALTINFO_s
{
    QString name;//姓名
    QString idnumber;//身份证
//    QString hschkdate; //检测时间
    QString hsdatetime; //检测时间
    QString hsresult;  //检测结果
    QString hsdateflag;//24,48,72 等 
    QString vaccine; //疫苗接种剂次
    QString vaccinedate; //疫苗接种日期
    QString type; //类型,粤康码/深I您
    QString color; //green,yellow,red
    QString showTime; //显示时间
    QString temperature; //温度
    int qrcode;//健康码状态
    double warningTemp;
    QString msg;
}HEALTINFO_t;


typedef struct IdentifyFaceRecord_s
{
    double time_Start;//起始时间
    double time_End;//结束时间
    float temp_value;//温度值
    int TimeoutSec;//超时多少秒
    int Tick;//计数器
    int FaceType;//是否陌生人, 1是陌生人，2注册人员， 0未识别
    int face_personid;//人脸库id
    int face_persontype;//是否陌生人（1陌生人，2非陌生人）
    int face_mack;
    //bool Languagetips;//语音提示
    bool Finish;//是否完成识别
    /*下面字段是人员录入信息库*/
    long rid; //主键id号
    int Identifyed; //是否识别
    int passType;
    QString face_name;//名称
    QString face_sex;//性别
    QString face_uuid;//人脸信息的数据库的标一标识码
    QString face_idcardnum;//身份证号
    QString face_iccardnum;//ic卡号
    QString face_gids;//组
    QString face_aids;//开门方式
    QString process_state;//<1刷卡,2刷脸,3测温,4口罩,5二唯码,6身份证>
    QByteArray face_feature;//特征值
    QString face_attendanceMode;
    QString face_tenantId; 
    QString face_id;
    QString face_status;
    QString FaceImgPath;
    QString FaceFullImgPath;
    QDateTime createtime;
    CORE_FACE_S face;
}IdentifyFaceRecord_t;

#endif