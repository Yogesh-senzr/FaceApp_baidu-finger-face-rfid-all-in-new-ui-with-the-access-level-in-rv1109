#include "FaceDataResolverObj.h"

#include "Application/FaceApp.h"
#include "DB/RegisteredFacesDB.h"

#include <QMutex>
#include <QThread>
#include <QDebug>
#include <QWaitCondition>

typedef FF_CALLBACK(void(const int &id, const int &FaceType, const int &face_personid, const int &face_persontype,
                         const QString &face_name, const QString &face_sex, const QString &face_uuid, const QString &face_idcardnum,
                         const QString &face_iccardnum, const QString &face_gids, const QString &face_aids, const QByteArray &face_feature)) FaceRecognitionCallBack;

class FaceDataResolverObjPrivate
{
    Q_DECLARE_PUBLIC(FaceDataResolverObj)
public:
    FaceDataResolverObjPrivate(FaceDataResolverObj *dd);
private:
    int CheckIsIdentifyFace(QString &name, QString &sex, QString &idcard, QString &iccard, QString &uuid, int &persontype, int &personid, QString &gids, QString &pids);//识别人脸
private:
    mutable QMutex sync;
    QWaitCondition pauseCond;
    volatile bool is_pause;
private:
    CORE_FACE_S mFaceTask;
private:
    FaceRecognitionCallBack _FaceRecognitionCallBack = nullptr;
private:
    FaceDataResolverObj *const q_ptr;
};

FaceDataResolverObjPrivate::FaceDataResolverObjPrivate(FaceDataResolverObj *dd)
    : q_ptr(dd)
    , is_pause(true)
{
}

FaceDataResolverObj::FaceDataResolverObj(QObject *parent)
    : QThread(parent)
    , d_ptr(new FaceDataResolverObjPrivate(this))
{
    this->start();
}

FaceDataResolverObj::~FaceDataResolverObj()
{
    Q_D(FaceDataResolverObj);
    Q_UNUSED(d);
    this->requestInterruption();
    d->is_pause = false;
    d->pauseCond.wakeOne();

    this->quit();
    this->wait();
}

void FaceDataResolverObj::ResolverData(const CORE_FACE_S &task)
{
    Q_D(FaceDataResolverObj);
    d->mFaceTask = task;
    d->is_pause = false;
    d->pauseCond.wakeOne();
}

void FaceDataResolverObj::ClearData()
{
    Q_D(FaceDataResolverObj);
    d->is_pause = true;
}

void FaceDataResolverObj::SafeResolverData(const CORE_FACE_S &task)
{
    Q_D(FaceDataResolverObj);
    d->sync.lock();
    d->mFaceTask = task;
    d->is_pause = false;
    d->sync.unlock();
    d->pauseCond.wakeOne();
}

void FaceDataResolverObj::SafeClearData()
{
    Q_D(FaceDataResolverObj);
    d->sync.lock();
    d->is_pause = true;
    d->sync.unlock();
}

void FaceDataResolverObj::EchoFaceRecognition(FF_CALLBACK(void(const int &id, const int &, const int &face_personid, const int &face_persontype,
                                                               const QString &face_name, const QString &face_sex, const QString &face_uuid, const QString &face_idcardnum,
                                                               const QString &face_iccardnum, const QString &face_gids, const QString &face_aids, const QByteArray &face_feature))call)
{
    Q_D(FaceDataResolverObj);
    d->_FaceRecognitionCallBack = call;
}

int FaceDataResolverObjPrivate::CheckIsIdentifyFace(QString &name, QString &sex, QString &idcard, QString &iccard, QString &uuid, int &persontype, int &personid, QString &gids, QString &pids)
{//查询数据注册人员(如果扛不住2W的人员不停查询就改用内存查询)
    //return RegisteredFacesDB::GetInstance()->ComparisonPersonFaceFeature(name, sex, idcard, iccard, uuid, persontype, personid, gids, pids, QByteArray().append((char *)this->mFaceTask.pFaceFeature, this->mFaceTask.nFaceFeatureSize)) ? NOT_STRANGER : STRANGER;

    return RegisteredFacesDB::GetInstance()->ComparisonPersonFaceFeature_baidu(name, sex, idcard, iccard, uuid, persontype, personid, gids, pids, (unsigned char *)this->mFaceTask.pFaceFeature, this->mFaceTask.nFaceFeatureSize) ? NOT_STRANGER : STRANGER;
}

void FaceDataResolverObj::run()
{
    Q_D(FaceDataResolverObj);
    while (!isInterruptionRequested())
    {
        d->sync.lock();
        if (d->is_pause)d->pauseCond.wait(&d->sync);
        QString name, sex, idcard, iccard, uuid, gids, pids;
        int persontype = 0;
        int personid = 0;
    
        int FaceType = d->CheckIsIdentifyFace(name, sex, idcard, iccard, uuid, persontype, personid, gids, pids);
        d->is_pause = true;
        d->sync.unlock();
    
        d->_FaceRecognitionCallBack(d->mFaceTask.track_id, FaceType, personid, persontype, name, sex, uuid, idcard, iccard, gids, pids, QByteArray());
    }
}
