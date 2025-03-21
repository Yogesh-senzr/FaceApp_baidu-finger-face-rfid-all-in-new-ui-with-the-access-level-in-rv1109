#ifndef IDENTITYCARD_DD_H
#define IDENTITYCARD_DD_H
#include "EidSdkStruct.hpp"
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

class IdentityCardDDPrivate;
class IdentityCardDD : public QThread
//class IdentityCardDD : public QObject
{
    Q_OBJECT
public:
    IdentityCardDD(QObject *parent = Q_NULLPTR);
    ~IdentityCardDD();
    //static void EsIdcardCB(int notify, const char* data, int dataLen, LPVOID lpVoid);
    static inline IdentityCardDD *GetInstance(){static IdentityCardDD g;
    printf(">>>>%s,%s,%d,\n",__FILE__,__func__,__LINE__);	
    
    return &g;}
public://上传当前身份证信息
    void sendIdCardInfo(const QString name, const QString idCard, const QString sex, const QString path);
    Q_SIGNAL void sigIdCardInfoDD(const QString name, const QString idCard, const QString sex, const QString path);
    //Q_SIGNAL void sigIdCardFullInfo(const QString name,const QString idCardNo,const QString address,const QString sex, \
        const QString nationality,const QString birthDate,const QString issuingAuthority,const QString startDate, const QString endDate);
   // Q_SLOT  void slotIdCardInfoDDlocal(const QString name, const QString idCard, const QString sex, const QString path);             
private:
    void run();
private:
    int deviceIndex;
    QScopedPointer<IdentityCardDDPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(IdentityCardDD)
    Q_DISABLE_COPY(IdentityCardDD)
};


#endif /* IDENTITYCARD_DD_H */
