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
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QFrame>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QStyle>
#include <QApplication>

// Custom Ethernet Card Widget
class EthernetCardWidget : public QFrame
{
    Q_OBJECT
public:
    enum CardType { 
        ToggleCard,           // Ethernet On/Off
        CombinedConfigCard    // Combined IP config and parameters
    };
    
    EthernetCardWidget(const QString &title, CardType type, QWidget *parent = nullptr)
        : QFrame(parent), m_title(title), m_cardType(type)
    {
        setObjectName("EthernetCardWidget");
        setupUI(title, type);
    }
    
    // Getters and setters for toggle card
    void setToggleState(bool enabled)
    {
        if (m_cardType == ToggleCard && m_toggleCheckBox) {
            m_toggleCheckBox->setChecked(enabled);
        }
    }
    
    bool getToggleState() const
    {
        if (m_cardType == ToggleCard && m_toggleCheckBox) {
            return m_toggleCheckBox->isChecked();
        }
        return false;
    }
    
    // Getters and setters for combined config card
    void setDHCPMode(bool isDHCP)
    {
        if (m_cardType == CombinedConfigCard && m_buttonGroup) {
            m_buttonGroup->button(isDHCP ? 0 : 1)->setChecked(true);
            updateParameterFields(!isDHCP);
        }
    }
    
    bool isDHCPMode() const
    {
        if (m_cardType == CombinedConfigCard && m_buttonGroup) {
            return m_buttonGroup->checkedId() == 0;
        }
        return true;
    }
    
    // Parameter setters/getters
    void setIPAddress(const QString &ip) { if (m_ipEdit) m_ipEdit->setText(ip); }
    void setSubnetMask(const QString &mask) { if (m_maskEdit) m_maskEdit->setText(mask); }
    void setGateway(const QString &gateway) { if (m_gatewayEdit) m_gatewayEdit->setText(gateway); }
    void setDNS(const QString &dns) { if (m_dnsEdit) m_dnsEdit->setText(dns); }
    
    QString getIPAddress() const { return m_ipEdit ? m_ipEdit->text() : ""; }
    QString getSubnetMask() const { return m_maskEdit ? m_maskEdit->text() : ""; }
    QString getGateway() const { return m_gatewayEdit ? m_gatewayEdit->text() : ""; }
    QString getDNS() const { return m_dnsEdit ? m_dnsEdit->text() : ""; }

signals:
    void toggleChanged(bool enabled);
    void configModeChanged(bool isDHCP);

protected:
    void enterEvent(QEvent *event) override
    {
        setProperty("hovered", true);
        style()->unpolish(this);
        style()->polish(this);
        QFrame::enterEvent(event);
    }
    
    void leaveEvent(QEvent *event) override
    {
        setProperty("hovered", false);
        style()->unpolish(this);
        style()->polish(this);
        QFrame::leaveEvent(event);
    }

private:
    void setupUI(const QString &title, CardType type)
    {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(20, 16, 20, 16);
        mainLayout->setSpacing(15);
        
        if (type == ToggleCard) {
            setupToggleCard(title, mainLayout);
        } else if (type == CombinedConfigCard) {
            setupCombinedConfigCard(title, mainLayout);
        }
    }
    
    void setupToggleCard(const QString &title, QVBoxLayout *layout)
    {
        QHBoxLayout *headerLayout = new QHBoxLayout;
        headerLayout->setSpacing(15);
        
        // Icon frame
        QFrame *iconFrame = new QFrame;
        iconFrame->setFixedSize(40, 40);
        iconFrame->setObjectName("EthernetIconFrame");
        
        QHBoxLayout *iconLayout = new QHBoxLayout(iconFrame);
        iconLayout->setContentsMargins(0, 0, 0, 0);
        QLabel *iconLabel = new QLabel("🌐");
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setObjectName("EthernetIconLabel");
        iconLayout->addWidget(iconLabel);
        
        // Title and description
        QVBoxLayout *textLayout = new QVBoxLayout;
        textLayout->setSpacing(2);
        
        QLabel *titleLabel = new QLabel(title);
        titleLabel->setObjectName("EthernetTitleLabel");
        
        QLabel *descLabel = new QLabel(tr("Enable or disable Ethernet connection"));
        descLabel->setObjectName("EthernetDescLabel");
        
        textLayout->addWidget(titleLabel);
        textLayout->addWidget(descLabel);
        
        // Toggle switch
        m_toggleCheckBox = new QCheckBox;
        m_toggleCheckBox->setObjectName("EthernetToggleBox");
        connect(m_toggleCheckBox, &QCheckBox::toggled, this, &EthernetCardWidget::toggleChanged);
        
        headerLayout->addWidget(iconFrame);
        headerLayout->addLayout(textLayout);
        headerLayout->addStretch();
        headerLayout->addWidget(m_toggleCheckBox);
        
        layout->addLayout(headerLayout);
    }
    
    void setupCombinedConfigCard(const QString &title, QVBoxLayout *layout)
    {
        // Header
        QHBoxLayout *headerLayout = new QHBoxLayout;
        headerLayout->setSpacing(15);
        
        // Icon frame
        QFrame *iconFrame = new QFrame;
        iconFrame->setFixedSize(40, 40);
        iconFrame->setObjectName("ConfigIconFrame");
        
        QHBoxLayout *iconLayout = new QHBoxLayout(iconFrame);
        iconLayout->setContentsMargins(0, 0, 0, 0);
        QLabel *iconLabel = new QLabel("⚙️");
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setObjectName("EthernetIconLabel");
        iconLayout->addWidget(iconLabel);
        
        // Title and description
        QVBoxLayout *textLayout = new QVBoxLayout;
        textLayout->setSpacing(2);
        
        QLabel *titleLabel = new QLabel(title);
        titleLabel->setObjectName("EthernetTitleLabel");
        
        QLabel *descLabel = new QLabel(tr("Configure IP settings and network parameters"));
        descLabel->setObjectName("EthernetDescLabel");
        
        textLayout->addWidget(titleLabel);
        textLayout->addWidget(descLabel);
        
        headerLayout->addWidget(iconFrame);
        headerLayout->addLayout(textLayout);
        headerLayout->addStretch();
        
        layout->addLayout(headerLayout);
        
        // IP Configuration Mode Section
        QVBoxLayout *configSection = new QVBoxLayout;
        configSection->setSpacing(10);
        
        // Radio buttons for DHCP/Static
        QHBoxLayout *radioLayout = new QHBoxLayout;
        radioLayout->setSpacing(30);
        
        m_buttonGroup = new QButtonGroup(this);
        QRadioButton *dhcpRadio = new QRadioButton(tr("DHCP (Automatic)"));
        QRadioButton *staticRadio = new QRadioButton(tr("Static IP"));
        
        dhcpRadio->setObjectName("EthernetRadioButton");
        staticRadio->setObjectName("EthernetRadioButton");
        
        m_buttonGroup->addButton(dhcpRadio, 0);
        m_buttonGroup->addButton(staticRadio, 1);
        
        connect(m_buttonGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
                [this](int id) {
                    bool isDHCP = (id == 0);
                    updateParameterFields(!isDHCP);
                    emit configModeChanged(isDHCP);
                });
        
        radioLayout->addWidget(dhcpRadio);
        radioLayout->addWidget(staticRadio);
        radioLayout->addStretch();
        
        configSection->addLayout(radioLayout);
        
        // Separator line
        QFrame *separator = new QFrame;
        separator->setFrameShape(QFrame::HLine);
        separator->setFrameShadow(QFrame::Sunken);
        separator->setObjectName("EthernetSeparator");
        configSection->addWidget(separator);
        
        // Network Parameters Section
        QVBoxLayout *paramsLayout = new QVBoxLayout;
        paramsLayout->setSpacing(12);
        
        // IP Address
        QHBoxLayout *ipLayout = new QHBoxLayout;
        QLabel *ipLabel = new QLabel(tr("IP Address:"));
        ipLabel->setMinimumWidth(120);
        ipLabel->setObjectName("EthernetParamLabel");
        m_ipEdit = new QLineEdit;
        m_ipEdit->setContextMenuPolicy(Qt::NoContextMenu);
        m_ipEdit->setObjectName("EthernetParamEdit");
        setupIPValidator(m_ipEdit);
        ipLayout->addWidget(ipLabel);
        ipLayout->addWidget(m_ipEdit);
        
        // Subnet Mask
        QHBoxLayout *maskLayout = new QHBoxLayout;
        QLabel *maskLabel = new QLabel(tr("Subnet Mask:"));
        maskLabel->setMinimumWidth(120);
        maskLabel->setObjectName("EthernetParamLabel");
        m_maskEdit = new QLineEdit;
        m_maskEdit->setContextMenuPolicy(Qt::NoContextMenu);
        m_maskEdit->setObjectName("EthernetParamEdit");
        setupIPValidator(m_maskEdit);
        maskLayout->addWidget(maskLabel);
        maskLayout->addWidget(m_maskEdit);
        
        // Gateway
        QHBoxLayout *gatewayLayout = new QHBoxLayout;
        QLabel *gatewayLabel = new QLabel(tr("Gateway:"));
        gatewayLabel->setMinimumWidth(120);
        gatewayLabel->setObjectName("EthernetParamLabel");
        m_gatewayEdit = new QLineEdit;
        m_gatewayEdit->setContextMenuPolicy(Qt::NoContextMenu);
        m_gatewayEdit->setObjectName("EthernetParamEdit");
        setupIPValidator(m_gatewayEdit);
        gatewayLayout->addWidget(gatewayLabel);
        gatewayLayout->addWidget(m_gatewayEdit);
        
        // DNS
        QHBoxLayout *dnsLayout = new QHBoxLayout;
        QLabel *dnsLabel = new QLabel("DNS:");
        dnsLabel->setMinimumWidth(120);
        dnsLabel->setObjectName("EthernetParamLabel");
        m_dnsEdit = new QLineEdit;
        m_dnsEdit->setContextMenuPolicy(Qt::NoContextMenu);
        m_dnsEdit->setObjectName("EthernetParamEdit");
        setupIPValidator(m_dnsEdit);
        dnsLayout->addWidget(dnsLabel);
        dnsLayout->addWidget(m_dnsEdit);
        
        paramsLayout->addLayout(ipLayout);
        paramsLayout->addLayout(maskLayout);
        paramsLayout->addLayout(gatewayLayout);
        paramsLayout->addLayout(dnsLayout);
        
        configSection->addLayout(paramsLayout);
        layout->addLayout(configSection);
        
        // Set default values
        m_ipEdit->setText("192.168.8.123");
        m_maskEdit->setText("255.255.255.0");
        m_gatewayEdit->setText("192.168.8.1");
        m_dnsEdit->setText("8.8.8.8");
    }
    
    void setupIPValidator(QLineEdit *edit)
    {
        QRegExp netRegMask = QRegExp("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");
        edit->setValidator(new QRegExpValidator(netRegMask, edit));
    }
    
    void updateParameterFields(bool enabled)
    {
        if (m_ipEdit) m_ipEdit->setEnabled(enabled);
        if (m_maskEdit) m_maskEdit->setEnabled(enabled);
        if (m_gatewayEdit) m_gatewayEdit->setEnabled(enabled);
        if (m_dnsEdit) m_dnsEdit->setEnabled(enabled);
    }

private:
    QString m_title;
    CardType m_cardType;
    QCheckBox *m_toggleCheckBox = nullptr;
    QButtonGroup *m_buttonGroup = nullptr;
    QLineEdit *m_ipEdit = nullptr;
    QLineEdit *m_maskEdit = nullptr;
    QLineEdit *m_gatewayEdit = nullptr;
    QLineEdit *m_dnsEdit = nullptr;
};

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
    QVBoxLayout *m_pMainLayout;
    QScrollArea *m_pScrollArea;
    QWidget *m_pContentWidget;
    QVBoxLayout *m_pContentLayout;
    
    EthernetCardWidget *m_pToggleCard;
    EthernetCardWidget *m_pCombinedConfigCard;
    QPushButton *m_pSaveButton;
    
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

void EthernetViewFrmPrivate::InitUI()
{
    Q_Q(EthernetViewFrm);
    
    // Set main widget object name for styling
    q->setObjectName("EthernetViewMainWidget");
    
    // Main layout
    m_pMainLayout = new QVBoxLayout(q);
    m_pMainLayout->setContentsMargins(20, 20, 20, 20);
    m_pMainLayout->setSpacing(20);
    
    // Scroll area for cards
    m_pScrollArea = new QScrollArea;
    m_pScrollArea->setWidgetResizable(true);
    m_pScrollArea->setFrameShape(QFrame::NoFrame);
    m_pScrollArea->setObjectName("EthernetViewScrollArea");
    
    m_pContentWidget = new QWidget;
    m_pContentWidget->setObjectName("EthernetViewContentWidget");
    m_pContentLayout = new QVBoxLayout(m_pContentWidget);
    m_pContentLayout->setContentsMargins(0, 0, 0, 0);
    m_pContentLayout->setSpacing(15);
    
    // Create cards
    m_pToggleCard = new EthernetCardWidget(QObject::tr("Ethernet"), EthernetCardWidget::ToggleCard);
    m_pCombinedConfigCard = new EthernetCardWidget(QObject::tr("Network Configuration"), EthernetCardWidget::CombinedConfigCard);
    
    // Create save button
    m_pSaveButton = new QPushButton(QObject::tr("Save Settings & Reboot"));
    m_pSaveButton->setObjectName("EthernetSaveButton");
    m_pSaveButton->setMinimumHeight(50);
    
    // Add widgets to layout
    m_pContentLayout->addWidget(m_pToggleCard);
    m_pContentLayout->addWidget(m_pCombinedConfigCard);
    m_pContentLayout->addWidget(m_pSaveButton);
    m_pContentLayout->addStretch();
    
    m_pScrollArea->setWidget(m_pContentWidget);
    
    // Add to main layout (no header frame)
    m_pMainLayout->addWidget(m_pScrollArea);
}

void EthernetViewFrmPrivate::InitData()
{
    // Set initial state
    m_pCombinedConfigCard->setDHCPMode(true); // Default to DHCP
}

void EthernetViewFrmPrivate::InitConnect()
{
    Q_Q(EthernetViewFrm);
    
    // Connect toggle card
    QObject::connect(m_pToggleCard, &EthernetCardWidget::toggleChanged, 
                    [q](bool enabled) {
                        q->slotLanSwitchState(enabled ? 1 : 0);
                    });
    
    // Connect config mode changes
    QObject::connect(m_pCombinedConfigCard, &EthernetCardWidget::configModeChanged,
                    [this](bool isDHCP) {
                        // Update parameter fields state when mode changes
                        m_pCombinedConfigCard->setIPAddress(m_pCombinedConfigCard->getIPAddress());
                        m_pCombinedConfigCard->setSubnetMask(m_pCombinedConfigCard->getSubnetMask());
                        m_pCombinedConfigCard->setGateway(m_pCombinedConfigCard->getGateway());
                        m_pCombinedConfigCard->setDNS(m_pCombinedConfigCard->getDNS());
                    });
    
    // Connect save button
    QObject::connect(m_pSaveButton, &QPushButton::clicked,
                    q, &EthernetViewFrm::slotSaveButtonClicked);
}

EthernetViewFrm::EthernetViewFrm(QWidget *parent)
    : SettingBaseFrm(parent)
    , d_ptr(new EthernetViewFrmPrivate(this))
{
}

EthernetViewFrm::~EthernetViewFrm()
{
}

void EthernetViewFrm::setEnter()
{
    Q_D(EthernetViewFrm);
    
    // Load current settings
    int mode = ReadConfig::GetInstance()->getNetwork_Manager_Mode();
    d->m_pToggleCard->setToggleState((mode == 1) ? true : false);

    d->m_pCombinedConfigCard->setIPAddress(ReadConfig::GetInstance()->getLAN_IP());
    d->m_pCombinedConfigCard->setSubnetMask(ReadConfig::GetInstance()->getLAN_Maks());
    d->m_pCombinedConfigCard->setGateway(ReadConfig::GetInstance()->getLAN_Gateway());
    d->m_pCombinedConfigCard->setDNS(ReadConfig::GetInstance()->getLAN_DNS());
    
    bool isDHCP = ReadConfig::GetInstance()->getLan_DHCP();
    d->m_pCombinedConfigCard->setDHCPMode(isDHCP);
}

void EthernetViewFrm::slotSaveButtonClicked()
{
    Q_D(EthernetViewFrm);
    
    // Update button text to show saving state
    d->m_pSaveButton->setText(QObject::tr(" Saving & Rebooting..."));
    d->m_pSaveButton->setEnabled(false);
    
    int dhcpMode = d->m_pCombinedConfigCard->isDHCPMode() ? 0 : 1;
    
    NetworkControlThread::GetInstance()->setLinkLan(dhcpMode,
                                                d->m_pCombinedConfigCard->getIPAddress(),
                                                d->m_pCombinedConfigCard->getSubnetMask(),
                                                d->m_pCombinedConfigCard->getGateway(),
                                                d->m_pCombinedConfigCard->getDNS());

    ReadConfig::GetInstance()->setLAN_IP(d->m_pCombinedConfigCard->getIPAddress());
    ReadConfig::GetInstance()->setLAN_Maks(d->m_pCombinedConfigCard->getSubnetMask());
    ReadConfig::GetInstance()->setLAN_Gateway(d->m_pCombinedConfigCard->getGateway());
    ReadConfig::GetInstance()->setLAN_DNS(d->m_pCombinedConfigCard->getDNS());
    ReadConfig::GetInstance()->setLAN_DHCP(d->m_pCombinedConfigCard->isDHCPMode());
    
    LogD("%s,%s,%d,checkState=%d\n",__FILE__,__func__,__LINE__,d->m_pToggleCard->getToggleState());
    ReadConfig::GetInstance()->setNetwork_Manager_Mode(d->m_pToggleCard->getToggleState() ? 1 : 2);
    LogD("%s,%s,%d,getNetwork_Manager_Mode=%d\n",__FILE__,__func__,__LINE__,ReadConfig::GetInstance()->getNetwork_Manager_Mode());
    ReadConfig::GetInstance()->setSaveConfig();
    LogD("%s,%s,%d,getNetwork_Manager_Mode=%d\n",__FILE__,__func__,__LINE__,ReadConfig::GetInstance()->getNetwork_Manager_Mode());

    myHelper::Utils_Reboot();
}

void EthernetViewFrm::slotLanSwitchState(const int state)
{
#ifdef Q_OS_LINUX
    NetworkControlThread::GetInstance()->setNetworkType(state ? 1 : 2);
    ReadConfig::GetInstance()->setNetwork_Manager_Mode(state ? 1 : 2);
#endif
}

#ifdef SCREENCAPTURE
void EthernetViewFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif

#include "EthernetViewFrm.moc" // For EthernetCardWidget Q_OBJECT