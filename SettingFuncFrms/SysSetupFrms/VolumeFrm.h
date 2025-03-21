#ifndef VolumeFrm_H
#define VolumeFrm_H

#include <QDialog>

//音量设置
class VolumeFrmPrivate;
class VolumeFrm : public QDialog
{
    Q_OBJECT
public:
    explicit VolumeFrm(QWidget *parent = nullptr);
    ~VolumeFrm();
public:
    void setAdjustValue(const int &);
    int getAdjustValue() const;
private:
    Q_SLOT void slotValueChanged(int value);
    Q_SLOT void slotSliderReleased();
private:
    bool event(QEvent *event);
private:
    QScopedPointer<VolumeFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture     
    void mouseDoubleClickEvent(QMouseEvent*); 
#endif         
private:
    Q_DECLARE_PRIVATE(VolumeFrm)
    Q_DISABLE_COPY(VolumeFrm)
};

#endif // VolumeFrm_H
