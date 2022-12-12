#include "invoker.hpp"
#include "sock.hpp"
#include "ib.hpp"
#include "debug.hpp"
#include "utils.hpp"
#include <unistd.h>
#include <thread>
#include <future>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <random>

Invoker::Invoker() {

}

Invoker::Invoker(int id, std::map<std::string, std::vector<std::string>> ip, int port, struct ib_res r, int num) {
	this->id = id;
	this->peer_num = num;
	this->total_invoker_num = get_all_invokers().size();
	this->peer_ips = ip;
	this->port = port;
	this->res = r;

	printf("peer num: %d, id: %d\n", num, id);
}

int Invoker::run() {
	strcpy(this->res.cache_buf, "Mohammad Alian");

	while(1) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	this->clean_invoker();

	return 0;
}

int Invoker::setup_invoker() {
	int qp_num = 0;
	auto peer_inv_ips = this->peer_ips["invoker"];
	auto cont_ips = this->peer_ips["controller"];
	int cont_serv_sock = sock_bind_and_accept(this->port);

	// exchange QP info with controllers. QP[0] is for controller and sched_mr
	// TODO: 각 qp가 어떤 머신과 연결된 것인지 정보 저장이 필요한가?
	for (auto ip : cont_ips) {
		struct qp_info remote;
		sock_read_qp_info(cont_serv_sock, &remote);
		res.cache_rkey[qp_num] = remote.cache_rkey;
		res.cache_raddr[qp_num] = remote.cache_raddr;
		res.sched_rkey[qp_num] = remote.sched_rkey;
		res.sched_raddr[qp_num] = remote.sched_raddr;
		// uint32_t remote_qp_num = remote.qp_num;
		// uint16_t remote_lid = remote.lid;
		// uint64_t remote_inter_id = remote.interface_id;
		// uint64_t remote_subnet = remote.subnet_prefix;

		struct qp_info local;
		local.lid = res.port_attr.lid;
		local.qp_num = res.qp[qp_num]->qp_num;
		local.cache_rkey = res.cache_mr->rkey;
		local.cache_raddr = (uintptr_t)res.cache_mr->addr;
		local.sched_rkey = res.sched_mr->rkey;
		local.sched_raddr = (uintptr_t)res.sched_mr->addr;

		/* SR-IOV */
		local.interface_id = res.gid.global.interface_id;
		local.subnet_prefix = res.gid.global.subnet_prefix;
		sock_write_qp_info(cont_serv_sock, &local);

		printf("[%s]\n", ip.c_str());
		printf("[local] qp cache raddr: %p\n", local.cache_raddr);
		printf("[local] qp cache rkey: %u\n", local.cache_rkey);
		printf("[local] qp sched raddr: %p\n", local.sched_raddr);
		printf("[local] qp sched rkey: %u\n", local.sched_rkey);
		printf("[local] qp num: %u\n", local.qp_num);
		printf("[local] lid: %u\n", local.lid);
		printf("[local] interface id: %u\n", local.interface_id);
		printf("[local] subnet prefix: %u\n", local.subnet_prefix);

		printf("[remote] qp cache raddr: %p\n", remote.cache_raddr);
		printf("[remote] qp cache rkey: %u\n", remote.cache_rkey);
		printf("[remote] qp sched raddr: %p\n", remote.sched_raddr);
		printf("[remote] qp sched rkey: %u\n", remote.sched_rkey);
		printf("[remote] qp num: %u\n", remote.qp_num);
		printf("[remote] lid: %u\n", remote.lid);
		printf("[remote] interface id: %u\n", remote.interface_id);
		printf("[remote] subnet prefix: %u\n", remote.subnet_prefix);

		modify_qp_to_rts(res.qp[qp_num], remote.qp_num, remote.lid, remote.interface_id, remote.subnet_prefix);

		qp_num++;
	}

	printf("QP info exchange with controller end!\n");

	// exchange QP info with invokers
	// int cur_id = 0;
	int cur_sock = 0;
	this->inv_sockfd.resize(this->total_invoker_num, -1);
	int port_after_cont = this->port + 1;
	int per_inv_sleep_time = 300;
	int sleep_time = (id + 1) * per_inv_sleep_time;
	int tot_sleep_time = (this->total_invoker_num + 1) * per_inv_sleep_time;

	for (int i = 0; i < this->total_invoker_num; i++) {
		if (this->id != i) {
			std::string ip = peer_inv_ips[cur_sock];
			std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));	// need to connect socket safely
			printf("inv_sockfd connect try. IP: %s, port: %d, sock index: %d\n", ip.c_str(), port_after_cont, i);
			this->inv_sockfd[i] = sock_create_connect(ip.c_str(), port_after_cont);
			cur_sock++;

			if (this->id == i + 1) {
				continue;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(tot_sleep_time - sleep_time));	// need to connect socket safely
			printf("connected\n");
		}
		else {
			printf("accept for port: %d...\n", port_after_cont);
			this->server_socks = sock_bind_and_accept_multiple(port_after_cont, this->total_invoker_num, id);
			printf("accept completed\n");
		}
	}

	/*
	while(cur_id != total_invs) {
		if (cur_id == this->id) {
			this->other_inv_sock = sock_create_bind(inv_serv_port);
		}
		else {
			std::string ip = inv_ips[cur_sock];
			this->inv_sockfd[cur_sock] = sock_create_connect(ip.c_str(), inv_serv_port);
			cur_sock++;
		}
		cur_id++;
	}
	*/

	int send_qp_num = qp_num;
	for (int i = 0; i < this->total_invoker_num; i++) {
		if (i == this->id) {
			for (int j = 0; j < this->total_invoker_num; j++) {
				if (j == this->id) {
					continue;
				}
				struct qp_info local;
				local.lid = res.port_attr.lid;
				local.qp_num = res.qp[send_qp_num]->qp_num;
				local.cache_rkey = res.cache_mr->rkey;
				local.cache_raddr = (uintptr_t)res.cache_mr->addr;
				local.sched_rkey = res.sched_mr->rkey;
				local.sched_raddr = (uintptr_t)res.sched_mr->addr;
				std::this_thread::sleep_for(std::chrono::milliseconds(10));	// need to write socket safely
				printf("write to invoker%d\n", j);
				sock_write_qp_info(this->inv_sockfd[j], &local);

				send_qp_num++;
				printf("[local] qp num: %d\n", local.qp_num);
			}

			printf("[local] qp cache raddr: %p\n", res.cache_mr->addr);
			printf("[local] qp cache rkey: %d\n", res.cache_mr->rkey);
			printf("[local] qp sched raddr: %p\n", res.sched_mr->addr);
			printf("[local] qp sched rkey: %d\n", res.sched_mr->rkey);
			printf("[local] lid: %d\n", res.port_attr.lid);
		}
		else {
			// read from socket
			struct qp_info remote;
			printf("waiting for invoker%d\n", i);
			sock_read_qp_info(this->server_socks[i], &remote);
			res.cache_rkey[qp_num] = remote.cache_rkey;
			res.cache_raddr[qp_num] = remote.cache_raddr;
			res.sched_rkey[qp_num] = remote.sched_rkey;
			res.sched_raddr[qp_num] = remote.sched_raddr;
			uint32_t remote_qp_num = remote.qp_num;
			uint16_t remote_lid = remote.lid;

			printf("[remote] index of qp: %d\n", qp_num);
			// modify_qp_to_rts(res.qp[qp_num], remote_qp_num, remote_lid);
			qp_num++;

			printf("[remote] qp cache raddr: %p\n", remote.cache_raddr);
			printf("[remote] qp cache rkey: %d\n", remote.cache_rkey);
			printf("[remote] qp sched raddr: %p\n", remote.sched_raddr);
			printf("[remote] qp sched rkey: %d\n", remote.sched_rkey);
			printf("[remote] qp num: %d\n", remote_qp_num);
			printf("[remote] lid: %d\n", remote_lid);
		}
	}
	printf("\n===== setup finished =====\n\n");

	/*
	cur_id = 0;
	int send_qp_num = qp_num;
	while(cur_id != total_invs) {
		// broadcast to peers
		if (cur_id == this->id) {
			for (int i = 0; i < peer_inv_ips.size(); i++) {
				struct qp_info local;
				local.lid = res.port_attr.lid;
				local.qp_num = res.qp[send_qp_num]->qp_num;
				local.cache_rkey = res.cache_mr->rkey;
				local.cache_raddr = (uintptr_t)res.cache_mr->addr;
				local.sched_rkey = res.sched_mr->rkey;
				local.sched_raddr = (uintptr_t)res.sched_mr->addr;
				sock_write_qp_info(this->inv_sockfd[i], &local);

				send_qp_num++;
				printf("[local] qp num: %d\n", local.qp_num);
			}

			printf("[local] qp cache raddr: %p\n", res.cache_mr->addr);
			printf("[local] qp cache rkey: %d\n", res.cache_mr->rkey);
			printf("[local] qp sched raddr: %p\n", res.sched_mr->addr);
			printf("[local] qp sched rkey: %d\n", res.sched_mr->rkey);
			printf("[local] lid: %d\n", res.port_attr.lid);
		}
		else {
			// read from socket
			struct qp_info remote;
			sock_read_qp_info(this->other_inv_sock, &remote);
			res.cache_rkey[qp_num] = remote.cache_rkey;
			res.cache_raddr[qp_num] = remote.cache_raddr;
			res.sched_rkey[qp_num] = remote.sched_rkey;
			res.sched_raddr[qp_num] = remote.sched_raddr;
			uint32_t remote_qp_num = remote.qp_num;
			uint16_t remote_lid = remote.lid;

			printf("[remote] index of qp: %d\n", qp_num);
			modify_qp_to_rts(res.qp[qp_num], remote_qp_num, remote_lid);
			qp_num++;

			printf("[remote] qp cache raddr: %p\n", remote.cache_raddr);
			printf("[remote] qp cache rkey: %d\n", remote.cache_rkey);
			printf("[remote] qp sched raddr: %p\n", remote.sched_raddr);
			printf("[remote] qp sched rkey: %d\n", remote.sched_rkey);
			printf("[remote] qp num: %d\n", remote_qp_num);
			printf("[remote] lid: %d\n", remote_lid);
		}
		cur_id++;
	}
	*/
	close(cont_serv_sock);

	return 1;
}

void Invoker::clean_invoker() {
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