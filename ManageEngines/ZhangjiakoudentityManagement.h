#ifndef ZhangjiakoudentityManagement_H
#define ZhangjiakoudentityManagement_H

#include "IdentityManagement.h"

class ZhangjiakoudentityManagementPrivate;
class ZhangjiakoudentityManagement : public IdentityManagement
{
    Q_OBJECT
public:
    ZhangjiakoudentityManagement(QObject *parent = Q_NULLPTR);
    ~ZhangjiakoudentityManagement();
private:
    Q_SIGNAL void sigQueryHealthCode(const QString name, const QString idCard, const QString sex);
    Q_SIGNAL void sigPostTempRecognition(const QString, const QString personName, const QString idCard, const QString sex
                                         , const int deviceType, const int checkType, const int codeLevel
                                         , const float temperature, const float translate, const bool warning);
private:
    Q_SLOT virtual void slotIdentityCardInfo(const QString name, const QString idCard, const QString sex, const QString path);
private:
    Q_DECLARE_PRIVATE(ZhangjiakoudentityManagement)
    Q_DISABLE_COPY(ZhangjiakoudentityManagement)
};

#endif // ZhangjiakoudentityManagement_H
