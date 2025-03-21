#ifndef FACEPNGFRM_H
#define FACEPNGFRM_H

#include <QWidget>

class FacePngFrm : public QWidget
{
    Q_OBJECT
public:
    explicit FacePngFrm(const QString &img_path, QWidget *parent = nullptr);
private:
    void paintEvent(QPaintEvent *event);
#ifdef SCREENCAPTURE  //ScreenCapture       
    void mouseDoubleClickEvent(QMouseEvent*); 
#endif              
private:
    QImage mFaceImg;
};

#endif // FACEPNGFRM_H
