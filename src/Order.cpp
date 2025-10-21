#include "Order.h"
#include <atomic>

#ifdef __has_include
# if __has_include(<boost/uuid/uuid.hpp>)
#  include <boost/uuid/uuid.hpp>
#  include <boost/uuid/uuid_generators.hpp>
#  include <boost/uuid/uuid_io.hpp>
#  define HAVE_BOOST_UUID 1
# endif
# if __has_include(<nlohmann/json.hpp>)
#  include <nlohmann/json.hpp>
#  define HAVE_NLOHMANN_JSON 1
# endif
#endif

static std::atomic<uint64_t> global_id{1};

Order::Order(std::string id, const std::string& sym, Side s, OrderType t, uint64_t q, std::optional<double> p)
    : order_id([&]{
#if defined(HAVE_BOOST_UUID)
        if(id.empty()){
            boost::uuids::random_generator gen; 
            return boost::uuids::to_string(gen());
        }
#endif
        if(id.empty()) return std::to_string(global_id.fetch_add(1));
        return id;
    }())
    , symbol(sym)
    , side(s)
    , type(t)
    , quantity(q)
    , price(p)
    , ts(std::chrono::steady_clock::now())
{}

// (no-op) helper moved to header when nlohmann/json is available
