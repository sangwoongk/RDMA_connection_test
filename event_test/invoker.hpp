#pragma once
#include "setup.hpp"
// #include "monitor.hpp"
#include "config.hpp"
#include <vector>
#include <string>
#include <map>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <set>

class Invoker {
  private:
    int id;
    int port;
    int ow_sock;
    int sched_inv_id;
    bool poll_inv;
    std::map<std::string, std::vector<std::string>> peer_ips;
    // int server_sock;
    std::vector<int> server_socks;
    std::vector<int> inv_sockfd;
    std::vector<int> ow_sockfd;
    struct ib_res res;
    char prev_cont_msg[PER_INV_IB_BUF_SIZE];
    int peer_num;
    int total_invoker_num;

    // Monitor monitor;

    std::mutex mut;
    std::condition_variable cv;
    bool req_in_flight;
    bool first_exec_after_poll;
    std::mutex req_mut;
    std::mutex sched_mut;
    std::unique_lock<std::mutex> lock;

    std::set<std::string> prev_activ_set;
    std::queue<std::string> prev_activ_queue;
    std::mutex prev_activ_lock;

    std::queue<std::string> req_queue;
    std::mutex req_queue_lock;

  public:
    Invoker();
    Invoker(int id, std::map<std::string, std::vector<std::string>> ip, int port, struct ib_res res, int num);
    int setup_invoker();
    void export_mem();
    void test_skeleton();
    void check_write();
    int run();

    void clean_invoker();
};