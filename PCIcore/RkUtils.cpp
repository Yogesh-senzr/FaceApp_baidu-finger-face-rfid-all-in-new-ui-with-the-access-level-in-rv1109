#include "RkUtils.h"
#include "turbojpeg.h"
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/times.h>
#include <sys/time.h>
#include <netdb.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <sys/stat.h>
#include "MessageHandler/Log.h"
#include "SharedInclude/GlobalDef.h"

#include <QtGui/QImageReader>

int YNH_LJX::RkUtils::Utils_RgaPrepareInfo(unsigned char *buf, RgaSURF_FORMAT format, RectF rect, int sw, int sh, rga_info_t *info)
{
    memset(info, 0, sizeof(rga_info_t));

    info->fd = -1;
    info->virAddr = buf;
    info->mmuFlag = 1;

    return rga_set_rect(&info->rect, rect.x, rect.y, sw, sh, sw, sh, format);
}

int YNH_LJX::RkUtils::NV21CutImage(unsigned char * src, const int srcW, const int srcH, unsigned char * dest, const int destLen, const int x0, const int y0,
                                   const int x1, const int y1)
{
    /*verify param*/
    if (x0 > srcW || x1 > srcW || y0 > srcH || y1 > srcH)
    {
        LogV("[Error]YUV: Out of range! ");
        LogV("x0:%d x1:%d srcW:%d ", x0, x1, srcW);
        LogV("y0:%d y1:%d srcH:%d ", y0, y1, srcH);
        LogV("\n");
        return -1;
    }
    if (x1 <= x0 || y1 <= y0 || x1 - x0 > srcW || y1 - y0 > srcH)
    {
        LogV("[Error]YUV: Out of range!\n");
        return -1;
    }
    if ((x1 - x0) * (y1 - y0) * 1.5 > destLen)
    {
        LogV("[Error]YUV: The storage space is too small!\n");
        return -1;
    }
    if (y1 % 2 != 0 || y0 % 2 != 0 || x0 % 2 != 0 || x1 % 2 != 0)
    {
        LogV("[Error]YUV: It's not divisible by 2! (x0: %d y0: %d) -- (x1:%d  y1:%d) \n", x0, y0, x1, y1);
        return -1;
    }

    //cut image
    int dstw = x1 - x0;
    int dsth = y1 - y0;

    unsigned char *psrc = src + y0 * srcW + x0;
    unsigned char *pdest = dest;
    for (int i = 0; i < dsth; i++)
    {
        memcpy(pdest, psrc, dstw);
        pdest += dstw;
        psrc += srcW;
    }

    psrc = src + srcW * srcH + (y0 / 2) * srcW + x0;
    pdest = dest + dstw * dsth;

    for (int i = 0; i < dsth / 2; i++)
    {
        memcpy(pdest, psrc, dstw);
        pdest += dstw;
        psrc += srcW;
    }

    return 0;
}

int YNH_LJX::RkUtils::Utils_RgaDrawImage(unsigned char *src, RgaSURF_FORMAT src_format, RectF srcRect, int src_sw, int src_sh, unsigned char *dst,
                                         RgaSURF_FORMAT dst_format, RectF dstRect, int dst_sw, int dst_sh, int rotate, unsigned int blend)
{
    rga_info_t srcInfo;
    rga_info_t dstInfo;

    if (Utils_RgaPrepareInfo(src, src_format, srcRect, src_sw, src_sh, &srcInfo) < 0)  return -1;

    if (Utils_RgaPrepareInfo(dst, dst_format, dstRect, dst_sw, dst_sh, &dstInfo) < 0)  return -1;

    srcInfo.rotation = rotate;
    if (blend) srcInfo.blend = blend;

    return c_RkRgaBlit(&srcInfo, &dstInfo, nullptr);
}

int YNH_LJX::RkUtils::Utils_YUV420SPConvertToYUV420P(unsigned long nSrcAddr, unsigned long nDstAddr, int nWidth, int nHeight)
{
    unsigned long pTmpBuf1 = NULL;
    if (!nSrcAddr || !nDstAddr)
    {
        return 0;
    }

    pTmpBuf1 = nDstAddr;
    int nTmpSize = nWidth * nHeight;
    memcpy((char*) pTmpBuf1, (char*) nSrcAddr, nTmpSize);

    pTmpBuf1 = nDstAddr + nTmpSize;
    for (int i = 0; i < nTmpSize / 4; i++)
    {
        ((char*) pTmpBuf1)[i] = *(char*) (nSrcAddr + nTmpSize + i * 2);
    }

    pTmpBuf1 = pTmpBuf1 + nTmpSize / 4;
    for (int i = 0; i < nTmpSize / 4; i++)
    {
        ((char*) pTmpBuf1)[i] = *(char*) (nSrcAddr + nTmpSize + i * 2 + 1);
    }
    return 1;
}

int YNH_LJX::RkUtils::Utils_YVU420SPConvertToYUV420P(unsigned long nSrcAddr, unsigned long nDstAddr, int nWidth, int nHeight)
{
    unsigned long pTmpBuf1 = NULL;
    if (!nSrcAddr || !nDstAddr)
    {
        return 0;
    }
    pTmpBuf1 = nDstAddr;
    int nTmpSize = nWidth * nHeight;
    memcpy((char*) pTmpBuf1, (char*) nSrcAddr, nTmpSize);

    pTmpBuf1 = nDstAddr + nTmpSize;
    for (int i = 0; i < nTmpSize / 4; i++)
    {
        ((char*) pTmpBuf1)[i] = *(char*) (nSrcAddr + nTmpSize + i * 2);
    }

    pTmpBuf1 = pTmpBuf1 + nTmpSize / 4;
    for (int i = 0; i < nTmpSize / 4; i++)
    {
        ((char*) pTmpBuf1)[i] = *(char*) (nSrcAddr + nTmpSize + i * 2 + 1);
    }
    return 1;
}

int YNH_LJX::RkUtils::Utils_YV12ConVertToNV21(unsigned char *yv12, unsigned char *nv21, int width, int height)
{
    int frameSize = width * height;
    memcpy(nv21, yv12, frameSize);
    nv21 += frameSize;
    yv12 += frameSize;
    int halfWidth = width / 2;
    int halfHeight = height / 2;
    int quadFrame = halfWidth * halfHeight;
    for (int i = 0; i < halfHeight; i++)
    {
        for (int j = 0; j < halfWidth; j++)
        {
            *nv21++ = *(yv12 + i * halfWidth + j);
            *nv21++ = *(yv12 + quadFrame + i * halfWidth + j);
        }
    }
}

void *YNH_LJX::RkUtils::Utils_Malloc(int nSize)
{
    void *pAddr = malloc(nSize);
    if (pAddr != NULL)
    {
#ifdef _DEBUG_MALLOC_
        if (m_MemBuf == NULL)
        {
            m_MemBuf = (MEM_BUF_S*) malloc(MAX_MEM_BUF_SIZE * sizeof(MEM_BUF_S));
            memset(m_MemBuf, 0, sizeof(MAX_MEM_BUF_SIZE * sizeof(MEM_BUF_S)));
        }
        MEM_BUF_S *pMemBuf = NULL;
        for (int i = 0; i < MAX_MEM_BUF_SIZE; i++)
        {
            if (m_MemBuf[i].nBufAddr == 0)
            {
                pMemBuf = &m_MemBuf[i];
                break;
            }
        }
        if (pMemBuf != NULL)
        {
            pMemBuf->nBufAddr = (uint64_t) pAddr;
            pMemBuf->nSize = nSize;
            LogV("%s %s[%d] malloc %dm %dk %db \n", __FILE__, __FUNCTION__, __LINE__, (nSize / 1024 / 1024), (nSize / 1024), nSize);
        } else
        {
            LogV("%s %s[%d] malloc mem size %d too big ? \n", __FILE__, __FUNCTION__, __LINE__, MAX_MEM_BUF_SIZE);
        }
#endif
    } else
    {
        LogV("%s %s[%d] malloc  size %d error ? \n", __FILE__, __FUNCTION__, __LINE__, nSize);
    }

#ifdef _DEBUG_MALLOC_
    int nTotalAllocSize = 0;
    for (int i = 0; i < MAX_MEM_BUF_SIZE; i++)
    {
        nTotalAllocSize += m_MemBuf[i].nSize;
    }
    LogV("%s %s[%d] malloc pAddr %x nTotalAllocSize %dm %dk %db \n", __FILE__, __FUNCTION__, __LINE__,pAddr, (nTotalAllocSize / 1024 / 1024),
         (nTotalAllocSize / 1024), nTotalAllocSize);
#endif
    return pAddr;
}

void YNH_LJX::RkUtils::Utils_Free(void *pBuf)
{
	//printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
    if (pBuf != NULL)
    {
#ifdef _DEBUG_MALLOC_
        MEM_BUF_S *pMemBuf = NULL;
        for (int i = 0; i < MAX_MEM_BUF_SIZE; i++)
        {
            if (m_MemBuf[i].nBufAddr == (uint64_t) pBuf)
            {
                pMemBuf = &m_MemBuf[i];
                LogV("%s %s[%d] free  %dm %dk %db \n", __FILE__, __FUNCTION__, __LINE__, (pMemBuf->nSize / 1024 / 1024),
                     (pMemBuf->nSize / 1024), pMemBuf->nSize);
                break;
            }
        }

        if(pMemBuf == NULL)
        {
            LogV("%s %s[%d] free pBuf %x  no use Utils_Malloc \n", __FILE__, __FUNCTION__, __LINE__, pBuf);
        } else
        {
            pMemBuf->nBufAddr = pMemBuf->nSize = 0;
        }
#endif
        //printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
        free(pBuf);
    }
}

void YNH_LJX::RkUtils::Utils_ExecCmd(const char *szCmd)
{
    char buf[64] = { 0 };
    if (szCmd != NULL)
    {
        FILE *pFile = popen(szCmd, "r");
        if (pFile)
        {
            while (fgets(buf, sizeof(buf), pFile) != NULL)
            {
            }
            pclose(pFile);
        }
    }
}

int YNH_LJX::RkUtils::YUVtoJPEG(const char *filename, unsigned char *yuvbuf, int width, int height, int quality)
{
    tjhandle handle = NULL;
    unsigned char* jpeg_buff = NULL;
    unsigned long jpeg_size = 0;
    int needsize = tjBufSizeYUV2(width, 1, height, TJSAMP_420);
    //LogV("yuvtojpg needsize %d.\n", needsize);

    handle = tjInitCompress();
    if (handle == NULL)
    {
        LogV("jpeg turbo init failed.\n");
        return -1;
    }
    tjCompressFromYUV(handle, yuvbuf, width, 1, height, TJSAMP_420, &jpeg_buff, &jpeg_size, quality, TJFLAG_FASTDCT);

    FILE *file = fopen(filename, "wb");
    if (!file)
    {
        LogV("YUVtoJPEG open fail errno = %d reason = %s \n", errno, strerror(errno));
        return -1;
    }

    fwrite(jpeg_buff, jpeg_size, 1, file);
    fflush(file);
    fclose(file);

    tjDestroy(handle);
    tjFree(jpeg_buff);
    return jpeg_size;
}

int YNH_LJX::RkUtils::YUVtoJPEGbuf(void *buf, unsigned char *yuvbuf, int width, int height, int quality)
{
    if (buf == NULL || yuvbuf == NULL) return -1;

    tjhandle handle = NULL;
    unsigned char* jpeg_buff = NULL;
    unsigned long jpeg_size = 0;
    int needsize = 0;

    needsize = tjBufSizeYUV2(width, 1, height, TJSAMP_420);
    LogV("yuvtojpg needsize %d.\n", needsize);

    handle = tjInitCompress();
    if (handle == NULL)
    {
        LogV("jpeg turbo init failed.\n");
        return -1;
    }
    tjCompressFromYUV(handle, yuvbuf, width, 1, height, TJSAMP_420, &jpeg_buff, &jpeg_size, quality, TJFLAG_FASTDCT);
    memcpy(buf, jpeg_buff, jpeg_size);

    tjDestroy(handle);
    tjFree(jpeg_buff);
    return jpeg_size;
}

int YNH_LJX::RkUtils::RGBtoJPEG(const char *filename, unsigned char *rgbbuf, int width, int height, int quality)
{
    tjhandle handle = NULL;
    unsigned char* jpeg_buff = NULL;
    unsigned long jpeg_size = 0;

    handle = tjInitCompress();
    if (handle == NULL)
    {
        LogV("jpeg turbo init failed.\n");
        return -1;
    }
    tjCompress2(handle, rgbbuf, width, 0, height, TJPF_BGR, &jpeg_buff, &jpeg_size, TJSAMP_422, quality, TJFLAG_FASTDCT);

    FILE *file = fopen(filename, "wb");
    if (!file)
    {
        LogV("RGBtoJPEG open fail errno = %d reason = %s \n", errno, strerror(errno));
        return -1;
    }

    fwrite(jpeg_buff, jpeg_size, 1, file);
    fflush(file);
    fclose(file);

    tjDestroy(handle);
    tjFree(jpeg_buff);
    return jpeg_size;
}

QImage YNH_LJX::RkUtils::Utils_ReadImage(const QString &path)
{
#define MAX_IMAGE_LENGTH (1024)
    QImageReader reader(path);
    reader.setAutoTransform(true);
    reader.setDecideFormatFromContent(true);
    QSize size = reader.size();
    int length = qMax(size.width(), size.height());

    if (length > MAX_IMAGE_LENGTH)
    {
        int width = size.width();
        int height = size.height();
        int newWidth;
        int newHeight;
        if (width > height)
        {
            newWidth = MAX_IMAGE_LENGTH;
            newHeight = (int) (height * 1.0 * MAX_IMAGE_LENGTH / width) & ~3;
        } else
        {
            newHeight = MAX_IMAGE_LENGTH;
            newWidth = (int) (width * 1.0 * MAX_IMAGE_LENGTH / height) & ~3;
        }
        QSize size = QSize(newWidth, newHeight);

        reader.setScaledSize(size);
        reader.setScaledClipRect(QRect(QPoint(0, 0), size));
    } else
    {
        int width = size.width() & ~3;
        int height = size.height() & ~3;
        reader.setClipRect(QRect(0, 0, width, height));
    }
    return reader.read();
}

unsigned char *YNH_LJX::RkUtils::Utils_JpegToNV12(char *szJpegPath, int *nJpegWidth, int *nJpegHeight)
{
    QImage image = Utils_ReadImage(szJpegPath);

    if (!image.isNull())
    {
        int width = image.size().width();
        int height = image.size().height();
        // LogV("%s %s[%d] path %s width %d height %d \n", __FILE__, __FUNCTION__, __LINE__, szJpegPath, width, height);
        uchar* pNV12 = static_cast<uchar*>(malloc(width * height * 3 / 2));

        *nJpegWidth = width;
        *nJpegHeight = height;

        RectF rect;
        rect.x = image.rect().x();
        rect.y = image.rect().y();
        rect.w = image.rect().width();
        rect.h = image.rect().height();

        int ret = Utils_RgaDrawImage(image.bits(), RK_FORMAT_BGRA_8888, rect, width, height, pNV12, RK_FORMAT_YCbCr_420_SP, rect, width,
                                     height, 0, 0);
        if(ret != 0)LogV("%s %s[%d] path %s rgaDrawImage ret %d pNV12 %p \n", __FILE__, __FUNCTION__, __LINE__, szJpegPath, ret,pNV12);

        return pNV12;
    }
    return Q_NULLPTR;
}

unsigned char *YNH_LJX::RkUtils::Utils_JpegToNV12(const QImage &image, int *nJpegWidth, int *nJpegHeight)
{
    if (!image.isNull())
    {
        int width = image.size().width();
        int height = image.size().height();
        uchar* pNV12 = (uchar*)malloc(width * height * 3 / 2);
        if(pNV12 == ISC_NULL)
        {
        	LogD("%s %s[%d] malloc size %d fail \n",__FILE__,__FUNCTION__,__LINE__,(width * height * 3 / 2));
        	return ISC_NULL;
        }
        *nJpegWidth = width;
        *nJpegHeight = height;

        RectF rect;
        rect.x = image.rect().x();
        rect.y = image.rect().y();
        rect.w = image.rect().width();
        rect.h = image.rect().height();

        int ret = Utils_RgaDrawImage((uchar *)image.bits(), RK_FORMAT_BGRA_8888, rect, width, height, pNV12, RK_FORMAT_YCbCr_420_SP, rect, width,
                                     height, 0, 0);
        if(ret != 0)LogV("%s %s[%d] rgaDrawImage ret %d pNV12 %p \n", __FILE__, __FUNCTION__, __LINE__, ret,pNV12);

        return pNV12;
    }
    return Q_NULLPTR;
}

int YNH_LJX::RkUtils::Utils_getFileSize(const char *path)
{
    int size = 0;
    struct stat buf;
    if (stat(path, &buf) < 0)
    {
        return 0;
    }
    size = buf.st_size;
    return size;
}

QString YNH_LJX::RkUtils::GetIpAddress()
{
	struct ifaddrs *pifAddr = ISC_NULL;
	struct ifaddrs *ifaphead = ISC_NULL;
	void *pAddr = ISC_NULL;
	getifaddrs(&pifAddr);

	ifaphead = pifAddr;
	while (pifAddr != ISC_NULL && pifAddr->ifa_addr != ISC_NULL)
	{
		printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
		if (pifAddr->ifa_addr->sa_family == AF_INET)
		{
			printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
			// is a valid IP4 Address
			pAddr = &((struct sockaddr_in *) pifAddr->ifa_addr)->sin_addr;
			char addrBuf[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, pAddr, addrBuf, INET_ADDRSTRLEN);
//			qDebug("%s AF_INET IP Address %s/n", pifAddr->ifa_name, addrBuf);
			if (strcasecmp("127.0.0.1", addrBuf))
			{
				freeifaddrs(ifaphead);
				return QString(addrBuf);
			}
		} else if (pifAddr->ifa_addr->sa_family == AF_INET6)
		{
			// is a valid IP6 Address
			pAddr = &((struct sockaddr_in *) pifAddr->ifa_addr)->sin_addr;
			char addrBuf[INET6_ADDRSTRLEN];
			inet_ntop(AF_INET6, pAddr, addrBuf, INET6_ADDRSTRLEN);
//			qDebug("%s AF_INET6 IP Address %s/n", pifAddr->ifa_name, addrBuf);
			if (strcasecmp("127.0.0.1", addrBuf))
			{
				freeifaddrs(ifaphead);
				return QString(addrBuf);
			}
		}
		pifAddr = pifAddr->ifa_next;
	}
	freeifaddrs(ifaphead);

	if (!access("/sys/class/net/ppp0/", F_OK))
	{
		FILE *pFile = ISC_NULL;
		pFile = popen("ifconfig ppp0", "r");
		if (pFile)
		{
			char buf[256] = { 0 };
			fread(buf, sizeof(buf), 1, pFile);
			pclose(pFile);
			char *str = strstr(buf,"inet addr:");
			if(str)
			{
				std::string strRet;
				str += strlen("inet addr:");
				for (int i = 0; i < 15; i++)
				{
					if (str[i] == ' ')
					{
						break;
					}
					strRet += str[i];
				}
				return QString(strRet.c_str());
			}
		}
	}
	return QString("");
}

double YNH_LJX::RkUtils::GetElapsedRealtimeSeconds()
{
	int nSocClkTck = 0;
	struct tms stTims;
	nSocClkTck = sysconf(_SC_CLK_TCK);
	return times(&stTims) / (double) nSocClkTck;
}
