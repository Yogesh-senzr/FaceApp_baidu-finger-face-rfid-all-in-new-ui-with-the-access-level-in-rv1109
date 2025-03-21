#include "IdentityCard_DD_Manage.h"
#ifdef Q_OS_LINUX
#include "IdentityCard_dd.h"
#endif

class IdentityCard_DD_ManagePrivate
{
    Q_DECLARE_PUBLIC(IdentityCard_DD_Manage)
public:
    IdentityCard_DD_ManagePrivate(IdentityCard_DD_Manage *dd);
private:
    void Init();
    void InitConnect();   
private:
#ifdef Q_OS_LINUX
    IdentityCardDD *m_pIdentityCardDD;
#endif
private:
    IdentityCard_DD_Manage *const q_ptr;
};

IdentityCard_DD_ManagePrivate::IdentityCard_DD_ManagePrivate(IdentityCard_DD_Manage *dd)
    : q_ptr(dd)
{
    this->Init();
    this->InitConnect();
}

IdentityCard_DD_Manage::IdentityCard_DD_Manage(QObject *parent)
    : QObject(parent)
    , d_ptr(new IdentityCard_DD_ManagePrivate(this))
{

}

IdentityCard_DD_Manage::~IdentityCard_DD_Manage()
{
}

void IdentityCard_DD_ManagePrivate::Init()
{
#ifdef Q_OS_LINUX
    //m_pIdentityCardDD = new IdentityCardDD;
    m_pIdentityCardDD = IdentityCardDD::GetInstance();
      
#endif
}

#if 0
void IdentityCard_DD_Manage::slotIdCardInfoDDlocal(const QString name, const QString idCard, const QString sex, const QString path)
{
    printf(">>>>%s,%s,%d step 2 \n",__FILE__,__func__,__LINE__);
    emit sigIdentityCardInfoDD(name, idCard, sex, path);
}    
#endif 

void IdentityCard_DD_ManagePrivate::InitConnect()
{
#ifdef Q_OS_LINUX
    printf(">>>>%s,%s,%d IdentityCard_DD_ManagePrivate::InitConnect()  \n",__FILE__,__func__,__LINE__);
    QObject::connect(m_pIdentityCardDD, &IdentityCardDD::sigIdCardInfoDD, q_func(), &IdentityCard_DD_Manage::sigIdentityCardInfoDD);  
    //QObject::connect(m_pIdentityCardDD, &IdentityCardDD::sigIdCardInfoDD, q_func(), &IdentityCard_DD_Manage::slotIdCardInfoDDlocal);    
   // QObject::connect(m_pIdentityCardDD, &IdentityCardDD::sigIdCardFullInfo, q_func(), &IdentityCard_DD_Manage::sigIdentityCardFullInfoDD);
    
#endif
}
