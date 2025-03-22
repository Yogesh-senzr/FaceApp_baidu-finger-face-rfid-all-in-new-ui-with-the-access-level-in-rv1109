#include "PasswordDialog.h"
#include <QVBoxLayout>
#include <QMessageBox>
#include <QKeyEvent>
#include <QDebug>
#include <QInputMethod>
#include <QGuiApplication>
#include "vmKeyboardInput/frminput.h"


PasswordDialog::PasswordDialog(QWidget *parent) 
    : QDialog(parent)
{
    setWindowTitle("Enter Password");
    setWindowModality(Qt::WindowModal);
    setFixedSize(400, 300);

    QVBoxLayout *layout = new QVBoxLayout(this);
    QLabel *label = new QLabel("Enter Password:", this);
    passwordEdit = new QLineEdit(this);
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setPlaceholderText("Password");

    hintLabel = new QLabel("<p><font color=\"red\">Incorrect Password!</font></p>", this);
    hintLabel->hide(); // Hide error initially

    okButton = new QPushButton("OK", this);
    cancelButton = new QPushButton("Cancel", this);

    layout->addWidget(label);
    layout->addWidget(passwordEdit);
    layout->addWidget(hintLabel);
    layout->addWidget(okButton);
    layout->addWidget(cancelButton);

    connect(okButton, &QPushButton::clicked, this, &PasswordDialog::onAccept);
    connect(cancelButton, &QPushButton::clicked, this, &PasswordDialog::reject);
    connect(passwordEdit, &QLineEdit::returnPressed, this, &PasswordDialog::onAccept);
}

PasswordDialog::~PasswordDialog() {}

void PasswordDialog::showEvent(QShowEvent *event) {
    QDialog::showEvent(event);  // Call the base class implementation

    passwordEdit->setFocus();          // Ensure the password field is focused
    passwordEdit->activateWindow();    // Activate the window for keyboard input
}




void PasswordDialog::onAccept()
{
    if (passwordEdit->text() == "admin123") { // Replace with dynamic password check
        accept();
    } else {
        hintLabel->show();
    }
}

QString PasswordDialog::getPassword() const
{
    return passwordEdit->text();
}