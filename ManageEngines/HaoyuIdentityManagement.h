#ifndef HAOYUIDENTITYMANAGEMENT_H
#define HAOYUIDENTITYMANAGEMENT_H

#include "IdentityManagement.h"
//浩宇平台
class HaoyuIdentityManagementPrivate;
class HaoyuIdentityManagement : public IdentityManagement
{
    Q_OBJECT
public:
    HaoyuIdentityManagement(QObject *parent = Q_NULLPTR);
    ~HaoyuIdentityManagement();
private:
    Q_SIGNAL void sigPostFaceInfo(const float TemperatureValue, const int mask);
private:
    Q_SLOT void slotUserAuthResult(const bool, const QString);
private:
    Q_DECLARE_PRIVATE(HaoyuIdentityManagement)
    Q_DISABLE_COPY(HaoyuIdentityManagement)
};

#endif // HAOYUIDENTITYMANAGEMENT_H
