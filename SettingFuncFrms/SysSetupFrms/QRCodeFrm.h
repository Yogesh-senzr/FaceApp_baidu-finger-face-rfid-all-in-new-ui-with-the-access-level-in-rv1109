#ifndef QRCODEFRM_H
#define QRCODEFRM_H

#include <QDialog>
#include <QLabel>

class QRCodeFrm : public QDialog
{
    Q_OBJECT
public:
    explicit QRCodeFrm(QWidget *parent = nullptr);
    virtual ~QRCodeFrm();  // Make destructor virtual
    void setSerialNumber();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void InitUI();
    QImage generateQRImage(const QString &text, int size = 400);

private:
    QLabel *m_pQRLabel;
    QPushButton *m_pBackButton;
    QString m_serialNumber;
};

#endif // QRCODEFRM_H
