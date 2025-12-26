#ifndef _POSTPERSONRECORDTHREAD_H_
#define _POSTPERSONRECORDTHREAD_H_

#include <QtCore/QThread>
#include <QtCore/QWaitCondition>
#include <QtCore/QMutex>
#include "SharedInclude/GlobalDef.h"

class PostPersonRecordThreadPrivate;
class PostPersonRecordThread : public QThread
{
    Q_OBJECT
public:
	PostPersonRecordThread(QObject *parent = Q_NULLPTR);
    ~PostPersonRecordThread();
public:
    static inline PostPersonRecordThread *GetInstance(){static PostPersonRecordThread g;return &g;}
private:
    void run();

public:
    void appRecordData(const IdentifyFaceRecord_t &t){emit sigAppRecordData(t);}
    bool pushRecordToServer(const QString &name, const QString &time, 
                                                 const QString &imgPath, int rid);
public:
    Q_SIGNAL void sigAppRecordData(const IdentifyFaceRecord_t);
    Q_SIGNAL void sigManualPushProgress(int current, int total, bool success);
    Q_SIGNAL void sigManualPushComplete(bool success, QString message);

public:
     Q_SLOT void slotManualPushRecords(const QDateTime &startDateTime, const QDateTime &endDateTime);

private:
    Q_SLOT void slotAppRecordData(const IdentifyFaceRecord_t);

private:
    QScopedPointer<PostPersonRecordThreadPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(PostPersonRecordThread)
    Q_DISABLE_COPY(PostPersonRecordThread)
};

#endif // _PostPersonRecordThread_H_
