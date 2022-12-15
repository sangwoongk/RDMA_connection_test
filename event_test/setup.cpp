#include <infiniband/verbs.h>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "setup.hpp"
#include "debug.hpp"
#include "config.hpp"

struct ib_res setup_ib(int peer_num) {
	struct ib_res res;
	memset(&res, 0, sizeof(struct ib_res));

	struct ibv_device **dev_list = NULL;
	dev_list = ibv_get_device_list(NULL);

	if (dev_list == NULL) {
		error("Failed to get ib device list");
	}

	res.ctx = ibv_open_device(*dev_list);
	if (res.ctx == NULL) {
		error("Failed to open ib device");
	}

	int ret = ibv_query_port(res.ctx, IB_PORT, &res.port_attr);
	if (ret != 0) {
		error("Failed to query IB port information");
	}

	ret = ibv_query_gid(res.ctx, IB_PORT, 0, &res.gid);
	if (ret != 0) {
		error("Failed to query GID information");
	}

	res.pd = ibv_alloc_pd(res.ctx);
	if (res.pd == NULL) {
		error("Failed to allocate protected domain");
	}


	// peer_num - number of controllers (1) + invoker itself (1) = peer_num;
	// int cache_size = peer_num * PER_INV_IB_BUF_SIZE;
	// int cache_size = CACHE_ENTRY_SIZE * CACHE_ENTRY_NUM + SCORE_MSG_SIZE * 2;
	// int cache_size = CACHE_ENTRY_SIZE * CACHE_ENTRY_NUM + SCORE_CACHE_SIZE;
	res.cache_buf = new char[TOTAL_SIZE];
	res.sched_buf = new char[PER_INV_IB_BUF_SIZE];

	strcpy(res.cache_buf, INIT_MSG);
	strcpy(res.sched_buf, INIT_MSG);
	// memset(res.cache_buf, NULL, TOTAL_SIZE);
	// memset(res.sched_buf, NULL, PER_INV_IB_BUF_SIZE);

	res.cache_rkey = new uint32_t[peer_num];
	res.cache_raddr = new uint64_t[peer_num];
	res.sched_rkey = new uint32_t[peer_num];
	res.sched_raddr = new uint64_t[peer_num];

	if (res.cache_buf == NULL) {
		error("Failed to allocate read_buf");
	}
	if (res.sched_buf == NULL) {
		error("Failed to allocate write_buf");
	}

	res.cache_mr = ibv_reg_mr(res.pd, (void*)res.cache_buf, TOTAL_SIZE,
		IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_WRITE);

	if (res.cache_mr == NULL) {
		error("Failed to register read_mr");
	}

	res.sched_mr = ibv_reg_mr(res.pd, (void*)res.sched_buf, PER_INV_IB_BUF_SIZE,
		IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_WRITE);

	if (res.sched_mr == NULL) {
		error("Failed to register write_mr");
	}

	ret = ibv_query_device(res.ctx, &res.dev_attr);
	if (ret != 0) {
		error("Failed to query device");
	}

	res.cq = new ibv_cq*[peer_num];
	res.channel = new ibv_comp_channel*[peer_num];

	for (int i = 0; i < peer_num; i++) {
		res.cq[i] = ibv_create_cq(res.ctx, res.dev_attr.max_cqe, NULL, NULL, 0);
		/*
		res.channel = ibv_create_comp_channel(res.ctx);
		if (res.channel == NULL) {
			error("Failed to allocate comp channel");
		}
		res.cq[i] = ibv_create_cq(res.ctx, res.dev_attr.max_cqe, NULL, res.channel, 0);
		*/

		if (res.cq[i] == NULL) {
			error("Failed to create cq");
		}

		/*
		ret = ibv_req_notify_cq(res.cq[i], 0);
		if (ret != 0) {
			error("Failed to request CQ notification");
		}
		*/
	}

	std::vector<struct ibv_qp_init_attr> qp_init_attr;
	qp_init_attr.reserve(peer_num);

	for (int i = 0; i < peer_num; i++) {
		memset(&qp_init_attr[i], 0, sizeof(qp_init_attr[i]));
		qp_init_attr[i].send_cq = res.cq[i];
		qp_init_attr[i].recv_cq = res.cq[i];
		qp_init_attr[i].cap.max_send_wr = 2048;
		qp_init_attr[i].cap.max_recv_wr = 2048;
		qp_init_attr[i].cap.max_send_sge = 1;
		qp_init_attr[i].cap.max_recv_sge = 1;
		qp_init_attr[i].qp_type = IBV_QPT_RC;
	}

	res.qp = new ibv_qp*[peer_num];
	if (res.qp == NULL) {
		error("Failed to allocate qp");
	}

	for (int i = 0; i < peer_num; i++) {
		res.qp[i] = ibv_create_qp(res.pd, &qp_init_attr[i]);

		if (res.qp[i] == NULL) {
			error("Failed to create qp");
		}
	}

	ibv_free_device_list(dev_list);

	return res;
}