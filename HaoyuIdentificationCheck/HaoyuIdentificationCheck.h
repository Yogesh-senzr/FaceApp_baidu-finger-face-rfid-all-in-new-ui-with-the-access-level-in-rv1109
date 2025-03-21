#ifndef HAOYUIDENTIFICATIONCHECK_H
#define HAOYUIDENTIFICATIONCHECK_H

#include <QObject>

class HaoyuIdentificationCheck : public QObject
{
    Q_OBJECT
public:
    explicit HaoyuIdentificationCheck(QObject *parent = nullptr);
public:
    Q_SLOT void slotPostFaceInfo(const float TemperatureValue, const int mask);
public:
    Q_SIGNAL void sigUserAuthResult(const bool, const QString);
};

#endif // HAOYUIDENTIFICATIONCHECK_H
