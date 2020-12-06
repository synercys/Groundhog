#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cryptoTools/Common/config.h>
#include <cryptoTools/Common/CLP.h>
#include <cryptoTools/Common/Defines.h>
#include <cryptoTools/Network/Channel.h>
#include <cryptoTools/Network/Session.h>
#include <cryptoTools/Network/IOService.h>
#include "util.h"


using namespace osuCrypto;


int main(int argc, char** argv) {
    oc::CLP cmd;
    cmd.parse(argc, argv);

    std::vector<std::string> ips { "10.0.60.177", "10.0.60.64", "10.0.60.185"};
    u64 n = ips.size();
    // connect(ips, n);
    getLatency(ips, n);

}