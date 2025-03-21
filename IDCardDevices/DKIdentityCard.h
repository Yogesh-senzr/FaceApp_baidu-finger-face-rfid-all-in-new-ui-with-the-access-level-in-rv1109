#ifndef DKIdentityCard_H_
#define DKIdentityCard_H_

#include <QThread>

class DKIdentityCardPrivate;
class DKIdentityCard : public QThread
{
    Q_OBJECT
public:
    DKIdentityCard(QObject *parent = Q_NULLPTR);
    ~DKIdentityCard();
public://上传当前身份证信息
    Q_SIGNAL void sigIdCardInfo(const QString name, const QString idCard, const QString sex, const QString path);
private:
    void run();
private:
    QScopedPointer<DKIdentityCardPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(DKIdentityCard)
    Q_DISABLE_COPY(DKIdentityCard)
};


#endif /* ISC_FACE_APP_DERKIOT_DKIdentityCard_H_ */
