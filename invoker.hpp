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
    void connect_local_ow();
    void connect_remote_ow();
    void inv_read(int sock_index);
    void poll_sched_buf();
    int schedule_invoker(char *entry);
    void schedule_poll_invoker();
    void maintain_prev_info(std::string msg);
    int get_lowest_cold_invoker(char *ptr);
    int setup_invoker();
    void send_msg_to_target(std::string msg, int inv_id);
    void send_poll_inv(std::string id);
    void send_activation_msg(std::string msg);
    void send_activation_msg_to_target(std::string msg, int inv_id);
    void send_sock_msg_to_target(std::string msg, int inv_id);
    void send_local_ow(std::string msg);
    std::string build_sock_msg(std::string msg);
    std::string build_activation_msg(std::string activation_id, std::string func_name, int exec_time, std::string transid, std::string start_time);
    int run();

    void clean_invoker();
};