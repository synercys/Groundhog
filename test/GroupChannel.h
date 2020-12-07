#pragma once

#include <cryptoTools/Common/config.h>
#include <cryptoTools/Common/Defines.h>
#include <cryptoTools/Network/Session.h>
#include <cryptoTools/Network/IOService.h>
#include <dEnc/Defines.h>
#include "util.h"


namespace osuCrypto
{
    struct GroupChannel
    {
        std::vector<Session> nSessions;
        u64 current_node; // index of current node's ip in vector of ips

        void connect(std::vector<std::string> ips, u64 n, IOService& ios) {
            nSessions.resize(n);
            
            std::string ip = getIP();
            for (int i = 0; i < n ; i++) {
                if (ips[i].compare(ip) == 0) {
                    current_node = i;    
                }
            }
            
            for(int i = 0; i < n ; i++) {
                if (i < current_node) {
                    //connect as a client  
                    nSessions[i].start(ios, ips[i], SessionMode::Client);
                } else if (i == current_node) {
                    continue;
                } else {   
                    //connect as a server
                    nSessions[i].start(ios, ip, SessionMode::Server);
                }
            }
        }
    };
}