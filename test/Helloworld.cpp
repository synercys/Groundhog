#include <cryptoTools/Common/CLP.h>
#include <cryptoTools/Common/Timer.h>
#include <cryptoTools/Network/IOService.h>
#include <dEnc/distEnc/AmmrClient.h>
#include <dEnc/dprf/Npr03SymDprf.h>
#include "Algorithm.h"
#include "GroupChannel.h"
#include "RandomNodePicker.h"
#include "util.h"


using namespace osuCrypto;

static const std::vector<std::string> ips {"172.31.42.227","172.31.37.209","172.31.36.13","172.31.46.44"};


void writeSeed(oc::block seed, std::string stateFileName) 
{
    std::ofstream stateFile(stateFileName);
    stateFile << seed;
    stateFile.close();
    
    std::cout << "stored seed in file" << std::endl;
}

u32 convertString(std::string str)
{
    return std::stoul(str, nullptr, 16);
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
    std::vector<std::vector<block>> data(batch), ciphertext(batch);
    for (auto& d : data) d.resize(blockCount);

    // we are interested in latency and therefore we 
    // will only have one encryption in flight at a time.
    for (u64 t = 0; t < trials; ++t) 
        initiator.encrypt(data[0], ciphertext[0]);

    auto e = t.setTimePoint("end");

    enc.close();

    auto online = (double)std::chrono::duration_cast<std::chrono::milliseconds>(e - s).count();

    // print the statistics.
    std::cout << tag <<"      n:" << n << "  m:" << m << "   t:" << trials << "     enc/s:" << 1000 * trials / online << "   ms/enc:" << online / trials << " \t "
        << " Mbps:" << (trials * sizeof(block) * 2 * (m - 1) * 8 / (1 << 20)) / (online / 1000)
        << std::endl;
}


void AmmrSymClient_tp_Perf_test(u64 n, u64 m, u64 blockCount, u64 trials, u64 numAsync, u64 batch, bool lat, std::string stateFileName)
{
    // set up the networking
    IOService ios;
    GroupChannel gc(ips, n, ios);

    
    std::cout << "asymclient" << std::endl;
    oc::block seed;
    std::ifstream stateFile(stateFileName);
    if (stateFile.good()) { // file exists
        std::string e;
        stateFile >> e;
        std::cout<< e << std::endl;
        auto ret = std::array<u32, 4>{convertString(e.substr(24,8)), convertString(e.substr(16,8)) , convertString(e.substr(8,8)), convertString(e.substr(0,8))};
        memcpy(&seed, &ret, sizeof(block));
        std::cout << "seed from file converted :" << seed <<std::endl;
        stateFile.close();
    }
    else{
        if(gc.current_node == 0)
        {
            // Initialize the parties using a random seed from the OS.
            seed = sysRandomSeed();
            for (u64 i = 1; i < n; i++)
            {
                gc.getChannel(i).send(seed);
            }
        } else 
        {
            try
            {
                gc.getChannel(0).recv(seed);
                writeSeed(seed,stateFileName);
                std::cout << "seed=" << seed << std::endl;
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
                //gc.reconnectChannel(i,ios,ips[i]);
            }
            
        }
    }

    // allocate the DPRFs and the encryptors
    dEnc::AmmrClient<dEnc::Npr03SymDprf> enc;
    dEnc::Npr03SymDprf dprf;

    std::cout << "Line 112="  << std::endl;
    PRNG prng(seed);
    // Generate the master key for this DPRF.
    dEnc::Npr03SymDprf::MasterKey mk;
    mk.KeyGen(n, m, prng);
    std::cout << "Line 117="  << std::endl;

    // initialize the DPRF and the encrypters
    dprf.init(gc.current_node, m, gc.nChannels, gc.nChannels, prng.get<block>(), mk.keyStructure, mk.getSubkey(gc.current_node));
    std::cout << "Line 121="  << std::endl;
    enc.init(gc.current_node, prng.get<block>(), &dprf);
    std::cout << "Line 123="  << std::endl;
    
    // Perform the benchmark.                                          
    if (gc.current_node == 0) {
        eval(enc, n, m, blockCount, batch, trials, numAsync, lat, "Sym      ");
    }
    else{
        while(1);
    }
}


void try_connect(u64 n)
{
    // set up the networking
    IOService ios;
    GroupChannel gc(ips, n, ios);
     

    while(true)
    {
        Channel chl0 = gc.getChannel(0);
        try
        {
            chl0.send(getIP());
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            gc.reconnectChannel(0,chl0,ios,ips[0]);
        }  
                
    }

}


int main(int argc, char** argv) {

        CLP cmd;
        cmd.parse(argc, argv);

        // u64 n = cmd.get<u64>("n");
        // RandomNodePicker nodePicker(n);
        // std::cout << "Generators for n=" << n << " are " << std::endl;
        // for(u64 i = 0; i < nodePicker.generators.size(); i++) {
        //     std::cout << nodePicker.generators[i].first << ": ";
        //     for (u64 x: nodePicker.generators[i].second)
        //         std::cout << x << " ";
        //     std::cout << std::endl;
        // }

        // u64 attackTime = 300, rebootTime = 100, t = 5;
        // std::string stateFileName = "reboot_state";

        // std::cout << "attackTime: " << attackTime << "ms" << std::endl;
        // std::cout << "rebootTime: " << rebootTime << "ms" << std::endl;
        // std::cout << "t+1: " << t+1 << std::endl;
        // for (int i = 0; i < 5; i++) {
        //     std::cout << "------------------" << std::endl;
        //     RandomNodePicker _nodePicker(n);
        //     Algorithm algorithm(ips, n, attackTime, rebootTime, t, _nodePicker, stateFileName);
        //     algorithm.run();
        // }
        
        // std::remove(stateFileName.c_str()); // remove file if exists

        u64 n = ips.size();
        //getLatency(ips, n);

        u64 t = 4096;
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
        std::string stateFileName = "/home/ubuntu/redise/dise/test/seed_file";

        if (m > n)
        {
            std::cout << "can not have a threshold larger than the number of parties. theshold=" << m << ", #parties=" << n << std::endl;
            return -1;
        }

        AmmrSymClient_tp_Perf_test(n, m, size, t, a, b, l, stateFileName);
    

    return 0;
    //try_connect(n);
    }
        

