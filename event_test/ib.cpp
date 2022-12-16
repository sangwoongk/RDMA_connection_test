#include <arpa/inet.h>
#include "ib.hpp"
#include "config.hpp"
#include "debug.hpp"

int modify_qp_to_rts(struct ibv_qp *qp, uint32_t target_qp_num, uint16_t target_lid, uint64_t interface_id, uint64_t subnet) {
	int ret = 0;

	/* change QP state to INIT */
	{
		struct ibv_qp_attr qp_attr;
		memset(&qp_attr, 0, sizeof(qp_attr));

		qp_attr.qp_state = IBV_QPS_INIT;
		qp_attr.qp_access_flags = IBV_ACCESS_LOCAL_WRITE |
															IBV_ACCESS_REMOTE_READ |
															IBV_ACCESS_REMOTE_ATOMIC |
															IBV_ACCESS_REMOTE_WRITE;
		qp_attr.pkey_index = 0;
		qp_attr.port_num = IB_PORT;

		ret = ibv_modify_qp(qp, &qp_attr,
												IBV_QP_STATE | IBV_QP_PKEY_INDEX |
												IBV_QP_PORT | IBV_QP_ACCESS_FLAGS);

		if (ret != 0) {
			error("Failed to modify qp to INIT");
			return -1;
		}
	}

	/* Change QP state to RTR */
	{
		struct ibv_qp_attr qp_attr;
		memset(&qp_attr, 0, sizeof(qp_attr));

		qp_attr.qp_state = IBV_QPS_RTR;
		qp_attr.path_mtu = IB_MTU;
		qp_attr.rq_psn = 0;
		qp_attr.dest_qp_num = target_qp_num;
		qp_attr.ah_attr.dlid = target_lid;
		qp_attr.ah_attr.sl = IB_SL;
		qp_attr.ah_attr.src_path_bits = 0;
		// qp_attr.ah_attr.is_global = 0;
		/* SR-IOV */
		qp_attr.ah_attr.is_global = 1;	// 1 for SR-IOV, default value: 0
		qp_attr.ah_attr.grh.sgid_index = 0;

		union ibv_gid tmp_gid;
		tmp_gid.global.interface_id = interface_id;
		tmp_gid.global.subnet_prefix = subnet;
		qp_attr.ah_attr.grh.dgid = tmp_gid;
		/* SR-IOV */

		qp_attr.ah_attr.port_num = IB_PORT;
		qp_attr.max_dest_rd_atomic = 1;
		qp_attr.min_rnr_timer = 0x12;

		printf("dest_qp_num: %d\n", qp_attr.dest_qp_num);
		printf("dlid: %d\n", qp_attr.ah_attr.dlid);

		ret = ibv_modify_qp(qp, &qp_attr,
												IBV_QP_STATE | IBV_QP_AV |
												IBV_QP_PATH_MTU | IBV_QP_DEST_QPN |
												IBV_QP_RQ_PSN | IBV_QP_MAX_DEST_RD_ATOMIC |
												IBV_QP_MIN_RNR_TIMER);

		if (ret != 0) {
			error("Failed to change qp to RTR");
			return -1;
		}
	}

	/* Change QP state to RTS */
	{
		struct ibv_qp_attr qp_attr;
		memset(&qp_attr, 0, sizeof(qp_attr));

		qp_attr.qp_state = IBV_QPS_RTS;
		qp_attr.sq_psn = 0;
		qp_attr.max_rd_atomic = 1;
		qp_attr.timeout = 14;
		qp_attr.retry_cnt = 7;
		qp_attr.rnr_retry = 7;

		ret = ibv_modify_qp(qp, &qp_attr,
												IBV_QP_STATE | IBV_QP_TIMEOUT |
												IBV_QP_RETRY_CNT | IBV_QP_RNR_RETRY |
												IBV_QP_SQ_PSN | IBV_QP_MAX_QP_RD_ATOMIC);

		if (ret != 0) {
			error("Failed to change qp to RTS");
			return -1;
		}
	}

	return 0;
}

int post_write(uint32_t req_size, uint32_t lkey, uint32_t rkey, uint64_t raddr, struct ibv_qp* qp, char *buf) {
	struct ibv_send_wr *bad;
	struct ibv_send_wr send_wr;
	struct ibv_sge list;

	memset(&list, 0, sizeof(list));
	memset(&send_wr, 0, sizeof(send_wr));

	list.addr = (uintptr_t)buf;
	list.length = req_size;
	list.lkey = lkey;

	send_wr.sg_list = &list;
	send_wr.num_sge = 1;
	send_wr.opcode = IBV_WR_RDMA_WRITE;
	send_wr.send_flags = IBV_SEND_SIGNALED;
	send_wr.wr.rdma.remote_addr = raddr;
	send_wr.wr.rdma.rkey = rkey;

	int ret = ibv_post_send(qp, &send_wr, &bad);

	return ret;
}

int post_write_imm(uint32_t req_size, uint32_t lkey, uint32_t rkey, uint64_t raddr, struct ibv_qp* qp, char *buf, uint32_t imm) {
	struct ibv_send_wr *bad;
	struct ibv_send_wr send_wr;
	struct ibv_sge list;

	memset(&list, 0, sizeof(list));
	memset(&send_wr, 0, sizeof(send_wr));

	list.addr = (uintptr_t)buf;
	list.length = req_size;
	list.lkey = lkey;

	send_wr.sg_list = &list;
	send_wr.num_sge = 1;
	send_wr.opcode = IBV_WR_RDMA_WRITE_WITH_IMM;
	send_wr.imm_data = htonl(imm);
	send_wr.send_flags = IBV_SEND_SIGNALED;
	send_wr.wr.rdma.remote_addr = raddr;
	send_wr.wr.rdma.rkey = rkey;

	int ret = ibv_post_send(qp, &send_wr, &bad);

	return ret;
}

int post_read(uint32_t req_size, uint32_t lkey, uint32_t rkey, uint64_t raddr, struct ibv_qp* qp, char *buf) {
	struct ibv_send_wr *bad;
	struct ibv_send_wr send_wr;
	struct ibv_sge list;

	memset(&list, 0, sizeof(list));
	memset(&send_wr, 0, sizeof(send_wr));

	list.addr = (uintptr_t)buf;
	list.length = req_size;
	list.lkey = lkey;

	send_wr.sg_list = &list;
	send_wr.num_sge = 1;
	send_wr.opcode = IBV_WR_RDMA_READ;
	send_wr.send_flags = IBV_SEND_SIGNALED;
	send_wr.wr.rdma.remote_addr = raddr;
	send_wr.wr.rdma.rkey = rkey;

	int ret = ibv_post_send(qp, &send_wr, &bad);

	return ret;
}

int post_recv(uint32_t req_size, uint32_t lkey, uint64_t wr_id, struct ibv_qp *qp, char *buf) {
	struct ibv_recv_wr *bad;
	struct ibv_sge list;
	struct ibv_recv_wr recv_wr;

	memset(&list, 0, sizeof(list));
	memset(&recv_wr, 0, sizeof(recv_wr));

	list.addr = (uintptr_t)buf;
	list.length = req_size;
	list.lkey = lkey;

	recv_wr.wr_id = wr_id;
	recv_wr.sg_list = &list;
	recv_wr.num_sge = 1;
	recv_wr.next = NULL;

	return ibv_post_recv(qp, &recv_wr, &bad);
}