#ifndef CHANGEDATETIMEFRM_H
#define CHANGEDATETIMEFRM_H

#include <QDialog>
//更改时间和日期
class ChangeDateTimeFrmPrivate;
class ChangeDateTimeFrm : public QDialog
{
    Q_OBJECT
public:
    ChangeDateTimeFrm(QWidget *parent = Q_NULLPTR);
    ~ChangeDateTimeFrm();
private:
    QScopedPointer<ChangeDateTimeFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture     
    void mouseDoubleClickEvent(QMouseEvent*);    
#endif       
private:
    Q_DECLARE_PRIVATE(ChangeDateTimeFrm)
    Q_DISABLE_COPY(ChangeDateTimeFrm)
};

#endif // CHANGEDATETIMEFRM_H
