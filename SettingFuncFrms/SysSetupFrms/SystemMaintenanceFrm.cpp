#include "SystemMaintenanceFrm.h"
#include "Config/ReadConfig.h"
#include <QtWidgets/QLabel>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QFrame>
#include <QtWidgets/QTimeEdit>
#include <QtCore/QTime>

// Custom Option Card for System Maintenance
class MaintenanceOptionCard : public QFrame
{
public:
    MaintenanceOptionCard(const QString &title, QWidget *control, QWidget *parent = nullptr)
        : QFrame(parent)
    {
        setObjectName("MaintenanceOptionCard");
        setupUI(title, control);
    }

private:
    void setupUI(const QString &title, QWidget *control) {
        setFixedHeight(85);
        
        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->setContentsMargins(25, 18, 25, 18);
        layout->setSpacing(20);
        
        QLabel *titleLabel = new QLabel(title);
        titleLabel->setObjectName("MaintenanceOptionTitle");
        
        layout->addWidget(titleLabel);
        layout->addStretch();
        layout->addWidget(control);
    }
};

class SystemMaintenanceFrmPrivate
{
    Q_DECLARE_PUBLIC(SystemMaintenanceFrm)
public:
    SystemMaintenanceFrmPrivate(SystemMaintenanceFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
    void ApplyCardStyles();
private:
    // Header
    QWidget *m_pHeaderWidget;
    QLabel *m_pTitleLabel;
    
    // Controls
    QButtonGroup *m_pAutoRebootGroup;
    QTimeEdit *m_pTimeEdit;
    
    // Action buttons
    QPushButton *m_pConfirmButton;
    QPushButton *m_pCancelButton;
private:
    SystemMaintenanceFrm *const q_ptr;
};

SystemMaintenanceFrmPrivate::SystemMaintenanceFrmPrivate(SystemMaintenanceFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
    this->ApplyCardStyles();
}

SystemMaintenanceFrm::SystemMaintenanceFrm(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , d_ptr(new SystemMaintenanceFrmPrivate(this))
{
}

SystemMaintenanceFrm::~SystemMaintenanceFrm()
{
}

void SystemMaintenanceFrmPrivate::InitUI()
{
    q_func()->setObjectName("SystemMaintenanceFrm");
    
    // Increase dialog size from 450x350 to larger dimensions
    q_func()->setFixedSize(550, 420); // Increased width by 100px, height by 70px
    
    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(q_func());
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Header section - increase height proportionally
    m_pHeaderWidget = new QWidget;
    m_pHeaderWidget->setObjectName("MaintenanceHeader");
    m_pHeaderWidget->setFixedHeight(80); // Increased from 70 to 80
    
    QHBoxLayout *headerLayout = new QHBoxLayout(m_pHeaderWidget);
    headerLayout->setContentsMargins(25, 0, 25, 0); // Increased margins from 20 to 25
    
    m_pTitleLabel = new QLabel(QObject::tr("SystemMaintenance"));
    m_pTitleLabel->setObjectName("MaintenanceTitle");
    m_pTitleLabel->setAlignment(Qt::AlignCenter);
    
    headerLayout->addWidget(m_pTitleLabel);
    
    // Content section - increase margins for better spacing
    QWidget *contentWidget = new QWidget;
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(35, 30, 35, 30); // Increased from (25, 25, 25, 25)
    contentLayout->setSpacing(25); // Increased from 20
    
    // Create controls
    m_pAutoRebootGroup = new QButtonGroup;
    
    // Create radio button container
    QWidget *radioContainer = new QWidget;
    QHBoxLayout *radioLayout = new QHBoxLayout(radioContainer);
    radioLayout->setSpacing(25); // Increased from 20
    radioLayout->addStretch();
    
    QRadioButton *noneRadio = new QRadioButton(QObject::tr("None"));
    QRadioButton *dayRadio = new QRadioButton(QObject::tr("Day"));
    
    noneRadio->setObjectName("MaintenanceRadio");
    dayRadio->setObjectName("MaintenanceRadio");
    
    m_pAutoRebootGroup->addButton(noneRadio, 0);
    m_pAutoRebootGroup->addButton(dayRadio, 1);
    
    radioLayout->addWidget(noneRadio);
    radioLayout->addWidget(dayRadio);
    radioLayout->addStretch();
    
    m_pTimeEdit = new QTimeEdit;
    m_pTimeEdit->setObjectName("MaintenanceTimeEdit");
    m_pTimeEdit->setContextMenuPolicy(Qt::NoContextMenu);
    
    // Create option cards with increased height
    MaintenanceOptionCard *rebootCard = new MaintenanceOptionCard(QObject::tr("AutomaticReboot"), radioContainer);
    MaintenanceOptionCard *timeCard = new MaintenanceOptionCard(QObject::tr("RebootTime"), m_pTimeEdit);
    
    // Action buttons
    QWidget *actionsWidget = new QWidget;
    actionsWidget->setFixedHeight(80); // Set fixed height to ensure buttons are visible
    QHBoxLayout *actionsLayout = new QHBoxLayout(actionsWidget);
    actionsLayout->setContentsMargins(15, 15, 15, 15); // Reduced top margin from 25 to 15
    actionsLayout->setSpacing(20);
    
    m_pConfirmButton = new QPushButton(QObject::tr("Ok"));
    m_pConfirmButton->setObjectName("MaintenanceConfirmButton");
    m_pConfirmButton->setMinimumHeight(50); // Ensure minimum button height
    
    m_pCancelButton = new QPushButton(QObject::tr("Cancel"));
    m_pCancelButton->setObjectName("MaintenanceCancelButton");
    m_pCancelButton->setMinimumHeight(50); // Ensure minimum button height
    
    actionsLayout->addWidget(m_pConfirmButton);
    actionsLayout->addWidget(m_pCancelButton);
    
    contentLayout->addWidget(rebootCard);
    contentLayout->addWidget(timeCard);
    contentLayout->addStretch(); // Use stretch instead of fixed spacing to push buttons down
    contentLayout->addWidget(actionsWidget);
    
    // Assemble main layout
    mainLayout->addWidget(m_pHeaderWidget);
    mainLayout->addWidget(contentWidget, 1);
}

void SystemMaintenanceFrmPrivate::ApplyCardStyles()
{
    q_func()->setStyleSheet(
        "QDialog#SystemMaintenanceFrm {"
        "    background: white;"
        "    border-radius: 15px;"
        "}"
        
        // Header styles
        "QWidget#MaintenanceHeader {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "                stop:0 #2196F3, stop:1 #1976D2);"
        "    border-top-left-radius: 15px;"
        "    border-top-right-radius: 15px;"
        "}"
        "QLabel#MaintenanceTitle {"
        "    color: white;"
        "    font-size: 20px;"
        "    font-weight: 600;"
        "}"
        
        // Option card styles
        "MaintenanceOptionCard#MaintenanceOptionCard {"
        "    background: #f8f9fa;"
        "    border: 2px solid #e1e5e9;"
        "    border-radius: 12px;"
        "}"
        
        "QLabel#MaintenanceOptionTitle {"
        "    font-size: 16px;"
        "    font-weight: 600;"
        "    color: #2c3e50;"
        "}"
        
        // Radio button styles
        "QRadioButton#MaintenanceRadio {"
        "    font-size: 14px;"
        "    color: #2c3e50;"
        "    spacing: 8px;"
        "}"
        "QRadioButton#MaintenanceRadio::indicator {"
        "    width: 18px;"
        "    height: 18px;"
        "}"
        "QRadioButton#MaintenanceRadio::indicator:unchecked {"
        "    border: 2px solid #dee2e6;"
        "    border-radius: 9px;"
        "    background: white;"
        "}"
        "QRadioButton#MaintenanceRadio::indicator:checked {"
        "    border: 2px solid #2196F3;"
        "    border-radius: 9px;"
        "    background: #2196F3;"
        "}"
        
        // Time edit styles
        "QTimeEdit#MaintenanceTimeEdit {"
        "    padding: 8px 12px;"
        "    border: 2px solid #e1e5e9;"
        "    border-radius: 8px;"
        "    font-size: 14px;"
        "    width: 198px;"
        "    height: 52px;"
        "}"
        
        // Button styles
        "QPushButton#MaintenanceConfirmButton {"
        "    background: #2196F3;"
        "    color: white;"
        "    border: none;"
        "    padding: 12px 24px;"
        "    border-radius: 8px;"
        "    font-size: 16px;"
        "    font-weight: 600;"
        "    height: 64px;"
        "}"
        "QPushButton#MaintenanceConfirmButton:hover {"
        "    background: #1976D2;"
        "}"
        "QPushButton#MaintenanceCancelButton {"
        "    background: #6c757d;"
        "    color: white;"
        "    border: none;"
        "    padding: 12px 24px;"
        "    border-radius: 8px;"
        "    font-size: 16px;"
        "    font-weight: 600;"
        "    height: 64px;"
        "}"
        "QPushButton#MaintenanceCancelButton:hover {"
        "    background: #545b62;"
        "}"
    );
}

void SystemMaintenanceFrmPrivate::InitData()
{
    m_pAutoRebootGroup->button(0)->setChecked(true);
    m_pTimeEdit->setDisplayFormat("hh:mm:ss");
    m_pTimeEdit->setTime(QTime::currentTime());
}

void SystemMaintenanceFrmPrivate::InitConnect()
{
    QObject::connect(m_pConfirmButton, &QPushButton::clicked, [this] {
        ReadConfig::GetInstance()->setMaintenance_boot(m_pAutoRebootGroup->checkedId());
        printf("%s %s[%d]  \n", __FILE__, __FUNCTION__, __LINE__); 
        ReadConfig::GetInstance()->setMaintenance_bootTimer(m_pTimeEdit->text());
        ReadConfig::GetInstance()->setSaveConfig();
        q_func()->done(0); 
    });
    QObject::connect(m_pCancelButton, &QPushButton::clicked, [this] {
        q_func()->done(1);
    });
}

void SystemMaintenanceFrm::setData(const int &mode, const QString &time)
{
    Q_D(SystemMaintenanceFrm);
    int index = mode > 1 ? 1: mode;
    d->m_pAutoRebootGroup->button(index)->setChecked(true);
    d->m_pTimeEdit->setTime(QTime::fromString(time));
    ReadConfig::GetInstance()->setMaintenance_boot(mode);
    printf("%s %s[%d]  \n", __FILE__, __FUNCTION__, __LINE__); 
    ReadConfig::GetInstance()->setMaintenance_bootTimer(time);
}

int SystemMaintenanceFrm::getTimeMode() const
{
    return d_func()->m_pAutoRebootGroup->checkedId();
}

QString SystemMaintenanceFrm::getTime() const
{
    return d_func()->m_pTimeEdit->text();
}

void SystemMaintenanceFrm::slotConfirmButton()
{
    Q_D(SystemMaintenanceFrm);
    int boot = 1;
    for(int i = 0 ;i < 2; i++) {
        if(d->m_pAutoRebootGroup->button(i)->isChecked()) {
            boot = i;
            break;
        }
    }

    ReadConfig::GetInstance()->setMaintenance_boot(boot);
    printf("%s %s[%d]  \n", __FILE__, __FUNCTION__, __LINE__); 
    ReadConfig::GetInstance()->setMaintenance_bootTimer(d->m_pTimeEdit->text());
    ReadConfig::GetInstance()->setSaveConfig();
}

#ifdef SCREENCAPTURE
void SystemMaintenanceFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif
