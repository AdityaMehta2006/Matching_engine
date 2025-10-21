#include "MatchingEngine.h"
#include "Order.h"
#include <iostream>

int main() {
    MatchingEngine me;
    // simple hard-coded test run
    Order o1("", "BTCUSD", Side::BUY, OrderType::LIMIT, 100, 50000.0);
    auto trades = me.submit_order(o1);
    std::cout << "Submitted order, trades: " << trades.size() << "\n";
    return 0;
}
