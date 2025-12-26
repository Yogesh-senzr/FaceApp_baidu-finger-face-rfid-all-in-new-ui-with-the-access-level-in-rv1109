// Modified NetworkSetupFrm.cpp - CSS-integrated card-based theme

#include "NetworkSetupFrm.h"
#include "../SetupItemDelegate/CItemWidget.h"
#include "Config/ReadConfig.h"
#include "../SetupItemDelegate/CItemBoxWidget.h"
#ifdef Q_OS_LINUX
#include "RkNetWork/NetworkControlThread.h"
#endif
#include "MessageHandler/Log.h"
#include "RkNetWork/Network4GControlThread.h"

#include <fcntl.h>
#include <unistd.h>
#include <QListWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QtConcurrent/QtConcurrent>

// Custom Card Widget that uses CSS styling
class NetworkCardWidget : public QFrame
{
    Q_OBJECT
public:
    NetworkCardWidget(const QString &title, const QString &iconPath, QWidget *parent = nullptr)
        : QFrame(parent), m_title(title)
    {
        setObjectName("NetworkCardWidget");
        setupUI(title, iconPath);
    }
    
    QString getTitle() const { return m_title; }
    
    void setStatus(const QString &status)
    {
        m_statusLabel->setText(status);
    }
    
    void setConnected(bool connected)
    {
        m_statusIndicator->setObjectName(connected ? "ConnectedIndicator" : "DisconnectedIndicator");
        m_statusIndicator->style()->unpolish(m_statusIndicator);
        m_statusIndicator->style()->polish(m_statusIndicator);
    }

signals:
    void cardClicked(const QString &title);

protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        emit cardClicked(m_title);
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
    void setupUI(const QString &title, const QString &iconPath)
    {
        QHBoxLayout *mainLayout = new QHBoxLayout(this);
        mainLayout->setContentsMargins(12, 10, 12, 10); // Reduced margins
mainLayout->setSpacing(12);
        
        // Icon container
        QFrame *iconFrame = new QFrame;
        iconFrame->setFixedSize(24, 24);
        iconFrame->setObjectName(getIconObjectName(title));
        
        QHBoxLayout *iconLayout = new QHBoxLayout(iconFrame);
        iconLayout->setContentsMargins(0, 0, 0, 0);
        
        QLabel *iconLabel = new QLabel;
        iconLabel->setText(getIconText(title));
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setObjectName("NetworkIconLabel");
        iconLayout->addWidget(iconLabel);
        
        // Text container
        QVBoxLayout *textLayout = new QVBoxLayout;
        textLayout->setSpacing(3);
        
        QLabel *titleLabel = new QLabel(title);
        titleLabel->setObjectName("NetworkTitleLabel");
        
        m_statusLabel = new QLabel("Ready");
        m_statusLabel->setObjectName("NetworkStatusLabel");
        
        textLayout->addWidget(titleLabel);
        textLayout->addWidget(m_statusLabel);
        
        // Status indicator and arrow
        QHBoxLayout *rightLayout = new QHBoxLayout;
        rightLayout->setSpacing(10);
        
        m_statusIndicator = new QLabel;
        m_statusIndicator->setFixedSize(8, 8);
        m_statusIndicator->setObjectName("DisconnectedIndicator");
        
        QLabel *arrowLabel = new QLabel("›");
        arrowLabel->setObjectName("NetworkArrowLabel");
        
        rightLayout->addWidget(m_statusIndicator);
        rightLayout->addWidget(arrowLabel);
        
        // Main layout assembly
        mainLayout->addWidget(iconFrame);
        mainLayout->addLayout(textLayout);
        mainLayout->addStretch();
        mainLayout->addLayout(rightLayout);
    }
    
    QString getIconObjectName(const QString &title)
    {
        if (title.contains("Wi-Fi") || title.contains("wifi", Qt::CaseInsensitive))
            return "WifiIconFrame";
        else if (title.contains("Ethernet", Qt::CaseInsensitive))
            return "EthernetIconFrame";
        else if (title.contains("4G", Qt::CaseInsensitive))
            return "MobileIconFrame";
        return "DefaultIconFrame";
    }
    
    QString getIconText(const QString &title)
{
    if (title.contains("Wi-Fi") || title.contains("wifi", Qt::CaseInsensitive))
        return ""; // WiFi icon
    else if (title.contains("Ethernet", Qt::CaseInsensitive))
        return ""; // Ethernet icon
    else if (title.contains("4G", Qt::CaseInsensitive))
        return ""; // 4G icon
    return ""; // Network icon
}

private:
    QString m_title;
    QLabel *m_statusLabel;
    QLabel *m_statusIndicator;
};

class NetworkSetupFrmPrivate
{
    Q_DECLARE_PUBLIC(NetworkSetupFrm)
public:
    NetworkSetupFrmPrivate(NetworkSetupFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QVBoxLayout *m_pMainLayout;
   // QLabel *m_pHeaderLabel;
   // QFrame *m_pHeaderFrame;
    QScrollArea *m_pScrollArea;
    QWidget *m_pContentWidget;
    QVBoxLayout *m_pContentLayout;
    QList<NetworkCardWidget*> m_cardWidgets;
private:
    NetworkSetupFrm *const q_ptr;
};

NetworkSetupFrmPrivate::NetworkSetupFrmPrivate(NetworkSetupFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

void NetworkSetupFrmPrivate::InitUI()
{
    Q_Q(NetworkSetupFrm);
    
    // Set main widget object name for styling
    q->setObjectName("NetworkSetupMainWidget");
    
    // Main layout
    m_pMainLayout = new QVBoxLayout(q);
    m_pMainLayout->setContentsMargins(12, 12, 12, 12);
    m_pMainLayout->setSpacing(20);
    
    
    // Scroll area for cards
    m_pScrollArea = new QScrollArea;
    m_pScrollArea->setWidgetResizable(true);
    m_pScrollArea->setFrameShape(QFrame::NoFrame);
    m_pScrollArea->setObjectName("NetworkSetupScrollArea");
    
    m_pContentWidget = new QWidget;
    m_pContentWidget->setObjectName("NetworkSetupContentWidget");
    m_pContentLayout = new QVBoxLayout(m_pContentWidget);
    m_pContentLayout->setContentsMargins(0, 0, 0, 0);
    m_pContentLayout->setSpacing(6);
    
    m_pScrollArea->setWidget(m_pContentWidget);
    
    // Add to main layout
  //  m_pMainLayout->addWidget(m_pHeaderFrame);
    m_pMainLayout->addWidget(m_pScrollArea);
}

void NetworkSetupFrmPrivate::InitData()
{
    QStringList listName;
    if(!access("/sys/class/net/wlan0/",F_OK))
    {
        listName << QObject::tr("Wi-FiSetup");
    }
    listName << QObject::tr("EthernetSetup");
#if 1
    {
        listName << QObject::tr("4G");
    }
#endif     

    for(int i = 0; i < listName.count(); i++)
    {
        NetworkCardWidget *cardWidget = new NetworkCardWidget(listName.at(i), "");
        
        // Set initial status based on network type
        if(listName.at(i).contains("Wi-Fi"))
        {
            cardWidget->setStatus("Searching for networks...");
            cardWidget->setConnected(false);
        }
        else if(listName.at(i).contains("Ethernet"))
        {
            cardWidget->setStatus("Cable disconnected");
            cardWidget->setConnected(false);
        }
        else if(listName.at(i).contains("4G"))
        {
            cardWidget->setStatus("Signal strength: Good");
            cardWidget->setConnected(true);
        }
        
        m_cardWidgets.append(cardWidget);
        m_pContentLayout->addWidget(cardWidget);
    }
    
    // Add stretch at the end
    m_pContentLayout->addStretch();
}

void NetworkSetupFrmPrivate::InitConnect()
{
    for(NetworkCardWidget *card : m_cardWidgets)
    {
        QObject::connect(card, &NetworkCardWidget::cardClicked, 
                        [this](const QString &title) {
            Q_Q(NetworkSetupFrm);
            emit q->sigShowFrm(title);
        });
    }
}

// Rest of your existing NetworkSetupFrm implementation...
NetworkSetupFrm::NetworkSetupFrm(QWidget *parent)
    : SettingBaseFrm(parent)
    , d_ptr(new NetworkSetupFrmPrivate(this))
{
}

NetworkSetupFrm::~NetworkSetupFrm()
{
}

void NetworkSetupFrm::setLeaveEvent()
{
    QtConcurrent::run(ReadConfig::GetInstance(), &ReadConfig::setSaveConfig);
}

void NetworkSetupFrm::setEnter()
{
    Q_D(NetworkSetupFrm);    
    // Update card statuses when entering the screen
    // You can add network status checking here
}

void NetworkSetupFrm::slotIemClicked(QListWidgetItem *item)
{
    // This method is now handled by card click events
    // Keep for compatibility if needed
}

void NetworkSetupFrm::onCardClicked(const QString &cardTitle)
{
    Q_D(NetworkSetupFrm);

#ifdef SCREENCAPTURE
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");        
#endif    

    emit sigShowFrm(cardTitle);
}

void NetworkSetupFrm::updateNetworkStatus(const QString &networkType, const QString &status, bool connected)
{
    Q_D(NetworkSetupFrm);
    
    for(NetworkCardWidget *card : d->m_cardWidgets)
    {
        if(card->getTitle().contains(networkType, Qt::CaseInsensitive))
        {
            card->setStatus(status);
            card->setConnected(connected);
            break;
        }
    }
}

#ifdef SCREENCAPTURE
void NetworkSetupFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif

#include "NetworkSetupFrm.moc" // For NetworkCardWidget Q_OBJECT