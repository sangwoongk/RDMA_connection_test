#include <iostream>
#include <boost/property_tree/json_parser.hpp>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include "config.hpp"
#include "debug.hpp"
using boost::property_tree::ptree;

int get_invoker_id() {
	std::string ip = get_self_ip();
	auto it = read_config_json("invoker");
	int i = 0;
	int ret = -1;

	for (auto &kv : it) {
		std::string tmp = kv.second.get<std::string>("");
		if (tmp == ip) {
			ret = i;
		}
		i++;
	}

	if (ret == -1) {
		error("There is no IP of this invoker");
	}

	return ret;
}

ptree read_config_json(std::string child) {
	ptree props;
	boost::property_tree::read_json("ip_config.json", props);

	ptree it = props.get_child(child);
	return it;
}

bool is_controller() {
	std::string self_ip = get_self_ip();
	auto it = read_config_json("controller");

	for (auto &kv : it) {
		std::string tmp = kv.second.get<std::string>("");
		if (tmp == self_ip) {
			return true;
		}
	}

	return false;
}

std::string get_self_ip() {
	int sockfd;
	unsigned char addr[4] = {0, };
	struct ifreq ifrq;
	struct sockaddr_in* sin;
	std::string interfaces[] = {"eno1", "eno2", "eno3", "ens1", "ens2", "ens3"};
	int fail_cnt = 0;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	for (auto it : interfaces) {
		strcpy(ifrq.ifr_name, it.c_str());
		if (ioctl(sockfd, SIOCGIFADDR, &ifrq) < 0) {
			fail_cnt++;
		}
		else {
			break;
		}
	}

	if (fail_cnt == 3) {
		error("ioctl() SIOCGIFADDR error.");
	}

	sin = (struct sockaddr_in *)&ifrq.ifr_addr;
	memcpy(addr, (void*)&sin->sin_addr, sizeof(sin->sin_addr));
	close(sockfd);

	std::string self_ip = std::to_string((int)addr[0]) + "." + std::to_string((int)addr[1])
			+ "." + std::to_string((int)addr[2]) + "." + std::to_string((int)addr[3]);

	return self_ip;
}

std::vector<std::string> get_controller_ip(std::string ip) {
	std::vector<std::string> ret;
	auto it = read_config_json("controller");

	for (auto &kv : it) {
		std::string tmp = kv.second.get<std::string>("");
		if (tmp == ip) {
			continue;
		}
		ret.push_back(tmp);
	}

	return ret;
}

std::vector<std::string> get_invoker_ip(std::string ip) {
	std::vector<std::string> ret;
	auto it = read_config_json("invoker");

	for (auto &kv : it) {
		std::string tmp = kv.second.get<std::string>("");
		if (tmp == ip) {
			continue;
		}
		ret.push_back(tmp);
	}

	return ret;
}

std::vector<std::string> get_all_invokers() {
	std::vector<std::string> ret;
	auto it = read_config_json("invoker");

	for (auto &kv : it) {
		std::string tmp = kv.second.get<std::string>("");
		ret.push_back(tmp);
	}

	return ret;
}

std::map<std::string, std::vector<std::string>> get_peer_ips() {
	std::string self_ip = get_self_ip();
	std::vector<std::string> cont_ip = get_controller_ip(self_ip);
	std::vector<std::string> inv_ip = get_invoker_ip(self_ip);

	std::map<std::string, std::vector<std::string>> ret;
	ret.insert({"controller", cont_ip});
	ret.insert({"invoker", inv_ip});

	return ret;
}