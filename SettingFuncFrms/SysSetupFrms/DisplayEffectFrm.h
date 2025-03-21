#ifndef DisplayEffectFrm_H
#define DisplayEffectFrm_H

#include <QDialog>

//显示效果(目前主界面配置没有启用)
class DisplayEffectFrmPrivate;
class DisplayEffectFrm : public QDialog
{
    Q_OBJECT
public:
    explicit DisplayEffectFrm(QWidget *parent = nullptr);
    ~DisplayEffectFrm();
private:
    QScopedPointer<DisplayEffectFrmPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture     
    void mouseDoubleClickEvent(QMouseEvent*);      
#endif        
private:
    Q_DECLARE_PRIVATE(DisplayEffectFrm)
    Q_DISABLE_COPY(DisplayEffectFrm)
};

#endif // DisplayEffectFrm_H
