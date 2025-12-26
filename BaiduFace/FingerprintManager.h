#ifndef FINGERPRINTMANAGER_H
#define FINGERPRINTMANAGER_H

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QMutex>
#include <QScopedPointer>
#include <QWidget>
#include <QDateTime>

// Forward declarations
class FingerprintManagerPrivate;


class FingerprintManager : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(FingerprintManager)
    
public:
    
    explicit FingerprintManager(QObject *parent = nullptr);
    
   
    ~FingerprintManager();
    
   
    static inline FingerprintManager *GetInstance() {
        static FingerprintManager instance;
        return &instance;
    }
    
   
    bool initFingerprintSensor();
    

    bool isSensorReady() const;
    void startContinuousMonitoring();
    void stopContinuousMonitoring();
    bool isMonitoring() const;
    void checkForFingerprint();
    bool compareFingerprintWithDatabase();

    bool startEnrollment(uint16_t fingerId);
    

    bool identifyFingerprint(uint16_t &matchedFingerId, float &confidence);
    

    bool identifyAndShowResult(QWidget* parent = nullptr);
    
   
    bool deleteFingerprintTemplate(uint16_t fingerId);
    

    bool deleteAllFingerprints();
    
 
    int getTemplateCount();
    
    bool downloadFingerprintTemplate(uint16_t fingerId, QByteArray &templateData);
    
    bool uploadTemplateToSensor(uint16_t fingerId, const QByteArray &templateData);
    
   
    bool captureFingerprint(QByteArray &fingerprintTemplate);
    
    
    bool storeFingerprintTemplate(uint16_t fingerId, const QByteArray &templateData);
    
    
    bool downloadTemplateFromSensor(uint16_t fingerId, QByteArray &templateData);
    bool checkUARTHealth(); 
    

signals:
    
    void sigEnrollmentProgress(int stage, const QString &message);
    void sigEnrollmentComplete(uint16_t fingerId);
    void sigEnrollmentFailed(const QString &error);
    void sigFingerprintMatched(uint16_t fingerId, float confidence);
    void sigFingerprintNotMatched();
    void sigFingerDetected();
    void sigSensorError(const QString &error);
    void sigFingerprintMatched(const QString &personId, 
                              const QString &name, 
                              const int &confidence);
    void sigFingerprintEnrolled(const QString &personId, bool success);
    void sigFingerprintError(const QString &errorMsg);
    void sigFingerprintNotRecognized();
    
private:
    QScopedPointer<FingerprintManagerPrivate> d_ptr;
    QTimer *m_monitoringTimer;
    bool m_isMonitoring;
    
    // Prevent copying
    FingerprintManager(const FingerprintManager&) = delete;
    FingerprintManager& operator=(const FingerprintManager&) = delete;
};

#endif // FINGERPRINTMANAGER_H