#ifndef PASSWORDDIALOG_H
#define PASSWORDDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

class PasswordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PasswordDialog(QWidget *parent = nullptr);
    ~PasswordDialog();

    QString getPassword() const;


protected:
    void showEvent(QShowEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
     void paintEvent(QPaintEvent *event) override;


private slots:
    void onAccept();
    void onCancel();
    void onPasswordChanged();
   // void onNumpadButtonClicked();
    void clearErrorMessage();
    void showErrorAnimation();

private:
    void setupUI();
    void setupNumpadLayout();
    void createAnimations();
    bool validatePassword(const QString &password);
    void handleFailedAttempt();
    void setErrorMessage(const QString &message);
    
    // UI Components
    QFrame *m_mainCard;
    QFrame *m_headerFrame;
    QFrame *m_inputFrame;
    QFrame *m_numpadFrame;
    QFrame *m_buttonFrame;
    
    QLabel *m_iconLabel;
    QLabel *m_titleLabel;
    QLabel *m_descLabel;
    QLineEdit *m_passwordEdit;
    QLabel *m_errorLabel;
    
    QPushButton *m_confirmBtn;
    QPushButton *m_cancelBtn;
    
    QList<QPushButton*> m_numpadButtons;
    QPushButton *m_clearBtn;
    QPushButton *m_backspaceBtn;
    
    // Logic
    QString m_correctPassword;
    int m_attempts;
    int m_maxAttempts;
    
    // Animations
    QPropertyAnimation *m_fadeAnimation;
    QPropertyAnimation *m_shakeAnimation;
    QGraphicsOpacityEffect *m_opacityEffect;
    QTimer *m_errorTimer;
};

#endif // PASSWORDDIALOG_H