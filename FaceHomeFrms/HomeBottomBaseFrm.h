#ifndef HOMEBOTTOMBASEFRM_H
#define HOMEBOTTOMBASEFRM_H

#include <QWidget>

class HomeBottomBaseFrm : public QWidget
{
    Q_OBJECT
public:
    explicit HomeBottomBaseFrm(QWidget *parent = nullptr);
public:
    virtual void setHomeDisplay_SnNum(const int &){}
    virtual void setHomeDisplay_Mac(const int &){}
    virtual void setHomeDisplay_IP(const int &){}
    virtual void setHomeDisplay_PersonNum(const int &){}
public:
    virtual void setPersonNum(const int &){}
    virtual void setNetInfo(const QString &, const QString &){}
    virtual void setSNNUm(const QString &){}
};

#endif // HOMEBOTTOMBASEFRM_H
