#ifndef FINGERDEBUGFRM_H
#define FINGERDEBUGFRM_H

#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QDebug>
#include <cstdint>

class FingerDebugFrm : public QDialog
{
    Q_OBJECT

public:
    explicit FingerDebugFrm(QWidget *parent = nullptr);
    ~FingerDebugFrm();

private slots:
    void onDebugButtonClicked();
    void onTestCompleted(bool success, const QString& message);
    void onStatusUpdate(const QString& status);

private:
    void initUI();
    void initConnect();

private:
    void setupWorkerThread();
    
    QPushButton *m_pDebugButton;
    QLabel *m_pStatusLabel;
    QThread *m_pWorkerThread;        // Add this
    class FingerprintWorker *m_pWorker;  // Add this
};

#endif // FINGERDEBUGFRM_H
