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


namespace osuCrypto
{
    struct GroupChannel
    {
        //std::vector<Channel> nChls; // to send and receive msgs
        std::vector<Session> nSessions;
        u64 current_node; // index of current node's ip in vector of ips

        void connect(std::vector<std::string> ips, u64 n) {
            nSessions.resize(n);

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
                    //Channel clientChl = Session(ios, ips[i], SessionMode::Client).addChannel();
                    nSessions[i].start(ios, ips[i], SessionMode::Client);

                } 
                else if (i == current_node) 
                {
                    continue;
                } 
                else 
                {  //connect as a server
                    nSessions[i].start(ios, ip, SessionMode::Server);
                    // Channel serverChl = perPartySession.addChannel();
                    // std::chrono::milliseconds timeout(1000000000000);
                    // serverChl.waitForConnection();
                }
                
            }
        }

    };

}