#pragma once
#include "setup.hpp"
#include <vector>
#include <map>
#include <string>
#include <boost/iostreams/device/mapped_file.hpp>

class Controller {
	private:
		// key: "controller"/"invoker", value: vector of IPs
		std::map<std::string, std::vector<std::string>> ips;
		// key: invoker IP, value: index of mapped QP
		std::map<std::string, int> qp_map;
		struct ib_res res;
		int port;
		int peer_num;

		int write_ptr;
		boost::iostreams::mapped_file mf;
		char *bytes;

	public:
		Controller();
		Controller(std::map<std::string, std::vector<std::string>> ip, int port, struct ib_res r, int num);
		int run();
		int setup_controller();
		int rdma_read_while(int thread_id);
		int rdma_write_while(int thread_id);
		int ev_rdma_read_while(int thread_id);
		int rdma_read_once(std::string msg, int invoker_id);
		void clean_controller();
};