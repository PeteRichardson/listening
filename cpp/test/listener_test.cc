#include "listener.h"

#include <cassert>
#include <string>

int main() {
    Listener l("node                        10166 pete   31u  IPv4 0xe4ad34249b227fc5      0t0  TCP 127.0.0.1:45623 (LISTEN)\n");
    assert(l.command == "node");
    assert(l.pid == 10166);
    assert(l.user == "pete");
    assert(l.fd == "31u");
    assert(l.node == "TCP");
    assert(l.inaddr == "127.0.0.1");
    assert(l.port == 45623);
    assert(l.action == "LISTEN");

    Listener escaped("Adobe\\x20Desktop\\x20Service  1102 pete   10u  IPv4 0x6c6607cf201365e5      0t0  TCP 127.0.0.1:15292 (LISTEN)\n");
    assert(escaped.command == "Adobe Desktop Service");
    assert(escaped.port == 15292);

    return 0;
}
