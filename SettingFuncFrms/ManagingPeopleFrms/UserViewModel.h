#ifndef UserViewModel_H
#define UserViewModel_H

#include <QWidget>
//通行记录模型
class UserViewModelPrivate;
class UserViewModel : public QWidget
{
    Q_OBJECT
public:
    explicit UserViewModel(QWidget *parent = nullptr);
    ~UserViewModel();
public:
    void setData(const QString &name, const QString &sex, const QString &icCard, const QString &idCard, const QString &addtime);
    QString getName()const;
    QString getCreateTime()const;
public:
    Q_SIGNAL void sigDeleteButton();
private:
    QScopedPointer<UserViewModelPrivate>d_ptr;
#ifdef SCREENCAPTURE  //ScreenCapture    
    void mouseDoubleClickEvent(QMouseEvent*);     
#endif     
private:
    Q_DECLARE_PRIVATE(UserViewModel)
    Q_DISABLE_COPY(UserViewModel)
};

#endif // UserViewModel_H
