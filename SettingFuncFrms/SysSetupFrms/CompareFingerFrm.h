#ifndef COMPAREFINGERFRM_H
#define COMPAREFINGERFRM_H

#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

class CompareFingerFrm : public QDialog
{
    Q_OBJECT

public:
    explicit CompareFingerFrm(QWidget *parent = nullptr);
    ~CompareFingerFrm();

private slots:
    void onCompareClicked();
    void onBackClicked();

private:
    void initUI();
    void initConnect();

private:
    QPushButton *m_pCompareBtn;
    QPushButton *m_pBackBtn;
    QLabel *m_pStatusLabel;
};

#endif // COMPAREFINGERFRM_H
