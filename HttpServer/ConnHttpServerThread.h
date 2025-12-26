#ifndef _CONNHTTPSERVERTHREAD_H_
#define _CONNHTTPSERVERTHREAD_H_

#include <QtCore/QThread>
#include <QtCore/QWaitCondition>
#include <QtCore/QMutex>
#include <QtCore/QTimer> // Add QTimer for cleanupTimer
#include <QtCore/QQueue>
#include <QtCore/QMap>
#include <QtCore/QString>
#include "DB/RegisteredFacesDB.h"

class ConnHttpServerThreadPrivate;
class ConnHttpServerThread : public QThread
{
    Q_OBJECT
public:
	ConnHttpServerThread(QObject *parent = Q_NULLPTR);
    ~ConnHttpServerThread();
public:
    static inline ConnHttpServerThread *GetInstance(){static ConnHttpServerThread g;return &g;}
private:
    void run();

public:
    bool reportDeviceInfo();

private:
    QScopedPointer<ConnHttpServerThreadPrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(ConnHttpServerThread)
    Q_DISABLE_COPY(ConnHttpServerThread)

private:
    static const int SYNC_BATCH_SIZE = 3;  // Process 3 users at a time
    static const int SYNC_BATCH_DELAY = 100; // 100ms delay between batches

    // Constants for limited queue
    static const int MAX_QUEUE_LENGTH = 15;            // Maximum queue size
    static const int QUEUE_REFILL_THRESHOLD = 5;       // When to refill queue
    static const int MIN_QUEUE_LENGTH = 3; 

    // Helper functions for queue-based synchronization
    bool fillQueueToLimit(QQueue<QString>& queue, 
                         QMap<QString, QString>::iterator& iterator,
                         const QMap<QString, QString>::iterator& end,
                         const QMap<QString, QString>& localUserMap);
    
    bool shouldSyncUser(const QString& employeeId, 
                       const QString& serverTimeUTC,
                       const QMap<QString, QString>& localUserMap);
    
    void processQueueBatch(QQueue<QString>& queue, int& successCount, int& failCount);
    
    // Progress tracking
    void updateSyncProgress(int processed, int total, int queued);
    void debugPrintQueueStatus(const QQueue<QString>& queue, 
                              const QString& context);
    bool isSyncInProgress() const;

public:
    bool sendUserToServer(const QString &person_uuid, const QString &name, 
    const QString &idCard, const QString &icCard, 
    const QString &sex, const QString &department,
    const QString &timeOfAccess, 
    const QByteArray &faceFeature);
    float getHeartbeatLivenessThreshold() const;
    float getHeartbeatQualityThreshold() const; 
    float getHeartbeatComparisonThreshold() const;
    bool hasHeartbeatThresholds() const;
    int getHeartbeatIdentificationInterval() const;    // Add this
    int getHeartbeatRecognitionDistance() const; 
public:
    // User synchronization functions
    void syncUsersWithServer();
    void syncMissingUsers();
    void removeExtraUsers();
    QDateTime getLocalLastModifiedTime();

    void checkAndSyncUsers(const QString& heartbeatResponse);
    
    // Store last heartbeat response
    void setLastHeartbeatResponse(const QString& response);
    QString getLastHeartbeatResponse() const;
    void updateSyncDisplay(const QString &status, int currentCount, int totalCount);
    // Add this new method for updating tenant name
    bool isServerReachable();
    void updateTenantName(const QString &tenantName);
    void updateLocalFaceCount();
    bool storeServerDateUpdatedForUser(const QString& employeeId, const QString& serverDateUpdated);
    QDateTime getDeviceLastModifiedTime();
    bool determineIfFirstTimeSync();
    QString getLastSyncTime();
    void updateLastSyncTime();
public slots:
    void syncIndividualUser(const QString& employeeId);
private:
    QString m_lastHeartbeatResponse;

private:
    QDateTime convertUTCToIST(const QString& utcTimeStr);
    QString findUuidByEmployeeId(const QString& employeeId);
    void performFullSync(int serverCount = 0);
    int getValidUserCount(const QList<PERSONS_t>& users);
    QMap<QString, QString> getServerUserList();
    bool fetchAndAddUser(const QString& employeeId, const QString& serverTimeFromList = "");
    void debugPrintUserCounts();
    void debugPrintSyncConfig();
    QDateTime convertServerUTCToIST(const QString& utcTimeStr);
    void initializeCleanup();
    void cleanupTempFiles();

    QTimer* cleanupTimer;

private slots:
    void performPeriodicCleanup();

signals:
    void syncStatusChanged(const QString &status);
    void syncUserCountChanged(int current, int total);
    void serverTimeUpdated(const QDateTime& istTime);
    void lastSyncTimeChanged(const QString &time);
    void userSyncCompleted(const QString& employeeId, bool success);
                         
};

#endif // _CONNHTTPSERVERTHREAD_H_