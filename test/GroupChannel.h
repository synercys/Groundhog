#pragma once

#include <cryptoTools/Common/config.h>
#include <cryptoTools/Common/Defines.h>
#include <cryptoTools/Network/Session.h>
#include <cryptoTools/Network/IOService.h>
#include <dEnc/Defines.h>
#include "util.h"


using namespace osuCrypto;

class GroupChannel {
    public:
        std::vector<Session> nSessions;
        std::vector<Channel> nChannels;
        u64 current_node; // index of current node's ip in vector of ips

        GroupChannel(std::vector<std::string> ips, u64 n, IOService& ios) {
            if (nSessions.size())
                throw std::runtime_error("GroupChannel's connect can be called once " LOCATION);
            nSessions.resize(n);
            
            std::string ip = getIP();
            current_node = getCurrNodeIdx(ips, ip);
            
            for(int i = 0; i < n ; i++) {
                if (i < current_node) {
                    //connect as a client  
                    nSessions[i].start(ios, ips[i], SessionMode::Client);
                    
                    Channel clientChl = nSessions[i].addChannel();
                    nChannels.push_back(clientChl);
                } else if (i == current_node) {
                    // Channel dummyChl;
                    // nChannels.push_back(dummyChl);
                    continue;
                } else {   
                    //connect as a server
                    nSessions[i].start(ios, ip, SessionMode::Server);
                    
                    Channel serverChl = nSessions[i].addChannel();
                    std::chrono::milliseconds timeout(1000000000000);
                    serverChl.waitForConnection();
                    nChannels.push_back(serverChl);
                }
            }
        }
};