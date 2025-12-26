// Updated ManagingPeopleFrm.h - Card-based theme support

#ifndef MANAGINGPEOPLEFRM_H
#define MANAGINGPEOPLEFRM_H

#include "SettingFuncFrms/SettingBaseFrm.h"

class QListWidgetItem;
class ManagingPeopleFrmPrivate;

class ManagingPeopleFrm : public SettingBaseFrm
{
    Q_OBJECT
public:
    explicit ManagingPeopleFrm(QWidget *parent = nullptr);
    ~ManagingPeopleFrm();
    
    // Method to handle card clicks
    void handleCardClicked(const QString &title);

private slots:
    void slotIemClicked(QListWidgetItem *item); // Keep for compatibility

signals:
    void sigShowFrm(const QString &frmName);

private:
    QScopedPointer<ManagingPeopleFrmPrivate> d_ptr;
    
#ifdef SCREENCAPTURE
    void mouseDoubleClickEvent(QMouseEvent*);        
#endif    

private:
    Q_DECLARE_PRIVATE(ManagingPeopleFrm)
    Q_DISABLE_COPY(ManagingPeopleFrm)
};

#endif // MANAGINGPEOPLEFRM_H