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
        std::vector<Session> mSessions;
        std::vector<Channel> mRequestChls, mListenChls;
        u64 current_node; // index of current node's ip in vector of ips

        GroupChannel(std::vector<std::string> ips, u64 numParties, oc::IOService& ios) {
            std::string ip = getIP();
            u64 partyIdx = getCurrNodeIdx(ips, ip);
            current_node = partyIdx;

            if (partyIdx >= numParties)
                std::cout << "My IP is " << ip << ", but I couldn't find it in the list of node IPs" << std::endl;
            //else
            //    std::cout << "My IP is " << ip << ", my idx is " << partyIdx << std::endl;
            
            if (mSessions.size())
                throw std::runtime_error("connect can be called once " LOCATION);

            if (partyIdx == 0) {
                mSessions.resize(numParties-1);
                mRequestChls.resize(numParties-1);
                mListenChls.resize(numParties-1);

                for (u64 i = 1; i < numParties; ++i)
                {
                    mSessions[i-1].start(ios, ip, oc::EpMode::Server);
                    auto listenChl = mSessions[i-1].addChannel("listen", "request");
                    auto requestChl = mSessions[i-1].addChannel("request", "listen");

                    std::chrono::milliseconds timeout(numParties * 1 * 1000);
                    requestChl.waitForConnection(timeout);
                    listenChl.waitForConnection(timeout);

                    mListenChls[i-1] = listenChl;
                    mRequestChls[i-1] = requestChl;
                }
            } else {
                mSessions.resize(1);
                mRequestChls.resize(1);
                mListenChls.resize(1);

                mSessions[0].start(ios, ips[0], oc::EpMode::Client);
                auto listenChl = mSessions[0].addChannel("listen", "request");
                auto requestChl = mSessions[0].addChannel("request", "listen");

                std::chrono::milliseconds timeout(numParties * 1 * 1000);
                listenChl.waitForConnection(timeout);
                requestChl.waitForConnection(timeout);

                mListenChls[0] = listenChl;
                mRequestChls[0] = requestChl;
            }
        }
/*
        GroupChannel(std::vector<std::string> ips, u64 n, IOService& ios) {
            nSessions.resize(n);
            
            std::string ip = getIP();
            current_node = getCurrNodeIdx(ips, ip);
            if (current_node >= n) {
                std::cout << "My IP is " << ip << ", but I couldn't find it in the list of node IPs" << std::endl;
            } else
                std::cout << "My ip is " << ip << ", my idx is " << current_node << std::endl;
            
            for(int i = 0; i < n ; i++) {
                if (i > 0 && i != current_node) {
                    //connect as a client
                    auto mode = i < current_node ? SessionMode::Client : SessionMode::Server;
                    nSessions[i].start(ios, ips[i], mode);

                    Channel requestChl = nSessions[i].addChannel("request", "listen");
                    Channel listenChl  = nSessions[i].addChannel("listen", "request");

                    std::chrono::milliseconds timeout(10000000);
                    requestChl.waitForConnection(timeout);
                    listenChl.waitForConnection(timeout);

                    nRequestChls.push_back(requestChl);
                    nListenChls.push_back(listenChl);

                } else if (i == current_node) {
                    continue;
                }*//* else { // i > current_node
                    //connect as a server
                    nSessions[i].start(ios, ip, SessionMode::Server);
                    Channel requestChl = nSessions[i].addChannel("request", "listen");
                    Channel listenChl = nSessions[i].addChannel("listen", "request");
                    std::chrono::milliseconds timeout(10000000000);
                    requestChl.waitForConnection(timeout);
                    listenChl.waitForConnection(timeout);
                    nRequestChls.push_back(requestChl);
                    nListenChls.push_back(listenChl);
                }*/
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
            //}
        //}
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
            return mSessions[nodeIdx];
        }
/*
        Channel& reconnectChannel(u64 nodeIdx, Channel old, IOService& ios,std::string ip)
        {
            
            Session client(ios, ip, SessionMode::Client);
            Channel requestChl = client.addChannel();
            std::chrono::milliseconds timeout(10000000);
            requestChl.waitForConnection(timeout);
            std::replace(nChannels.begin(), nChannels.end(),old, requestChl);
            return requestChl;

        }*/
};