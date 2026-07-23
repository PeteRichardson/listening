import XCTest
@testable import listening

final class ListenerTests: XCTestCase {
    func testParsesBasicFields() throws {
        let line = "node                        10166 pete   31u  IPv4 0xe4ad34249b227fc5      0t0  TCP 127.0.0.1:45623 (LISTEN)"
        let listener = try XCTUnwrap(Listener(line))
        XCTAssertEqual(listener.command, "node")
        XCTAssertEqual(listener.pid, 10166)
        XCTAssertEqual(listener.user, "pete")
        XCTAssertEqual(listener.fd, "31u")
        XCTAssertEqual(listener.node, "TCP")
        XCTAssertEqual(listener.inaddr, "127.0.0.1")
        XCTAssertEqual(listener.port, 45623)
        XCTAssertEqual(listener.action, "LISTEN")
    }

    func testParsesEscapedSpacesInCommand() throws {
        let line = "Adobe\\x20Desktop\\x20Service  1102 pete   10u  IPv4 0x6c6607cf201365e5      0t0  TCP 127.0.0.1:15292 (LISTEN)"
        let listener = try XCTUnwrap(Listener(line))
        XCTAssertEqual(listener.command, "Adobe Desktop Service")
        XCTAssertEqual(listener.port, 15292)
    }

    func testRejectsNonListenLine() {
        let line = "not a valid lsof line"
        XCTAssertNil(Listener(line))
    }
}
