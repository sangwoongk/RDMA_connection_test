#include "controller.hpp"
#include "sock.hpp"
#include "ib.hpp"
#include "config.hpp"
#include "debug.hpp"
// #include "log.h"
#include <unistd.h>
#include <thread>
#include <future>
#include <iostream>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>
namespace bio = boost::iostreams;

Controller::Controller() {

}

Controller::Controller(std::map<std::string,std::vector<std::string>> ip, int port, struct ib_res r, int num) {
	this->peer_num = num;
	this->ips = ip;
	this->port = port;
	this->res = r;
	this->write_ptr = 1;
}

std::string Controller::rdma_read_cache(std::string msg, int invoker_id) {
	// TODO: invoker_id will be changed dynamically. Set to 0 temporary.
	struct ibv_qp *qp = res.qp[invoker_id];
	struct ibv_cq *cq = res.cq[invoker_id];
	struct ibv_wc *wc = nullptr;
	uint32_t read_rkey = res.cache_rkey[invoker_id];
	uint64_t read_raddr = res.cache_raddr[invoker_id];
	uint32_t lkey = res.cache_mr->lkey;

	char *buf = res.cache_buf;
	int num_wc = 20;

	int ret = post_read(PER_INV_IB_BUF_SIZE, lkey, read_rkey, read_raddr, qp, buf);
	if (ret != 0) {
		error("read failure");
	}

	printf("[read] %s\n", buf);
	strcpy(buf, "hyosangkim");
	printf("[read changed] %s\n", buf);

	int n = ibv_poll_cq(cq, num_wc, wc);
	if (n < 0) {
		error("Failed to poll cq");
	}

	return std::string(buf);
}

int Controller::rdma_read_while(int thread_id) {
	struct ibv_qp *qp = res.qp[thread_id];
	struct ibv_cq *cq = res.cq[thread_id];
	struct ibv_wc *wc = nullptr;
	uint32_t read_rkey = res.cache_rkey[thread_id];
	uint64_t read_raddr = res.cache_raddr[thread_id];
	uint32_t lkey = res.cache_mr->lkey;

	char *buf = res.cache_buf;
	bool stop = false;
	int num_wc = 20;

	wc = (struct ibv_wc*)calloc(num_wc, sizeof(struct ibv_wc));

	while(!stop) {
		int ret = post_read(PER_INV_IB_BUF_SIZE, lkey, read_rkey, read_raddr, qp, buf);
		if (ret != 0) {
			error("read failure");
		}

		if (strcmp(buf, END_MSG) == 0) {
			stop = true;
		}

		printf("====================\n");
		printf("[read] %s\n", buf);
		strcpy(buf, "hyosangkim");
		printf("[read buf flushed] %s\n", buf);
		printf("====================\n");

		int n = ibv_poll_cq(cq, num_wc, wc);
		if (n < 0) {
			error("Failed to poll cq");
		}

		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	return 0;
}

int Controller::run() {
	std::thread read_thread(&Controller::rdma_read_while, this, 0);
	read_thread.join();

	this->clean_controller();

	return 0;
}

int Controller::setup_controller() {
	int i = 0;
	// if want to connect with another controller, do below loop again
	auto cont_ips = ips["controller"];
	auto inv_ips = ips["invoker"];

	// TODO: Need ip <-> qp_num map
	for (auto ip : inv_ips) {
		int sock = sock_create_connect(ip.c_str(), this->port);

		struct qp_info local;
		local.lid = res.port_attr.lid;
		local.qp_num = res.qp[i]->qp_num;
		local.cache_rkey = res.cache_mr->rkey;
		local.cache_raddr = (uintptr_t)res.cache_mr->addr;
		local.sched_rkey = res.sched_mr->rkey;
		local.sched_raddr = (uintptr_t)res.sched_mr->addr;

		/* SR-IOV */
		local.interface_id = res.gid.global.interface_id;
		local.subnet_prefix = res.gid.global.subnet_prefix;

		sock_write_qp_info(sock, &local);

		struct qp_info remote;
		sock_read_qp_info(sock, &remote);
		res.cache_rkey[i] = remote.cache_rkey;
		res.cache_raddr[i] = remote.cache_raddr;
		res.sched_rkey[i] = remote.sched_rkey;
		res.sched_raddr[i] = remote.sched_raddr;

		printf("[%s]\n", ip.c_str());
		printf("[local] qp cache raddr: %p\n", local.cache_raddr);
		printf("[local] qp cache rkey: %u\n", local.cache_rkey);
		printf("[local] qp sched raddr: %p\n", local.sched_raddr);
		printf("[local] qp sched rkey: %u\n", local.sched_rkey);
		printf("[local] qp num: %u\n", local.qp_num);
		printf("[local] lid: %u\n", local.lid);

		printf("[remote] qp cache raddr: %p\n", remote.cache_raddr);
		printf("[remote] qp cache rkey: %u\n", remote.cache_rkey);
		printf("[remote] qp sched raddr: %p\n", remote.sched_raddr);
		printf("[remote] qp sched rkey: %u\n", remote.sched_rkey);
		printf("[remote] qp num: %u\n", remote.qp_num);
		printf("[remote] lid: %u\n", remote.lid);

		modify_qp_to_rts(res.qp[i], remote.qp_num, remote.lid, remote.interface_id, remote.subnet_prefix);
		this->qp_map.insert({ip, i});
		close(sock);

		i++;
	}

	return 1;
}

void Controller::clean_controller() {
	for (int i = 0; i < this->peer_num; i++) {
		if (res.qp[i] != NULL) {
			ibv_destroy_qp(res.qp[i]);
		}
	}
	delete[] res.qp;

	for (int i = 0; i < this->peer_num; i++) {
		if (res.cq[i] != NULL) {
			ibv_destroy_cq(res.cq[i]);
		}
	}
	delete[] res.cq;

	if (res.cache_mr != NULL) {
		ibv_dereg_mr(res.cache_mr);
	}

	if (res.sched_mr != NULL) {
		ibv_dereg_mr(res.sched_mr);
	}

	if (res.pd != NULL) {
		ibv_dealloc_pd(res.pd);
	}

	if (res.ctx != NULL) {
		ibv_close_device(res.ctx);
	}

	if (res.cache_buf != NULL) {
		delete[] res.cache_buf;
	} 

	if (res.sched_buf != NULL) {
		delete[] res.sched_buf;
	}

	if (res.cache_raddr != NULL) {
		delete[] res.cache_raddr;
	}

	if (res.cache_rkey != NULL) {
		delete[] res.cache_rkey;
	}

	if (res.sched_raddr != NULL) {
		delete[] res.sched_raddr;
	}

	if (res.sched_rkey != NULL) {
		delete[] res.sched_rkey;
	}
}