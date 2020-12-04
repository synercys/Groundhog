#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cryptoTools/Common/config.h>

#include <cryptoTools/Common/Defines.h>
#include <cryptoTools/Network/Channel.h>
#include <cryptoTools/Network/Session.h>
#include <cryptoTools/Network/IOService.h>


using namespace osuCrypto;

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

std::string getIP() {
    std::string result = exec("ifconfig");
    std::istringstream iss(result);
    std::vector<std::string> tokens{std::istream_iterator<std::string>{iss},
                                    std::istream_iterator<std::string>{}};

    std::string ip;
    for(int i=0; i< tokens.size(); i++){
        if(tokens[i].compare("inet") == 0)
        {
            ip = tokens[i+1];
            break;
        } 
    }

    return ip;
}

void connect(std::vector<std::string> ips, u64 n) {
    IOService ios(n);
    auto port = 1212;

    std::string ip = getIP();
    std::cout << "IP = " << ip << std::endl;

    int current_node;
    
    for(int i = 0; i < n ; i++) {
        if(ips[i].compare(ip) == 0){
            current_node = i;    
        }
    }
    
    for(int i = 0; i < n ; i++) {
        if(i < current_node) {
            //connect as a client
        } else if (i == current_node) {
            continue;
        } else {
           //connect as a server 
           std::string serversIpAddress = ips[i] + ':' + std::to_string(port + current_node); 
           Session perPartySession(ios, serversIpAddress, SessionMode::Server);
            ++port; 
            Channel serverChl = perPartySession.addChannel();
        }
        
        
    }
}


int main(int argc, char** argv) {
    IOService ios(2);
    ios.showErrorMessages(true);
    
    auto ip1 = std::string("10.0.60.177");
    auto ip2 = std::string("10.0.60.64");
    //auto port = 1212;

    std::string serversIpAddress = ip1;
    //+ ':' + std::to_string(port);
    std::string sessionHint = "party0_party1";

    // Create a pair of sessions that connect to eachother. Note that
    // the sessionHint parameter is options.
    Session client(ios, serversIpAddress, SessionMode::Client, sessionHint);
    
    std::string channelName = "channelName";
    Channel namedChl1 = client.addChannel(channelName);
    
    std::chrono::milliseconds timeout(100);
    bool open = namedChl1.waitForConnection(timeout);

    std::vector<int> dest;
    namedChl1.recv(dest);
    
    std::vector<int> data{ 0,1,2,3,4,79896 };
    namedChl1.send(data);

    for(int i=0; i < dest.size(); i++)
        std::cout << dest.at(i) << ' ';

    std::cout << "Channel connected = " << namedChl1.isConnected() << std::endl;

}