
#include <vector>
#include <cmath>
#include <iostream>

using namespace std;

vector<int> sieve(int n) {
    vector<bool> isPrime(n + 1, true);
    isPrime[0] = false;
    isPrime[1] = false;
    
    for (int p = 2; p <= sqrt(n); ++p) {
        if (isPrime[p]) {
            for (int i = p * p; i <= n; i += p) {
                isPrime[i] = false;
            }
        }
    }
    
    vector<int> primes;
    for (int i = 2; i <= n; ++i) {
        if (isPrime[i]) {
            primes.push_back(i);
        }
    }
    return primes;
}

int main() {
    vector<int> primes = sieve(1000);
    for (int prime : primes) {
        cout << prime << "\t";
    }
    return 0;
}