#ifndef HOMEFRM_H
#define HOMEFRM_H

#include <QWidget>

//首页功能界面
class HomeFrmPrivate;
class HomeFrm : public QWidget
{
    Q_OBJECT
public:
    explicit HomeFrm(QWidget *parent = nullptr);
    ~HomeFrm();
private:
    QScopedPointer<HomeFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture     
    void mouseDoubleClickEvent(QMouseEvent*);  
#endif           
private:
    Q_DECLARE_PRIVATE(HomeFrm)
    Q_DISABLE_COPY(HomeFrm)
};

#endif // HOMEFRM_H
