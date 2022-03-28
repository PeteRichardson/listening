#include <iostream>

#include "listener.h"
#include "fort.hpp"


fort::char_table& operator<<(fort::char_table& out, Listener l) {
    out << l.port << l.command << l.pid << l.user << l.node  << l.name
        << l.action << l.full_command;
    return out;
}

int main(int argc, char*argv[]) {

    auto listeners = GetListeners();

    fort::char_table table;
    table.set_border_style(FT_SOLID_ROUND_STYLE);
    table.column(5).set_cell_text_align(fort::text_align::right);

    table << fort::header << "PORT" << "COMMAND" << "PID" << "USER" << "NODE" << "NAME" << "ACTION" << "FULL COMMAND" << fort::endr;
    for (auto & l : listeners)
        table << l << fort::endr;
    std::cout << table.to_string() << std::endl;
}
