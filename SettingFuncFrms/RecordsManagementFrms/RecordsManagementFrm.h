#ifndef RECORDSMANAGEMENTFRM_H
#define RECORDSMANAGEMENTFRM_H

#include "SettingFuncFrms/SettingBaseFrm.h"

//记录管理UI
class QListWidgetItem;
class RecordsManagementFrmPrivate;
class RecordsManagementFrm : public SettingBaseFrm
{
    Q_OBJECT
public:
    explicit RecordsManagementFrm(QWidget *parent = nullptr);
    ~RecordsManagementFrm();
private:
    virtual void setLeaveEvent();//退出
private:
    Q_SLOT void slotIemClicked(QListWidgetItem *item);
private:
    QScopedPointer<RecordsManagementFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture   
    void mouseDoubleClickEvent(QMouseEvent*);  
#endif         
private:
    Q_DECLARE_PRIVATE(RecordsManagementFrm)
    Q_DISABLE_COPY(RecordsManagementFrm)
};

#endif // RECORDSMANAGEMENTFRM_H
