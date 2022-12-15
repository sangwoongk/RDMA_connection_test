#pragma once
#include <string>
#include <vector>
#include <infiniband/verbs.h>
#include <endian.h>
#include <byteswap.h>

#if __BYTE_ORDER == __LITTLE_ENDIAN
static inline uint64_t htonll (uint64_t x) {return bswap_64(x); }
static inline uint64_t ntohll (uint64_t x) {return bswap_64(x); }
#elif __BYTE_ORDER == __BIG_ENDIAN
static inline uint64_t htonll (uint64_t x) {return x; }
static inline uint64_t ntohll (uint64_t x) {return x; }
#else
#error __BYTE_ORDER is neither __LITTLE_ENDIAN nor __BIG_ENDIAN
#endif

struct qp_info {
	uint16_t lid;
	uint32_t qp_num;
	uint32_t cache_rkey;
	uint64_t cache_raddr;
	uint32_t sched_rkey;
	uint64_t sched_raddr;

	/* SR-IOV */
	uint64_t interface_id;
	uint64_t subnet_prefix;
} __attribute__((packed));

struct ib_res {
	struct ibv_context *ctx;
	struct ibv_pd *pd;
	struct ibv_mr *cache_mr;
	struct ibv_mr *sched_mr;
	struct ibv_cq **cq;
	struct ibv_qp **qp;
	struct ibv_port_attr port_attr;
	struct ibv_device_attr dev_attr;
	struct ibv_comp_channel **channel;
	union ibv_gid gid;

	char *cache_buf;
	char *sched_buf;

	uint32_t *cache_rkey;
	uint64_t *cache_raddr;
	uint32_t *sched_rkey;
	uint64_t *sched_raddr;
};

struct ib_res setup_ib(int peer_num);