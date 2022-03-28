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
        << l.action;
    return out;
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

    table << fort::header << "PORT" << "COMMAND" << "PID" << "USER" << "NODE" << "NAME" << "ACTION" << fort::endr;
    for (auto & l : listeners)
        table << l << fort::endr;
    table.column(5).set_cell_text_align(fort::text_align::right);

    std::cout << table.to_string() << std::endl;

}


// TODO:  Display the full command line used to start each listener.
// Steps...
// 1. add a command field to the Listener struct.
// 2. Build a map<pid,Listener> from the output of lsof
// 3. Call ps as below and parse the output lines to fill in the command values.
// 4. Make the full command line available maybe with a -c option.
//
// $ ps -p 967,10035,10147,10166 -o pid="",command=""
//   967 /usr/libexec/rapportd
// 10035 /System/Library/CoreServices/ControlCenter.app/Contents/MacOS/ControlCenter
// 10147 /Library/Application Support/Adobe/Adobe Desktop Common/ADS/Adobe Desktop Service.app/Contents/MacOS/Adobe Desktop Service --onOSstartup=true --showwindow=false --
// 10166 /Library/Application Support/Adobe/Creative Cloud Libraries/CCLibrary.app/Contents/MacOS/../libs/node /Library/Application Support/Adobe/Creative Cloud Libraries/C