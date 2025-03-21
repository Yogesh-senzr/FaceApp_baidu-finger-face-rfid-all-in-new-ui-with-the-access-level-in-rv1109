#ifndef SYSTEMMAINTENANCEMANAGE_H
#define SYSTEMMAINTENANCEMANAGE_H

#include <QObject>

//系统维护管理类（用于检测数据动态删除、定时重启）
class SystemMaintenanceManagePrivate;
class SystemMaintenanceManage : public QObject
{
    Q_OBJECT
public:
    explicit SystemMaintenanceManage(QObject *parent = nullptr);
    ~SystemMaintenanceManage();
public:
    static inline SystemMaintenanceManage *GetInstance(){static SystemMaintenanceManage g; return &g;}
public:
    void setBootTimer(const int mode, const QString t){emit sigBootTimer(mode, t);}
private:
    Q_SIGNAL void sigBootTimer(const int , const QString);
private:
    Q_SLOT void slotBootTimer(const int , const QString);
private:
    QScopedPointer<SystemMaintenanceManagePrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(SystemMaintenanceManage)
    Q_DISABLE_COPY(SystemMaintenanceManage)
};

#endif // SYSTEMMAINTENANCEMANAGE_H
