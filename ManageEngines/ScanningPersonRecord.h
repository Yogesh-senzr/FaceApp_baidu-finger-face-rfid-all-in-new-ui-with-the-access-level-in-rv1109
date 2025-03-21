#ifndef SCANNINGPERSONRECORD_H
#define SCANNINGPERSONRECORD_H

#include <QThread>

//主要用于扫描已进行识别过的记录
class ScanningPersonRecordPrivate;
class ScanningPersonRecord : public QThread
{
    Q_OBJECT
public:
    ScanningPersonRecord(QObject *parent = Q_NULLPTR);
    ~ScanningPersonRecord();
public:
    void appPersonRecord(const QString &name, const int &type, const int &);
    bool CheckQueueValid(const QString &name);
private:
    void run();
private:
    QScopedPointer<ScanningPersonRecordPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(ScanningPersonRecord)
    Q_DISABLE_COPY(ScanningPersonRecord)
};

#endif // SCANNINGPERSONRECORD_H
