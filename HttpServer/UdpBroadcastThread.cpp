#include "UdpBroadcastThread.h"
#include "local_service.h"
#include "PCIcore/Watchdog.h"
#include "MessageHandler/Log.h"
#include "SharedInclude/GlobalDef.h"
#include "PCIcore/RkUtils.h"
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <sys/select.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <linux/if.h>
#include <string.h>
#include <arpa/inet.h>

using namespace std;

#define IP_SIZE     16

UdpBroadcastThread::UdpBroadcastThread(QObject *parent) :
		QThread(parent)
{
	this->start();
}

UdpBroadcastThread::~UdpBroadcastThread()
{
	this->requestInterruption();
	this->pauseCond.wakeOne();

	this->quit();
	this->wait();
}

static int get_local_ip(const char *eth_inf, char *ip)
{
	int sd;
	struct sockaddr_in sin;
	struct ifreq ifr;

	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (-1 == sd)
	{
		return -1;
	}

	strncpy(ifr.ifr_name, eth_inf, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0;

	// if error: No such device
	if (ioctl(sd, SIOCGIFADDR, &ifr) < 0)
	{
		close(sd);
		return -1;
	}

	memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
	snprintf(ip, IP_SIZE, "%s", inet_ntoa(sin.sin_addr));

	close(sd);
	return 0;
}

int get_broad_addr(const char *eth_inf, char *broad_ip)
{
	int sd;
	struct sockaddr_in sin;
	struct ifreq ifr;

	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (-1 == sd)
	{
		//printf("socket error: %s\n", strerror(errno));
		return -1;
	}

	strncpy(ifr.ifr_name, eth_inf, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0;

	// if error: No such device
	if (ioctl(sd, SIOCGIFBRDADDR, &ifr) < 0)
	{
		//printf("ioctl error: %s\n", strerror(errno));
		close(sd);
		return -1;
	}

	memcpy(&sin, &ifr.ifr_broadaddr, sizeof(sin));
	snprintf(broad_ip, IP_SIZE, "%s", inet_ntoa(sin.sin_addr));

	close(sd);
	return 0;
}

static void do_sendBroadcast(char *ipAddr)
{
	char brAddr[128] = { 0 };
	int sockfd;
	struct sockaddr_in des_addr;
	int r;
	//char sendline[1024] = {"Hello yinnuoheng"};
	std::string sendline = "hello this is yinnuoheng ip:";
	std::string sendline2 = ipAddr;
	sendline.append(sendline2);
	const int on = 1;
	get_broad_addr("eth0", brAddr);

//	printf("send udp eth0 broad meg:%s\n",brAddr);
	if (strlen(brAddr) < 4)
	{
		get_broad_addr("wlan0", brAddr);
		//printf("send udp wlan0 broad meg:%s\n",brAddr);
	}
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)); //设置套接字选项
	bzero(&des_addr, sizeof(des_addr));
	des_addr.sin_family = AF_INET;
	des_addr.sin_addr.s_addr = inet_addr(brAddr); //广播地址
	des_addr.sin_port = htons(9999);
	r = sendto(sockfd, sendline.c_str(), sendline.length(), 0, (struct sockaddr*) &des_addr, sizeof(des_addr));
	if (r <= 0)
	{
//		LogV("%s %s[%d] error %s \n", __FILE__, __FUNCTION__, __LINE__, strerror(errno));
	}
	close(sockfd);
}

void UdpBroadcastThread::run()
{
	LogV("get IP addr.\n");
	char ipAddr[128] = { 0 };
	char ip_new[128] = { 0 };
	int nBroadcastCount = 0;
	sleep(4);
	while (strlen(ipAddr) < 4)
	{
		if (get_local_ip("eth0", ipAddr) == 0 && strlen(ipAddr) > 4)
		{
			LogV("machine eth0 IP:%s\n", ipAddr);
			break;
		}
		if (get_local_ip("wlan0", ipAddr) == 0 && strlen(ipAddr) > 4)
		{
			LogV("machine wlan0 IP:%s\n", ipAddr);
			break;
		}
		sleep(1);
	}

	local_service_init(ipAddr);

	while (!isInterruptionRequested())
	{
		this->sync.lock();

		if (nBroadcastCount++ >= 5)
		{
			do_sendBroadcast(ipAddr);
			nBroadcastCount = 0;
		}

		get_local_ip("eth0", ip_new);
		if (strlen(ip_new) < 4)
		{
			get_local_ip("wlan0", ip_new);
		}
		if (memcmp(ip_new, ipAddr, strlen(ip_new)) != 0)
		{
			LogD("%s %s[%d] ip_new %s \n", __FILE__, __FUNCTION__, __LINE__, ip_new);
			memset(ipAddr, 0, 128);
			memcpy(ipAddr, ip_new, strlen(ip_new));
		}

#ifdef _old_version
		{
			int i = 0;
			char sn_path[100];
			for (int i = 0; i < 10; i++)
			{
				memset(sn_path, 0, sizeof(sn_path));
				sprintf(sn_path, "/media/usb%d/sn", i);
				if (!access(sn_path, F_OK))
				{
					break;
				}
			}
			if (!access(sn_path, F_OK))
			{
				DIR* pDir = ISC_NULL;
				struct dirent *pDirEntry = NULL;
				pDir = opendir(sn_path);
				if (pDir != ISC_NULL)
				{
					while ((pDirEntry = readdir(pDir)) != ISC_NULL)
					{
						if (strlen(pDirEntry->d_name) < 10)
						{
							continue;
						}
						LogD("%s %s[%d] file %s \n", __FILE__, __FUNCTION__, __LINE__, pDirEntry->d_name);

						int ret = ISC_ERROR;
						ret = Ai_AlgoActive(pDirEntry->d_name, strlen(pDirEntry->d_name));
						if (ret == 1)
						{
							//算法激活状态还没更新,延迟
							sleep(5);
						}
						YNH_LJX::RkUtils::Utils_ExecCmd("sync");
						if (ret == ISC_OK)
						{
							string mkdir_dir = std::string(sn_path) + std::string("/sn_ok/");
							mkdir(mkdir_dir.c_str(), 0777);
							std::string cmd = "touch " + std::string(sn_path) + std::string("/sn_ok/") + std::string(pDirEntry->d_name);
							YNH_LJX::RkUtils::Utils_ExecCmd(cmd.c_str());
							YNH_LJX::RkUtils::Utils_ExecCmd("sync");

							cmd = "touch " + std::string("/param/") + std::string(pDirEntry->d_name) + std::string("_online_license.txt");
							YNH_LJX::RkUtils::Utils_ExecCmd(cmd.c_str());
							YNH_LJX::RkUtils::Utils_ExecCmd("sync");
							while (1)
							{
								std::string cmd = "rm -rf " + std::string(sn_path) + std::string("/") + std::string(pDirEntry->d_name);
								YNH_LJX::RkUtils::Utils_ExecCmd(cmd.c_str());
								YNH_LJX::RkUtils::Utils_ExecCmd("sync");
								sleep(3);
								Audio_PlayPCMFileAsync(BIT_WIDTH_16, SAMPLE_RATE_44100, SOUND_MODE_STEREO, "/isc/res/zh/algo_actived.wav",
										true);
							}
						}
					}
					closedir(pDir);
				}
			}
		}
#endif

		this->pauseCond.wait(&this->sync, 1000);
		this->sync.unlock();
	}
}
