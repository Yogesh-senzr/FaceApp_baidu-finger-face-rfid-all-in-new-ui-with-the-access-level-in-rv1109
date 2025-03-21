#ifndef RECORDSMANAGEMENTFRM_H
#define RECORDSMANAGEMENTFRM_H

#include <QWidget>

//记录管理UI
class RecordsManagementFrmPrivate;
class RecordsManagementFrm : public QWidget
{
    Q_OBJECT
public:
    explicit RecordsManagementFrm(QWidget *parent = nullptr);
    ~RecordsManagementFrm();
private:
    QScopedPointer<RecordsManagementFrmPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(RecordsManagementFrm)
    Q_DISABLE_COPY(RecordsManagementFrm)
};

#endif // RECORDSMANAGEMENTFRM_H
