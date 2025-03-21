#include "HealthCodeManage.h"
#ifdef Q_OS_LINUX
#include "DKHealthCode.h"
#include "LRHealthCode.h"
#endif
#include "SharedInclude/GlobalDef.h"

class HealthCodeManagePrivate
{
    Q_DECLARE_PUBLIC(HealthCodeManage)
public:
    HealthCodeManagePrivate(HealthCodeManage *dd);
private:
    void Init();
    void InitConnect();
private:
#ifdef Q_OS_LINUX
    DKHealthCode *m_pDKHealthCode;//远景国康码
#endif

private:
    HealthCodeManage *const q_ptr;

};

HealthCodeManagePrivate::HealthCodeManagePrivate(HealthCodeManage *dd)
    : q_ptr(dd)
{
	qRegisterMetaType<HEALTINFO_t>("HEALTINFO_t");    
    this->Init();
    this->InitConnect();
    

}

HealthCodeManage::HealthCodeManage(QObject *parent)
    : QObject(parent)
    , d_ptr(new HealthCodeManagePrivate(this))
{

}

HealthCodeManage::~HealthCodeManage()
{
}

void HealthCodeManagePrivate::Init()
{
#ifdef Q_OS_LINUX
    m_pDKHealthCode = new DKHealthCode;
#endif
}

void HealthCodeManagePrivate::InitConnect()
{
#ifdef Q_OS_LINUX
    QObject::connect(m_pDKHealthCode, &DKHealthCode::sigDKHealthCodeMsg, q_func(), &HealthCodeManage::sigHealthCodeInfo);
#endif
}

void HealthCodeManage::slotDKQueryHealthCode(const QString name, const QString idCard, const QString sex)
{
    Q_UNUSED(name);
    Q_UNUSED(idCard);
    Q_UNUSED(sex);
#ifdef Q_OS_LINUX
    Q_D(HealthCodeManage);
    //远景平台查询健码
    d->m_pDKHealthCode->setQueryHealthCode(name, idCard, sex);
#endif
}
