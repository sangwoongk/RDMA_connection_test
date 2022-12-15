#include <infiniband/verbs.h>

int modify_qp_to_rts(struct ibv_qp *qp, uint32_t target_qp_num, uint16_t target_lid, uint64_t interface_id, uint64_t subnet);
int post_write(uint32_t req_size, uint32_t lkey, uint32_t rkey, uint64_t raddr, struct ibv_qp* qp, char *buf);
int post_write_imm(uint32_t req_size, uint32_t lkey, uint32_t rkey, uint64_t raddr, struct ibv_qp* qp, char *buf, uint32_t imm);
int post_read(uint32_t req_size, uint32_t lkey, uint32_t rkey, uint64_t raddr, struct ibv_qp* qp, char *buf);
int post_recv(uint32_t req_size, uint32_t lkey, uint64_t wr_id, struct ibv_qp *qp, char *buf);
int write_cache();