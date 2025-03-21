#ifndef IdentifyDistanceFrm_H
#define IdentifyDistanceFrm_H

#include <QDialog>

//语言设置
class IdentifyDistanceFrmPrivate;
class IdentifyDistanceFrm : public QDialog
{
    Q_OBJECT
public:
    explicit IdentifyDistanceFrm(QWidget *parent = nullptr);
    ~IdentifyDistanceFrm();
public://设置选中语言
    void setDistanceMode(const int &);
    int getDistanceMode()const;
private:
    void mousePressEvent(QMouseEvent *event);
private:
    QScopedPointer<IdentifyDistanceFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture       
    void mouseDoubleClickEvent(QMouseEvent*);        
#endif    
private:
    Q_DECLARE_PRIVATE(IdentifyDistanceFrm)
    Q_DISABLE_COPY(IdentifyDistanceFrm)
};

#endif // IdentifyDistanceFrm_H
