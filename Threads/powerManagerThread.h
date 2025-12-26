#ifndef POWERMANAGERTHREAD_H
#define POWERMANAGERTHREAD_H

#include <QObject>

class QTimer;
class powerManagerThread : public QObject
{
    Q_OBJECT
public:
    powerManagerThread(QObject *parent = Q_NULLPTR);
    ~powerManagerThread();
public:
    void setFillLightMode(const int &mode);
    //识别状态
    void setIdentifyState(const bool);
public:
    void setRecognitionInProgress(bool inProgress);
public:
    Q_SLOT void slotDisMissMessage(const bool state);//当前无人脸（false没人脸， true有人脸）
private:
    Q_SLOT void slotFillLightTimerOut();
private:
    int mFillLightMode;//补光模式
private:
    bool mRecognitionInProgress = false;
private:
    QTimer *m_pFillLightTimer;//补光定时器
};

#endif // POWERMANAGERTHREAD_H
