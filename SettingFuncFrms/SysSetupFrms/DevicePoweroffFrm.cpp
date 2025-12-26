#include "DevicePoweroffFrm.h"
#include "Helper/myhelper.h"

#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>

class DevicePoweroffFrmPrivate
{
    Q_DECLARE_PUBLIC(DevicePoweroffFrm)
public:
    DevicePoweroffFrmPrivate(DevicePoweroffFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
    void ApplyCardStyles();
private:
    // Header
    QWidget *m_pHeaderWidget;
    QLabel *m_pTitleLabel;
    
    // Content
    QFrame *m_pContentCard;
    QLabel *m_pHintLabel;
    
    // Action buttons
    QPushButton *m_pRebootBtn;
    QPushButton *m_pCancelBtn;
private:
    DevicePoweroffFrm *const q_ptr;
};

DevicePoweroffFrmPrivate::DevicePoweroffFrmPrivate(DevicePoweroffFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
    this->ApplyCardStyles();
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
    q_func()->setObjectName("DevicePoweroffFrm");
    q_func()->setFixedSize(500, 350); // INCREASED SIZE: from 400x250 to 500x350
    
    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(q_func());
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Header section - INCREASED HEIGHT
    m_pHeaderWidget = new QWidget;
    m_pHeaderWidget->setObjectName("PoweroffHeader");
    m_pHeaderWidget->setFixedHeight(90); // Increased from 70 to 90
    
    QHBoxLayout *headerLayout = new QHBoxLayout(m_pHeaderWidget);
    headerLayout->setContentsMargins(20, 0, 20, 0);
    
    m_pTitleLabel = new QLabel(QObject::tr("Device Reboot"));
    m_pTitleLabel->setObjectName("PoweroffTitle");
    m_pTitleLabel->setAlignment(Qt::AlignCenter);
    
    headerLayout->addWidget(m_pTitleLabel);
    
    // Content section
    QWidget *contentWidget = new QWidget;
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(30, 30, 30, 30); // Increased margins
    contentLayout->setSpacing(25); // Increased spacing
    
    // Content card
    m_pContentCard = new QFrame;
    m_pContentCard->setObjectName("PoweroffContentCard");
    
    QVBoxLayout *cardLayout = new QVBoxLayout(m_pContentCard);
    cardLayout->setContentsMargins(30, 30, 30, 30); // Increased padding
    cardLayout->setSpacing(20); // Increased spacing
    cardLayout->setAlignment(Qt::AlignCenter);
    
    // Reboot icon - LARGER ICON
    QLabel *rebootIcon = new QLabel("↻");
    rebootIcon->setObjectName("RebootIcon");
    rebootIcon->setAlignment(Qt::AlignCenter);
    
    QLabel *messageLabel = new QLabel(QObject::tr("Device will restart. Continue?"));
    messageLabel->setObjectName("PoweroffMessage");
    messageLabel->setAlignment(Qt::AlignCenter);
    messageLabel->setWordWrap(true);
    
    m_pHintLabel = new QLabel;
    m_pHintLabel->setObjectName("PoweroffHintLabel");
    m_pHintLabel->setAlignment(Qt::AlignCenter);
    m_pHintLabel->hide();
    
    cardLayout->addWidget(rebootIcon);
    cardLayout->addWidget(messageLabel);
    cardLayout->addWidget(m_pHintLabel);
    
    // Action buttons
    QWidget *actionsWidget = new QWidget;
    QHBoxLayout *actionsLayout = new QHBoxLayout(actionsWidget);
    actionsLayout->setContentsMargins(0, 30, 0, 0); // Increased top margin
    actionsLayout->setSpacing(20); // Increased spacing
    
    m_pRebootBtn = new QPushButton(QObject::tr("Reboot"));
    m_pRebootBtn->setObjectName("PoweroffRebootButton");
    m_pRebootBtn->setMinimumSize(120, 50); // Larger button
    
    m_pCancelBtn = new QPushButton(QObject::tr("Cancel"));
    m_pCancelBtn->setObjectName("PoweroffCancelButton");
    m_pCancelBtn->setMinimumSize(120, 50); // Larger button
    
    actionsLayout->addStretch();
    actionsLayout->addWidget(m_pCancelBtn);
    actionsLayout->addWidget(m_pRebootBtn);
    actionsLayout->addStretch();
    
    contentLayout->addStretch();
    contentLayout->addWidget(m_pContentCard);
    contentLayout->addWidget(actionsWidget);
    contentLayout->addStretch();
    
    // Assemble main layout
    mainLayout->addWidget(m_pHeaderWidget);
    mainLayout->addWidget(contentWidget, 1);
}

void DevicePoweroffFrmPrivate::ApplyCardStyles()
{
    q_func()->setStyleSheet(
        "QDialog#DevicePoweroffFrm {"
        "    background: white;"
        "    border-radius: 18px;" // Slightly larger radius
        "}"
        
               // Header styles
"QWidget#PoweroffHeader {"
"    background: #ff9800;"  // Solid color instead of gradient
"    border-top-left-radius: 18px;" // Match main radius
"    border-top-right-radius: 18px;" // Match main radius
"}"
        "QLabel#PoweroffTitle {"
        "    color: white;"
        "    font-size: 24px;" // Larger font
        "    font-weight: 600;"
        "}"
        
        // Content card styles
        "QFrame#PoweroffContentCard {"
        "    background: #f8f9fa;"
        "    border: 2px solid #e1e5e9;"
        "    border-radius: 15px;" // Larger radius
        "}"
        
        "QLabel#RebootIcon {"
        "    font-size: 64px;" // Larger icon: from 48px to 64px
        "    color: #ff9800;"
        "}"
        "QLabel#PoweroffMessage {"
        "    font-size: 18px;" // Larger font: from 16px to 18px
        "    font-weight: 500;"
        "    color: #2c3e50;"
        "}"
        "QLabel#PoweroffHintLabel {"
        "    font-size: 16px;" // Larger font: from 14px to 16px
        "    color: #28a745;"
        "    font-weight: 600;"
        "}"
        
        // Button styles - LARGER BUTTONS
        "QPushButton#PoweroffRebootButton {"
        "    background: #ff9800;"
        "    color: white;"
        "    border: none;"
        "    padding: 15px 30px;" // Increased padding
        "    border-radius: 10px;" // Larger radius
        "    font-size: 18px;" // Larger font
        "    font-weight: 600;"
        "    min-width: 120px;" // Wider button
        "}"
        "QPushButton#PoweroffRebootButton:hover {"
        "    background: #f57c00;"
        "}"
        "QPushButton#PoweroffCancelButton {"
        "    background: #6c757d;"
        "    color: white;"
        "    border: none;"
        "    padding: 15px 30px;" // Increased padding
        "    border-radius: 10px;" // Larger radius
        "    font-size: 18px;" // Larger font
        "    font-weight: 600;"
        "    min-width: 120px;" // Wider button
        "}"
        "QPushButton#PoweroffCancelButton:hover {"
        "    background: #545b62;"
        "}"
    );
}

void DevicePoweroffFrmPrivate::InitData()
{
    // No additional initialization needed
}

void DevicePoweroffFrmPrivate::InitConnect()
{
    QObject::connect(m_pRebootBtn, &QPushButton::clicked, [this] {
        m_pHintLabel->show();
        m_pHintLabel->setText(QObject::tr("Rebooting"));
        myHelper::Utils_Reboot();
    });
    QObject::connect(m_pCancelBtn, &QPushButton::clicked, [this] {
       q_func()->done(0);
    });
}

#ifdef SCREENCAPTURE
void DevicePoweroffFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif