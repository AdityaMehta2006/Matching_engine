#include "OrderBook.h"

void OrderBook::add_limit(const Order& o) {
    double p = o.price.value();
    if (o.side == Side::BUY) {
        auto it = bids.find(p);
        if (it == bids.end()) {
            PriceLevel pl; pl.price = p; pl.orders.push_back(o);
            auto [newit, inserted] = bids.emplace(p, std::move(pl));
            it = newit;
        } else {
            it->second.orders.push_back(o);
        }
        auto lit = std::prev(it->second.orders.end());
        order_map.emplace(it->second.orders.back().order_id, OrderLocation{p, o.side, lit});
    } else {
        auto it = asks.find(p);
        if (it == asks.end()) {
            PriceLevel pl; pl.price = p; pl.orders.push_back(o);
            auto [newit, inserted] = asks.emplace(p, std::move(pl));
            it = newit;
        } else {
            it->second.orders.push_back(o);
        }
        auto lit = std::prev(it->second.orders.end());
        order_map.emplace(it->second.orders.back().order_id, OrderLocation{p, o.side, lit});
    }
}

std::optional<std::pair<double,uint64_t>> OrderBook::best_bid() const {
    if (bids.empty()) return std::nullopt;
    const auto &pl = bids.begin()->second;
    uint64_t size = 0;
    for (const auto &o : pl.orders) size += o.quantity;
    return std::make_pair(pl.price, size);
}

std::optional<std::pair<double,uint64_t>> OrderBook::best_ask() const {
    if (asks.empty()) return std::nullopt;
    const auto &pl = asks.begin()->second;
    uint64_t size = 0;
    for (const auto &o : pl.orders) size += o.quantity;
    return std::make_pair(pl.price, size);
}

bool OrderBook::remove_order(const std::string& order_id){
    auto it = order_map.find(order_id);
    if(it==order_map.end()) return false;
    auto loc = it->second;
    if(loc.side==Side::BUY){
        auto pit = bids.find(loc.price);
        if(pit!=bids.end()){
            pit->second.orders.erase(loc.it);
            if(pit->second.orders.empty()) bids.erase(pit);
        }
    } else {
        auto pit = asks.find(loc.price);
        if(pit!=asks.end()){
            pit->second.orders.erase(loc.it);
            if(pit->second.orders.empty()) asks.erase(pit);
        }
    }
    order_map.erase(it);
    return true;
}

uint64_t OrderBook::available_qty_for_buy(double price_limit) const{
    uint64_t sum=0;
    for(const auto &kv : asks){
        if(kv.first>price_limit) break;
        for(const auto &o: kv.second.orders) sum += o.quantity;
    }
    return sum;
}

uint64_t OrderBook::available_qty_for_sell(double price_limit) const{
    uint64_t sum=0;
    for(const auto &kv : bids){
        if(kv.first<price_limit) break;
        for(const auto &o: kv.second.orders) sum += o.quantity;
    }
    return sum;
}

void OrderBook::match_buy(Order& incoming, std::vector<Trade>& trades){
    // iterate asks from lowest price upward
    for(auto pit = asks.begin(); pit!=asks.end() && incoming.quantity>0; ){ 
        double p = pit->first;
        auto &pl = pit->second;
        while(!pl.orders.empty() && incoming.quantity>0){
            Order &rest = pl.orders.front();
            uint64_t exec = std::min(incoming.quantity, rest.quantity);
            trades.push_back(Trade{incoming.order_id, rest.order_id, p, exec});
            incoming.quantity -= exec;
            rest.quantity -= exec;
            if(rest.quantity==0){
                order_map.erase(rest.order_id);
                pl.orders.pop_front();
            }
        }
        if(pl.orders.empty()) pit = asks.erase(pit); else ++pit;
    }
}

void OrderBook::match_sell(Order& incoming, std::vector<Trade>& trades){
    // iterate bids from highest price downward
    for(auto pit = bids.begin(); pit!=bids.end() && incoming.quantity>0; ){
        double p = pit->first;
        auto &pl = pit->second;
        while(!pl.orders.empty() && incoming.quantity>0){
            Order &rest = pl.orders.front();
            uint64_t exec = std::min(incoming.quantity, rest.quantity);
            trades.push_back(Trade{rest.order_id, incoming.order_id, p, exec});
            incoming.quantity -= exec;
            rest.quantity -= exec;
            if(rest.quantity==0){
                order_map.erase(rest.order_id);
                pl.orders.pop_front();
            }
        }
        if(pl.orders.empty()) pit = bids.erase(pit); else ++pit;
    }
}
