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
		int setup_controller();
		int run();
		void check_ack();
		int rdma_read_while(int thread_id);
		std::string rdma_read_cache(std::string msg, int invoker_id);
		int write_sched_buf(std::string msg);
		void clean_controller();
		void mmap_connect();
		void mmap_read_thread();
		void mmap_write(std::string msg);
		char* map_file(const char* fname, size_t &len);
};