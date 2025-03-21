#include "Application/FaceApp.h"
#include "Helper/myhelper.h"
#include "MessageHandler/Log.h"
#include <QSharedMemory>
#include <QTextCodec>
#include "Version.h"
#include <sys/time.h>
#include <time.h>

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

int main(int argc, char *argv[])
{
	//更改时区为印度  Kolkata 时区
	system("cp /usr/share/zoneinfo/Asia/Kolkata /etc/localtime");
	Log_Init(); //日记库
    LogD("FaceApp starting %s,%s,%d\n",__FILE__,__func__,__LINE__);

    time_t rawtime = time(NULL);
    LogD("%s %s[%d] rawtime %ld \n", __FILE__, __FUNCTION__, __LINE__,rawtime);
	   if(rawtime < ISC_BUILD_TIME)
	   {
		    struct timeval tv;
		    struct timezone tz;
		    tv.tv_sec = ISC_BUILD_TIME;
		    tv.tv_usec = 0;
		    tz.tz_minuteswest = 0;    // 和格林威治时间相差的分钟数
		    tz.tz_dsttime = 0;        // 夏令时的设置
		    settimeofday(&tv, &tz);
		    LogD("%s %s[%d] set time %ld \n", __FILE__, __FUNCTION__, __LINE__,ISC_BUILD_TIME);
		   Utils_ExecCmd("/sbin/hwclock -w -u");
	   }
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
