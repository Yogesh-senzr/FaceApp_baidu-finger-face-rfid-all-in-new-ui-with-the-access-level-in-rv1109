#include "SettingMenuTitleFrm.h"

#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>

class SettingMenuTitleFrmPrivate
{
    Q_DECLARE_PUBLIC(SettingMenuTitleFrm)
public:
    SettingMenuTitleFrmPrivate(SettingMenuTitleFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QPushButton *m_pReturnBtn;//返回
    QLabel *m_pHintLabel;//提示
private:
    SettingMenuTitleFrm *const q_ptr;
};


SettingMenuTitleFrmPrivate::SettingMenuTitleFrmPrivate(SettingMenuTitleFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

SettingMenuTitleFrm::SettingMenuTitleFrm(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new SettingMenuTitleFrmPrivate(this))
{

}

SettingMenuTitleFrm::~SettingMenuTitleFrm()
{

}

void SettingMenuTitleFrmPrivate::InitUI()
{
    m_pReturnBtn = new QPushButton;//返回
    m_pHintLabel = new QLabel;//提示

    QHBoxLayout *layout = new QHBoxLayout(q_func());
    layout->addWidget(m_pReturnBtn);
    layout->addStretch();
    layout->addWidget(m_pHintLabel);
    layout->addStretch();
}

void SettingMenuTitleFrmPrivate::InitData()
{
    m_pReturnBtn->setObjectName("SettingMenuTitleRetBtn");
    m_pHintLabel->setObjectName("SettingMenuTitleHintLabel");

    //m_pReturnBtn->setText("返回");
    m_pHintLabel->setText("菜单");
}

void SettingMenuTitleFrmPrivate::InitConnect()
{

}
