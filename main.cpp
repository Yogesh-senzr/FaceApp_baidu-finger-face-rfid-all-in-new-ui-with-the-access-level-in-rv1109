#include "Application/FaceApp.h"
#include "Helper/myhelper.h"
#include "MessageHandler/Log.h"
#include <QSharedMemory>
#include <QTextCodec>
#include "Version.h"
#include <sys/time.h>
#include <iostream>
#include <icryptoauth/bdca_types.h>
#include <icryptoauth/bdca_interface.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "interface/bface_types.h"
#include "helper/face_utils.hpp"
#include "helper/image_convert.hpp"
#include "helper/timer/timer.h"
#include "interface/faceid_interface.h"
#include "interface/bd_default_param.h"
#include "baidu_face_sdk.h"
#include <QDebug>
#include <QDateTime>
#ifndef ISC_BUILD_TIME
//20240612
#define ISC_BUILD_TIME  1717171200
#endif

static inline void Utils_ExecCmd(const char* szCmd)
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

using namespace bface;
using namespace bdca;

void print_crypto_info(const struct CryptoInfo& crypto_info) {
    printf("crypto detail: \n");
    printf("slot: %d\n", crypto_info.slot_id);
    printf("digest: \n");
    for (int i = 0; i < 32; ++i) {
        printf("%02x ", crypto_info.digest[i]);
    }
    printf("\nsn:\n");

    for (int i = 0; i < 9; ++i) {
        printf("%02x ", crypto_info.sn[i]);
    }
    printf("\nrng:\n");
    for (int i = 0; i < 32; ++i) {
        printf("%02x ", crypto_info.rng[i]);
    }
    printf("\n");
}


int main(int argc, char* argv[]) 
{
    int bus_num = 3;
    BDCA_STATUS ret = BDCA_FAILURE;
	//更改时区为印度  Kolkata 时区
	// Set system timezone files
qDebug() << "App Start Time: " << QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
system("cp /usr/share/zoneinfo/Asia/Kolkata /etc/localtime");
system("echo 'Asia/Kolkata' > /etc/timezone");

// Set environment variables for current process and children
setenv("TZ", "Asia/Kolkata", 1);
tzset(); // Apply timezone change immediately

// Verify timezone is set correctly
LogD("Timezone set to: %s", getenv("TZ"));
	Log_Init(); //日记库
    LogD("FaceApp starting %s,%s,%d\n",__FILE__,__func__,__LINE__);

    time_t rawtime = time(NULL);
    //LogD("%s %s[%d] rawtime %ld \n", __FILE__, __FUNCTION__, __LINE__,rawtime);
	   if(rawtime < ISC_BUILD_TIME)
	   {
		    struct timeval tv;
		    struct timezone tz;
		    tv.tv_sec = ISC_BUILD_TIME;
		    tv.tv_usec = 0;
		    tz.tz_minuteswest = 0;    // 和格林威治时间相差的分钟数
		    tz.tz_dsttime = 0;        // 夏令时的设置
		    settimeofday(&tv, &tz);
		    //LogD("%s %s[%d] set time %ld \n", __FILE__, __FUNCTION__, __LINE__,ISC_BUILD_TIME);
		   Utils_ExecCmd("/sbin/hwclock -w -u");
	   }


    char szVersionInfo[32] = { 0 };
    bface::BFACE_STATUS status = bface::BFACE_SUCCESS;
    bface::bface_get_version(szVersionInfo, sizeof(szVersionInfo));

    /// only do authentification, don't care the details.
    //status = bdca_do_crypto_auth();
    //or
    //status = bdca_do_crypto_auth(1); //1->i2c bus num
    if(0 <= bus_num && bus_num <= 8)
    {
    	ret = bdca_do_crypto_auth(bus_num); //i2c bus num
        //printf("i2c bus number %d\n", bus_num);
    }else{
    	ret = bdca_do_crypto_auth();
    }

    if (ret != BDCA_SUCCESS) {
        //printf("bdca authentification failed\n");
//		return -1;
    } else {
        //printf("bdca authentification success\n");
    }

    CryptoInfo crypto_info;

    //status = bdca_do_crypto_auth(&crypto_info);
    //or
    //status = bdca_do_crypto_auth(&crypto_info, 1); //1->i2c bus num
    if(0 <= bus_num && bus_num <= 8)
    {
    	ret = bdca_do_crypto_auth(&crypto_info, bus_num); //i2c bus num
    }else{
    	ret = bdca_do_crypto_auth(&crypto_info);
    }
    if (ret != BDCA_SUCCESS) {
        //printf("bdca authentification failed\n");
//		return -1;
    } else {
        //printf("bdca authentification success\n");
        //print_crypto_info(crypto_info);
    }

    status = bface::bd_sdk_init("/isc/models_encrypted", false);//true,false

    //QCoreApplication::addLibraryPath("./plugins");
    //该功能可防止允许运行多次本软件
//    QSharedMemory mem("FaceApp");
//    if (!mem.create(1)) { return 0; }
#ifdef Q_OS_LINUX
    //qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));
    #if 0    
    qputenv("QT_IM_MODULE",QByteArray("YNHInput"));
    #else 
    qputenv("QT_IM_MODULE",QByteArray("Qt5Input"));
    #endif 
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    /*
    1.QT_AUTO_SCREEN_SCALE_FACTOR [boolean] 基于显示器的像素密度实现自动缩放。 这不会改变点大小字体的大小，因为点是物理单位。 多个屏幕可能会获得不同的比例因子。
    2.QT_SCALE_FACTOR [numeric] 定义整个应用程序的全局比例因子，包括点大小的字体。
    3.QT_SCREEN_SCALE_FACTORS [list] 指定每个屏幕的比例因子。 这不会改变点大小字体的大小。 此环境变量主要用于调试
    */
    //环境变量启用
    //qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "1");
    //属性方式启用
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#else
    QCoreApplication::setAttribute(Qt::AA_Use96Dpi);
#endif
#if defined(_MSC_VER) && (_MSC_VER >= 1600)
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
#else
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
#endif

    QCoreApplication::setOrganizationName(QString("FaceApp"));
    QCoreApplication::setApplicationName(QString("FaceApp"));

    myHelper::Utils_ExecCmd("/sbin/insmod /vendor/lib/modules/wiegand_input.ko");
    myHelper::Utils_ExecCmd("/sbin/insmod /vendor/lib/modules/wiegand_output.ko");
    FaceApp a(argc, argv);
    return a.exec();
}
