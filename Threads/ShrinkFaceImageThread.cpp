#include "ShrinkFaceImageThread.h"
#ifdef Q_OS_LINUX
#include "PCIcore/Watchdog.h"
#include "SettingFuncFrms/SysSetupFrms/StorageCapacityFrm.h"
#include<unistd.h>
#endif
#include "Application/FaceApp.h"
#include "FaceMainFrm.h"

#include <QSqlDriver>
#include <QProcess>
#include <QDebug>
#include <QDateTime>
#include <QStorageInfo>


static inline int queryRowCount(QSqlQuery &query)
{
    int initialPos = query.at();
    // Very strange but for no records .at() returns -2
    int pos = 0;
    if (query.last()) {
        pos = query.at() + 1;
    }
    else {
        pos = 0;
    }
    // Important to restore initial pos
    query.seek(initialPos);
    return pos;
}


ShrinkFaceImageThread::ShrinkFaceImageThread(QObject *parent)
    : QThread(parent)
{
    this->start();
}

ShrinkFaceImageThread::~ShrinkFaceImageThread()
{
    this->requestInterruption();
    this->pauseCond.wakeOne();

    this->quit();
    this->wait();
}

void ShrinkFaceImageThread::doShrinkFaceImage()
{
	char cmdline[256];
	QString maxUuid;

	int shrinkCount=0;
	shrinkCount = StorageCapacityFrm::GetInstance()->getCountTotal()*0.1;  //总记录数的 10%
	
	//查询出符合删除条件的记录,并取出最大日期 

#ifdef Q_OS_LINUX		
	    QSqlQuery query(QSqlDatabase::database("isc_ir_arcsoft_face"));

		query.prepare(QString("select max(time)as maxuuid from ( select * from identifyrecord order by time limit %1)").arg(shrinkCount));
		query.exec();
		
		while(query.next()) {
			QString result = query.value(0).toString();
			maxUuid = query.value(0).toString();
			//qDebug()<<"maxUuid: "<<maxUuid<<";shrinkCount="<<shrinkCount<<",sql="<<QString("select max(time)as maxuuid from ( select * from identifyrecord order by time limit %1)").arg(shrinkCount);			
		}		
#endif 		

	if (maxUuid.isEmpty() )  return;



	//批量删除记录
	//QString sql = " sqlite3  /mnt/user/facedb/isc_ir_arcsoft_face.db 'delete from identifyrecord where  time<='"+ maxUuid+"'  ;";
	//QString sql = " sqlite3  /mnt/user/facedb/isc_ir_arcsoft_face.db 'delete from identifyrecord where  time<="""""+ maxUuid+""""" '  ;";
	sprintf(cmdline,"sqlite3  /mnt/user/facedb/isc_ir_arcsoft_face.db 'delete from identifyrecord where  time<=\"%s\" '  ;",maxUuid.toStdString().data());	  	  
	qDebug()<<"sql: "<<cmdline;

	system(cmdline);
	
	
	//转换日期模式
	QDateTime time = QDateTime::fromString(maxUuid,"yyyy/MM/dd hh:mm:ss");	
	maxUuid = time.toString("yyyy-MM-dd hh:mm:ss");
	//批量删除图片
#if 0
/mnt/user/face_crop_image/2021-12-08/Full_202112080129845_10.jpg
/mnt/user/face_crop_image/2021-12-08/Full_202112080138124_252.jpg
#endif 	
    //参考语句        find /mnt/user/face_crop_image/ -name *.jpg | grep Full | awk -F'_' ' {if (substr($3,7,10)""substr($4,9,2) ":" substr($4,11,2) ":" substr($4,13,2)>="2021-12-08 01:28:40") {print $0 }} '
	sprintf(cmdline,"find /mnt/user/face_crop_image/ -name *.jpg | grep Full | awk -F'_' ' {if (substr($3,7,10)\"\"substr($4,9,2) \":\" substr($4,11,2) \":\" substr($4,13,2)>=\"%s\") {print $0 }} ' | xargs rm -rf ",maxUuid.toUtf8().data());	  	 

	qDebug()<<"cmdline Full: "<<cmdline;
	
	system(cmdline);
#if 0
/mnt/user/face_crop_image/2021-12-08/202112080137431_182.jpg
/mnt/user/face_crop_image/2021-12-08/202112080129959_11.jpg
/mnt/user/face_crop_image/2021-12-08/202112080137202_182.jpg
#endif 	
	//此句 是 tid_
	//sprintf(cmdline,"find /mnt/user/face_crop_image/ -name *.jpg  | awk -F'_' ' {if ($7\"\"substr($8,1,2) \":\" substr($8,3,2) \":\" substr($8,5,2)>=\"%s\") {print $0 }} ' | xargs rm -rf ",maxUuid.toUtf8().data());	  	 

	// 参考语句: find /mnt/user/face_crop_image/ -name *.jpg  | awk -F'_' ' {if (substr($3,7,10)""substr($3,26,2) ":" substr($3,28,2) ":" substr($3,30,2)>="2021-12-08 01:28:40") {print $0}} ' 

	sprintf(cmdline,"find /mnt/user/face_crop_image/ -name *.jpg  | awk -F'_' ' {if (substr($3,7,10)\"\"substr($3,26,2) \":\" substr($3,28,2) \":\" substr($3,30,2)>=\"%s\") {print $0 }} ' | xargs rm -rf ",maxUuid.toUtf8().data());	  	 

	qDebug()<<"cmdline: "<<cmdline;
	
	system(cmdline);

}

void ShrinkFaceImageThread::run()
{
    while (!isInterruptionRequested())
    {
#ifdef Q_OS_LINUX		
	    QSqlQuery query(QSqlDatabase::database("isc_ir_arcsoft_face"));
		query.prepare("select * from identifyrecord");
		query.exec();

		int totalCnt=0;	
		if (query.driver()->hasFeature(QSqlDriver::QuerySize))
		{
			totalCnt = query.size();
		}
		else
		{
			totalCnt = queryRowCount(query);
		}
		
	//按每个相片 200K 计算 
	    auto storage = QStorageInfo("mnt/user");
	    storage.refresh();
		qint64 bytesTotal = storage.bytesTotal();
		qint64 bytesFree = storage.bytesFree();

		//qDebug()<<"totalCnt="<<totalCnt<<",Total()*0.9" \
		   <<StorageCapacityFrm::GetInstance()->getCountTotal()*0.9 \
		   <<",bytesFree="<<bytesFree<<",bytesTotal=" <<bytesTotal <<",bytesTotal/1024="<<bytesTotal/1024 \
		   <<",bytesFree/1024="<<bytesFree/1024;

		if (totalCnt >= StorageCapacityFrm::GetInstance()->getCountTotal()*0.9 || bytesFree<bytesTotal *0.1 )
		{
			this->doShrinkFaceImage();
		}
#endif

		msleep(15*1000);
    }
 	
}
