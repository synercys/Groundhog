#include "Npr03SymDprf.h"
#include <boost/math/special_functions/binomial.hpp>
#include <cryptoTools/Common/Timer.h>
#include <cryptoTools/Network/Session.h>
#include <cryptoTools/Network/IOService.h>
#include <cryptoTools/Network/Channel.h>
#include <cryptoTools/Common/BitVector.h>
#include <cryptoTools/Common/MatrixView.h>
#include <chrono>
#include <fstream>
#include <numeric>

#define PORT  5000

const unsigned char proto_dn = 'd', proto_up = 'u', proto_rq = 'r';

namespace dEnc {


    Npr03SymDprf::~Npr03SymDprf()
    {
        close();

        // if we have started listening to the network, then 
        // wait for the server callbacks to complete.
        if (mServerListenCallbacks.size())
            mServerDone.get();
    }

    //void processStateFile(std::string filename, std::vector<float>& times, std::vector<std::string>& states);

    void Npr03SymDprf::MasterKey::KeyGen(u64 n, u64 m, PRNG & prng)
    {

        // each subkey i will be distributed to subsetSize-out-of-n of the parties.
        auto subsetSize = n - m + 1;

        // the number of sub keys
        i64 d = boost::math::binomial_coefficient<double>(n, subsetSize);

        // the number of keys each party will hold.
        i64 cc = boost::math::binomial_coefficient<double>(n - 1, subsetSize - 1);

        // A matrix where row i contains a list of the key that party i will hold.
        subKeys.resize(n, cc);
        keyStructure.resize(n, cc);

        // currentPartyKeyCount[i] is the number of keys that have been given to party i.
        std::vector<u64> currentPartyKeyCount(n, 0);

        // a vector that will hold the current subset of the parties what will
        // receive the current key, keys[curKeyIdx]
        std::vector<u64> cur; cur.reserve(subsetSize);
        u64 curKeyIdx = 0;

        // Gnerate the keys using the PRNG.
        keys.resize(d);
        prng.get(keys.data(), keys.size());


        // This is a recursive function which iterates through every
        // subset of {0, 1, ..., n-1} with size subsetSize. When
        // such a subset is found then it holds that remainingRoom == 0. To this
        // subset we give the key, keys[curKeyIdx]. Then the next 
        // such subset is found and given the next key.
        std::function<void(u64, u64)> go = [&](u64 curSize, u64 remainingRoom)
        {
            if (remainingRoom)
            {
                // the idea here is that we will consider the following sets
                // cur' = cur U { i }
                // cur' = cur U { i+1 }
                //    ...
                // cur' = cur U { n-1 }
                // For each of these we will recurse until we get a large enough set.
                // Note that cur is a subset of  {0,1,...,i-1} and therefore we wont
                // repeat any index.
                for (u64 i = curSize; i < n; ++i)
                {
                    cur.push_back(i);
                    go(i + 1, remainingRoom - 1);
                    cur.pop_back();
                }
            }
            else
            {
                // ok, cur now holds the next subset of parties. Lets give 
                // them all the next key.
                for (u64 i = 0; i < cur.size(); ++i)
                {
                    // the party Idx
                    auto p = cur[i];
                    // the next free key position for this party
                    auto jj = currentPartyKeyCount[p]++;

                    keyStructure(p, jj) = curKeyIdx;
                    subKeys(p, jj) = keys[curKeyIdx];
                }
                ++curKeyIdx;
            }
        };


        go(0, subsetSize);
    }

    // ASHISH TODO: In init initialize the sequence vector.
    void Npr03SymDprf::init(
        std::vector<float>& t,
        std::vector<std::string>& s,
        u64 partyIdx,
        u64 m,
        u64 n,
        span<Channel> requestChls,
        span<Channel> listenChls,
        block seed,
        oc::Matrix<u64>& keyStructure,
        span<block> keys)
    {

        mPartyIdx = partyIdx;
        mRequestChls = { requestChls.begin(), requestChls.end() };
        mListenChls = { listenChls.begin(), listenChls.end() };
        mPrng.SetSeed(seed);
        mIsClosed = false;

        //threshold (because trials was t :(, m is consistently threshold in code))
        mM = m;
        //
        mN = n;
         // ASHISH TODO: copy the sequence vector.
        
        times = &t;
        states = &s;

        //process state.txt file that was populated by uptime_server.py - TODO HARD-CODING FOR NOW
        processStateFile("state.txt", t, s);
        
        /* Disable this prints to see the times initialized with
        for (auto t:*times)
            std::cout<<t<<" ";
        */
        std::cout<<"\n";

        if(times->empty())
            std::cout<<"vector times is empty"<<std::endl;
        else if (states->empty())
            std::cout<<"vector states is empty"<<std::endl;

        //std::vector<std::string> mX = states;

        // Each subKey k_i will be distributed to subsetSize-out-of-n of the parties.
        auto subsetSize = mN - mM + 1;
        
        // the totak number of k_i
        mD = boost::math::binomial_coefficient<double>(mN, subsetSize);

        mDefaultKeys.resize(mN);
        for (u64 i = mPartyIdx, j = 0; j < mM; ++j)
        {
            constructDefaultKeys(i, keyStructure, keys);

            // i = i - 1 mod mN
            // mathematical mod where i can not be negative
            i = i ? (i - 1) : mN - 1;
        }

        //connect to the local python server on node0 which has the current status of nodes (up,down)
        // Creating socket file descriptor
        if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
            throw std::runtime_error(LOCATION);
	    }
        
        memset(&servaddr, 0, sizeof(servaddr));

        // Filling server information
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(PORT);
        servaddr.sin_addr.s_addr = inet_addr("10.0.0.254");

        total_count = 0;

        //Recording Abort Count
        send_Abort = 0;
        receive_queue_Abort = 0;
        receive_output_Abort = 0;

        //Filling in start time
        start_time = std::chrono::high_resolution_clock::now();
        std::time_t start_time_conv = std::chrono::system_clock::to_time_t(start_time);
        time(&start_time_conv);

        std::cout<<"Done initializing in"<<" "<<__func__<<std::endl;

        startListening();
    }

    void Npr03SymDprf::serveOne(span<u8> rr, u64 chlIdx)
    {
        TODO("Add support for allowing the request to specify which parties are involved in this evaluation. "
            "This can be done by sending a bit vector of the parties that contribute keys and then have this "
            "party figure out which keys to use in a similar way that constructDefaultKeys(...) does it.");

        // Right now we only support allowing 16 bytes to be the OPRF input.
        // When a multiple is sent, this its interpreted as requesting 
        // several OPRF evaluations.
        if(rr.size() % sizeof(block))
            throw std::runtime_error(LOCATION);

        // Get a view of the data as blocks.
        span<block> request( (block*)rr.data(), rr.size() / sizeof(block) );

        // a vector to hold the OPRF output shares.
        std::vector<oc::block> fx(request.size());

        // compute the partyIdx based on the channel idx.
        auto pIdx = chlIdx + (chlIdx >= mPartyIdx ? 1 : 0);

        if (request.size() == 1)
        {
            // If only one OPRF input is used, we vectorize the AES evaluation
            // so that several keys are evaluated in parallel.

            fx.resize(mDefaultKeys[pIdx].mAESs.size());
            mDefaultKeys[pIdx].ecbEncBlock(request[0], fx.data());

            for (u64 j = 1; j < fx.size(); ++j)
                fx[0] = fx[0] ^ fx[j];

            fx.resize(1);
        }
        else
        {
            // If several OPRF values are evaluated in parallel, then we apply a single key to several
            // OPRF inputs at a time. 

            auto numKeys = mDefaultKeys[pIdx].mAESs.size();
            std::vector<block> buff2(request.size());
            for (u64 i = 0; i < numKeys; ++i)
            {
                mDefaultKeys[pIdx].mAESs[i].ecbEncBlocks(request.data(), request.size(), buff2.data());
                for (u64 j = 0; j < request.size(); ++j)
                {
                    fx[j] = fx[j] ^ buff2[j];
                }
            }
        }

        // send back the OPRF output share.
        mListenChls[chlIdx].asyncSend(std::move(fx));
    }

    block Npr03SymDprf::eval(block input)
    {
        // simply call the async version and then block for it to complete
        block output = asyncEval(input).get()[0];
        //std::cout<<"Size of Output after asyncEval = "<<sizeof(output)<<std::endl;
        return output;
    }

    std::tuple<Channel, Channel> reconnectChannel(Channel& chl)
    {
        chl.cancel();
        chl.getSession().stop();
        osuCrypto::Session session;
        session.start(chl.getSession().getIOService(), "10.0.0.2", osuCrypto::SessionMode::Server);
        auto listenChl = session.addChannel("listen", "request");
        auto requestChl = session.addChannel("request", "listen");
        return {listenChl, requestChl};
    }

    void Npr03SymDprf::processStateFile(std::string filename, std::vector<float>& times, std::vector<std::string>& states){
        
        std::ifstream file_handle(filename);
        float number_read;
        std::string string_read;

        while(file_handle >> number_read >> string_read){
            times.push_back(number_read);
            states.push_back(string_read);
        }
    }

    void Npr03SymDprf::return_up_down_nodes(std::vector<int>& up_nodes, std::vector<int>& down_nodes, long double cur_time){

        std::string result{""};
        int i;
        /*std::cout<<"In"<<" "<<__func__<<"searching for"<<cur_time<<std::endl;
        std::cout<<"Size of time"<<times->size()<<std::endl;*/

        for (i = 0; i < (times->size()-1); i++){
                if(((*times)[i] <= cur_time) && ((*times)[i+1] > cur_time)){
                        //std::cout<<"Found the time slot"<<std::endl;
                        result.assign((*states)[i]);
                        if (i > 1)
                            times->erase(times->begin() + i - 1);
                        break;
                }
        }

        /*std::cout<<"Finished parsing times()"<<std::endl;*/

        if (result.empty()){
            //std::cout<<cur_time<<" "<<"is outside the limits of "<<(*times)[0]<<" and "<<(*times)[times->size()-1]<<std::endl;
            if (cur_time < (*times)[0])
                result.assign((*states)[0]);
            else if (cur_time > (*times)[times->size()-1])
                result.assign((*states)[times->size()-1]);
        }

        for (int i = 0; i != result.size(); i++){
            //std::cout<<"Node "<<i<<" is "<<result[i]<<std::endl;
            if(result[i] == 'u')
                    up_nodes.push_back(i);
            else if (result[i] == 'd')
                    down_nodes.push_back(i);
        }
    }

    /* std::string bucket(const std::vector<float>& times, const std::vector<std::string>& states, float cur_time){

        std::string result{""};
        int i;

        for (i = 0; i < (times.size()-1); i++){
                if((times[i] <= cur_time) && (times[i+1] > cur_time)){
                        result.assign(states[i]);
                        break;
                }
        }
        return result;
    }

    int Npr03SymDprf::calcBucket(std::vector<int>& up_nodes, std::vector<int>& down_nodes){
        std::ifstream file_name("state.txt");
        float number_read;
        std::string string_read;

        std::vector<float> times{};
        std::vector<std::string> states{};

        while(file_name >> number_read >> string_read){
            times.push_back(number_read);
            states.push_back(string_read);
        }

        auto result = bucket(times, states, 50.0);
            for (auto& i: up_nodes)
        for (int i = 0; i != result.size(); i++){
            //std::cout<<"Node "<<i<<" is "<<result[i]<<std::endl;
            if(result[i] == 'u')
                    up_nodes.push_back(i);
            else if (result[i] == 'd')
                    down_nodes.push_back(i);
        }
        return 0;
    } */

    void Npr03SymDprf::printStats(std::vector<long double>& v){

        long double sum = std::accumulate(v.begin(), v.end(), 0.0);
        long double mean = sum / v.size();

        std::vector<long double> diff(v.size());
        //std::transform(v.begin(), v.end(), diff.begin(),std::bind2nd(std::minus<double>(), mean));
        std::transform(v.begin(), v.end(), diff.begin(), [mean](long double x) { return x - mean; });
        long double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
        long double stdev = std::sqrt(sq_sum / v.size());

        std::cout<<"Mean:"<<mean<<"\t"<<"Std Deviation:"<<stdev<<std::endl;

    }

    void Npr03SymDprf::printAbortStats(){
        std::cout<<"============================="<<std::endl;
        std::cout<<"Abort at the NPR03SYMDPRF.CPP"<<std::endl;
        std::cout<<"============================="<<std::endl;
        std::cout<<"Total Count:"<<total_count<<" "
            <<"SendAbort:"<<send_Abort<<" "
            <<"Receive Queue Abort:"<<receive_queue_Abort<<" "
            <<"Receive Output Abort:"<<receive_output_Abort<<std::endl; 
    }


    void Npr03SymDprf::processTimes(){
        
        std::cout<<"---- OPRF-Send-Times\t";
        printStats(oprf_send_times);

        std::cout<<"---- OPRF-Receive-Times\t";
        printStats(oprf_receive_times);

        std::cout<<"---- OPRF-Combine-Times\t";
        printStats(oprf_combine_times);

    }

    AsyncEval Npr03SymDprf::asyncEval(block input) // TODO: fix
    {
        // ASHISH TODO: find the current live node

        //ASHISH TODO: exact opposite of the python script. 
        // prev_time, curr_time. Use that to find our bucket. 
        // up those nodes we query. -> vector of live node 

        //std::cout<<"In "<<__func__<<std::endl;
        //std::cout<<"Size of time"<<times->size()<<std::endl;

        auto cur_time = std::chrono::high_resolution_clock::now();
        std::time_t cur_time_conv = std::chrono::system_clock::to_time_t(cur_time);
        time(&cur_time_conv);

        long double time_delta_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(cur_time - start_time).count();
        long double NANOSECONDS_PER_SECOND = 1000000000;

        // std::ofstream time_file_handle("thing.txt");
        // time_file_handle << time_delta_ns;
        long double time_delta_s = time_delta_ns/NANOSECONDS_PER_SECOND;

        //std::cout<<"---- In "<<__func__<<" Time delta "<<time_delta_ns<<" "<<time_delta_s<<" "<<std::endl;

        TODO("Add support for sending the party identity for allowing encryption to be distinguished from decryption. ");
        // mM threshold
        // mN node count
        // mPartyIdx this node's ID

        std::vector<int> up_nodes, down_nodes;
        return_up_down_nodes(up_nodes, down_nodes, time_delta_s);

        /*std::cout<<"Up Nodes include "<<std::endl;
        for (auto& ch: up_nodes)
            std::cout<<ch<<" ";*/

        // partyidx of encryptor is 0. I query node 1 - node t-1.

        // Send the OPRF input to the next m-1 parties
        std::chrono::time_point<std::chrono::high_resolution_clock> oprf_send_start = std::chrono::high_resolution_clock::now();
        for (auto& i: up_nodes)
        /*auto end = mPartyIdx + mM;
        for (u64 i = mPartyIdx + 1; i < end; ++i)*/
        {
            auto c = i % mN;
            if (c > mPartyIdx) --c;
            try {
                mRequestChls[c].asyncSendCopy(&input, 1);
            } catch(const std::exception & e) {
                send_Abort++;
                std::cout << "Line 270" << std::endl;
            }
        }

        auto oprf_send_finish = std::chrono::high_resolution_clock::now();
        long double oprf_send_duration_in_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(oprf_send_finish - oprf_send_start).count();
        long double oprf_send_duration_in_s = oprf_send_duration_in_ns/NANOSECONDS_PER_SECOND;
        oprf_send_times.push_back(oprf_send_duration_in_s);
        //std::cout<<"---- In "<<"Send the OPRF input to the next m-1 parties"<<" Time delta "<<oprf_send_duration_in_ns<<" "<<oprf_send_duration_in_s<<" "<<std::endl;


        //std::cout<<"---- Done with sending the OPRF inputs"<<std::endl;

        // Set up the completion callback "AsyncEval".
        // This object holds a function that is called when 
        // the user wants to async eval to complete. This involves
        // receiving the OPRF output shares from the other parties
        // and combining them with the local share.
        AsyncEval ae;

        struct State
        {
            State(u64 m)
            :fx(m)
            ,async(m-1)
            {}
            std::vector<block> fx;
            std::vector<std::future<void>> async;

        };
        // allocate space to store the OPRF output shares
        auto w = std::make_shared<State>(mM);
        //std::cout<<"---- Space Allocated for Output Share = "<<sizeof(w)<<" "<<"Number of parties = "<<mM<<std::endl;

        // Futures which allow us to block until the repsonces have 
        // been received


        // Evaluate the local OPRF output share.
        std::vector<block> buff(mDefaultKeys[mPartyIdx].mAESs.size());
        mDefaultKeys[mPartyIdx].ecbEncBlock(input, buff.data());

        // store this share at the end of fx
        auto& b = w->fx.back();
        b = oc::ZeroBlock;
        for (u64 i = 0; i < buff.size(); ++i)
            b = b ^ buff[i];

        //std::cout<<"---- Going to queue up the receive operations to receive the OPRF output shares"<<std::endl;
        // queue up the receive operations to receive the OPRF output shares
        int j = 0;
        std::chrono::time_point<std::chrono::high_resolution_clock> oprf_receive_start = std::chrono::high_resolution_clock::now();
        for (auto& i: up_nodes){
            if (j < w->async.size()){
                auto c = i % mN;
                if (c > mPartyIdx) --c;
                try{
                    w->async[j] = mRequestChls[c].asyncRecv(&w->fx[j], 1);
                }
                catch(osuCrypto::BadReceiveBufferSize & e)                {
                    std::cout<<"Line 443"<<std::endl;
                    receive_queue_Abort++;
                    /*std::cout << "received only header from node " << c << std::endl;
                    // std::cout << "received: " << w->async[j].get() << std::endl;
                    std::cout << "trying to move on... " << i << std::endl;
                    //e.setBadRecvErrorState(osuCrypto::BadReceiveBufferSize.str());*/
                }
                j++;
            }
        }
        auto oprf_receive_finish = std::chrono::high_resolution_clock::now();
        long double oprf_receive_duration_in_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(oprf_receive_finish - oprf_receive_start).count();
        long double oprf_receive_duration_in_s = oprf_receive_duration_in_ns/NANOSECONDS_PER_SECOND;
        oprf_receive_times.push_back(oprf_receive_duration_in_s);
        //std::cout<<"---- In "<<"Going to queue up the receive operations to receive the OPRF output shares"<<" Time delta "<<oprf_receive_duration_in_ns<<" "<<oprf_receive_duration_in_s<<" "<<std::endl;

        
        /*for (u64 i = mPartyIdx + 1, j = 0; j < w->async.size(); ++i, ++j)
        {
            auto c = i % mN;
            if (c > mPartyIdx) --c;
            try
            {
                w->async[j] = mRequestChls[c].asyncRecv(&w->fx[j], 1);
            }
            catch(osuCrypto::BadReceiveBufferSize & e)
            {
                std::cout << "received only header from node " << c << std::endl;
                // std::cout << "received: " << w->async[j].get() << std::endl;
                std::cout << "trying to move on... " << i << std::endl;
                //e.setBadRecvErrorState(osuCrypto::BadReceiveBufferSize.str());
            }
        }*/

        //std::cout<<"---- Done queuing up the receive operations to receive the OPRF output shares"<<std::endl;
        
        // This function is called when the user wants the actual 
        // OPRF output. It must combine the OPRF output shares
        std::chrono::time_point<std::chrono::high_resolution_clock> oprf_combine_start = std::chrono::high_resolution_clock::now();
        ae.get = [this, w = std::move(w), up_nodes]() mutable ->std::vector<block>
        {
            // block until all of the OPRF output shares have arrived.
            for (u64 i = 0; i < mM - 1; ++i)
            {   
                total_count++;
                try {
                    w->async[i].get();
                } catch (std::exception & e){
                    receive_output_Abort++;
                    //std::cout <<count_abort++<<std::endl;
                    //std::cout << " Line 330" << std::endl;
                }
            }
            //std::cout<<"---- Got the OPRF output shares"<<std::endl;

            // XOR all of the output shares
            std::vector<block> ret{ w->fx[0] };
            for (u64 i = 1; i < mM; ++i)
                ret[0] = ret[0] ^ w->fx[i];

            //std::cout<<"---- Size of Output After XOR = "<<ret.size()<<" "<<"Size of block = "<<sizeof(block)<<std::endl;
            //std::cout<<"---- XOR the OPRF output shares done"<<std::endl;
            return (ret);
        };

        auto oprf_combine_finish = std::chrono::high_resolution_clock::now();
        long double oprf_combine_duration_in_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(oprf_combine_finish - oprf_combine_start).count();
        long double oprf_combine_duration_in_s = oprf_combine_duration_in_ns/NANOSECONDS_PER_SECOND;
        oprf_combine_times.push_back(oprf_combine_duration_in_s);
        //std::cout<<"---- In "<<"combine the OPRF output shares"<<" Time delta "<<oprf_combine_duration_in_ns<<" "<<oprf_combine_duration_in_s<<" "<<std::endl;

        return ae;
    }

    AsyncEval Npr03SymDprf::asyncEval(span<block> in)
    {
        struct State
        {
            std::vector<block> out, in, fxx;
            std::unique_ptr<std::future<void>[]> async;
        };
        auto state = std::make_shared<State>();

        // allocate space to store the OPRF outputs.
        state->out.resize(in.size());


        // Copy the inputs into a shared vector so that it 
        // can be sent to all parties using one allocation.
        state->in.insert(state->in.end(), in.begin(), in.end());

        // send this input to all parties
        auto end = mPartyIdx + mM;
        for (u64 i = mPartyIdx + 1; i < end; ++i)
        {
            auto c = i % mN;
            if (c > mPartyIdx) --c;

            // This send is smart and will increment the ref count of
            // the shared pointer
            mRequestChls[c].asyncSend(state->in);
        }

        auto numKeys = mDefaultKeys[mPartyIdx].mAESs.size();

        // evaluate the local OPRF output shares
        std::vector<block> buff2(in.size());
        for (u64 i = 0; i < numKeys; ++i)
        {
            mDefaultKeys[mPartyIdx].mAESs[i].ecbEncBlocks(in.data(), in.size(), buff2.data());
            auto& out = state->out;
            for (u64 j = 0; j < out.size(); ++j)
            {
                out[j] = out[j] ^ buff2[j];
            }
        }

        // allocate space to store the other OPRF output shares
        auto numRecv = (mM - 1);
        state->fxx.resize(numRecv* in.size());
        //auto fxx(new block[numRecv * in.size()]);

        // Each row of fx will hold a the OPRF output shares from one party
        oc::MatrixView<block> fx(state->fxx.begin(), state->fxx.end(), in.size());

        // allocate space to store the futures which allow us to block until the
        // other OPRF output shares have arrived.
        state->async.reset(new std::future<void>[numRecv]);

        // schedule the receive operations for the other OPRF output shares.
        for (u64 i = mPartyIdx + 1, j = 0; j < numRecv; ++i, ++j)
        {
            auto c = i % mN;
            if (c > mPartyIdx) --c;

               state->async[j] = mRequestChls[c].asyncRecv(fx[j]);
        }

        // construct the completion handler that is called when the user wants to 
        // actual OPRF output. This requires blocking to receive the OPRF output
        // and then combining it.
        AsyncEval ae;
        ae.get = [state, numRecv, fx]() mutable -> std::vector<block>
        {
            auto& o = state->out;
            for (u64 i = 0; i < numRecv; ++i)
            {
                state->async[i].get();

                auto buff2 = fx[i];
                for (u64 j = 0; j < o.size(); ++j)
                {
                    o[j] = o[j] ^ buff2[j];
                }
            }
            return std::move(o);
        };

        return ae;
    }

    void Npr03SymDprf::startListening()
    {

        mRecvBuff.resize(mRequestChls.size());
        mListens = mListenChls.size();
        mServerListenCallbacks.resize(mListenChls.size());


        for (u64 i = 0; i < mListenChls.size(); ++i)
        {
            mServerListenCallbacks[i] = [&, i]()
            {
/*                std::cout << "Received buffer from " << i << " : 0x";
                for (auto v: mRecvBuff)
                    for (auto c: v)
                        std::cout << std::hex << (int)c;
                std::cout << std::endl;
*/
                // If the client sends more than one byte, interpret this
                // as a request to evaluate the DPRF.
                if (mRecvBuff[i].size() > 1)
                {
                    // Evaluate the DPRF and send the result back.
                    serveOne(mRecvBuff[i], i);

                    // Eueue up another receive operation which will call 
                    // this callback when the request arrives.
                       mListenChls[i].asyncRecv(mRecvBuff[i], mServerListenCallbacks[i]);
                }
                else
                {
                    // One byte means that the cleint is done requiresting 
                    // DPRf evaluations. We can close down.
                    if (--mListens == 0)
                    {
                        // If this is the last callback to close, set
                        // the promise that denotes that the server
                        // callback loops have all completed.
                        mServerDoneProm.set_value();
                    }
                }
            };

               mListenChls[i].asyncRecv(mRecvBuff[i], mServerListenCallbacks[i]);
        }
    }

    void Npr03SymDprf::constructDefaultKeys(u64 pIdx, oc::Matrix<u64>& mKeyIdxs, span<block> myKeys)
    {
        // Default keys are computed taking all the keys that party pIdx has
        // followed by all the missing keys party pIdx+1 has and so on.

        // A list indicating which keys have already been accounted for.
        std::vector<u8> keyList(mD, 0);
        auto p = pIdx;

        // First lets figure out what keys are been provided 
        // by parties {pIdx, pIdx+1, ..., mPartyIdx-1} 
        while (p != mPartyIdx)
        {
            for (u64 i = 0; i < myKeys.size(); ++i)
                keyList[mKeyIdxs(p, i)] = 1;

            p = (p + 1) % mN;
        }

        // Now lets see if any remaining keys that this party can contribute.
        std::vector<block> keys; keys.reserve(myKeys.size());
        for (u64 j = 0; j < myKeys.size(); ++j)
        {
            if (keyList[mKeyIdxs(mPartyIdx, j)] == 0)
                keys.push_back(myKeys[j]);
        }

        // initialize the "multi-key" AES instance with these keys.
        mDefaultKeys[pIdx].setKeys(keys);
    }

    void Npr03SymDprf::close()
    {
        if (mIsClosed == false)
        {
            mIsClosed = true;

            u8 close[1];
            close[0] = 0;

            // closing the channel is done by sending a single byte.
            for (auto& c : mRequestChls)
                c.asyncSendCopy(close, 1);

        }
    }

}
