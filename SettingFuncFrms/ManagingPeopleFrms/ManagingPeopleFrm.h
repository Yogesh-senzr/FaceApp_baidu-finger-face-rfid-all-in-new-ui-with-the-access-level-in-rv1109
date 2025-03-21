#ifndef MANAGINGPEOPLEFRM_H
#define MANAGINGPEOPLEFRM_H

#include "SettingFuncFrms/SettingBaseFrm.h"

//用户管理UI
class QListWidgetItem;
class ManagingPeopleFrmPrivate;
class ManagingPeopleFrm : public SettingBaseFrm
{
    Q_OBJECT
public:
    explicit ManagingPeopleFrm(QWidget *parent = nullptr);
    ~ManagingPeopleFrm();
private:
    Q_SLOT void slotIemClicked(QListWidgetItem *item);
private:
    QScopedPointer<ManagingPeopleFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture     
    void mouseDoubleClickEvent(QMouseEvent*);         
#endif     
private:
    Q_DECLARE_PRIVATE(ManagingPeopleFrm)
    Q_DISABLE_COPY(ManagingPeopleFrm)
};

#endif // MANAGINGPEOPLEFRM_H
