#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <vector>

#include "fort.hpp"

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

Listener::Listener(std::string lsof_line) {
    // node                        10166 pete   31u  IPv4 0xe4ad34249b227fc5      0t0  TCP 127.0.0.1:45623 (LISTEN)
    std::stringstream ss{lsof_line};
    std::string dummy;

    ss >> this->command >> this->pid >> this->user;
    ss >> dummy >> dummy >> dummy >> dummy >> this->node;
    ss >> this->name;

    std::string escaped_space = "\\x20";
    while(this->command.find(escaped_space) != std::string::npos) {
        this->command.replace(this->command.find(escaped_space), escaped_space.size(), " ");
    };

    size_t pos = name.find(":");
    auto namestr = name.substr(pos+1);
    this->port = stoi(namestr);
    this->full_command = command;
}

std::ostream& operator<<(std::ostream& out, Listener l) {
    out << std::setw(7) << l.port
         << std::setw(25) << l.command
         << std::setw(7) << l.pid
         << std::setw(7) << l.user
         << std::setw(5) << l.node
         << std::setw(17) << l.name
         << std::setw(8) << l.action;
    return out;
}

fort::char_table& operator<<(fort::char_table& out, Listener l) {
    // out << std::setw(7) <<  << " " <<  << " " <<  << " " <<  << " " << e << " " <<  << " " << << endl;
    out << l.port
        << l.command
        << l.pid
        << l.user
        << l.node
        << l.name
        << l.action
        << l.full_command;
    return out;
}

void dump_full_commands(std::vector<Listener> listeners) {
    std::stringstream pids{};
    for (auto & l : listeners) {
        pids << l.pid << ",";
    }
    std::string cmd{"ps -o pid=\"\",command=\"\" -p "};
    cmd = cmd + pids.str();

    FILE *fp = popen(cmd.c_str(), "r");
    if (fp == NULL) {
        std::cerr << "Failed to run command '" << cmd << "'" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    const size_t kBUFSIZE = 512;
    char line_buffer[kBUFSIZE];
    while (!feof(fp)) {
        if (fgets(line_buffer, kBUFSIZE, fp) != NULL) {
            std::string line{line_buffer};
            std::cout << line << std::endl;
        }
    }
    pclose(fp);
}

int main(int argc, char*argv[]) {
    char cmd[]{"lsof -nP +c 0 -i4"};

    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        std::cerr << "Failed to run command '" << cmd << "'" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::vector<Listener> listeners{};

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

    sort(listeners.begin(), listeners.end(), [] (Listener& lhs, Listener& rhs) { return lhs.port < rhs.port; });

    fort::char_table table;
    table.set_border_style(FT_SOLID_ROUND_STYLE);

    table << fort::header << "PORT" << "COMMAND" << "PID" << "USER" << "NODE" << "NAME" << "ACTION" << "FULL COMMAND" << fort::endr;
    for (auto & l : listeners)
        table << l << fort::endr;
    table.column(5).set_cell_text_align(fort::text_align::right);

    std::cout << table.to_string() << std::endl;

    dump_full_commands(listeners);

}
