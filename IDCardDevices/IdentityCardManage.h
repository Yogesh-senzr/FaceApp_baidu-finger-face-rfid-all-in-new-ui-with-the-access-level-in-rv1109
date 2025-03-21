#ifndef IDENTITYCARDMANAGE_H
#define IDENTITYCARDMANAGE_H

#include <QObject>
//身份证模块管理
class IdentityCardManagePrivate;
class IdentityCardManage : public QObject
{
    Q_OBJECT
public:
    explicit IdentityCardManage(QObject *parent = nullptr);
    ~IdentityCardManage();
public:
    //上传当前身份证信息
    Q_SIGNAL void sigIdentityCardInfo(const QString name, const QString idCard, const QString sex, const QString path);
private:
    QScopedPointer<IdentityCardManagePrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(IdentityCardManage)
    Q_DISABLE_COPY(IdentityCardManage)
};

#endif // IDENTITYCARDMANAGE_H
