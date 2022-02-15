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
        std::vector<Channel> nSendChannels;
        std::vector<Channel> nRecvChannels;
        u64 current_node; // index of current node's ip in vector of ips
        // TODO: try making separate receive and listen channels? rather than reusing one depending on index
        GroupChannel(std::vector<std::string> ips, u64 n, IOService& ios) {
            nSessions.resize(n);
            
            std::string ip = getIP();
            current_node = getCurrNodeIdx(ips, ip);
            if (current_node >= n) {
                std::cout << "My IP is " << ip << ", but I couldn't find it in the list of node IPs" << std::endl;
            } else
                std::cout << "My ip is " << ip << ", my idx is " << current_node << std::endl;
            
            for(int i = 0; i < n ; i++) {
                if (i > 0) {
                    //connect as a client  
                    nSessions[i].start(ios, ips[i], SessionMode::Client);
                    Channel clientChl = nSessions[i].addChannel("request", "listen");
                    Channel serverChl = nSessions[i].addChannel("listen", "request");
                    std::chrono::milliseconds timeout(10000000);
                    clientChl.waitForConnection(timeout);
                    serverChl.waitForConnection(timeout);
                    nSendChannels.push_back(clientChl);
                    nRecvChannels.push_back(serverChl);

                } else if (i == current_node) {
                    // Channel dummyChl;
                    // nChannels.push_back(dummyChl);
                    continue;
                } else { // i > current_node
                    //connect as a server
                    nSessions[i].start(ios, ip, SessionMode::Server);
                    Channel clientChl = nSessions[i].addChannel("request", "listen");
                    Channel serverChl = nSessions[i].addChannel("listen", "request");
                    std::chrono::milliseconds timeout(10000000000);
                    clientChl.waitForConnection(timeout);
                    serverChl.waitForConnection(timeout);
                    nSendChannels.push_back(clientChl);
                    nRecvChannels.push_back(serverChl);
                }
                /*
                std::cout << "setting up idx " << i << ", mine is " << current_node << std::endl;
                if (i == current_node) {
                    std::cout << "my idx is " << i << std::endl;
                    continue;
                }
                if (i < current_node)
                    nSessions[i].start(ios, ips[i], SessionMode::Client);
                else
                    nSessions[i].start(ios, my_ip+":8080", SessionMode::Server);
                
                Channel newChannel = nSessions[i].addChannel();
                std::chrono::milliseconds timeout(1000 * 1000);
                newChannel.waitForConnection(timeout);
                nChannels.push_back(newChannel);*/
            }
        }
/*
        Channel& getChannel(u64 nodeIdx) {
            if (nodeIdx == current_node)
                throw std::runtime_error("No Channel for current_node");
            
            if (nodeIdx < current_node)
                return nChannels[nodeIdx];
            else
                return nChannels[nodeIdx-1];
        }
*/
        Session& getSession(u64 nodeIdx) {
            return nSessions[nodeIdx];
        }
/*
        Channel& reconnectChannel(u64 nodeIdx, Channel old, IOService& ios,std::string ip)
        {
            
            Session client(ios, ip, SessionMode::Client);
            Channel clientChl = client.addChannel();
            std::chrono::milliseconds timeout(10000000);
            clientChl.waitForConnection(timeout);
            std::replace(nChannels.begin(), nChannels.end(),old, clientChl);
            return clientChl;

        }*/
};