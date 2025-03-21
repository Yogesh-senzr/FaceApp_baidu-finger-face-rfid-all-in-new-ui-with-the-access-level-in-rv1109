// QRCodeGenerator.h
#ifndef QRCODEGENERATOR_H
#define QRCODEGENERATOR_H

#include <QImage>
#include <QString>
#include <vector>

class QRCodeGenerator {
public:
    QImage generateQR(const QString& text, int size = 250);

private:
    static const int VERSION = 3;  // Version 3 QR Code, 29x29 modules
    static const int SIZE = 29;    // Module count for version 3
    
    std::vector<bool> generateQRData(const QString& text);
    void addFinderPattern(std::vector<bool>& qr, int x, int y);
    void addAlignmentPattern(std::vector<bool>& qr, int x, int y);
    void addTimingPatterns(std::vector<bool>& qr);
    void addFormatInfo(std::vector<bool>& qr);
};

#endif

