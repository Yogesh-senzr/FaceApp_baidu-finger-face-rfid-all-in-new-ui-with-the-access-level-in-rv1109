#ifndef WATCHDOG_H
#define WATCHDOG_H

namespace YNH_LJX{
class Watchdog
{
public:
    static int WatchDog_OpenWatchDog();
    static void WatchDog_FeedWatchDog(unsigned int nTimeOutSec);
    static void WatchDog_CloseWatchDog();
};
}
#endif // WATCHDOG_H
