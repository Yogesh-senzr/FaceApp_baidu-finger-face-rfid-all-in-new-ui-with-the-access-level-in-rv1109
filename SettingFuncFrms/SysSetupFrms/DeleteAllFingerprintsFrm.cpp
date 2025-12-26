#include "DeleteAllFingerprintsFrm.h"
#include "BaiduFace/FingerprintManager.h"
#include "MessageHandler/Log.h"
#include <QMessageBox>
#include <QApplication>

DeleteAllFingerprintsFrm::DeleteAllFingerprintsFrm(QWidget *parent)
    : QDialog(parent)
    , m_pDeleteBtn(nullptr)
    , m_pCancelBtn(nullptr)
    , m_pWarningLabel(nullptr)
{
    initUI();
}

DeleteAllFingerprintsFrm::~DeleteAllFingerprintsFrm()
{
}
void DeleteAllFingerprintsFrm::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    
    // Reset to initial state every time dialog is shown
    m_pWarningLabel->setText(tr("Warning!\n\n"
                                "This will permanently delete ALL fingerprints\n"
                                "from the sensor database.\n\n"
                                "This action cannot be undone!"));
    m_pWarningLabel->setStyleSheet("QLabel { color: red; font-size: 14pt; font-weight: bold; }");
    
    m_pDeleteBtn->setEnabled(true);
    m_pCancelBtn->setEnabled(true);
}
void DeleteAllFingerprintsFrm::initUI()
{
    setWindowTitle(tr("Delete All Fingerprints"));
    setFixedSize(400, 250);
    setModal(true);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    
    // Warning label
    m_pWarningLabel = new QLabel(this);
    m_pWarningLabel->setText(tr("Warning!\n\n"
                                "This will permanently delete ALL fingerprints\n"
                                "from the sensor database.\n\n"
                                "This action cannot be undone!"));
    m_pWarningLabel->setAlignment(Qt::AlignCenter);
    m_pWarningLabel->setStyleSheet("QLabel { color: red; font-size: 14pt; font-weight: bold; }");
    m_pWarningLabel->setWordWrap(true);
    
    // Button layout
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(20);
    
    // Delete button
    m_pDeleteBtn = new QPushButton(tr("Delete All"), this);
    m_pDeleteBtn->setFixedSize(150, 50);
    m_pDeleteBtn->setStyleSheet("QPushButton { "
                                "background-color: #d32f2f; "
                                "color: white; "
                                "font-size: 14pt; "
                                "border-radius: 5px; "
                                "} "
                                "QPushButton:pressed { "
                                "background-color: #b71c1c; "
                                "}");
    
    // Cancel button
    m_pCancelBtn = new QPushButton(tr("Cancel"), this);
    m_pCancelBtn->setFixedSize(150, 50);
    m_pCancelBtn->setStyleSheet("QPushButton { "
                                "background-color: #757575; "
                                "color: white; "
                                "font-size: 14pt; "
                                "border-radius: 5px; "
                                "} "
                                "QPushButton:pressed { "
                                "background-color: #616161; "
                                "}");
    
    btnLayout->addWidget(m_pDeleteBtn);
    btnLayout->addWidget(m_pCancelBtn);
    
    mainLayout->addWidget(m_pWarningLabel);
    mainLayout->addStretch();
    mainLayout->addLayout(btnLayout);
    
    // Connect signals
    connect(m_pDeleteBtn, &QPushButton::clicked, this, &DeleteAllFingerprintsFrm::onDeleteClicked);
    connect(m_pCancelBtn, &QPushButton::clicked, this, &DeleteAllFingerprintsFrm::onCancelClicked);
}

void DeleteAllFingerprintsFrm::onDeleteClicked()
{
    LogD("%s %s[%d] Delete All button clicked\n", __FILE__, __FUNCTION__, __LINE__);
    
    // Show progress message
    m_pWarningLabel->setText(tr("Deleting all fingerprints...\nPlease wait..."));
    m_pWarningLabel->setStyleSheet("QLabel { color: orange; font-size: 14pt; }");
    m_pDeleteBtn->setEnabled(false);
    m_pCancelBtn->setEnabled(false);
    
    QApplication::processEvents(); // Update UI
    
    // Call the delete all function - FIX: Use GetInstance() instead of getInstance()
    FingerprintManager *fpManager = FingerprintManager::GetInstance();
    if (!fpManager) {
        LogE("%s %s[%d] FingerprintManager instance is NULL!\n", __FILE__, __FUNCTION__, __LINE__);
        QMessageBox::critical(this, tr("Error"), tr("Fingerprint manager not initialized!"));
        reject();
        return;
    }
    
    bool success = fpManager->deleteAllFingerprints();
    
    if (success) {
        LogD("%s %s[%d] All fingerprints deleted successfully\n", __FILE__, __FUNCTION__, __LINE__);
        QMessageBox::information(this, tr("Success"), 
                                tr("All fingerprints have been deleted successfully!"));
        accept(); // Close dialog with success
    } else {
        LogE("%s %s[%d] Failed to delete all fingerprints\n", __FILE__, __FUNCTION__, __LINE__);
        QMessageBox::critical(this, tr("Error"), 
                             tr("Failed to delete fingerprints!\nPlease try again."));
        reject(); // Close dialog with failure
    }
}

void DeleteAllFingerprintsFrm::onCancelClicked()
{
    LogD("%s %s[%d] Cancel button clicked\n", __FILE__, __FUNCTION__, __LINE__);
    reject(); // Close dialog without action
}
