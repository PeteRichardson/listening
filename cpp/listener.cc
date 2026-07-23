#include "listener.h"

#include <sstream>
#include <iomanip>
#include <stdio.h>
#include <map>
#include <memory>
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
    if (pos != std::string::npos) {
        auto inaddrstr = inaddr.substr(pos+1);
        this->inaddr.erase(pos);
        if (!inaddrstr.empty() &&
            inaddrstr.find_first_not_of("0123456789") == std::string::npos) {
            this->port = stoi(inaddrstr);
        }
    }
    this->full_command = command;
}

std::ostream& operator<<(std::ostream& out, const Listener& l) {
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
    std::map<pid_t, Listener*> pid_lookup_table{};

    std::stringstream pids{};
    for (auto & l : listeners) {
        pids << l.pid << ",";
        pid_lookup_table.insert({l.pid, &l});
    }
    std::string cmd{"ps -o pid=\"\",command=\"\" -p "};
    cmd = cmd + pids.str();

    auto fp = std::unique_ptr<FILE, decltype(&pclose)>(popen(cmd.c_str(), "r"), pclose);
    if (!fp) {
        std::cerr << "Failed to run command '" << cmd << "'" << std::endl;
        std::exit(EXIT_FAILURE);
    }


    const size_t kBUFSIZE = ARG_MAX; // need sufficient mem else long cmd lines
                                     // will be split and parsing will fail
    std::vector<char> line_buffer(kBUFSIZE);
    while (!feof(fp.get())) {
        if (fgets(line_buffer.data(), kBUFSIZE, fp.get()) != NULL) {
            pid_t pid { -1 };
            std::string full_command{};
            std::stringstream ss{line_buffer.data()};
            ss >> pid >> std::ws;
            if (pid != -1) {
                getline(ss, full_command);
                auto it = pid_lookup_table.find(pid);
                if (it != pid_lookup_table.end()) {
                    it->second->full_command = full_command;
                }

            };
        }
    }
}

Listeners GetListeners(bool resolve_full_commands) {
    Listeners listeners{};
    char cmd[]{"lsof -nP +c 0 -i4 2>&1"};

    auto fp = std::unique_ptr<FILE, decltype(&pclose)>(popen(cmd, "r"), pclose);
    if (!fp) {
        std::cerr << "Failed to run command '" << cmd << "'" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    // lsof lines can include long process paths; a small buffer would let
    // fgets silently truncate them, producing garbled parsed fields.
    const size_t kBUFSIZE = 4096;
    char line_buffer[kBUFSIZE];
    while (!feof(fp.get())) {
        if (fgets(line_buffer, kBUFSIZE, fp.get()) != NULL) {
            std::string line{line_buffer};
            if (line.ends_with("(LISTEN)\n"))
                listeners.push_back(Listener(line));
        }
    }

    if (resolve_full_commands) {
        load_full_commands(listeners);
    }

    sort(listeners.begin(), listeners.end(), [] (Listener& lhs, Listener& rhs) { return lhs.port < rhs.port; });
    return listeners;
}