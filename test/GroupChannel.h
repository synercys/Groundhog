#pragma once

#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cryptoTools/Common/config.h>
#include <cryptoTools/Common/CLP.h>
#include <cryptoTools/Common/Defines.h>
#include <cryptoTools/Network/Channel.h>
#include <cryptoTools/Network/Session.h>
#include <cryptoTools/Network/IOService.h>
#include <dEnc/Defines.h>
#include <vector>
#include <test/util.h>

std::string exec(const char* cmd) 
{
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

std::string getIP()
{
    std::string result = exec("ifconfig");
    std::istringstream iss(result);
    std::vector<std::string> tokens{std::istream_iterator<std::string>{iss},
                                    std::istream_iterator<std::string>{}};

    std::string ip;
    for(int i=0; i< tokens.size(); i++)
    {
        if(tokens[i].compare("inet") == 0)
        {
            ip = tokens[i+1];
            break;
        } 
    }

    return ip;
}

namespace osuCrypto
{
    struct GroupChannel
    {
        std::vector<Channel> nChls; // to send and receive msgs
        u64 current_node; // index of current node's ip in vector of ips


        

        void connect(std::vector<std::string> ips, u64 n) {
            nChls.resize(n);

            IOService ios(n);
            ios.showErrorMessages(true);
            
            std::string ip = getIP();
            for(int i = 0; i < n ; i++) 
            {
                if(ips[i].compare(ip) == 0)
                {
                    current_node = i;    
                }
            }
            
            for(int i = 0; i < n ; i++) {
                if(i < current_node) 
                {
                    //connect as a client  
                    Channel clientChl = Session(ios, ips[i], SessionMode::Client).addChannel();
                    nChls[i] = clientChl;

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
                    nChls[i] = serverChl;
                }
                
            }
        }

    };

}