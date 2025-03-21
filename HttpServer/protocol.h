#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_
#include<pthread.h>

struct PAPIConfig {
	int port;
	std::string ip_addr;
	std::string password;
	std::string heartbeat_url;
	std::string sn;
	std::string mac;
	std::string firmware_version;
	std::string log_conf_path;
};

void start_papi_protocol(char *ip_addr);
void stop_papi_protocol();

//int post_record_to_http(unsigned char *buf,int size,int person_id, char *jpg_name);
#endif

