#ifndef IDENTITYMANAGEMENT_H
#define IDENTITYMANAGEMENT_H

#include <QObject>

//识别管理(主要用于处理识别到人脸数据， 进行检测人脸、通行、上传、服务器， 等等交互信息)
class IdentityManagementPrivate;
class IdentityManagement : public QObject
{
    Q_OBJECT
public:
    explicit IdentityManagement(QObject *parent = nullptr);
    ~IdentityManagement();
public:
    Q_SLOT void slotFaceInitState(const bool);
private:
    QScopedPointer<IdentityManagementPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(IdentityManagement)
    Q_DISABLE_COPY(IdentityManagement)
};

#endif // IDENTITYMANAGEMENT_H
