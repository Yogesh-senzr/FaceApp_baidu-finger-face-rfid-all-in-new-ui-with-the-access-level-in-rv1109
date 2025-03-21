#ifndef HomeMenuFrm_H
#define HomeMenuFrm_H

#include "SettingFuncFrms/SettingBaseFrm.h"

//首页配置功能菜单UI
class HomeMenuFrmPrivate;
class HomeMenuFrm : public SettingBaseFrm
{
    Q_OBJECT
public:
    explicit HomeMenuFrm(QWidget *parent = nullptr);
    ~HomeMenuFrm();
public:
    void setHomeDisplay_SnNum(const int &);
    void setHomeDisplay_Mac(const int &);
    void setHomeDisplay_IP(const int &);
    void setHomeDisplay_PersonNum(const int &);
private:
    Q_SLOT void slotManagingPeopleClicked();//人员管理
    Q_SLOT void slotRecordsManagementClicked();//记录管理
    Q_SLOT void slotNetworkSetupClicked();//网络设置
    Q_SLOT void slotSrvSetupClicked();//服务器设置
    Q_SLOT void slotSysSetupClicked();//系统配置
    Q_SLOT void slotIdentifySetupClicked();//识别设置
private:
    QScopedPointer<HomeMenuFrmPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(HomeMenuFrm)
    Q_DISABLE_COPY(HomeMenuFrm)
protected:
#ifdef SCREENCAPTURE  //ScreenCapture     
    void mouseDoubleClickEvent(QMouseEvent*);    
#endif     
};

#endif // HomeMenuFrm_H
