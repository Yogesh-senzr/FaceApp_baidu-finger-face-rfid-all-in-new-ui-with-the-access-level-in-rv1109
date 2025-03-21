#ifndef SRVSETUPFRM_H
#define SRVSETUPFRM_H

#include <QWidget>

//服务器设置UI
class SrvSetupFrmPrivate;
class SrvSetupFrm : public QWidget
{
    Q_OBJECT
public:
    explicit SrvSetupFrm(QWidget *parent = nullptr);
    ~SrvSetupFrm();
private:
    QScopedPointer<SrvSetupFrmPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(SrvSetupFrm)
    Q_DISABLE_COPY(SrvSetupFrm)
};

#endif // SRVSETUPFRM_H
