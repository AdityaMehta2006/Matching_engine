#pragma once

#include <string>
#include <functional>

class WSBroadcaster {
public:
    WSBroadcaster();
    ~WSBroadcaster();
    bool start(unsigned short port);
    void stop();
    void broadcast(const std::string &msg);
};
