#ifndef CAMERAINFO_H
#define CAMERAINFO_H

#include <QObject>
#include <QSize>

class CameraInfo : public QObject
{
    Q_OBJECT
public:
    CameraInfo();
    QSize resolution;
    bool mirrored;
    qint32 rotation;
};

#endif // CAMERAINFO_H
