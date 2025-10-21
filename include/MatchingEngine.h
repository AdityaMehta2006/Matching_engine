#pragma once
#include "Order.h"
#include "OrderBook.h"
#include "Types.h"
#include <vector>
#include <functional>

class MatchingEngine {
public:
    using TradeCallback = std::function<void(const Trade&)>;
    using BBO = std::pair<std::optional<std::pair<double,uint64_t>>, std::optional<std::pair<double,uint64_t>>>; // (best_bid, best_ask)
    using BBOCallback = std::function<void(const BBO&)>;
    MatchingEngine(TradeCallback cb = nullptr, BBOCallback bbo_cb = nullptr);
    std::vector<Trade> submit_order(const Order& o);
    bool cancel_order(const std::string& order_id);
private:
    TradeCallback on_trade;
    BBOCallback on_bbo;
    BBO last_bbo;
    OrderBook ob;
};
