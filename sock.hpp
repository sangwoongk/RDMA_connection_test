#pragma once
#include <vector>
#include <unistd.h>

int sock_bind(int port);
int sock_bind_and_accept(int port);
std::vector<int> sock_bind_and_accept_multiple(int port, int inv_num, int id);
int sock_create_connect(const char *ip, int port);
ssize_t sock_read(int sock, void *buf, size_t len);
ssize_t sock_write(int sock, void *buf, size_t len);
int sock_write_qp_info(int sock, struct qp_info *info);
int sock_read_qp_info(int sock, struct qp_info *info);