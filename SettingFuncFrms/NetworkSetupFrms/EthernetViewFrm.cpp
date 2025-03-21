#include "EthernetViewFrm.h"

#include "RkNetWork/NetworkControlThread.h"

#include "Config/ReadConfig.h"
#include "Helper/myhelper.h"
#include "MessageHandler/Log.h"

#include <QtWidgets/QLineEdit>
#include <QtWidgets/QLabel>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QButtonGroup>
#include <QtGui/QRegExpValidator>

#include <QtWidgets/QHBoxLayout>

class EthernetViewFrmPrivate
{
    Q_DECLARE_PUBLIC(EthernetViewFrm)
public:
    EthernetViewFrmPrivate(EthernetViewFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QPushButton *m_pSaveButton;//保存
    QCheckBox *m_pLanSwitchBox;//以太网开/关
    QButtonGroup *m_pLanTypeGroup;//以太网IP模式
    QLineEdit *m_pLanIPEdit;
    QLineEdit *m_pLanMakeEdit;
    QLineEdit *m_pLanGatewayEdit;
    QLineEdit *m_pLanDnsEdit;
private:
    EthernetViewFrm *const q_ptr;
};

EthernetViewFrmPrivate::EthernetViewFrmPrivate(EthernetViewFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

EthernetViewFrm::EthernetViewFrm(QWidget *parent)
    : SettingBaseFrm(parent)
    , d_ptr(new EthernetViewFrmPrivate(this))
{

}

EthernetViewFrm::~EthernetViewFrm()
{

}

void EthernetViewFrmPrivate::InitUI()
{
    m_pLanSwitchBox = new QCheckBox;//以太网开/关
    m_pLanTypeGroup = new QButtonGroup;//以太网IP模式
    m_pLanIPEdit = new QLineEdit;
    m_pLanMakeEdit = new QLineEdit;
    m_pLanGatewayEdit = new QLineEdit;
    m_pLanDnsEdit = new QLineEdit;
    m_pSaveButton = new QPushButton;//保存

    m_pLanIPEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_pLanMakeEdit->setContextMenuPolicy(Qt::NoContextMenu);    
	m_pLanGatewayEdit->setContextMenuPolicy(Qt::NoContextMenu);
	m_pLanDnsEdit->setContextMenuPolicy(Qt::NoContextMenu);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addStretch();
    layout->addWidget(new QLabel(QObject::tr("Ethernet")));//"以太网")
    layout->addStretch();
    layout->addWidget(m_pLanSwitchBox);
    layout->addStretch();

    m_pLanTypeGroup->addButton(new QRadioButton(QObject::tr("EthernetDHCPIP")), 0);//"以太网动态IP"
    m_pLanTypeGroup->addButton(new QRadioButton(QObject::tr("EthernetStaticIP")), 1);//"以太网静态IP"

    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addStretch();
    layout1->addWidget(m_pLanTypeGroup->button(0));
    layout1->addStretch();
    layout1->addWidget(m_pLanTypeGroup->button(1));
    layout1->addStretch();

    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addWidget(new QLabel(QObject::tr("IPAddress")));//"IP地址"
    vlayout->addWidget(new QLabel(QObject::tr("subnetmask")));//"子网掩码"
    vlayout->addWidget(new QLabel(QObject::tr("gateway")));//"网关"
    vlayout->addWidget(new QLabel("DNS"));
    ((QLabel *)vlayout->itemAt(0)->widget())->setAlignment(Qt::AlignRight | Qt::AlignCenter);
    ((QLabel *)vlayout->itemAt(1)->widget())->setAlignment(Qt::AlignRight | Qt::AlignCenter);
    ((QLabel *)vlayout->itemAt(2)->widget())->setAlignment(Qt::AlignRight | Qt::AlignCenter);
    ((QLabel *)vlayout->itemAt(3)->widget())->setAlignment(Qt::AlignRight | Qt::AlignCenter);

    QVBoxLayout *vlayout1 = new QVBoxLayout;
    vlayout1->addWidget(m_pLanIPEdit);
    vlayout1->addWidget(m_pLanMakeEdit);
    vlayout1->addWidget(m_pLanGatewayEdit);
    vlayout1->addWidget(m_pLanDnsEdit);


    QHBoxLayout *layout2 = new QHBoxLayout;
    layout2->addLayout(vlayout);
    layout2->addLayout(vlayout1);

    QHBoxLayout *layout3 = new QHBoxLayout;
    layout3->addStretch();
    layout3->addWidget(m_pSaveButton);
    layout3->addStretch();

    QVBoxLayout *malayout = new QVBoxLayout(q_func());
    malayout->setContentsMargins(20, 0, 20, 20);
    malayout->addLayout(layout);
    malayout->addSpacing(20);
    malayout->addLayout(layout1);
    malayout->addSpacing(20);
    malayout->addLayout(layout2);
    malayout->addSpacing(10);
    malayout->addLayout(layout3);
    malayout->addStretch();
}

void EthernetViewFrmPrivate::InitData()
{
    m_pLanTypeGroup->button(0)->setChecked(true);
    m_pLanIPEdit->setEnabled(false);
    m_pLanMakeEdit->setEnabled(false);
    m_pLanGatewayEdit->setEnabled(false);
    m_pLanDnsEdit->setEnabled(false);

    m_pLanIPEdit->setText("192.168.8.123");
    m_pLanMakeEdit->setText("255.255.255.0");
    m_pLanGatewayEdit->setText("192.168.8.1");
    m_pLanDnsEdit->setText("8.8.8.8");

    QRegExp netRegMask = QRegExp("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");
    m_pLanIPEdit->setValidator(new QRegExpValidator(netRegMask));
    m_pLanMakeEdit->setValidator(new QRegExpValidator(netRegMask));
    m_pLanGatewayEdit->setValidator(new QRegExpValidator(netRegMask));
    m_pLanDnsEdit->setValidator(new QRegExpValidator(netRegMask));

    m_pLanSwitchBox->setObjectName("LanSwitchBox");
    m_pSaveButton->setFixedSize(282, 62);
    //m_pSaveButton->setText(QObject::tr("Save"));//"保存"
    m_pSaveButton->setText(QObject::tr("SaveAndReboot"));//"保存并重启"
}

void EthernetViewFrmPrivate::InitConnect()
{
    QObject::connect(m_pSaveButton, &QPushButton::clicked, q_func(), &EthernetViewFrm::slotSaveButtonClicked);
    QObject::connect(m_pLanSwitchBox, &QCheckBox::stateChanged, q_func(), &EthernetViewFrm::slotLanSwitchState);

    QObject::connect(m_pLanTypeGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), [=](int index) {
        m_pLanIPEdit->setEnabled(index);
        m_pLanMakeEdit->setEnabled(index);
        m_pLanGatewayEdit->setEnabled(index);
        m_pLanDnsEdit->setEnabled(index);
    });
}

void EthernetViewFrm::setEnter()
{
    Q_D(EthernetViewFrm);
    int mode = ReadConfig::GetInstance()->getNetwork_Manager_Mode();
    d->m_pLanSwitchBox->setChecked((mode == 1) ? true : false);

    d->m_pLanIPEdit->setText(ReadConfig::GetInstance()->getLAN_IP());
    d->m_pLanMakeEdit->setText(ReadConfig::GetInstance()->getLAN_Maks());
    d->m_pLanGatewayEdit->setText(ReadConfig::GetInstance()->getLAN_Gateway());
    d->m_pLanDnsEdit->setText(ReadConfig::GetInstance()->getLAN_DNS());
    if (ReadConfig::GetInstance()->getLan_DHCP())
    {
       d->m_pLanTypeGroup->button(0)->setChecked(true);
       d->m_pLanTypeGroup->buttonClicked(false);
    }
    else 
    {
       d->m_pLanTypeGroup->button(1)->setChecked(true);
       d->m_pLanTypeGroup->buttonClicked(true);
    }
}

void EthernetViewFrm::slotSaveButtonClicked()
{
    Q_D(EthernetViewFrm);
    NetworkControlThread::GetInstance()->setLinkLan(d->m_pLanTypeGroup->checkedId(),
                                                d->m_pLanIPEdit->text(),
                                                d->m_pLanMakeEdit->text(),
                                                d->m_pLanGatewayEdit->text(),
                                                d->m_pLanDnsEdit->text());

    ReadConfig::GetInstance()->setLAN_IP( d->m_pLanIPEdit->text());
    ReadConfig::GetInstance()->setLAN_Maks(d->m_pLanMakeEdit->text());
    ReadConfig::GetInstance()->setLAN_Gateway(d->m_pLanGatewayEdit->text());
    ReadConfig::GetInstance()->setLAN_DNS(d->m_pLanDnsEdit->text());
    ReadConfig::GetInstance()->setLAN_DHCP(!d->m_pLanTypeGroup->checkedId());
    LogD("%s,%s,%d,checkState=%d\n",__FILE__,__func__,__LINE__,d->m_pLanSwitchBox->checkState());
    ReadConfig::GetInstance()->setNetwork_Manager_Mode(d->m_pLanSwitchBox->checkState() ? 1 : 2);
    LogD("%s,%s,%d,getNetwork_Manager_Mode=%d\n",__FILE__,__func__,__LINE__,ReadConfig::GetInstance()->getNetwork_Manager_Mode());
	ReadConfig::GetInstance()->setSaveConfig();
    LogD("%s,%s,%d,getNetwork_Manager_Mode=%d\n",__FILE__,__func__,__LINE__,ReadConfig::GetInstance()->getNetwork_Manager_Mode());


	myHelper::Utils_Reboot();
}

void EthernetViewFrm::slotLanSwitchState(const int state)
{
    Q_UNUSED(state);
#ifdef Q_OS_LINUX
        NetworkControlThread::GetInstance()->setNetworkType(state ? 1 : 2);
        ReadConfig::GetInstance()->setNetwork_Manager_Mode(state ? 1 : 2);
#endif
}
#ifdef SCREENCAPTURE  //ScreenCapture 
void EthernetViewFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif 