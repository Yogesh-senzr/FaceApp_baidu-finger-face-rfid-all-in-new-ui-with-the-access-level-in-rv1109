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
    Q_SLOT void slotPushButton();  // NEW

private:
    //处理导出进度(导出进度， 保存状态)
    Q_SLOT void slotExportProgressShell(const bool, const bool, const int total, const int dealcnt);
    Q_SLOT void slotManualPushProgress(int current, int total, bool success);  // NEW
    Q_SLOT void slotManualPushComplete(bool success, QString message);  // NEW
private:
    Q_SIGNAL void sigExportPersons(const QString);
    Q_SIGNAL void sigManualPushRecords(const QDateTime &startDateTime, const QDateTime &endDateTime);
private:
    QScopedPointer<ViewAccessRecordsFrmPrivate>d_ptr;
    void mouseDoubleClickEvent(QMouseEvent*);      
private:
    Q_DECLARE_PRIVATE(ViewAccessRecordsFrm)
    Q_DISABLE_COPY(ViewAccessRecordsFrm)
};

#endif // VIEWACCESSRECORDSFRM_H
