#include "QRCodeFrm.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <qrencode.h>
#include <QDateTime>
#include <QCloseEvent>
#include <QPushButton>
#include <QStyle>
#include "DeviceInfo.h" 

QRCodeFrm::QRCodeFrm(QWidget *parent)
    : QDialog(parent)
    , m_pQRLabel(nullptr)
    , m_pBackButton(nullptr)
{
    InitUI();
    setFixedSize(500, 500);
    setWindowTitle(tr("Device QR Code"));
    setWindowModality(Qt::ApplicationModal);
    setAttribute(Qt::WA_DeleteOnClose, false);
    setStyleSheet(
        "QDialog {"
        "    background-color: white;"
        "}"
        "QPushButton {"
        "    background-color: #0087BD;"  
        "    color: white;"
        "    border: none;"
        "    padding: 8px 16px;"
        "    min-width: 100px;"
        "    border-radius: 4px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #33A3D6;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #33A3D6;"
        "}"
    );
}

QRCodeFrm::~QRCodeFrm()
{
    if (m_pQRLabel) {
        delete m_pQRLabel;
        m_pQRLabel = nullptr;
    }
}

void QRCodeFrm::InitUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
 
    
    m_pQRLabel = new QLabel(this);
    m_pQRLabel->setAlignment(Qt::AlignCenter);
    m_pQRLabel->setMinimumSize(400, 400);
    layout->addWidget(m_pQRLabel);

    QHBoxLayout *buttonLayout = new QHBoxLayout;

    m_pBackButton = new QPushButton(tr("Back"), this);
    m_pBackButton->setCursor(Qt::PointingHandCursor);
    connect(m_pBackButton, &QPushButton::clicked, this, &QDialog::accept);

    buttonLayout->addStretch();  // Push buttons to center
    buttonLayout->addWidget(m_pBackButton);
    buttonLayout->addStretch();

    layout->addLayout(buttonLayout);  

}

QImage QRCodeFrm::generateQRImage(const QString &text, int outputSize)
{
    // Use high error correction level
    QRcode *qrcode = QRcode_encodeString(text.toUtf8().constData(), 
                                        0,               // Auto version
                                        QR_ECLEVEL_H,    // Highest error correction
                                        QR_MODE_8,       // 8-bit encoding
                                        1);              // Case sensitive

    if (!qrcode) {
        return QImage();
    }

    // Calculate scaling factors
    int scale = 10;  // Each QR module will be 10x10 pixels
    int quietZone = 40;  // White border around QR code
    int size = qrcode->width * scale + 2 * quietZone;
    
    // Create base image
    QImage image(size, size, QImage::Format_RGB32);
    image.fill(Qt::white);

    QPainter painter(&image);
    
    // Draw orange border
    painter.fillRect(0, 0, size, size, QColor(0, 135, 189));
    painter.fillRect(10, 10, size-20, size-20, Qt::white);

    // Draw QR code modules
    for(int y = 0; y < qrcode->width; y++) {
        for(int x = 0; x < qrcode->width; x++) {
            if(qrcode->data[y * qrcode->width + x] & 0x01) {
                QRectF r(quietZone + x * scale, 
                        quietZone + y * scale, 
                        scale, 
                        scale);
                painter.fillRect(r, Qt::black);
            }
        }
    }

    QRcode_free(qrcode);

    // Scale to desired output size while maintaining aspect ratio
    return image.scaled(outputSize, outputSize, 
                       Qt::KeepAspectRatio, 
                       Qt::SmoothTransformation);
}

void QRCodeFrm::setSerialNumber()
{
    // Retrieve serial number from DeviceInfo
    QString serialNumber = DeviceInfo::getInstance()->getSerialNumber();

    // Format the data in a standard way
    QString qrData = QString("Device Name:AI Face Galaxy\n"
                             "Device Type:Face\n"
                             "Door Type:1-Door\n"
                             "SN:%1\n"
                             "Time:%2")
                           .arg(serialNumber)
                           .arg(QDateTime::currentDateTime()
                                .toString("yyyy-MM-dd hh:mm:ss"));
    
    QImage qrImage = generateQRImage(qrData);
    if (!qrImage.isNull()) {
        m_pQRLabel->setPixmap(QPixmap::fromImage(qrImage));
    }
}

void QRCodeFrm::closeEvent(QCloseEvent *event)
{
    done(0);  // This will properly close the dialog
    event->accept();
}
