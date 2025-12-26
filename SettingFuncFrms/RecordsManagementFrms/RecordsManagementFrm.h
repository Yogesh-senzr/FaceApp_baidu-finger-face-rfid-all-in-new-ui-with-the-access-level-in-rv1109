// Updated RecordsManagementFrm.h - Card-based theme support

#ifndef RECORDSMANAGEMENTFRM_H
#define RECORDSMANAGEMENTFRM_H

#include "SettingFuncFrms/SettingBaseFrm.h"

class QListWidgetItem;
class RecordsManagementFrmPrivate;

class RecordsManagementFrm : public SettingBaseFrm
{
    Q_OBJECT
public:
    explicit RecordsManagementFrm(QWidget *parent = nullptr);
    ~RecordsManagementFrm();
    
    // Method to handle card clicks
    void handleCardClicked(const QString &title);

private:
    virtual void setLeaveEvent();
    
private slots:
    void slotIemClicked(QListWidgetItem *item); // Keep for compatibility

signals:
    void sigShowFrm(const QString &frmName);

private:
    QScopedPointer<RecordsManagementFrmPrivate> d_ptr;
    
#ifdef SCREENCAPTURE
    void mouseDoubleClickEvent(QMouseEvent*);        
#endif    

private:
    Q_DECLARE_PRIVATE(RecordsManagementFrm)
    Q_DISABLE_COPY(RecordsManagementFrm)
};

#endif // RECORDSMANAGEMENTFRM_H