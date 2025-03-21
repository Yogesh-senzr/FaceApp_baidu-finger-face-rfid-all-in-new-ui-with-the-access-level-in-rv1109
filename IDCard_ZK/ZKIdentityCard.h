#ifndef ZKIdentityCard_H_
#define ZKIdentityCard_H_

#include <QThread>


typedef struct 
{
    char szName[64];
    char szIDCardNum[64];
    char szIDAddress[200];
    char szSex[8];
    char szNationality[20];
    char szBirthDate[20];
    char szIssuingAuthority[200];
    char szStartDate[20];
    char szEndDate[20];    
} IDCardInfo;

class ZKIdentityCardPrivate;
class ZKIdentityCard : public QThread
{
    Q_OBJECT
public:
    ZKIdentityCard(QObject *parent = Q_NULLPTR);
    ~ZKIdentityCard();
public://上传当前身份证信息
    Q_SIGNAL void sigIdCardInfo(const QString name, const QString idCard, const QString sex, const QString path);
    Q_SIGNAL void sigIdCardFullInfo(const QString name,const QString idCardNo,const QString address,const QString sex,
        const QString nationality,const QString birthDate,const QString issuingAuthority,const QString startDate, const QString endDate);
private:
    void run();
private:
    int deviceIndex;
    QScopedPointer<ZKIdentityCardPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(ZKIdentityCard)
    Q_DISABLE_COPY(ZKIdentityCard)
};


#endif /* ISC_FACE_APP_DERKIOT_DKIdentityCard_H_ */
