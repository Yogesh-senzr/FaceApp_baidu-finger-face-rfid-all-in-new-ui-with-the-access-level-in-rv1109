#ifndef IDENTIFYSETUPFRM_H
#define IDENTIFYSETUPFRM_H

#include <QWidget>

//识别设置UI
class IdentifySetupFrmPrivate;
class IdentifySetupFrm : public QWidget
{
    Q_OBJECT
public:
    explicit IdentifySetupFrm(QWidget *parent = nullptr);
    ~IdentifySetupFrm();
private:
    QScopedPointer<IdentifySetupFrmPrivate>d_ptr;    
private:
    Q_DECLARE_PRIVATE(IdentifySetupFrm)
    Q_DISABLE_COPY(IdentifySetupFrm)
};

#endif // IDENTIFYSETUPFRM_H
