#include "Log.h"
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <list>
#include <string>
#include <time.h>

#include <QString>
#ifdef Q_OS_LINUX
extern FILE *stdout;
#endif
static FILE *g_LogFile = NULL;
static int g_nDebugLevel = DEBUG_LEVEL_ALL;

static inline void Utils_ExecCmd(const char *szCmd)
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

long LogTime() {
	// 真实时间
	//struct timeval tv;
	//gettimeofday(&tv, NULL);
	//return tv.tv_sec * 1000 + tv.tv_usec / 1000;
	// 开机毫秒数
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

void Log_Init()
{
    if (g_LogFile == NULL)
    {
        char szDate[64] = { 0 };
        time_t nSeconds;
        struct tm * pTM;
        time(&nSeconds);
        pTM = localtime(&nSeconds);
        /**判断log目录大小，如果太大了，就删掉，避免log太大存满了空间**/

        DIR *pDir = NULL;
        struct dirent *pEntry = NULL;
        struct stat statbuf;
        uint64_t nTotalSize = 0;
        std::list<std::string> deleteFilePathList;
        std::list<uint64_t> deteleFileSizeList;

        if ((pDir = opendir("/mnt/user/log/")) != NULL)
        {
            while ((pEntry = readdir(pDir)) != NULL)
            {
                if (strcmp(".", pEntry->d_name) == 0 || strcmp("..", pEntry->d_name) == 0)
                {
                    continue;
                }

                std::string path = "/mnt/user/log/" +std::string(pEntry->d_name);
#ifdef Q_OS_LINUX
                lstat(path.c_str(), &statbuf);
#else
                stat(path.c_str(), &statbuf);
#endif
                deleteFilePathList.push_back(path);
                deteleFileSizeList.push_back(statbuf.st_size);
                nTotalSize += statbuf.st_size;
//                printf("%s %s[%d] path %s size :%d \n", __FILE__, __FUNCTION__, __LINE__, path.c_str(),statbuf.st_size);
            }
            closedir(pDir);
        }
        printf("%s %s[%d] nTotalSize :%lld \n", __FILE__, __FUNCTION__, __LINE__, nTotalSize);
        if (nTotalSize > 200 * 1024 * 1024)
        {
            Utils_ExecCmd("rm -rf /mnt/user/log");
            Utils_ExecCmd("sync");
//            printf("%s %s[%d] delete  /mnt/user/log/ \n", __FILE__, __FUNCTION__, __LINE__);
        }
#ifdef Q_OS_LINUX
        mkdir("/mnt/user/log/", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#else
        mkdir("/mnt/user/log/");
#endif

        /* 系统日期,格式:YYYMMDDHHmmSS */
        sprintf(szDate, "/mnt/user/log/%04d%02d%02d%02d%02d%02d.log", pTM->tm_year + 1900, pTM->tm_mon + 1, pTM->tm_mday, pTM->tm_hour,
                pTM->tm_min, pTM->tm_sec);

        g_LogFile = fopen(szDate, "ab");

        std::list<std::string>::iterator iter0 = deleteFilePathList.begin();
        std::list<uint64_t>::iterator iter1 = deteleFileSizeList.begin();
        for (int i = 0; i < deleteFilePathList.size(); i++)
        {
//            LogD("%s %s[%d] delete  %s  %lld \n", __FILE__, __FUNCTION__, __LINE__, (*iter0).c_str(), (*iter1));
            ++iter0;
            ++iter1;
        }
        LogD("%s %s[%d] nTotalSize  %lld  \n", __FILE__, __FUNCTION__, __LINE__,nTotalSize);
    }
}

void Log_SetDebugLevel(int nDebugLevel)
{
    g_nDebugLevel = nDebugLevel;
}

void Log_Exit()
{
    if (g_LogFile)
    {
        fflush(g_LogFile);
        fclose(g_LogFile);
        g_LogFile = NULL;
    }
    if (stdout)
    {
        fflush(stdout);
        fclose(stdout);
    }
}

void LogV(const char *format, ...)
{
    if (g_nDebugLevel >= DEBUG_LEVEL_ALL)
    {
        va_list arg_ptr;
        va_start(arg_ptr, format);

        // time
        long tick = LogTime();
        int msecs = (int)(tick % 1000);
        tick /= 1000;
        int secs = (int)(tick % 60);
        tick /= 60;
        int minutes = (int)(tick % 60);
        tick /= 60;
        int hours = (int)tick;
        // buf
        char buf[64] = { 0 };
        sprintf(buf, "[%02d:%02d:%02d.%03d]", hours, minutes, secs, msecs);
        printf("%-16s", buf);
                
        if (g_LogFile)
        {
            vfprintf(g_LogFile, format, arg_ptr);
            fflush(g_LogFile);
        }
        if (stdout)
        {
            vfprintf(stdout, format, arg_ptr);
            fflush(stdout);
        }
        va_end(arg_ptr);
    }
}

void LogD(const char *format, ...)
{
    if (g_nDebugLevel >= DEBUG_LEVEL_DEBUG)
    {
        va_list arg_ptr;
        va_start(arg_ptr, format);
#if 0        
        // time
        long tick = LogTime();
        int msecs = (int)(tick % 1000);
        tick /= 1000;
        int secs = (int)(tick % 60);
        tick /= 60;
        int minutes = (int)(tick % 60);
        tick /= 60;
        int hours = (int)tick;
        // buf
        char buf[64] = { 0 };
        sprintf(buf, "[%02d:%02d:%02d.%03d]", hours, minutes, secs, msecs);
        printf("%-16s", buf);
#endif 
        if (g_LogFile)
        {
            vfprintf(g_LogFile, format, arg_ptr);
            fflush(g_LogFile);
        }
        if (stdout)
        {
            vfprintf(stdout, format, arg_ptr);
            fflush(stdout);
        }
        va_end(arg_ptr);

    }
}

void LogE(const char *format, ...)
{
    if (g_nDebugLevel >= DEBUG_LEVEL_ERROR)
    {
        va_list arg_ptr;
        va_start(arg_ptr, format);
        if (g_LogFile)
        {
            vfprintf(g_LogFile, format, arg_ptr);
            fflush(g_LogFile);
        }
        if (stdout)
        {
            vfprintf(stdout, format, arg_ptr);
            fflush(stdout);
        }
        va_end(arg_ptr);
    }
}

void LogDelete(int day)
{

    time_t nCurrentSeconds, nOldSeconds;
    struct tm * pTM;
    time(&nCurrentSeconds);
    char filePath[100];
    DIR *dir = NULL;
    struct dirent *ptr = NULL;
    if ((dir = opendir("/mnt/user/log/")) == NULL)
    {
        LogV("opendir /mnt/user/log/ failed!");
        return;
    }

    bool needCreateLog = true;

    while ((ptr = readdir(dir)) != NULL)
    {
        if (ptr->d_name[0] == '.')
            continue;
        //LogV("name is %s \n",ptr->d_name);
        char Path[100];
        bool isMatch = false;
        for (int i = 0; i < day; i++)
        {
            nOldSeconds = nCurrentSeconds - i * 3600 * 24;
            pTM = localtime(&nOldSeconds);
            sprintf(Path, "%04d%02d%02d", pTM->tm_year + 1900, pTM->tm_mon + 1, pTM->tm_mday);
            if (strncmp(ptr->d_name, Path, strlen(Path)) == 0)
            {
                isMatch = true;
                //当今天的log已经创建的时候，无需再创建log
                if (i == 0)
                {
                    needCreateLog = false;
                }
                break;
            }
        }
        if (isMatch || strstr(ptr->d_name, "jserver"))
        {
            continue;
        } else
        {
            memset(filePath, 0, sizeof(filePath));
            sprintf(filePath, "rm -rf /mnt/user/log/%s", ptr->d_name);
            //LogV("filePath %s \n", filePath);
            system(filePath);
        }
    }
    if (dir)
        closedir(dir);

    if (needCreateLog)
    {
        if (g_LogFile)
        {
            fflush(g_LogFile);
            fclose(g_LogFile);
            g_LogFile = NULL;
        }
        Log_Init();
    }

}
