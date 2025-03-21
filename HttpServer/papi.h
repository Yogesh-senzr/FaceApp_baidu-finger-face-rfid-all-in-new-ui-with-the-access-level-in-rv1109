/**********************************************
*
* Jun, 29, 2019
*
* Jacksong @ ReadSense
* jack.song@readsense.cn
*
* offline API for Pad used in LAN network
*
**********************************************/



#ifndef _PAPI_H_
#define _PAPI_H_


#ifdef __cplusplus
extern "C" {
#endif

#define PAPI_STR_LEN 128
#define PAPI_STR_LEN64 64
#define PAPI_ULR_LEN 512
#define MAX_NUM_PER_PAGE 50
#define MSG_MAX_LENGTH 1024

/////////////////////////////////////
// Error Code definition
/////////////////////////////////////
#define PAPI_ERR_OK 1
#define PAPI_ERR_CALLBACK_NOT_EXISTS 101
#define PAPI_ERR_INVALID_PASSWORD 102
#define PAPI_ERR_INVALID_PARAMS 103
#define PAPI_ERR_UNKNOW 1004


#define PAPI_SUCCESS 1
#define PAPI_FAIL    0


#define NET_MODE_LAN 0
#define NET_MODE_WIFI 1

#define NET_DHCP 0
#define NET_STATIC_IP 1

/////////////////////////////
// CMD
/////////////////////////////
enum OP_PAPI_CMD
{
	PAPI_CMD_PASSWORD_UPDATE = 0,
	PAPI_CMD_NET_CONFIG,
	PAPI_CMD_DEVICE_RESTART,
	PAPI_CMD_HEARTBEAT_SET,
	PAPI_CMD_RECORD_CALLBACK,
	PAPI_CMD_RECORDS_DELETE_ALL,

	PAPI_CMD_PEOPLE_ADD = 100,
	PAPI_CMD_PEOPLE_UPDATE,
	PAPI_CMD_PEOPLE_DELETE,
	PAPI_CMD_PEOPLE_FIND,
	PAPI_CMD_PEOPLE_GET_ALL,

};

typedef struct papi_response_t {
	int  status_code;
	int  success; // 0: fail, 1: success
}papi_response;

typedef struct papi_request_filedata_t{
	int file_size;
	void *file_data;
	char file_info[32];
	char file_md5[32];
}papi_request_filedata;

typedef struct papi_response_with_msg_t {
	int  status_code;
	int  success; // 0: fail, 1: success
	char message[128];     // message should < 128 size;
}papi_response_with_msg;


typedef struct papi_response_with_versioninfo_t {
	int  status_code;
	int  success; // 0: fail, 1: success
	char active_state[16];
	char device_sn[33];
	char algo_version[64];
	char device_version[64];
}papi_response_with_versioninfo;


typedef struct papi_response_with_photodata_t {
	int  	status_code;
	int  	success; // 0: fail, 1: success
	int 	rgb_size;
	void 	*rgb_data;
	int 	ir_size;
	void 	*ir_data;
	char message[128];
}papi_response_with_photodata;


// password update
typedef struct papi_request_password_t {
	char old_password[PAPI_STR_LEN];
	char new_password[PAPI_STR_LEN];
}papi_request_password;



typedef struct papi_request_opendoor_t {
	int delay_relay_time;
	char pcmName[32];
}papi_request_opendoord;

// callback setting
typedef struct papi_request_record_callback_t {
	char callback_url[PAPI_ULR_LEN];
}papi_request_record_callback, papi_request_heartbeat_url;

// callback setting
typedef struct papi_request_set_callback_t {
	int type;  //0 识别回调 //1 注册照片回调  //2 心跳地址回调
	char callback_url[PAPI_ULR_LEN];
}papi_request_set_callback;


// network config
typedef struct papi_request_net_config_t {
	int net_mode;  
	int dhcp; 
	char ip_addr[PAPI_STR_LEN];
	char gateway[PAPI_STR_LEN];
	char subnet_mask[PAPI_STR_LEN];
	char dns1[PAPI_STR_LEN];
	char dns2[PAPI_STR_LEN];
	char ssid[PAPI_STR_LEN];
	char password[PAPI_STR_LEN];
}papi_request_net_config;

typedef struct papi_response_net_config_t {
	int net_mode;  
	int dhcp; 
	char ip_addr[PAPI_STR_LEN];
	char gateway[PAPI_STR_LEN];
	char subnet_mask[PAPI_STR_LEN];
	char dns1[PAPI_STR_LEN];
	char dns2[PAPI_STR_LEN];

    int  status_code;
    int  success; // 0: fail, 1: success
    char message[MSG_MAX_LENGTH];
}papi_response_net_config;

//device config
typedef struct papi_request_dev_config_t {

	char dev_name[PAPI_STR_LEN];
	char location[PAPI_STR_LEN];
	char save_crop[PAPI_STR_LEN];
	char sleep_time[PAPI_STR_LEN];
	char visit_record[PAPI_STR_LEN];
	char id_record[PAPI_STR_LEN];
	char id_method[PAPI_STR_LEN];
	char check_bt[PAPI_STR_LEN];
	char temp_detect[PAPI_STR_LEN];
	char current_lang[PAPI_STR_LEN];
	char pass_start_time[PAPI_STR_LEN];
	char pass_end_time[PAPI_STR_LEN];
	char flash_range[PAPI_STR_LEN];
	char temp_mode[PAPI_STR_LEN];
	char delay_relay_time[PAPI_STR_LEN];
	char show_ic_name[PAPI_STR_LEN];
	char enable_idcard[PAPI_STR_LEN];
	char enable_have_face[PAPI_STR_LEN];
	char settings_password[PAPI_STR_LEN];
}papi_request_dev_config;

typedef struct papi_response_dev_config_t {

	char dev_name[PAPI_STR_LEN];
	char location[PAPI_STR_LEN];
	char support_language[PAPI_STR_LEN];
	char save_crop[PAPI_STR_LEN];
	char sleep_time[PAPI_STR_LEN];
	char visit_record[PAPI_STR_LEN];
	char id_record[PAPI_STR_LEN];
	char temp_detect[PAPI_STR_LEN];
	char check_bt[PAPI_STR_LEN];
	char id_method[PAPI_STR_LEN];
	char pass_start_time[PAPI_STR_LEN];
	char pass_end_time[PAPI_STR_LEN];
	char current_lang[PAPI_STR_LEN];
	char flash_range[PAPI_STR_LEN];
	char temp_mode[PAPI_STR_LEN];
	char delay_relay_time[PAPI_STR_LEN];
	char show_ic_name[PAPI_STR_LEN];
	char enable_idcard[PAPI_STR_LEN];
	char enable_have_face[PAPI_STR_LEN];
	char settings_password[PAPI_STR_LEN];
	int  status_code;
	int  success; // 0: fail, 1: success
	char message[MSG_MAX_LENGTH];
}papi_response_dev_config;

typedef struct papi_request_algo_setting_t {
	char ir_threshold[8];
	char sim_threshold[8];
	char rgb_threshold[8];
	char id_threshold[8];
}papi_request_algo_setting;


// people add and udpate
typedef struct papi_request_people_add_t {
	char person_uuid[128];  // unique-id from 3rdparty systems
	char id_card_no[PAPI_STR_LEN64];
	char card_no[PAPI_STR_LEN64];
	char name[PAPI_STR_LEN64];
	char group[PAPI_STR_LEN];
	char male[8];
	char add_time[128];
	char feature[128*4];
	int person_type;
	unsigned char *image_data; // tmp file 
	int image_data_size;
	char auth_type[PAPI_STR_LEN64];
	char time_range[PAPI_STR_LEN64];
	char time_data[PAPI_STR_LEN64];

}papi_request_people_add, papi_request_person_info;

#define PAPI_MAX_GROUP_NUM 20
typedef struct papi_response_get_group_t {
	int  status_code;
	int  success; // 0: fail, 1: success
	int  group_size;
	char grop_data[PAPI_MAX_GROUP_NUM][32] ;
}papi_response_get_group;


typedef struct papi_response_person_info_t {
	char person_uuid[PAPI_STR_LEN];  // unique-id from 3rdparty systems
	char id_card_no[PAPI_STR_LEN];
	char card_no[PAPI_STR_LEN];
	char name[PAPI_STR_LEN];
	char group[PAPI_STR_LEN];
	char male[8];
	char add_time[128];
	int person_type;

	int  status_code;
    int  success; // 0: fail, 1: success
    char message[MSG_MAX_LENGTH];
}papi_response_person_info;


// people delete
typedef struct papi_request_people_delete_t {
	int person_type;
	char person_uuids[PAPI_STR_LEN * MAX_NUM_PER_PAGE]; // max num of person to be deleted at once
}papi_request_people_delete;

typedef struct papi_response_people_delete_t {
	char effective_uuids[PAPI_STR_LEN * MAX_NUM_PER_PAGE]; 
	char invalid_uuids[PAPI_STR_LEN * MAX_NUM_PER_PAGE]; 
	int effective_count;
	int invalid_count;
        int status_code;
        int success; // 0: fail, 1: success
        char message[MSG_MAX_LENGTH];
}papi_response_people_delete;

// people find and get_all
typedef struct papi_request_people_get_all_t {
	int page;
	int per_page; // max 50
	char person_uuid[PAPI_STR_LEN];
}papi_request_people_get_all;

typedef struct papi_request_people_find_t
{
	int type; //1,by name ;2,by group; 3,by uid;4,by add time;
	int person_type;
	int page;
	int per_page; // max 50
	char name[PAPI_STR_LEN];
	char group[PAPI_STR_LEN];
	char uid[PAPI_STR_LEN];
	char start_time[PAPI_STR_LEN];
	char end_time[PAPI_STR_LEN];
}papi_request_people_find;


typedef struct papi_request_people_get_by_uncertain_value
{
	int page;
	int per_page; // max 50
	char person_uuid[PAPI_STR_LEN];
	char person_name[PAPI_STR_LEN];
}papi_request_people_get_by_uncertain_value;


typedef struct papi_response_people_get_by_condition
{
	int status_code;
	int success;
	char message[MSG_MAX_LENGTH];
	int total_page;
	int current_page;
	int per_page;
	long long count;
	papi_request_person_info people_records[MAX_NUM_PER_PAGE];
}papi_response_people_get_by_condition, papi_response_people_get_all;



typedef struct papi_request_people_set_pass_permission
{
    char person_uuid[PAPI_STR_LEN];
    char auth_type[PAPI_STR_LEN];
    char data[PAPI_STR_LEN*2];
    char time_range[MSG_MAX_LENGTH];
    int pass_flag;
	int person_type;
}papi_request_people_set_pass_permission;

typedef struct papi_request_people_get_pass_permission
{
	int person_type;
    char person_uuid[PAPI_STR_LEN];
}papi_request_people_get_pass_permission;

typedef struct papi_response_people_get_pass_permission
{
    char person_uuid[PAPI_STR_LEN];
    char auth_type[PAPI_STR_LEN];
    char data[PAPI_STR_LEN*2];
    char time_range[MSG_MAX_LENGTH];
    int pass_flag;
	int person_type;
	int status_code;
	int success; // 0: fail, 1: success
	char message[MSG_MAX_LENGTH];	
}papi_response_people_get_pass_permission;

typedef struct papi_request_people_delete_pass_permission
{
	int person_type;
    char person_uuid[PAPI_STR_LEN];
}papi_request_people_delete_pass_permission;

typedef struct papi_request_device_set_pass_permission
{
    char auth_type[PAPI_STR_LEN];
    char data[PAPI_STR_LEN*2];
    char time_range[MSG_MAX_LENGTH];
    int pass_flag;
} papi_request_device_set_pass_permission;



//device update
//device config
typedef struct papi_request_dev_update_t {

	char fw_version[PAPI_STR_LEN];
	unsigned char *fw_data; // tmp file 
	int data_size;
}papi_request_dev_update;

typedef struct papi_response_dev_update_t {

	char fw_version[PAPI_STR_LEN];

	int  status_code;
	int  success; // 0: fail, 1: success
	char message[MSG_MAX_LENGTH];
}papi_response_dev_update;


//record delete
typedef struct papi_request_record_delete_t {
	int type; //1:delete by name 2:delete by uid 3:delete by time 4:delete by rID
	int person_type;
	long rID;
	char uuid[PAPI_STR_LEN];
	char name[PAPI_STR_LEN];
	char start_time[PAPI_STR_LEN];
	char end_time[PAPI_STR_LEN];
}papi_request_records_delete;


//record select
typedef struct papi_response_record_info_t {
	int face_mask;
	int person_type;
	long rID;
	char gender[32];
	char id_card_no[PAPI_STR_LEN64];
	char card_no[PAPI_STR_LEN64];	
	char uuid[PAPI_STR_LEN];  // unique-id from 3rdparty systems
	char image[PAPI_STR_LEN]; //image file name
	char name[PAPI_STR_LEN];
	char group[PAPI_STR_LEN];
	char time[PAPI_STR_LEN];
	char temp_value[8];

	int  status_code;
    int  success; // 0: fail, 1: success
    char message[MSG_MAX_LENGTH];
}papi_response_record_info;

typedef struct papi_request_record_select_t{
	int type; // 1:select by name 2:select by time
	int page;
	int per_page; // max 50
	int person_type;
	char name[PAPI_STR_LEN];
	char uuid[PAPI_STR_LEN];
	char start[PAPI_STR_LEN]; 	//start of time
	char end[PAPI_STR_LEN];		// end of time
}papi_request_records_select;

typedef struct papi_response_record_select_t{
	int total_page;
	int current_page;
	int per_page;
	int count;
	char location[PAPI_STR_LEN];
	char dev_name[PAPI_STR_LEN];
	papi_response_record_info people_records[MAX_NUM_PER_PAGE]; //TODO

	int  status_code;
	int  success; // 0: fail, 1: success
	char message[MSG_MAX_LENGTH];
}papi_response_records_select;


typedef struct papi_request_device_threshold_t{
	char threshold[PAPI_STR_LEN];
}papi_request_device_threshold;

typedef struct papi_response_device_threshold_t{
	char threshold[PAPI_STR_LEN];

	int  status_code;
	int  success; // 0: fail, 1: success
	char message[MSG_MAX_LENGTH];
}papi_response_device_threshold;


typedef struct papi_request_device_alive_level_t{
	char alive_level[PAPI_STR_LEN];
}papi_request_device_alive_level;

typedef struct papi_response_device_alive_level_t{
	char alive_level[PAPI_STR_LEN];

	int  status_code;
	int  success; // 0: fail, 1: success
	char message[MSG_MAX_LENGTH];
}papi_response_device_alive_level;

typedef struct papi_request_device_timesync_t{
	char time[PAPI_STR_LEN];
}papi_request_device_timesync;

typedef struct papi_response_device_timesync_t{
	char time[PAPI_STR_LEN];

	int  status_code;
	int  success; // 0: fail, 1: success
	char message[MSG_MAX_LENGTH];
}papi_response_device_timesync;


typedef struct papi_request_device_mode_config_t
{
    char working_mode[PAPI_STR_LEN];
    char orion_server[PAPI_STR_LEN];
    int  orion_port;
    char app_key[PAPI_STR_LEN];
    char app_secret[PAPI_STR_LEN];
}papi_request_device_mode_config;

typedef struct papi_response_device_get_error_code_t{
    char error_code[PAPI_STR_LEN];
    papi_response_with_msg_t state_msg;
}papi_response_device_get_error_code;


typedef struct papi_request_httpserver_info{
   char http_passwd[PAPI_STR_LEN];
   char http_server_url[256];
};





/////////////////////////////
// callback functions
/////////////////////////////

typedef void (*papi_method_handler_ptr)(void* request, void *response);

////////////////
// port, password, heartbeat_url, 
// sn, mac, 
////////////////

int papi_v1_start_server(
    int port,
    const char *ip_addr,
    const char *password,
    const char *heartbeat_url,
    const char *sn,
    const char *mac,
    const char *firmware_version,
    const char *log_conf_path
    );

int papi_v1_stop_server();


// papi_v1_on_device_update_password
void papi_v1_on_device_update_password(papi_method_handler_ptr handler);

// papi_v1_on_device_get_versioninfo
void papi_v1_on_device_get_versioninfo(papi_method_handler_ptr handler);


void papi_v1_on_device_get_picture(papi_method_handler_ptr handler);

void papi_v1_on_device_do_active(papi_method_handler_ptr handler);

void papi_v1_on_device_do_update(papi_method_handler_ptr handler);

// papi_v1_on_device_net_config
void papi_v1_on_device_net_config(papi_method_handler_ptr handler);


// papi_v1_on_device_restart
void papi_v1_on_device_restart(papi_method_handler_ptr handler);


// papi_v1_on_device_open_door
void papi_v1_on_device_open_door(papi_method_handler_ptr handler);


// papi_v1_on_device_heart_beat_config
void papi_v1_on_device_heart_beat_config(papi_method_handler_ptr handler);


// papi_v1_on_device_record_callback
void papi_v1_on_device_record_callback(papi_method_handler_ptr handler);


// papi_v1_on_device_delete_all_records
void papi_v1_on_device_delete_all_records(papi_method_handler_ptr handler);


// papi_v1_on_people_add
void papi_v1_on_people_add(papi_method_handler_ptr handler);


// papi_v1_on_people_update
void papi_v1_on_people_update(papi_method_handler_ptr handler);


// papi_v1_on_people_delete
void papi_v1_on_people_delete(papi_method_handler_ptr handler);

// delete all person.
void papi_v1_on_people_delete_all(papi_method_handler_ptr handler);

// papi_v1_on_people_find
void papi_v1_on_people_find(papi_method_handler_ptr handler);

// papi_v1_on_people_get_all_group
void papi_v1_on_people_get_all_group(papi_method_handler_ptr handler);

// papi_v1_on_people_get_all
void papi_v1_on_people_get_all(papi_method_handler_ptr handler);

//papi_v1_on_device_config
void papi_v1_on_device_config(papi_method_handler_ptr handler);

//papi_v1_on_device_config
void papi_v1_on_getdevice_config(papi_method_handler_ptr handler);

//papi_v1_on_device_update
void papi_v1_on_device_update(papi_method_handler_ptr handler);

// papi_v1_on_record_delete
void papi_v1_on_record_delete(papi_method_handler_ptr handler);

// papi_v1_on_record_select
void papi_v1_on_record_select(papi_method_handler_ptr handler);

// papi_v1_on_people_find_by_time
void papi_v1_on_people_find_by_time(papi_method_handler_ptr handler);

// papi_v1_on_people_find_by_name_or_uuid
void papi_v1_on_people_find_by_uncertain_value(papi_method_handler_ptr handler);

// papi_v1_on_set_peole_pass_permission
void papi_v1_on_people_set_pass_permission(papi_method_handler_ptr handler);

// papi_v1_on_get_peole_pass_permission
void papi_v1_on_people_get_pass_permission(papi_method_handler_ptr handler);

// papi_v1_on_set_device_pass_permission
void papi_v1_on_device_set_pass_permission(papi_method_handler_ptr handler);

// papi_v1_on_delete_peole_pass_permission
void papi_v1_on_people_delete_pass_permission(papi_method_handler_ptr handler);

// papi_v1_on_delete_device_pass_permission
void papi_v1_on_device_delete_pass_permission(papi_method_handler_ptr handler);


//papi_v1_on_device_threshold
void papi_v1_on_device_threshold(papi_method_handler_ptr handler);

//papi_v1_on_device_alive_level
void papi_v1_on_device_alive_level(papi_method_handler_ptr handler);

//papi_v1_on_device_timesync
void papi_v1_on_device_timesync(papi_method_handler_ptr handler);

// papi_v1_on_device_mode_config
void papi_v1_on_device_mode_config(papi_method_handler_ptr handler);

// papi_v1_on_device_reset_factory_settings
void papi_v1_on_device_reset_factory_settings(papi_method_handler_ptr handler);

// papi_v1_on_device_del_userdata
void papi_v1_on_device_del_userdata(papi_method_handler_ptr handler);

void papi_v1_on_algo_reset(papi_method_handler_ptr handler);

void papi_v1_on_algo_setting(papi_method_handler_ptr handler);

void papi_v1_on_device_httpserver_setting(papi_method_handler_ptr handler);
void papi_v1_on_device_time_setting(papi_method_handler_ptr handler);
void papi_v1_on_device_record_url_setting(papi_method_handler_ptr handler);
// papi_v1_on_device_get_error_code
void papi_v1_on_device_get_error_code(papi_method_handler_ptr handler);

void papi_face_data_set_device_connect_password(const char* password);

#ifdef __cplusplus
}
#endif



#endif
