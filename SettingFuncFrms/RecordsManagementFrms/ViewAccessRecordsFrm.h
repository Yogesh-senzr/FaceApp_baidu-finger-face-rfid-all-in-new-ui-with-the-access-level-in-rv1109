#ifndef VIEWACCESSRECORDSFRM_H
#define VIEWACCESSRECORDSFRM_H

#include "SettingFuncFrms/SettingBaseFrm.h"

//查看通行记录/包含导出
class ViewAccessRecordsFrmPrivate;
class ViewAccessRecordsFrm : public SettingBaseFrm
{
    Q_OBJECT
public:
    explicit ViewAccessRecordsFrm(QWidget *parent = nullptr);
    ~ViewAccessRecordsFrm();
private:
    virtual void setEnter();//进入
private:
    Q_SLOT void slotCurrentPageChanged(const int page);
    Q_SLOT void slotQueryButton();
    Q_SLOT void slotExportButton();
    Q_SLOT void slotCleanButton();    
private:
    //处理导出进度(导出进度， 保存状态)
    Q_SLOT void slotExportProgressShell(const bool, const bool, const int total, const int dealcnt);
private:
    Q_SIGNAL void sigExportPersons(const QString);
private:
    QScopedPointer<ViewAccessRecordsFrmPrivate>d_ptr;
    void mouseDoubleClickEvent(QMouseEvent*);      
private:
    Q_DECLARE_PRIVATE(ViewAccessRecordsFrm)
    Q_DISABLE_COPY(ViewAccessRecordsFrm)
};

#endif // VIEWACCESSRECORDSFRM_H
