#ifndef IDENTITYCARD_ZK_MANAGE_H
#define IDENTITYCARD_ZK_MANAGE_H

#include <QObject>
//身份证模块管理
class IdentityCard_ZK_ManagePrivate;

class IdentityCard_ZK_Manage : public QObject
{
    Q_OBJECT
public:
    explicit IdentityCard_ZK_Manage(QObject *parent = nullptr);
    ~IdentityCard_ZK_Manage();
public:
    //上传当前身份证信息
    Q_SIGNAL void sigZKIdentityCardInfo(const QString name, const QString idCard, const QString sex, const QString path);
    
    Q_SIGNAL void sigZKIdentityCardFullInfo(const QString name,const QString idCardNo,const QString address,const QString sex,
        const QString nationality,const QString birthDate,const QString issuingAuthority,const QString startDate, const QString endDate);
private:
    QScopedPointer<IdentityCard_ZK_ManagePrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(IdentityCard_ZK_Manage)
    Q_DISABLE_COPY(IdentityCard_ZK_Manage)
};

#endif // IDENTITYCARD_ZK_MANAGE_H
