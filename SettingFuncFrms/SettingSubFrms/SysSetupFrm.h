#ifndef SYSSETUPFRM_H
#define SYSSETUPFRM_H

#include <QWidget>

//系统设置UI
class SysSetupFrmPrivate;
class SysSetupFrm : public QWidget
{
    Q_OBJECT
public:
    explicit SysSetupFrm(QWidget *parent = nullptr);
    ~SysSetupFrm();
private:
    QScopedPointer<SysSetupFrmPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(SysSetupFrm)
    Q_DISABLE_COPY(SysSetupFrm)
};

#endif // SYSSETUPFRM_H
