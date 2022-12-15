#include <iostream>
#include <cstdlib>
#include "config.hpp"
#include "setup.hpp"
#include "controller.hpp"
#include "invoker.hpp"
#include "setup.hpp"
// #include "log.h"
using namespace boost;
using namespace std;

int main(int argc, char* argv[]) {
	bool is_cont = is_controller();
	auto peer_ip = get_peer_ips();
	int peer_num = peer_ip["controller"].size() + peer_ip["invoker"].size();
	struct ib_res tmp = setup_ib(peer_num);
	/*
	FILE *fp = fopen("log.log", "w");
	log_add_fp(fp, LOG_TRACE);
	log_set_quiet(true);
	*/

	if (is_cont) {
		Controller controller(peer_ip, SOCK_PORT_START, tmp, peer_num);
		controller.setup_controller();
		controller.run();
	}
	else {
		int id = get_invoker_id();
		Invoker invoker(id, peer_ip, SOCK_PORT_START, tmp, peer_num);
		invoker.setup_invoker();
		invoker.run();
	}

	return 0;
}