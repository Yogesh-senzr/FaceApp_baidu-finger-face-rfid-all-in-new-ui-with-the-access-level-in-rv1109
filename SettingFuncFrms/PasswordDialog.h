#ifndef PASSWORDDIALOG_H
#define PASSWORDDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class PasswordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PasswordDialog(QWidget *parent = nullptr);
    ~PasswordDialog();

    QString getPassword() const;

protected:
    void showEvent(QShowEvent *event) override; // Ensure input resets on open

private slots:
    void onAccept();

private:
    QLineEdit *passwordEdit;
    QPushButton *okButton;
    QPushButton *cancelButton;
    QLabel *hintLabel;
};

#endif // PASSWORDDIALOG_H
