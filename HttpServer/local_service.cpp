//#include "local_service.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

#include <vector>
#include <algorithm>
#include <iostream>
#include <sstream>

#include <unistd.h>
#include "protocol.h"
#include "local_service.h"
#include "SharedInclude/GlobalDef.h"
#include "MessageHandler/Log.h"
#include "ManageEngines/PersonRecordToDB.h"

int local_service_init(char *ip_addr)
{
	start_papi_protocol(ip_addr);
	return 0;
}

int local_service_exit()
{
	
	stop_papi_protocol();

	return 0;
}

class LocalServicePrivate
{
	Q_DECLARE_PUBLIC(LocalService)
public:
	LocalServicePrivate(LocalService *dd);
private:
	LocalService * const q_ptr;
};

LocalServicePrivate::LocalServicePrivate(LocalService *dd) :
		q_ptr(dd)
{
	qRegisterMetaType<IdentifyFaceRecord_t>("IdentifyFaceRecord_t");

	QObject::connect(q_func(), &LocalService::sigAppRecordData, q_func(), &LocalService::slotAppRecordData);
}

void LocalService::slotAppRecordData(const IdentifyFaceRecord_t)
{
	
}

LocalService::LocalService(QObject *parent) :
		QObject(parent), d_ptr(new LocalServicePrivate(this))
{
}

LocalService::~LocalService()
{
}

