#include <iostream>
#include <iomanip>
#include <filesystem>

#include "listener.h"
#include "fort.hpp"

static struct {
    bool  show_full_commands {false};    // -c: list full invocation command lines after table
} options;

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

void ParseOptions(int argc, char* argv[]) {
    // only option is optional -c
    // otherwise, show usage and quit.
    if  (argc > 2)
        UsageAndQuit(argv[0]);
    
    if (argc == 2) {
        if (strcmp(argv[1], "-c") == 0)
            options.show_full_commands = true;
        else
            UsageAndQuit(argv[0]);
    }
}

int main(int argc, char*argv[]) {

    ParseOptions(argc, argv);
            
    auto listeners = GetListeners();

    // Set up formatted table for output.   See https://github.com/seleznevae/libfort
    fort::char_table table;
    table.set_border_style(FT_SOLID_ROUND_STYLE);
    table.column(0).set_cell_text_align(fort::text_align::right);   // port
    table.column(2).set_cell_text_align(fort::text_align::right);   // pid
    table.column(5).set_cell_text_align(fort::text_align::right);   // name (e.g. 127.0.0.1) looks better right justified

    table << fort::header << "PORT" << "COMMAND" << "PID" << "USER" << "NODE" << "INADDR" << "ACTION"<< fort::endr;
    for (auto & l : listeners)
        table << l << fort::endr;
    std::cout << table.to_string() << std::endl;

    if (options.show_full_commands)
        for (auto & l : listeners)
            std::cout << std::right << std::setw(7) << l.port << "  " << l.full_command << std::endl;

}
