#include <iostream>
#include <iomanip>
#include <filesystem>

#include "listener.h"
#include "fort.hpp"

static bool show_full_commands=false;

fort::char_table& operator<<(fort::char_table& out, Listener l) {
    out << l.port << l.command << l.pid << l.user << l.node  << l.name
        << l.action;
    return out;
}

void UsageAndQuit(char* first_arg) {
    auto exe_name = std::filesystem::path(first_arg).stem().string();
    std::cerr << "# usage: " << exe_name << " [-c]    # Show IPv4 TCP ports open for listening\n";
    std::cerr << "#          -c     # list full invocation command lines after table" << std::endl;
    exit(EXIT_FAILURE);
}

int main(int argc, char*argv[]) {

    if  (argc >  2)
        UsageAndQuit(argv[0]);
    
    if (argc == 2) {
        if (strcmp(argv[1], "-c") == 0)
            show_full_commands = true;
        else
            UsageAndQuit(argv[0]);
    }
            
    auto listeners = GetListeners();

    fort::char_table table;
    table.set_border_style(FT_SOLID_ROUND_STYLE);
    table.column(0).set_cell_text_align(fort::text_align::right);
    table.column(5).set_cell_text_align(fort::text_align::right);

    table << fort::header << "PORT" << "COMMAND" << "PID" << "USER" << "NODE" << "NAME" << "ACTION"<< fort::endr;
    for (auto & l : listeners)
        table << l << fort::endr;
    std::cout << table.to_string() << std::endl;

    if (show_full_commands)
        for (auto & l : listeners)
            std::cout << std::right << std::setw(7) << l.port << "  " << l.full_command << std::endl;

}
