#include "utils.hpp"
#include "config.hpp"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <cmath>
#include <boost/format.hpp>
#include <random>
#include <iostream>

std::vector<std::string> split_str(std::string msg, std::string delim) {
	std::vector<std::string> ret;
	boost::split(ret, msg, boost::is_any_of(delim));

	return ret;
}

uint16_t djb2_hash(std::string in) {
	unsigned long hash = 5381;
	int c;
	for (int i = 0; i < in.length(); ++i) {
		c = in[i];
		hash = (((hash << 5) + hash) + c) % CACHE_ENTRY_NUM;
	}

	return hash % CACHE_ENTRY_NUM;
}

std::vector<std::pair<float, int>> decode_score_from_cache(char *ptr) {
	std::vector<std::pair<float, int>> ret;
	for (int i = 0; i < WARM_INV_LIST_SIZE; i++) {
		int offset = SCORE_ENTRY_SIZE * i;
		char *entry = ptr + offset;
		auto tmp = decode_score_from_entry(entry);
		ret.push_back(tmp);
	}

	return ret;
}

std::pair<float, int> decode_score_from_entry(std::string msg) {
	if (msg.length() == 0) {
		return {-1, -1};
	}
	std::string warm = msg.substr(0, WARM_SCORE_SIZE);
	std::string cold = msg.substr(WARM_SCORE_SIZE);

	float warm_sc = decode_warm_score(warm);
	int cold_sc = std::stoi(cold);

	std::pair<float, int> ret = {warm_sc, cold_sc};

	return ret;
}

std::string encode_warm_score(float score) {
	int int_part = score;
	int float_part = (score - int_part) * pow(10, WARM_SCORE_SIZE - 2);
	std::string warm_int = (boost::format("%02d") % int_part).str();
	std::string warm_float = (boost::format("%04d") % float_part).str();

	return warm_int + warm_float;
}

float decode_warm_score(std::string msg) {
	std::string int_part = msg.substr(0, 2);
	std::string float_part = msg.substr(2);
	int ip = stoi(int_part);
	float div = pow(10, 4);
	float fp = stof(float_part) / div;

	return ip + fp;
}

int get_random_int(int min, int max) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dis(min, max);

	return dis(gen);
}

uint32_t encode_imm(int inv_id, int buf_index) {
	uint32_t ret = inv_id * PADDING + buf_index;
	return ret;
}

std::pair<int, int> decode_imm(uint32_t imm) {
	int inv_id = imm / PADDING;
	int buf_index = imm % PADDING;

	return {inv_id, buf_index};
}