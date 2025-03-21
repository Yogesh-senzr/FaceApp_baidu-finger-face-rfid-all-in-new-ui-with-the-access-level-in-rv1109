#ifndef IDENTITYCARD_DD_MANAGE_H
#define IDENTITYCARD_DD_MANAGE_H

#include <QObject>
//身份证模块管理
class IdentityCard_DD_ManagePrivate;

class IdentityCard_DD_Manage : public QObject
{
    Q_OBJECT
public:
    explicit IdentityCard_DD_Manage(QObject *parent = nullptr);
    ~IdentityCard_DD_Manage();
public:
    //上传当前身份证信息
    Q_SIGNAL void sigIdentityCardInfoDD(const QString name, const QString idCard, const QString sex, const QString path);
    
    //Q_SIGNAL void sigIdentityCardFullInfoDD(const QString name,const QString idCardNo,const QString address,const QString sex,\
        const QString nationality,const QString birthDate,const QString issuingAuthority,const QString startDate, const QString endDate);
    //Q_SLOT  void slotIdCardInfoDD(const QString name, const QString idCard, const QString sex, const QString path);          
   // Q_SLOT  void slotIdCardInfoDDlocal(const QString name, const QString idCard, const QString sex, const QString path);       
private:
    QScopedPointer<IdentityCard_DD_ManagePrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(IdentityCard_DD_Manage)
    Q_DISABLE_COPY(IdentityCard_DD_Manage)
};

#endif // IDENTITYCARD_DD_MANAGE_H
