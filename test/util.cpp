#include "util.h"

using namespace osuCrypto;
#include <test/GroupChannel.h>
#include <cryptoTools/Common/Log.h>
#include <cryptoTools/Common/Timer.h>
#include <cryptoTools/Network/IOService.h>
#include <cryptoTools/Network/Endpoint.h>
#include <cryptoTools/Network/Channel.h>
#define tryCount 2



void getLatency(std::vector<std::string> ips, u64 n)
{
	GroupChannel gc;
    gc.connect(ips, n);

    if (gc.current_node != 0) {
        u8 dummy[1];
        gc.nChls[0].asyncRecv(dummy, 1);
        std::cout << "Received " << dummy[0] << std::endl;
        // recverGetLatency(gc.nChls[0]);
        return;
    }

    for (int i = 1; i < n ; i++) {
        u8 dummy[1];
        dummy[0] = 0;
        gc.nChls[i].asyncSend(dummy, 1);
        // senderGetLatency(gc.nChls[i]);
    }

    // for (int i = 0; i < n ; i++) 
    // {
    //     if(i < gc.current_node) 
    //     {
    //         recverGetLatency(gc.nChls[i]);
    //     }
    //     else if(i == gc.current_node) {
    //         continue;
    //     }
    //     else
    //     {
    //         std::cout << "" << std::endl;
    //         senderGetLatency(gc.nChls[i]);
    //     }
    // }
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
		chl.asyncRecv(oneMbit.data(), oneMbit.size());
		chl.asyncSend(oneMbit.data(), oneMbit.size());
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
    std::cout << "latency:   " << std::chrono::duration_cast<std::chrono::milliseconds>(rrt).count() << " ms" << std::endl;

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
		f = chl.asyncRecv(oneMbit.data(), oneMbit.size());
		chl.asyncSend(oneMbit.data(), oneMbit.size());
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
