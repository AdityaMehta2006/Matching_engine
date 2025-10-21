// Lightweight stub broadcaster used when websocketpp is not available on the system.
#include "ws_broadcaster.h"
#include <iostream>

WSBroadcaster::WSBroadcaster() {}
WSBroadcaster::~WSBroadcaster() {}

bool WSBroadcaster::start(unsigned short port){
    std::cerr << "WSBroadcaster stub: websocketpp not available, bridge needed for ws support.\n";
    (void)port;
    return false;
}

void WSBroadcaster::stop(){ }

void WSBroadcaster::broadcast(const std::string &msg){
    // no-op; print at debug level so local testing can see a hint
    std::cerr << "WSBroadcaster stub ignoring message: " << msg << "\n";
}
