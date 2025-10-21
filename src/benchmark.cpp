#include "MatchingEngine.h"
#include <chrono>
#include <iostream>

int main(){
    using namespace std::chrono;
    MatchingEngine engine([](const Trade&t){ /*noop*/ }, [](const MatchingEngine::BBO& bbo){ /*noop*/ });
    const int N = 100000; // number of orders
    auto t0 = high_resolution_clock::now();
    for(int i=0;i<N;i++){
        Order o("", "SYM", (i%2==0?Side::BUY:Side::SELL), OrderType::LIMIT, 1, 100.0 + (i%10));
        engine.submit_order(o);
    }
    auto t1 = high_resolution_clock::now();
    double secs = duration<double>(t1-t0).count();
    std::cout << "Submitted "<<N<<" orders in "<<secs<<"s ("<<(N/secs)<<" ops/s)\n";
    return 0;
}
