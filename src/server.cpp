#include <iostream>
#include <mutex>
#include <vector>
#include <chrono>
#include <thread>
#include <sstream>
#include <queue>
#include <atomic>

#include "MatchingEngine.h"
#include "Order.h"

#include "httplib.h" // downloaded at configure-time into binary dir include
#include "ws_broadcaster.h"

#if defined(__has_include)
# if __has_include(<nlohmann/json.hpp>)
#  include <nlohmann/json.hpp>
#  define HAVE_JSON 1
# endif
#endif

#if !defined(HAVE_JSON)
#error "nlohmann/json.hpp is required to build me_server. Please install nlohmann_json and reconfigure CMake."
#else
using json = nlohmann::json;
#endif

int main(){
    std::vector<Trade> trades;
    std::mutex trades_m;
    std::optional<std::pair<double,uint64_t>> last_bid, last_ask;
    std::mutex bbo_m;

    std::queue<Order> order_queue;
    std::mutex queue_m;
    std::condition_variable queue_cv;
    std::atomic<bool> stop{false};

    // event queue for long-polling clients (trade/bbo)
    struct Event { std::string type; json payload; };
    std::deque<Event> events;
    std::mutex events_m;
    std::condition_variable events_cv;

    // create callbacks separately to avoid overly-complex inline lambda parsing by MSVC
    WSBroadcaster ws;

    MatchingEngine::TradeCallback trade_cb = [&](const Trade &t){
        {
            std::lock_guard<std::mutex> lk(trades_m); trades.push_back(t);
        }
        {
            std::lock_guard<std::mutex> el(events_m);
            events.push_back(Event{"trade", json{{"buy_order_id", t.buy_order_id},{"sell_order_id", t.sell_order_id},{"quantity", t.quantity},{"price", t.price}}});
        }
        events_cv.notify_all();
        try{ ws.broadcast(json{{"type","trade"},{"payload", json{{"buy_order_id", t.buy_order_id},{"sell_order_id", t.sell_order_id},{"quantity", t.quantity},{"price", t.price}}}}.dump()); } catch(...){}
    };

    MatchingEngine::BBOCallback bbo_cb = [&](const MatchingEngine::BBO &bbo){
        {
            std::lock_guard<std::mutex> lk(bbo_m); last_bid = bbo.first; last_ask = bbo.second;
        }
        {
            std::lock_guard<std::mutex> el(events_m);
            json j;
            if(bbo.first) j["bid"] = json{{"price", bbo.first->first},{"size", bbo.first->second}}; else j["bid"] = nullptr;
            if(bbo.second) j["ask"] = json{{"price", bbo.second->first},{"size", bbo.second->second}}; else j["ask"] = nullptr;
            events.push_back(Event{"bbo", j});
        }
        events_cv.notify_all();
        try{ ws.broadcast(json{{"type","bbo"},{"payload", json{ {"bid", (bbo.first? json{{"price", bbo.first->first},{"size", bbo.first->second}}:json(nullptr))}, {"ask", (bbo.second? json{{"price", bbo.second->first},{"size", bbo.second->second}}:json(nullptr))} }}}.dump()); } catch(...){}
    };

    MatchingEngine engine(trade_cb, bbo_cb);

    // worker thread to process orders from queue
    std::thread worker([&]{
        while(true){
            std::unique_lock<std::mutex> lk(queue_m);
            queue_cv.wait(lk, [&]{ return !order_queue.empty() || stop.load(); });
            if(stop.load() && order_queue.empty()) break;
            Order o = std::move(order_queue.front()); order_queue.pop();
            lk.unlock();
            engine.submit_order(o);
        }
    });

    httplib::Server svr;

    // Always expect JSON input/return JSON output. If nlohmann is not available,
    // compilation will fail; CMake links nlohmann when present. We prefer JSON.
    svr.Post("/order", [&](const httplib::Request& req, httplib::Response& res){
        try{
#if HAVE_JSON
            auto j = json::parse(req.body);
            std::string side = j.at("side").get<std::string>();
            uint64_t qty = j.at("quantity").get<uint64_t>();
            double price = j.value("price", 0.0);
            std::string symbol = j.at("symbol").get<std::string>();
            Side s = (side=="BUY"?Side::BUY:Side::SELL);
            Order o("", symbol, s, OrderType::LIMIT, qty, price>0.0?std::optional<double>(price):std::nullopt);
            {
                std::lock_guard<std::mutex> ql(queue_m); order_queue.push(o);
            }
            queue_cv.notify_one();
            json resp = { {"status","accepted"}, {"order_id", o.order_id} };
            res.set_content(resp.dump(), "application/json");
#else
            (void)req; (void)res; // unreachable without nlohmann
#endif
        }catch(const std::exception &e){ res.status=400; res.set_content(json({{"error", std::string("bad json: ")+e.what()}}).dump(), "application/json"); }
    });

    svr.Post("/cancel", [&](const httplib::Request& req, httplib::Response& res){
        try{
#if HAVE_JSON
            auto j = json::parse(req.body);
            std::string id = j.at("order_id").get<std::string>();
            bool ok = engine.cancel_order(id);
            res.set_content(json({{"status", ok?"canceled":"not_found"}}).dump(), "application/json");
#else
            (void)req; (void)res;
#endif
        }catch(const std::exception &e){ res.status=400; res.set_content(json({{"error", std::string("bad json: ")+e.what()}}).dump(), "application/json"); }
    });

    svr.Get("/trades", [&](const httplib::Request& req, httplib::Response& res){
        std::lock_guard<std::mutex> lk(trades_m);
        json a = json::array();
        for(const auto &t: trades){
            a.push_back(json{{"buy_order_id", t.buy_order_id},{"sell_order_id", t.sell_order_id},{"quantity", t.quantity},{"price", t.price}});
        }
        res.set_content(a.dump(), "application/json");
    });

    svr.Get("/md", [&](const httplib::Request& req, httplib::Response& res){
        std::lock_guard<std::mutex> lk(bbo_m);
        json o;
        if(last_bid) o["bid"] = json{{"price", last_bid->first},{"size", last_bid->second}}; else o["bid"] = nullptr;
        if(last_ask) o["ask"] = json{{"price", last_ask->first},{"size", last_ask->second}}; else o["ask"] = nullptr;
        res.set_content(o.dump(), "application/json");
    });

    // Long-poll endpoint for clients to receive trade/BBO events. Waits up to 10s
    svr.Get("/events/poll", [&](const httplib::Request& req, httplib::Response& res){
        std::unique_lock<std::mutex> lk(events_m);
        if(events.empty()){
            events_cv.wait_for(lk, std::chrono::seconds(10));
        }
        json a = json::array();
        while(!events.empty()){
            auto e = events.front(); events.pop_front();
            a.push_back(json{{"type", e.type},{"payload", e.payload}});
        }
        if(a.empty()){
            res.status = 204; // no content
            res.set_content("", "application/json");
            return;
        }
        res.set_content(a.dump(), "application/json");
    });

    // Add a shutdown endpoint to allow tests to stop the server gracefully
    svr.Post("/shutdown", [&](const httplib::Request& req, httplib::Response& res){
        res.set_content(json({{"status","shutting_down"}}).dump(), "application/json");
        // signal stop on a separate thread to allow response to return
        std::thread([&]{
            stop.store(true);
            queue_cv.notify_all();
            // give the client time to receive response
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            svr.stop();
        }).detach();
    });

    // start websocket broadcaster on 8081
    if(!ws.start(8081)){
        std::cerr << "Failed to start WebSocket broadcaster on port 8081\n";
    }

    std::cout << "Server listening on http://localhost:8080 and ws://localhost:8081\n";
    svr.listen("0.0.0.0", 8080);

    // shutdown
    stop.store(true); queue_cv.notify_all();
    ws.stop();
    worker.join();
    return 0;
}
