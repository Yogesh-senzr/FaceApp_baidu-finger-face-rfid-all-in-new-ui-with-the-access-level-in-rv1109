#ifndef HEALTHCODEMANAGE_H
#define HEALTHCODEMANAGE_H

#include <QObject>
#include "SharedInclude/GlobalDef.h"
//远景达
class HealthCodeManagePrivate;
class HealthCodeManage : public QObject
{
    Q_OBJECT
public:
    explicit HealthCodeManage(QObject *parent = nullptr);
    ~HealthCodeManage();
public://身份证扫描信息， 用于查键康码
    Q_SLOT void slotDKQueryHealthCode(const QString name, const QString idCard, const QString sex);
public:
    //健康码信息(type=(1健康码扫描头 2身份证查询)
    Q_SIGNAL void sigHealthCodeInfo(const int type, const QString name, const QString idCard, const int qrCodeType, const double warningTemp, const QString msg);
    Q_SIGNAL void sigLRHealthCodeInfo(const int type, const QString name, const QString idCard, const int qrCodeType, const double warningTemp, const QString msg);
    Q_SIGNAL void sigLRHealthCodeInfo( HEALTINFO_t info) const;
private:
    QScopedPointer<HealthCodeManagePrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(HealthCodeManage)
    Q_DISABLE_COPY(HealthCodeManage)
};

#endif // HEALTHCODEMANAGE_H
