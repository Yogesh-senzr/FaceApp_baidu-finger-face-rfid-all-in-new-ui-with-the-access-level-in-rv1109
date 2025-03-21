#include "IdentityCard_ZK_Manage.h"
#ifdef Q_OS_LINUX
#include "ZKIdentityCard.h"
#endif

class IdentityCard_ZK_ManagePrivate
{
    Q_DECLARE_PUBLIC(IdentityCard_ZK_Manage)
public:
    IdentityCard_ZK_ManagePrivate(IdentityCard_ZK_Manage *dd);
private:
    void Init();
    void InitConnect();
private:
#ifdef Q_OS_LINUX
    ZKIdentityCard *m_pZKIdentityCard;
#endif
private:
    IdentityCard_ZK_Manage *const q_ptr;
};

IdentityCard_ZK_ManagePrivate::IdentityCard_ZK_ManagePrivate(IdentityCard_ZK_Manage *dd)
    : q_ptr(dd)
{
    this->Init();
    this->InitConnect();
}

IdentityCard_ZK_Manage::IdentityCard_ZK_Manage(QObject *parent)
    : QObject(parent)
    , d_ptr(new IdentityCard_ZK_ManagePrivate(this))
{

}

IdentityCard_ZK_Manage::~IdentityCard_ZK_Manage()
{
}

void IdentityCard_ZK_ManagePrivate::Init()
{
#ifdef Q_OS_LINUX
    m_pZKIdentityCard = new ZKIdentityCard;
      
#endif
}

void IdentityCard_ZK_ManagePrivate::InitConnect()
{
#ifdef Q_OS_LINUX
    QObject::connect(m_pZKIdentityCard, &ZKIdentityCard::sigIdCardInfo, q_func(), &IdentityCard_ZK_Manage::sigZKIdentityCardInfo);
    QObject::connect(m_pZKIdentityCard, &ZKIdentityCard::sigIdCardFullInfo, q_func(), &IdentityCard_ZK_Manage::sigZKIdentityCardFullInfo);
    
#endif
}
