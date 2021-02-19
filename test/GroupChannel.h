#pragma once

#include <cryptoTools/Common/config.h>
#include <cryptoTools/Common/Defines.h>
#include <cryptoTools/Network/Session.h>
#include <cryptoTools/Network/IOService.h>
#include <dEnc/Defines.h>
#include "util.h"
#include <algorithm>


using namespace osuCrypto;

class GroupChannel {
    public:
        std::vector<Session> nSessions;
        std::vector<Channel> nChannels;
        u64 current_node; // index of current node's ip in vector of ips

        GroupChannel(std::vector<std::string> ips, u64 n, IOService& ios) {
            nSessions.resize(n);
            
            std::string ip = getIP();
            current_node = getCurrNodeIdx(ips, ip);
            
            for(int i = 0; i < n ; i++) {
                if (i < current_node) {
                    //connect as a client  
                    nSessions[i].start(ios, ips[i], SessionMode::Client);
                    Channel clientChl = nSessions[i].addChannel();
                    std::chrono::milliseconds timeout(10000000);
                    clientChl.waitForConnection(timeout);
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
                    serverChl.waitForConnection(timeout);
                    nChannels.push_back(serverChl);

                }
            }
        }

        Channel& getChannel(u64 nodeIdx) {
            if (nodeIdx == current_node)
                throw std::runtime_error("No Channel for current_node");
            
            if (nodeIdx < current_node)
                return nChannels[nodeIdx];
            else
                return nChannels[nodeIdx-1];
        }

        Session& getSession(u64 nodeIdx) {
            return nSessions[nodeIdx];
        }

        Channel& connectasClient(std::string ip, IOService& ios, u64 nodeIdx)
        {
            nSessions[nodeIdx].stop();
            nSessions[nodeIdx].start(ios, ip, SessionMode::Client);
            Channel clientChl = nSessions[nodeIdx].addChannel();
            std::chrono::milliseconds timeout(10000000);
            clientChl.waitForConnection(timeout);
            std::replace(nChannels.begin(), nChannels.end(), getChannel(nodeIdx), clientChl);
            return clientChl;    

        }

        Channel& connectasServer(std::string ip, IOService& ios, u64 nodeIdx)
        {
            nSessions[nodeIdx].stop();
            nSessions[nodeIdx].start(ios, ip, SessionMode::Server);
            Channel serverChl = nSessions[nodeIdx].addChannel();
            std::chrono::milliseconds timeout(1000000000000);
            serverChl.waitForConnection(timeout);
            std::replace(nChannels.begin(), nChannels.end(), getChannel(nodeIdx), serverChl);
            return serverChl;
        }


        Channel& reconnectChannel(u64 nodeIdx, IOService& ios,std::string ip)
        {
            
            if (nodeIdx < current_node) {
                    return connectasClient(ip, ios, nodeIdx);
                } 
            else{   
                   return connectasServer(ip, ios, nodeIdx);
                }

        }
};