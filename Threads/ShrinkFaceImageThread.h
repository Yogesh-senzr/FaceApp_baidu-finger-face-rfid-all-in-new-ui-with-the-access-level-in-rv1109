#ifndef SHRINKFACEIMAGE_H
#define SHRINKFACEIMAGE_H

#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include <QSqlQuery>
#include <QSqlDatabase>

//监控磁盘线程
class ShrinkFaceImageThread : public QThread
{
    Q_OBJECT
public:
    ShrinkFaceImageThread(QObject *parent = Q_NULLPTR);
    ~ShrinkFaceImageThread();
public:
    static inline ShrinkFaceImageThread *GetInstance(){static ShrinkFaceImageThread g;return &g;}
private:
    void run();
	void doShrinkFaceImage();
private:
    mutable QMutex sync;
    QWaitCondition pauseCond;
public:
    Q_SIGNAL void sigShrinkTip(const QString);//发送Shrink提示	
};

#endif // SHRINKFACEIMAGE_H
