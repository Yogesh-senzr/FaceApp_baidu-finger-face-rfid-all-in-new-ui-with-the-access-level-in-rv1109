#ifndef REGISTEREDFACESDB_H
#define REGISTEREDFACESDB_H

#include <QtCore/QThread>
#include "SharedInclude/GlobalDef.h"

class PERSONS_s;
class RegisteredFacesDBPrivate;
class RegisteredFacesDB : public QThread
{
    Q_OBJECT
public:
    RegisteredFacesDB(QObject *parent = Q_NULLPTR);
    ~RegisteredFacesDB();
public:
    static inline RegisteredFacesDB *GetInstance(){static RegisteredFacesDB g;return &g;}
public:
    void setThanthreshold(const float &);
public:
    bool selectICcardPerson(const QString &, PERSONS_s &s);
    bool selectIDcardPerson(const QString &, PERSONS_s &s);
    bool CheckPassageOfTime(QString person_uuid);
public:
    /*uuid , 姓名、身份证、卡号、性别、部门、通行时间、特征值*/
    bool RegPersonToDBAndRAM(const QString &uuid,const QString &Name, const QString &idCard, const QString &icCard, const QString &Sex, const QString &department, const QString &timeOfAccess, const QByteArray &FaceFeature);

    //更新人员
    int UpdatePersonToDBAndRAM(const QString &uuid,const QString &name, const QString &idCard, const QString &icCard, const QString &sex, const QString &department, const QString &timeOfAccess, const QString &jpeg);

    //批量人员注册
    bool RegPersonToDBAndRAM(const QString &name, const QString &sex, const QString &nation, const QString &idcardnum,
                       const QString &Marital, const QString &education, const QString &location, const QString &phone,
                       const QString &politics_status, const QString &date_birth, const QString &number,const QString &branch,
                       const QString &hiredate, const QString &extension_phone, const QString &mailbox, const QString &status,
                       const QString &persontype, const QString &iccardnum, const QByteArray &);

    bool ComparisonPersonFaceFeature(QString &name, QString &sex, QString &idcard, QString &iccard, QString &uuid, int &persontype, int &personid, QString &gids, QString &pids, const QByteArray &FaceFeature);
    bool ComparisonPersonFaceFeature_baidu(QString &name, QString &sex, QString &idcard, QString &iccard, QString &uuid, int &persontype, int &personid, QString &gids, QString &pids, unsigned char *FaceFeature, int FaceFeatureSize);
    //删除人
    bool DelPersonDBInfo(const QString &name, const QString &createtime);
    //查询人是否重名
    bool QueryNameRepetition(const QString &name)const;

    //获取内存中的所有人员
    QList<PERSONS_t> GetAllPersonFromRAM();

    //根据名称，分页查询内存中的人员
    QList<PERSONS_t> GetPersonDataByNameFromRAM(int nCurrPage,int nPerPage,QString name);
    //根据名称，获得所有人员的总数
    int GetPersonTotalNumByNameFromRAM(QString name);

    //根据uuid，分页查询内存中的人员
    QList<PERSONS_t> GetPersonDataByPersonUUIDFromRAM(QString uuid);
    //根据uuid，获得所有人员的总数
    int GetPersonTotalNumByPersonUUIDFromRAM(QString uuid);

    //根据人员的注册时间段，分页查询内存中的人员
    QList<PERSONS_t> GetPersonDataByTimeFromRAM(int nCurrPage,int nPerPage,QTime startTime,QTime endTime);
    //根据人员的注册时间段，获得所有人员的总数
    int GetPersonTotalNumByTimeFromRAM(QTime startTime,QTime endTime);

    //删除人
     bool DelPersonByPersionUUIDFromDBAndRAM(QString uuid);

private:
    void run();
private:
    QScopedPointer<RegisteredFacesDBPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(RegisteredFacesDB)
    Q_DISABLE_COPY(RegisteredFacesDB)
};

#endif // REGISTEREDFACESDB_H
