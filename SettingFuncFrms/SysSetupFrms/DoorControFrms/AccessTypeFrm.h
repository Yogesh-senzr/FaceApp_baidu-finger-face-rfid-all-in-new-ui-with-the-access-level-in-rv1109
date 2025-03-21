#ifndef AccessTypeFrm_H
#define AccessTypeFrm_H

#include <QDialog>

//出入类型
class AccessTypeFrmPrivate;
class AccessTypeFrm : public QDialog
{
    Q_OBJECT
public:
    explicit AccessTypeFrm(QWidget *parent = nullptr);
    ~AccessTypeFrm();
public:
    void setAccessType(const int &);
    int getAccessType()const;
private:
    void mousePressEvent(QMouseEvent *event);
private:
    QScopedPointer<AccessTypeFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture     
    void mouseDoubleClickEvent(QMouseEvent*);        
#endif     
private:
    Q_DECLARE_PRIVATE(AccessTypeFrm)
    Q_DISABLE_COPY(AccessTypeFrm)
};

#endif // AccessTypeFrm_H
