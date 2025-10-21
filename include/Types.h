#pragma once
#include <string>

struct Trade {
    std::string buy_order_id;
    std::string sell_order_id;
    double price;
    uint64_t quantity;
};
