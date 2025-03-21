// PersonRegistrationThread.h
#ifndef PERSONREGISTRATIONTHREAD_H
#define PERSONREGISTRATIONTHREAD_H

#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>
#include "json-cpp/json.h"

class PersonRegistrationThreadPrivate;

class PersonRegistrationThread : public QThread
{
    Q_OBJECT
public:
    static PersonRegistrationThread* GetInstance();
    ~PersonRegistrationThread();

    // Method to register a new person
    // Extended signature to support message type, SN, and command ID
    bool registerPerson(const Json::Value& personData, 
                        const std::string& msgType = "",
                        const std::string& sn = "",
                        const std::string& cmdId = "");

protected:
    void run() override;

private:
    explicit PersonRegistrationThread(QObject *parent = nullptr);
    
    Q_DECLARE_PRIVATE(PersonRegistrationThread)
    QScopedPointer<PersonRegistrationThreadPrivate> d_ptr;
    
    static PersonRegistrationThread* m_pInstance;
    static QMutex m_mutex;
};

#endif // PERSONREGISTRATIONTHREAD_H
