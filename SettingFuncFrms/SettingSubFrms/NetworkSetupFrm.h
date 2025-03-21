#ifndef NETWORKSETUPFRM_H
#define NETWORKSETUPFRM_H

#include <QWidget>

//网络设置UI
class NetworkSetupFrmPrivate;
class NetworkSetupFrm : public QWidget
{
    Q_OBJECT
public:
    explicit NetworkSetupFrm(QWidget *parent = nullptr);
    ~NetworkSetupFrm();
private:
    QScopedPointer<NetworkSetupFrmPrivate>d_ptr;  
private:
    Q_DECLARE_PRIVATE(NetworkSetupFrm)
    Q_DISABLE_COPY(NetworkSetupFrm)
};

#endif // NETWORKSETUPFRM_H
