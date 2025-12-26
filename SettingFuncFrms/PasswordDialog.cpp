#include "PasswordDialog.h"
#include <QApplication>
#include <QScreen>
#include <QPainter>
#include <QDebug>
#include <QKeyEvent>
#include <QInputMethod>
#include <QGuiApplication>

PasswordDialog::PasswordDialog(QWidget *parent)
    : QDialog(parent)
    , m_correctPassword("admin123")
    , m_attempts(0)
    , m_maxAttempts(3)
    , m_fadeAnimation(nullptr)
    , m_shakeAnimation(nullptr)
    , m_opacityEffect(nullptr)
    , m_errorTimer(new QTimer(this))
{
    setWindowTitle(tr("Security Access"));
    setWindowModality(Qt::WindowModal);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    setFixedSize(400, 320);
    // Removed: setAttribute(Qt::WA_TranslucentBackground);
    
    setupUI();
    createAnimations();
    
    m_errorTimer->setSingleShot(true);
    connect(m_errorTimer, &QTimer::timeout, this, &PasswordDialog::clearErrorMessage);
    
    // Center on screen
    if (parent) {
        QRect parentGeometry = parent->geometry();
        move(parentGeometry.center() - rect().center());
    } else {
        QScreen *screen = QGuiApplication::primaryScreen();
        if (screen) {
            QRect screenGeometry = screen->geometry();
            move(screenGeometry.center() - rect().center());
        }
    }
}

PasswordDialog::~PasswordDialog()
{
}

void PasswordDialog::paintEvent(QPaintEvent *event)
{
    QDialog::paintEvent(event);
}

void PasswordDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // Main card container - now with solid background
    m_mainCard = new QFrame;
    m_mainCard->setObjectName("PasswordCardWidget");
    m_mainCard->setStyleSheet("QFrame#PasswordCardWidget {"
                             "background: white;"
                             "border-radius: 8px;"
                             "border: 1px solid #e0e0e0;"
                             "}");
    
    QVBoxLayout *cardLayout = new QVBoxLayout(m_mainCard);
    cardLayout->setContentsMargins(30, 30, 30, 30);
    cardLayout->setSpacing(25);
    
    // Header Frame
    m_headerFrame = new QFrame;
    m_headerFrame->setObjectName("PasswordHeaderFrame");
    m_headerFrame->setFixedHeight(120);
    
    QVBoxLayout *headerLayout = new QVBoxLayout(m_headerFrame);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(12);
    
    // Icon
    m_iconLabel = new QLabel;
    m_iconLabel->setObjectName("PasswordIconLabel");
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setFixedSize(60, 60);
    
    QPixmap lockIcon(":/Images/lock.png");
    if (!lockIcon.isNull()) {
        m_iconLabel->setPixmap(lockIcon.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        m_iconLabel->setText("🔒");
    }
    
    // Title
    m_titleLabel = new QLabel(tr("Security Access"));
    m_titleLabel->setObjectName("PasswordTitleLabel");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet("QLabel#PasswordTitleLabel {"
                               "font-size: 18px;"
                               "font-weight: bold;"
                               "color: #333333;"
                               "}");
    
    // Description
    m_descLabel = new QLabel(tr("Enter administrator password to continue"));
    m_descLabel->setObjectName("PasswordDescLabel");
    m_descLabel->setAlignment(Qt::AlignCenter);
    m_descLabel->setWordWrap(true);
    m_descLabel->setStyleSheet("QLabel#PasswordDescLabel {"
                              "font-size: 12px;"
                              "color: #666666;"
                              "}");
    
    headerLayout->addWidget(m_iconLabel, 0, Qt::AlignCenter);
    headerLayout->addWidget(m_titleLabel);
    headerLayout->addWidget(m_descLabel);
    headerLayout->addStretch();
    
    // Input Frame
    m_inputFrame = new QFrame;
    m_inputFrame->setObjectName("PasswordInputFrame");
    
    QVBoxLayout *inputLayout = new QVBoxLayout(m_inputFrame);
    inputLayout->setContentsMargins(0, 0, 0, 0);
    inputLayout->setSpacing(8);
    
    m_passwordEdit = new QLineEdit;
    m_passwordEdit->setObjectName("PasswordEditField");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText(tr("Enter password"));
    m_passwordEdit->setAlignment(Qt::AlignCenter);
    m_passwordEdit->setFixedHeight(45);
    m_passwordEdit->setStyleSheet("QLineEdit#PasswordEditField {"
                                 "border: 2px solid #e0e0e0;"
                                 "border-radius: 6px;"
                                 "padding: 8px;"
                                 "font-size: 14px;"
                                 "background: white;"
                                 "}");
    
    m_errorLabel = new QLabel;
    m_errorLabel->setObjectName("PasswordErrorLabel");
    m_errorLabel->setAlignment(Qt::AlignCenter);
    m_errorLabel->setWordWrap(true);
    m_errorLabel->hide();
    m_errorLabel->setStyleSheet("QLabel#PasswordErrorLabel {"
                               "color: #d32f2f;"
                               "font-size: 11px;"
                               "padding: 5px;"
                               "}");
    
    inputLayout->addWidget(m_passwordEdit);
    inputLayout->addWidget(m_errorLabel);
    
    // Button Frame
    m_buttonFrame = new QFrame;
    m_buttonFrame->setObjectName("PasswordButtonFrame");
    m_buttonFrame->setFixedHeight(60);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout(m_buttonFrame);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(15);
    
    m_cancelBtn = new QPushButton(tr("Cancel"));
    m_cancelBtn->setObjectName("PasswordCancelButton");
    m_cancelBtn->setFixedSize(120, 40);
    m_cancelBtn->setStyleSheet("QPushButton#PasswordCancelButton {"
                              "background: #f5f5f5;"
                              "border: 1px solid #e0e0e0;"
                              "border-radius: 6px;"
                              "color: #666666;"
                              "font-weight: bold;"
                              "}"
                              "QPushButton#PasswordCancelButton:hover {"
                              "background: #e8e8e8;"
                              "}"
                              "QPushButton#PasswordCancelButton:pressed {"
                              "background: #dcdcdc;"
                              "}");
    
    m_confirmBtn = new QPushButton(tr("Confirm"));
    m_confirmBtn->setObjectName("PasswordConfirmButton");
    m_confirmBtn->setFixedSize(120, 40);
    m_confirmBtn->setEnabled(false);
    m_confirmBtn->setStyleSheet("QPushButton#PasswordConfirmButton {"
                               "background: #2196F3;"
                               "border: none;"
                               "border-radius: 6px;"
                               "color: white;"
                               "font-weight: bold;"
                               "}"
                               "QPushButton#PasswordConfirmButton:hover {"
                               "background: #1976D2;"
                               "}"
                               "QPushButton#PasswordConfirmButton:pressed {"
                               "background: #0D47A1;"
                               "}"
                               "QPushButton#PasswordConfirmButton:disabled {"
                               "background: #bdbdbd;"
                               "color: #eeeeee;"
                               "}");
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_cancelBtn);
    buttonLayout->addWidget(m_confirmBtn);
    buttonLayout->addStretch();
    
    // Assemble card layout
    cardLayout->addWidget(m_headerFrame);
    cardLayout->addWidget(m_inputFrame);
    cardLayout->addWidget(m_buttonFrame);
    
    mainLayout->addWidget(m_mainCard);
    
    // Connect signals
    connect(m_passwordEdit, &QLineEdit::textChanged, this, &PasswordDialog::onPasswordChanged);
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &PasswordDialog::onAccept);
    connect(m_confirmBtn, &QPushButton::clicked, this, &PasswordDialog::onAccept);
    connect(m_cancelBtn, &QPushButton::clicked, this, &PasswordDialog::onCancel);
}

void PasswordDialog::createAnimations()
{
    m_opacityEffect = new QGraphicsOpacityEffect(this);
    m_errorLabel->setGraphicsEffect(m_opacityEffect);
    
    m_fadeAnimation = new QPropertyAnimation(m_opacityEffect, "opacity");
    m_fadeAnimation->setDuration(300);
    
    m_shakeAnimation = new QPropertyAnimation(m_mainCard, "geometry");
    m_shakeAnimation->setDuration(500);
}

void PasswordDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    
    m_attempts = 0;
    m_passwordEdit->clear();
    clearErrorMessage();
    
    m_passwordEdit->setFocus();
    m_passwordEdit->activateWindow();
}

void PasswordDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        onCancel();
        return;
    }
    
    QDialog::keyPressEvent(event);
}

QString PasswordDialog::getPassword() const
{
    return m_passwordEdit->text();
}

bool PasswordDialog::validatePassword(const QString &password)
{
    return password == m_correctPassword;
}

void PasswordDialog::onAccept()
{
    if (validatePassword(m_passwordEdit->text())) {
        accept();
    } else {
        handleFailedAttempt();
    }
}

void PasswordDialog::onCancel()
{
    reject();
}

void PasswordDialog::onPasswordChanged()
{
    clearErrorMessage();
    m_confirmBtn->setEnabled(!m_passwordEdit->text().isEmpty());
}

void PasswordDialog::handleFailedAttempt()
{
    m_attempts++;
    
    QString errorMessage;
    int remainingAttempts = m_maxAttempts - m_attempts;
    
    if (remainingAttempts > 0) {
        errorMessage = tr("Incorrect password! %1 attempts remaining.").arg(remainingAttempts);
    } else {
        errorMessage = tr("Maximum attempts exceeded. Access denied.");
        QTimer::singleShot(2000, [this]() {
            reject();
        });
    }
    
    setErrorMessage(errorMessage);
    m_passwordEdit->clear();
}

void PasswordDialog::setErrorMessage(const QString &message)
{
    m_errorLabel->setText(message);
    m_errorLabel->show();
    showErrorAnimation();
    m_errorTimer->start(3000);
}

void PasswordDialog::clearErrorMessage()
{
    m_errorLabel->hide();
    m_errorTimer->stop();
}

void PasswordDialog::showErrorAnimation()
{
    // Fade in error
    m_fadeAnimation->setStartValue(0.0);
    m_fadeAnimation->setEndValue(1.0);
    m_fadeAnimation->start();
    
    // Shake card
    QRect originalGeometry = m_mainCard->geometry();
    QRect shakeLeft = originalGeometry.translated(-10, 0);
    QRect shakeRight = originalGeometry.translated(10, 0);
    
    m_shakeAnimation->setKeyValueAt(0, originalGeometry);
    m_shakeAnimation->setKeyValueAt(0.2, shakeLeft);
    m_shakeAnimation->setKeyValueAt(0.4, shakeRight);
    m_shakeAnimation->setKeyValueAt(0.6, shakeLeft);
    m_shakeAnimation->setKeyValueAt(0.8, shakeRight);
    m_shakeAnimation->setKeyValueAt(1, originalGeometry);
    m_shakeAnimation->start();
}