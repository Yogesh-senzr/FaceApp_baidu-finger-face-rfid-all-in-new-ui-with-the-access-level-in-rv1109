#include "FaceDB.h"
#include "dbtable.h"

#include "MessageHandler/Log.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>
#include <QSqlDriver>
#include <QSqlRecord>

#define DB_TYPE_SQLITE 1
#ifdef Q_OS_LINUX
#define DB_SAVEDIR "/mnt/user/facedb"
#else
#define DB_SAVEDIR "./"
#endif

#define DB_ARCSOFT_FACE_PATH  DB_SAVEDIR"/isc_arcsoft_face.db"
#define IR_ARCSOFT_FACE_PATH  DB_SAVEDIR"/isc_ir_arcsoft_face.db"

class FaceDBPrivate
{
    Q_DECLARE_PUBLIC(FaceDB)
public:
    FaceDBPrivate(FaceDB *dd);
private:
    void InitDB();
    void InitFaceDB();
    void InitIrDB();
private:
    QSqlDatabase Facedb;
    QSqlDatabase FaceIrdb;
private:
    FaceDB *const q_ptr;
};


FaceDBPrivate::FaceDBPrivate(FaceDB *dd)
    : q_ptr(dd)
{
    this->InitDB();
}

FaceDB::FaceDB(QObject *parent)
    : QObject(parent)
    , d_ptr(new FaceDBPrivate(this))
{
}

FaceDB::~FaceDB()
{

}

void FaceDBPrivate::InitDB()
{
    this->InitFaceDB();
    this->InitIrDB();
}

void FaceDBPrivate::InitFaceDB()
{
    Facedb = QSqlDatabase::addDatabase("QSQLITE", "isc_arcsoft_face");
    Facedb.setDatabaseName(DB_ARCSOFT_FACE_PATH);
    if( !Facedb.open())
    {
        LogV("%s %s[%d] Error: Failed to connect Face database. %s \n", __FILE__, __FUNCTION__, __LINE__, Facedb.lastError().text().toStdString().c_str());
    }else
    {
        QByteArray szSql;
        QSqlQuery sql_query(Facedb);
        //建立person表，存储人员信息
        _CREATE_PERSON_TABLE(szSql);
        sql_query.prepare(szSql);
        sql_query.exec();
        //新增person表组关联字段
        _INSERT_PERSON_COLUMN_GIDS(szSql);
        sql_query.prepare(szSql);
        sql_query.exec();
#if 0
        //增加存放人头像图片(不需要)
        _INSERT_PERSON_COLUMN_CATCH_IMG(szSql);
        sql_query.prepare(szSql);
        sql_query.exec();
#endif
        /**aids**/
        _INSERT_PERSON_COLUMN_AIDS(szSql);
        sql_query.prepare(szSql);
        sql_query.exec();

        //新增person表权限关联字段
        _INSERT_PERSON_COLUMN_TIMEOFACCESS(szSql);
        sql_query.prepare(szSql);
        sql_query.exec();
        //建立action表，存储行为信息
        _CREATE_ACTION_TABLE(szSql);
        sql_query.prepare(szSql);
        sql_query.exec();
        //建立_group表，存储组信息
        _CREATE_GROUP_TABLE(szSql);
        sql_query.prepare(szSql);
        sql_query.exec();
        /*通行时段表*/
        _CREATE_PASSAGETIME_TABLE(szSql);
        sql_query.prepare(szSql);
        sql_query.exec();

        /**增加部门名称**/
        _INSERT_PERSON_COLUMN_DEPARTMENT(szSql);
        sql_query.prepare(szSql);
        sql_query.exec();
    }
}

void FaceDBPrivate::InitIrDB()
{
    FaceIrdb = QSqlDatabase::addDatabase("QSQLITE", "isc_ir_arcsoft_face");
    FaceIrdb.setDatabaseName(IR_ARCSOFT_FACE_PATH);
    if( !FaceIrdb.open())
    {
        LogV("%s %s[%d] Error: Failed to connect FaceIr database. %s \n", __FILE__, __FUNCTION__, __LINE__, FaceIrdb.lastError().text().toStdString().c_str());
    }else
    {
        QByteArray szSql;
        QSqlQuery sql_query(FaceIrdb);
        //建立识别记录表，存储人员识别记录信息
        _CREATE_IDENTIFYRECORD_TABLE(szSql);
        sql_query.prepare(szSql);
        sql_query.exec();
        //新增identifyrecord表组关联字段
        _INSERT_IDENTIFYRECORD_COLUMN_GIDS(szSql);
        sql_query.prepare(szSql);
        sql_query.exec();
        //新增identifyrecord表权限关联字段
        _INSERT_IDENTIFYRECORD_COLUMN_PIDS(szSql);
        sql_query.prepare(szSql);
        sql_query.exec();
        //新增identifyrecord表ID卡号关联字段
        _INSERT_IDENTIFYRECORD_COLUMN_IDCARDNUM(szSql);
        sql_query.prepare(szSql);
        sql_query.exec();
        //新增identifyrecord表IC卡号关联字段
        _INSERT_IDENTIFYRECORD_COLUMN_ICCARDNUM(szSql);
        sql_query.prepare(szSql);
        sql_query.exec();
        //新增identifyrecord表 是否已上传字段
        _INSERT_IDENTIFYRECORD_COLUMN_UPLOADFLAG(szSql);
        sql_query.prepare(szSql);
        sql_query.exec();
        //新增identifyrecord表 温度记录
        _INSERT_IDENTIFYRECORD_COLUMN_TEMPVALUE(szSql);
        sql_query.prepare(szSql);
        sql_query.exec();
        //新增identifyrecord存储人员识别记录sex信息
        _INSERT_IDENTIFYRECORD_COLUMN_SEX(szSql);
        sql_query.prepare(szSql);
        sql_query.exec();
        //新增identifyrecord存储某些客户特殊的msg信息
        _INSERT_IDENTIFYRECORD_COLUMN_MSG(szSql);
        sql_query.prepare(szSql);
        sql_query.exec();
    }
}
