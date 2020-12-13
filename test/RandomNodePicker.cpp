#include <dEnc/Defines.h>
#include <bits/stdc++.h>

using namespace osuCrypto;


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

void findGeneratedNums(u64 i, u64 n, u64 prime, std::vector<u64>& generatedNums) {
    /* set generatedNums vector of size n of generated nums if i^x % prime generates all numbers from 1 to n 
       for different x values else clear the vector */

    std::set<u64> generatedNumsSet;
    u64 powersOfiModPrime = 1;
    for (u64 x = 0; x < prime; x++) {
        if (generatedNumsSet.find(powersOfiModPrime) == generatedNumsSet.end()) { // element not found in set
            generatedNumsSet.insert(powersOfiModPrime);
            if (powersOfiModPrime >= 1 && powersOfiModPrime <= n) {
                generatedNums.push_back(powersOfiModPrime);
            }
        }
        powersOfiModPrime = (powersOfiModPrime * i) % prime; // ab mod p = [(a mod p) (b mod p)] mod p
    }
}

u64 rangeRand(u64 range_from, u64 range_to) {
    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());
    std::uniform_int_distribution<u64> distr(range_from, range_to);
    return distr(generator);
}


class RandomNodePicker {
    public:
        u64 n, prime, currGeneratorIdx, nextGeneratedNumIdx;
        std::vector<std::pair<u64, std::vector<u64>>> generators;

        RandomNodePicker(u64 n) {
            this->n = n;
            prime = findNextPrime(n);
            for (u64 i = 1; i <= n; i++) {
                std::vector<u64> generatedNums;
                findGeneratedNums(i, n, prime, generatedNums);
                if (generatedNums.size() == n)
                    generators.push_back(std::make_pair(i, generatedNums));
            }

            // randomize order of generators used
            srand(time(NULL));
            std::random_shuffle(generators.begin(), generators.end());

            currGeneratorIdx = 0;
            nextGeneratedNumIdx = 0;
        }

        u64 nextNode() {
            // pick a node from 1 to n to be rebooted next
            u64 nodeNum = generators[currGeneratorIdx].second[nextGeneratedNumIdx++];
            if (nextGeneratedNumIdx == n) {
                nextGeneratedNumIdx = 0;
                currGeneratorIdx = (currGeneratorIdx + 1) % generators.size();
            }
            return nodeNum;
        }
};