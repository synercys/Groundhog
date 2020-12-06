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
    ios.showErrorMessages(true);
    std::string ip = getIP();

    int current_node;
    
    for(int i = 0; i < n ; i++) {
        if(ips[i].compare(ip) == 0){
            current_node = i;    
        }
    }
    
    for(int i = 0; i < n ; i++) {
        if(i < current_node) 
        {
            //connect as a client  
            Channel clientChl = Session(ios, ips[i], SessionMode::Client).addChannel();
            clientChl.send(getIP());
            std::string msg;
            clientChl.recv(msg);
            std::cout << msg << std::endl;
        } 
        else if (i == current_node) 
        {
            continue;
        } 
        else 
        {  //connect as a server
            Session perPartySession(ios, ip, SessionMode::Server);
            Channel serverChl = perPartySession.addChannel();
            std::chrono::milliseconds timeout(1000000000000);
            serverChl.waitForConnection();
            serverChl.send(getIP());
            std::string msg;
            serverChl.recv(msg);
            std::cout << msg << std::endl;
        }
        
        
    }
}


int main(int argc, char** argv) {
    std::vector<std::string> ips { "10.0.60.177", "10.0.60.64", "10.0.60.185"};
    u64 n = ips.size();
    connect(ips, n);

}