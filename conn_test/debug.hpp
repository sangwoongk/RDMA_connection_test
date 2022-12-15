#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define	clean_errno() (errno == 0 ? "None" : strerror(errno))
#define error(M, ...) { fprintf(stderr, "[ERROR] (%s:%d:%s: errno: %d, %s) " M "\n", __FILE__, __LINE__, __func__, errno, clean_errno(), ##__VA_ARGS__); exit(1); }
#define debug(M, ...) { fprintf(stderr, "[DEBUG] (%s:%d:%s: errno: %s) " M "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__); }

// void handle_error(const char* msg) {
// 	perror(msg);
// 	exit(255);
// }