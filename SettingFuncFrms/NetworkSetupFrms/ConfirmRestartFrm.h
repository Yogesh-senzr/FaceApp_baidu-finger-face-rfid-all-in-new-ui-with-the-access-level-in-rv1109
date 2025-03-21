#ifndef CONFIRMRESTARTFRM_H
#define CONFIRMRESTARTFRM_H

#include <QDialog>

//切换网络时，弹出此对话框，提示用户是否现在重启
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