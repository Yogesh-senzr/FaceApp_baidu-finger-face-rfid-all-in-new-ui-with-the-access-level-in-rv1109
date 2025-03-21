#ifndef MANAGINGPEOPLEFRM_H
#define MANAGINGPEOPLEFRM_H

#include <QWidget>

//用户管理UI
class ManagingPeopleFrmPrivate;
class ManagingPeopleFrm : public QWidget
{
    Q_OBJECT
public:
    explicit ManagingPeopleFrm(QWidget *parent = nullptr);
    ~ManagingPeopleFrm();
private:
    QScopedPointer<ManagingPeopleFrmPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(ManagingPeopleFrm)
    Q_DISABLE_COPY(ManagingPeopleFrm)
};

#endif // MANAGINGPEOPLEFRM_H
