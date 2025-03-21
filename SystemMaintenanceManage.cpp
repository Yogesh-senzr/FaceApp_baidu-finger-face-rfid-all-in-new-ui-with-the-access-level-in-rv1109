#include "SystemMaintenanceManage.h"
#include "Helper/myhelper.h"

#include <QThread>
#include <QTimer>
#include <QDateTime>
#include <QDebug>

class SystemMaintenanceManagePrivate
{
    Q_DECLARE_PUBLIC(SystemMaintenanceManage)
public:
    SystemMaintenanceManagePrivate(SystemMaintenanceManage *dd);
private:
    QTimer *m_pBootTimer;
private:
    SystemMaintenanceManage *const q_ptr;
};

SystemMaintenanceManagePrivate::SystemMaintenanceManagePrivate(SystemMaintenanceManage *dd)
    : q_ptr(dd)
    , m_pBootTimer(new QTimer)
{
    QObject::connect(m_pBootTimer, &QTimer::timeout, [&]{
    	printf("%s %s[%d] \n",__FILE__,__FUNCTION__,__LINE__);
        myHelper::Utils_Reboot();
    });
}

SystemMaintenanceManage::SystemMaintenanceManage(QObject *parent)
    : QObject(parent)
    , d_ptr(new SystemMaintenanceManagePrivate(this))
{
    QObject::connect(this, &SystemMaintenanceManage::sigBootTimer, this, &SystemMaintenanceManage::slotBootTimer);
    QThread *thread = new QThread;
    d_func()->m_pBootTimer->moveToThread(thread);
    this->moveToThread(thread);
    thread->start();
}

SystemMaintenanceManage::~SystemMaintenanceManage()
{

}

void SystemMaintenanceManage::slotBootTimer(const int mode, const QString t)
{
    Q_D(SystemMaintenanceManage);
    switch(mode)
    {
    case 1:
    {//月
        QDate date = QDate::currentDate();
        QDateTime time = QDateTime::currentDateTime(); // 获取当前时间
        int year = time.date().year(); // 年
        int month = time.date().month(); //月
        qint64 t1 = QDateTime::fromString(QString("%1-%2-%3 %4:00").arg(year).arg(month).arg(date.daysInMonth()).arg(t), "yyyy-MM-dd hh:mm:ss").toSecsSinceEpoch();
        qint64 t2 = QDateTime::currentSecsSinceEpoch();
        if(t2>t1)
        {
            date = date.addMonths(1);
            t1 = QDateTime::fromString(QString("%1-%2 %3:00").arg(date.toString("yyyy-MM")).arg(date.daysInMonth()).arg(t), "yyyy-MM-dd hh:mm:ss").toSecsSinceEpoch();
        }
        qint64 value = t1  - t2;
        d->m_pBootTimer->start(value *1000);
        qDebug()<<"wait month timer:"<<value*1000;
    }break;
    case 2:
    {//周

        QDate date = QDate::currentDate();
        date = date.addDays(-date.dayOfWeek()+1);
        qDebug()<<date.addDays(6).toString("yyyy-MM-dd");
        qint64 t1 = QDateTime::fromString(QString("%1 %2:00").arg(date.addDays(6).toString("yyyy-MM-dd")).arg(t), "yyyy-MM-dd hh:mm:ss").toSecsSinceEpoch();
        qint64 t2 = QDateTime::currentSecsSinceEpoch();
        if(t2>t1)
        {
            t1 = QDateTime::fromString(QString("%1 %2:00").arg(QDate::currentDate().addDays(6).toString("yyyy-MM-dd")).arg(t), "yyyy-MM-dd hh:mm:ss").toSecsSinceEpoch();
        }
        qint64 value = t1  - t2;
        d->m_pBootTimer->start(value *1000);
        qDebug()<<"wait Days timer:"<<value*1000;
    }break;
    case 3:
    {//日
        qint64 t1 = QDateTime::fromString(QString("%1 %2:00").arg(QDate::currentDate().toString("yyyy-MM-dd")).arg(t), "yyyy-MM-dd hh:mm:ss").toSecsSinceEpoch();
        qint64 t2 = QDateTime::currentSecsSinceEpoch();
        if(t2>t1)
        {//已经超过时间了
            t1 = QDateTime::fromString(QString("%1 %2:00").arg(QDate::currentDate().addDays(1).toString("yyyy-MM-dd")).arg(t), "yyyy-MM-dd hh:mm:ss").toSecsSinceEpoch();
        }
        qint64 value = t1  - t2;
        d->m_pBootTimer->start(value *1000);
        qDebug()<<"wait today timer:"<<value*1000;
    }break;
    default:d->m_pBootTimer->stop();break;
    }
}
