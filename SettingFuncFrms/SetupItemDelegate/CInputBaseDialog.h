#ifndef CInputBaseDialog_H
#define CInputBaseDialog_H

#include <QDialog>

class CInputBaseDialogPrivate;
class CInputBaseDialog : public QDialog
{
    Q_OBJECT
public:
    CInputBaseDialog(QWidget *parent = Q_NULLPTR);
    ~CInputBaseDialog();
public:
    void setTitleText(const QString &);
    void setPlaceholderText(const QString &);
    void setData(const QString &);
    void setIntValidator(const int &min, const int &max);
    void setFloatValidator(const double &min, const double &max);
    void setRegExpValidator(const QString &);
    QString getData()const;
private:
    void paintEvent(QPaintEvent *event);
private:
    QScopedPointer<CInputBaseDialogPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(CInputBaseDialog)
    Q_DISABLE_COPY(CInputBaseDialog)
};

#endif // CInputBaseDialog_H
