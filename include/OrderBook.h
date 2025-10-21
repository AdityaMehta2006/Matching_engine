#pragma once
#include "Order.h"
#include <map>
#include <list>
#include <deque>
#include <unordered_map>
#include <optional>
#include <vector>
#include "Types.h"

struct PriceLevel {
    double price;
    std::list<Order> orders; // FIFO, stable iterators for removal
};

class OrderBook {
public:
    void add_limit(const Order& o);
    // remove an order by id (returns true if removed)
    bool remove_order(const std::string& order_id);
    std::optional<std::pair<double,uint64_t>> best_bid() const;
    std::optional<std::pair<double,uint64_t>> best_ask() const;
    // compute available quantity on the ask side up to (and including) price_limit
    uint64_t available_qty_for_buy(double price_limit) const;
    // compute available quantity on the bid side down to (and including) price_limit
    uint64_t available_qty_for_sell(double price_limit) const;
    // match an incoming buy order against asks; modifies incoming.quantity and produces trades
    void match_buy(Order& incoming, std::vector<Trade>& trades);
    // match an incoming sell order against bids
    void match_sell(Order& incoming, std::vector<Trade>& trades);

private:
    std::map<double, PriceLevel, std::greater<double>> bids; // high->low
    std::map<double, PriceLevel> asks; // low->high
    // map order_id -> (price, side, iterator)
    struct OrderLocation { double price; Side side; std::list<Order>::iterator it; };
    std::unordered_map<std::string, OrderLocation> order_map;
};
