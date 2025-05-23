#include <cryptoTools/Common/CLP.h>
#include <cryptoTools/Common/Timer.h>
#include <cryptoTools/Network/IOService.h>
#include <dEnc/distEnc/AmmrClient.h>
#include <dEnc/dprf/Npr03SymDprf.h>
#include "Algorithm.h"
#include "GroupChannel.h"
#include "RandomNodePicker.h"
//#include "util.h"
#include <unistd.h>

using namespace osuCrypto;

std::vector<std::string> ips;


template<typename DPRF>
void eval(dEnc::AmmrClient<DPRF>& enc, u64 n, u64 m, u64 blockCount, u64 batch, u64 trials,
u64 numAsync, bool lat, std::string tag)
{
    // The party that will initiate the encryption.
    // The other parties will respond to requests on the background.
    // This happens using the threads created by IOService
    dEnc::AmmrClient<DPRF>& initiator = enc;

    /*
    * std::cout<<"In "<<__func__<<std::endl;
    */

    // the buffers to hold the data.
    std::vector<std::vector<block>> data(batch), ciphertext(batch);
    for (std::vector<block>& d : data) d.resize(blockCount);

    Timer t;
    auto s = t.setTimePoint("start");
    unsigned int count_aborts = 0;
    unsigned int ctr = 0;

    // we are interested in latency and therefore we 
    // will only have one encryption in flight at a time.
    std::cout<<"---- Size of Plain Text = "<<sizeof(data[0])<<" "<<
        "Number of Trials = "<<trials<<std::endl;

    for (u64 t = 0; t < trials; ++t) {
        // ASHISH TODO: get result ? check if abort.(Check with Prof) If abort continue with next trial
        try{
            // std::cout << "Initiate encryption trial #" << ctr++<< std::endl;
            initiator.encrypt(data[0], ciphertext[0]);
        }
        catch(const std::exception& e){ 
            count_aborts++;
            std::cout<<"Encryption Aborts in HelloWorld"<<std::endl;
        }
    }

    auto e = t.setTimePoint("end");

    enc.close();

    auto online = (double)std::chrono::duration_cast<std::chrono::milliseconds>(e - s).count();

    // Print the statistics.
    std::cout << tag <<"      n:" << n << "  m:" << m << "   t:" << trials
        << "     enc/s:" << 1000 * trials / online << "   ms/enc:" << online / trials << " \t "
        << " Mbps:" << (trials * sizeof(block) * 2 * (m - 1) * 8 / (1 << 20)) / (online / 1000)<<" "<<"Aborts = "<<count_aborts<<" "
        <<"Time (ms) = "<<online<<std::endl;
}


void AmmrSymClient_tp_Perf_test(u64 n, u64 m, u64 blockCount, u64 trials, u64 numAsync,
u64 batch, bool lat, bool isClient)
{

    //ASHISH TODO: Read uptime server reboot sequence file and store in a vector. 
    // every 30 seconds we reboot something. So lets say we want to run for 3 mins. 

    std::string state_file = "state.txt";
    std::vector<float> times;
    std::vector<std::string> states;
    std::ifstream state_file_handle(state_file);

    // char ch;
    // while(!state_file_handle.eof()){
    //     state_file_handle>>ch;
    //     cur_state.push_back(ch);
    // }

    // set up the networking
    IOService ios;
    ios.showErrorMessages(true);
    GroupChannel gc(ips, n, ios, isClient);

    // oc::block seed;
    // if(isClient)
    // {
    //     // Initialize the parties using a random seed from the OS.
    //     seed = sysRandomSeed();
    //     for (u64 i = 0; i < n-1; i++)
    //     {
    //         gc.mRequestChls[i].send(seed);
    //     }
    //     std::cout << "Done sending seed. Starting key exchange." << std::endl;
    // } else 
    // {
    //     gc.mListenChls[0].recv(seed);
    // }

    // allocate the DPRFs and the encryptors
    dEnc::AmmrClient<dEnc::Npr03SymDprf> enc;
    dEnc::Npr03SymDprf dprf;

    PRNG prng(oc::ZeroBlock);
    // Generate the master key for this DPRF.
    dEnc::Npr03SymDprf::MasterKey mk;
    mk.KeyGen(n, m, prng);

    // initialize the DPRF and the encrypters
    // ASHISH TODO: In init pass the sequence vector. Fix compilation issue. 
    try{
        dprf.init(times, states, gc.current_node, m, n,
            gc.mRequestChls, gc.mListenChls, prng.get<block>(),
            mk.keyStructure, mk.getSubkey(gc.current_node)
        );
    }
    catch(const std::exception& e){
        std::cout<<"T1"<<std::endl;
    }

    try{
        enc.init(gc.current_node, prng.get<block>(), &dprf);
    }
    catch(const std::exception& e){
        std::cout<<"T2"<<std::endl;
    }
    
    // Perform the benchmark.
    if (isClient) {
        std::cout << "Key exchange done. Starting  benchmark." << std::endl;
        eval(enc, n, m, blockCount, batch, trials, numAsync, lat, "Net      ");
    }
    dprf.processTimes();
    //dprf.printAbortStats();
}

/*
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
*/

void get_ips(int count, std::vector<std::string> &out) {
    std::string subnet = "10.0.";
    std::stringstream ss;
    count += 2; // 10.0.0.1 is gateway, 10.0.0.2 is client, 10.0.0.3+ are server nodes
    for (int node_index = 2; node_index < count; node_index++) {
        ss << subnet << (node_index>>8) << '.' << (node_index & 0xff);
        out.push_back(ss.str());
        ss.str( std::string() );
    }
}

int main(int argc, char** argv) {
    CLP cmd;
    cmd.parse(argc, argv);
    std::cout<<"Application Start "<<__FILE__<<" "<<__LINE__<<std::endl;

    /*
    for (int d = 0; d <= ips.size(); d++)
       std::cout << ips[d] << std::endl;

    u64 n = cmd.get<u64>("n");
    RandomNodePicker nodePicker(n);
    std::cout << "Generators for n=" << n << " are " << std::endl;
    for(u64 i = 0; i < nodePicker.generators.size(); i++) {
        std::cout << nodePicker.generators[i].first << ": ";
        for (u64 x: nodePicker.generators[i].second)
            std::cout << x << " ";
        std::cout << std::endl;
    }

    u64 attackTime = 300, rebootTime = 100, t = 5;
    std::string stateFileName = "reboot_state";

    std::cout << "attackTime: " << attackTime << "ms" << std::endl;
    std::cout << "rebootTime: " << rebootTime << "ms" << std::endl;
    std::cout << "t+1: " << t+1 << std::endl;
    for (int i = 0; i < 5; i++) {
        std::cout << "------------------" << std::endl;
        RandomNodePicker _nodePicker(n);
        Algorithm algorithm(ips, n, attackTime, rebootTime, t, _nodePicker, stateFileName);
        algorithm.run();
    }
    
    std::remove(stateFileName.c_str()); // remove file if exists
    */

    u64 b = 128;
    u64 a = 1024 / b;
    cmd.setDefault("a", a);
    b = cmd.getOr<u64>("b", b);
    a = cmd.get<u64>("a");
    
    u64 t    = cmd.getOr<u64>("t",  4096);
    u64 n    = cmd.getOr<u64>("n",     9) + 1; // +1 for client
    u64 size = cmd.getOr<u64>("size", 20);
    i64 mc   = cmd.getOr<i64>("mc",   -1);
    float mf = cmd.getOr<float>("mf",  0.5);

    bool isClient = cmd.isSet("client");
    bool l = cmd.isSet("l");

    if (mf <= 0 || mf > 1){
        std::cout << "bad mf. Must be in (0,1]" << std::endl;
        return -2;
    }

    u64 m = std::max<u64>(2, (mc == -1) ? n * mf : mc);

    /*
    * Uncomment below line to enable reviewer's protocol
    * TODO : Make this a command-line argument in the application
    */
    //m = 1;

    if (m > n)
    {
        std::cout << "cannot have a threshold larger than the number of parties. "
            << "threshold=" << m << ", #parties=" << n << std::endl;
        return -1;
    }

    get_ips(n, ips);
    //getLatency(ips, n);

    //for(u64 ii = 0; ii < 50; ii++)
    AmmrSymClient_tp_Perf_test(n, m, size, t, a, b, l, isClient);

    return 0;
    //try_connect(n);
}
