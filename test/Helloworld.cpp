#include <cryptoTools/Common/CLP.h>
#include <cryptoTools/Common/Timer.h>
#include <cryptoTools/Network/IOService.h>
#include <dEnc/distEnc/AmmrClient.h>
#include <thread>
#include <dEnc/dprf/Npr03SymDprf.h>
#include "Algorithm.h"
#include "GroupChannel.h"
#include "RandomNodePicker.h"
#include "util.h"


using namespace osuCrypto;

bool eq(span<block> a, span<block>b)
{
    return (a.size() == b.size() &&
        memcmp(a.data(), b.data(), a.size() * sizeof(block)) == 0);
};


static const std::vector<std::string> ips {"10.0.0.228","10.0.0.6", "10.0.0.204", "10.0.0.135"};

void try_connect(u64 n, IOService *ios)
{
    // set up the networking
    //IOService ios;
    //GroupChannel gc(ips, n, ios);
    std::string msg;
    std::string ip = getIP();
    std::cout << "attackTime: ";
    

    while(true)
    {
        for (u64 i = 1; i < n; i++)
        {
            IOService ios;
            Session perPartySession(ios, ip, SessionMode::Server /* , serviceName */);
            Channel serverChl = perPartySession.addChannel();
            //try
            //{
                //chl0.send(ip);
                serverChl.recv(msg);
                std::cout << msg << std::endl;
            //}
            // catch(const std::exception& e)
            // {
            //     std::cerr << e.what() << '\n';
            //     //gc.reconnectChannel(i,ios,ips[i]);
            // }

            
        }            
    }

}

template<typename DPRF>
void eval(dEnc::AmmrClient<DPRF>& enc, u64 n, u64 m, u64 blockCount, u64 batch, u64 trials, u64 numAsync, bool lat, std::string tag)
{
    Timer t;
    auto s = t.setTimePoint("start");

    // The party that will initiate the encryption.
    // The other parties will respond to requests on the background.
    // This happens using the threads created by IOService
    auto& initiator = enc;

    // the buffers to hold the data.
    std::vector<std::vector<block>> data(batch), ciphertext(batch), data2(batch);
    for (auto& d : data) d.resize(blockCount);

    // we are interested in latency and therefore we 
    // will only have one encryption in flight at a time.
    for (u64 t = 0; t < trials; ++t)
    {
        //try
        //{
            // std::cout << t << std::endl;
            // std::chrono::milliseconds timespan(300);
            // std::this_thread::sleep_for(timespan);
            initiator.encrypt(data[0], ciphertext[0]);

            //check if encryption and decryption works
            //initiator.decrypt(ciphertext[0],data2[0]);

            // if (!eq(data[0], data2[0]))
            //     throw std::runtime_error(LOCATION);
        //}
        /*catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            std::cout << "encrypt" << std::endl;
            //gc.reconnectChannel(i,ios,ips[i]);
        }*/
    }

    auto e = t.setTimePoint("end");

    enc.close();

    auto online = (double)std::chrono::duration_cast<std::chrono::milliseconds>(e - s).count();

    // print the statistics.
    std::cout << tag <<"      n:" << n << "  m:" << m << "   t:" << trials << "     enc/s:" << 1000 * trials / online << "   ms/enc:" << online / trials << " \t "
        << " Mbps:" << (trials * sizeof(block) * 2 * (m - 1) * 8 / (1 << 20)) / (online / 1000)
        << std::endl;
}


void AmmrSymClient_tp_Perf_test(u64 n, u64 m, u64 blockCount, u64 trials, u64 numAsync, u64 batch, bool lat)
{
    // set up the networking
    IOService ios;
    GroupChannel gc(ips, n, ios);

    // std::cout << "asymclient" << std::endl;
    oc::block seed;
    if(gc.current_node == 0)
    {
        // Initialize the parties using a random seed from the OS.
        seed = sysRandomSeed();
        for (u64 i = 1; i < n; i++)
        {
            try
            {
                gc.getChannel(i).send(seed);
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
                //gc.reconnectChannel(i,ios,ips[i]);
            }
        }
    } else 
    {
        gc.getChannel(0).recv(seed);
    }

    // allocate the DPRFs and the encryptors
    dEnc::AmmrClient<dEnc::Npr03SymDprf> enc;
    dEnc::Npr03SymDprf dprf;

    PRNG prng(seed);
    // Generate the master key for this DPRF.
    dEnc::Npr03SymDprf::MasterKey mk;
    mk.KeyGen(n, m, prng);

    // initialize the DPRF and the encrypters
    dprf.init(gc.current_node, m, gc.nChannels, gc.nChannels, prng.get<block>(), mk.keyStructure, mk.getSubkey(gc.current_node));
    enc.init(gc.current_node, prng.get<block>(), &dprf);
    
    // Perform the benchmark.                                          
    if (gc.current_node == 0) {
            eval(enc, n, m, blockCount, batch, trials, numAsync, lat, "Sym      ");
    }
}




int main(int argc, char** argv) 
{

    //std::cout << "attackTime2: " << std::endl;

            
    CLP cmd;
    cmd.parse(argc, argv);

    u64 n = ips.size();
    //getLatency(ips, n);

    u64 t = 10000;
    u64 b = 128;
    u64 a = 1024 / b;
    cmd.setDefault("t", t);
    cmd.setDefault("b", b);
    cmd.setDefault("a", a);
    cmd.setDefault("size", 20);
    t = cmd.get<u64>("t");
    b = cmd.get<u64>("b");
    a = cmd.get<u64>("a");
    auto size = cmd.get<u64>("size");
    bool l = cmd.isSet("l");

    cmd.setDefault("mf", "0.5");
    auto mFrac = cmd.get<double>("mf");
    if (mFrac <= 0 || mFrac > 1)
    {
        std::cout << ("bad mf") << std::endl;
        return 0;
    }

    cmd.setDefault("mc", -1);
    auto mc = cmd.get<i64>("mc");

    auto m = std::max<u64>(2, (mc == -1) ? n * mFrac : mc);
    m = 2;

    if (m > n)
    {
        std::cout << "can not have a threshold larger than the number of parties. theshold=" << m << ", #parties=" << n << std::endl;
        return -1;
    }

    
    AmmrSymClient_tp_Perf_test(n, m, size, t, a, b, l);
    return 0;
    //try_connect(n);
}
