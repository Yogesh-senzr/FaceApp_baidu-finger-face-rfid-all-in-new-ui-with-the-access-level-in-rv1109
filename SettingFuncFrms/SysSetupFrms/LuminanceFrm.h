#ifndef LuminanceFrm_H
#define LuminanceFrm_H

#include <QDialog>

//亮度设置
class LuminanceFrmPrivate;
class LuminanceFrm : public QDialog
{
    Q_OBJECT
public:
    explicit LuminanceFrm(QWidget *parent = nullptr);
    ~LuminanceFrm();
public:
    void setAdjustValue(const int &);
    int getAdjustValue() const;
private:
    Q_SLOT void slotValueChanged(int value);
private:
    QScopedPointer<LuminanceFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture     
    void mouseDoubleClickEvent(QMouseEvent*);  
#endif          
private:
    Q_DECLARE_PRIVATE(LuminanceFrm)
    Q_DISABLE_COPY(LuminanceFrm)
};

#endif // LuminanceFrm_H