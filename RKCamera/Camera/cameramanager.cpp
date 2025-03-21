#include "cameramanager.h"
#include "PCIcore/RkUtils.h"
#include "MessageHandler/Log.h"
#include "Config/ReadConfig.h"
#include "Helper/myhelper.h"
#include "aiq_control.h"
#include "camrgb_control.h"
#include "camir_control.h"
#include <sys/times.h>
#include <sys/time.h>

#include <QThread>
#include <QSize>

class CameraManagerPrivate
{
	Q_DECLARE_PUBLIC (CameraManager)
public:
	CameraManagerPrivate(const QSize &RgaResolution, const bool &RgaMirrored, const qint32 &RgaRotation, const QSize &IrResolution,
			const bool &IrMirrored, const qint32 &IrRotation, CameraManager *dd);
private:
	CameraPreviewYUVCallBack mCameraPreviewYUVCallBack = NULL;
private:
	CameraManager * const q_ptr;
};
static pthread_t gCameraPreviewYUVCallBackThread = 0;

static pthread_mutex_t gRgaBufLock;
static int gRgbFmt = 0;
static int gRgbWidth = 640;
static int gRgbHeight = 480;
static char *gRgbBuf0 = Q_NULLPTR;
static char *gTakeRgbPhotoYUVBuf = Q_NULLPTR;
static char *gTakeRgbPhotoBuf = Q_NULLPTR;
static int gRgbBufSize = 0;
static int gRgbRotation = 0;

static pthread_mutex_t gIrBufLock;
static int gIrFmt = 0;
static int gIrWidth = 640;
static int gIrHeight = 480;
static char *gIrBuf0 = Q_NULLPTR;
static char *gTakeIrPhotoYUVBuf = Q_NULLPTR;
static char *gTakeIrPhotoBuf = Q_NULLPTR;
static int gIrBufSize = 0;
static int gIrRotation = 0;
static int gDeskW = 800;
static int gDeskH = 1280;
static bool volatile g_bIsAiProcessable = false;

static double m_dRgbPreviewTime = 0;
static double m_dIrPreviewTime = 0;
static int m_nRgbResetCount = 0;
static int m_nIrResetCount = 0;

static inline double GetElapsedRealtimeSeconds()
{
	int nSocClkTck = 0;
	struct tms stTims;
	nSocClkTck = sysconf(_SC_CLK_TCK);
	return times(&stTims) / (double) nSocClkTck;
}

static inline void *CameraPreviewYUVCallBackLoop(void *arg)
{
	CameraManager *obj = (CameraManager *) arg;

	char *pRgbBuf = (char*) malloc(gDeskW * gDeskH * 3 / 2);
	char *pIrBuf = (char*) malloc(gDeskW * gDeskH * 3 / 2);
	double nowTime = 0;
	double timeDiff = 3;

	while (true)
	{
		nowTime = GetElapsedRealtimeSeconds();
#if 0
		bool bRebootFlag = 0;
		if(m_dRgbPreviewTime != 0 && m_dIrPreviewTime != 0)
		{
			if(nowTime - m_dRgbPreviewTime > 4*timeDiff || nowTime - m_dIrPreviewTime > 4*timeDiff)
			{
				LogD("%s %s[%d] nowTime -m_dRgbPreviewTime %f \n",__FILE__,__FUNCTION__,__LINE__,(nowTime - m_dRgbPreviewTime));
				LogD("%s %s[%d] nowTime -m_dIrPreviewTime %f \n",__FILE__,__FUNCTION__,__LINE__,(nowTime - m_dIrPreviewTime));
				myHelper::Utils_Reboot();
			}
			if(nowTime - m_dRgbPreviewTime > timeDiff && m_nRgbResetCount++ < 2)
			{
				LogD("%s %s[%d] nowTime -m_dRgbPreviewTime %f \n",__FILE__,__FUNCTION__,__LINE__,(nowTime - m_dRgbPreviewTime));
				//复位可见光摄像头
				myHelper::Utils_ExecCmd("echo 0 > /sys/class/custom_class/custom_dev/reset");
				m_dRgbPreviewTime = nowTime;
			}
			if(nowTime - m_dIrPreviewTime > timeDiff && m_nIrResetCount++ < 2)
			{
				LogD("%s %s[%d] nowTime -m_dIrPreviewTime %f \n",__FILE__,__FUNCTION__,__LINE__,(nowTime - m_dIrPreviewTime));
				//复位红外摄像头
				myHelper::Utils_ExecCmd("echo 1 > /sys/class/custom_class/custom_dev/reset");
				m_dIrPreviewTime = nowTime;
			}
		}
#endif

		if (g_bIsAiProcessable == false)
		{
			sleep(1);
			continue;
		}

		pthread_mutex_lock(&gRgaBufLock);
		memcpy(pRgbBuf, gRgbBuf0, gRgbBufSize);
		pthread_mutex_unlock(&gRgaBufLock);

		pthread_mutex_lock(&gIrBufLock);
		memcpy(pIrBuf, gIrBuf0, gIrBufSize);
		pthread_mutex_unlock(&gIrBufLock);

		if (pRgbBuf != Q_NULLPTR && pIrBuf != Q_NULLPTR && gIrBufSize && gRgbBufSize)
		{
			obj->setCameraPreviewYUVDataCall(gRgbFmt, (unsigned long) pRgbBuf, 0.0, gRgbWidth, gRgbHeight, gRgbBufSize, gRgbRotation,
					(unsigned long) pIrBuf, 0.0, gIrWidth, gIrHeight, gIrBufSize, gIrRotation);
		}
	}
}

static void onRgbPreview(void *ptr, int fd, int fmt, int w, int h, int rotation)
{
	(void) fd;
	//printf("%s %s[%d] rga %d \n", __FILE__, __FUNCTION__, __LINE__, fd);

	m_dRgbPreviewTime =  GetElapsedRealtimeSeconds();
	m_nRgbResetCount = 0;
	pthread_mutex_lock(&gRgaBufLock);
	if (ptr == Q_NULLPTR || w <= 0 || h <= 0)
	{
		pthread_mutex_unlock(&gRgaBufLock);
		return;
	}

	gRgbFmt = fmt;
	gRgbWidth = w;
	gRgbHeight = h;
	gRgbRotation = rotation;

	if (gRgbBufSize < w * h * 3 / 2)
	{
		printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
		gRgbBufSize = w * h * 3 / 2;
		if (gRgbBuf0 != Q_NULLPTR)
		{
			free(gRgbBuf0);
		}
		gRgbBuf0 = (char*) malloc(gRgbBufSize);
		if (gRgbBuf0 == Q_NULLPTR)
		{
			gRgbBufSize = 0;
			pthread_mutex_unlock(&gRgaBufLock);
			return;
		}
	}

	if (gRgbBuf0 != Q_NULLPTR)
	{
		memcpy(gRgbBuf0, ptr, gRgbBufSize);
	}
	pthread_mutex_unlock(&gRgaBufLock);
}

static void onIrPreview(void *ptr, int fd, int fmt, int w, int h, int rotation)
{
	(void) fd;
	m_dIrPreviewTime =  GetElapsedRealtimeSeconds();
	m_nIrResetCount = 0;
	// printf("%s %s[%d] ir %d \n", __FILE__, __FUNCTION__, __LINE__, fd);
	pthread_mutex_lock(&gIrBufLock);
	if (ptr == Q_NULLPTR || w <= 0 || h <= 0)
	{
		pthread_mutex_unlock(&gIrBufLock);
		return;
	}

	gIrFmt = fmt;
	gIrWidth = w;
	gIrHeight = h;
	gIrRotation = rotation;

	if (gIrBufSize < w * h * 3 / 2)
	{
		gIrBufSize = w * h * 3 / 2;
		if (gIrBuf0 != Q_NULLPTR)
		{
			printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
			free(gIrBuf0);
		}
		gIrBuf0 = (char*) malloc(gIrBufSize);
		if (gIrBuf0 == Q_NULLPTR)
		{
			gIrBufSize = 0;
			pthread_mutex_unlock(&gIrBufLock);
			return;
		}
		::g_bIsAiProcessable = true;
	}
	if (gIrBuf0 != Q_NULLPTR)
	{
		memcpy(gIrBuf0, ptr, gIrBufSize);
	}
	pthread_mutex_unlock(&gIrBufLock);
}

CameraManagerPrivate::CameraManagerPrivate(const QSize &RgaResolution, const bool &/*RgaMirrored*/, const qint32 &RgaRotation,
		const QSize &IrResolution, const bool &/*IrMirrored*/, const qint32 &IrRotation, CameraManager *dd) :
		q_ptr(dd)
{
	pthread_mutex_init(&gRgaBufLock, Q_NULLPTR);
	pthread_mutex_init(&gIrBufLock, Q_NULLPTR);

	set_rgb_rotation(RgaRotation);
	set_rgb_param(RgaResolution.width(), RgaResolution.height(), NULL);
	set_rgb_process(onRgbPreview);

	set_ir_rotation(IrRotation);
	set_ir_param(IrResolution.width(), IrResolution.height(), NULL);
	set_ir_process(onIrPreview);

	display_switch(DISPLAY_VIDEO_RGB);
}

CameraManager::CameraManager(const QSize RgaResolution, const bool RgaMirrored, const qint32 RgaRotation, const QSize IrResolution,
		const bool IrMirrored, const qint32 IrRotation, const int w, const int h, QObject *parent) :
		QObject(parent), d_ptr(
				new CameraManagerPrivate(RgaResolution, RgaMirrored, RgaRotation, IrResolution, IrMirrored, IrRotation, this))
{
	gDeskH = h;
	gDeskW = w;
	QThread *thread = new QThread;
	this->moveToThread(thread);
	thread->start();
}

CameraManager::~CameraManager()
{

}

bool CameraManager::startPreview(bool bDualCamera, QSize size)
{
	display_init(size.width(), size.height());
	bool flag = true;
	aiq_control_alloc(AIQ_CONTROL_RGB);
	flag = camrgb_control_init() == 0;
	if (bDualCamera)
	{
		aiq_control_alloc(AIQ_CONTROL_IR);
		flag &= camir_control_init() == 0;
	}

	aiq_control_setFrameRate(20);
	return flag;
}

void CameraManager::stopPreview()
{
	display_exit();
	camir_control_exit();
	aiq_control_free(AIQ_CONTROL_IR);

	camrgb_control_exit();
	aiq_control_free(AIQ_CONTROL_RGB);
}

bool CameraManager::startIrPreview()
{
	int res = 0;
	if (!camir_control_run())
	{
		aiq_control_alloc(AIQ_CONTROL_IR);
		res = camir_control_init();
	}
	return res == 0;
}

void CameraManager::stopIrPreview()
{
	if (camir_control_run())
	{
		camir_control_exit();
		aiq_control_free(AIQ_CONTROL_IR);
	}
}

void CameraManager::uninit()
{
	stopPreview();
}

void CameraManager::setFaceInfo(enum display_face_type *enType, int nSize)
{
	display_set_info(enType, nSize);
}

void CameraManager::setFaceRect(MRECT *stRect, int nSize)
{
	display_set_rect(stRect, nSize);
}

void CameraManager::setCameraPreviewYUVDataCall(int nPixelFormat, unsigned long nYuvVirAddr0, unsigned long nYuvPhyAddr0, int nWidth0,
		int nHeight0, int nSize0, int rotation0, unsigned long nYuvVirAddr1, unsigned long nYuvPhyAddr1, int nWidth1, int nHeight1,
		int nSize1, int rotation1)
{
	Q_D (CameraManager);
	if (d->mCameraPreviewYUVCallBack)
		d->mCameraPreviewYUVCallBack(nPixelFormat, nYuvVirAddr0, nYuvPhyAddr0, nWidth0, nHeight0, nSize0, rotation0, nYuvVirAddr1,
				nYuvPhyAddr1, nWidth1, nHeight1, nSize1, rotation1);
}

void CameraManager::setCameraPreviewYUVCallBack(CameraPreviewYUVCallBack call)
{
	Q_D (CameraManager);
	d->mCameraPreviewYUVCallBack = call;
}

void CameraManager::slotDisMissMessage()
{
	MRECT stMRect = { 0 };
	this->setFaceRect(&stMRect, 1);
}

void CameraManager::slotDrawFaceRect(const QList<CORE_FACE_RECT_S> list)
{
	MRECT stMRect[MAX_FACES] = { 0 };
	enum display_face_type enDisplayFaceType[MAX_FACES];
	for (int i = 0; i < list.size(); i++)
	{
		stMRect[i].left = list.at(i).nX;
		stMRect[i].top = list.at(i).nY;
		stMRect[i].right = list.at(i).nWidth + list.at(i).nX;
		stMRect[i].bottom = list.at(i).nHeight + list.at(i).nY;

		enDisplayFaceType[i] = DISPLAY_FACE_NORMAL;
		if (list.at(i).nColor == 0x00FF00)
		{
			enDisplayFaceType[i] = DISPLAY_FACE_SUCCESS;
		}
	}

	this->setFaceInfo(enDisplayFaceType, list.size());
	this->setFaceRect(stMRect, list.size());
}

void CameraManager::startAiProcess()
{
	if (gCameraPreviewYUVCallBackThread == 0)
	{
		pthread_create(&gCameraPreviewYUVCallBackThread, Q_NULLPTR, CameraPreviewYUVCallBackLoop, this);
	}
}

void CameraManager::stopAiProcess()
{
	pthread_mutex_lock(&gRgaBufLock);
	::g_bIsAiProcessable = false;
	pthread_mutex_unlock(&gRgaBufLock);
}

bool CameraManager::takePhotos(char **ppDstRgbData, int *pDstRgbSize, char **ppDstIrData, int *pDstIrSize)
{
	bool ret = false;
	bool bCanTakePhoto = true;
	if (gTakeRgbPhotoYUVBuf == ISC_NULL)
	{
		gTakeRgbPhotoYUVBuf = (char*) malloc(gDeskW * gDeskH * 3 / 2);
	}
	if (gTakeRgbPhotoYUVBuf == ISC_NULL)
	{
		return false;
	}
	memset(gTakeRgbPhotoYUVBuf, 0, gDeskW * gDeskH * 3 / 2);

	if (gTakeIrPhotoYUVBuf == ISC_NULL)
	{
		gTakeIrPhotoYUVBuf = (char*) malloc(gDeskW * gDeskH * 3 / 2);
	}
	if (gTakeIrPhotoYUVBuf == ISC_NULL)
	{
		return false;
	}
	memset(gTakeIrPhotoYUVBuf, 0, gDeskW * gDeskH * 3 / 2);

	if (gTakeRgbPhotoBuf == ISC_NULL)
	{
		gTakeRgbPhotoBuf = (char*) malloc(gDeskW * gDeskH * 3 / 2);
	}
	if (gTakeRgbPhotoBuf == ISC_NULL)
	{
		return false;
	}
	memset(gTakeRgbPhotoBuf, 0, gDeskW * gDeskH * 3 / 2);

	if (gTakeIrPhotoBuf == ISC_NULL)
	{
		gTakeIrPhotoBuf = (char*) malloc(gDeskW * gDeskH * 3 / 2);
	}
	if (gTakeIrPhotoBuf == ISC_NULL)
	{
		return false;
	}
	memset(gTakeIrPhotoBuf, 0, gDeskW * gDeskH * 3 / 2);

	pthread_mutex_lock(&gRgaBufLock);
	bCanTakePhoto = false;
	if (gRgbBuf0 != ISC_NULL)
	{
		bCanTakePhoto = true;
		memcpy(gTakeRgbPhotoYUVBuf, gRgbBuf0, gRgbBufSize);
	}
	pthread_mutex_unlock(&gRgaBufLock);

	pthread_mutex_lock(&gIrBufLock);
	bCanTakePhoto = false;
	if (gIrBuf0 != ISC_NULL)
	{
		bCanTakePhoto = true;
		memcpy(gTakeIrPhotoYUVBuf, gIrBuf0, gIrBufSize);
	}
	pthread_mutex_unlock(&gIrBufLock);

	if (bCanTakePhoto == true)
	{
		YNH_LJX::RectF srcrect = { 0, 0, gRgbWidth, gRgbHeight };
		YNH_LJX::RectF dstrect = { 0, 0, gDeskW, gDeskH };
		char *pRGBRotationBuf = ISC_NULL;
		char *pIrRotationBuf = ISC_NULL;
		int nRgbCameraRotation = 0;
		int nIrCameraRotation = 0;
#define HAL_TRANSFORM_ROT_90     0x04
#define HAL_TRANSFORM_ROT_270    0x07
#define HAL_TRANSFORM_ROT_0    0x02
#define HAL_TRANSFORM_ROT_180    0x03

		pRGBRotationBuf = (char*) malloc(gDeskW * gDeskH * 3 / 2);
		if (pRGBRotationBuf == ISC_NULL)
		{
			goto FAIL;
		}

		memset(pRGBRotationBuf, 0, gDeskW * gDeskH * 3 / 2);

		nRgbCameraRotation = ReadConfig::GetInstance()->getRgbCameraRotation();
		nIrCameraRotation = ReadConfig::GetInstance()->getIrCameraRotation();
        printf(">>>%s,%s,%d,nRgbCameraRotation=%d\n",__FILE__,__func__,__LINE__,nRgbCameraRotation);
		if(nRgbCameraRotation == 270)
		{
			nRgbCameraRotation = HAL_TRANSFORM_ROT_270;
		} else if(nRgbCameraRotation == 90)
		{
			nRgbCameraRotation = HAL_TRANSFORM_ROT_90;
		} else if(nRgbCameraRotation == 0)
		{
			nRgbCameraRotation = HAL_TRANSFORM_ROT_0;
		} else if(nRgbCameraRotation == 180)
		{
			nRgbCameraRotation = HAL_TRANSFORM_ROT_180;
		}


		YNH_LJX::RkUtils::Utils_RgaDrawImage((unsigned char *) gTakeRgbPhotoYUVBuf, RK_FORMAT_YCbCr_420_SP, srcrect, gRgbWidth,
				gRgbHeight, (unsigned char*) pRGBRotationBuf, RK_FORMAT_YCbCr_420_SP, dstrect, gDeskW, gDeskH, nRgbCameraRotation, 0);

		YNH_LJX::RkUtils::Utils_YVU420SPConvertToYUV420P((unsigned long) pRGBRotationBuf, (unsigned long) gTakeRgbPhotoYUVBuf, gDeskW,
				gDeskH);
		YNH_LJX::RkUtils::YUVtoJPEGbuf(gTakeRgbPhotoBuf, (unsigned char*) gTakeRgbPhotoYUVBuf, gDeskW, gDeskH, 90);

		pIrRotationBuf = (char*) malloc(gDeskW * gDeskH * 3 / 2);
		if (pIrRotationBuf == ISC_NULL)
		{
			goto FAIL;
		}
		memset(pIrRotationBuf, 0, gDeskW * gDeskH * 3 / 2);
		if(nIrCameraRotation == 270)
		{
			nIrCameraRotation = HAL_TRANSFORM_ROT_270;
		} else if(nIrCameraRotation == 90)
		{
			nIrCameraRotation = HAL_TRANSFORM_ROT_90;
		}
		YNH_LJX::RkUtils::Utils_RgaDrawImage((unsigned char *) gTakeIrPhotoYUVBuf, RK_FORMAT_YCbCr_420_SP, srcrect, gIrWidth, gIrHeight,
				(unsigned char*) pIrRotationBuf, RK_FORMAT_YCbCr_420_SP, dstrect, gDeskW, gDeskH, nIrCameraRotation, 0);

		YNH_LJX::RkUtils::Utils_YVU420SPConvertToYUV420P((unsigned long) pIrRotationBuf, (unsigned long) gTakeIrPhotoYUVBuf, gDeskW,
				gDeskH);
		YNH_LJX::RkUtils::YUVtoJPEGbuf(gTakeIrPhotoBuf, (unsigned char*) gTakeIrPhotoYUVBuf, gDeskW, gDeskH, 90);

		*ppDstRgbData = gTakeRgbPhotoBuf;
		*pDstRgbSize = gRgbBufSize;

		*ppDstIrData = gTakeIrPhotoBuf;
		*pDstIrSize = gRgbBufSize;

		ret = true;

		FAIL:
		{
			if (pRGBRotationBuf != ISC_NULL)
			{printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
				free(pRGBRotationBuf);
			}
			if (pIrRotationBuf != ISC_NULL)
			{printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
				free(pIrRotationBuf);
			}
		}
		return ret;
	}

	return false;
}
