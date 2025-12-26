#ifndef CONFIRMRESTARTFRM_H
#define CONFIRMRESTARTFRM_H

#include <QDialog>

class ConfirmRestartFrmPrivate;
class ConfirmRestartFrm : public QDialog
{
    Q_OBJECT

public:
    explicit ConfirmRestartFrm(QWidget *parent = nullptr);
    ~ConfirmRestartFrm();

private:
    QScopedPointer<ConfirmRestartFrmPrivate> d_ptr;
    Q_DECLARE_PRIVATE(ConfirmRestartFrm)
    Q_DISABLE_COPY(ConfirmRestartFrm)
};

#endif