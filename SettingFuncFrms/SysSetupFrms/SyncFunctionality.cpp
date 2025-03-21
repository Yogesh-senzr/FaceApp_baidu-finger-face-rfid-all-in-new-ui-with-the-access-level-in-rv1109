#include <QDialog>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QSettings>
#include "Config/ReadConfig.h"

class SyncFunctionality
{
public:
    static void ShowSyncDialog();
};

void SyncFunctionality::ShowSyncDialog()
{
    QDialog dialog;
    dialog.setWindowTitle("Device Synchronization");
    dialog.setFixedSize(400, 250);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    
    QLabel *titleLabel = new QLabel("Device Synchronization:");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #333;");
    layout->addWidget(titleLabel);

    // Sync Toggle Button
    QPushButton *syncButton = new QPushButton;
    syncButton->setCheckable(true);
    syncButton->setFixedSize(80, 40);
    syncButton->setStyleSheet(
        "QPushButton {"
        "   border-radius: 20px;"
        "   background-color: #d9534f;" // Default OFF (red)
        "   color: white;"
        "}"
        "QPushButton:checked {"
        "   background-color: #5cb85c;" // ON (green)
        "}"
    );

    // Read current sync state from settings
    QSettings settings("/mnt/user/parameters.ini", QSettings::IniFormat);
    bool isSyncEnabled = settings.value("Sync_Settings/EnableSync", false).toBool();
    syncButton->setChecked(isSyncEnabled);

    // Toggle text
    QLabel *syncStatusLabel = new QLabel(isSyncEnabled ? "Enabled" : "Disabled");
    syncStatusLabel->setStyleSheet("font-size: 16px; color: #555;");

    QObject::connect(syncButton, &QPushButton::clicked, [&]() {
        bool state = syncButton->isChecked();
        syncStatusLabel->setText(state ? "Enabled" : "Disabled");
        settings.setValue("Sync_Settings/EnableSync", state);
        settings.sync();
    });

    // Layout for switch and label
    QHBoxLayout *toggleLayout = new QHBoxLayout;
    toggleLayout->addWidget(syncButton);
    toggleLayout->addWidget(syncStatusLabel);
    layout->addLayout(toggleLayout);

    // Back Button
    QPushButton *backButton = new QPushButton("Back");
    backButton->setStyleSheet(
        "padding: 10px; font-size: 16px; background-color:rgb(79, 116, 217); color: white; border-radius: 5px;"
    );

    QObject::connect(backButton, &QPushButton::clicked, [&]() {
        dialog.close();
    });

    layout->addWidget(backButton);
    dialog.setLayout(layout);
    dialog.exec();
}
