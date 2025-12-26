#include "PersonRecordToDB.h"

#include "PCIcore/RkUtils.h"
#include <sys/stat.h>
#include "MessageHandler/Log.h"
#include "HttpServer/local_service.h"
#include "HttpServer/PostPersonRecordThread.h"
#include "Config/ReadConfig.h"

#include <QtCore/QWaitCondition>
#include <QtCore/QMutex>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtCore/QDebug>

#include <QtCore/QDateTime>
#include <QtCore/QUuid>
#include <QtWidgets/QApplication>
#include <QtGui/QScreen>
#include <QtWidgets/QDesktopWidget>
#include <QtCore/QBuffer>
#include <QtCore/QThread>

#define DateTime QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss")
#define Today QDateTime::currentDateTime().toString("yyyy-MM-dd")

class PersonRecordToDBPrivate
{
    Q_DECLARE_PUBLIC(PersonRecordToDB)
public:
    PersonRecordToDBPrivate(PersonRecordToDB *dd);
private:
    void saveFaceImgToDisk(const QString &, const CORE_FACE_S &);
    void saveFaceFullImgToDisk(const QString &, const CORE_FACE_S &);
    bool appRecordData(const QString &name, const QString &sex, const QString &idcard, const QString &iccard, const QString &uuid, const int &persontype, const int &personid, const QString &gids, const QString &pids, const QString &time, const QString &img_path, const float &tempvalue, const int &face_mask);
private:
    bool mRecordsPanoramaImg;
    bool mRecordsFaceImg;
    bool mRecordsStranger;
    mutable QMutex sync;
private:
    int mDeskW;
    int mDeskH;
private:
    PersonRecordToDB *const q_ptr;
};

PersonRecordToDBPrivate::PersonRecordToDBPrivate(PersonRecordToDB *dd)
    : q_ptr(dd)
    , mRecordsPanoramaImg(false)
    , mRecordsFaceImg(true)
    , mRecordsStranger(true)
{
    qRegisterMetaType<IdentifyFaceRecord_t>("IdentifyFaceRecord_t");

    mDeskW = QApplication::desktop()->screenGeometry().width();
    mDeskH = QApplication::desktop()->screenGeometry().height();

    QObject::connect(q_func(), &PersonRecordToDB::sigappRecordData, q_func(), &PersonRecordToDB::slotappRecordData);
}

PersonRecordToDB::PersonRecordToDB(QObject *parent)
    : QObject(parent)
    , d_ptr(new PersonRecordToDBPrivate(this))
{
    QThread *thread = new QThread;
    this->moveToThread(thread);
    thread->start();
}

PersonRecordToDB::~PersonRecordToDB()
{
}

static void cropDir_Init()
{
#ifdef Q_OS_LINUX
    QString  m_crop_path = QString(FACE_CROP_PATH) + "/" + Today;
    if (access(m_crop_path.toLatin1(), F_OK) != 0)
    {
        LogV("Dir : %s not exits!\n", m_crop_path.toLatin1().data());
        mkdir(FACE_CROP_PATH, 0777);
        mkdir(m_crop_path.toLatin1(), 0777);
    }
#endif
}

void PersonRecordToDBPrivate::saveFaceImgToDisk(const QString &imgPath, const CORE_FACE_S &FaceTask)
{
    Q_UNUSED(imgPath);
    Q_UNUSED(FaceTask);
#ifdef Q_OS_LINUX
    QString path = imgPath;
    unsigned char* pTmpBuf = (unsigned char*) YNH_LJX::RkUtils::Utils_Malloc(this->mDeskW * this->mDeskH * 3 / 2);
    unsigned char* pTmpBuf_1 = (unsigned char*) YNH_LJX::RkUtils::Utils_Malloc(this->mDeskW * this->mDeskH * 3 / 2);
    int x, y, width, height;
    int result = -1;
    x = FaceTask.stFaceRect.nX;
    y = FaceTask.stFaceRect.nY;
    width = FaceTask.stFaceRect.nWidth;
    height = FaceTask.stFaceRect.nHeight;

    x = x + (x % 2 ? -1 : 0);
    y = y + (y % 2 ? -1 : 0);
    if ((width % 4) != 0)
        width = 4 * ((width / 4) - 1);
    if ((height % 4) != 0)
        height = 4 * ((height / 4) - 1);
    if (x < 0)
    {
        x = 0;
    }
    if (y < 0)
    {
        y = 0;
    }
    if ((x + width) > 736)
        width = (736 - x);
    if ((y + height) > this->mDeskH)
        height = (this->mDeskH - y);

    result = YNH_LJX::RkUtils::NV21CutImage((unsigned char *) FaceTask.FaceOffscreen.ppu8Plane[0], this->mDeskW, this->mDeskH, pTmpBuf,
            width * height * 3 / 2, x, y, x + width, y + height);
    if(result < 0)
    {
        YNH_LJX::RkUtils::Utils_Free(pTmpBuf);
        YNH_LJX::RkUtils::Utils_Free(pTmpBuf_1);
    	return;
    }

    YNH_LJX::RkUtils::Utils_YVU420SPConvertToYUV420P((unsigned long)pTmpBuf, (unsigned long) pTmpBuf_1, width,height);
    YNH_LJX::RkUtils::YUVtoJPEG(path.toLatin1(), (unsigned char*) pTmpBuf_1, width, height);
    YNH_LJX::RkUtils::Utils_ExecCmd("sync");
    YNH_LJX::RkUtils::Utils_Free(pTmpBuf);
    YNH_LJX::RkUtils::Utils_Free(pTmpBuf_1);
    YNH_LJX::RkUtils::Utils_ExecCmd("sync");
#endif
}

void PersonRecordToDBPrivate::saveFaceFullImgToDisk(const QString &imgPath, const CORE_FACE_S &FaceTask)
{
    Q_UNUSED(imgPath);
    Q_UNUSED(FaceTask);
#ifdef Q_OS_LINUX
    QString path = imgPath;
    unsigned char* pTmpBuf = (unsigned char*) YNH_LJX::RkUtils::Utils_Malloc(this->mDeskW * this->mDeskH * 3 / 2);
    YNH_LJX::RkUtils::Utils_YVU420SPConvertToYUV420P((unsigned long)FaceTask.FaceOffscreen.ppu8Plane[0], (unsigned long) pTmpBuf, this->mDeskW,this->mDeskH);
    YNH_LJX::RkUtils::YUVtoJPEG(path.toLatin1(), (unsigned char*) pTmpBuf, this->mDeskW, this->mDeskH);
    YNH_LJX::RkUtils::Utils_ExecCmd("sync");
    YNH_LJX::RkUtils::Utils_Free(pTmpBuf);
    YNH_LJX::RkUtils::Utils_ExecCmd("sync");
#endif
}

void PersonRecordToDB::setRecords_PanoramaImg(const bool &b)
{
    Q_D(PersonRecordToDB);
    d->mRecordsPanoramaImg = b;
}

void PersonRecordToDB::setRecords_FaceImg(const bool &b)
{
    Q_D(PersonRecordToDB);
    d->mRecordsFaceImg = b;
}

void PersonRecordToDB::setRecords_Stranger(const bool &b)
{
    Q_D(PersonRecordToDB);
    d->mRecordsStranger = b;
}

static inline int getidentifyrecordMaxRid()
{
    QSqlQuery query(QSqlDatabase::database("isc_ir_arcsoft_face"));
    query.exec("select rid from identifyrecord ORDER BY rid DESC LIMIT 1");
    while(query.next())
    {
        return query.value("rid").toInt() + 1;
    }
    return 0;
}

// In PersonRecordToDB.cpp - Modified slotappRecordData function

void PersonRecordToDB::slotappRecordData(const IdentifyFaceRecord_t t)
{
    Q_D(PersonRecordToDB);
    cropDir_Init();
    QString FaceImgPath;
    if(d->mRecordsPanoramaImg)d->saveFaceFullImgToDisk(t.FaceFullImgPath, t.face);
    if(d->mRecordsFaceImg)
    {
        if(t.face.catch_face_quality > 0 && !access(t.face.FaceImgPath,F_OK))
        {
            FaceImgPath = QString::fromUtf8(t.face.FaceImgPath, strlen(t.face.FaceImgPath));
        }else
        {
            FaceImgPath = t.FaceImgPath;
            d->saveFaceImgToDisk(t.FaceImgPath, t.face);
        }
        LogD("%s %s[%d] FaceImgPath %s \n",__FILE__,__FUNCTION__,__LINE__,FaceImgPath.toStdString().c_str());
    }

    if((t.FaceType == STRANGER) || (t.FaceType == 0))
    {
        LocalService::GetInstance()->appRecordData(t);
        // REMOVED: PostPersonRecordThread::GetInstance()->appRecordData(t); // Don't post stranger records
        
        if(d->mRecordsStranger)
        {// (Save records to local database only)
            if(d->mRecordsPanoramaImg)
            {
                qDebug()<<"陌生人保存全景图"<<d->appRecordData(t.face_name, t.face_sex, t.face_idcardnum, t.face_iccardnum, t.face_uuid, t.face_persontype, t.face_personid, t.face_gids, t.face_aids, DateTime, t.FaceFullImgPath, t.temp_value, t.face.attr_info.face_mask);
            }
            if(d->mRecordsFaceImg)
            {
                qDebug()<<"陌生人保存人脸图"<<d->appRecordData(t.face_name, t.face_sex, t.face_idcardnum, t.face_iccardnum, t.face_uuid, t.face_persontype, t.face_personid, t.face_gids, t.face_aids, DateTime, FaceImgPath, t.temp_value, t.face.attr_info.face_mask);
            }
        } 
    }else
    {//非陌生人 (Face recognized records)
        // Post both to local service AND server for face recognized records
        //局域网服务接收识别记录
        LocalService::GetInstance()->appRecordData(t);
        PostPersonRecordThread::GetInstance()->appRecordData(t); // POST TO SERVER for recognized faces only
        
        QString  mustMode = ReadConfig::GetInstance()->getDoor_MustOpenMode();
        QString mOptionmode = ReadConfig::GetInstance()->getDoor_OptionalOpenMode();        
        
        if (( mustMode=="1" || mOptionmode.contains("1") ) && t.process_state.contains("1")) //ICCard
        {
            qDebug()<<"非陌生人不刷脸保存人脸图"<<d->appRecordData(t.face_name, t.face_sex, t.face_idcardnum, t.face_iccardnum, t.face_uuid, t.face_persontype, t.face_personid, t.face_gids, t.face_aids, DateTime, "/mnt/user/face_crop_image", t.temp_value, t.face.attr_info.face_mask);
        }
        else if(d->mRecordsPanoramaImg)
        {
            qDebug()<<"非陌生人保存全景图"<<d->appRecordData(t.face_name, t.face_sex, t.face_idcardnum, t.face_iccardnum, t.face_uuid, t.face_persontype, t.face_personid, t.face_gids, t.face_aids, DateTime, t.FaceFullImgPath, t.temp_value, t.face.attr_info.face_mask);
        } else if(d->mRecordsFaceImg)
        {
            qDebug()<<"非陌生人保存人脸图"<<d->appRecordData(t.face_name, t.face_sex, t.face_idcardnum, t.face_iccardnum, t.face_uuid, t.face_persontype, t.face_personid, t.face_gids, t.face_aids, DateTime, FaceImgPath, t.temp_value, t.face.attr_info.face_mask);
        } else if(!d->mRecordsPanoramaImg && !d->mRecordsFaceImg)
        {
            qDebug()<<"非陌生人不保存人脸图"<<d->appRecordData(t.face_name, t.face_sex, t.face_idcardnum, t.face_iccardnum, t.face_uuid, t.face_persontype, t.face_personid, t.face_gids, t.face_aids, DateTime, "/mnt/user/face_crop_image", t.temp_value, t.face.attr_info.face_mask);
        }
    }
}


bool PersonRecordToDB::appDoorRecordData(const int &persontype,const QString &img_path)
{
	if(access(img_path.toStdString().c_str(),F_OK))
	{
		LogE("%s %s[%d]  %s not exist , so ignore \n",__FILE__,__FUNCTION__,__LINE__,img_path.toStdString().c_str());
		return false;
	}
    QSqlQuery query(QSqlDatabase::database("isc_ir_arcsoft_face"));
    QString strSql;
    strSql.append("INSERT INTO identifyrecord");
    strSql.append("(rid,");
    strSql.append("Identifyed,");
    strSql.append("uuid,");
    strSql.append("persontype,");
    strSql.append("name,");
    strSql.append("img_path,");
    strSql.append("time,");
    strSql.append("face_mask,");
    strSql.append("personid) ");

    strSql.append("VALUES(?,?,?,?,?,?,?,?,?)");
    query.prepare(strSql);

    query.bindValue(0, getidentifyrecordMaxRid());
    query.bindValue(1,  0 );
    query.bindValue(2,  QUuid::createUuid().toString());
    query.bindValue(3, 0);//persontype
    query.bindValue(4,  QObject::tr("stranger") );//陌生人
    query.bindValue(5, img_path);
    query.bindValue(6, QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss"));
    query.bindValue(7,  0 );
    query.bindValue(8,  0 );

    return query.exec();

}

bool PersonRecordToDBPrivate::appRecordData(const QString &name, const QString &sex, const QString &idcard, const QString &iccard, const QString &uuid, const int &persontype, const int &personid, const QString &gids, const QString &pids, const QString &time, const QString &img_path, const float &tempvalue, const int &face_mask)
{
    QString  mustMode = ReadConfig::GetInstance()->getDoor_MustOpenMode();
	if(access(img_path.toStdString().c_str(),F_OK) && mustMode.contains("2") ) //Face wipe 
	{
		LogE("%s %s[%d] %s %s not exist , so ignore \n",__FILE__,__FUNCTION__,__LINE__,name.toStdString().c_str(),img_path.toStdString().c_str());
		return false;        
	}
    LogD("%s %s[%d]  \n",__FILE__,__FUNCTION__,__LINE__);
    QSqlQuery query(QSqlDatabase::database("isc_ir_arcsoft_face"));
    QString strSql;
    strSql.append("INSERT INTO identifyrecord");
    strSql.append("(rid,");
    strSql.append("Identifyed,");
    strSql.append("face_mask,");
    strSql.append("personid,");
    strSql.append("uuid,");
    strSql.append("persontype,");
    strSql.append("name,");
    strSql.append("img_path,");
    strSql.append("time,");
    strSql.append("gids,");
    strSql.append("pids,");
    strSql.append("idcardnum,");
    strSql.append("iccardnum,");
    strSql.append("tempvalue,");
    strSql.append("sex)");
    strSql.append("VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
    query.prepare(strSql);
    query.bindValue(0, getidentifyrecordMaxRid());
    query.bindValue(1, name.isEmpty() ? 0 : 1);
    query.bindValue(2, face_mask);
    query.bindValue(3, personid);
    query.bindValue(4, uuid.isEmpty() ? QUuid::createUuid().toString() : uuid);
    query.bindValue(5, persontype);
    query.bindValue(6, name.isEmpty() ? QObject::tr("stranger") : name);//陌生人
    query.bindValue(7, img_path);
    query.bindValue(8, time);
    query.bindValue(9, gids);
    query.bindValue(10, pids);
    query.bindValue(11, idcard);
    query.bindValue(12, iccard);
    query.bindValue(13, tempvalue);
    query.bindValue(14, sex);

    LogD("%s %s[%d]  face_mask=%d,img_path=%s,name=%s\n",__FILE__,__FUNCTION__,__LINE__,
         face_mask,img_path.toStdString().c_str(),name.toStdString().c_str());
    return query.exec();

}


bool PersonRecordToDB::UpdatePersonRecordUploadFlag(int rid, const bool isUpload)
{
    Q_D(PersonRecordToDB);
    QMutexLocker lock(&d->sync);
	//更新数据表
    QSqlQuery query(QSqlDatabase::database("isc_ir_arcsoft_face"));
    QString strSql("UPDATE identifyrecord SET uploadflag='%1' where rid='%2' ");
    strSql = strSql.arg(isUpload ? 1: 0);
    strSql = strSql.arg(rid);

    printf(">>>>%s,%s,%d,strSql=%s\n",__FILE__,__func__,__LINE__,strSql.toStdString().c_str());
    if(query.exec(strSql) == false)
    {
    	QSqlError error = query.lastError();
    	LogE("%s %s[%d] strSql %s \n",__FILE__,__FUNCTION__,__LINE__,strSql.toStdString().c_str());
    	LogE("%s %s[%d] error %s \n",__FILE__,__FUNCTION__,__LINE__,error.text().toStdString().c_str());
        // sync.unlock();
    	return false;
    }
    //sync.unlock();
    return true;
}

QList<IdentifyFaceRecord_t> PersonRecordToDB::GetPersonRecordDataByName(int nCurrPage,int nPerPage,QString name,bool filterUpdateFlag)
{
    Q_D(PersonRecordToDB);
    QMutexLocker lock(&d->sync);

    QList<IdentifyFaceRecord_t> result;
    QSqlQuery query(QSqlDatabase::database("isc_ir_arcsoft_face"));
    QString strSql;
    if(filterUpdateFlag == true)
    {
    	strSql =  QString("select * from identifyrecord where name='%1' and uploadflag='0' order by time desc Limit %3 Offset %4 ");
    }else
    {
    	strSql =  QString("select * from identifyrecord where name='%1' order by time desc Limit %3 Offset %4 ");
    }
    strSql = strSql.arg(name);
    strSql = strSql.arg(nPerPage);
    strSql = strSql.arg((nCurrPage - 1) * nPerPage);

    if(query.exec(strSql) == false)
    {
    	QSqlError error = query.lastError();
    	LogE("%s %s[%d] strSql %s \n",__FILE__,__FUNCTION__,__LINE__,strSql.toStdString().c_str());
    	LogE("%s %s[%d] error %s \n",__FILE__,__FUNCTION__,__LINE__,error.text().toStdString().c_str());
    	return result;
    }
    while(query.next())
    {
    	IdentifyFaceRecord_t t{};
    	t.rid = query.value("rid").toInt();
    	t.Identifyed = query.value("Identifyed").toInt();
    	t.face_name = query.value("name").toInt();
    	t.face_sex = query.value("sex").toString();
    	t.face_uuid = query.value("uuid").toString();
    	t.createtime = QDateTime::fromString(query.value("time").toString(), "yyyy/MM/dd hh:mm:ss");
    	t.face_idcardnum = query.value("idcardnum").toString();
    	t.face_iccardnum = query.value("iccardnum").toString();
    	t.FaceImgPath = query.value("img_path").toString();
    	t.temp_value = query.value("tempvalue").toString().toFloat();
    	t.face_persontype = query.value("persontype").toString().toInt();
    	result.append(t);
    }
    return result;
}

int PersonRecordToDB::GetPersonRecordTotalNumByName(QString name,bool filterUpdateFlag)
{
    Q_D(PersonRecordToDB);
    QMutexLocker lock(&d->sync);

    int nTotalNum = 0;
    QList<IdentifyFaceRecord_t> result;
    QSqlQuery query(QSqlDatabase::database("isc_ir_arcsoft_face"));
    QString strSql;
    if(filterUpdateFlag == true)
    {
    	strSql =  QString("select count(rid) from identifyrecord where name='%1' and uploadflag='0' ");
    }else
    {
    	strSql =  QString("select count(rid) from identifyrecord where name='%1'");
    }
    strSql = strSql.arg(name);

    if(query.exec(strSql) == false)
    {
    	QSqlError error = query.lastError();
    	LogE("%s %s[%d] strSql %s \n",__FILE__,__FUNCTION__,__LINE__,strSql.toStdString().c_str());
    	LogE("%s %s[%d] error %s \n",__FILE__,__FUNCTION__,__LINE__,error.text().toStdString().c_str());
    	return nTotalNum;
    }
    if (query.next())
    {
    	nTotalNum= query.value(0).toInt();
    }
    return nTotalNum;
}

QList<IdentifyFaceRecord_t>  PersonRecordToDB::GetPersonRecordDataByPersonUUID(int nCurrPage,int nPerPage,QString uuid,bool filterUpdateFlag)
{
    Q_D(PersonRecordToDB);
    QMutexLocker lock(&d->sync);

    QList<IdentifyFaceRecord_t> result;
    QSqlQuery query(QSqlDatabase::database("isc_ir_arcsoft_face"));
    QString strSql;
    if(filterUpdateFlag == true)
    {
    	strSql = QString("select * from identifyrecord where uuid='%1' and uploadflag='0' order by time desc Limit %3 Offset %4 ");
    }else
    {
    	strSql = QString("select * from identifyrecord where uuid='%1' order by time desc Limit %3 Offset %4 ");
    }
    strSql = strSql.arg(uuid);
    strSql = strSql.arg(nPerPage);
    strSql = strSql.arg((nCurrPage - 1) * nPerPage);

    if(query.exec(strSql) == false)
    {
    	QSqlError error = query.lastError();
    	LogE("%s %s[%d] strSql %s \n",__FILE__,__FUNCTION__,__LINE__,strSql.toStdString().c_str());
    	LogE("%s %s[%d] error %s \n",__FILE__,__FUNCTION__,__LINE__,error.text().toStdString().c_str());
    	return result;
    }
    while(query.next())
    {
    	IdentifyFaceRecord_t t{};
    	t.rid = query.value("rid").toInt();
    	t.Identifyed = query.value("Identifyed").toInt();
    	t.face_name = query.value("name").toString();
    	t.face_sex = query.value("sex").toString();
    	t.face_uuid = query.value("uuid").toString();
    	t.createtime = QDateTime::fromString(query.value("time").toString(), "yyyy/MM/dd hh:mm:ss");
    	t.face_idcardnum = query.value("idcardnum").toString();
    	t.face_iccardnum = query.value("iccardnum").toString();
    	t.FaceImgPath = query.value("img_path").toString();
    	t.temp_value = query.value("tempvalue").toString().toFloat();
    	t.face_persontype = query.value("persontype").toString().toInt();
    	result.append(t);
    }
    return result;
}

//根据用户uuid，分页查询识别记录总数
int PersonRecordToDB::GetPersonRecordTotalNumByPersonUUID(QString uuid, bool filterUpdateFlag)
{
    Q_D(PersonRecordToDB);
    QMutexLocker lock(&d->sync);

    int nTotalNum = 0;
    QList<IdentifyFaceRecord_t> result;
    QSqlQuery query(QSqlDatabase::database("isc_ir_arcsoft_face"));
    QString strSql;
    if(filterUpdateFlag == true)
    {
    	strSql = QString("select count(rid) from identifyrecord where uuid='%1' and uploadflag='0'");
    }else
    {
    	strSql = QString("select count(rid) from identifyrecord where uuid='%1'");
    }
    strSql = strSql.arg(uuid);

    if(query.exec(strSql) == false)
    {
    	QSqlError error = query.lastError();
    	LogE("%s %s[%d] strSql %s \n",__FILE__,__FUNCTION__,__LINE__,strSql.toStdString().c_str());
    	LogE("%s %s[%d] error %s \n",__FILE__,__FUNCTION__,__LINE__,error.text().toStdString().c_str());
    	return nTotalNum;
    }
    if (query.next())
    {
    	nTotalNum= query.value(0).toInt();
    }
    return nTotalNum;
}


QList<IdentifyFaceRecord_t>  PersonRecordToDB::GetPersonRecordDataByDateTime(int nCurrPage,int nPerPage,QDateTime startDateTime,QDateTime endDateTime,bool filterUpdateFlag)
{
    Q_D(PersonRecordToDB);
    QMutexLocker lock(&d->sync);

    QList<IdentifyFaceRecord_t> result;
    QSqlQuery query(QSqlDatabase::database("isc_ir_arcsoft_face"));
    QString strSql;
    if(filterUpdateFlag == true)
    {
    	strSql = QString("select * from identifyrecord where time between '%1' and '%2' and uploadflag='0' order by time desc Limit %4 Offset %5");
    }else
    {
    	strSql = QString("select * from identifyrecord where time between '%1' and '%2' order by time desc Limit %4 Offset %5");
    }
    strSql = strSql.arg(startDateTime.toString("yyyy/MM/dd hh:mm:ss"));
    strSql = strSql.arg(endDateTime.toString("yyyy/MM/dd hh:mm:ss"));
    strSql = strSql.arg(nPerPage);
    strSql = strSql.arg((nCurrPage - 1) * nPerPage);
    //LogD("%s %s[%d] strSql %s \n",__FILE__,__FUNCTION__,__LINE__,strSql.toStdString().c_str());
    if(query.exec(strSql) == false)
    {
    	QSqlError error = query.lastError();
    	LogE("%s %s[%d] strSql %s \n",__FILE__,__FUNCTION__,__LINE__,strSql.toStdString().c_str());
    	LogE("%s %s[%d] error %s \n",__FILE__,__FUNCTION__,__LINE__,error.text().toStdString().c_str());
    	return result;
    }
    while(query.next())
    {
    	IdentifyFaceRecord_t t{};
    	t.rid = query.value("rid").toInt();
    	t.Identifyed = query.value("Identifyed").toInt();
    	t.face_name = query.value("name").toString();
    	t.face_sex = query.value("sex").toString();
    	t.createtime = QDateTime::fromString(query.value("time").toString(), "yyyy/MM/dd hh:mm:ss");
    	t.face_uuid = query.value("uuid").toString();
    	t.face_idcardnum = query.value("idcardnum").toString();
    	t.face_iccardnum = query.value("iccardnum").toString();
    	t.FaceImgPath = query.value("img_path").toString();
    	t.temp_value = query.value("tempvalue").toString().toFloat();
    	t.face_persontype = query.value("persontype").toString().toInt();
    	result.append(t);
    }
    return result;
}

int PersonRecordToDB::GetPersonRecordTotalNumByDateTime(QDateTime startDateTime,QDateTime endDateTime, bool filterUpdateFlag)
{
    Q_D(PersonRecordToDB);
    QMutexLocker lock(&d->sync);

    int nTotalNum = 0;
    QList<IdentifyFaceRecord_t> result;
    QSqlQuery query(QSqlDatabase::database("isc_ir_arcsoft_face"));
    QString strSql;
    if(filterUpdateFlag == true)
    {
    	strSql = QString("select count(rid) from identifyrecord where time between '%1' and '%2' and uploadflag='0' ");
    }else
    {
    	strSql = QString("select count(rid) from identifyrecord where time between '%1' and '%2' ");
    }
    strSql = strSql.arg(startDateTime.toString("yyyy/MM/dd hh:mm:ss"));
    strSql = strSql.arg(endDateTime.toString("yyyy/MM/dd hh:mm:ss"));
    //LogD("%s %s[%d] strSql %s \n",__FILE__,__FUNCTION__,__LINE__,strSql.toStdString().c_str());
    if(query.exec(strSql) == false)
    {
    	QSqlError error = query.lastError();
    	LogE("%s %s[%d] strSql %s \n",__FILE__,__FUNCTION__,__LINE__,strSql.toStdString().c_str());
    	LogE("%s %s[%d] error %s \n",__FILE__,__FUNCTION__,__LINE__,error.text().toStdString().c_str());
    	return nTotalNum;
    }
    if (query.next())
    {
    	nTotalNum= query.value(0).toInt();
    }
    return nTotalNum;
}

bool PersonRecordToDB::DeletePersonRecordByName(QString name)
{
    Q_D(PersonRecordToDB);
    QMutexLocker lock(&d->sync);
    QSqlQuery query(QSqlDatabase::database("isc_ir_arcsoft_face"));
    QString strSql("delete from identifyrecord where name=\'%1\'");
    strSql = strSql.arg(name);
    if(query.exec(strSql) == false)
    {
    	QSqlError error = query.lastError();
    	LogE("%s %s[%d] strSql %s \n",__FILE__,__FUNCTION__,__LINE__,strSql.toStdString().c_str());
    	LogE("%s %s[%d] error %s \n",__FILE__,__FUNCTION__,__LINE__,error.text().toStdString().c_str());
    	return false;
    }
    return true;
}

bool PersonRecordToDB::DeletePersonRecordByRID(long rid)
{
    Q_D(PersonRecordToDB);
    QMutexLocker lock(&d->sync);
    QSqlQuery query(QSqlDatabase::database("isc_ir_arcsoft_face"));
    QString strSql("delete from identifyrecord where rid=\'%1\'");
    strSql = strSql.arg(rid);
    if(query.exec(strSql) == false)
    {
    	QSqlError error = query.lastError();
    	LogE("%s %s[%d] strSql %s \n",__FILE__,__FUNCTION__,__LINE__,strSql.toStdString().c_str());
    	LogE("%s %s[%d] error %s \n",__FILE__,__FUNCTION__,__LINE__,error.text().toStdString().c_str());
    	return false;
    }
    return true;
}

bool PersonRecordToDB::DeletePersonRecordByTime(QDateTime startDateTime,QDateTime endDateTime)
{
    Q_D(PersonRecordToDB);
    QMutexLocker lock(&d->sync);
    QSqlQuery query(QSqlDatabase::database("isc_ir_arcsoft_face"));
    QString strSql("delete from identifyrecord where time between %1 and %2 ");
    strSql = strSql.arg(startDateTime.toString("yyyy/MM/dd hh:mm:ss"));
    strSql = strSql.arg(endDateTime.toString("yyyy/MM/dd hh:mm:ss"));
    if(query.exec(strSql) == false)
    {
    	QSqlError error = query.lastError();
    	LogE("%s %s[%d] strSql %s \n",__FILE__,__FUNCTION__,__LINE__,strSql.toStdString().c_str());
    	LogE("%s %s[%d] error %s \n",__FILE__,__FUNCTION__,__LINE__,error.text().toStdString().c_str());
    	return false;
    }
    return true;
}
