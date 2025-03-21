#ifndef CItemWidget_H
#define CItemWidget_H

#include <QWidget>

//配置功能显示模板UI
class CItemWidgetPrivate;
class CItemWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CItemWidget(QWidget *parent = nullptr);
    ~CItemWidget();
public:
    void setData(const QString &Name, const QString &qstrPic = QString(), const QString &text = QString());
    //不显示右边图片
    void setHideRPng();
    //设置右边文字提示
    void setRNameText(const QString &);
    QString getNameText()const;
    QString getRNameText()const;

    void setAddSpacing(const int spcaing = 10);
private:
    QScopedPointer<CItemWidgetPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(CItemWidget)
    Q_DISABLE_COPY(CItemWidget)
};

#endif // CItemWidget_H
