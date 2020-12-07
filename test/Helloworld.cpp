#include <cryptoTools/Common/CLP.h>
#include "util.h"


using namespace osuCrypto;


int main(int argc, char** argv) {
    oc::CLP cmd;
    cmd.parse(argc, argv);

    std::vector<std::string> ips {"10.0.60.64", "10.0.60.177", "10.0.60.185"};
    u64 n = ips.size();
    getLatency(ips, n);

}