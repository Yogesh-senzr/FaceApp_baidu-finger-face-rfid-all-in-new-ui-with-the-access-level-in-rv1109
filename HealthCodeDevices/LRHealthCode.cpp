#include "LRHealthCode.h"
#include "PCIcore/UARTUart.h"
#include "PCIcore/Audio.h"
#include "json-cpp/json.h"
#include "Helper/myhelper.h" 
#include "SharedInclude/GlobalDef.h"
#include "Application/FaceApp.h"
#include "BaseFace/BaseFaceManager.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>
#include <fcntl.h>
#include <dirent.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/prctl.h>
#include <time.h>

#include "MessageHandler/Log.h"

#include <QtCore/QThread>
#include <QtCore/QWaitCondition>
#include <QtCore/QMutex>
#include <QtCore/QDebug>
//串口2 
#define UART_PORT  2
using namespace YNH_LJX;
class LRHealthCodePrivate
{
	Q_DECLARE_PUBLIC(LRHealthCode)
public:
	LRHealthCodePrivate(LRHealthCode *dd);
private:
	void readSerial() const;
	void DealLRHealthCode(const int &type, const QString) const;

private:
	void Uart_OpenUartDev(const int BaudRate);

private:
	mutable QMutex sync;
	QWaitCondition pauseCond;
private:
	LRHealthCode * const q_ptr;
};

LRHealthCodePrivate::LRHealthCodePrivate(LRHealthCode *dd) :
		q_ptr(dd)
{
	qRegisterMetaType<HEALTINFO_t>("HEALTINFO_t");
	Uart_OpenUartDev(115200); //115200
}

LRHealthCode::LRHealthCode(QObject *parent) :
		QThread(parent), d_ptr(new LRHealthCodePrivate(this))
{
	LogV("%s %s[%d]  \n", __FILE__, __FUNCTION__, __LINE__);
	this->start();
}

LRHealthCode::~LRHealthCode()
{
	Q_D(LRHealthCode);
	this->requestInterruption();
	d->pauseCond.wakeOne();
	this->quit();
	this->wait();
}

void LRHealthCodePrivate::Uart_OpenUartDev(const int BaudRate)
{
	UART_ATTR_S stUartAttr;
	stUartAttr.RDBlock = 0;//1;
	stUartAttr.mBlockData = 10; // 30;
	stUartAttr.nBaudRate = 115200;
	stUartAttr.nAttr = 8;

	if (YNH_LJX::UARTUart::Uart_OpenUartDev(UART_PORT, stUartAttr) != ISC_OK)
	{
		LogV("%s %s[%d] Uart_OpenUartDev nUartPort %d \n", __FILE__, __FUNCTION__, __LINE__, UART_PORT);
	} else
		LogV("%s %s[%d] Uart_OpenUartDev nUartPort %d ,BaudRate =%dok \n", __FILE__, __FUNCTION__, __LINE__, UART_PORT, BaudRate);

}

static inline std::string PosLRHealthCode(int type, std::string url, std::string requestData)
{
	std::string ResString;
	std::string cmd;
	if (type == 1)
	{
		cmd = "/usr/bin/curl -H \"Content-Type:application/json;charset=UTF-8\" ";
		cmd += "  ";
		cmd += url.c_str();
		cmd += "  --cacert /isc/cacert.pem ";
		cmd += "--connect-timeout 3 -s -X POST ";
		//cmd += "--data-raw '";
		// cmd += "-d 'json=";
		cmd += "-d '";
		cmd += requestData.c_str();
		cmd += "'";
	} else if (type == 2)
	{
		cmd = "/usr/bin/curl  ";
		cmd += "  --cacert /isc/cacert.pem \"";
		cmd += url.c_str();
		cmd += " \"";
		printf(">>>%s,%s,%d,cmd=%s\n", __FILE__, __func__, __LINE__, cmd.c_str());
	}

	std::string ret = "";
#define ISC_NULL                 0L

	//qDebug()<<"cmd : "<<cmd.c_str();
	FILE *pFile = popen(cmd.c_str(), "r");
	if (pFile != ISC_NULL)
	{
		char buf[4096] = { 0 };
		int readSize = 0;
		do
		{
			readSize = fread(buf, 1, sizeof(buf), pFile);
			if (readSize > 0)
			{
				ret += std::string(buf, 0, readSize);
			}
		} while (readSize > 0);
		pclose(pFile);
		//qDebug()<<"buf : "<<buf;
		if (ret.length() > 10)
		{
			// res = CURLE_OK;
			ResString = ret;
		}
	}

	return ResString;
}

void LRHealthCodePrivate::readSerial() const
{
	float fValue = 0;
	//LogV("%s %s[%d]  \n", __FILE__, __FUNCTION__, __LINE__);
	unsigned char recvBuf[1024] = { 0 };
	int recvLen = 0;
	Json::Reader reader;
	Json::Value root;
	Json::Value jsonData;
	Json::Value jsonInit;
	Json::Value rootInit;
	Json::Value rootData;


	int nRet = YNH_LJX::UARTUart::Uart_ReadUart(UART_PORT, recvBuf, sizeof(recvBuf));
	//printf(">>>%s,%s,%d, mData=%s,nRet=%d \n", __FILE__, __func__, __LINE__, recvBuf, nRet);
	YNH_LJX::UARTUart::Uart_ReadFlush(UART_PORT);
	if (nRet > 0)
	{
		std::string str = (char *) recvBuf;
		auto occurrences = [&str](const std::string &dest) {
		    size_t pos, pre = 0, count = 0;
		    while ( (pos = str.find(dest, pre)) != std::string::npos ) {
		        ++count;
		        pre = pos + 1;
		    }
		    return count;
		};
		if(occurrences("\r\n") > 1)
		{
				//快速刷二维码时，串口会读出错误数据，这里判断下，如果是错误数据就丢弃
			LogE("%s,%s,%d, mData=%s,nRet=%d \n", __FILE__, __func__, __LINE__, recvBuf, nRet);
			return;
		}
		YNH_LJX::Audio::Audio_PlayCustomerPcm("zh", "Buzzer.wav", true);//放在这里嘀
		QString strSerial = QString::fromStdString(str);
		DealLRHealthCode(1, strSerial);
	}

}

void LRHealthCodePrivate::DealLRHealthCode(const int &type, const QString strRetJson) const
{
	//qDebug()<<strRetJson;
	Json::Reader reader;
	Json::Value root;
	Json::Value rootInit;
	Json::Value jsonInit;
	Json::Value jsonData;
     
	 	printf("%s %s[%d] size : %d \n", __FILE__, __FUNCTION__, __LINE__, strRetJson.size());
		 HEALTINFO_t info;
		 //info.name = QString::fromStdString(root["data"]["name"].asString());  
		 info.name = strRetJson;
		 printf("%s %s[%d] strRetJson : %s \n", __FILE__, __FUNCTION__, __LINE__, strRetJson.toStdString().c_str());
			emit q_func()->sigLRHealthCodeMsg(info);

  return;

	if(qXLApp->GetAlgoFaceManager()->hasPerson() == false)
	{
		YNH_LJX::Audio::Audio_PlayCustomerPcm("zh", "PlsLookScreen.wav",true);
		return;
	}

	printf("%s %s[%d] size : %d \n", __FILE__, __FUNCTION__, __LINE__, strRetJson.size());
	if (strRetJson.size())
	{
		jsonInit["etp_id"] = "6dd47c02aefa432aab7569239c8cc2e3";
		jsonInit["secret_key"] = "7ea02712370b40d2bb2afd69c631d5af";
		jsonInit["sn"] = myHelper::getCpuSerial().toStdString();
		std::string strRetJson1 = PosLRHealthCode(1, "http://159.75.19.97:8010/api/hc/init", jsonInit.toStyledString());

		if (!reader.parse(strRetJson1, rootInit))
		{
			LogE("%s %s[%d] strRetJson1%s\n", __FILE__, __FUNCTION__, __LINE__, strRetJson1.c_str());
			YNH_LJX::Audio::Audio_PlayCustomerPcm("zh", "net_error.wav", true);
			return;
		}
	    switch(rootInit["code"].asInt())
		{
    		case 1012:
				YNH_LJX::Audio::Audio_PlayCustomerPcm("zh", "sn_not_exist_tips.wav", true);
				return ;
		}

		if (!rootInit["data"].empty() &&  rootInit["data"]["token"].empty())
		{
			YNH_LJX::Audio::Audio_PlayCustomerPcm("zh", "sn_not_exist_tips.wav", true);
			return ;			
		}

		if (!rootInit["data"].empty() &&  !rootInit["data"]["token"].empty())
		  jsonData["token"] = rootInit["data"]["token"].asString();
		jsonData["qrcode"] = strRetJson.toStdString();

		LogV("%s %s[%d] jsonData%s\n", __FILE__, __FUNCTION__, __LINE__, jsonData.toStyledString().c_str());
		strRetJson1 = PosLRHealthCode(1, "http://159.75.19.97:8010/api/hc/parseHealthyCode", jsonData.toStyledString());
		printf("%s %s[%d],strRetJson1=%s\n", __FILE__, __FUNCTION__, __LINE__, strRetJson1.c_str());

		if (reader.parse(strRetJson1, root))
		{
		    switch(root["code"].asInt())
			{
				case 2002:   
					YNH_LJX::Audio::Audio_PlayCustomerPcm("zh", "InvalidHealtCode.wav", true);
					return ;
					break;
				case 2003:
					YNH_LJX::Audio::Audio_PlayCustomerPcm("zh", "HealtcodeIsOverdue.wav", true);
					return ;
					break;
				case 0:
					//YNH_LJX::Audio::Audio_PlayCustomerPcm("zh", "greenplsPass.wav", true);
					
					break;
				default:
				   	YNH_LJX::Audio::Audio_PlayCustomerPcm("zh", "net_error.wav", true);
					return ;
					break;
			}


		//	printf("%s %s[%d] ,type=%s, hsDateFlag=%s, hsResult=%s\n", __FILE__, __FUNCTION__, __LINE__, root["data"]["type"].asString().c_str(),\
					root["data"]["hsDateFlag"].asString().c_str(), root["data"]["hsResult"].asString().c_str());
		
			HEALTINFO_t info;
			if (!root["data"].empty() && !root["data"]["type"].empty())
			{
				if (root["data"]["type"].asInt() ==2) //粤康码
				{
					if (!root["data"]["name"].empty())
					  info.name = QString::fromStdString(root["data"]["name"].asString());  //姓名
					if (!root["data"]["idNumber"].empty())
					  info.idnumber = QString::fromStdString(root["data"]["idNumber"].asString());  //身份证
					if (!root["data"]["hsDateTime"].empty())
					  info.hsdatetime = QString::fromStdString(root["data"]["hsDateTime"].asString());  //检测时间
					if (!root["data"]["hsResult"].empty())
					  info.hsresult = QString::fromStdString(root["data"]["hsResult"].asString());  //检测结果
					if (!root["data"]["hsDateFlag"].empty())
					  info.hsdateflag = QString::fromStdString(root["data"]["hsDateFlag"].asString());  //24,48,72 等
					if (!root["data"]["vaccine"].empty())
					  info.vaccine = QString::fromStdString(root["data"]["vaccine"].asString());  //疫苗接种剂次
					if (!root["data"]["vaccineDate"].empty())
					  info.vaccinedate = QString::fromStdString(root["data"]["vaccineDate"].asString());  //疫苗接种日期
					if (!root["data"]["type"].empty())
					  info.type = QString::fromStdString(root["data"]["type"].asString());  //类型,粤康码/深I您
					if (!root["data"]["color"].empty())
					  info.color = QString::fromStdString(root["data"]["color"].asString());  //green,yellow,red
					if (!root["data"]["showTime"].empty())
					  info.showTime = QString::fromStdString(root["data"]["showTime"].asString());  //显示时间
				} else //深I您
				{
					if (!root["data"]["name"].empty())
					  info.name = QString::fromStdString(root["data"]["name"].asString());  //姓名
					if (!root["data"]["idNumber"].empty())
					  info.idnumber = QString::fromStdString(root["data"]["idNumber"].asString());  //身份证
					if (!root["data"]["hsDateTime"].empty())
					  info.hsdatetime = QString::fromStdString(root["data"]["hsDateTime"].asString());  //检测时间
					if (!root["data"]["hsResult"].empty())
					  info.hsresult = QString::fromStdString(root["data"]["hsResult"].asString());  //检测结果
					if (!root["data"]["hsDateFlag"].empty())
					  info.hsdateflag = QString::fromStdString(root["data"]["hsDateFlag"].asString());  //24,48,72 等
					if (!root["data"]["vaccine"].empty())
					  info.vaccine = QString::fromStdString(root["data"]["vaccine"].asString());  //疫苗接种剂次
					if (!root["data"]["vaccineDate"].empty())
					  info.vaccinedate = QString::fromStdString(root["data"]["vaccineDate"].asString());  //疫苗接种日期
					if (!root["data"]["type"].empty())
					  info.type = QString::fromStdString(root["data"]["type"].asString());  //类型,粤康码/深I您
					if (!root["data"]["color"].empty())
					  info.color = QString::fromStdString(root["data"]["color"].asString());  //green,yellow,red
					if (!root["data"]["showTime"].empty())
					  info.showTime = QString::fromStdString(root["data"]["showTime"].asString());  //显示时间
				}

				info.qrcode = info.color.toInt();
			printf("%s %s[%d] ,showTime=%s\n", __FILE__, __FUNCTION__, __LINE__,info.showTime.toStdString().c_str());
				emit q_func()->sigLRHealthCodeMsg(info);
				//发送到 识别流程
			}
#if 0
			int qrCodeType = -1;

			std::string name;
			std::string idcardNum;
			name = root["name"].asString();
			idcardNum = root["cid"].asString();
#endif 

		}
	} else
	{            //未读到正常数据，即表示网络异常，或服务器异常
		printf("%s %s[%d] \n", __FILE__, __FUNCTION__, __LINE__);
		YNH_LJX::Audio::Audio_PlayCustomerPcm("zh", "net_error.wav", true);
	}
}

void LRHealthCode::run()
{
	LogV("%s %s[%d]  \n", __FILE__, __FUNCTION__, __LINE__);	
	Q_D(LRHealthCode);
	sleep(5);
	//while (!isInterruptionRequested())
	while (true)
	{
	//LogV("%s %s[%d]  \n", __FILE__, __FUNCTION__, __LINE__);		
		d->sync.lock();
	//LogV("%s %s[%d]  \n", __FILE__, __FUNCTION__, __LINE__);		
		d->readSerial();
		d->pauseCond.wait(&d->sync, 1000);
		d->sync.unlock();
	}
}
