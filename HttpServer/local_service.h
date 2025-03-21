#ifndef _LOCAL_SERVICE_H_
#define _LOCAL_SERVICE_H_

#include <QtCore/QObject>
#include "SharedInclude/GlobalDef.h"

int local_service_init(char *ip_addr);
int local_service_exit();

class LocalServicePrivate;
class LocalService : public QObject
{
    Q_OBJECT
public:
	LocalService(QObject *parent = Q_NULLPTR);
    ~LocalService();

public:
    void appRecordData(const IdentifyFaceRecord_t &t){emit sigAppRecordData(t);}
public:
    Q_SIGNAL void sigAppRecordData(const IdentifyFaceRecord_t);
private:
    Q_SLOT void slotAppRecordData(const IdentifyFaceRecord_t);

public:
    static inline LocalService *GetInstance(){static LocalService g;return &g;}
private:
    QScopedPointer<LocalServicePrivate>d_ptr;
private:
    Q_DECLARE_PRIVATE(LocalService)
    Q_DISABLE_COPY(LocalService)
};

#endif
