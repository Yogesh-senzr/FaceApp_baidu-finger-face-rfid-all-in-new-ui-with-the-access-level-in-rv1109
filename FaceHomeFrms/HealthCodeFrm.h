#ifndef HEALTHCODEFRM_H
#define HEALTHCODEFRM_H

#include <QWidget>

class HealthCodeFrmPrivate;
class HealthCodeFrm : public QWidget
{
    Q_OBJECT
public:
    explicit HealthCodeFrm(QWidget *parent = nullptr);
    ~HealthCodeFrm();
public:
    //显示健康码信息
    void setHealthCodeInfo(const int &type, const QString &name, const QString &idCard, const int &qrCodeType, const QString &tip);
private:
    QScopedPointer<HealthCodeFrmPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(HealthCodeFrm)
    Q_DISABLE_COPY(HealthCodeFrm)
};

#endif // HEALTHCODEFRM_H
