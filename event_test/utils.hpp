#include <vector>
#include <string>

std::vector<std::string> split_str(std::string msg, std::string delim);
uint16_t djb2_hash(std::string in);
std::vector<std::pair<float, int>> decode_score_from_cache(char *ptr);
std::pair<float, int> decode_score_from_entry(std::string msg);
std::string encode_warm_score(float score);
float decode_warm_score(std::string msg);
int get_random_int(int min, int max);