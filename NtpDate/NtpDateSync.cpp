#include "NtpDateSync.h"
#include "MessageHandler/Log.h"

NtpDateSync::NtpDateSync(QObject *parent)
    : QThread(parent)
    , is_pause(true)
{
    this->start();
}

NtpDateSync::~NtpDateSync()
{
    this->requestInterruption();
    this->is_pause = false;
    this->pauseCond.wakeOne();
    this->quit();
    this->wait();
}

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

void NtpDateSync::run()
{
    while (!isInterruptionRequested())
    {
        this->sync.lock();
        if (this->is_pause)this->pauseCond.wait(&this->sync);
#ifdef Q_OS_LINUX
        //Utils_ExecCmd("/isc/bin/ntpdate  202.112.10.60");//有
        Utils_ExecCmd("/isc/bin/ntpdate  185.209.85.222");
	Utils_ExecCmd("/sbin/hwclock -w -u");

        Utils_ExecCmd("/isc/bin/ntpdate  ntp.sjtu.edu.cn"); //ntp.sjtu.edu.cn 202.120.2.101 (上海交通大学网络中心NTP服务器地址）
        //Utils_ExecCmd("/isc/bin/ntpdate  202.120.2.101"); //ntp.sjtu.edu.cn 202.120.2.101 (上海交通大学网络中心NTP服务器地址）
	Utils_ExecCmd("/sbin/hwclock -w -u");


        Utils_ExecCmd("/isc/bin/ntpdate  s1a.time.edu.cn"); //s1a.time.edu.cn 北京邮电大学
	Utils_ExecCmd("/sbin/hwclock -w -u");

       //Utils_ExecCmd("/isc/bin/ntpdate  s1b.time.edu.cn"); //s1b.time.edu.cn 清华大学
        //Utils_ExecCmd("/isc/bin/ntpdate  s1c.time.edu.cn"); //s1c.time.edu.cn 北京大学
        //Utils_ExecCmd("/isc/bin/ntpdate  s1d.time.edu.cn"); //s1d.time.edu.cn 东南大学
        //Utils_ExecCmd("/isc/bin/ntpdate  s1e.time.edu.cn"); //s1e.time.edu.cn 清华大学
        //Utils_ExecCmd("/isc/bin/ntpdate  s2a.time.edu.cn"); //s2a.time.edu.cn 清华大学
        //Utils_ExecCmd("/isc/bin/ntpdate  s2b.time.edu.cn"); //s2b.time.edu.cn 清华大学
        Utils_ExecCmd("/isc/bin/ntpdate  s2c.time.edu.cn"); //s2c.time.edu.cn 北京邮电大学
	Utils_ExecCmd("/sbin/hwclock -w -u");

        //Utils_ExecCmd("/isc/bin/ntpdate  s2d.time.edu.cn"); //s2d.time.edu.cn 西南地区网络中心
        //Utils_ExecCmd("/isc/bin/ntpdate  s2e.time.edu.cn"); //s2e.time.edu.cn 西北地区网络中心
        Utils_ExecCmd("/isc/bin/ntpdate  s2f.time.edu.cn"); //s2f.time.edu.cn 东北地区网络中心
	Utils_ExecCmd("/sbin/hwclock -w -u");

        //Utils_ExecCmd("/isc/bin/ntpdate  s2g.time.edu.cn"); //s2g.time.edu.cn 华东南地区网络中心
        //Utils_ExecCmd("/isc/bin/ntpdate  s2h.time.edu.cn"); //s2h.time.edu.cn 四川大学网络管理中心
        //Utils_ExecCmd("/isc/bin/ntpdate  s2j.time.edu.cn"); //s2j.time.edu.cn 大连理工大学网络中心
        //Utils_ExecCmd("/isc/bin/ntpdate  s2k.time.edu.cn"); //s2k.time.edu.cn CERNET桂林主节点
        //Utils_ExecCmd("/isc/bin/ntpdate  s2m.time.edu.cn"); //s2m.time.edu.cn 北京大学

        char szNow[128] = { 0 };
        time_t t2;
        time(&t2);
        struct tm *p;
        p = localtime(&t2);
        sprintf(szNow, "%04d%02d%02d%02d%02d%02d", (1900 + p->tm_year), (1 + p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
        LogV("%s %s[%d] szNow %s \n", __FILE__, __FUNCTION__, __LINE__, szNow);
#endif
        this->is_pause = true;
        this->sync.unlock();
    }
}
