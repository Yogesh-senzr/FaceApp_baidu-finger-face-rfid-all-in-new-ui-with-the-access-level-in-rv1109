#include "DevicePoweroffFrm.h"
#include "Helper/myhelper.h"

#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>

class DevicePoweroffFrmPrivate
{
    Q_DECLARE_PUBLIC(DevicePoweroffFrm)
public:
    DevicePoweroffFrmPrivate(DevicePoweroffFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    //QPushButton *m_pShutdownBtn;//关机
    QPushButton *m_pRebootBtn;//重启
    QPushButton *m_pCancelBtn;//取消
    QLabel *m_pHintLabel;
private:
    DevicePoweroffFrm *const q_ptr;
};

DevicePoweroffFrmPrivate::DevicePoweroffFrmPrivate(DevicePoweroffFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

DevicePoweroffFrm::DevicePoweroffFrm(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , d_ptr(new DevicePoweroffFrmPrivate(this))
{

}

DevicePoweroffFrm::~DevicePoweroffFrm()
{

}

void DevicePoweroffFrmPrivate::InitUI()
{
    //m_pShutdownBtn = new QPushButton(QObject::tr("PowerOff"));//关机
    m_pRebootBtn = new QPushButton(QObject::tr("Reboot"));//重启
    m_pCancelBtn = new QPushButton(QObject::tr("Cancel"));//取消
    m_pHintLabel = new QLabel;

    QHBoxLayout *hlayout = new QHBoxLayout;
    //hlayout->addWidget(m_pShutdownBtn);
    hlayout->addWidget(m_pRebootBtn);
    hlayout->addWidget(m_pCancelBtn);

    QVBoxLayout *vlayout = new QVBoxLayout(q_func());
    vlayout->setMargin(10);
    vlayout->addStretch();
    vlayout->addLayout(hlayout);
    vlayout->addWidget(m_pHintLabel);
    vlayout->addStretch();
}

void DevicePoweroffFrmPrivate::InitData()
{
    q_func()->setObjectName("DevicePoweroffFrm");

    m_pHintLabel->setObjectName("DevicePoweroffFrmLabel");
    m_pHintLabel->hide();

    //m_pShutdownBtn->setObjectName("DevicePoweroffButton");
    m_pRebootBtn->setObjectName("DevicePoweroffButton");
    m_pCancelBtn->setObjectName("DevicePoweroffButton");
}

void DevicePoweroffFrmPrivate::InitConnect()
{
#if 0    
    QObject::connect(m_pShutdownBtn, &QPushButton::clicked, [&] {
        m_pHintLabel->show();
        m_pHintLabel->setText(QObject::tr("PowerOffing"));//关机中...
        myHelper::Utils_Poweroff();
    });
#endif     
    QObject::connect(m_pRebootBtn, &QPushButton::clicked, [&] {
        m_pHintLabel->show();
        m_pHintLabel->setText(QObject::tr("Rebooting"));//重启中...
        myHelper::Utils_Reboot();
    });
    QObject::connect(m_pCancelBtn, &QPushButton::clicked, [&] {
       q_func()->done(0);
    });
}
#ifdef SCREENCAPTURE  //ScreenCapture 
void DevicePoweroffFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif