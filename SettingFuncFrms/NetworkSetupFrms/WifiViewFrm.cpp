// Modified WifiViewFrm.cpp - Card-based theme implementation

#include "WifiViewFrm.h"
#include "../SetupItemDelegate/CItemWifiBoxWidget.h"
#include "../SetupItemDelegate/CItemWifiWidget.h"

#ifdef Q_OS_LINUX
#include "RkNetWork/NetworkControlThread.h"
#endif
#include "InputWifiPasswordFrm.h"
#include "ConfirmRestartFrm.h"
#include "Config/ReadConfig.h"
#include "Helper/myhelper.h"
#include "MessageHandler/Log.h"

#include <QListWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QCheckBox>
#include <QScrollArea>
#include <QJsonObject>
#include <QApplication>
#include <QScreen>
#include <QDesktopWidget>
#include <QtConcurrent/QtConcurrent>
#include <QScrollBar>

// Custom WiFi Toggle Card Widget
class WifiToggleCardWidget : public QFrame
{
    Q_OBJECT
public:
    WifiToggleCardWidget(QWidget *parent = nullptr)
        : QFrame(parent)
    {
        setObjectName("WifiToggleCardWidget");
        setupUI();
    }
    
    void setWifiState(bool enabled)
    {
        if (m_toggleCheckBox) {
            m_toggleCheckBox->setChecked(enabled);
        }
    }
    
    bool getWifiState() const
    {
        return m_toggleCheckBox ? m_toggleCheckBox->isChecked() : false;
    }

signals:
    void wifiSwitchChanged(bool enabled);
    void restartRequested();

private:
    void setupUI()
    {
        QHBoxLayout *mainLayout = new QHBoxLayout(this);
        mainLayout->setContentsMargins(20, 16, 20, 16);
        mainLayout->setSpacing(15);
        
        // Icon container
        QFrame *iconFrame = new QFrame;
        iconFrame->setFixedSize(40, 40);
        iconFrame->setObjectName("WifiToggleIconFrame");
        
        QHBoxLayout *iconLayout = new QHBoxLayout(iconFrame);
        iconLayout->setContentsMargins(0, 0, 0, 0);
        
        QLabel *iconLabel = new QLabel;
        iconLabel->setText("📶");
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setObjectName("WifiIconLabel");
        iconLayout->addWidget(iconLabel);
        
        // Text container
        QVBoxLayout *textLayout = new QVBoxLayout;
        textLayout->setSpacing(2);
        
        QLabel *titleLabel = new QLabel("Wi-Fi");
        titleLabel->setObjectName("WifiTitleLabel");
        
        m_statusLabel = new QLabel("Network connection");
        m_statusLabel->setObjectName("WifiStatusLabel");
        
        textLayout->addWidget(titleLabel);
        textLayout->addWidget(m_statusLabel);
        
        // Toggle switch
        m_toggleCheckBox = new QCheckBox;
        m_toggleCheckBox->setObjectName("WifiToggleBox");
        
        QObject::connect(m_toggleCheckBox, &QCheckBox::toggled, this, &WifiToggleCardWidget::wifiSwitchChanged);
        
        // Main layout assembly
        mainLayout->addWidget(iconFrame);
        mainLayout->addLayout(textLayout);
        mainLayout->addStretch();
        mainLayout->addWidget(m_toggleCheckBox);
    }

private:
    QLabel *m_statusLabel;
    QCheckBox *m_toggleCheckBox;
};

// Custom WiFi Network Card Widget
class WifiNetworkCardWidget : public QFrame
{
    Q_OBJECT
public:
    WifiNetworkCardWidget(QWidget *parent = nullptr)
        : QFrame(parent)
    {
        setObjectName("WifiNetworkCardWidget");
        setupUI();
        setVisible(false); // Hidden by default
    }
    
    void setData(const QString &service, const QString &ssid, float strength)
    {
        m_serviceText = service;
        m_ssidText = ssid;
        
        if (m_ssidLabel) {
            m_ssidLabel->setText(ssid);
        }
        
        if (m_strengthLabel) {
            int strengthPercent = qRound(strength * 100);
            m_strengthLabel->setText(QString("%1%").arg(strengthPercent));
        }
        
        // Update signal strength icon
        updateSignalIcon(strength);
        
        // Update connection status styling
        updateConnectionStatus(ssid);
    }
    
    QString getServiceText() const { return m_serviceText; }
    QString getSSIDText() const { return m_ssidText; }

signals:
    void networkClicked(const QString &service, const QString &ssid);

protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        emit networkClicked(m_serviceText, m_ssidText);
        QFrame::mousePressEvent(event);
    }
    
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
    void setupUI()
    {
        QHBoxLayout *mainLayout = new QHBoxLayout(this);
        mainLayout->setContentsMargins(20, 16, 20, 16);
        mainLayout->setSpacing(15);
        
        // Signal strength icon
        m_signalFrame = new QFrame;
        m_signalFrame->setFixedSize(32, 32);
        m_signalFrame->setObjectName("WifiSignalIconFrame");
        
        QHBoxLayout *signalLayout = new QHBoxLayout(m_signalFrame);
        signalLayout->setContentsMargins(0, 0, 0, 0);
        
        m_signalIcon = new QLabel;
        m_signalIcon->setText("📶");
        m_signalIcon->setAlignment(Qt::AlignCenter);
        m_signalIcon->setObjectName("WifiSignalIconLabel");
        signalLayout->addWidget(m_signalIcon);
        
        // Network info
        QVBoxLayout *infoLayout = new QVBoxLayout;
        infoLayout->setSpacing(2);
        
        m_ssidLabel = new QLabel("Network Name");
        m_ssidLabel->setObjectName("WifiNetworkNameLabel");
        
        m_statusLabel = new QLabel("Available");
        m_statusLabel->setObjectName("WifiNetworkStatusLabel");
        
        infoLayout->addWidget(m_ssidLabel);
        infoLayout->addWidget(m_statusLabel);
        
        // Signal strength
        m_strengthLabel = new QLabel("0%");
        m_strengthLabel->setObjectName("WifiStrengthLabel");
        
        // Main layout assembly
        mainLayout->addWidget(m_signalFrame);
        mainLayout->addLayout(infoLayout);
        mainLayout->addStretch();
        mainLayout->addWidget(m_strengthLabel);
    }
    
    void updateSignalIcon(float strength)
    {
        QString iconObjectName;
        if (strength >= 0.8f) {
            iconObjectName = "WifiSignalStrongFrame";
        } else if (strength >= 0.5f) {
            iconObjectName = "WifiSignalMediumFrame";
        } else if (strength >= 0.2f) {
            iconObjectName = "WifiSignalWeakFrame";
        } else {
            iconObjectName = "WifiSignalVeryWeakFrame";
        }
        
        m_signalFrame->setObjectName(iconObjectName);
        m_signalFrame->style()->unpolish(m_signalFrame);
        m_signalFrame->style()->polish(m_signalFrame);
    }
    
    void updateConnectionStatus(const QString &ssid)
    {
        if (ssid.endsWith(QObject::tr("\nConnected"))) {
            m_statusLabel->setText("Connected");
            m_statusLabel->setObjectName("WifiConnectedStatusLabel");
            setObjectName("WifiConnectedCardWidget");
        } else if (ssid.endsWith(QObject::tr("\nConnecting"))) {
            m_statusLabel->setText("Connecting...");
            m_statusLabel->setObjectName("WifiConnectingStatusLabel");
            setObjectName("WifiConnectingCardWidget");
        } else {
            m_statusLabel->setText("Available");
            m_statusLabel->setObjectName("WifiNetworkStatusLabel");
            setObjectName("WifiNetworkCardWidget");
        }
        
        style()->unpolish(this);
        style()->polish(this);
    }

private:
    QString m_serviceText;
    QString m_ssidText;
    QFrame *m_signalFrame;
    QLabel *m_signalIcon;
    QLabel *m_ssidLabel;
    QLabel *m_statusLabel;
    QLabel *m_strengthLabel;
};

class WifiViewFrmPrivate
{
    Q_DECLARE_PUBLIC(WifiViewFrm)
public:
    WifiViewFrmPrivate(WifiViewFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    void setHideNetworks(int startIndex);
private:
    QVBoxLayout *m_pMainLayout;
    QLabel *m_pHeaderLabel;
    QFrame *m_pHeaderFrame;
    QScrollArea *m_pScrollArea;
    QWidget *m_pContentWidget;
    QVBoxLayout *m_pContentLayout;
    
    WifiToggleCardWidget *m_wifiToggleCard;
    QList<WifiNetworkCardWidget*> m_networkCards;
private:
    WifiViewFrm *const q_ptr;
    bool ExecOnce;
    QString CurrentWiFi;
};

WifiViewFrmPrivate::WifiViewFrmPrivate(WifiViewFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

void WifiViewFrmPrivate::InitUI()
{
    Q_Q(WifiViewFrm);
    
    // Set main widget object name for styling
    q->setObjectName("WifiViewMainWidget");
    
    // Main layout
    m_pMainLayout = new QVBoxLayout(q);
    m_pMainLayout->setContentsMargins(20, 20, 20, 20);
    m_pMainLayout->setSpacing(20);
    
    // Header frame
    m_pHeaderFrame = new QFrame;
    m_pHeaderFrame->setObjectName("WifiViewHeaderFrame");
    
    QHBoxLayout *headerLayout = new QHBoxLayout(m_pHeaderFrame);
    headerLayout->setContentsMargins(20, 15, 20, 15);
    headerLayout->setSpacing(15);
    
    QFrame *headerIconFrame = new QFrame;
    headerIconFrame->setFixedSize(40, 40);
    headerIconFrame->setObjectName("WifiViewHeaderIcon");
    
    QHBoxLayout *headerIconLayout = new QHBoxLayout(headerIconFrame);
    headerIconLayout->setContentsMargins(0, 0, 0, 0);
    QLabel *headerIcon = new QLabel("📶");
    headerIcon->setAlignment(Qt::AlignCenter);
    headerIcon->setObjectName("WifiIconLabel");
    headerIconLayout->addWidget(headerIcon);
    
    m_pHeaderLabel = new QLabel(QObject::tr("Wi-Fi Networks"));
    m_pHeaderLabel->setObjectName("WifiViewHeaderTitle");
    
    headerLayout->addWidget(headerIconFrame);
    headerLayout->addWidget(m_pHeaderLabel);
    headerLayout->addStretch();
    
    // Scroll area for cards
    m_pScrollArea = new QScrollArea;
    m_pScrollArea->setWidgetResizable(true);
    m_pScrollArea->setFrameShape(QFrame::NoFrame);
    m_pScrollArea->setObjectName("WifiViewScrollArea");
    m_pScrollArea->setContextMenuPolicy(Qt::NoContextMenu);
    m_pScrollArea->horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    m_pScrollArea->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    
    m_pContentWidget = new QWidget;
    m_pContentWidget->setObjectName("WifiViewContentWidget");
    m_pContentLayout = new QVBoxLayout(m_pContentWidget);
    m_pContentLayout->setContentsMargins(0, 0, 0, 0);
    m_pContentLayout->setSpacing(8);
    
    m_pScrollArea->setWidget(m_pContentWidget);
    
    // Add to main layout
    m_pMainLayout->addWidget(m_pHeaderFrame);
    m_pMainLayout->addWidget(m_pScrollArea);
}

void WifiViewFrmPrivate::InitData()
{
    printf(">>>%s,%s,%d,\n",__FILE__,__func__,__LINE__);
    
    // Create WiFi toggle card
    m_wifiToggleCard = new WifiToggleCardWidget;
    m_pContentLayout->addWidget(m_wifiToggleCard);
    
    // Set initial WiFi state
    bool wifiEnabled = (ReadConfig::GetInstance()->getNetwork_Manager_Mode() == 2);
    m_wifiToggleCard->setWifiState(wifiEnabled);
    
    // Create network cards (30 maximum)
    for(int i = 0; i < 30; i++) {
        WifiNetworkCardWidget *networkCard = new WifiNetworkCardWidget;
        m_networkCards.append(networkCard);
        m_pContentLayout->addWidget(networkCard);
    }
    
    // Add stretch at the end
    m_pContentLayout->addStretch();
    
    // Disable context menus for all scroll bars
    foreach(QObject *widget, qApp->allWidgets()) {
        QScrollBar *scrollBar = qobject_cast<QScrollBar*>(widget);
        if(scrollBar) {
            scrollBar->setContextMenuPolicy(Qt::NoContextMenu);
        }
    }
    
    // Hide all network cards initially if WiFi is disabled
    if (!wifiEnabled) {
        setHideNetworks(0);
    }
}

void WifiViewFrmPrivate::InitConnect()
{
    // WiFi toggle connections
    QObject::connect(m_wifiToggleCard, &WifiToggleCardWidget::wifiSwitchChanged, 
                    q_func(), &WifiViewFrm::slotWifiSwitchState);
    QObject::connect(m_wifiToggleCard, &WifiToggleCardWidget::restartRequested, 
                    q_func(), &WifiViewFrm::slotSaveAndRestart);
    
    // Network card connections
    for(WifiNetworkCardWidget *card : m_networkCards) {
        QObject::connect(card, &WifiNetworkCardWidget::networkClicked,
                        [this](const QString &service, const QString &ssid) {
            Q_Q(WifiViewFrm);
            q->handleNetworkClicked(service, ssid);
        });
    }

#ifdef Q_OS_LINUX
    QObject::connect(NetworkControlThread::GetInstance(), &NetworkControlThread::sigWifiList, 
                    q_func(), &WifiViewFrm::slotWifiList);
#endif
}

void WifiViewFrmPrivate::setHideNetworks(int startIndex)
{
    for(int i = startIndex; i < m_networkCards.count(); i++) {
        m_networkCards[i]->setVisible(false);
    }
}

// Rest of your existing WifiViewFrm implementation...
WifiViewFrm::WifiViewFrm(QWidget *parent)
    : SettingBaseFrm(parent)
    , d_ptr(new WifiViewFrmPrivate(this))
{
}

WifiViewFrm::~WifiViewFrm()
{
}

void WifiViewFrm::setEnter()
{
    Q_D(WifiViewFrm);
    
    // Update WiFi toggle state
    bool wifiEnabled = (ReadConfig::GetInstance()->getNetwork_Manager_Mode() == 2);
    d->m_wifiToggleCard->setWifiState(wifiEnabled);
    
    d->ExecOnce = false;
    d->CurrentWiFi.clear();

#ifdef Q_OS_LINUX
    if(wifiEnabled) {
        NetworkControlThread::GetInstance()->setWifiSearchMode(1);
        NetworkControlThread::GetInstance()->resume();
    }
#endif
}

void WifiViewFrm::setLeaveEvent()
{
    Q_D(WifiViewFrm);

#ifdef Q_OS_LINUX
    NetworkControlThread::GetInstance()->pause();
#endif

    foreach (QWidget *w, qApp->topLevelWidgets()) {
        if(w->objectName() == "InputWifiPassword") {
            QDialog *dlg = (QDialog *)w;
            dlg->done(1);
        }
    }
    
    d->setHideNetworks(0);
    
    // Save current WiFi state
    bool wifiEnabled = d->m_wifiToggleCard->getWifiState();
    ReadConfig::GetInstance()->setNetwork_Manager_Mode(wifiEnabled ? 2 : 1);
    QtConcurrent::run(ReadConfig::GetInstance(), &ReadConfig::setSaveConfig);
    LogD("%s,%s,%d,getNetwork_Manager_Mode=%d\n",__FILE__,__func__,__LINE__,ReadConfig::GetInstance()->getNetwork_Manager_Mode());
}

void WifiViewFrm::slotWifiList(const QList<QVariant> obj)
{
    Q_D(WifiViewFrm);
    
    int cnt = qMin(obj.count(), 29);
    
    for(int i = 0; i < cnt; i++) {
        const auto &jsObj = obj.at(i).toJsonObject();
        if (i < d->m_networkCards.count()) {
            WifiNetworkCardWidget *networkCard = d->m_networkCards[i];
            QString name = jsObj.value("sName").toString();
            
            if (name.length() > 0) {
                LogD("%s,%s,%d,ssid=%s\n",__FILE__,__func__,__LINE__,name.toStdString().c_str());
                
                networkCard->setData(jsObj.value("sService").toString(), 
                                   name, 
                                   jsObj.value("Strength").toDouble()/100.0f);
                networkCard->setVisible(true);
                
                // Handle connected WiFi
                if(name.endsWith(QObject::tr("\nConnected"))) {
                    QString wifiname = name;
                    wifiname.chop(QObject::tr("\nConnected").length());
                    
                    if(wifiname.compare(d->CurrentWiFi)) {
                        d->CurrentWiFi = wifiname;
                        
                        if(d->ExecOnce) {
                            ConfirmRestartFrm dlg(this);
                            dlg.setSizeGripEnabled(true);
                            dlg.setModal(true);
                            dlg.show();
                            if(dlg.exec() == 0) {
                                slotSaveAndRestart();
                            }
                        }
                    }
                }
                
                if(name.endsWith(QObject::tr("\nConnecting"))) {
                    d->CurrentWiFi = name;
                    d->CurrentWiFi.chop(QObject::tr("\nConnecting").length());
                }
            }
        }
    }
    
    d->ExecOnce = true;
    if(cnt > 0) d->setHideNetworks(cnt);
}

void WifiViewFrm::slotWifiSwitchState(bool state)
{
    Q_D(WifiViewFrm);

#ifdef Q_OS_LINUX
    LogD(">>>%s,%s,%d,state=%d\n",__FILE__,__func__,__LINE__,state);    
    
    if (state) {
        NetworkControlThread::GetInstance()->setNetworkType(2);
        ReadConfig::GetInstance()->setNetwork_Manager_Mode(2);        
        QtConcurrent::run(ReadConfig::GetInstance(), &ReadConfig::setSaveConfig);
        NetworkControlThread::GetInstance()->setWifiSearchMode(1);
        NetworkControlThread::GetInstance()->resume();
        
        // Show network cards
        for(int i = 0; i < d->m_networkCards.count(); i++) {
            d->m_networkCards[i]->setVisible(false); // Will be shown when networks are found
        }
    } else {
        NetworkControlThread::GetInstance()->setNetworkType(1); 
        ReadConfig::GetInstance()->setNetwork_Manager_Mode(1);
        NetworkControlThread::GetInstance()->pause();
        
        // Hide all network cards
        d->setHideNetworks(0);
        d->CurrentWiFi.clear();  
    }
#endif
}

void WifiViewFrm::handleNetworkClicked(const QString &service, const QString &ssid)
{
#ifdef SCREENCAPTURE
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");     
#endif     

    InputWifiPasswordFrm dlg(this);
    dlg.setData(service, ssid);
    dlg.show();
    if(dlg.exec() == 0) {
        ReadConfig::GetInstance()->setWIFI_Name(dlg.getWifiName());
        ReadConfig::GetInstance()->setWIFI_Password(dlg.getPassword());

#ifdef Q_OS_LINUX
        NetworkControlThread::GetInstance()->DisconnectAllWifi();
        NetworkControlThread::GetInstance()->setLinkWlanSSID(dlg.getWifiName(), dlg.getPassword());
        NetworkControlThread::GetInstance()->setLinkWlan(service, dlg.getPassword());
#endif
    }
}

void WifiViewFrm::slotIemClicked(QListWidgetItem *item)
{
    // Compatibility method - functionality moved to handleNetworkClicked
}

void WifiViewFrm::slotSaveAndRestart()
{
    Q_D(WifiViewFrm);
    
    NetworkControlThread::GetInstance()->setNetworkType(2);            
    bool wifiEnabled = d->m_wifiToggleCard->getWifiState();
    ReadConfig::GetInstance()->setNetwork_Manager_Mode(wifiEnabled ? 2 : 1);
    ReadConfig::GetInstance()->setSaveConfig();
    LogD(">>>%s,%s,%d,getNetwork_Manager_Mode=%d\n",__FILE__,__func__,__LINE__,ReadConfig::GetInstance()->getNetwork_Manager_Mode());
    myHelper::Utils_Reboot();
}

#ifdef SCREENCAPTURE
void WifiViewFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif

#include "WifiViewFrm.moc" // For custom card widgets