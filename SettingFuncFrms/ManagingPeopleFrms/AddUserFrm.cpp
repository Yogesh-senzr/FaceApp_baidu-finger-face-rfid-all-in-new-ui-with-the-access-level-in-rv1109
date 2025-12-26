#include "AddUserFrm.h"

#include "CameraPicFrm.h"
#include "SettingFuncFrms/SettingMenuTitleFrm.h"

#include "BaiduFace/BaiduFaceManager.h"
#include "OperationTipsFrm.h"

#include "DB/RegisteredFacesDB.h"
#include "Application/FaceApp.h"

#include "SettingFuncFrms/SysSetupFrms/LanguageFrm.h"
#include "UserViewFrm.h"
#include "HttpServer/ConnHttpServerThread.h"
#include "BaiduFace/FingerprintManager.h" 

#include <QPropertyAnimation>
#include <QScreen>
#include <QApplication>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QStyleOption>
#include <QtGui/QPainter>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtCore/QFile>
#include <QtCore/QDebug>
#include <QApplication>
#include <QTransform>
#include <QSettings>
#include <QMessageBox>
#include <QTimer>
#include <QGraphicsEffect>

static PERSONS_t mPerson = {0};

class AddUserFrmPrivate
{
    Q_DECLARE_PUBLIC(AddUserFrm)
public:
    AddUserFrmPrivate(AddUserFrm *dd);
private:
    void InitUI();
    void InitData();
    void InitConnect();
    void ApplyModernStyles();
    void UpdateButtonStates();
    void ShowStatusMessage(const QString &message, const QString &type = "");
private:
    CameraPicFrm *m_pCameraPicFrm;
private:
    // Header components
    QWidget *m_pHeaderWidget;
    QPushButton *m_pBackButton;
    QLabel *m_pTitleLabel;
    
    // Form overlay components
    QWidget *m_pFormOverlay;
    QWidget *m_pFormContent;
    
    // Input components
    QWidget *m_pNameRow;
    QLabel *m_pNameLabel;
    QLineEdit *m_pNameEdit;
    
    QWidget *m_pIDCardRow;
    QLabel *m_pIDCardLabel;
    QLineEdit *m_pIDCardEdit;
    
    QWidget *m_pCardRow;
    QLabel *m_pCardLabel;
    QLineEdit *m_pCardEdit;
    
    QWidget *m_pGenderRow;
    QLabel *m_pGenderLabel;
    QComboBox *m_psexBox;
    
    // Action buttons
    QWidget *m_pButtonRow;
    QPushButton *m_pCaptureFaceButton;
    QPushButton *m_pRegFaceButton;
    QPushButton *m_pCaptureFingerButton;
    QPushButton *m_pRegFingerButton;
    QPushButton *m_pDeleteFingerButton;
    
    // Status message
    QLabel *m_pStatusLabel;
    QLabel *m_pFingerStatusLabel;
    QTimer *m_pStatusTimer;
    
    // Camera overlay for capture mode
    QWidget *m_pCaptureOverlay;
    QWidget *m_pCaptureFrame;
    QLabel *m_pCaptureInstructions;
    
private:
    QByteArray mFaceFeature;
    QByteArray mFingerTemplate;
    QString mHintText;
    QString mFingerHintText;
    bool mIsModifyMode;
    bool mHasFaceData;
private:
    AddUserFrm *const q_ptr;
};

AddUserFrmPrivate::AddUserFrmPrivate(AddUserFrm *dd)
    : q_ptr(dd), mIsModifyMode(false), mHasFaceData(false)
{
    this->InitUI();
    this->InitData();
    this->InitConnect();
    this->ApplyModernStyles();
}

AddUserFrm::AddUserFrm(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new AddUserFrmPrivate(this))
{
    d_func()->m_pCameraPicFrm = new CameraPicFrm(this);
    d_func()->m_pCameraPicFrm->setFixedSize(300, 256);
    d_func()->m_pCameraPicFrm->hide();
    QObject::connect(this, &AddUserFrm::sigModifyUser, this, &AddUserFrm::slotModifyUser);
}

AddUserFrm::~AddUserFrm()
{

}

void AddUserFrmPrivate::InitUI()
{
    // Main layout - stack camera feed with overlays
    QVBoxLayout *mainLayout = new QVBoxLayout(q_func());
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Header widget
    m_pHeaderWidget = new QWidget;
    m_pHeaderWidget->setObjectName("HeaderWidget");
    m_pHeaderWidget->setFixedHeight(70);
    
    QHBoxLayout *headerLayout = new QHBoxLayout(m_pHeaderWidget);
    headerLayout->setContentsMargins(20, 15, 20, 15);
    
    // Back button
    m_pBackButton = new QPushButton;
    m_pBackButton->setObjectName("BackButton");
    m_pBackButton->setText("←");
    m_pBackButton->setFixedSize(40, 40);
    
    // Title label
    m_pTitleLabel = new QLabel;
    m_pTitleLabel->setObjectName("TitleLabel");
    m_pTitleLabel->setAlignment(Qt::AlignCenter);
    
    // Spacer for centering
    QWidget *spacer = new QWidget;
    spacer->setFixedSize(40, 40);
    
    headerLayout->addWidget(m_pBackButton);
    headerLayout->addWidget(m_pTitleLabel, 1);
    headerLayout->addWidget(spacer);

    // Status message label
    m_pStatusLabel = new QLabel;
    m_pStatusLabel->setObjectName("StatusMessage");
    m_pStatusLabel->setAlignment(Qt::AlignCenter);
    m_pStatusLabel->hide();
    m_pStatusTimer = new QTimer;
    m_pStatusTimer->setSingleShot(true);

    // Form overlay widget
    m_pFormOverlay = new QWidget;
    m_pFormOverlay->setObjectName("FormOverlay");
    
    QVBoxLayout *overlayLayout = new QVBoxLayout(m_pFormOverlay);
    overlayLayout->setContentsMargins(25, 25, 25, 25);
    overlayLayout->setSpacing(15);
    
    // Form content
    m_pFormContent = new QWidget;
    QVBoxLayout *formLayout = new QVBoxLayout(m_pFormContent);
    formLayout->setSpacing(15);
    formLayout->setContentsMargins(0, 0, 0, 0);

    // Name input row
    m_pNameRow = new QWidget;
    m_pNameRow->setObjectName("InputRow");
    QHBoxLayout *nameLayout = new QHBoxLayout(m_pNameRow);
    nameLayout->setContentsMargins(15, 12, 15, 12);
    nameLayout->setSpacing(15);
    
    m_pNameLabel = new QLabel;
    m_pNameLabel->setObjectName("InputLabel");
    m_pNameLabel->setMinimumWidth(80);
    
    m_pNameEdit = new QLineEdit;
    m_pNameEdit->setObjectName("InputField");
    
    nameLayout->addWidget(m_pNameLabel);
    nameLayout->addWidget(m_pNameEdit, 1);

    // Employee ID input row
    m_pIDCardRow = new QWidget;
    m_pIDCardRow->setObjectName("InputRow");
    QHBoxLayout *idLayout = new QHBoxLayout(m_pIDCardRow);
    idLayout->setContentsMargins(15, 12, 15, 12);
    idLayout->setSpacing(15);
    
    m_pIDCardLabel = new QLabel;
    m_pIDCardLabel->setObjectName("InputLabel");
    m_pIDCardLabel->setMinimumWidth(80);
    
    m_pIDCardEdit = new QLineEdit;
    m_pIDCardEdit->setObjectName("InputField");
    
    idLayout->addWidget(m_pIDCardLabel);
    idLayout->addWidget(m_pIDCardEdit, 1);

    // Card number input row
    m_pCardRow = new QWidget;
    m_pCardRow->setObjectName("InputRow");
    QHBoxLayout *cardLayout = new QHBoxLayout(m_pCardRow);
    cardLayout->setContentsMargins(15, 12, 15, 12);
    cardLayout->setSpacing(15);
    
    m_pCardLabel = new QLabel;
    m_pCardLabel->setObjectName("InputLabel");
    m_pCardLabel->setMinimumWidth(80);
    
    m_pCardEdit = new QLineEdit;
    m_pCardEdit->setObjectName("InputField");
    
    cardLayout->addWidget(m_pCardLabel);
    cardLayout->addWidget(m_pCardEdit, 1);

    // Gender input row
    m_pGenderRow = new QWidget;
    m_pGenderRow->setObjectName("InputRow");
    QHBoxLayout *genderLayout = new QHBoxLayout(m_pGenderRow);
    genderLayout->setContentsMargins(15, 12, 15, 12);
    genderLayout->setSpacing(15);
    
    m_pGenderLabel = new QLabel;
    m_pGenderLabel->setObjectName("InputLabel");
    m_pGenderLabel->setMinimumWidth(80);
    
    m_psexBox = new QComboBox;
    m_psexBox->setObjectName("Dropdown");
    
    genderLayout->addWidget(m_pGenderLabel);
    genderLayout->addWidget(m_psexBox, 1);

    // Action buttons row (Face buttons)
    m_pButtonRow = new QWidget;
    QHBoxLayout *buttonLayout = new QHBoxLayout(m_pButtonRow);
    buttonLayout->setContentsMargins(0, 10, 0, 0);
    buttonLayout->setSpacing(15);
    
    m_pCaptureFaceButton = new QPushButton;
    m_pCaptureFaceButton->setObjectName("CaptureButton");
    
    m_pRegFaceButton = new QPushButton;
    m_pRegFaceButton->setObjectName("RegisterButton");
    
    buttonLayout->addWidget(m_pCaptureFaceButton, 1);
    buttonLayout->addWidget(m_pRegFaceButton, 1);

    // ✅ FIX: ADD INPUT ROWS FIRST, THEN FINGERPRINT SECTION
    formLayout->addWidget(m_pNameRow);
    formLayout->addWidget(m_pIDCardRow);
    formLayout->addWidget(m_pCardRow);
    formLayout->addWidget(m_pGenderRow);
    formLayout->addWidget(m_pButtonRow);
    
    // ✅ NOW ADD FINGERPRINT SECTION AFTER INPUT ROWS
    QLabel *fingerSectionLabel = new QLabel(QObject::tr("━━━━━ Fingerprint ━━━━━"));
    fingerSectionLabel->setAlignment(Qt::AlignCenter);
    fingerSectionLabel->setStyleSheet("QLabel { color: #666; font-size: 14px; margin-top: 10px; }");
    
    formLayout->addWidget(fingerSectionLabel);
    
    // Fingerprint status label
    m_pFingerStatusLabel = new QLabel;
    m_pFingerStatusLabel->setAlignment(Qt::AlignCenter);
    m_pFingerStatusLabel->setStyleSheet("QLabel { color: #888; font-size: 13px; }");
    m_pFingerStatusLabel->setText(QObject::tr("No fingerprint captured"));
    
    formLayout->addWidget(m_pFingerStatusLabel);
    formLayout->addSpacing(10);
    
    // Fingerprint buttons
    m_pCaptureFingerButton = new QPushButton;
    m_pCaptureFingerButton->setFixedSize(220, 62);
    m_pCaptureFingerButton->setText(QObject::tr("Capture Finger"));
    m_pCaptureFingerButton->setObjectName("CaptureButton");
    
    m_pRegFingerButton = new QPushButton;
    m_pRegFingerButton->setFixedSize(220, 62);
    m_pRegFingerButton->setText(QObject::tr("Register Finger"));
    m_pRegFingerButton->setObjectName("RegisterButton");
    m_pRegFingerButton->setEnabled(false);
    
    m_pDeleteFingerButton = new QPushButton;
    m_pDeleteFingerButton->setFixedSize(220, 62);
    m_pDeleteFingerButton->setText(QObject::tr("Delete Fingerprint"));
    m_pDeleteFingerButton->setObjectName("RegisterButton");
    m_pDeleteFingerButton->setEnabled(false);
    
    QHBoxLayout *fingerButtonLayout = new QHBoxLayout;
    fingerButtonLayout->addStretch();
    fingerButtonLayout->addWidget(m_pCaptureFingerButton);
    fingerButtonLayout->addWidget(m_pRegFingerButton);
    fingerButtonLayout->addWidget(m_pDeleteFingerButton);
    fingerButtonLayout->addStretch();
    
    formLayout->addLayout(fingerButtonLayout);

    
    overlayLayout->addWidget(m_pFormContent);
    
    // Capture overlay (hidden by default)
    m_pCaptureOverlay = new QWidget;
    m_pCaptureOverlay->setObjectName("CaptureOverlay");
    m_pCaptureOverlay->hide();
    
    QVBoxLayout *captureLayout = new QVBoxLayout(m_pCaptureOverlay);
    captureLayout->setAlignment(Qt::AlignCenter);
    
    m_pCaptureFrame = new QWidget;
    m_pCaptureFrame->setObjectName("CaptureFrame");
    m_pCaptureFrame->setFixedSize(600, 450);
    
    m_pCaptureInstructions = new QLabel;
    m_pCaptureInstructions->setObjectName("CaptureInstructions");
    m_pCaptureInstructions->setAlignment(Qt::AlignCenter);
    
    captureLayout->addWidget(m_pCaptureFrame);
    captureLayout->addSpacing(30);
    captureLayout->addWidget(m_pCaptureInstructions);

    // Main layout assembly
    mainLayout->addWidget(m_pHeaderWidget);
    mainLayout->addStretch(1); // Camera feed area
    mainLayout->addWidget(m_pStatusLabel);
    mainLayout->addWidget(m_pFormOverlay);
    
    // Add capture overlay as separate widget
    m_pCaptureOverlay->setParent(q_func());
    m_pCaptureOverlay->resize(q_func()->size());

    // Set context menu policies
    m_pNameEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_pIDCardEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_pCardEdit->setContextMenuPolicy(Qt::NoContextMenu);

    // Set focus and touch policies
    m_pNameEdit->setFocusPolicy(Qt::StrongFocus);
    m_pIDCardEdit->setFocusPolicy(Qt::StrongFocus);
    m_pCardEdit->setFocusPolicy(Qt::StrongFocus);

    m_pNameEdit->setAttribute(Qt::WA_AcceptTouchEvents);
    m_pIDCardEdit->setAttribute(Qt::WA_AcceptTouchEvents);
    m_pCardEdit->setAttribute(Qt::WA_AcceptTouchEvents);

    m_pNameEdit->installEventFilter(q_func());
    m_pIDCardEdit->installEventFilter(q_func());
    m_pCardEdit->installEventFilter(q_func());
}

void AddUserFrmPrivate::ApplyModernStyles()
{
    q_func()->setStyleSheet(
        "AddUserFrm {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "                stop:0 #2a2a2a, stop:1 #1a1a1a);"
        "}"
        
        // Header styles
        "QWidget#HeaderWidget {"
        "    background: rgba(255, 255, 255, 0.95);"
        "    border-radius: 0px;"
        "}"
        "QPushButton#BackButton {"
        "    background: #2196F3;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 8px;"
        "    font-size: 18px;"
        "    font-weight: bold;"
        "}"
        "QPushButton#BackButton:hover {"
        "    background: #1976D2;"
        "}"
        "QLabel#TitleLabel {"
        "    font-size: 24px;"
        "    font-weight: 600;"
        "    color: #2c3e50;"
        "}"
        
        // Status message styles
        "QLabel#StatusMessage {"
        "    background: rgba(0, 0, 0, 0.8);"
        "    color: white;"
        "    padding: 10px 20px;"
        "    border-radius: 8px;"
        "    font-size: 14px;"
        "    margin: 0 50px;"
        "}"
        "QLabel#StatusMessage[status=\"success\"] {"
        "    background: rgba(76, 175, 80, 0.9);"
        "}"
        "QLabel#StatusMessage[status=\"error\"] {"
        "    background: rgba(244, 67, 54, 0.9);"
        "}"
        
        // Form overlay styles
        "QWidget#FormOverlay {"
        "    background: rgba(255, 255, 255, 0.95);"
        "    border-radius: 15px 15px 0px 0px;"
        "}"
        
        // Input row styles
        "QWidget#InputRow {"
        "    background: white;"
        "    border-radius: 8px;"
        "    border-left: 4px solid #e9ecef;"
        "}"
        "QWidget#InputRow:hover {"
        "    border-left-color: #2196F3;"
        "}"
        "QWidget#InputRow[required=\"true\"] {"
        "    border-left-color: #f44336;"
        "}"
        
        // Label styles
        "QLabel#InputLabel {"
        "    font-size: 16px;"
        "    font-weight: 600;"
        "    color: #2c3e50;"
        "}"
        
        // Input field styles
        "QLineEdit#InputField {"
        "    padding: 8px 12px;"
        "    border: 1px solid #e1e5e9;"
        "    border-radius: 6px;"
        "    font-size: 16px;"
        "    background: #f8f9fa;"
        "}"
        "QLineEdit#InputField:focus {"
        "    border-color: #2196F3;"
        "    background: white;"
        "}"
        "QLineEdit#InputField:read-only {"
        "    background-color: #e9ecef;"
        "    color: #6c757d;"
        "}"
        
        // Dropdown styles
        "QComboBox#Dropdown {"
        "    padding: 8px 12px;"
        "    border: 1px solid #e1e5e9;"
        "    border-radius: 6px;"
        "    font-size: 16px;"
        "    background: #f8f9fa;"
        "}"
        "QComboBox#Dropdown:focus {"
        "    border-color: #2196F3;"
        "    background: white;"
        "}"
        "QComboBox#Dropdown:disabled {"
        "    background-color: #e9ecef;"
        "    color: #6c757d;"
        "}"
        
        // Button styles
        "QPushButton#CaptureButton {"
        "    background: #2196F3;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 8px;"
        "    font-size: 16px;"
        "    font-weight: 600;"
        "    padding: 14px 20px;"
        "}"
        "QPushButton#CaptureButton:hover {"
        "    background: #1976D2;"
        "}"
        "QPushButton#RegisterButton {"
        "    background: #6c757d;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 8px;"
        "    font-size: 16px;"
        "    font-weight: 600;"
        "    padding: 14px 20px;"
        "}"
        "QPushButton#RegisterButton[enabled=\"true\"] {"
        "    background: #4CAF50;"
        "}"
        "QPushButton#RegisterButton[enabled=\"true\"]:hover {"
        "    background: #45a049;"
        "}"
        "QPushButton:disabled {"
        "    background: #9e9e9e;"
        "}"
        
        // Capture overlay styles
        "QWidget#CaptureOverlay {"
        "    background: rgba(0, 0, 0, 0.95);"
        "}"
        "QWidget#CaptureFrame {"
        "    border: 3px solid #2196F3;"
        "    border-radius: 15px;"
        "    background: rgba(255, 255, 255, 0.05);"
        "}"
        "QLabel#CaptureInstructions {"
        "    color: white;"
        "    font-size: 18px;"
        "    font-weight: 500;"
        "}"
    );
}

void AddUserFrmPrivate::InitData()
{
    q_func()->setObjectName("AddUserFrm");
    
    // Set labels
    m_pNameLabel->setText(QObject::tr("Name:") + " <span style='color: #f44336;'>*</span>");
    m_pIDCardLabel->setText(QObject::tr("EmpID:") + " <span style='color: #f44336;'>*</span>");
    m_pCardLabel->setText(QObject::tr("CardNo:"));
    m_pGenderLabel->setText(QObject::tr("Gender:"));
    
    // Set button texts
    m_pCaptureFaceButton->setText(QObject::tr("AcquisitionFace"));
    m_pRegFaceButton->setText(QObject::tr("RegisterFace"));
    
    // Set placeholders
    m_pNameEdit->setPlaceholderText(QObject::tr("Enter full name"));
    m_pIDCardEdit->setPlaceholderText(QObject::tr("Enter the employeeID as per SammyApp"));
    m_pCardEdit->setPlaceholderText(QObject::tr("Optional"));
    
    // Setup combobox
    m_psexBox->addItems(QStringList() << QObject::tr("male") << QObject::tr("female"));
    
    // Set title
    m_pTitleLabel->setText(QObject::tr("Add Person"));
    
    // Set capture instructions
    m_pCaptureInstructions->setText(QObject::tr("Position your face in the center\nLook directly at the camera"));

     // NEW: Fingerprint buttons
    m_pCaptureFingerButton->setFixedSize(220, 62);
    m_pRegFingerButton->setFixedSize(220, 62);
    m_pCaptureFingerButton->setText(QObject::tr("Capture Finger"));
    m_pRegFingerButton->setText(QObject::tr("Register Finger"));
    m_pRegFingerButton->setEnabled(false);
    
    // NEW: Fingerprint status label
    m_pFingerStatusLabel->setAlignment(Qt::AlignCenter);
    m_pFingerStatusLabel->setStyleSheet("QLabel { color: #888; font-size: 13px; }");
    m_pFingerStatusLabel->setText(QObject::tr("No fingerprint captured"));
    
    // Mark required fields
    m_pNameRow->setProperty("required", true);
    m_pIDCardRow->setProperty("required", true);
    
    // Initial button state
    UpdateButtonStates();
}

void AddUserFrmPrivate::InitConnect()
{
    QObject::connect(m_pBackButton, &QPushButton::clicked, q_func(), &AddUserFrm::slotReturnSuperiorClicked);
    QObject::connect(m_pCaptureFaceButton, &QPushButton::clicked, q_func(), &AddUserFrm::slotCaptureFaceButton);
    QObject::connect(m_pRegFaceButton, &QPushButton::clicked, q_func(), &AddUserFrm::slotRegFaceButton);

    // âœ… FIX: Add null checks before connecting fingerprint buttons
    if (m_pCaptureFingerButton) {
        QObject::connect(m_pCaptureFingerButton, &QPushButton::clicked, 
                        q_func(), &AddUserFrm::slotCaptureFingerButton);
    }
    
    if (m_pRegFingerButton) {
        QObject::connect(m_pRegFingerButton, &QPushButton::clicked, 
                        q_func(), &AddUserFrm::slotRegFingerButton);
    }
    
    if (m_pDeleteFingerButton) {
        QObject::connect(m_pDeleteFingerButton, &QPushButton::clicked, 
                        q_func(), &AddUserFrm::slotDeleteFingerButton);
    }

    QObject::connect(m_pNameEdit, &QLineEdit::textChanged, q_func(), &AddUserFrm::slotTextChanged);
    QObject::connect(m_pIDCardEdit, &QLineEdit::textChanged, q_func(), &AddUserFrm::slotTextChanged);

    QObject::connect(UserViewFrm::GetInstance(), &UserViewFrm::sigPersonInfo, q_func(), &AddUserFrm::slotPersonInfo);
    
    if (m_pStatusTimer) {
        QObject::connect(m_pStatusTimer, &QTimer::timeout, [this]() {
            if (m_pStatusLabel) {
                m_pStatusLabel->hide();
            }
        });
    }
}

void AddUserFrmPrivate::UpdateButtonStates()
{
    bool isFormValid = !m_pNameEdit->text().isEmpty() && !m_pIDCardEdit->text().isEmpty();
    bool canRegister = isFormValid && mHasFaceData;
    
    m_pRegFaceButton->setEnabled(canRegister);
    m_pRegFaceButton->setProperty("enabled", canRegister);
    
    // Force style update
    m_pRegFaceButton->style()->unpolish(m_pRegFaceButton);
    m_pRegFaceButton->style()->polish(m_pRegFaceButton);
}

void AddUserFrmPrivate::ShowStatusMessage(const QString &message, const QString &type)
{
    m_pStatusLabel->setText(message);
    m_pStatusLabel->setProperty("status", type);
    m_pStatusLabel->style()->unpolish(m_pStatusLabel);
    m_pStatusLabel->style()->polish(m_pStatusLabel);
    m_pStatusLabel->show();
    
    m_pStatusTimer->start(3000);
}

void AddUserFrm::modifyRecord(QString aName, QString aIDCard, QString aCardNo, QString asex, QString apersonid, QString auuid)
{
    Q_D(AddUserFrm);

    d->mIsModifyMode = true;
    d->m_pTitleLabel->setText(QObject::tr("Modify Person Info"));
    
    d->m_pNameEdit->setText(aName);
    d->m_pIDCardEdit->setText(aIDCard);
    d->m_pCardEdit->setText(aCardNo);
    
    if (asex == QObject::tr("male"))
        d->m_psexBox->setCurrentIndex(0);
    else if (asex == QObject::tr("female"))
        d->m_psexBox->setCurrentIndex(1);
    
    // Make fields read-only in modify mode
    d->m_pNameEdit->setReadOnly(true);
    d->m_pIDCardEdit->setReadOnly(true);
    d->m_pCardEdit->setReadOnly(true);
    d->m_psexBox->setEnabled(false);
    
    // Hide gender row in modify mode
    d->m_pGenderRow->hide();

    mPerson.name = aName;
    mPerson.sex = asex;
    mPerson.idcard = aIDCard;
    mPerson.iccard = aCardNo;
    mPerson.personid = apersonid.toInt();
    mPerson.uuid = auuid;
    
    d->UpdateButtonStates();
}

void AddUserFrm::slotModifyUser()
{
    Q_D(AddUserFrm);
    mModify++;
    if (mModify <= 1 && mPerson.name != nullptr) {
        d->mIsModifyMode = true;
        d->m_pTitleLabel->setText(QObject::tr("Modify Person Info"));
        
        d->m_pNameEdit->setText(mPerson.name);
        d->m_pIDCardEdit->setText(mPerson.idcard);
        d->m_pCardEdit->setText(mPerson.iccard);
        
        d->m_pNameEdit->setReadOnly(true);
        d->m_pIDCardEdit->setReadOnly(true);
        d->m_pCardEdit->setReadOnly(true);
        d->m_psexBox->setEnabled(false);
        
        // Hide gender row in modify mode
        d->m_pGenderRow->hide();
        
        d->UpdateButtonStates();
    }
}

void AddUserFrm::slotCaptureFaceButton()
{
    Q_D(AddUserFrm);
    
    // Show full-screen capture overlay
    d->m_pCaptureOverlay->show();
    d->m_pCaptureOverlay->resize(this->size());
    d->m_pCaptureOverlay->raise();
    
    // Start capture process
    int faceNum = 0;
    double threshold = 0;
    int ret = -1;
    QString path;
    
    system("rm /mnt/user/facedb/RegImage.jpg");
    system("rm /mnt/user/facedb/RegImage.jpeg");
    
    path = ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->getCurFaceImgPath(100);
    if (path.length() == 0) {
        d->m_pCaptureOverlay->hide();
        if (QMessageBox::question(this, QObject::tr("Tips"), QObject::tr("Please activate the algorithm first!"),
            QMessageBox::Ok) == QMessageBox::Ok) {
            return;
        }
    }

    QImage img = QImage(path);
    mPath = path;
    mDraw = true;
    
    // Rotation handling
    QSettings sysIniFile("/param/RV1109_PARAM.txt", QSettings::IniFormat);
    int rotation = sysIniFile.value("PLATFORM_ROTATION", 0).toInt();
    if (rotation > 0) {
        QTransform transform;
        transform.rotate(rotation);
        img = img.transformed(transform);
    }
    
    d->m_pCameraPicFrm->setShowImage(img);
    d->m_pCameraPicFrm->setShowImage(img);
    d->m_pCameraPicFrm->move(this->width() - 175, this->height() - 625);
    d->m_pCameraPicFrm->update();
    d->m_pCameraPicFrm->show();
    
    ret = ((BaiduFaceManager *)qXLApp->GetAlgoFaceManager())->RegistPerson(path, faceNum, threshold, d->mFaceFeature);
    
    // Hide capture overlay after processing
    QTimer::singleShot(2000, [this, d]() {
        d->m_pCaptureOverlay->hide();
    });
    
    if (ret == -1) {
        d->mHintText = QObject::tr("ErrorDataAcquisiting");
        d->ShowStatusMessage(d->mHintText, "error");
    } else if (ret == 0 && faceNum == 0) {
        d->mHintText = QObject::tr("NotFindFace");
        d->ShowStatusMessage(d->mHintText, "error");
    } else if (ret == 0 && faceNum >= 1) {
        d->mHintText = QObject::tr("FaceDataAcquisitOk");
        d->mHasFaceData = true;
        d->ShowStatusMessage(QObject::tr("Face data acquired successfully!"), "success");
    } else {
        d->mHintText.clear();
    }
    
    QFile fileTemp(path);
    fileTemp.remove();
    
    d->UpdateButtonStates();
    this->update();
}

void AddUserFrm::slotRegFaceButton()
{
    Q_D(AddUserFrm);
    
    // Disable button during processing
    d->m_pRegFaceButton->setEnabled(false);
    d->m_pRegFaceButton->setText(QObject::tr("Registering..."));
    
    // Your existing registration logic here...
    // [Keep all the existing slotRegFaceButton implementation]
    
    OperationTipsFrm dlg(this);
    bool isModifyingPerson = d->mIsModifyMode;
    
    d->m_pCameraPicFrm->setShowImage(QImage());
    d->m_pCameraPicFrm->hide();
    
    bool ret = false;
    
    if (isModifyingPerson) {
        ret = RegisteredFacesDB::GetInstance()->UpdatePersonFaceFeature(mPerson.uuid, d->mFaceFeature);
        
        if (ret) {
            bool sendResult = ConnHttpServerThread::GetInstance()->sendUserToServer(
                mPerson.uuid, mPerson.name, mPerson.idcard, mPerson.iccard, 
                mPerson.sex, "", "", d->mFaceFeature
            );
            
            d->ShowStatusMessage(QObject::tr("Face updated successfully for 【%1】!!!").arg(mPerson.name), "success");
        } else {
            d->ShowStatusMessage(QObject::tr("Failed to update face for 【%1】!!!").arg(mPerson.name), "error");
        }
    } else {
        // Create new person logic...
        ret = RegisteredFacesDB::GetInstance()->RegPersonToDBAndRAM(
            "", d->m_pNameEdit->text(), d->m_pIDCardEdit->text(), 
            d->m_pCardEdit->text(), d->m_psexBox->currentText(),
            "", "", d->mFaceFeature
        );
        
        if (ret) {
            d->ShowStatusMessage(QObject::tr("Person registered successfully!"), "success");
        } else {
            d->ShowStatusMessage(QObject::tr("Registration failed!"), "error");
        }
    }
    
    // Reset button
    d->m_pRegFaceButton->setText(QObject::tr("RegisterFace"));
    d->UpdateButtonStates();
    d->mHintText.clear();
}

void AddUserFrm::slotCaptureFingerButton()
{
    Q_D(AddUserFrm);
    
    printf("\n==========================================\n");
    printf("FINGERPRINT CAPTURE STARTED\n");
    printf("==========================================\n");
    
    // Validate employee ID first
    QString employeeId = d->m_pIDCardEdit->text().trimmed();
    if (employeeId.isEmpty()) {
        printf("[ERROR] Employee ID is empty!\n");
        QMessageBox::warning(this, QObject::tr("Warning"), 
                           QObject::tr("Please enter Employee ID first!"));
        return;
    }
    
    printf("[DEBUG] Employee ID: %s\n", employeeId.toStdString().c_str());
    
    // Update UI
    d->m_pFingerStatusLabel->setText(QObject::tr("Initializing sensor..."));
    d->m_pFingerStatusLabel->setStyleSheet("QLabel { color: blue; font-size: 13px; }");
    d->m_pCaptureFingerButton->setEnabled(false);
    QApplication::processEvents();
    
    // Get fingerprint manager
    FingerprintManager* fpManager = qXLApp->GetFingerprintManager();
    
    // Initialize sensor if needed
    if (!fpManager->isSensorReady()) {
        printf("[DEBUG] Initializing fingerprint sensor...\n");
        if (!fpManager->initFingerprintSensor()) {
            printf("[ERROR] Sensor initialization FAILED!\n");
            d->m_pFingerStatusLabel->setText(QObject::tr("Sensor initialization failed!"));
            d->m_pFingerStatusLabel->setStyleSheet("QLabel { color: red; font-size: 13px; }");
            d->m_pCaptureFingerButton->setEnabled(true);
            return;
        }
        printf("[SUCCESS] Sensor initialized\n");
    }
    
    uint16_t sensorFingerId = RegisteredFacesDB::getNextAvailableFingerId();
    
    printf("[DEBUG] Generated SEQUENTIAL Sensor Finger ID: %d for Employee: %s\n", 
           sensorFingerId, employeeId.toStdString().c_str());
    
    // Update UI
    d->m_pFingerStatusLabel->setText(QObject::tr("Starting enrollment..."));
    QApplication::processEvents();
    
    printf("\n[ENROLLMENT] Starting enrollment process...\n");
    
    // Clear previous data
    d->mFingerTemplate.clear();
    
    // ✅ STEP 1: Enroll fingerprint to sensor
    bool enrollmentSuccess = fpManager->startEnrollment(sensorFingerId);
    
    if (!enrollmentSuccess) {
        printf("[ERROR] Fingerprint enrollment FAILED!\n");
        d->mFingerHintText = QObject::tr("Failed to enroll fingerprint");
        d->m_pFingerStatusLabel->setText(d->mFingerHintText);
        d->m_pFingerStatusLabel->setStyleSheet("QLabel { color: red; font-size: 13px; }");
        d->m_pCaptureFingerButton->setEnabled(true);
        return;
    }
    
    printf("[SUCCESS] Fingerprint enrolled at sensor ID: %d\n", sensorFingerId);
    
    // ✅ STEP 2: Download template from sensor for backup
    QByteArray templateBackup;
    d->m_pFingerStatusLabel->setText(QObject::tr("Downloading template..."));
    QApplication::processEvents();
    
    bool downloadSuccess = fpManager->downloadFingerprintTemplate(sensorFingerId, templateBackup);
    
    if (!downloadSuccess || templateBackup.isEmpty()) {
        printf("[WARNING] Template download failed, but enrollment succeeded\n");
        // Continue anyway - sensor has the template
    } else {
        printf("[SUCCESS] Template downloaded: %d bytes\n", templateBackup.size());
    }
    
    // ✅ STEP 3: Create combined data structure
    // Store: [finger_id (2 bytes)][template_size (2 bytes)][template_data (variable)]
    QByteArray fingerprintData;
    QDataStream stream(&fingerprintData, QIODevice::WriteOnly);
    stream << sensorFingerId;              // uint16_t
    stream << (uint16_t)templateBackup.size();  // uint16_t
    if (!templateBackup.isEmpty()) {
        fingerprintData.append(templateBackup);
    }
    
    d->mFingerTemplate = fingerprintData;
    
    printf("[DEBUG] Fingerprint data prepared: %d bytes (ID=%d, Template=%d bytes)\n",
           fingerprintData.size(), sensorFingerId, templateBackup.size());
    
    d->mFingerHintText = QObject::tr("Fingerprint enrolled successfully!");
    d->m_pFingerStatusLabel->setText(QString("✓ %1\nSensor ID: %2")
                                    .arg(d->mFingerHintText)
                                    .arg(sensorFingerId));
    d->m_pFingerStatusLabel->setStyleSheet("QLabel { color: green; font-size: 13px; font-weight: bold; }");
    
    // Enable registration button
    bool canRegister = !d->m_pNameEdit->text().isEmpty() && 
                      !d->m_pIDCardEdit->text().isEmpty();
    d->m_pRegFingerButton->setEnabled(canRegister);
    
    d->m_pCaptureFingerButton->setEnabled(true);
    
    printf("==========================================\n");
    printf("FINGERPRINT CAPTURE COMPLETED\n");
    printf("==========================================\n\n");
    
    this->update();
}

void AddUserFrm::clearFormAndResetUI()
{
    Q_D(AddUserFrm);
    
    // Clear existing fields
    d->m_pNameEdit->clear();
    d->m_pIDCardEdit->clear();
    d->m_pCardEdit->clear();
    d->mFaceFeature.clear();
    d->m_pRegFaceButton->setEnabled(false);
    d->mHintText.clear();
    
    // NEW: Clear fingerprint data
    d->mFingerTemplate.clear();
    d->m_pRegFingerButton->setEnabled(false);
    d->mFingerHintText.clear();
    d->m_pFingerStatusLabel->setText(QObject::tr("No fingerprint captured"));
    d->m_pFingerStatusLabel->setStyleSheet("QLabel { color: #888; font-size: 13px; }");
    
    this->update();
}

void AddUserFrm::slotRegFingerButton()
{
    Q_D(AddUserFrm);
    
    printf("\n==========================================\n");
    printf("FINGERPRINT REGISTRATION TO DATABASE\n");
    printf("==========================================\n");
    
    d->m_pRegFingerButton->setEnabled(false);
    
    QString employeeId = d->m_pIDCardEdit->text().trimmed();
    QString userName = d->m_pNameEdit->text().trimmed();
    
    if (employeeId.isEmpty() || userName.isEmpty()) {
        QMessageBox::warning(this, QObject::tr("Warning"), 
                           QObject::tr("Please fill in Name and Employee ID!"));
        d->m_pRegFingerButton->setEnabled(true);
        return;
    }
    
    if (d->mFingerTemplate.isEmpty()) {
        QMessageBox::warning(this, QObject::tr("Warning"), 
                           QObject::tr("Please capture fingerprint first!"));
        d->m_pRegFingerButton->setEnabled(true);
        return;
    }
    
    // Extract sensor ID from stored data
    QDataStream stream(d->mFingerTemplate);
    uint16_t sensorFingerId;
    uint16_t templateSize;
    stream >> sensorFingerId >> templateSize;
    
    printf("[DEBUG] Extracted - Sensor ID: %d, Template size: %d\n", 
           sensorFingerId, templateSize);
    
    QApplication::setOverrideCursor(Qt::WaitCursor);
    
    RegisteredFacesDB* faceDB = RegisteredFacesDB::GetInstance();
    QString uuid = faceDB->findUuidByEmployeeId(employeeId);
    
    if (uuid.isEmpty()) {
        QApplication::restoreOverrideCursor();
        QMessageBox::critical(this, QObject::tr("Error"), 
                            QObject::tr("User not found! Please register user first."));
        d->m_pRegFingerButton->setEnabled(true);
        return;
    }
    
    // Store complete fingerprint data (ID + template backup)
    bool dbSuccess = faceDB->UpdatePersonFingerprint(uuid, d->mFingerTemplate);
    
    // Also update finger_id column separately
    bool idSuccess = faceDB->UpdatePersonFingerId(uuid, sensorFingerId);
    
    QApplication::restoreOverrideCursor();
    
    if (dbSuccess && idSuccess) {
        printf("[SUCCESS] Fingerprint registered - Employee: %s, UUID: %s, Sensor ID: %d\n", 
               userName.toStdString().c_str(), uuid.toStdString().c_str(), sensorFingerId);
        
        d->m_pFingerStatusLabel->setText(QObject::tr("✓ Registered!\nSensor ID: %1").arg(sensorFingerId));
        d->m_pFingerStatusLabel->setStyleSheet("QLabel { color: green; font-size: 13px; font-weight: bold; }");
        
        OperationTipsFrm dlg(this);
        dlg.setMessageBox(QObject::tr("Success"), 
                        QObject::tr("Fingerprint registered for 「%1」!\n\nSensor ID: %2")
                        .arg(userName).arg(sensorFingerId));
        dlg.exec();
        
        d->mFingerTemplate.clear();
        d->mFingerHintText.clear();
        
    } else {
        printf("[ERROR] Database storage FAILED!\n");
        
        QMessageBox::critical(this, QObject::tr("Error"), 
                            QObject::tr("Failed to save to database!"));
    }
    
    d->m_pRegFingerButton->setEnabled(!d->mFingerTemplate.isEmpty());
    
    printf("==========================================\n\n");
    
    this->update();
}

void AddUserFrm::slotReturnSuperiorClicked()
{
    Q_D(AddUserFrm);
    
    d->m_pCameraPicFrm->setShowImage(QImage());
    d->m_pCameraPicFrm->hide();
    
    if (d->mIsModifyMode) {
        // Reset to add mode
        d->mIsModifyMode = false;
        d->m_pTitleLabel->setText(QObject::tr("Add Person"));
        
        d->m_pNameEdit->setReadOnly(false);
        d->m_pIDCardEdit->setReadOnly(false);
        d->m_pCardEdit->setReadOnly(false);
        d->m_psexBox->setEnabled(true);
        
        // Show gender row in add mode
        d->m_pGenderRow->show();
    }

    emit sigShowFaceHomeFrm();

    UserViewFrm::GetInstance()->setModifyFlag(0);
    
    d->m_pNameEdit->clear();
    d->m_pIDCardEdit->clear();
    d->m_pCardEdit->clear();
    d->mHasFaceData = false;
    mPerson = {0};
    mModify = 0;
    
    d->UpdateButtonStates();
}

void AddUserFrm::slotTextChanged(const QString &text)
{
    Q_D(AddUserFrm);
    d->UpdateButtonStates();
}

void AddUserFrm::slotPersonInfo(const QString &aName, const QString &asex, const QString &aCard, const QString &cardNo)
{
    Q_D(AddUserFrm);

    mPerson.name = aName;
    mPerson.sex = asex;
    mPerson.idcard = aCard;
    mPerson.iccard = cardNo;
}

void AddUserFrm::slotDeleteFingerButton()
{
    Q_D(AddUserFrm);  // <-- ADD THIS LINE
    
    QString employeeId = d->m_pIDCardEdit->text().trimmed();
    if (employeeId.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please enter Employee ID"));
        return;
    }
    
    // Get user from database
    RegisteredFacesDB* faceDB = RegisteredFacesDB::GetInstance();
    QString uuid = faceDB->findUuidByEmployeeId(employeeId);
    
    if (uuid.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("User not found"));
        return;
    }
    
    // Get sensor finger ID from database
    uint16_t sensorFingerId = faceDB->getFingerId(uuid);
    
    if (sensorFingerId == 0) {
        QMessageBox::information(this, tr("Info"), tr("No fingerprint enrolled for this user"));
        return;
    }
    
    // Confirm deletion
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Confirm"), 
                                  tr("Delete fingerprint for %1?\n\nSensor ID: %2")
                                  .arg(d->m_pNameEdit->text()).arg(sensorFingerId),
                                  QMessageBox::Yes | QMessageBox::No);
    
    if (reply != QMessageBox::Yes) {
        return;
    }
    
    // Delete from sensor
    FingerprintManager* fpManager = qXLApp->GetFingerprintManager();
    
    if (fpManager->deleteFingerprintTemplate(sensorFingerId)) {
        // Delete from database
        bool dbSuccess = faceDB->clearFingerprint(uuid);
        
        if (dbSuccess) {
            QMessageBox::information(this, tr("Success"), 
                                    tr("Fingerprint deleted successfully"));
            
            d->m_pFingerStatusLabel->setText(tr("No fingerprint enrolled"));
            d->m_pDeleteFingerButton->setEnabled(false);
        } else {
            QMessageBox::warning(this, tr("Warning"), 
                                tr("Deleted from sensor but database update failed"));
        }
    } else {
        QMessageBox::critical(this, tr("Error"), 
                             tr("Failed to delete fingerprint from sensor"));
    }
}

void AddUserFrm::hideEvent(QHideEvent *event)
{
    Q_D(AddUserFrm);
    d->mHintText.clear();
    d->mHasFaceData = false;
    d->UpdateButtonStates();
    QWidget::hideEvent(event);
}

void AddUserFrm::paintEvent(QPaintEvent *event)
{
    Q_D(AddUserFrm);
    QWidget::paintEvent(event);
    emit sigModifyUser();

#ifdef SCREENCAPTURE
    if (!mPath.isEmpty() && mDraw) {
        mDraw = false;
        QPainter painter(this);
        QPixmap pix;
        pix.load(mPath);
        painter.drawPixmap(0, 0, 800, 1280, pix);
        grab().save(QString("/mnt/user/screenshot/painterAddUserFrm.png"), "png");
    }
#endif
}

void AddUserFrm::resizeEvent(QResizeEvent *event)
{
    Q_D(AddUserFrm);
    QWidget::resizeEvent(event);
    
    // Resize capture overlay to match window
    if (d->m_pCaptureOverlay) {
        d->m_pCaptureOverlay->resize(this->size());
    }
}

bool AddUserFrm::eventFilter(QObject *obj, QEvent *event)
{
    // Keep your existing eventFilter implementation for keyboard handling
    return QWidget::eventFilter(obj, event);
}

#ifdef SCREENCAPTURE
void AddUserFrm::mouseDoubleClickEvent(QMouseEvent* event)
{
    // Keep your existing implementation
}
#endif