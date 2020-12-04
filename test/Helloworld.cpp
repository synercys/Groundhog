#include <iostream>
#include <cryptoTools/Common/config.h>
#include <cryptoTools/Common/Defines.h>
#include <cryptoTools/Network/Channel.h>
#include <cryptoTools/Network/Session.h>
#include <cryptoTools/Network/IOService.h>


using namespace osuCrypto;

int main(int argc, char** argv)
{
    IOService ios(2);

    ios.showErrorMessages(true);

    auto ip1 = std::string("10.0.60.177");
    auto ip2 = std::string("10.0.60.64");
    auto port = 1212;

    // std::string serversIpAddress = ip1 + ':' + std::to_string(port);
    std::string serversIpAddress = ip1;
    std::string sessionHint = "party0_party1";

    // Create a pair of sessions that connect to eachother. Note that
    // the sessionHint parameter is options.
    Session server(ios, serversIpAddress, SessionMode::Server, sessionHint);
    
    std::string channelName = "channelName";
    Channel namedChl1 = server.addChannel(channelName);

    std::chrono::milliseconds timeout(1000000000000);
    namedChl1.waitForConnection();

    namedChl1.onConnect([](const error_code& ec) {
        if (ec)
        {
            std::cout << "chl0 failed to connect: " << ec.message() << std::endl;
        }
        else
        {
            std::cout << "Channel connected = " << std::endl;
        }
    });

    
    std::vector<int> data{ 0,1,2,3,4,5,6,7 };
    namedChl1.send(data);

    std::vector<int> dest;
    namedChl1.recv(dest);

    for(int i=0; i < dest.size(); i++)
        std::cout << dest.at(i) << ' ';
    

    std::cout << "Channel connected = " << namedChl1.isConnected() << std::endl;

   



}