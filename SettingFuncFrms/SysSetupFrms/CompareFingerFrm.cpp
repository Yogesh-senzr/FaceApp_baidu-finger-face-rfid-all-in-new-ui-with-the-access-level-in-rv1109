#include "CompareFingerFrm.h"
#include "BaiduFace/FingerprintManager.h"
#include "Application/FaceApp.h"
#include <QDebug>
#include <QTimer>
#include <QMessageBox>

CompareFingerFrm::CompareFingerFrm(QWidget *parent)
    : QDialog(parent)
    , m_pCompareBtn(nullptr)
    , m_pBackBtn(nullptr)
    , m_pStatusLabel(nullptr)
{
    initUI();
    initConnect();
}

CompareFingerFrm::~CompareFingerFrm()
{
}

void CompareFingerFrm::initUI()
{
    setFixedSize(400, 300);
    setWindowTitle(tr("Compare Finger"));
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Status label
    m_pStatusLabel = new QLabel(tr("Click Compare to start fingerprint comparison"), this);
    m_pStatusLabel->setAlignment(Qt::AlignCenter);
    m_pStatusLabel->setStyleSheet("font-size: 16px; margin: 20px;");
    
    // Buttons
    m_pCompareBtn = new QPushButton(tr("Compare"), this);
    m_pBackBtn = new QPushButton(tr("Back"), this);
    
    m_pCompareBtn->setFixedHeight(50);
    m_pBackBtn->setFixedHeight(50);
    
    m_pCompareBtn->setStyleSheet("QPushButton { font-size: 16px; background-color: #4CAF50; color: white; border: none; border-radius: 5px; }"
                                "QPushButton:hover { background-color: #45a049; }");
    m_pBackBtn->setStyleSheet("QPushButton { font-size: 16px; background-color: #f44336; color: white; border: none; border-radius: 5px; }"
                             "QPushButton:hover { background-color: #da190b; }");
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_pCompareBtn);
    buttonLayout->addWidget(m_pBackBtn);
    buttonLayout->setSpacing(20);
    
    mainLayout->addWidget(m_pStatusLabel);
    mainLayout->addLayout(buttonLayout);
    mainLayout->setContentsMargins(30, 30, 30, 30);
}

void CompareFingerFrm::initConnect()
{
    connect(m_pCompareBtn, &QPushButton::clicked, this, &CompareFingerFrm::onCompareClicked);
    connect(m_pBackBtn, &QPushButton::clicked, this, &CompareFingerFrm::onBackClicked);
}

void CompareFingerFrm::onCompareClicked()
{
    qDebug() << "=== Compare Finger Clicked ===";
    
    // Disable button during operation
    m_pCompareBtn->setEnabled(false);
    m_pStatusLabel->setText(tr("Place your finger on the sensor..."));
    
    // Get fingerprint manager instance
    FingerprintManager* fpManager = qXLApp->GetFingerprintManager();
    
    if (!fpManager) {
        QMessageBox::critical(this, "Error", "Fingerprint manager not initialized!");
        m_pCompareBtn->setEnabled(true);
        m_pStatusLabel->setText(tr("Error: System not ready"));
        return;
    }
    
    // Check sensor ready
    if (!fpManager->isSensorReady()) {
        QMessageBox::warning(this, "Sensor Error", 
                           "Fingerprint sensor not ready!\n\nPlease check connection.");
        m_pCompareBtn->setEnabled(true);
        m_pStatusLabel->setText(tr("Sensor not ready"));
        return;
    }
    
    // Update UI
    m_pStatusLabel->setText(tr("Scanning fingerprint... Please wait."));
    QApplication::processEvents(); // Force UI update
    
    // Perform identification in a separate thread-safe manner
    QTimer::singleShot(100, this, [this, fpManager]() {
        bool success = fpManager->identifyAndShowResult(this);
        
        // Re-enable button
        m_pCompareBtn->setEnabled(true);
        
        if (success) {
            m_pStatusLabel->setText(tr("✓ Fingerprint recognized successfully!"));
            m_pStatusLabel->setStyleSheet("color: green; font-size: 16px; font-weight: bold;");
        } else {
            m_pStatusLabel->setText(tr("✗ Fingerprint not recognized"));
            m_pStatusLabel->setStyleSheet("color: red; font-size: 16px;");
        }
        
        // Reset status after 3 seconds
        QTimer::singleShot(3000, this, [this]() {
            m_pStatusLabel->setText(tr("Click Compare to start fingerprint comparison"));
            m_pStatusLabel->setStyleSheet("font-size: 16px; margin: 20px;");
        });
    });
}

void CompareFingerFrm::onBackClicked()
{
    qDebug() << "Back button clicked";
    this->reject(); // Close the dialog
}
