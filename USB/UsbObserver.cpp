#include "UsbObserver.h"
#include "usbhost.h"
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <dirent.h>
#include "Application/FaceApp.h"
#include "BaiduFace/BaiduFaceManager.h"
#include "Helper/myhelper.h"
#include "MessageHandler/Log.h"
#include "PCIcore/Audio.h"
#include "PCIcore/GPIO.h"

#include <QThread>

static uint16_t gUsbPrintVid[] = { 0x6868, 0x0483 };
static uint16_t gUsbPrintPid[] = { 0x0200, 0x070b };
UsbObserver::UsbObserver(QObject *parent)
    : QThread(parent)
    , is_pause(false)
{
    memset(mUsbDevPath, 0, sizeof(mUsbDevPath));
    this->start();
}

static inline void doMountUdisk()
{
    char result[1024];

    FILE *fp =NULL;
    fp = popen("df", "r");
    if (fp == NULL)
        return;
    while (fgets(result, sizeof(result), fp) != NULL) {
        if(strncmp(result,"/dev/sd",strlen("/dev/sd"))==0){
            printf("%s %s[%d] udisk have mount! \n", __FILE__, __FUNCTION__, __LINE__);
            pclose(fp);
            return;
        }
        memset(result, 0, sizeof(result));
    }
    pclose(fp);
    char szUsbDev[7] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g' };
    char szUsbPath[64] = { 0 };
    char strCmd[256] = { 0 };

    for (int i = 0; i < sizeof(szUsbDev); i++)
    {
        memset(szUsbPath, 0, sizeof(szUsbPath));
        snprintf(szUsbPath, sizeof(szUsbPath), "/dev/sd%c", szUsbDev[i]);
        if (!access(szUsbPath, F_OK))
        {
            snprintf(strCmd, sizeof(strCmd), "mount /dev/sd%c  /udisk", szUsbDev[i]);
            system(strCmd);
            printf("%s %s[%d] strCmd %s\n", __FILE__, __FUNCTION__, __LINE__,strCmd);
        }
        for (int j = 0; j < 9; j++)
        {
            memset(szUsbPath, 0, sizeof(szUsbPath));
            snprintf(szUsbPath, sizeof(szUsbPath), "/dev/sd%c%d", szUsbDev[i],j);
            if (!access(szUsbPath, F_OK))
            {
                memset(strCmd, 0, sizeof(strCmd));
                snprintf(strCmd, sizeof(strCmd), "mount /dev/sd%c%d  /udisk", szUsbDev[i], j);
                system(strCmd);
                printf("%s %s[%d] strCmd %s \n", __FILE__, __FUNCTION__, __LINE__, strCmd);
            }
        }
    }
}

static inline int usb_device_removed(const char *devname, void* client_data)
{
    //printf("%s %s[%d] devname :%s \n",__FILE__,__FUNCTION__,__LINE__,devname);
	printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
    UsbObserver *thiz = (UsbObserver*) client_data;
    if(thiz)
    {
    	printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
        thiz->removeUsbDev(devname);
    }
    printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
    return 0;
}

static inline int usb_device_added(const char *devname, void* client_data)
{
    struct usb_descriptor_header* desc;
    struct usb_descriptor_iter iter;
    UsbObserver *thiz = (UsbObserver*) client_data;

    struct usb_device *device = usb_device_open(devname);
    if (!device)
    {
        //pthread_mutex_unlock(&mUsbObserverMutex);
        printf("%s %s[%d] usb_device_open failed \n",__FILE__,__FUNCTION__,__LINE__);
        return 0;
    }

    uint16_t vendorId = usb_device_get_vendor_id(device);
    uint16_t productId = usb_device_get_product_id(device);
    //	qDebug("yaosen %s %s[%d] vendorId :%x",__FILE__,__FUNCTION__,__LINE__,vendorId);
    //	qDebug("yaosen %s %s[%d] productId :%x", __FILE__, __FUNCTION__, __LINE__, productId);
    //	qDebug("%s %s[%d] devname %s",__FILE__,__FUNCTION__,__LINE__,devname);
    const usb_device_descriptor* deviceDesc = usb_device_get_device_descriptor(device);

    // uint8_t deviceClass = deviceDesc->bDeviceClass;
    //uint8_t deviceSubClass = deviceDesc->bDeviceSubClass;
    // uint8_t protocol = deviceDesc->bDeviceProtocol;

    //	qDebug("yaosen %s %s[%d] deviceClass :%x",__FILE__,__FUNCTION__,__LINE__,deviceClass);
    //	qDebug("yaosen %s %s[%d] protocol :%x",__FILE__,__FUNCTION__,__LINE__,protocol);
    usb_descriptor_iter_init(device, &iter);

    unsigned int nInterfaceClass = 0;
    while ((desc = usb_descriptor_iter_next(&iter)) != NULL)
    {
        if (desc->bDescriptorType == USB_DT_INTERFACE)
        {
            struct usb_interface_descriptor *interface = (struct usb_interface_descriptor *) desc;

            // push class, subclass, protocol and number of endpoints into interfaceValues vector
            //			qDebug(
            //					"%s %s[%d] interface bInterfaceNumber :%d bInterfaceClass :%d bInterfaceSubClass :%d bInterfaceProtocol :%d bNumEndpoints :%d",
            //					__FILE__, __FUNCTION__, __LINE__, interface->bInterfaceNumber, interface->bInterfaceClass, interface->bInterfaceSubClass,
            //					interface->bInterfaceProtocol, interface->bNumEndpoints);
            nInterfaceClass = interface->bInterfaceClass;
        } else if (desc->bDescriptorType == USB_DT_ENDPOINT)
        {
            //struct usb_endpoint_descriptor *endpoint = (struct usb_endpoint_descriptor *) desc;

            // push address, attributes, max packet size and interval into endpointValues vector
            //			qDebug("%s %s[%d] endpoint bEndpointAddress :%d bmAttributes :%d wMaxPacketSize :%d bInterfaceProtocol :%d ",
            //			__FILE__, __FUNCTION__, __LINE__, endpoint->bEndpointAddress, endpoint->bmAttributes, __le16_to_cpu(endpoint->wMaxPacketSize),
            //			endpoint->bInterval);
        }
    }

    if (thiz)
    {
        thiz->addUsbDev(devname, vendorId, productId, nInterfaceClass);
    }
    usb_device_close(device);
    printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
    return 0;
}

void UsbObserver::dousbStoragePlugIn()
{
    DIR* dir_pointer = NULL;
    struct dirent *pDirEntry = NULL;
    //printf("%s %s[%d]\n",__FILE__,__FUNCTION__,__LINE__);
    //检测到U盘插入 ,等待几秒，防止读U盘文件读不到
    sleep(3);
    doMountUdisk();

    dir_pointer = opendir("/udisk");
    if(!dir_pointer)
    {
        printf("opendir failed!\n");
        return ;
    }
    while( (pDirEntry = readdir(dir_pointer)) != NULL )
    {
        printf(" - %s\n", pDirEntry->d_name);
    }

    closedir(dir_pointer);
#if 0
    //资源
    if(!access("/udisk/UpData/res/",F_OK))
    {
        system("mkdir -p /param/res/ ; sync");
        system("cp -rf /udisk/UpData/res/*  /param/res/; sync");
        system("cp -rf /udisk/UpData/res/*  /isc/res/; sync");
    }
    //动态库
    if(!access("/udisk/UpData/Libs/",F_OK))system("cp -rf /udisk/UpData/Libs/*  /isc/lib/; sync");
    //应用程序
    if(!access("/udisk/UpData/App/",F_OK))system("cp -rf /udisk/UpData/App/*  /isc/bin/; sync");
    //健康码配置表
    if(!access("/udisk/HealthCodeSrv.ini",F_OK))
    {
        LogV("%s %s[%d] updata HealthCodeSrv.ini ok\n", __FILE__, __FUNCTION__, __LINE__);
        system("cp -rf /udisk/HealthCodeSrv.ini  /mnt/user/HealthCodeSrv.ini; sync");
    }
    //配置表
    if(!access("/udisk/isc_config.ini",F_OK))
    {
        LogV("%s %s[%d] updata isc_config.ini ok\n", __FILE__, __FUNCTION__, __LINE__);
        system("cp -rf /udisk/isc_config.ini  /isc/isc_config.ini; sync");
    }
    //系统参数配置
    if(!access("/udisk/parameters.ini",F_OK))
    {
        LogV("%s %s[%d] updata parameters.ini ok\n", __FILE__, __FUNCTION__, __LINE__);
        system("cp -rf /udisk/parameters.ini  /mnt/user/parameters.ini; sync");
    }
    //copy log 到U盘
#if 1
    system("cp /mnt/user/log/  /udisk/ -rf");
    system("cp /isc/bin/core  /udisk/log/ -rf");
#endif
#endif
#if 1 //U盘中根目录有 requestLog.cmd, 就触发 copy log 命令
    if(!access("/udisk/requestLog.cmd",F_OK))
    {
        LogV("%s %s[%d] Export log \n", __FILE__, __FUNCTION__, __LINE__);        
        myHelper::Utils_ExecCmd("/bin/cp  -rf /mnt/user/log  /udisk/");
        //system("cp /isc/bin/core  /udisk/log/ -rf");
    }
#endif

    if(!access("/udisk/license_offline",F_OK))
    {
        LogV("%s %s[%d] /udisk/license_offline \n", __FILE__, __FUNCTION__, __LINE__);
        QString license_0 = myHelper::ReadFile("/udisk/license_offline/license.ini");
        QString license_1 = myHelper::ReadFile("/param/license.ini");
        if( !license_0.compare(license_1) )
        {
        	LogV("%s %s[%d] /udisk/license_offline license is same \n", __FILE__, __FUNCTION__, __LINE__);
        }else
        {
            myHelper::Utils_ExecCmd("/bin/cp  -rf /param/license.ini  /param/license.ini_bak");
            myHelper::Utils_ExecCmd("/bin/cp  -rf /param/license.key  /param/license.key_bak");
            myHelper::Utils_ExecCmd("/bin/cp  -rf /udisk/license_offline/license.ini  /param/");
            myHelper::Utils_ExecCmd("/bin/cp  -rf /udisk/license_offline/license.key  /param/");
            myHelper::Utils_ExecCmd("sync");
            myHelper::Utils_ExecCmd("reboot");
        }
    }

    //升级包
    if(this->doCheckUpdateImage("/udisk/update.img") == true)
    {
    	this->doDeviceUpdate();
    }

    if(!access("/udisk/sn",F_OK))
    {
    	mkdir("/udisk/sn_ok",0777);
    	QList<QString> unusedKeys;
    	QList<QString> usedKeys;

        dir_pointer = opendir("/udisk/sn");
        if(!dir_pointer)
        {
            printf("opendir failed!\n");
            return ;
        }
        while( (pDirEntry = readdir(dir_pointer)) != NULL )
        {
			if (strlen(pDirEntry->d_name) < 10)
			{
				continue;
			}
			LogD("%s %s[%d] sn %s \n", __FILE__, __FUNCTION__, __LINE__,pDirEntry->d_name);
			unusedKeys.append(QString(pDirEntry->d_name));
        }
        closedir(dir_pointer);

        dir_pointer = opendir("/udisk/sn_ok");
        if(!dir_pointer)
        {
            printf("opendir failed!\n");
            return ;
        }
        while( (pDirEntry = readdir(dir_pointer)) != NULL )
        {
			if (strlen(pDirEntry->d_name) < 10)
			{
				continue;
			}
			LogD("%s %s[%d] ok sn %s \n", __FILE__, __FUNCTION__, __LINE__,pDirEntry->d_name);
			usedKeys.append(QString(pDirEntry->d_name));
        }
        closedir(dir_pointer);

        for(int i = 0 ;i < unusedKeys.size(); i++)
        {
        	bool isUsed = false;
        	for(int j = 0; j < usedKeys.size(); j++)
        	{
        		if(unusedKeys[i] == usedKeys[j])
        		{
        			isUsed = true;
        			break;
        		}
        	}
        	if(isUsed == false)
        	{
        		LogD("%s %s[%d] use sn %s \n", __FILE__, __FUNCTION__, __LINE__,unusedKeys[i].toStdString().c_str());
        		if (((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->algoActive(unusedKeys[i]) == true)
        		{
        			myHelper::Utils_ExecCmd("touch /udisk/sn_ok/"+unusedKeys[i]);
        			myHelper::Utils_ExecCmd("sync");
					while (1)
					{
						myHelper::Utils_ExecCmd("sync");
						sleep(3);
						myHelper::Utils_ExecCmd("sync");
						myHelper::Utils_ExecCmd("umount /udisk");
						sleep(2);
						myHelper::Utils_ExecCmd("sync");
						YNH_LJX::Audio::Audio_PlayCustomerPcm("zh", "algo_actived.wav", true);
					}
        		}
        	}
        }
    }
}
static void *UpdateStateLedThread(void *arg)
{
	int state = 0;
	//setGPIOValue(2, 26, 0); //GPIO2_D2 灭白灯
    YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Light_White, 0);
	//setGPIOValue(2, 25, 0); //GPIO2_D1 灭红灯
    YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Light_Red, 0);
	while (true)
	{
		sleep(1);	
		state = (state == 1) ? 0 : 1;
		//setGPIOValue(2, 27, state); //GPIO2_D3  升级闪绿灯
        YNH_LJX::GPIO::Device_SetDeviceState(DEVICE_Light_Green, state);

	}
}

bool UsbObserver::doCheckUpdateImage(QString path)
{
	bool isOK = false;
	if(path == "/mnt/user/update.zip")
	{
		emit sigUpDateTip(QObject::tr("UnzipTheFirmware"));//正在解压固件包..,Unzip the firmware package
		myHelper::Utils_ExecCmd("rm -rf /update/update.img; sync");
		myHelper::Utils_ExecCmd("cd /update; unzip /mnt/user/update.zip; sync");
		path = "/update/update.img";
	}

    if(!access(path.toStdString().c_str(),F_OK))
    {
        pthread_t updateThreadId;
        pthread_create(&updateThreadId,NULL,UpdateStateLedThread,NULL);        
        emit sigUpDateTip(QObject::tr("CopyFirmware"));//正在拷贝固件...
        if(path == "/udisk/update.img")
        {
        	myHelper::Utils_ExecCmd("cp -rf  /udisk/update.img /update/update.img; sync");
        }
        char buf[64] = { 0 };

        emit sigUpDateTip(QObject::tr("CheckMd5Firmware"));//正在校验固件...
        std::string cmd = "/isc/bin/update_img_check_md5 /update/update.img";
        LogD("%s %s[%d] %s \n", __FILE__, __FUNCTION__, __LINE__, cmd.c_str());
        FILE *pFile = popen(cmd.c_str(), "r");
        if (pFile)
        {
            while (fgets(buf, sizeof(buf), pFile) != NULL)
            {
                //printf("%s %s[%d] %s \n", __FILE__, __FUNCTION__, __LINE__, buf);
                if (strstr(buf, "analyticImage ok."))
                {
                    LogV("%s %s[%d] \n", __FILE__, __FUNCTION__, __LINE__);
                    isOK = true;
                    break;
                }
            }
            pclose(pFile);
        }

        LogD("%s %s[%d] isOK %d \n", __FILE__, __FUNCTION__, __LINE__, isOK);
        if(isOK == false)
		{
			myHelper::Utils_ExecCmd("rm -rf /update/update.img; sync");
			emit sigUpDateTip(QObject::tr("FirmwareVerificationFailed"));//固件校验失败,Firmware verification failed
		}
    }
    return isOK;
}

void UsbObserver::doDeviceUpdate()
{
    emit sigUpDateTip(QObject::tr("UpgradingFirmware"));//正在升级固件...,请耐心等待
    myHelper::System_Update("/update/update.img");
}

static inline void dousbStoragePlugOut()
{
    printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
}

void UsbObserver::removeUsbDev(const char *path)
{
    for (int i = 0; i < MAX_USB_DEV_NUM; i++)
    {
        if (!strncasecmp(mUsbDevPath[i].path, path, MAX_USB_DEV_NAME_LEN))
        {
            if(mUsbDevPath[i].nInterfaceClass == USB_STORAGE_INTERFACE_CLASS)
            {
                dousbStoragePlugOut();
            }
            memset(&mUsbDevPath[i], 0, sizeof(UsbDevPath));
            break;
        }
    }
    emit sigUpDateTip("");
}

void UsbObserver::addUsbDev(const char *path, unsigned int vid, unsigned int pid, unsigned int nInterfaceClass)
{
    // printf("%s %s[%d] path %s  nInterfaceClass %x \n",__FILE__,__FUNCTION__,__LINE__,path,nInterfaceClass);
    if (path == NULL)
    {
        return;
    }
    for (int i = 0; i < MAX_USB_DEV_NUM; i++)
    {
        if (strlen(mUsbDevPath[i].path) < 6)
        {
            strncpy(mUsbDevPath[i].path, path, strlen(path));
            mUsbDevPath[i].vid = vid;
            mUsbDevPath[i].pid = pid;
            mUsbDevPath[i].nInterfaceClass = nInterfaceClass;
            break;
        }
    }

    if (nInterfaceClass == USB_STORAGE_INTERFACE_CLASS)
    {
        dousbStoragePlugIn();
    }
}

bool UsbObserver::isUsbStroagePlugin()
{
    for (int i = 0; i < MAX_USB_DEV_NUM; i++)
    {
        if (mUsbDevPath[i].nInterfaceClass == USB_STORAGE_INTERFACE_CLASS)
        {
            return true;
        }
    }
    return false;
}

void UsbObserver::run()
{
    while (!isInterruptionRequested())
    {
        this->sync.lock();
        if(this->is_pause) this->pauseCond.wait(&this->sync);
        this->is_pause = true;
        //printf("%s %s[%d] \n", __FILE__, __FUNCTION__, __LINE__);
        struct usb_host_context* context = usb_host_init();
        if (!context)
        {
            LogV("%s %s[%d] usb_host_init failed \n", __FILE__, __FUNCTION__, __LINE__);
        } else
        {
            usb_host_run(context, usb_device_added, usb_device_removed, NULL, this);
        }
        printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
        this->sync.unlock();
    }
}
