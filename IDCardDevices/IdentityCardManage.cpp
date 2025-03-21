#include "IdentityCardManage.h"
#ifdef Q_OS_LINUX
#include "DKIdentityCard.h"
#endif

class IdentityCardManagePrivate
{
    Q_DECLARE_PUBLIC(IdentityCardManage)
public:
    IdentityCardManagePrivate(IdentityCardManage *dd);
private:
    void Init();
    void InitConnect();
private:
#ifdef Q_OS_LINUX
    DKIdentityCard *m_pDKIdentityCard;
#endif
private:
    IdentityCardManage *const q_ptr;
};

IdentityCardManagePrivate::IdentityCardManagePrivate(IdentityCardManage *dd)
    : q_ptr(dd)
{
    this->Init();
    this->InitConnect();
}

IdentityCardManage::IdentityCardManage(QObject *parent)
    : QObject(parent)
    , d_ptr(new IdentityCardManagePrivate(this))
{

}

IdentityCardManage::~IdentityCardManage()
{
}

void IdentityCardManagePrivate::Init()
{
#ifdef Q_OS_LINUX
    m_pDKIdentityCard = new DKIdentityCard;
#endif
}

void IdentityCardManagePrivate::InitConnect()
{
#ifdef Q_OS_LINUX
    QObject::connect(m_pDKIdentityCard, &DKIdentityCard::sigIdCardInfo, q_func(), &IdentityCardManage::sigIdentityCardInfo);
#endif
}
