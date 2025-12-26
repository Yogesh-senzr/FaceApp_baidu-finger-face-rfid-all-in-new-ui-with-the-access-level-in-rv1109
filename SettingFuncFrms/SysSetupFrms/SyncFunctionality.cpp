#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QSettings>
#include <QDebug>
#include <QPixmap>
#include <QFrame>
#include "Config/ReadConfig.h"

class SyncFunctionality
{
public:
    static void ShowSyncDialog();
};

void SyncFunctionality::ShowSyncDialog()
{
    QDialog dialog;
    dialog.setObjectName("SyncDialog");
    dialog.setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    dialog.setFixedSize(450, 350);
    
    // Apply card-based styling
    dialog.setStyleSheet(
        "QDialog#SyncDialog {"
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
        
        // Toggle container
        "QFrame#ToggleContainer {"
        "    background: #f8f9fa;"
        "    border: 2px solid #e1e5e9;"
        "    border-radius: 12px;"
        "    padding: 20px;"
        "}"
        
        // Toggle button
        "QPushButton#ToggleButton {"
        "    border: none;"
        "    border-radius: 15px;"
        "    background: #ccc;"
        "}"
        "QPushButton#ToggleButton:checked {"
        "    background: #2196F3;"
        "}"
        
        // Status label
        "QLabel#StatusLabel {"
        "    font-size: 16px;"
        "    font-weight: 500;"
        "    color: #2c3e50;"
        "}"
        
        // Action button
        "QPushButton#BackButton {"
        "    background: #2196F3;"
        "    color: white;"
        "    border: none;"
        "    padding: 12px 24px;"
        "    border-radius: 8px;"
        "    font-size: 16px;"
        "    font-weight: 600;"
        "    min-width: 100px;"
        "}"
        "QPushButton#BackButton:hover {"
        "    background: #1976D2;"
        "}"
        
        // Separator
        "QFrame#DialogSeparator {"
        "    color: #e9ecef;"
        "    background-color: #e9ecef;"
        "}"
    );

    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Header section
    QWidget *headerWidget = new QWidget;
    headerWidget->setObjectName("DialogHeader");
    headerWidget->setFixedHeight(70);
    
    QHBoxLayout *headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(20, 0, 20, 0);
    
    QLabel *titleLabel = new QLabel("Sync Data");
    titleLabel->setObjectName("DialogTitle");
    titleLabel->setAlignment(Qt::AlignCenter);
    
    headerLayout->addWidget(titleLabel);
    
    // Content section
    QWidget *contentWidget = new QWidget;
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(25, 25, 25, 25);
    contentLayout->setSpacing(25);
    
    // Toggle container card
    QFrame *toggleContainer = new QFrame;
    toggleContainer->setObjectName("ToggleContainer");
    
    QHBoxLayout *toggleLayout = new QHBoxLayout(toggleContainer);
    toggleLayout->setSpacing(15);
    toggleLayout->setAlignment(Qt::AlignCenter);
    
    // Read current sync state
    bool isSyncEnabled = ReadConfig::GetInstance()->getSyncEnabled() != 0;
    qDebug() << "SYNC_DIALOG: Initial sync state from singleton:" << isSyncEnabled;
    
    // Custom toggle button
    QPushButton *syncButton = new QPushButton;
    syncButton->setObjectName("ToggleButton");
    syncButton->setCheckable(true);
    syncButton->setChecked(isSyncEnabled);
    syncButton->setFixedSize(60, 30);
    
    // Function to update button appearance
    auto updateButtonAppearance = [](QPushButton* button, bool enabled) {
        // Try to load images first
        QPixmap pixmap;
        if (enabled) {
            pixmap.load(":/Images/wifiOn.png");
        } else {
            pixmap.load(":/Images/wifiOff.png");
        }
        
        if (!pixmap.isNull()) {
            QPixmap scaledPixmap = pixmap.scaled(button->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            button->setIcon(QIcon(scaledPixmap));
            button->setIconSize(button->size());
            button->setText("");
        } else {
            // Fallback to text if images not found
            button->setIcon(QIcon());
            button->setText(enabled ? "ON" : "OFF");
            button->setStyleSheet(button->styleSheet() + 
                QString("color: white; font-weight: bold; font-size: 12px;"));
        }
    };
    
    // Status label
    QLabel *syncStatusLabel = new QLabel(isSyncEnabled ? "Enabled" : "Disabled");
    syncStatusLabel->setObjectName("StatusLabel");
    
    // Apply initial appearance
    updateButtonAppearance(syncButton, isSyncEnabled);
    
    // Connect toggle functionality
    QObject::connect(syncButton, &QPushButton::clicked, [syncButton, syncStatusLabel, updateButtonAppearance]() {
        bool newState = syncButton->isChecked();
        qDebug() << "SYNC_DIALOG: Button clicked, new state:" << newState;
        
        syncStatusLabel->setText(newState ? "Enabled" : "Disabled");
        updateButtonAppearance(syncButton, newState);
        
        // Save the state immediately using singleton
        qDebug() << "SYNC_DIALOG: Saving state to singleton...";
        ReadConfig::GetInstance()->setSyncEnabled(newState ? 1 : 0);
        
        // Force save to file immediately
        ReadConfig::GetInstance()->setSaveConfig();
        
        // Verify the state was saved by reading it back
        bool savedState = ReadConfig::GetInstance()->getSyncEnabled() != 0;
        qDebug() << "SYNC_DIALOG: State verification - requested:" << newState << "saved:" << savedState;
        
        if (savedState != newState) {
            qDebug() << "ERROR: SYNC_DIALOG: State was not saved correctly!";
        } else {
            qDebug() << "SUCCESS: SYNC_DIALOG: State saved correctly";
        }
    });
    
    toggleLayout->addWidget(syncButton);
    toggleLayout->addWidget(syncStatusLabel);
    
    // Action buttons section
    QWidget *actionsWidget = new QWidget;
    QHBoxLayout *actionsLayout = new QHBoxLayout(actionsWidget);
    actionsLayout->setContentsMargins(0, 20, 0, 0);
    actionsLayout->setAlignment(Qt::AlignCenter);
    
    QPushButton *backButton = new QPushButton("Back");
    backButton->setObjectName("BackButton");
    
    QObject::connect(backButton, &QPushButton::clicked, [&]() {
        // Verify state before closing
        bool finalState = ReadConfig::GetInstance()->getSyncEnabled() != 0;
        qDebug() << "SYNC_DIALOG: Dialog closing - final sync state:" << finalState;
        dialog.close();
    });
    
    actionsLayout->addWidget(backButton);
    
    // Add separator line
    QFrame *separator = new QFrame;
    separator->setFrameShape(QFrame::HLine);
    separator->setObjectName("DialogSeparator");
    
    contentLayout->addWidget(toggleContainer);
    contentLayout->addWidget(separator);
    contentLayout->addWidget(actionsWidget);
    
    // Assemble main layout
    mainLayout->addWidget(headerWidget);
    mainLayout->addWidget(contentWidget, 1);
    
    // Debug initial state
    qDebug() << "SYNC_DIALOG: Dialog opened - initial sync state:" << isSyncEnabled;
    
    dialog.exec();
    
    // Final debug after dialog closes
    bool finalState = ReadConfig::GetInstance()->getSyncEnabled() != 0;
    qDebug() << "SYNC_DIALOG: Dialog execution finished - final state:" << finalState;
}