#pragma once
#include <vector>
#include <string>
#include <map>
#include <boost/property_tree/ptree.hpp>

int get_invoker_id();
boost::property_tree::ptree read_config_json(std::string child);
bool is_controller();
std::vector<std::string> get_controller_ip(std::string ip);
std::vector<std::string> get_invoker_ip(std::string ip);
std::vector<std::string> get_all_invokers();
std::map<std::string, std::vector<std::string>> get_peer_ips();
std::string get_self_ip();

#define	IB_PORT	1
#define	SOCK_PORT_START	20000
#define	IB_SL	0
#define	IB_MTU	IBV_MTU_4096
#define PER_INV_IB_BUF_SIZE	2048
#define	INV_SOCK_BUF_SIZE	10
#define	LOCAL_OW_PORT	7778
// #define	REMOTE_OW_PORT	8889
#define	SOCK_BUF_SIZE	4096
#define	MON_PERIOD	30 // ms
#define	IDLE_THRESHOLD	3600*1000	// ms
#define	IDLE_WARM_SCORE	1.0f
#define	IDLE_COLD_SCORE	3000	// TODO: change to waitTime + initTime of solo-run
#define	WRITE_SIZE	46
#define	SET_SIZE	10
#define	MSG_SIZE	512

#define	CACHE_ENTRY_NUM	65536	// Default: 2^16
// #define	CACHE_ENTRY_NUM 4096	// 2^12
// #define	CACHE_ENTRY_NUM 2048	// 2^11
// #define	CACHE_ENTRY_NUM	1024	// 2^10
// #define	CACHE_ENTRY_NUM	512	// 2^9
// #define	CACHE_ENTRY_NUM	256	// 2^8
// #define	CACHE_ENTRY_NUM	128	// 2^7
// #define	CACHE_ENTRY_NUM	64	// 2^6
#define	LAT_MSG_SIZE	7
#define	FUNC_NAME_SIZE	7	// funcXXX type

#define	DELIM	"@"
#define CONT_DELIM  "#"
#define	SCORE_DELIM	"/"
#define	NAME_DELIM	"$"
#define	LAT_DELIM	"!"
#define	EXEC_CONSIST_DELIM	"^"
#define	OW_MSG_DELIM	"*"
#define	SOCK_MSG_DELIM	"&"
// #define	INIT_MSG	"PICKME"
const char INIT_MSG[10] = "PICKME";
#define	END_MSG	"END"
#define	ACK_MSG	"THISISACKNOWLEDGEMENTMESSAGE"
#define	SOCK_LAST_CHAR	';'

#define	WARM_SCORE_SIZE	6
#define	COLD_SCORE_SIZE	5
const int WARM_INV_LIST_SIZE = get_all_invokers().size();
const int SCORE_ENTRY_SIZE = WARM_SCORE_SIZE + COLD_SCORE_SIZE + 1;
const int CACHE_ENTRY_SIZE = WARM_INV_LIST_SIZE + LAT_MSG_SIZE + FUNC_NAME_SIZE + 1;	// last 1: null
// delim: 1, warm score: 6, delim: 1, cold score: 5, null: 1
// #define SCORE_ENTRY_SIZE 14
const int SCORE_CACHE_SIZE = SCORE_ENTRY_SIZE * WARM_INV_LIST_SIZE;
const int CACHE_SIZE = CACHE_ENTRY_SIZE * CACHE_ENTRY_NUM;
const int TOTAL_SIZE = CACHE_SIZE + SCORE_CACHE_SIZE;
