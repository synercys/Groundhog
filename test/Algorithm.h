#pragma once

#include <dEnc/Defines.h>
#include <bits/stdc++.h>
#include "RandomNodePicker.h"
#include "util.h"

using namespace osuCrypto;

u64 restoreNumRebootsFromFile(std::string stateFileName) {
    // return number of reboots to restore state of nodePicker
    u64 numRebootsSoFar = 0;
    std::ifstream stateFile(stateFileName);
    if (stateFile.good()) { // file exists
        std::string numRebootsStr;
        stateFile >> numRebootsStr;
        numRebootsSoFar = std::stoi(numRebootsStr);
        stateFile.close();
    }

    return numRebootsSoFar;
}

void rebootAfterTime(u64 timeToReboot, std::string stateFileName, u64 numRebootsSoFar) {
    std::chrono::milliseconds timespan(timeToReboot);
    std::this_thread::sleep_for(timespan);

    numRebootsSoFar++;
    std::ofstream stateFile(stateFileName);
    stateFile << numRebootsSoFar;
    stateFile.close();
    
    std::cout << "REBOOT " << numRebootsSoFar << std::endl;
}


void algorithm(std::vector<std::string> ips, u64 n, u64 attackTime, u64 rebootTime, u64 t, RandomNodePicker& nodePicker) {
    u64 mIntervals = std::max((u64)1, (u64)std::floor(attackTime/rebootTime));

    std::string ip = getIP();
    u64 currNodeIdx = getCurrNodeIdx(ips, ip);
    currNodeIdx = 1;
    // std::cout << "currNodeIdx: " << currNodeIdx << std::endl;
    
    std::string stateFileName = "reboot_state";
    u64 numRebootsSoFar = restoreNumRebootsFromFile(stateFileName);

    if (t+1 < mIntervals) {

    } else if ((t+1) % mIntervals == 0) {
        // TODO: Check if it works when n is odd
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
        rebootAfterTime(timeToReboot, stateFileName, numRebootsSoFar);
    } else { // t+1 > m and not a multiple of m

    }
}