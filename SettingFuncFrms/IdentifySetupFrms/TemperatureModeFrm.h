#ifndef TemperatureModeFrm_H
#define TemperatureModeFrm_H

#include <QDialog>

//测温环境
class TemperatureModeFrmPrivate;
class TemperatureModeFrm : public QDialog
{
    Q_OBJECT
public:
    explicit TemperatureModeFrm(QWidget *parent = nullptr);
    ~TemperatureModeFrm();
public://设置选中测温环境
    void setTemperatureMode(const int &);
    int getTemperatureMode()const;
private:
    void mousePressEvent(QMouseEvent *event);
private:
    QScopedPointer<TemperatureModeFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture       
    void mouseDoubleClickEvent(QMouseEvent*);      
#endif    
private:
    Q_DECLARE_PRIVATE(TemperatureModeFrm)
    Q_DISABLE_COPY(TemperatureModeFrm)
};

#endif // TemperatureModeFrm_H
