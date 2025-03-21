#ifndef SETTINGMENUTITLEFRM_H
#define SETTINGMENUTITLEFRM_H

#include <QWidget>

class SettingMenuTitleFrmPrivate;
class SettingMenuTitleFrm : public QWidget
{
    Q_OBJECT
public:
    explicit SettingMenuTitleFrm(QWidget *parent = nullptr);
    ~SettingMenuTitleFrm();
private:
    QScopedPointer<SettingMenuTitleFrmPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(SettingMenuTitleFrm)
    Q_DISABLE_COPY(SettingMenuTitleFrm)    
};

#endif // SETTINGMENUTITLEFRM_H
