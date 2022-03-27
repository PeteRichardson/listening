#include <iostream>
#include <sstream>
#include <stdio.h>
#include <vector>

using std::cout, std::endl;

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
    out << l.port << " " << l.command << " " << l.pid << " " << l.user << " " << l.node << " " << l.name << " " << l.action << endl;
    return out;
}

void handle_line(std::string line) {
}

int main(int argc, char*argv[]) {
    char cmd[]{"lsof -nP +c 0 -i4"};

    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        std::cerr << "Failed to run command '" << cmd << "'" << endl;
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

    for (auto & l : listeners) {
        cout << l;
    }
}