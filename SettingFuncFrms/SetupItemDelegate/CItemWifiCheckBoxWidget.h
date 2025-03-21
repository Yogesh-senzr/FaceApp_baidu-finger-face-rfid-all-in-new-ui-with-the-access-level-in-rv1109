#ifndef CItemWifiCheckBoxWidget_H
#define CItemWifiCheckBoxWidget_H

#include <QWidget>

//配置功能显示模板UI
class CItemWifiCheckBoxWidgetPrivate;
class CItemWifiCheckBoxWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CItemWifiCheckBoxWidget(QWidget *parent = nullptr);
    ~CItemWifiCheckBoxWidget();
public:
    void setData(const QString &Name);
    bool getWifiState()const;
public:
    Q_SIGNAL void sigWifiSwitchState(const int);
private:
    QScopedPointer<CItemWifiCheckBoxWidgetPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(CItemWifiCheckBoxWidget)
    Q_DISABLE_COPY(CItemWifiCheckBoxWidget)
};

#endif // CItemWifiCheckBoxWidget_H
