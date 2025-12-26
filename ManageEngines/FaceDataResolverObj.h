#ifndef FaceDataResolverObj_H
#define FaceDataResolverObj_H

#include <QThread>
#include <QScopedPointer>
#include "SharedInclude/CallBindDef.h"
#include "SharedInclude/GlobalDef.h"

//任务处理线程
class FaceDataResolverObjPrivate;
class FaceDataResolverObj : public QThread
{
    Q_OBJECT
public:
    FaceDataResolverObj(QObject *parent = Q_NULLPTR);
    ~FaceDataResolverObj();
public:
    void ResolverData(const CORE_FACE_S &);
    void ClearData();
public:
    void SafeResolverData(const CORE_FACE_S &);
    void SafeClearData();
public:
    //设置处理完成时回调通知
    void EchoFaceRecognition(FF_CALLBACK(void(const int &id, const int &FaceType, const int &face_personid, const int &face_persontype,
                                              const QString &face_name, const QString &face_sex, const QString &face_uuid, const QString &face_idcardnum,
                                              const QString &face_iccardnum, const QString &face_gids, const QString &face_aids, const QByteArray &face_feature))call);
private:
    void run();
private:
    QScopedPointer<FaceDataResolverObjPrivate>d_ptr;
private:
    Q_DISABLE_COPY(FaceDataResolverObj)
    Q_DECLARE_PRIVATE(FaceDataResolverObj)
};

#endif