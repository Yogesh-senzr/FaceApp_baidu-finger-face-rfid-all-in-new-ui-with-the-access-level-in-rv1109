#ifndef FACEHOMEBOTTOMFRM_H
#define FACEHOMEBOTTOMFRM_H

#include "HomeBottomBaseFrm.h"

class FaceHomeBottomFrmPrivate;
class FaceHomeBottomFrm : public HomeBottomBaseFrm
{
    Q_OBJECT
public:
    explicit FaceHomeBottomFrm(QWidget *parent = nullptr);
    ~FaceHomeBottomFrm();
private:
    void setHomeDisplay_SnNum(const int &);
    void setHomeDisplay_Mac(const int &);
    void setHomeDisplay_IP(const int &);
    void setHomeDisplay_PersonNum(const int &);
    void setHomeDisplay_Essl(const int &);
private:
    void setPersonNum(const int &);
    void setNetInfo(const QString &, const QString &);
    void setSNNUm(const QString &);
private:
    void paintEvent(QPaintEvent *event);
private:
    QScopedPointer<FaceHomeBottomFrmPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(FaceHomeBottomFrm)
    Q_DISABLE_COPY(FaceHomeBottomFrm)
};

#endif // FACEHOMEBOTTOMFRM_H
