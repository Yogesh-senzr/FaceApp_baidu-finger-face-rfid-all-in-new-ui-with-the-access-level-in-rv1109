#ifndef CItemDefWidget_H
#define CItemDefWidget_H

#include <QWidget>

//配置功能显示模板UI
class CItemDefWidgetPrivate;
class CItemDefWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CItemDefWidget(QWidget *parent = nullptr);
    ~CItemDefWidget();
public:
    void setData(const QString &Name, const QString &qstrPic);
    QString getNameText()const;
private:
    QScopedPointer<CItemDefWidgetPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(CItemDefWidget)
    Q_DISABLE_COPY(CItemDefWidget)
};

#endif // CItemDefWidget_H
