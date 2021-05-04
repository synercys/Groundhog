#include "util.h"

using namespace osuCrypto;
#include <bits/stdc++.h>
#include <cryptoTools/Common/Timer.h>
#include <cryptoTools/Network/IOService.h>
#include <cryptoTools/Network/Channel.h>
#include "GroupChannel.h"
#define tryCount 2


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
    std::vector<std::string> tokens{std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{}};

    std::string ip;
    for(int i=0; i< tokens.size(); i++) {
        if(tokens[i].compare("inet") == 0) {
            ip = tokens[i+1];
            break;
        } 
    }

    return ip;
}

u64 getCurrNodeIdx(std::vector<std::string> ips, std::string ip) {
    u64 current_node = -1;
    for (int i = 0; i < ips.size(); i++) {
        if (ips[i].compare(ip) == 0) {
            current_node = i;
            break;  
        }
    }
    return current_node;
}

void getLatency(std::vector<std::string> ips, u64 n)
{
    IOService ios;
    ios.showErrorMessages(true);

    GroupChannel gc(ips, n, ios);

    for (int i = 0; i < n ; i++) 
    {
        if (i == gc.current_node) 
        {
            continue;
        }

        gc.getChannel(i).send(getIP());
        std::string msg;
        gc.getChannel(i).recv(msg);
        std::cout << "Received " << msg << std::endl;
        if (i < gc.current_node) 
        {
            recverGetLatency(gc.getChannel(i));    
        } 
        else 
        {
            senderGetLatency(gc.getChannel(i));
        }
    }
}

void senderGetLatency(Channel& chl)
{
    u8 dummy[1];

    chl.asyncSend(dummy, 1);

    chl.recv(dummy, 1);
    chl.asyncSend(dummy, 1);

    std::vector<u8> oneMbit((1 << 20) / 8);
    for (u64 i = 0; i < tryCount; ++i)
    {
        chl.recv(dummy, 1);

        for(u64 j =0; j < (1<<10); ++j)
            chl.asyncSend(oneMbit.data(), oneMbit.size());
    }
    chl.recv(dummy, 1);

	for (u64 j = 0; j < (1 << 10); ++j)
	{
        try{
		chl.asyncRecv(oneMbit.data(), oneMbit.size());
		chl.asyncSend(oneMbit.data(), oneMbit.size());
        }
        catch(const std::exception& e){
                        std::cerr << e.what() << "util" << '\n';
                        // mListenChls[i] = reconnectChannel( mListenChls[i]);
                    }
	}

	chl.recv(dummy, 1);
}

void recverGetLatency(Channel& chl)
{
    u8 dummy[1];
    chl.recv(dummy, 1);
    Timer timer;
    auto start = timer.setTimePoint("");
    chl.asyncSend(dummy, 1);

    chl.recv(dummy, 1);

    auto mid = timer.setTimePoint("");
    auto recvStart = mid;
    auto recvEnd = mid;

    auto rrt = mid - start;
    std::cout << "latency:   " << std::chrono::duration_cast<std::chrono::microseconds>(rrt).count() << " microseconds" << std::endl;

    std::vector<u8> oneMbit((1 << 20) / 8);
    for (u64 i = 0; i < tryCount; ++i)
    {
        recvStart = timer.setTimePoint("");
        chl.asyncSend(dummy, 1);

        for (u64 j = 0; j < (1 << 10); ++j)
            chl.recv(oneMbit);

        recvEnd = timer.setTimePoint("");

        // nanoseconds per GegaBit
        auto uspGb = std::chrono::duration_cast<std::chrono::nanoseconds>(recvEnd - recvStart - rrt / 2).count();

        // nanoseconds per second
        double usps = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds(1)).count();

        // MegaBits per second
        auto Mbps = usps / uspGb *  (1 << 10);

        std::cout << "bandwidth ->: " << Mbps << " Mbps" << std::endl;
    }

    chl.asyncSend(dummy, 1);
	std::future<void> f;
	recvStart = timer.setTimePoint("");
	for (u64 j = 0; j < (1 << 10); ++j)
	{
        try{
		f = chl.asyncRecv(oneMbit.data(), oneMbit.size());
		chl.asyncSend(oneMbit.data(), oneMbit.size());
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << "Line364" << '\n';
            // mListenChls[i] = reconnectChannel( mListenChls[i]);
        }
	}
	f.get();
	recvEnd = timer.setTimePoint("");
	chl.send(dummy, 1);

	// nanoseconds per GegaBit
	auto uspGb = std::chrono::duration_cast<std::chrono::nanoseconds>(recvEnd - recvStart - rrt / 2).count();

	// nanoseconds per second
	double usps = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds(1)).count();

	// MegaBits per second
	auto Mbps = usps / uspGb *  (1 << 10);

	std::cout << "bandwidth <->: " << Mbps << " Mbps" << std::endl;
}

bool isPrime(u64 n) {
    if (n <= 1)  return false;  
    if (n <= 3)  return true;  
    if (n%2 == 0 || n%3 == 0) return false;  
    
    for (u64 i=5; i*i<=n; i=i+6)
        if (n%i == 0 || n%(i+2) == 0)  
           return false;  
    
    return true;  
}

u64 findNextPrime(u64 n) {
    if (n <= 1) 
        return 2; 
  
    u64 prime = n;
    while (1) {
        prime++; 
        if (isPrime(prime)) 
            break; 
    } 
  
    return prime; 
}

u64 rangeRand(u64 range_from, u64 range_to) {
    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());
    std::uniform_int_distribution<u64> distr(range_from, range_to);
    return distr(generator);
}