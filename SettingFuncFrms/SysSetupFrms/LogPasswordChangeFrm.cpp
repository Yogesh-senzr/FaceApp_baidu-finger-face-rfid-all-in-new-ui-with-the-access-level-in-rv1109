#include "LogPasswordChangeFrm.h"
#include "Config/ReadConfig.h"

#include <QLabel>
#include <QLineEdit>
#include <QButtonGroup>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>

class LogPasswordChangeFrmPrivate
{
    Q_DECLARE_PUBLIC(LogPasswordChangeFrm)
public:
    LogPasswordChangeFrmPrivate(LogPasswordChangeFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
    void ApplyCardStyles();
private:
    // Header
    QWidget *m_pHeaderWidget;
    QLabel *m_pTitleLabel;
    
    // Input fields
    QWidget *m_pInputGroup;
    QLineEdit *m_pOldPassword;
    QLineEdit *m_pNewPassword;
    QLineEdit *m_pConfirmNewPassword;
    QLabel *m_pHintLabel;
    
    // Action buttons
    QPushButton *m_pConfirmButton;
    QPushButton *m_pCancelButton;
private:
    LogPasswordChangeFrm *const q_ptr;
};

LogPasswordChangeFrmPrivate::LogPasswordChangeFrmPrivate(LogPasswordChangeFrm *dd)
    : q_ptr(dd)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
    this->ApplyCardStyles();
}

LogPasswordChangeFrm::LogPasswordChangeFrm(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , d_ptr(new LogPasswordChangeFrmPrivate(this))
{

}

LogPasswordChangeFrm::~LogPasswordChangeFrm()
{

}

void LogPasswordChangeFrmPrivate::InitUI()
{
    q_func()->setObjectName("LogPasswordChangeFrm");
    q_func()->setFixedSize(500, 450);
    
    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(q_func());
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Header section
    m_pHeaderWidget = new QWidget;
    m_pHeaderWidget->setObjectName("DialogHeader");
    m_pHeaderWidget->setFixedHeight(70);
    
    QHBoxLayout *headerLayout = new QHBoxLayout(m_pHeaderWidget);
    headerLayout->setContentsMargins(20, 0, 20, 0);
    
    m_pTitleLabel = new QLabel(QObject::tr("Change Password"));
    m_pTitleLabel->setObjectName("DialogTitle");
    m_pTitleLabel->setAlignment(Qt::AlignCenter);
    
    headerLayout->addWidget(m_pTitleLabel);
    
    // Content section
    QWidget *contentWidget = new QWidget;
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(25, 25, 25, 25);
    contentLayout->setSpacing(25);
    
    // Input group
    m_pInputGroup = new QWidget;
    QVBoxLayout *inputLayout = new QVBoxLayout(m_pInputGroup);
    inputLayout->setSpacing(10);
    
    m_pOldPassword = new QLineEdit;
    m_pOldPassword->setObjectName("InputField");
    m_pOldPassword->setPlaceholderText(QObject::tr("Old Password"));
    m_pOldPassword->setEchoMode(QLineEdit::Password);
    m_pOldPassword->setContextMenuPolicy(Qt::NoContextMenu);
    
    m_pNewPassword = new QLineEdit;
    m_pNewPassword->setObjectName("InputField");
    m_pNewPassword->setPlaceholderText(QObject::tr("New Password"));
    m_pNewPassword->setEchoMode(QLineEdit::Password);
    m_pNewPassword->setContextMenuPolicy(Qt::NoContextMenu);
    
    m_pConfirmNewPassword = new QLineEdit;
    m_pConfirmNewPassword->setObjectName("InputField");
    m_pConfirmNewPassword->setPlaceholderText(QObject::tr("Confirm New Password"));
    m_pConfirmNewPassword->setEchoMode(QLineEdit::Password);
    m_pConfirmNewPassword->setContextMenuPolicy(Qt::NoContextMenu);
    
    m_pHintLabel = new QLabel;
    m_pHintLabel->setObjectName("ErrorMessage");
    m_pHintLabel->setVisible(false);
    
    inputLayout->addWidget(m_pOldPassword);
    inputLayout->addWidget(m_pNewPassword);
    inputLayout->addWidget(m_pConfirmNewPassword);
    inputLayout->addWidget(m_pHintLabel);
    
    // Action buttons
    QWidget *actionsWidget = new QWidget;
    QHBoxLayout *actionsLayout = new QHBoxLayout(actionsWidget);
    actionsLayout->setContentsMargins(0, 20, 0, 0);
    actionsLayout->setSpacing(15);
    actionsLayout->setAlignment(Qt::AlignCenter);
    
    m_pCancelButton = new QPushButton(QObject::tr("Cancel"));
    m_pCancelButton->setObjectName("CancelButton");
    
    m_pConfirmButton = new QPushButton(QObject::tr("OK"));
    m_pConfirmButton->setObjectName("ConfirmButton");
    
    actionsLayout->addWidget(m_pCancelButton);
    actionsLayout->addWidget(m_pConfirmButton);
    
    // Add separator line
    QFrame *separator = new QFrame;
    separator->setFrameShape(QFrame::HLine);
    separator->setObjectName("DialogSeparator");
    
    contentLayout->addWidget(m_pInputGroup);
    contentLayout->addWidget(separator);
    contentLayout->addWidget(actionsWidget);
    
    // Assemble main layout
    mainLayout->addWidget(m_pHeaderWidget);
    mainLayout->addWidget(contentWidget, 1);
}

void LogPasswordChangeFrmPrivate::ApplyCardStyles()
{
    q_func()->setStyleSheet(
        "QDialog#LogPasswordChangeFrm {"
        "    background: white;"
        "    border-radius: 15px;"
        "}"
        
        // Header styles
        "QWidget#DialogHeader {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "                stop:0 #2196F3, stop:1 #1976D2);"
        "    border-top-left-radius: 15px;"
        "    border-top-right-radius: 15px;"
        "}"
        "QLabel#DialogTitle {"
        "    color: white;"
        "    font-size: 20px;"
        "    font-weight: 600;"
        "}"
        
        // Input field styles
        "QLineEdit#InputField {"
        "    padding: 12px 16px;"
        "    border: 2px solid #e1e5e9;"
        "    border-radius: 8px;"
        "    font-size: 16px;"
        "    background: white;"
        "    margin-bottom: 10px;"
        "}"
        "QLineEdit#InputField:focus {"
        "    border-color: #2196F3;"
        "}"
        
        // Error message
        "QLabel#ErrorMessage {"
        "    color: #dc3545;"
        "    font-size: 14px;"
        "    margin-top: 5px;"
        "}"
        
        // Button styles
        "QPushButton#ConfirmButton {"
        "    background: #2196F3;"
        "    color: white;"
        "    border: none;"
        "    padding: 12px 24px;"
        "    border-radius: 8px;"
        "    font-size: 16px;"
        "    font-weight: 600;"
        "    min-width: 100px;"
        "}"
        "QPushButton#ConfirmButton:hover {"
        "    background: #1976D2;"
        "}"
        "QPushButton#CancelButton {"
        "    background: #6c757d;"
        "    color: white;"
        "    border: none;"
        "    padding: 12px 24px;"
        "    border-radius: 8px;"
        "    font-size: 16px;"
        "    font-weight: 600;"
        "    min-width: 100px;"
        "}"
        "QPushButton#CancelButton:hover {"
        "    background: #545b62;"
        "}"
        
        // Separator
        "QFrame#DialogSeparator {"
        "    color: #e9ecef;"
        "    background-color: #e9ecef;"
        "}"
    );
}

void LogPasswordChangeFrmPrivate::InitData()
{
    // No additional data initialization needed
}

void LogPasswordChangeFrmPrivate::InitConnect()
{
    QObject::connect(m_pConfirmButton, &QPushButton::clicked, [this] {
        m_pHintLabel->setVisible(false);
        
        if (m_pNewPassword->text() == m_pConfirmNewPassword->text()) {
            if (m_pOldPassword->text() == ReadConfig::GetInstance()->getLoginPassword()) {
                q_func()->done(0);
            } else {
                m_pHintLabel->setText(QObject::tr("Wrong old password"));
                m_pHintLabel->setVisible(true);
            }
        } else {
            m_pHintLabel->setText(QObject::tr("Passwords do not match"));
            m_pHintLabel->setVisible(true);
        }
    });
    
    QObject::connect(m_pCancelButton, &QPushButton::clicked, [this] {
        q_func()->done(1);
    });
}

QString LogPasswordChangeFrm::getOldPasw()
{
    return d_func()->m_pOldPassword->text();
}

QString LogPasswordChangeFrm::getNewPasw()
{
    return d_func()->m_pNewPassword->text();
}

QString LogPasswordChangeFrm::getConfirmNewPasw()
{
    return d_func()->m_pConfirmNewPassword->text();
}

void LogPasswordChangeFrm::showEvent(QShowEvent *event)
{
    Q_D(LogPasswordChangeFrm);
    d->m_pHintLabel->setVisible(false);
    d->m_pNewPassword->clear();
    d->m_pConfirmNewPassword->clear();
    d->m_pOldPassword->clear();
    QDialog::showEvent(event);
}

#ifdef SCREENCAPTURE
void LogPasswordChangeFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    grab().save(QString("/mnt/user/screenshot/%1.png").arg(this->metaObject()->className()),"png");    
}
#endif