#include "SettingMenuTitleFrm.h"

#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QPainter>
#include <QStyleOption>
#include <QPalette>

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
    QPalette pal;
    pal.setColor(QPalette::Background, QColor(231,231,231,255));
    this->setAutoFillBackground(true);
    this->setPalette(pal);
}

SettingMenuTitleFrm::~SettingMenuTitleFrm()
{

}

void SettingMenuTitleFrmPrivate::InitUI()
{
    m_pReturnBtn = new QPushButton;//返回
    m_pHintLabel = new QLabel;//提示

    QHBoxLayout *layout = new QHBoxLayout(q_func());
    layout->setMargin(0);
    layout->addSpacing(25);
    layout->addWidget(m_pReturnBtn);
    layout->addStretch();
    layout->addWidget(m_pHintLabel);
    layout->addStretch();
    layout->addSpacing(60);
}

void SettingMenuTitleFrmPrivate::InitData()
{
    m_pReturnBtn->setObjectName("SettingMenuTitleRetBtn");
    m_pHintLabel->setObjectName("SettingMenuTitleHintLabel");
    m_pHintLabel->setText(QObject::tr("Menu"));//菜单
    q_func()->setFixedHeight(90);
}

void SettingMenuTitleFrmPrivate::InitConnect()
{
    QObject::connect(m_pReturnBtn, &QPushButton::clicked, q_func(), &SettingMenuTitleFrm::sigReturnSuperiorClicked);
}

void SettingMenuTitleFrm::setTitleText(const QString &text)
{
    Q_D(SettingMenuTitleFrm);
    d->m_pHintLabel->setText(text);
}

QString SettingMenuTitleFrm::getTitleText()
{
    Q_D(SettingMenuTitleFrm);
    return d->m_pHintLabel->text();
}
#ifdef SCREENCAPTURE  //ScreenCapture    
void SettingMenuTitleFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");   
}
#endif 