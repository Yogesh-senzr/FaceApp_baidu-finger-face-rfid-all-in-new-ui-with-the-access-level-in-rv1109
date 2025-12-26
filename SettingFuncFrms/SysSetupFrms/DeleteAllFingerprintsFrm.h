#ifndef DELETEALLFINGERPRINTSFRM_H
#define DELETEALLFINGERPRINTSFRM_H

#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

class DeleteAllFingerprintsFrm : public QDialog
{
    Q_OBJECT
    
public:
    explicit DeleteAllFingerprintsFrm(QWidget *parent = nullptr);
    ~DeleteAllFingerprintsFrm();
protected:
    void showEvent(QShowEvent *event) override;
    
private slots:
    void onDeleteClicked();
    void onCancelClicked();
    
private:
    void initUI();
    
    QPushButton *m_pDeleteBtn;
    QPushButton *m_pCancelBtn;
    QLabel *m_pWarningLabel;
};

#endif // DELETEALLFINGERPRINTSFRM_H
