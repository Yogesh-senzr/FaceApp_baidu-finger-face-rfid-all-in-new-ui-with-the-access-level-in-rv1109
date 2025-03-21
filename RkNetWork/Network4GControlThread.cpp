#include "Network4GControlThread.h"
#include "Helper/myhelper.h"
#include "MessageHandler/Log.h"
#include "Config/ReadConfig.h"

#include <netserver.h>
#include <dbserver.h>

Network4GControlThread::Network4GControlThread(QObject *parent) :
		QThread(parent)
{
	this->start();
}

Network4GControlThread::~Network4GControlThread()
{
	this->requestInterruption();
	this->pauseCond.wakeOne();
	this->quit();
	this->wait();
}
#define ISC_NULL                 0L
void Network4GControlThread::run()
{
	this->sync.lock();

	LogD("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
	char *json_str = NULL;
	json_str = dbserver_ethernet_power_set(0);
	LogD("%s %s[%d] dbserver_ethernet_power_set 0 %s \n", __FILE__, __FUNCTION__, __LINE__, json_str);
	if (json_str)
	{printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
		free(json_str);
	}

	json_str = dbserver_wifi_power_set(0);
	LogD("%s %s[%d] dbserver_wifi_power_set 0 %s \n", __FILE__, __FUNCTION__, __LINE__, json_str);
	if (json_str)
	{printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
		free(json_str);
	}

	myHelper::Utils_ExecCmd("/sbin/ifconfig ppp0 down");
	myHelper::Utils_ExecCmd("/sbin/ifconfig p2p0 down");
	myHelper::Utils_ExecCmd("/sbin/ifconfig usb0 down");
	myHelper::Utils_ExecCmd("/sbin/ifconfig eth0 up"); //要保留eth0,以便激活
	myHelper::Utils_ExecCmd("/sbin/ifconfig eth1 down");
	myHelper::Utils_ExecCmd("/sbin/ifconfig wlan0 down");
	myHelper::Utils_ExecCmd("/sbin/insmod /vendor/lib/modules/GobiNet.ko");
	
	if (ReadConfig::GetInstance()->getNetwork_Manager_Mode() ==3 )
	{
		while (true)
		{		
			//printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
			myHelper::Utils_ExecCmd("if [ -e '/dev/ttyUSB2' ]; then  /isc/bin/yuga.lte-pppd &  else echo 'not' ;fi ");
			sleep(5);

			FILE *pFile = ISC_NULL;
			pFile = popen("ifconfig p2p0 | grep 'inet addr' | wc -l", "r");
			if (pFile)
			{
				printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
				char buf[10] = { 0 };
				fread(buf, sizeof(buf), 1, pFile);
				pclose(pFile);
				
				if (atoi(buf) > 0 )
				{
					printf("%s %s[%d] buf=%d\n",__FILE__,__FUNCTION__,__LINE__,atoi(buf));
					break; 
				}
			}
			
			//printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
			sleep(15);		//100
			printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
		}
	}
	
	this->sync.unlock();
}
