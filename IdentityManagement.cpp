#include "IdentityManagement.h"

#include <QThread>

class IdentityManagementPrivate
{
    Q_DECLARE_PUBLIC(IdentityManagement)
public:
    IdentityManagementPrivate(IdentityManagement *dd);
private:
    IdentityManagement *const q_ptr;
};

IdentityManagementPrivate::IdentityManagementPrivate(IdentityManagement *dd)
    : q_ptr(dd)
{

}

IdentityManagement::IdentityManagement(QObject *parent)
    : QObject(parent)
    , d_ptr(new IdentityManagementPrivate(this))
{
    QThread *thread = new QThread;
    this->moveToThread(thread);
    thread->start();
}

IdentityManagement::~IdentityManagement()
{

}

void IdentityManagement::slotFaceInitState(const bool)
{

}
