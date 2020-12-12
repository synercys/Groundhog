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

u64 powerModP(u64 x, u64 y, u64 p) {
    // return x^y mod p

    u64 res = 1; // Initialize result  
    x = x % p; // Update x if it is >= p 
    if (x == 0) return 0; // In case x is divisible by p; 
  
    while (y > 0) {
        // If y is odd, multiply x with result
        if (y & 1)
            res = (res*x) % p;
        
        // y must be even now
        y = y>>1; // y = y/2
        x = (x*x) % p;
    }
    return res;
}  

bool isGenerator(u64 i, u64 n, u64 prime) {
    // return true if i^x % prime generates all numbers from 0 to n-1 for different x values

    std::set<u64> generatedNums;
    u64 powersOfiModPrime = 1;
    for (u64 x = 0; x < prime; x++) {
        generatedNums.insert(powersOfiModPrime);
        powersOfiModPrime = (powersOfiModPrime * i) % prime; // ab mod p = [(a mod p) (b mod p)] mod p
    }

    for (u64 x = 1; x <= n; x++)
        if (generatedNums.find(x) == generatedNums.end()) // element not found in set
            return false;
    return true;
}

u64 rangeRand(u64 range_from, u64 range_to) {
    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());
    std::uniform_int_distribution<u64> distr(range_from, range_to);
    return distr(generator);
}


class RandomNodePicker {
    public:
        u64 n, prime;
        std::vector<u64> generators;

        RandomNodePicker(u64 n) {
            this->n = n;
            prime = findNextPrime(n);
            for (u64 i = 1; i <= n; i++)
                if (isGenerator(i, n, prime))
                    generators.push_back(i);
        }

        u64 nextNode() {
            // randomly pick one of the generator numbers to generate a number which will be between 1 and n
            u64 generatorNum = generators[rangeRand(0, generators.size()-1)];
            u64 pickedNode = powerModP(generatorNum, rangeRand(0, n-1), prime);
            return pickedNode >= 1 && pickedNode <= n ? pickedNode : nextNode();
        }
};