// QRCodeGenerator.cpp
#include "QRCodeGenerator.h"
#include <QPainter>

QImage QRCodeGenerator::generateQR(const QString& text, int size)
{
    // Generate QR data
    std::vector<bool> qrData = generateQRData(text);
    
    // Create QImage
    QImage image(size, size, QImage::Format_RGB32);
    image.fill(Qt::white);
    
    QPainter painter(&image);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    
    double scale = size / (double)SIZE;
    
    // Draw QR code
    for(int y = 0; y < SIZE; y++) {
        for(int x = 0; x < SIZE; x++) {
            if(qrData[y * SIZE + x]) {
                QRectF r(x * scale, y * scale, scale, scale);
                painter.drawRect(r);
            }
        }
    }
    
    return image;
}

std::vector<bool> QRCodeGenerator::generateQRData(const QString& text)
{
    std::vector<bool> qr(SIZE * SIZE, false);
    
    // Add finder patterns
    addFinderPattern(qr, 0, 0);
    addFinderPattern(qr, SIZE - 7, 0);
    addFinderPattern(qr, 0, SIZE - 7);
    
    // Add alignment pattern
    addAlignmentPattern(qr, SIZE - 9, SIZE - 9);
    
    // Add timing patterns
    addTimingPatterns(qr);
    
    // Add format information
    addFormatInfo(qr);
    
    // Add simple data pattern (for demonstration)
    int dataStart = 8;
    for(int i = 0; i < text.length() && i < 10; i++) {
        int ch = text[i].toLatin1();
        for(int bit = 0; bit < 8; bit++) {
            int pos = (dataStart + i * 8 + bit);
            int x = pos % SIZE;
            int y = pos / SIZE;
            qr[y * SIZE + x] = (ch & (1 << bit)) != 0;
        }
    }
    
    return qr;
}

void QRCodeGenerator::addFinderPattern(std::vector<bool>& qr, int x, int y)
{
    // Draw 7x7 finder pattern
    for(int dy = 0; dy < 7; dy++) {
        for(int dx = 0; dx < 7; dx++) {
            if((dx == 0 || dx == 6 || dy == 0 || dy == 6) || 
               (dx >= 2 && dx <= 4 && dy >= 2 && dy <= 4)) {
                qr[(y + dy) * SIZE + (x + dx)] = true;
            }
        }
    }
}

void QRCodeGenerator::addAlignmentPattern(std::vector<bool>& qr, int x, int y)
{
    // Draw 5x5 alignment pattern
    for(int dy = 0; dy < 5; dy++) {
        for(int dx = 0; dx < 5; dx++) {
            if(dx == 0 || dx == 4 || dy == 0 || dy == 4 || (dx == 2 && dy == 2)) {
                qr[(y + dy) * SIZE + (x + dx)] = true;
            }
        }
    }
}

void QRCodeGenerator::addTimingPatterns(std::vector<bool>& qr)
{
    // Horizontal timing pattern
    for(int x = 8; x < SIZE - 8; x++) {
        if(x % 2 == 0) {
            qr[6 * SIZE + x] = true;
        }
    }
    
    // Vertical timing pattern
    for(int y = 8; y < SIZE - 8; y++) {
        if(y % 2 == 0) {
            qr[y * SIZE + 6] = true;
        }
    }
}

void QRCodeGenerator::addFormatInfo(std::vector<bool>& qr)
{
    // Simple format information (not actual QR code format info)
    for(int i = 0; i < 8; i++) {
        qr[8 * SIZE + i] = (i % 2 == 0);
        qr[i * SIZE + 8] = (i % 2 == 0);
    }
}
