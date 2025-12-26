#ifndef USERVIEWFRM_H
#define USERVIEWFRM_H

#include <QWidget>
#include "SettingFuncFrms/SettingBaseFrm.h"
#include <QHash>
#include <QStringList>

class UserViewFrmPrivate;

class UserViewFrm : public SettingBaseFrm
{
    Q_OBJECT
public:
    explicit UserViewFrm(QWidget *parent = nullptr);
    ~UserViewFrm();   
public:    
    static inline UserViewFrm *GetInstance() { static UserViewFrm g; return &g; }     
private:
    virtual void setEnter();
private:
    Q_SLOT void slotCurrentPageChanged(const int page);
    Q_SLOT void slotUserItemClicked(int row);
    Q_SLOT void slotBackToListView();
    Q_SLOT void slotFilterActionTriggered();
    Q_SLOT void slotExportButton();
    Q_SLOT void slotRefreshUserData();
    // Remove: Q_SLOT void slotQueryButton(); // Removed to avoid duplicate declaration
public:
    Q_SIGNAL void sigShowFaceHomeFrm(const int index = 0);
    Q_SIGNAL void sigShowFrm(const QString &name);
private:
    Q_SLOT void slotExportProgressShell(const bool, const bool, const int total, const int dealcnt);
private:
    Q_SIGNAL void sigExportPersons();
    int mModifyFlag = 0;
public:
    Q_SIGNAL void sigPersonInfo(const QString &, const QString &, const QString &, const QString &);    
    void setModifyFlag(int value);
    //void UpdateTotalCountLabel();

public slots:
    void slotAddFaceButton();
    void slotEditFaceButton();
    void slotUpdateUserList();
    void slotQueryButton(); // Keep this declaration
private:
    void refreshCurrentUserDetails();
    void onUserSyncCompleted(const QString& employeeId, bool success);
private:
    QScopedPointer<UserViewFrmPrivate> d_ptr; 
#ifdef SCREENCAPTURE
    void mouseDoubleClickEvent(QMouseEvent*);
#endif     
private:
    Q_DECLARE_PRIVATE(UserViewFrm)
    Q_DISABLE_COPY(UserViewFrm)
};

#endif // USERVIEWFRM_H