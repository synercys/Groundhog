#pragma once
// This file and the associated implementation has been placed in the public domain, waiving all copyright. No restrictions are placed on its use.

#include <cryptoTools/Network/Channel.h>
using namespace osuCrypto;


void senderGetLatency(Channel& chl);

void recverGetLatency(Channel& chl);

void getLatency(std::vector<std::string> ips, u64 n);

std::string exec(const char* cmd);

std::string getIP();

u64 getCurrNodeIdx(std::vector<std::string> ips, std::string ip);


u64 rangeRand(u64 range_from, u64 range_to);