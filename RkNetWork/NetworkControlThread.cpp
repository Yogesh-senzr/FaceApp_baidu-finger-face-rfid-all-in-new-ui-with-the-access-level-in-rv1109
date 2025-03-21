#if _MSC_VER >= 1600    // VC2010
#pragma  execution_character_set("UTF-8")
#endif
#include "NetworkControlThread.h"

#include "Application/FaceApp.h"
#include "BaiduFace/BaiduFaceManager.h"
#include "json-c/json.h"
#include <netserver.h>
#include <dbserver.h>
#include "Config/ReadConfig.h"


#include <QtCore/QDebug>
#include <QtCore/QJsonParseError>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtNetwork/QNetworkConfigurationManager>
#include <unistd.h>

NetworkControlThread::NetworkControlThread(QObject *parent)
    : QThread(parent)
    , is_pause(true)
    , mSearchMode(1)
    , mWaitLinkCnt(255)
{
    this->start();
}

NetworkControlThread::~NetworkControlThread()
{
    this->requestInterruption();
    this->pauseCond.wakeOne();
    this->is_pause = false;
    this->quit();
    this->wait();
}

static inline bool isVaildIp(unsigned char *ip)
{
    int dots = 0; /*字符.的个数*/
    int setions = 0; /*ip每一部分总和（0-255）*/

    if (NULL == ip || *ip == '.')
    { /*排除输入参数为NULL, 或者一个字符为'.'的字符串*/
        return false;
    }
    while (*ip)
    {
        if (*ip == '.')
        {
            dots++;
            if (setions >= 0 && setions <= 255)
            { /*检查ip是否合法*/
                setions = 0;
                ip++;
                continue;
            }
            return false;
        } else if (*ip >= '0' && *ip <= '9')
        { /*判断是不是数字*/
            setions = setions * 10 + (*ip - '0'); /*求每一段总和*/
        } else
            return false;
        ip++;
    }
    /*判断IP最后一段是否合法*/
    if (setions >= 0 && setions <= 255)
    {
        if (dots == 3)
        {
            return true;
        }
    }
    return false;
}

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

void NetworkControlThread::openLan()
{//打开网口
	printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
    free(dbserver_ethernet_power_set(1));
    printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
}

void NetworkControlThread::openWifi()
{//打开wifi
	printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
    free(dbserver_wifi_power_set(1));
}

void NetworkControlThread::closeLan()
{//关闭网口
    system("ifconfig eth0 down");
    printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
    free(dbserver_ethernet_power_set(0));
}

void NetworkControlThread::closeWifi()
{//关闭wifi
    system("ifconfig wlan0 down");
    printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
    free(dbserver_wifi_power_set(0));
}

void NetworkControlThread::setNetworkType(const int &type)
{
    system("ifconfig p2p0 down");
    system("ifconfig ppp0 down");
	system("ifconfig usb0 down");
	
    switch(type)
    {
    case 1:
    {//LAN
        system("ifconfig eth1 down");    
        this->closeWifi();
        this->openLan();
    }break;
    case 2:
    {//WIFI
		if(!access("/param/license.ini",0)) //存在离线激活的license.ini
		{
			if (((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->getAlgoFaceInitState())
			{
				this->closeLan();
				this->openWifi();					
			}
				
		} else 
		{
			this->closeLan();
			this->openWifi();			
		}
	
    }break;
    }
}

static inline void parse_network_service_getJson(const QByteArray &he_now_json)
{
    QJsonParseError err_rpt;
    QJsonDocument  root_Doc = QJsonDocument::fromJson(he_now_json, &err_rpt);//字符串格式化为JSON
    if(err_rpt.error != QJsonParseError::NoError)
    {
        qDebug() << "JSON格式错误";
        return;
    }else//JSON格式正确
    {
        const QJsonObject& root_obj = root_Doc.object();
        const QJsonArray& root_Array = root_obj.value("jData").toArray();
        int cnt = root_Array.count();
        for(int i = 0; i<cnt; i++)
        {
            const QJsonObject &obj = root_Array.at(i).toObject();
            dbserver_network_service_delete( obj.value("sService").toString().toLatin1().data());
        }
    }
}

//{ "iReturn": 0, "sErrMsg": "", "jData": [ { "id": 1, "sService": "wifi_2cd26b0d6c75_594e48_managed_psk", "sPassword": "yinnuoheng", "iFavorite": 1, "iAutoconnect": 1 }, { "id": 2, "sService": "", "sPassword": "yinnuoheng", "iFavorite": 1, "iAutoconnect": 1 }, { "id": 4, "sService": "wifi_2ed26b0d6c75_54656e64615f323542314130_managed_none", "sPassword": "yinnuoheng", "iFavorite": 1, "iAutoconnect": 1 } ] }
void NetworkControlThread::DisconnectAllWifi()
{
    char *json_str = NULL;
    json_str = dbserver_network_service_get(NULL); //获取当前连接的网络
    if (json_str != NULL)
        parse_network_service_getJson(QByteArray().append(json_str));

    printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
    free(json_str);
}

//dbserver_network_service_get:{ "iReturn": 0, "sErrMsg": "", "jData": [ { "id": 9, "sService": "wifi_2cd26b0d6c75_594e48_managed_psk", "sPassword": "yinnuoheng", "iFavorite": 1, "iAutoconnect": 1 } ] }
//netserver_get_config:[ { "sService": "wifi_2cd26b0d6c75_594e48_managed_psk", "sDNS": "192.168.8.1", "sSecurity": "psk", "ipv4": { "sV4Method": "dhcp", "sV4Address": "192.168.8.34", "sV4Netmask": "255.255.255.0", "sV4Gateway": "192.168.8.1" }, "link": { "sNicSpeed": "auto", "sInterface": "wlan0", "sAddress": "2C:D2:6B:0D:6C:75" }, "dbconfig": { "sPassword": "yinnuoheng", "iFavorite": 1, "iAutoconnect": 1 } } ]
//netserver_get_service:[ { "sService": "wifi_2cd26b0d6c75_594e48_managed_psk", "sType": "wifi", "sState": "online", "sName": "YNH", "sSecurity": "psk", "Favorite": 1, "Strength": 65, "dbconfig": { "iPower": 1 } }, { "sService": "wifi_2cd26b0d6c75_54656e64615f354232383830_managed_psk", "sType": "wifi", "sState": "idle", "sName": "Tenda_5B2880", "sSecurity": "psk", "Favorite": 0, "Strength": 64, "dbconfig": { "iPower": 1 } }, { "sService": "wifi_2cd26b0d6c75_54656e64615f323542314130_managed_none", "sType": "wifi", "sState": "idle", "sName": "Tenda_25B1A0", "sSecurity": "none", "Favorite": 0, "Strength": 60, "dbconfig": { "iPower": 1 } }, { "sService": "wifi_2cd26b0d6c75_hidden_managed_psk", "sType": "wifi", "sState": "idle", "sName": "", "sSecurity": "psk", "Favorite": 0, "Strength": 58, "dbconfig": { "iPower": 1 } }, { "sService": "wifi_2cd26b0d6c75_4449524543542d4d434445534b544f502d423333414f46326d735054_managed_psk", "sType": "wifi", "sState": "idle", "sName": "DIRECT-MCDESKTOP-B33AOF2msPT", "sSecurity": "psk wps", "Favorite": 0, "Strength": 56, "dbconfig": { "iPower": 1 } }, { "sService": "wifi_2cd26b0d6c75_4368696e614e65742d494e4e4f4849_managed_psk", "sType": "wifi", "sState": "idle", "sName": "ChinaNet-INNOHI", "sSecurity": "psk wps", "Favorite": 0, "Strength": 50, "dbconfig": { "iPower": 1 } }, { "sService": "wifi_2cd26b0d6c75_6d6569687561303037_managed_psk", "sType": "wifi", "sState": "idle", "sName": "meihua007", "sSecurity": "psk", "Favorite": 0, "Strength": 48, "dbconfig": { "iPower": 1 } }, { "sService": "wifi_2cd26b0d6c75_4368696e614e65742d6b4d7341_managed_psk", "sType": "wifi", "sState": "idle", "sName": "ChinaNet-kMsA", "sSecurity": "psk wps", "Favorite": 0, "Strength": 46, "dbconfig": { "iPower": 1 } }, { "sService": "wifi_2cd26b0d6c75_4368696e614e65742d76743941_managed_psk", "sType": "wifi", "sState": "idle", "sName": "ChinaNet-vt9A", "sSecurity": "psk wps", "Favorite": 0, "Strength": 46, "dbconfig": { "iPower": 1 } }, { "sService": "wifi_2cd26b0d6c75_6b6974_managed_psk", "sType": "wifi", "sState": "idle", "sName": "kit", "sSecurity": "psk wps", "Favorite": 0, "Strength": 45, "dbconfig": { "iPower": 1 } }, { "sService": "wifi_2cd26b0d6c75_4855415745492d464d56_managed_psk", "sType": "wifi", "sState": "idle", "sName": "HUAWEI-FMV", "sSecurity": "psk wps", "Favorite": 0, "Strength": 41, "dbconfig": { "iPower": 1 } }, { "sService": "wifi_2cd26b0d6c75_4356525f49365f363636_managed_psk", "sType": "wifi", "sState": "idle", "sName": "CVR_I6_666", "sSecurity": "psk", "Favorite": 0, "Strength": 34, "dbconfig": { "iPower": 1 } }, { "sService": "wifi_2cd26b0d6c75_4368696e614e65742d63635a55_managed_psk", "sType": "wifi", "sState": "idle", "sName": "ChinaNet-ccZU", "sSecurity": "psk wps", "Favorite": 0, "Strength": 0, "dbconfig": { "iPower": 1 } }, { "sService": "wifi_2cd26b0d6c75_47464b4a32_managed_psk", "sType": "wifi", "sState": "idle", "sName": "GFKJ2", "sSecurity": "psk", "Favorite": 0, "Strength": 0, "dbconfig": { "iPower": 1 } } ]
QString NetworkControlThread::getCurrentWifiName()
{
    // QString essid = "";
    // QString mWifiService ="";
    //char *json_str = NULL;
    //json_str = dbserver_network_service_get(NULL); //获取当前连接的网络
    //json_str = netserver_get_config("wifi_2cd26b0d6c75_594e48_managed_psk");
    //json_str = netserver_get_service("wifi");

    // printf("wifi config %s \n",json_str);
    //	if (json_str != ISC_NULL) {
    //		Json::Reader reader;
    //		Json::Value root;
    //		if (reader.parse(json_str, root)) {
    //			if(root["jData"].size()>0){
    //				for(int i=0;i<root["jData"].size();i++){
    //					mWifiService = tr(root["jData"][i]["sService"].asString().c_str());
    //					json_str = netserver_get_config((char*)mWifiService.toStdString().c_str());//解析找到的service的配置
    //					//LogV("wifi config %s \n",json_str);
    //					std::string config = string(json_str);
    //					Json::Value ipconfig;
    //					//找到是wifi的Service
    //					if(config.find("wlan0")!= config.npos){//找到wlan0配置
    //						if (reader.parse(json_str, ipconfig)) {
    //							std:string ip = ipconfig[0]["ipv4"]["sV4Address"].asString();
    //							if(ip.length()>4){
    //								mWifiService = QString(ipconfig[0]["sService"].asString().c_str());
    //								mCurrentWifiService = mWifiService;
    //								//LogV("wifi config %s \n",ipconfig[0]["sService"].asString().c_str());
    //								//根据wifi service找到wifi name
    //								json_str = netserver_get_service("wifi");//找到有IP地址的wifi名字
    //								Json::Value name;
    //								if (reader.parse(json_str, name)) {
    //									for(int i =0;i<name.size();i++){
    //										if(name[i]["sService"].asString().compare(mWifiService.toStdString())==0){
    //											essid = tr(name[i]["sName"].asString().c_str());
    //											//LogV("wifi name %s \n",essid.toStdString().c_str());
    //											return essid;
    //										}

    //									}
    //								}
    //								break;
    //							}
    //						}
    //					}else{
    //						continue;
    //					}
    //				}

    //			}
    //		}
    //	}
    return "";
}

void NetworkControlThread::setLinkWlan(const QString &service, const QString &password)
{
    this->mWaitLinkCnt = 0;
    int favorite = 1; // invalid parameter
    int autoconnect = 1;
    char *json_str = NULL;
    json_str = dbserver_network_service_connect_set(service.toLatin1().data(),password.toLatin1().data(), &favorite, &autoconnect);
    // printf("%s %s[%d] dbserver_network_service_connect_set %s \n", __FILE__, __FUNCTION__, __LINE__, json_str);
    printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
    free(json_str);
}

void NetworkControlThread::setLinkLan(const int &type, const QString &ip, const QString &make, const QString &gateway, const QString &dns)
{
    switch(type)
    {
    case 0:
    {
    	printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
        free(dbserver_network_ipv4_set("eth0", "dhcp", NULL, NULL, NULL));
    }break;
    case 1:
    {
        char szIpAddress[18] = { 0 };
        char szNetMask[18] = { 0 };
        char szGateWay[18] = { 0 };
        char szDns[6][18] = { 0 };

        strncpy((char*) szIpAddress, ip.toUtf8().data(), 18);
        strncpy((char*) szNetMask, make.toUtf8().data(), 18);
        strncpy((char*) szGateWay, gateway.toUtf8().data(), 18);
        strncpy((char*) szDns[0], dns.toUtf8().data(), 18);

        if (!isVaildIp((uchar *)szIpAddress) || !isVaildIp((uchar *)szNetMask) || !isVaildIp((uchar *)szGateWay))
        {
            qDebug("setNetAddress params error \n");return;
        }
        char *json_str = dbserver_network_ipv4_set("eth0", "manual", szIpAddress, szNetMask, szGateWay);
        if (json_str)
        {
        	printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
            free(json_str);
        }
        if (szDns != NULL)
        {
            for (int i = 0; i < 6; i++)
            {
                if (szDns[i] != NULL && isVaildIp((uchar *)szDns[i]))
                {
                    std::string cmd = "echo nameserver "+ std::string(((char*)szDns[i])) + " > /etc/resolv.conf";
                    Utils_ExecCmd(cmd.c_str());
                    json_str = dbserver_network_dns_set("eth0", szDns[i], NULL);
                    printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
                    free(json_str);
                    break;
                }
            }
        }
    }break;
    }
}

void NetworkControlThread::resume()
{
    this->sync.lock();

    this->is_pause = false;
    this->sync.unlock();
    this->pauseCond.wakeAll();
}

void NetworkControlThread::pause()
{
    this->sync.lock();
    this->is_pause = true;
    this->sync.unlock();
}

typedef struct
{
    QString Service;
    QString SSID;
    float Strength;//int
}WifiInfo_t;

static inline bool SubListSort(const WifiInfo_t &inf1, const WifiInfo_t &inf2)
{
    return inf1.Strength > inf2.Strength;
}

void NetworkControlThread::parseJson(const QByteArray he_now_json)
{
    QJsonParseError err_rpt;
    QJsonDocument  root_Doc = QJsonDocument::fromJson(he_now_json, &err_rpt);//字符串格式化为JSON
    if(err_rpt.error != QJsonParseError::NoError)
    {
        qDebug() << "JSON格式错误";
        return;
    }else//JSON格式正确
    {
        switch(this->mSearchMode) //1:扫描wifi, 2:查找ssid的service
        {
        case 1:
        {
            QNetworkConfigurationManager ncmger;
            QMap<QString, WifiInfo_t>map;//用来过滤同名
            const QJsonArray& root_Array = root_Doc.array();
            int cnt = root_Array.count();
            for(int i = 0; i<cnt; i++)
            {
                const QJsonObject &obj = root_Array.at(i).toObject();
                QString ssid = obj.value("sName").toString();
                if(!ssid.isEmpty())map[ssid]= WifiInfo_t{obj.value("sService").toString(), ssid, obj.value("Strength").toDouble()};
            }
            auto wifiList = map.values();//排序好了
            qSort(wifiList.begin(), wifiList.end(), SubListSort);
			//同一SSID,不能有多个，如有则重来
			int nCnt = 0;			
			for(int i = 0; i<wifiList.count(); i++)
            {
				const auto &t = wifiList.at(i);
                if(t.SSID == this->mSSID)
                {
					nCnt++;
				}
			}
			
			if (nCnt>1)
			{
				printf(">>>%s,%s,%s, nCnt=%d \n",__FILE__,__func__,__LINE__,nCnt);
				return;
			}
            //重新打包成JSON
            QList<QVariant>tList;
            for(int i = 0; i<wifiList.count(); i++)
            {
                const auto &t = wifiList.at(i);
                QJsonObject json;
                json.insert("sService", t.Service);
                json.insert("Strength", t.Strength);
                if(t.SSID == this->mSSID)
                {
                    //if((this->mWaitLinkCnt <= 2) && (t.SSID == this->mSSID) && !ncmger.isOnline())
					if( (t.SSID == this->mSSID) && !ncmger.isOnline())
                    {
						if (this->mWaitLinkCnt <= 2) 
						{
							++this->mWaitLinkCnt;
							json.insert("sName", t.SSID + QObject::tr("\nConnecting"));//\n正在连接
						} else if (this->mWaitLinkCnt >=3) 
						{
							//重连
							
							const QJsonArray& root_Array = root_Doc.array();
							int cnt = root_Array.count();
							for(int i = 0; i<cnt; i++)
							{
								const QJsonObject &obj = root_Array.at(i).toObject();
								QString ssid = obj.value("sName").toString();					
								if(ssid == this->mSSID)
								{
									this->is_pause = true;
									this->setLinkWlan(obj.value("sService").toString(), this->mSSID_Password);
									this->is_pause = false;
									//return;
								}
							}
				
						}
						
						printf(">>>%s,%s,%d, sName =%s conwifi=%s, conwifipws=%s，mWaitLinkCnt＝%d\n",
						    __FILE__,__func__,__LINE__,t.SSID.toStdString().c_str(),
							ReadConfig::GetInstance()->getWIFI_Name().toStdString().c_str(),
							ReadConfig::GetInstance()->getWIFI_Password().toStdString().c_str(),this->mWaitLinkCnt);
							
                    }
                    else if((t.SSID == this->mSSID) && ncmger.isOnline())
                    {
                        this->mWaitLinkCnt = 255;
                        json.insert("sName", t.SSID + QObject::tr("\nConnected"));//"\n已连接"
						printf(">>>%s,%s,%d, sName =%s conwifi=%s, conwifipws=%s\n",
						    __FILE__,__func__,__LINE__,t.SSID.toStdString().c_str(),
							ReadConfig::GetInstance()->getWIFI_Name().toStdString().c_str(),
							ReadConfig::GetInstance()->getWIFI_Password().toStdString().c_str());
						
                    }
                    tList.insert(0, json);
                }
                else
                {
                    json.insert("sName", t.SSID);
                    tList.append(json);
                }
            }
            printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
            emit sigWifiList(tList);
        }break;
        case 2:
        {
            printf("%s %s[%d] mSearchCount=%d\n",__FILE__,__FUNCTION__,__LINE__,mSearchCount);	
						printf(">>>%s,%s,%d,  conwifi=%s, conwifipws=%s\n",
						    __FILE__,__func__,__LINE__,
							ReadConfig::GetInstance()->getWIFI_Name().toStdString().c_str(),
							ReadConfig::GetInstance()->getWIFI_Password().toStdString().c_str());			
            if(++mSearchCount>3)
            {
                this->mSearchCount = 0;
                //this->is_pause = true; //如果连不上,则要继续搜索
            }else
            {
                const QJsonArray& root_Array = root_Doc.array();
                int cnt = root_Array.count();
            printf("%s %s[%d] cnt=%d\n",__FILE__,__FUNCTION__,__LINE__,cnt);				
                for(int i = 0; i<cnt; i++)
                {
            printf("%s %s[%d] i=%d\n",__FILE__,__FUNCTION__,__LINE__,i);					
                    const QJsonObject &obj = root_Array.at(i).toObject();
                    QString ssid = obj.value("sName").toString();
            printf("%s %s[%d] ssid=%s,mSSID=%s\n",__FILE__,__FUNCTION__,__LINE__,
			    ssid.toStdString().c_str(),mSSID.toStdString().c_str() );						
                    if(ssid == this->mSSID)
                    {
            printf("%s %s[%d]n",__FILE__,__FUNCTION__,__LINE__ );							
                        this->is_pause = true;
                        this->mSearchCount = 0;
                        this->setLinkWlan(obj.value("sService").toString(), this->mSSID_Password);
                        return;
                    }
                }
            }
        }break;
        }
    }
}

void NetworkControlThread::run()
{
    while (!isInterruptionRequested())
    {
        this->sync.lock();
        if (this->is_pause)this->pauseCond.wait(&this->sync);
		system("/sbin/ifconfig p2p0 down");
		
        netserver_scan_wifi();
        char *json_str = netserver_get_service((char *) "wifi");
		if(json_str !=NULL)
        {
            this->parseJson(QByteArray().append(json_str));
            printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
            free(json_str);
        }
        
        this->pauseCond.wait(&this->sync, 5000);
        this->sync.unlock();
    }
}
