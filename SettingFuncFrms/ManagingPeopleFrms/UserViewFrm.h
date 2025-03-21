#ifndef UserViewFrm_H
#define UserViewFrm_H

#include "SettingFuncFrms/SettingBaseFrm.h"



//查看用户/包含导出功能
class UserViewFrmPrivate;
class UserViewFrm : public SettingBaseFrm
{
    Q_OBJECT
public:
    explicit UserViewFrm(QWidget *parent = nullptr);
    ~UserViewFrm();   
public:    
    static inline UserViewFrm *GetInstance(){static UserViewFrm g;return &g;}     
private:
    virtual void setEnter();//进入
private:
    Q_SLOT void slotCurrentPageChanged(const int page);
    Q_SLOT void slotQueryButton();
    Q_SLOT void slotDeleteButton();
    Q_SLOT void slotExportButton();
    Q_SLOT void slotRowDoubleClicked(const QModelIndex &); 
public://显示人脸主界面
    Q_SIGNAL void sigShowFaceHomeFrm(const int index = 0);          
private:
    //处理导出进度(导出进度， 保存状态)
    Q_SLOT void slotExportProgressShell(const bool, const bool, const int total, const int dealcnt);
private:
    Q_SIGNAL void sigExportPersons();//导出人员信息
    int mModifyFlag=0;
public:
    Q_SIGNAL void sigPersonInfo(const QString &,const QString &,const QString &,const QString &);    
    void setModifyFlag( int value);
private:
    QScopedPointer<UserViewFrmPrivate>d_ptr; 
#ifdef SCREENCAPTURE  //ScreenCapture     
    void mouseDoubleClickEvent(QMouseEvent*);         
#endif     
private:
    Q_DECLARE_PRIVATE(UserViewFrm)
    Q_DISABLE_COPY(UserViewFrm)
};

#endif // UserViewFrm_H
