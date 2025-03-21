#ifndef FillLightFrm_H
#define FillLightFrm_H

#include <QDialog>

//补光设置
class FillLightFrmPrivate;
class FillLightFrm : public QDialog
{
    Q_OBJECT
public:
    explicit FillLightFrm(QWidget *parent = nullptr);
    ~FillLightFrm();
public://设置选中补光模式
    void setFillLightMode(const int &);
    int getFillLightMode()const;
private:
    void mousePressEvent(QMouseEvent *event);
private:
    QScopedPointer<FillLightFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture     
    void mouseDoubleClickEvent(QMouseEvent*); 
#endif           
private:
    Q_DECLARE_PRIVATE(FillLightFrm)
    Q_DISABLE_COPY(FillLightFrm)
};

#endif // FillLightFrm_H
