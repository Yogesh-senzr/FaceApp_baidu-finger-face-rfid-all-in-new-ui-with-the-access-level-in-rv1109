#ifndef RKUTILS_H
#define RKUTILS_H

#include "rga/RgaApi.h"
#include <QImage>

namespace YNH_LJX{
typedef struct _RectF
{
    signed int x;
    signed int y;
    signed int w;
    signed int h;
} RectF;

class RkUtils
{
public:
    static int Utils_RgaPrepareInfo(unsigned char *buf, RgaSURF_FORMAT format, RectF rect, int sw, int sh, rga_info_t *info);
    static int Utils_RgaDrawImage(unsigned char *src, RgaSURF_FORMAT src_format, RectF srcRect, int src_sw, int src_sh, unsigned char *dst,
                                  RgaSURF_FORMAT dst_format, RectF dstRect, int dst_sw, int dst_sh, int rotate, unsigned int blend);

    static int NV21CutImage(unsigned char * src, const int srcW, const int srcH, unsigned char * dest, const int destLen, const int x0, const int y0,const int x1, const int y1);

    static int Utils_YUV420SPConvertToYUV420P(unsigned long nSrcAddr, unsigned long nDstAddr, int nWidth, int nHeight);

    static int Utils_YVU420SPConvertToYUV420P(unsigned long nSrcAddr, unsigned long nDstAddr, int nWidth, int nHeight);
    static int Utils_YV12ConVertToNV21(unsigned char *yv12, unsigned char *nv21, int width, int height);

    static void *Utils_Malloc(int nSize);
    static void Utils_Free(void *pBuf);
    static void Utils_ExecCmd(const char* szCmd);

    static int YUVtoJPEG(const char *filename, unsigned char *yuvbuf, int width, int height, int quality = 90);
    static int YUVtoJPEGbuf(void *buf, unsigned char *yuvbuf, int width, int height, int quality= 90);

    static int RGBtoJPEG(const char *filename, unsigned char *rgbbuf, int width, int height, int quality = 70);

    static QImage Utils_ReadImage(const QString &path);
    static unsigned char *Utils_JpegToNV12(char *szJpegPath, int *nJpegWidth, int *nJpegHeight);
    static unsigned char *Utils_JpegToNV12(const QImage &, int *nJpegWidth, int *nJpegHeight);

    static int Utils_getFileSize(const char *path);
    static QString GetIpAddress();
    static double GetElapsedRealtimeSeconds();

};
}

#endif // RKUTILS_H
