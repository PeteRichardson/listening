#ifndef LISTENER_H
#define LISTENER_H

#include <string>
#include <iostream>
#include <vector>

struct Listener {
    int port{};
    std::string command{};
    int pid;
    std::string user;
    std::string node;
    std::string name{};
    std::string action{"LISTEN"};
    std::string full_command{};

    Listener(std::string);
};

using Listeners = std::vector<Listener>;

Listeners GetListeners(void);

std::ostream& operator<<(std::ostream&, Listener);

#endif