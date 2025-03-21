#ifndef CItemWifiBoxWidget_H
#define CItemWifiBoxWidget_H

#include <QWidget>

//配置功能显示模板UI
class CItemWifiBoxWidgetPrivate;
class CItemWifiBoxWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CItemWifiBoxWidget(QWidget *parent = nullptr);
    ~CItemWifiBoxWidget();
public:
    void setData(const QString &Name);
    bool getWifiState()const;
    void setWifiState(const bool &);
    void setAddSpacing(const int spcaing = 10);
public:
    Q_SIGNAL void sigWifiSwitchState(const int);
    Q_SIGNAL void sigRestartTakeEffect();
private:
    QScopedPointer<CItemWifiBoxWidgetPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(CItemWifiBoxWidget)
    Q_DISABLE_COPY(CItemWifiBoxWidget)
};

#endif // CItemWifiBoxWidget_H
