#ifndef WigginsOutputFrm_H
#define WigginsOutputFrm_H

#include <QDialog>

//韦根输出
class WigginsOutputFrmPrivate;
class WigginsOutputFrm : public QDialog
{
    Q_OBJECT
public:
    explicit WigginsOutputFrm(QWidget *parent = nullptr);
    ~WigginsOutputFrm();
public:
    void setWigginsOutputType(const int &);
    int getWigginsOutputType()const;
private:
    void mousePressEvent(QMouseEvent *event);
private:
    QScopedPointer<WigginsOutputFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture     
    void mouseDoubleClickEvent(QMouseEvent*);    
#endif         
private:
    Q_DECLARE_PRIVATE(WigginsOutputFrm)
    Q_DISABLE_COPY(WigginsOutputFrm)
};

#endif // WigginsOutputFrm_H
