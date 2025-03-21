#include "RegisteredFacesDB.h"

#ifdef Q_OS_LINUX
//#include "ArcsoftFaceManager.h"
#include <unistd.h>
#endif

#include "BaiduFaceManager.h"
#include "SharedInclude/GlobalDef.h"
#include "Application/FaceApp.h"
#include "MessageHandler/Log.h"
#include "FaceMainFrm.h"
#include <fcntl.h>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDatabase>
#include <QtCore/QWaitCondition>
#include <QtCore/QMutex>
#include <QtCore/QList>
#include <QtCore/QUuid>
#include <QtCore/QDateTime>
#include <QtSql/QSqlError>
#include <QtCore/QDebug>
#include <QtCore/QTime>

 double gAlogStateFaceSimilar = 0; //全局人脸对比阈值

class RegisteredFacesDBPrivate
{
    Q_DECLARE_PUBLIC(RegisteredFacesDB)
public:
    RegisteredFacesDBPrivate(RegisteredFacesDB *dd);
private:
    mutable QMutex sync;
    QWaitCondition pauseCond;
    volatile bool is_pause;
private:
    float mThanthreshold;
    QList<PERSONS_t>mPersons;
private:
    RegisteredFacesDB *const q_ptr;
};

RegisteredFacesDBPrivate::RegisteredFacesDBPrivate(RegisteredFacesDB *dd)
    : q_ptr(dd)
    , is_pause(false)
    , mThanthreshold(0.8)
{
}


RegisteredFacesDB::RegisteredFacesDB(QObject *parent)
    : QThread(parent)
    , d_ptr(new RegisteredFacesDBPrivate(this))
{
    this->start();
}

RegisteredFacesDB::~RegisteredFacesDB()
{
    Q_D(RegisteredFacesDB);
    this->requestInterruption();
    d->is_pause = false;
    d->pauseCond.wakeOne();
    this->quit();
    this->wait();
}

void RegisteredFacesDB::setThanthreshold(const float &value)
{
    Q_D(RegisteredFacesDB);
    d->mThanthreshold = value;
}

bool RegisteredFacesDB::selectICcardPerson(const QString &iccard, PERSONS_s &s)
{
    s.reader = true;
    s.iccard = QString();
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    query.exec(QString("select *from person ORDER BY iccardnum='%1' DESC LIMIT 1").arg(iccard));
    while(query.next())
    {
        s.feature = query.value("feature").toByteArray();
        s.name = query.value("name").toString();
        s.sex = query.value("sex").toString();
        s.idcard = query.value("idcardnum").toString();
        s.iccard = query.value("iccardnum").toString();
        s.uuid= query.value("uuid").toString();
        s.persontype = query.value("persontype").toInt();
        s.personid = query.value("personid").toInt();
        s.gids = query.value("gids").toString();
        s.pids = query.value("aids").toString();
        s.createtime = query.value("createtime").toString();
        return true;
    }
    return false;
}

bool RegisteredFacesDB::selectIDcardPerson(const QString &idCard, PERSONS_s &s)
{
    s.reader = true;
    s.idcard = QString();
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    query.exec(QString("select *from person ORDER BY idcardnum='%1' DESC LIMIT 1").arg(idCard));
    while(query.next())
    {
        s.feature = query.value("feature").toByteArray();
        s.name = query.value("name").toString();
        s.sex = query.value("sex").toString();
        s.idcard = query.value("idcardnum").toString();
        s.iccard = query.value("iccardnum").toString();
        s.uuid= query.value("uuid").toString();
        s.persontype = query.value("persontype").toInt();
        s.personid = query.value("personid").toInt();
        s.gids = query.value("gids").toString();
        s.pids = query.value("aids").toString();
        s.createtime = query.value("createtime").toString();
        return true;
    }
    return false;
}

bool RegisteredFacesDB::CheckPassageOfTime(QString person_uuid)
{
	/**先查询用户表里面，用户自己的通行权限*/
    QString strStartTime;
    QString strCurTime;
    QString strEndTime;

#if 0
case 1: person 通行  PassageTime 通行  预期:通行 
       1.1 person 为空
	   1.2 person 通行      00:10,10:30,1,1,1,1,1,1,1
	   1.3 PassageTime  为空
	   1.4 person 通行 	   
case 2  person 禁行  PassageTime 通行  预期:禁行
       2.1 person 禁行
	   2.2  PassageTime  为空
	   2.3  PassageTime 通行  00:00|23:59,1,1,1,1,1,1,1
case 3  person 通行  PassageTime 禁行  预期:禁行
       3.1 person 为空
	   3.2 person 通行 
	   3.3  PassageTime 禁行  00:00|23:59,0,0,0,0,0,0,0
case 4  person 禁行  PassageTime 禁行  预期:禁行
       4.1 person 禁行
	   4.2  PassageTime 禁行 

#endif 

   //  bool bAccessPerson = false;
	if(person_uuid.size() > 3)
	{
		Q_D(RegisteredFacesDB);

        //step 1:
		for(int i = 0; i < d->mPersons.size(); i++)
		{
			auto &t = d->mPersons.at(i);
			if(t.uuid == person_uuid)
			{
				QStringList sections = t.timeOfAccess.split(",");
				LogD("%s %s[%d] uuid %s timeOfAccess %s  sections=%d\n",__FILE__,__FUNCTION__,__LINE__,t.uuid.toStdString().c_str(),t.timeOfAccess.toStdString().c_str(),sections.size());
                if (sections.size() <= 1) 
                {
                   //bAccessPerson =true;
                }
				else if (sections.size() == 9)  //周循环
				{
					strStartTime = sections.at(0);
					strEndTime = sections.at(1);

					QTime curTime = QTime::currentTime();
					QTime stateTime = QTime::fromString(strStartTime, "hh:mm");
					QTime endTime = QTime::fromString(strEndTime, "hh:mm");

					if (curTime <= endTime && curTime >= stateTime)
					{
						switch (QDate::currentDate().dayOfWeek())
						{
						case 1:
							return (sections[2] == "1");
						case 2:
							return (sections[3] == "1");
						case 3:
							return (sections[4] == "1");
						case 4:
							return (sections[5] == "1");
						case 5:
							return (sections[6] == "1");
						case 6:
							return (sections[7] == "1");
						case 7:
							return (sections[8] == "1");
						}
					} 
                    
					//LogD("%s %s[%d] uuid %s time_of_access %s \n", __FILE__, __FUNCTION__, __LINE__, t.uuid.toStdString().c_str(),t.timeOfAccess.toStdString().c_str());
                    return false;
				} else  //时间范围
				{
					//t.timeOfAccess 2000/01/01 00:00;2000/01/01 00:00
					QStringList sections = t.timeOfAccess.split(";");
					if (sections.size() == 2)
					{
						QDateTime curTimer = QDateTime::currentDateTime();
						QDateTime startTimer = QDateTime::fromString(sections[0], "yyyy/MM/dd hh:mm");
						QDateTime endTimer = QDateTime::fromString(sections[1], "yyyy/MM/dd hh:mm");
						if (curTimer <= endTimer && curTimer >= startTimer)
						{
							return true;
						} else
						{
					        strCurTime = curTimer.toString(QString::fromLatin1("yyyy/MM/dd hh:mm"));
					        strStartTime = startTimer.toString(QString::fromLatin1("yyyy/MM/dd hh:mm"));
					        strEndTime = endTimer.toString(QString::fromLatin1("yyyy/MM/dd hh:mm"));

					        LogD("%s %s[%d] start %s cur %s end %s \n",__FILE__,__FUNCTION__,__LINE__,strStartTime.toStdString().c_str(),
					        		strCurTime.toStdString().c_str(),strEndTime.toStdString().c_str());
							return false;
						}
					}
				}
			}
		}
	}
  
    //step 2:
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    query.exec("select *from PassageTime");

    strCurTime = "";
    strStartTime = "";
    strEndTime = "";

    while(query.next())
    {
    	QTime curTime = QTime::currentTime();
    	QTime stateTime = QTime::fromString(query.value("stateTimer").toString(), "hh:mm");
    	QTime endTime =  QTime::fromString(query.value("endTimer").toString(), "hh:mm");
        strCurTime = curTime.toString(QString::fromLatin1("hh:mm"));
        strStartTime = stateTime.toString(QString::fromLatin1("hh:mm"));
        strEndTime = endTime.toString(QString::fromLatin1("hh:mm"));

        if(curTime<=endTime && curTime>=stateTime)
        {
            switch(QDate::currentDate().dayOfWeek())
            {
            case 1: return query.value("Monday").toBool();
            case 2: return query.value("Tuesday").toBool();
            case 3: return query.value("Wednesday").toBool();
            case 4: return query.value("Thursday").toBool();
            case 5: return query.value("Friday").toBool();
            case 6: return query.value("Saturday").toBool();
            case 7: return query.value("Sunday").toBool();
            }
        }
    }

    //两步校验 结束后
    //if (!bAccessPerson) 
    //  return false; //有一项禁止
        

    //默认没有通行权限，则任意时间段均可通行
    if(strStartTime.size() <= 0 || strEndTime.size() <= 0)
    {
    	return true;
    }


    LogD("%s %s[%d] start %s cur %s end %s ,day %d \n",__FILE__,__FUNCTION__,__LINE__,strStartTime.toStdString().c_str(),
    		strCurTime.toStdString().c_str(),strEndTime.toStdString().c_str(),QDate::currentDate().dayOfWeek());
    return false;
}

static inline int getPersonMaxid()
{
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    query.exec("select personid from person ORDER BY personid DESC LIMIT 1");
    while(query.next())
    {
        return query.value("personid").toInt() + 1;
    }
    return 0;
}

bool RegisteredFacesDB::RegPersonToDBAndRAM(const QString &uuid,const QString &Name, const QString &idCard, const QString &icCard, const QString &Sex, const QString &department, const QString &timeOfAccess, const QByteArray &FaceFeature)
{

    Q_D(RegisteredFacesDB);
    QMutexLocker lock(&d->sync);

    PERSONS_t t{};
    t.feature = FaceFeature;
    t.name = Name;
    t.sex = Sex.isEmpty() ? "Unknown" : Sex;;
    t.idcard = idCard;
    t.iccard = icCard.isEmpty() ? "000000" : icCard;
    if(uuid.size() > 3)
    {
    	t.uuid = uuid;
    }else
    {
    	t.uuid = QUuid::createUuid().toString();
    }

    t.persontype = 0;
    t.personid = getPersonMaxid();
    t.gids = QString("0");
    t.pids = QString("2");
    t.timeOfAccess = timeOfAccess;
    t.createtime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    t.department = department;

    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    QString strSql;
    strSql.append("INSERT OR REPLACE INTO person");
    strSql.append("(personid,");
    strSql.append("uuid,");
    strSql.append("persontype,");
    strSql.append("name,");
    strSql.append("sex,");
    strSql.append("image,");
    strSql.append("createtime,");
    strSql.append("idcardnum,");
    strSql.append("iccardnum,");
    strSql.append("feature,");
    strSql.append("featuresize,");
    strSql.append("gids,");
    strSql.append("aids,");
    strSql.append("time_of_access,");
    strSql.append("department)");
    strSql.append("VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
    query.prepare(strSql);
    query.bindValue(0, t.personid);
    query.bindValue(1, t.uuid);
    query.bindValue(2, t.persontype);
    query.bindValue(3, t.name);
    query.bindValue(4, t.sex);
    query.bindValue(5, QString("/mnt/user/"));
    query.bindValue(6, t.createtime);
    query.bindValue(7, t.idcard);
    query.bindValue(8, t.iccard);
    query.bindValue(9, t.feature);
    query.bindValue(10, FaceFeature.size());
    query.bindValue(11, t.gids);
    query.bindValue(12, t.pids);
    query.bindValue(13, t.timeOfAccess);
    query.bindValue(14, t.department);

//yaosen 增加下面的打印后，注册人员后，马上进行人脸识别必然出现误识别的问题，不知为何消失了？？？
    LogD("%s %s[%d] ,feature.size()=\n",__FILE__,__FUNCTION__,__LINE__,t.feature.size());
    for(int i =0; i < t.feature.size(); i++)
    {
    	printf("%x ",t.feature.data()[i]);
    }
    LogD("\n");
    //yaosen
	
    d->mPersons.append(t);

     bool ret = query.exec();
     if(ret == false)
     {
     	QSqlError error = query.lastError();
     	LogE("%s %s[%d] strSql %s \n",__FILE__,__FUNCTION__,__LINE__,strSql.toStdString().c_str());
     	LogE("%s %s[%d] error %s \n",__FILE__,__FUNCTION__,__LINE__,error.text().toStdString().c_str());
     	return ISC_UPDATE_PERSON_DB_ERROR;
     }else
     {
    	 qXLApp->GetFaceMainFrm()->updateHome_PersonNum();
     }




#ifdef Q_OS_LINUX
    system("sync");
#endif
    return ret;

}

int RegisteredFacesDB::UpdatePersonToDBAndRAM(const QString &uuid,const QString &name, const QString &idCard, const QString &icCard, const QString &sex, const QString &department, const QString &timeOfAccess, const QString &jpeg)
{
    Q_D(RegisteredFacesDB);
    QMutexLocker lock(&d->sync);
    int result = 0;
    PERSONS_t stPerson = {0};
    QByteArray faceFeature;

    LogV("%s %s[%d] uuid %s \n", __FILE__, __FUNCTION__, __LINE__, uuid.toStdString().c_str());
    LogV("%s %s[%d] name %s \n", __FILE__, __FUNCTION__, __LINE__, name.toStdString().c_str());
    LogV("%s %s[%d] idCard %s \n", __FILE__, __FUNCTION__, __LINE__, idCard.toStdString().c_str());
    LogV("%s %s[%d] icCard %s \n", __FILE__, __FUNCTION__, __LINE__, icCard.toStdString().c_str());
    LogV("%s %s[%d] sex %s \n", __FILE__, __FUNCTION__, __LINE__, sex.toStdString().c_str());
    LogV("%s %s[%d] department %s \n", __FILE__, __FUNCTION__, __LINE__, department.toStdString().c_str());
    LogV("%s %s[%d] timeOfAccess %s \n", __FILE__, __FUNCTION__, __LINE__, timeOfAccess.toStdString().c_str());
    LogV("%s %s[%d] jpeg %s \n", __FILE__, __FUNCTION__, __LINE__, jpeg.toStdString().c_str());
    if(uuid.size() <= 0)
    {
    	return -1;
    }

	int index = -1;
	for(int i = 0; i < d->mPersons.size(); i++)
	{
		auto &t = d->mPersons.at(i);
		if(t.uuid == uuid)
		{
			index = i;
			stPerson = t;
			break;
		}
	}

	if(index < 0)
	{
		return ISC_UPDATE_PERSON_NOT_EXIST;
	}

	stPerson.uuid = uuid;
	if(name.size() > 0)
	{
		stPerson.name = name;
	}
	if(idCard.size() > 0)
	{
		stPerson.idcard = idCard;
	}
	if(icCard.size() > 0)
	{
		stPerson.iccard = icCard;
	}
	if(sex.size() > 0)
	{
		stPerson.sex = sex;
	}
	if(department.size() > 0)
	{
		stPerson.department = department;
	}
	if(timeOfAccess.size() > 0)
	{
		stPerson.timeOfAccess = timeOfAccess;
	}

	if(jpeg.size() > 0 && !access(jpeg.toStdString().c_str(),F_OK))
	{
	    int faceNum = 0;
	    double threshold = 0;
	    result = ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->RegistPerson(jpeg, faceNum, threshold, faceFeature);
	    if(result == 0)
	    {
	    	stPerson.feature = faceFeature;
	    }else
	    {
	    	return result-10; //加10 处理 
	    }
	}

	//更新数据表
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    QString strSql;
    strSql.append("UPDATE person SET ");
    strSql.append("name=?,");
    strSql.append("sex=?,");
    strSql.append("idcardnum=?,");
    strSql.append("iccardnum=?,");
    if(jpeg.size() > 0 && !access(jpeg.toStdString().c_str(),F_OK))
    {
        strSql.append("feature=?,");
        strSql.append("featuresize=?,");
    }
    strSql.append("time_of_access=?,");
    strSql.append("department=?");
    strSql.append(" where uuid=?");
    int sqlIndex = 0;
    query.prepare(strSql);
    query.bindValue(sqlIndex++, stPerson.name);
    query.bindValue(sqlIndex++, stPerson.sex);
    query.bindValue(sqlIndex++, stPerson.idcard);
    query.bindValue(sqlIndex++, stPerson.iccard);
    if(jpeg.size() > 0 && !access(jpeg.toStdString().c_str(),F_OK))
    {
        query.bindValue(sqlIndex++, stPerson.feature);
        query.bindValue(sqlIndex++, stPerson.feature.size());
    }
    query.bindValue(sqlIndex++, stPerson.timeOfAccess);
    query.bindValue(sqlIndex++, stPerson.department);
    query.bindValue(sqlIndex++, stPerson.uuid);
    if(query.exec() == false)
    {
    	QSqlError error = query.lastError();
    	LogE("%s %s[%d] strSql %s \n",__FILE__,__FUNCTION__,__LINE__,strSql.toStdString().c_str());
    	LogE("%s %s[%d] error %s \n",__FILE__,__FUNCTION__,__LINE__,error.text().toStdString().c_str());
    	return ISC_UPDATE_PERSON_DB_ERROR;
    }
	d->mPersons.replace(index, stPerson);

#ifdef Q_OS_LINUX
	system("sync");
#endif
	//return result;
    if (result ==0)
      return 1;
    else 
      return result;
}

bool RegisteredFacesDB::RegPersonToDBAndRAM(const QString &name, const QString &sex, const QString &nation, const QString &idcardnum, const QString &Marital, const QString &education, const QString &location, const QString &phone, const QString &politics_status, const QString &date_birth, const QString &number, const QString &branch, const QString &hiredate, const QString &extension_phone, const QString &mailbox, const QString &status, const QString &persontype, const QString &iccardnum, const QByteArray &FaceFeature)
{
    return this->RegPersonToDBAndRAM("",name, idcardnum, iccardnum, sex, "","",FaceFeature);
}

bool RegisteredFacesDB::ComparisonPersonFaceFeature_baidu(QString &name, QString &sex, QString &idcard, QString &iccard, QString &uuid, int &persontype, 
   int &personid, QString &gids, QString &pids, unsigned char *FaceFeature, int FaceFeatureSize)
{
    Q_D(RegisteredFacesDB);
    QMutexLocker lock(&d->sync);

    gAlogStateFaceSimilar = 0;
    for(int i = 0; i<d->mPersons.count(); i++)
    {
        auto &t = d->mPersons.at(i);        
#if 0        
        LogV("%s %s[%d],i=%d feature.size=%d,FaceFeatureSize=%d\n", __FILE__, __FUNCTION__, __LINE__,i,t.feature.size(),FaceFeatureSize);   
    LogD("%s %s[%d] ,feature.size()=%d\n",__FILE__,__FUNCTION__,__LINE__,t.feature.size());
    for(int i =0; i < t.feature.size(); i++)
    {
    	printf("%02x ",t.feature.data()[i]);
    }
    LogD("\n");

    LogD("%s %s[%d] ,FaceFeatureSize=%d\n",__FILE__,__FUNCTION__,__LINE__,FaceFeatureSize);
    for(int i =0; i < FaceFeatureSize; i++)
    {
    	printf("%02x ",FaceFeature[i]);
    }
    LogD("\n");
#endif 
        double similar = ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->getFaceFeatureCompare_baidu((uchar *)t.feature.data(), t.feature.size(), FaceFeature, FaceFeatureSize);

        if(similar > gAlogStateFaceSimilar)
        {
        	gAlogStateFaceSimilar = similar;
        }

        if(similar>=d->mThanthreshold)// 0.8
        {
            name = t.name;
            sex = t.sex;
            idcard = t.idcard;
            iccard = t.iccard;
            uuid = t.uuid;
            persontype = t.persontype;
            personid = t.personid;
            gids = t.gids;
            pids = t.pids;
            return true;
        }
    }    
    return false;
}    

bool RegisteredFacesDB::ComparisonPersonFaceFeature(QString &name, QString &sex, QString &idcard, QString &iccard, QString &uuid, int &persontype, int &personid, QString &gids, QString &pids, const QByteArray &FaceFeature)
{
    Q_D(RegisteredFacesDB);
    QMutexLocker lock(&d->sync);

    gAlogStateFaceSimilar = 0;
    for(int i = 0; i<d->mPersons.count(); i++)
    {
        auto &t = d->mPersons.at(i);
     

        double similar = ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->getFaceFeatureCompare((uchar *)t.feature.data(), t.feature.size(), FaceFeature);
        if(similar > gAlogStateFaceSimilar)
        {
        	gAlogStateFaceSimilar = similar;
        }
        if(similar>=d->mThanthreshold)
        {
            name = t.name;
            sex = t.sex;
            idcard = t.idcard;
            iccard = t.iccard;
            uuid = t.uuid;
            persontype = t.persontype;
            personid = t.personid;
            gids = t.gids;
            pids = t.pids;
            return true;
        }
    }
    return false;
}

bool RegisteredFacesDB::DelPersonDBInfo(const QString &name, const QString &createtime)
{/*删除数据库信息*/
    Q_D(RegisteredFacesDB);
    QMutexLocker lock(&d->sync);
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    query.prepare(QString("DELETE FROM person WHERE name='%1' AND createtime='%2'")
                  .arg(name).arg(createtime));
    bool ret = query.exec();

    for(int i = 0; i<d->mPersons.count(); i++)
    {
        auto &t = d->mPersons.at(i);

        if(t.name == name && t.createtime == createtime)
        {
            d->mPersons.removeAt(i);

            qXLApp->GetFaceMainFrm()->updateHome_PersonNum();
            return ret;
        }
    }

    return ret;
}

bool RegisteredFacesDB::QueryNameRepetition(const QString &name) const
{
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    query.exec(QString("select name from person where name='%1'").arg(name));
    while(query.next())
    {
        return true;
    }
    return false;
}

void RegisteredFacesDB::run()
{
    Q_D(RegisteredFacesDB);
    while (!isInterruptionRequested())
    {
        d->sync.lock();
        if (d->is_pause)d->pauseCond.wait(&d->sync);
        QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
        query.exec("select *from person");
        while(query.next())
        {
            PERSONS_t t{};
            t.feature = query.value("feature").toByteArray();
            t.name = query.value("name").toString();
            t.sex = query.value("sex").toString();
            if(t.sex == "")
            {
            	t.sex = "unknow";
            }
            t.idcard = query.value("idcardnum").toString();
            t.iccard = query.value("iccardnum").toString();
            t.uuid = query.value("uuid").toString();
            t.persontype = query.value("persontype").toInt();
            t.personid = query.value("personid").toInt();
            t.gids = query.value("gids").toString();
            t.pids = query.value("aids").toString();
            t.createtime = query.value("createtime").toString();
            t.timeOfAccess = query.value("time_of_access").toString();
            t.department = query.value("department").toString();
            d->mPersons.append(t);
        }
        d->is_pause = true;
        d->sync.unlock();
    }
}

QList<PERSONS_t> RegisteredFacesDB::GetAllPersonFromRAM()
{
	QList<PERSONS_t> result;
	Q_D(RegisteredFacesDB);
	d->sync.lock();
	result = d->mPersons;

	d->sync.unlock();
	return result;
}

QList<PERSONS_t> RegisteredFacesDB::GetPersonDataByNameFromRAM(int nCurrPage,int nPerPage,QString name)
{
	QList<PERSONS_t> result;
	Q_D(RegisteredFacesDB);
	d->sync.lock();
	for (int i = nCurrPage; i < nCurrPage + nPerPage; i++)
	{
		if(i < 0 || d->mPersons.size() <= i)
		{
			break;
		}
		auto &t = d->mPersons.at(i);
		if (t.name == name)
		{
			result.append(t);
		}
	}
	d->sync.unlock();
	return result;
}

int RegisteredFacesDB::GetPersonTotalNumByNameFromRAM(QString name)
{
	Q_D(RegisteredFacesDB);
	int count = 0;
	d->sync.lock();
    for(int i = 0; i<d->mPersons.count(); i++)
    {
        auto &t = d->mPersons.at(i);
        if(t.name == name)
        {
        	count++;
        }
    }

	d->sync.unlock();
	return count;
}

QList<PERSONS_t> RegisteredFacesDB::GetPersonDataByPersonUUIDFromRAM(QString uuid)
{
	Q_D(RegisteredFacesDB);
	QList<PERSONS_t> result;
	d->sync.lock();
	for (int i = 0; i < d->mPersons.size(); i++)
    {
		if(i < 0 || d->mPersons.size() <= i)
		{
			break;
		}
        auto &t = d->mPersons.at(i);
        if(t.uuid == uuid)
        {
        	result.append(t);
        }
    }

	d->sync.unlock();
	return result;
}

int RegisteredFacesDB::GetPersonTotalNumByPersonUUIDFromRAM(QString uuid)
{
	Q_D(RegisteredFacesDB);
	int count = 0;
	d->sync.lock();
    for(int i = 0; i<d->mPersons.count(); i++)
    {
        auto &t = d->mPersons.at(i);
        if(t.uuid == uuid)
        {
        	count++;
        }
    }

	d->sync.unlock();
	return count;
}

QList<PERSONS_t> RegisteredFacesDB::GetPersonDataByTimeFromRAM(int nCurrPage,int nPerPage,QTime startTime,QTime endTime)
{
	Q_D(RegisteredFacesDB);
	QList<PERSONS_t> result;
	d->sync.lock();
	for (int i = nCurrPage; i < nCurrPage + nPerPage; i++)
    {
		if(i < 0 || d->mPersons.size() <= i)
		{
			break;
		}
        auto &t = d->mPersons.at(i);
        QTime tTime = QTime::fromString(t.createtime,"yyyy/MM/dd hh:mm:ss");
        if(tTime >= startTime && tTime <=  endTime)
        {
        	result.append(t);
        }
    }

	d->sync.unlock();
	return result;
}

int RegisteredFacesDB::GetPersonTotalNumByTimeFromRAM(QTime startTime,QTime endTime)
{
	Q_D(RegisteredFacesDB);
	int count = 0;
	d->sync.lock();
	for (int i = 0; i < d->mPersons.size(); i++)
    {
        auto &t = d->mPersons.at(i);
        QTime tTime = QTime::fromString(t.createtime,"yyyy/MM/dd hh:mm:ss");
        if(tTime >= startTime && tTime <=  endTime)
        {
        	count++;
        }
    }

	d->sync.unlock();
	return count;
}

bool RegisteredFacesDB::DelPersonByPersionUUIDFromDBAndRAM(QString uuid)
{/*删除数据库信息*/
    Q_D(RegisteredFacesDB);
    QMutexLocker lock(&d->sync);
    LogD("%s %s[%d] delete %s \n", __FILE__, __FUNCTION__, __LINE__, uuid.toStdString().c_str());
    QSqlQuery query(QSqlDatabase::database("isc_arcsoft_face"));
    query.prepare(QString("DELETE FROM person WHERE uuid='%1'").arg(uuid));
    bool ret = query.exec();

    for(int i = 0; i<d->mPersons.count(); i++)
    {
        auto &t = d->mPersons.at(i);

        if(t.uuid == uuid)
        {
            d->mPersons.removeAt(i);
            qXLApp->GetFaceMainFrm()->updateHome_PersonNum();
            return ret;
        }
    }
    return ret;
}
