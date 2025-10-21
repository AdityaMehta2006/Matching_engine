#include "ws_broadcaster.h"
#include <thread>
#include <atomic>
#include <iostream>

// Minimal implementation using websocketpp headers; keep this simple and best-effort.

#define WEBSOCKETPP_NO_CPP11_FUNCTIONAL
#define WEBSOCKETPP_NO_CPP11_STL

#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>

using server = websocketpp::server<websocketpp::config::asio>;

struct Impl {
    server srv;
    std::thread thr;
    std::atomic<bool> running{false};
    std::mutex conns_m;
    std::set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl>> conns;
};

WSBroadcaster::WSBroadcaster() : pimpl(new Impl()) {}
WSBroadcaster::~WSBroadcaster(){ stop(); delete pimpl; }

bool WSBroadcaster::start(unsigned short port){
    if(pimpl->running.load()) return true;
    try{
        pimpl->srv.init_asio();
        pimpl->srv.set_reuse_addr(true);
        pimpl->srv.set_open_handler([this](websocketpp::connection_hdl h){ std::lock_guard<std::mutex> lk(pimpl->conns_m); pimpl->conns.insert(h); });
        pimpl->srv.set_close_handler([this](websocketpp::connection_hdl h){ std::lock_guard<std::mutex> lk(pimpl->conns_m); pimpl->conns.erase(h); });
        pimpl->thr = std::thread([this,port]{
            pimpl->running.store(true);
            pimpl->srv.listen(port);
            pimpl->srv.start_accept();
            try{ pimpl->srv.run(); } catch(const std::exception &e){ std::cerr<<"ws run error: "<<e.what()<<"\n"; }
            pimpl->running.store(false);
        });
    }catch(const std::exception &e){ std::cerr<<"ws start failed: "<<e.what()<<"\n"; return false; }
    return true;
}

void WSBroadcaster::stop(){
    if(!pimpl->running.load()) return;
    try{
        pimpl->srv.stop_listening();
        pimpl->srv.stop();
    }catch(...){}
    if(pimpl->thr.joinable()) pimpl->thr.join();
}

void WSBroadcaster::broadcast(const std::string &msg){
    std::lock_guard<std::mutex> lk(pimpl->conns_m);
    for(auto h: pimpl->conns){
        websocketpp::lib::error_code ec;
        pimpl->srv.send(h, msg, websocketpp::frame::opcode::text, ec);
        if(ec){ std::cerr<<"ws send error: "<<ec.message()<<"\n"; }
    }
}
