#ifndef CITEMBOXWIDGET_H
#define CITEMBOXWIDGET_H

#include <QWidget>

class CItemBoxWidgetPrivate;
class CItemBoxWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CItemBoxWidget(QWidget *parent = nullptr);
    ~CItemBoxWidget();
public:
    Q_SIGNAL void sigSwitchState(const int);
    void setCheckBoxState(const int &);
    int getCheckBoxState()const;
    void setData(const QString &Name, const QString &qstrPic = QString());
    QString getNameText()const;
    void setAddSpacing(const int spcaing = 10);
private:
    void mousePressEvent(QMouseEvent *event);
private:
    QScopedPointer<CItemBoxWidgetPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(CItemBoxWidget)
    Q_DISABLE_COPY(CItemBoxWidget)
};

#endif // CITEMBOXWIDGET_H
