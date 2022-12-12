#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "debug.hpp"
#include "sock.hpp"
#include "setup.hpp"

int sock_bind(int port) {
	int con_sock = -1;
	struct sockaddr_in serv_addr;

	con_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (con_sock == -1) {
		error("socket creation error");
	}

	memset(&serv_addr, 0, sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);

	if (bind(con_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1) {
		error("bind error");
	}

	return con_sock;
}

int sock_bind_and_accept(int port) {
	int con_sock = sock_bind(port);
	int acc_sock = -1;
	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(struct sockaddr_in);
	int flag = 1;

	if (listen(con_sock, 10) == -1) {
		error("listen error");
	}

	if ((acc_sock = accept(con_sock, (struct sockaddr*) &client_addr, &client_addr_len)) == -1) {
		error("accept error");
	}

	int result = setsockopt(acc_sock, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(int));
	if (result < 0) {
		error("setsockopt() error");
	}
	

	return acc_sock;
}

std::vector<int> sock_bind_and_accept_multiple(int port, int inv_num, int id) {
	int con_sock = sock_bind(port);
	int acc_sock = -1;

	if (listen(con_sock, 10) == -1) {
		error("listen error");
	}

	std::vector<int> ret(inv_num, -1);
	int flag = 1;
	for(int i = 0; i < inv_num; i++) {
		struct sockaddr_in client_addr;
		socklen_t client_addr_len = sizeof(struct sockaddr_in);

		if (id == i) {
			continue;
		}
		printf("waiting for accept. id: %d, i: %d\n", id, i);
		if ((acc_sock = accept(con_sock, (struct sockaddr*) &client_addr, &client_addr_len)) == -1) {
			error("accept error");
		}

		int result = setsockopt(acc_sock, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(int));
		if (result < 0) {
			error("setsockopt() error");
		}
		ret[i] = acc_sock;
		printf("accepted. id: %d, i: %d\n", id, i);
	}

	return ret;
}

int sock_create_connect(const char *ip, int port) {
	struct sockaddr_in serv_addr;
	int sock = socket(PF_INET, SOCK_STREAM, 0);

	if (sock == -1) {
		error("socket creation error");
	}

	memset(&serv_addr, 0, sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(ip);
	serv_addr.sin_port = htons(port);

	if (connect(sock, (struct sockaddr*) &serv_addr, sizeof(struct sockaddr_in)) == -1) {
		error("connect error");
	}

	int flag = 1;
	int result = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(int));
	if (result < 0) {
		error("setsockopt() error");
	}

	return sock;
}

ssize_t sock_write(int sock, void *buf, size_t len) {
	ssize_t nw;
	size_t tot_written;
	const char *buffer = (char *)buf; // avoid pointer arithmetic on void pointer

	for (tot_written = 0; tot_written < len;) {
		nw = write(sock, buf, len - tot_written);

		if (nw <= 0) {
			if (nw == -1 && errno == EINTR) {
				continue;
			}
			else {
				return -1;
			}
		}

		tot_written += nw;
		buffer += nw;
	}
	return tot_written;	
}

ssize_t sock_read(int sock, void *buf, size_t len) {
	size_t nr, tot_read;
	char *buffer = (char *)buf; // avoid pointer arithmetic on void pointer
	tot_read = 0;

	while (len != 0 && (nr = read(sock, buf, len)) != 0) {
		if (nr < 0) {
			if (errno == EINTR) {
				continue;
			}
			else {
				return -1;
			}
		}
		len -= nr;
		buffer += nr;
		tot_read += nr;
	}

	return tot_read;
}

int sock_write_qp_info(int sock, struct qp_info *info) {
	struct qp_info tmp;
	tmp.lid = htons(info->lid);
	tmp.qp_num = htonl(info->qp_num);
	tmp.cache_raddr = htonll(info->cache_raddr);
	tmp.cache_rkey = htonl(info->cache_rkey);
	tmp.sched_raddr = htonll(info->sched_raddr);
	tmp.sched_rkey = htonl(info->sched_rkey);

	/* SR-IOV */
	tmp.interface_id = htonll(info->interface_id);
	tmp.subnet_prefix = htonll(info->subnet_prefix);

	int n = sock_write(sock, (char*)&tmp, sizeof(struct qp_info));

	return n;
}

int sock_read_qp_info(int sock, struct qp_info *info) {
	struct qp_info tmp;

	int n = sock_read(sock, (char*)&tmp, sizeof(struct qp_info));

	info->lid = ntohs(tmp.lid);
	info->qp_num = ntohl(tmp.qp_num);
	info->cache_raddr = ntohll(tmp.cache_raddr);
	info->cache_rkey = ntohl(tmp.cache_rkey);
	info->sched_raddr = ntohll(tmp.sched_raddr);
	info->sched_rkey = ntohl(tmp.sched_rkey);

	/* SR-IOV */
	info->interface_id = ntohll(tmp.interface_id);
	info->subnet_prefix = ntohll(tmp.subnet_prefix);

	return n;
}