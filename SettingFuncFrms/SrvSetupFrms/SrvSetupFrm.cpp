// Modified SrvSetupFrm.cpp - Card-based UI with ReadConfig integration

#include "SrvSetupFrm.h"
#include "Config/ReadConfig.h"
#include "MessageHandler/Log.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QScrollArea>
#include <QSettings>
#include <QTextCodec>
#include <QMessageBox>

// Custom Server Configuration Card Widget
class ServerConfigCardWidget : public QFrame
{
    Q_OBJECT
public:
    ServerConfigCardWidget(const QString &title, bool showPassword = false, QWidget *parent = nullptr)
        : QFrame(parent), m_title(title), m_showPassword(showPassword)
    {
        setObjectName("ServerConfigCardWidget");
        setupUI(title, showPassword);
    }
    
    QString getTitle() const { return m_title; }
    QString getAddressValue() const { return m_addressEdit ? m_addressEdit->text() : ""; }
    QString getPasswordValue() const { return m_passwordEdit ? m_passwordEdit->text() : ""; }
    
    void setAddressValue(const QString &value)
    {
        if (m_addressEdit) {
            m_addressEdit->setText(value);
        }
    }
    
    void setPasswordValue(const QString &value)
    {
        if (m_passwordEdit) {
            m_passwordEdit->setText(value);
        }
    }

private:
    void setupUI(const QString &title, bool showPassword)
    {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(20, 16, 20, 16);
        mainLayout->setSpacing(12);
        
        // Header with icon and title
        QHBoxLayout *headerLayout = new QHBoxLayout;
        headerLayout->setSpacing(12);
        
        // Icon container
        QFrame *iconFrame = new QFrame;
        iconFrame->setFixedSize(32, 32);
        iconFrame->setObjectName(getIconObjectName(title));
        
        QHBoxLayout *iconLayout = new QHBoxLayout(iconFrame);
        iconLayout->setContentsMargins(0, 0, 0, 0);
        
        QLabel *iconLabel = new QLabel;
        iconLabel->setText(getIconText(title));
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setObjectName("ServerConfigIconLabel");
        iconLayout->addWidget(iconLabel);
        
        // Title
        QLabel *titleLabel = new QLabel(title);
        titleLabel->setObjectName("ServerConfigTitleLabel");
        
        // Editable indicator
        QLabel *editableLabel = new QLabel("(Editable)");
        editableLabel->setObjectName("ServerConfigEditableLabel");
        
        headerLayout->addWidget(iconFrame);
        headerLayout->addWidget(titleLabel);
        headerLayout->addWidget(editableLabel);
        headerLayout->addStretch();
        
        // Address input field
        m_addressEdit = new QLineEdit;
        m_addressEdit->setObjectName("ServerConfigEdit");
        m_addressEdit->setPlaceholderText("Enter server URL (https://...)");
        m_addressEdit->setContextMenuPolicy(Qt::NoContextMenu);
        
        mainLayout->addLayout(headerLayout);
        mainLayout->addWidget(m_addressEdit);
        
        // Password field (if needed)
        if (showPassword) {
            QLabel *passwordLabel = new QLabel("Password:");
            passwordLabel->setObjectName("ServerConfigPasswordLabel");
            
            m_passwordEdit = new QLineEdit;
            m_passwordEdit->setObjectName("ServerConfigPasswordEdit");
            m_passwordEdit->setPlaceholderText("Enter password");
            m_passwordEdit->setEchoMode(QLineEdit::Password);
            m_passwordEdit->setContextMenuPolicy(Qt::NoContextMenu);
            
            mainLayout->addSpacing(8);
            mainLayout->addWidget(passwordLabel);
            mainLayout->addWidget(m_passwordEdit);
        }
    }
    
    QString getIconObjectName(const QString &title)
    {
        if (title.contains("HTTP") && title.contains("Server"))
            return "MainServerIconFrame";
        else if (title.contains("Person") && title.contains("Record"))
            return "PersonRecordIconFrame";
        else if (title.contains("Registration"))
            return "RegistrationIconFrame";
        else if (title.contains("Sync"))
            return "SyncIconFrame";
        else if (title.contains("Detail"))
            return "DetailIconFrame";
        return "DefaultServerIconFrame";
    }
    
    QString getIconText(const QString &title)
    {
        if (title.contains("HTTP") && title.contains("Server"))
            return "🌐";
        else if (title.contains("Person") && title.contains("Record"))
            return "📊";
        else if (title.contains("Registration"))
            return "👤";
        else if (title.contains("Sync"))
            return "🔄";
        else if (title.contains("Detail"))
            return "ℹ️";
        return "🖥️";
    }

private:
    QString m_title;
    bool m_showPassword;
    QLineEdit *m_addressEdit;
    QLineEdit *m_passwordEdit = nullptr;
};

class SrvSetupFrmPrivate
{
    Q_DECLARE_PUBLIC(SrvSetupFrm)
public:
    SrvSetupFrmPrivate(SrvSetupFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
private:
    QVBoxLayout *m_pMainLayout;
    QScrollArea *m_pScrollArea;
    QWidget *m_pContentWidget;
    QVBoxLayout *m_pContentLayout;
    
    // Configuration cards
    ServerConfigCardWidget *m_pHttpServerCard;
    ServerConfigCardWidget *m_pPostPersonRecordCard;
    ServerConfigCardWidget *m_pNewRegisteredPersonCard;
    ServerConfigCardWidget *m_pSyncUsersCard;
    ServerConfigCardWidget *m_pUserDetailCard;
    
    QPushButton *m_pConfirmButton;
private:
    SrvSetupFrm *const q_ptr;
};

SrvSetupFrmPrivate::SrvSetupFrmPrivate(SrvSetupFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
}

void SrvSetupFrmPrivate::InitUI()
{
    Q_Q(SrvSetupFrm);
    
    // Set main widget object name for styling
    q->setObjectName("SrvSetupMainWidget");
    
    // Main layout
    m_pMainLayout = new QVBoxLayout(q);
    m_pMainLayout->setContentsMargins(30, 20, 30, 20);
    m_pMainLayout->setSpacing(20);
    
    // Scroll area for cards
    m_pScrollArea = new QScrollArea;
    m_pScrollArea->setWidgetResizable(true);
    m_pScrollArea->setFrameShape(QFrame::NoFrame);
    m_pScrollArea->setObjectName("SrvSetupScrollArea");
    
    m_pContentWidget = new QWidget;
    m_pContentWidget->setObjectName("SrvSetupContentWidget");
    m_pContentLayout = new QVBoxLayout(m_pContentWidget);
    m_pContentLayout->setContentsMargins(0, 0, 0, 0);
    m_pContentLayout->setSpacing(12);
    
    m_pScrollArea->setWidget(m_pContentWidget);
    
    // Create configuration cards
    m_pHttpServerCard = new ServerConfigCardWidget(
        QObject::tr("HTTPServerAddress"), 
        false  // No password field - URL only
    );
    
    m_pPostPersonRecordCard = new ServerConfigCardWidget(
        QObject::tr("PostPersonRecordHttpServerAddress"),
        false  // No password field - URL only
    );
    
    m_pNewRegisteredPersonCard = new ServerConfigCardWidget(
        QObject::tr("NewRegisteredPersonHttpServerAddress"),
        false  // No password field - URL only
    );
    
    m_pSyncUsersCard = new ServerConfigCardWidget(
        QObject::tr("SyncUsersHttpServerAddress"),
        false  // No password field - URL only
    );
    
    m_pUserDetailCard = new ServerConfigCardWidget(
        QObject::tr("UserDetailHttpServerAddress"),
        false  // No password field - URL only
    );
    
    // Add cards to content layout
    m_pContentLayout->addWidget(m_pHttpServerCard);
    m_pContentLayout->addWidget(m_pPostPersonRecordCard);
    m_pContentLayout->addWidget(m_pNewRegisteredPersonCard);
    m_pContentLayout->addWidget(m_pSyncUsersCard);
    m_pContentLayout->addWidget(m_pUserDetailCard);
    m_pContentLayout->addStretch();
    
    // Confirm button
    m_pConfirmButton = new QPushButton;
    m_pConfirmButton->setObjectName("SrvSetupConfirmButton");
    
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_pConfirmButton);
    buttonLayout->addStretch();
    
    // Add to main layout
    m_pMainLayout->addWidget(m_pScrollArea);
    m_pMainLayout->addLayout(buttonLayout);
}

void SrvSetupFrmPrivate::InitData()
{
    // Setup confirm button
    m_pConfirmButton->setFixedSize(282, 62);
    m_pConfirmButton->setText(QObject::tr("Save"));
}

void SrvSetupFrmPrivate::InitConnect()
{
    QObject::connect(m_pConfirmButton, &QPushButton::clicked, q_func(), &SrvSetupFrm::slotConfirmButton);
}

// SrvSetupFrm implementation
SrvSetupFrm::SrvSetupFrm(QWidget *parent)
    : SettingBaseFrm(parent)
    , d_ptr(new SrvSetupFrmPrivate(this))
{
}

SrvSetupFrm::~SrvSetupFrm()
{
}

void SrvSetupFrm::setEnter()
{
    Q_D(SrvSetupFrm);

    // Load addresses from ReadConfig (user-configured values)
    d->m_pHttpServerCard->setAddressValue(
        ReadConfig::GetInstance()->getSrv_Manager_Address()
    );
    d->m_pHttpServerCard->setPasswordValue(
        ReadConfig::GetInstance()->getSrv_Manager_Password()
    );
    
    d->m_pPostPersonRecordCard->setAddressValue(
        ReadConfig::GetInstance()->getPost_PersonRecord_Address()
    );
    d->m_pPostPersonRecordCard->setPasswordValue(
        ReadConfig::GetInstance()->getPost_PersonRecord_Password()
    );
    
    d->m_pNewRegisteredPersonCard->setAddressValue(
        ReadConfig::GetInstance()->getPerson_Registration_Address()
    );
    d->m_pNewRegisteredPersonCard->setPasswordValue(
        ReadConfig::GetInstance()->getPerson_Registration_Password()
    );
    
    d->m_pSyncUsersCard->setAddressValue(
        ReadConfig::GetInstance()->getSyncUsersAddress()
    );
    d->m_pSyncUsersCard->setPasswordValue(
        ReadConfig::GetInstance()->getSyncUsersPassword()
    );
    
    d->m_pUserDetailCard->setAddressValue(
        ReadConfig::GetInstance()->getUserDetailAddress()
    );
    d->m_pUserDetailCard->setPasswordValue(
        ReadConfig::GetInstance()->getUserDetailPassword()
    );

    LogD("Server Setup page entered - loaded current configuration from ReadConfig\n");
}

void SrvSetupFrm::slotConfirmButton()
{
    Q_D(SrvSetupFrm);
    
    // Save BOTH addresses and passwords to ReadConfig
    // These values will be used throughout the application
    
    ReadConfig::GetInstance()->setSrv_Manager_Address(
        d->m_pHttpServerCard->getAddressValue()
    );
    ReadConfig::GetInstance()->setSrv_Manager_Password(
        d->m_pHttpServerCard->getPasswordValue()
    );
    
    ReadConfig::GetInstance()->setPost_PersonRecord_Address(
        d->m_pPostPersonRecordCard->getAddressValue()
    );
    ReadConfig::GetInstance()->setPost_PersonRecord_Password(
        d->m_pPostPersonRecordCard->getPasswordValue()
    );
    
    ReadConfig::GetInstance()->setPerson_Registration_Address(
        d->m_pNewRegisteredPersonCard->getAddressValue()
    );
    ReadConfig::GetInstance()->setPerson_Registration_Password(
        d->m_pNewRegisteredPersonCard->getPasswordValue()
    );
    
    ReadConfig::GetInstance()->setSyncUsersAddress(
        d->m_pSyncUsersCard->getAddressValue()
    );
    ReadConfig::GetInstance()->setSyncUsersPassword(
        d->m_pSyncUsersCard->getPasswordValue()
    );
    
    ReadConfig::GetInstance()->setUserDetailAddress(
        d->m_pUserDetailCard->getAddressValue()
    );
    ReadConfig::GetInstance()->setUserDetailPassword(
        d->m_pUserDetailCard->getPasswordValue()
    );
    
    // Log the saved configuration
    LogD("Configuration saved - user-configured addresses and passwords:\n");
    LogD("  Main Server: %s\n", d->m_pHttpServerCard->getAddressValue().toStdString().c_str());
    LogD("  Person Record: %s\n", d->m_pPostPersonRecordCard->getAddressValue().toStdString().c_str());
    LogD("  Person Registration: %s\n", d->m_pNewRegisteredPersonCard->getAddressValue().toStdString().c_str());
    LogD("  Sync Users: %s\n", d->m_pSyncUsersCard->getAddressValue().toStdString().c_str());
    LogD("  User Detail: %s\n", d->m_pUserDetailCard->getAddressValue().toStdString().c_str());
    
    // Save configuration to persistent storage
    ReadConfig::GetInstance()->setSaveConfig();
    
    // Show success message (optional)
    QMessageBox::information(this, 
                           tr("Success"),
                           tr("Server configuration saved successfully!\n\nThe new URLs will be used for all server communications."));
}

void SrvSetupFrm::slotIemClicked(QListWidgetItem */*item*/)
{
}

#ifdef SCREENCAPTURE
void SrvSetupFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif

#include "SrvSetupFrm.moc" // For ServerConfigCardWidget Q_OBJECT