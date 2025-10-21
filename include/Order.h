#pragma once
#include <string>
#include <optional>
#include <chrono>

enum class Side { BUY, SELL };
enum class OrderType { LIMIT, MARKET, IOC, FOK };

struct Order {
    std::string order_id;
    std::string symbol;
    Side side;
    OrderType type;
    uint64_t quantity;
    std::optional<double> price; // for limit orders
    std::chrono::steady_clock::time_point ts;

    Order(std::string id, const std::string& sym, Side s, OrderType t, uint64_t q, std::optional<double> p = std::nullopt);
};

#if defined(__has_include)
# if __has_include(<nlohmann/json.hpp>)
#  include <nlohmann/json.hpp>
inline nlohmann::json to_json(const Order& o){
    nlohmann::json j;
    j["order_id"] = o.order_id;
    j["symbol"] = o.symbol;
    j["side"] = (o.side == Side::BUY ? "BUY" : "SELL");
    j["type"] = (o.type == OrderType::LIMIT ? "LIMIT" : "OTHER");
    j["quantity"] = o.quantity;
    if(o.price) j["price"] = *o.price;
    return j;
}
# endif
#endif
