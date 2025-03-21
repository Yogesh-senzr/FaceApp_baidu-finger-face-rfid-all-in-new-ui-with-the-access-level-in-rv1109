#ifndef CItemWifiWidget_H
#define CItemWifiWidget_H

#include <QWidget>

//配置功能显示模板UI
class CItemWifiWidgetPrivate;
class CItemWifiWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CItemWifiWidget(QWidget *parent = nullptr);
    ~CItemWifiWidget();
public:
    void setData(const QString &Service, const QString &ssid, const float& = 0);
    QString getSSIDText()const;
    QString getServiceText()const;
    void setAddSpacing(const int spcaing = 10);
private:
    QScopedPointer<CItemWifiWidgetPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(CItemWifiWidget)
    Q_DISABLE_COPY(CItemWifiWidget)
};

#endif // CItemWifiWidget_H
