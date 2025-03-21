#ifndef PERSONRECORDTODB_H
#define PERSONRECORDTODB_H

#include <QtCore/QObject>
#include "SharedInclude/GlobalDef.h"

class PersonRecordToDBPrivate;
class PersonRecordToDB : public QObject
{
    Q_OBJECT
public:
    PersonRecordToDB(QObject *parent = Q_NULLPTR);
    ~PersonRecordToDB();
public:
    static inline PersonRecordToDB *GetInstance(){static PersonRecordToDB g; return &g;}
public:
    /*保存实别记录：全景图、人脸图、陌生人*/
    void setRecords_PanoramaImg(const bool &);
    void setRecords_FaceImg(const bool &);
    void setRecords_Stranger(const bool &);

    bool UpdatePersonRecordUploadFlag(int rid, bool isUpload);
    //根据名称，分页查询识别记录， filterUpdateFlag 为true 表示过滤掉后台已上传的识别记录
    QList<IdentifyFaceRecord_t> GetPersonRecordDataByName(int nCurrPage,int nPerPage,QString name,bool filterUpdateFlag);
    //根据名称，分页查询识别记录总数， filterUpdateFlag 为true 表示过滤掉后台已上传的识别记录
    int GetPersonRecordTotalNumByName(QString name,bool filterUpdateFlag);

    //根据用户uuid，分页查询识别记录， filterUpdateFlag 为true 表示过滤掉后台已上传的识别记录
    QList<IdentifyFaceRecord_t>  GetPersonRecordDataByPersonUUID(int nCurrPage,int nPerPage,QString uuid,bool filterUpdateFlag);
    //根据用户uuid，分页查询识别记录总数， filterUpdateFlag 为true 表示过滤掉后台已上传的识别记录
	int GetPersonRecordTotalNumByPersonUUID(QString uuid, bool filterUpdateFlag);

    //根据时间，分页查询识别记录， filterUpdateFlag 为true 表示过滤掉后台已上传的识别记录
    QList<IdentifyFaceRecord_t>  GetPersonRecordDataByDateTime(int nCurrPage,int nPerPage,QDateTime startDateTime,QDateTime endDateTime,bool filterUpdateFlag);
    //根据时间，分页查询识别记录总数， filterUpdateFlag 为true 表示过滤掉后台已上传的识别记录
	int GetPersonRecordTotalNumByDateTime(QDateTime startDateTime,QDateTime endDateTime, bool filterUpdateFlag);

	//根据名称删除识别记录
	bool DeletePersonRecordByName(QString name);
	//根据RID删除识别记录
	bool DeletePersonRecordByRID(long rid);
	//根据时间范围删除识别记录
	bool DeletePersonRecordByTime(QDateTime startDateTime,QDateTime endDateTime);

public:
    void appRecordData(const IdentifyFaceRecord_t &t){emit sigappRecordData(t);}
    bool appDoorRecordData(const int &persontype,const QString &img_path);
public:
    Q_SIGNAL void sigappRecordData(const IdentifyFaceRecord_t);
private:
    Q_SLOT void slotappRecordData(const IdentifyFaceRecord_t);
private:
    QScopedPointer<PersonRecordToDBPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(PersonRecordToDB)
    Q_DISABLE_COPY(PersonRecordToDB)
};

#endif // PERSONRECORDTODB_H
