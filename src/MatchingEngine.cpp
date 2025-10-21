#include "MatchingEngine.h"
#include <algorithm>

MatchingEngine::MatchingEngine(TradeCallback cb, BBOCallback bbo_cb) : on_trade(cb), on_bbo(bbo_cb) {}

std::vector<Trade> MatchingEngine::submit_order(const Order& o) {
    std::vector<Trade> trades;
    // copy incoming so we can modify quantity during matching
    Order in = o;
    if(in.type==OrderType::MARKET){
        if(in.side==Side::BUY) ob.match_buy(in, trades);
        else ob.match_sell(in, trades);
        // emit trades
        for(const auto &t: trades) if(on_trade) on_trade(t);
        // emit BBO if changed
        auto b = std::make_pair(ob.best_bid(), ob.best_ask());
        if(on_bbo && b != last_bbo){ on_bbo(b); last_bbo = b; }
        return trades;
    }

    if(in.type==OrderType::LIMIT){
        if(in.side==Side::BUY){
            // match while ask prices <= limit price
            if(in.price){
                // restrict matching to asks with price <= limit
                ob.match_buy(in, trades);
            }
            if(in.quantity>0) ob.add_limit(in);
            for(const auto &t: trades) if(on_trade) on_trade(t);
            auto b2 = std::make_pair(ob.best_bid(), ob.best_ask());
            if(on_bbo && b2 != last_bbo){ on_bbo(b2); last_bbo = b2; }
            return trades;
        } else {
            if(in.price){
                ob.match_sell(in, trades);
            }
            if(in.quantity>0) ob.add_limit(in);
            for(const auto &t: trades) if(on_trade) on_trade(t);
            auto b3 = std::make_pair(ob.best_bid(), ob.best_ask());
            if(on_bbo && b3 != last_bbo){ on_bbo(b3); last_bbo = b3; }
            return trades;
        }
    }

    if(in.type==OrderType::IOC){
        if(in.side==Side::BUY) ob.match_buy(in, trades);
        else ob.match_sell(in, trades);
        for(const auto &t: trades) if(on_trade) on_trade(t);
        auto b4 = std::make_pair(ob.best_bid(), ob.best_ask());
        if(on_bbo && b4 != last_bbo){ on_bbo(b4); last_bbo = b4; }
        return trades;
    }

    if(in.type==OrderType::FOK){
        // simulate availability
        bool can_fill=false;
        if(in.side==Side::BUY){
            double limit = in.price.value_or(std::numeric_limits<double>::infinity());
            uint64_t avail = ob.available_qty_for_buy(limit);
            can_fill = (avail >= in.quantity);
        } else {
            double limit = in.price.value_or(0.0);
            uint64_t avail = ob.available_qty_for_sell(limit);
            can_fill = (avail >= in.quantity);
        }
        if(can_fill){
            if(in.side==Side::BUY) ob.match_buy(in, trades);
            else ob.match_sell(in, trades);
            for(const auto &t: trades) if(on_trade) on_trade(t);
            auto b5 = std::make_pair(ob.best_bid(), ob.best_ask());
            if(on_bbo && b5 != last_bbo){ on_bbo(b5); last_bbo = b5; }
        }
        return trades;
    }

    return trades;
}

bool MatchingEngine::cancel_order(const std::string& order_id){
    return ob.remove_order(order_id);
}
