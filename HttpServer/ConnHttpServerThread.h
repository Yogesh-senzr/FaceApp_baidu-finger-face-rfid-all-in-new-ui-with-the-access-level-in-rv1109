#ifndef _CONNHTTPSERVERTHREAD_H_
#define _CONNHTTPSERVERTHREAD_H_

#include <QtCore/QThread>
#include <QtCore/QWaitCondition>
#include <QtCore/QMutex>

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

public:
    bool sendUserToServer(const QString &person_uuid, const QString &name, 
    const QString &idCard, const QString &icCard, 
    const QString &sex, const QString,
    const QString &timeOfAccess, 
    const QByteArray &faceFeature);
public:
    // User synchronization functions
    void syncUsersWithServer();
    void syncMissingUsers();
    void fetchAndAddUser(const QString& uuid);

    void checkAndSyncUsers(const QString& heartbeatResponse);
    
    // Store last heartbeat response
    void setLastHeartbeatResponse(const QString& response);
    QString getLastHeartbeatResponse() const;

private:
    QString m_lastHeartbeatResponse;
                         
};

#endif // _CONNHTTPSERVERTHREAD_H_