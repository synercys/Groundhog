#pragma once

#include <dEnc/Defines.h>
#include <bits/stdc++.h>
#include "RandomNodePicker.h"
#include "util.h"

using namespace osuCrypto;


class Algorithm {
    private:
        u64 n, attackTime, rebootTime, t, currNodeIdx, mIntervals, numRebootsSoFar;
        RandomNodePicker& nodePicker;
        const std::vector<std::string>& ips;
        std::string ip, stateFileName;

        void restoreNumRebootsFromFile() {
            // restore number of reboots to restore state of nodePicker
            numRebootsSoFar = 0;
            std::ifstream stateFile(stateFileName);
            if (stateFile.good()) { // file exists
                std::string numRebootsStr;
                stateFile >> numRebootsStr;
                numRebootsSoFar = std::stoi(numRebootsStr);
                stateFile.close();
            }
        }

        void rebootAfterTime(u64 timeToReboot) {
            numRebootsSoFar++;
            std::ofstream stateFile(stateFileName);
            stateFile << numRebootsSoFar;
            stateFile.close();

            std::chrono::milliseconds timespan(timeToReboot);
            std::this_thread::sleep_for(timespan);

            
            std::cout << "REBOOT " << numRebootsSoFar << std::endl;
        }

    public:
        Algorithm(const std::vector<std::string>& _ips, u64 _n, u64 _attackTime, u64 _rebootTime, u64 _t, RandomNodePicker& _nodePicker, 
                  std::string _stateFileName) : n(_n), attackTime(_attackTime), rebootTime(_rebootTime), t(_t), nodePicker(_nodePicker),
                  ips(_ips), stateFileName(_stateFileName) {
            ip = getIP();
            mIntervals = std::max((u64)1, (u64)std::floor(attackTime/rebootTime));
            currNodeIdx = getCurrNodeIdx(ips, ip);
            restoreNumRebootsFromFile();
            std::cout << "Current_node_idx:" << currNodeIdx << std::endl;
            
        }

        void run() {
            if (t+1 < mIntervals) {
                
                // we will have an idle time window. 
                int subsetSize = (t+1);
                u64 idle_time = (mIntervals - (t+1) ) * rebootTime;
                int count = 0;
                if(numRebootsSoFar > 0 )    count = (numRebootsSoFar-1) * n;
                u64 timeToReboot = 0;

                // restore nodePicker's state before reboot happened
                if (numRebootsSoFar > 0) {
                    nodePicker.currGeneratorIdx = (numRebootsSoFar-1) % nodePicker.generators.size();
                    nodePicker.nextGeneratedNumIdx = 0;
                    while (nodePicker.nextNode() != currNodeIdx)
                    {
                        ++count;
                    }
                }

                std::cout << "Generator's state: (" << nodePicker.currGeneratorIdx << ", " << nodePicker.nextGeneratedNumIdx << ")" << std::endl;
                std::cout << "count: " << count << std::endl;
                count = count % subsetSize;
                if(count > 0)
                {
                    timeToReboot = idle_time;
                }
                int nodesToWait = 0; // nodes to wait for before rebooting self
                while (nodePicker.nextNode() != currNodeIdx)    nodesToWait++;

                std::cout << "nodesToWait: " << nodesToWait << std::endl;
                count = nodesToWait - count;
                while((count - subsetSize)>=0)
                {
                    std::cout << "count: " << count << " time_to_reboot:" << timeToReboot << std::endl;
                    count = count - subsetSize;
                    timeToReboot = timeToReboot + idle_time; 
                }
                timeToReboot = timeToReboot + (nodesToWait * rebootTime);
                std::cout << "timeToReboot: " << timeToReboot << "ms" << std::endl;
                rebootAfterTime(timeToReboot);
               
                // TODO: NOT IMPLEMENTED!
            } else if ((t+1) % mIntervals == 0) {
                // TODO: Does not work correctly if n is not a multiple of subsetSize
                // two generators used = 1,2,3,4,5,6,7
                u64 subsetSize = (t+1)/mIntervals; // number of nodes rebooting in an interval

                // restore nodePicker's state before reboot happened
                if (numRebootsSoFar > 0) {
                    nodePicker.currGeneratorIdx = (numRebootsSoFar-1) % nodePicker.generators.size();
                    nodePicker.nextGeneratedNumIdx = 0;
                    while (nodePicker.nextNode() != currNodeIdx)    continue;
                    
                    // move counter for other rebooted nodes that interval
                    while (nodePicker.nextGeneratedNumIdx % subsetSize != 0)    nodePicker.nextNode();
                }

                std::cout << "Generator's state: (" << nodePicker.currGeneratorIdx << ", " << nodePicker.nextGeneratedNumIdx << ")" << std::endl;

                u64 nodesToWait = 0; // nodes to wait for before rebooting self
                while (nodePicker.nextNode() != currNodeIdx)    nodesToWait++;

                std::cout << "nodesToWait: " << nodesToWait << std::endl;
                u64 timeToReboot = (nodesToWait/subsetSize) * rebootTime;
                std::cout << "timeToReboot: " << timeToReboot << "ms" << std::endl;
                rebootAfterTime(timeToReboot);
            } else { // t+1 > m and not a multiple of m
                // TODO: NOT IMPLEMENTED!
            }
        }
};