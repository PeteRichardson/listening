#include "listener.h"

#include <sstream>
#include <iomanip>
#include <stdio.h>
#include <map>
#include <sys/syslimits.h>
#include <sys/types.h>

Listener::Listener(std::string lsof_line) {
    // node                        10166 pete   31u  IPv4 0xe4ad34249b227fc5      0t0  TCP 127.0.0.1:45623 (LISTEN)
    std::stringstream ss{lsof_line};
    std::string dummy;

    ss >> this->command >> this->pid >> this->user;
    ss >> this->fd >> dummy >> dummy >> dummy >> this->node;
    ss >> this->inaddr;

    std::string escaped_space = "\\x20";
    while(this->command.find(escaped_space) != std::string::npos) {
        this->command.replace(this->command.find(escaped_space), escaped_space.size(), " ");
    };

    size_t pos = inaddr.find(":");
    auto inaddrstr = inaddr.substr(pos+1);
    this->inaddr.erase(pos);
    this->port = stoi(inaddrstr);
    this->full_command = command;
}

std::ostream& operator<<(std::ostream& out, Listener l) {
    out << std::setw(7) << l.port
         << std::setw(25) << l.command
         << std::setw(7) << l.pid
         << std::setw(7) << l.fd
         << std::setw(7) << l.user
         << std::setw(5) << l.node
         << std::setw(17) << l.inaddr
         << std::setw(8) << l.action;
    return out;
}

void load_full_commands(std::vector<Listener> & listeners) {
    std::map<pid_t, Listener&> pid_lookup_table{};

    std::stringstream pids{};
    for (auto & l : listeners) {
        pids << l.pid << ",";
        pid_lookup_table.insert( std::pair<int, Listener&>(l.pid, l));
    }
    std::string cmd{"ps -o pid=\"\",command=\"\" -p "};
    cmd = cmd + pids.str();

    FILE *fp = popen(cmd.c_str(), "r");
    if (fp == NULL) {
        std::cerr << "Failed to run command '" << cmd << "'" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    
    const size_t kBUFSIZE = ARG_MAX; // need sufficient mem else long cmd lines
                                     // will be split and parsing will fail 
    char line_buffer[kBUFSIZE];
    while (!feof(fp)) {
        if (fgets(line_buffer, kBUFSIZE, fp) != NULL) {
            pid_t pid { -1 };   
            std::string full_command{};
            std::stringstream ss{line_buffer};
            ss >> pid >> std::ws;
            if (pid != -1) {
                getline(ss, full_command);
                pid_lookup_table.at(pid).full_command = full_command;
            };
        }
    }
    pclose(fp);
}

Listeners GetListeners(void) {
    Listeners listeners{};
    char cmd[]{"lsof -nP +c 0 -i4"};

    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        std::cerr << "Failed to run command '" << cmd << "'" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    const size_t kBUFSIZE = 512;
    char line_buffer[kBUFSIZE];
    while (!feof(fp)) {
        if (fgets(line_buffer, kBUFSIZE, fp) != NULL) {
            std::string line{line_buffer};
            if (line.ends_with("(LISTEN)\n"))
                listeners.push_back(Listener(line));
        }
    }
    pclose(fp);

    load_full_commands(listeners);

    sort(listeners.begin(), listeners.end(), [] (Listener& lhs, Listener& rhs) { return lhs.port < rhs.port; });
    return listeners;
}