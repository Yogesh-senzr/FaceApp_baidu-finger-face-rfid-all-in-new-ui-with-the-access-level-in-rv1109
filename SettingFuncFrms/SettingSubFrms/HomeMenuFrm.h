#ifndef HomeMenuFrm_H
#define HomeMenuFrm_H

#include <QWidget>

//首页配置功能菜单UI
class HomeMenuFrmPrivate;
class HomeMenuFrm : public QWidget
{
    Q_OBJECT
public:
    explicit HomeMenuFrm(QWidget *parent = nullptr);
    ~HomeMenuFrm();
public:
    Q_SIGNAL void sigManagingPeopleClicked();//人员管理
    Q_SIGNAL void sigRecordsManagementClicked();//记录管理
    Q_SIGNAL void sigNetworkSetupClicked();//网络设置
    Q_SIGNAL void sigSrvSetupClicked();//服务器设置
    Q_SIGNAL void sigSysSetupClicked();//系统设置
    Q_SIGNAL void sigIdentifySetupClicked();//识别设置
private:
    QScopedPointer<HomeMenuFrmPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(HomeMenuFrm)
    Q_DISABLE_COPY(HomeMenuFrm)
};

#endif // HomeMenuFrm_H
