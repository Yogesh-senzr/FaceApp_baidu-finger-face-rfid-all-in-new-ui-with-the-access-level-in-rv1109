// HomeMenuFrm.cpp - Full screen layout with PNG icons

#include "HomeMenuFrm.h"
#include "SettingFuncFrms/SysSetupFrms/LanguageFrm.h"
#include "PasswordDialog.h"

#include <QToolButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QLabel>
#include <QScrollArea>
#include <QStyleOption>
#include <QPainter>
#include <QMessageBox>
#include <QPixmap>

// Custom Home Menu Card Widget
class HomeMenuCardWidget : public QFrame
{
    Q_OBJECT
public:
    HomeMenuCardWidget(const QString &title, const QString &iconPath, QWidget *parent = nullptr)
        : QFrame(parent), m_title(title), m_iconPath(iconPath)
    {
        setObjectName("HomeMenuCardWidget");
        setupUI(title, iconPath);
    }
    
    QString getTitle() const { return m_title; }
    
    void setDescription(const QString &description)
    {
        if (m_descLabel) {
            m_descLabel->setText(description);
        }
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
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(20, 20, 20, 20);
        mainLayout->setSpacing(15);
        
        // Icon container - increased size
        QFrame *iconFrame = new QFrame;
        iconFrame->setFixedSize(80, 80);
        iconFrame->setObjectName(getIconObjectName(title));
        
        QVBoxLayout *iconLayout = new QVBoxLayout(iconFrame);
        iconLayout->setContentsMargins(0, 0, 0, 0);
        
        QLabel *iconLabel = new QLabel;
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setObjectName("HomeMenuIconLabel");
        
        // Load PNG icon - increased size
        QPixmap iconPixmap(iconPath);
        if (!iconPixmap.isNull()) {
            // Scale the icon to larger size (56x56 pixels)
            QPixmap scaledPixmap = iconPixmap.scaled(56, 56, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            iconLabel->setPixmap(scaledPixmap);
        } else {
            // Fallback to text icon if PNG not found
            iconLabel->setText(getFallbackIconText(title));
        }
        
        iconLayout->addWidget(iconLabel);
        
        // Title
        QLabel *titleLabel = new QLabel(title);
        titleLabel->setAlignment(Qt::AlignCenter);
        titleLabel->setObjectName("HomeMenuTitleLabel");
        titleLabel->setWordWrap(true);
        
        // Description
        m_descLabel = new QLabel(getDescription(title));
        m_descLabel->setAlignment(Qt::AlignCenter);
        m_descLabel->setObjectName("HomeMenuDescLabel");
        m_descLabel->setWordWrap(true);
        
        // Layout assembly
        mainLayout->addWidget(iconFrame, 0, Qt::AlignCenter);
        mainLayout->addWidget(titleLabel);
        mainLayout->addWidget(m_descLabel);
        mainLayout->addStretch();
    }
    
    QString getIconObjectName(const QString &title)
    {
        if (title.contains("Person") || title.contains("Managing"))
            return "PersonManagingIconFrame";
        else if (title.contains("Records"))
            return "RecordsManagementIconFrame";
        else if (title.contains("Network"))
            return "NetworkSetupIconFrame";
        else if (title.contains("Host") || title.contains("Server"))
            return "HostSetupIconFrame";
        else if (title.contains("System"))
            return "SystemSetupIconFrame";
        else if (title.contains("Identify"))
            return "IdentifySetupIconFrame";
        return "DefaultHomeMenuIconFrame";
    }
    
    QString getFallbackIconText(const QString &title)
    {
        if (title.contains("Person") || title.contains("Managing"))
            return "P";
        else if (title.contains("Records"))
            return "R";
        else if (title.contains("Network"))
            return "N";
        else if (title.contains("Host") || title.contains("Server"))
            return "H";
        else if (title.contains("System"))
            return "S";
        else if (title.contains("Identify"))
            return "I";
        return "?";
    }
    
    QString getDescription(const QString &title)
    {
        if (title.contains("Person") || title.contains("Managing"))
            return "Manage people database";
        else if (title.contains("Records"))
            return "View access records";
        else if (title.contains("Network"))
            return "Configure network settings";
        else if (title.contains("Host") || title.contains("Server"))
            return "Server configuration";
        else if (title.contains("System"))
            return "System preferences";
        else if (title.contains("Identify"))
            return "Recognition settings";
        return "Configuration";
    }

private:
    QString m_title;
    QString m_iconPath;
    QLabel *m_descLabel;
};

class HomeMenuFrmPrivate
{
    Q_DECLARE_PUBLIC(HomeMenuFrm)
public:
    HomeMenuFrmPrivate(HomeMenuFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
    QString getIconPath(const QString &title);
private:
    QVBoxLayout *m_pMainLayout;
    QScrollArea *m_pScrollArea;
    QWidget *m_pContentWidget;
    QGridLayout *m_pGridLayout;
    QList<HomeMenuCardWidget*> m_cardWidgets;
private:
    HomeMenuFrm *const q_ptr;
};

HomeMenuFrmPrivate::HomeMenuFrmPrivate(HomeMenuFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

void HomeMenuFrmPrivate::InitUI()
{
    Q_Q(HomeMenuFrm);
    
    // Set main widget object name for styling
    q->setObjectName("HomeMenuMainWidget");
    
    // Main layout - NO HEADER, direct to content
    m_pMainLayout = new QVBoxLayout(q);
    m_pMainLayout->setContentsMargins(20, 20, 20, 20);
    m_pMainLayout->setSpacing(0); // No spacing since we only have one widget
    
    // Scroll area for cards - fills entire screen
    m_pScrollArea = new QScrollArea;
    m_pScrollArea->setWidgetResizable(true);
    m_pScrollArea->setFrameShape(QFrame::NoFrame);
    m_pScrollArea->setObjectName("HomeMenuScrollArea");
    m_pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_pScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    m_pContentWidget = new QWidget;
    m_pContentWidget->setObjectName("HomeMenuContentWidget");
    
    // Grid layout for cards (2 columns, 3 rows) - fills available height
    m_pGridLayout = new QGridLayout(m_pContentWidget);
    m_pGridLayout->setContentsMargins(10, 10, 10, 10);
    m_pGridLayout->setHorizontalSpacing(20);
    m_pGridLayout->setVerticalSpacing(20);
    
    m_pScrollArea->setWidget(m_pContentWidget);
    
    // Add scroll area to main layout - it takes all available space
    m_pMainLayout->addWidget(m_pScrollArea, 1);
}

QString HomeMenuFrmPrivate::getIconPath(const QString &title)
{
    // Define your PNG icon paths here
    if (title.contains("Person") || title.contains("Managing"))
        return ":/Images/person_icon.png";
    else if (title.contains("Records"))
        return ":/Images/records_icon.png";
    else if (title.contains("Network"))
        return ":/Images/network_icon.png";
    else if (title.contains("Host") || title.contains("Server"))
        return ":/Images/server_icon.png";
    else if (title.contains("System"))
        return ":/Images/system_icon.png";
    else if (title.contains("Identify"))
        return ":/Images/identify_icon.png";
    return ":/Images/default_icon.png";
}

void HomeMenuFrmPrivate::InitData()
{
    // Create cards for each menu option
    QStringList cardTitles = {
        QObject::tr("PersonManaging"),      // Index 0
        QObject::tr("RecordsManagement"),   // Index 1
        QObject::tr("NetworkSetup"),        // Index 2
        QObject::tr("HostSetup"),           // Index 3
        QObject::tr("IdentifySetup"),       // Index 4
        QObject::tr("SystemSetup")          // Index 5
    };
    
    for(int i = 0; i < cardTitles.count(); i++)
    {
        QString title = cardTitles.at(i);
        QString iconPath = getIconPath(title);
        HomeMenuCardWidget *cardWidget = new HomeMenuCardWidget(title, iconPath);
        m_cardWidgets.append(cardWidget);
        
        // Arrange in 2x3 grid
        int row = i / 2;
        int col = i % 2;
        m_pGridLayout->addWidget(cardWidget, row, col);
    }
    
    // Make rows expand equally to fill available height
    for(int row = 0; row < 3; row++) {
        m_pGridLayout->setRowStretch(row, 1);
    }
    
    // Make columns expand equally
    m_pGridLayout->setColumnStretch(0, 1);
    m_pGridLayout->setColumnStretch(1, 1);
}

void HomeMenuFrmPrivate::InitConnect()
{
    for(int i = 0; i < m_cardWidgets.count(); i++)
    {
        HomeMenuCardWidget *card = m_cardWidgets[i];
        QObject::connect(card, &HomeMenuCardWidget::cardClicked, 
                        [this, i](const QString &title) {
            Q_Q(HomeMenuFrm);
            q->handleCardClicked(i, title);
        });
    }
}

// HomeMenuFrm implementation
HomeMenuFrm::HomeMenuFrm(QWidget *parent)
    : SettingBaseFrm(parent)
    , d_ptr(new HomeMenuFrmPrivate(this))
{
    // Ensure the main widget uses full available space
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

HomeMenuFrm::~HomeMenuFrm()
{
}

void HomeMenuFrm::handleCardClicked(int cardIndex, const QString &title)
{
#ifdef SCREENCAPTURE
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");   
#endif

    switch(cardIndex) {
        case 0: // Person Managing
            slotManagingPeopleClicked();
            break;
        case 1: // Records Management
            slotRecordsManagementClicked();
            break;
        case 2: // Network Setup
            slotNetworkSetupClicked();
            break;
        case 3: // Host Setup
            slotSrvSetupClicked();
            break;
        case 4: // Identify Setup
            slotIdentifySetupClicked();
            break;
        case 5: // System Setup
            slotSysSetupClicked();
            break;
    }
}

void HomeMenuFrm::slotManagingPeopleClicked()
{
    emit sigShowFrm(QObject::tr("PersonManaging"));
}

void HomeMenuFrm::slotRecordsManagementClicked()
{
    emit sigShowFrm(QObject::tr("RecordsManagement"));
}

void HomeMenuFrm::slotNetworkSetupClicked()
{
    emit sigShowFrm(QObject::tr("NetworkSetup"));
}

void HomeMenuFrm::slotSrvSetupClicked()
{
    PasswordDialog pwdDialog(this);
    pwdDialog.setModal(true);
    pwdDialog.setFocus();

    if (pwdDialog.exec() == QDialog::Accepted) {
        emit sigShowFrm(QObject::tr("HostSetup"));
    }
}

void HomeMenuFrm::slotSysSetupClicked()
{
    emit sigShowFrm(QObject::tr("SystemSetup"));
}

void HomeMenuFrm::slotIdentifySetupClicked()
{
    emit sigShowFrm(QObject::tr("IdentifySetup"));
}

#ifdef SCREENCAPTURE
void HomeMenuFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");   
}
#endif

#include "HomeMenuFrm.moc" // For HomeMenuCardWidget Q_OBJECT