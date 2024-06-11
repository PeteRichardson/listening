//
//  Listener.swift
//  listening-swift
//
//  Created by Peter Richardson on 6/14/22.
//

import Foundation

struct Listener : CustomStringConvertible {
    let port : Int
    let command : String
    let pid : Int
    let fd : String
    let user : String
    let node : String
    let inaddr : String
    let action : String
    let fullCommand : String
    
    init(port:Int, command:String, pid:Int, fd:String, user:String,
         node:String, inaddr:String, action:String, fullCommand:String) {
        self.port = port
        self.command = command
        self.pid = pid
        self.fd = fd
        self.user = user
        self.node = node
        self.inaddr = inaddr
        self.action = action
        self.fullCommand = fullCommand
    }
    
    // regex that matches lines from "/usr/sbin/lsof -nP +c 0 -i4" like:
    // Adobe\x20Desktop\x20Service  1102 pete   10u  IPv4 0x6c6607cf201365e5      0t0  TCP 127.0.0.1:15292 (LISTEN)
    let lsofRegex = #/
        (?<command>\S+)
        \s+
        (?<pid>\d+)
        \s+
        (?<user>\w+)
        \s+
        (?<fd>\d+u)
        \s+
        IPv4
        \s+
        0x[0-9a-f]{16}
        \s+
        0t0
        \s+
        (?<node>\w+)
        \s+
        (?<inaddr>(\*|[\.0-9]+))
        :
        (?<port>\d+)
        \s+
        \(
        (?<action>LISTEN)
        \)
    /#
    
    /// use regex to create a Listener from a line from the output of /usr/sbin/lsof -nP +c 0 -i4
    init?(_ lsofRow : String) {
        guard let match = try? lsofRegex.wholeMatch(in: lsofRow) else {
            //            print("# ERROR: Listener(\"\(lsofRow)\") init failed.")
            return nil
        }
        
        // TODO:  Just call init() with info below, and make this a convenience initializer?
        
        port        = Int(String(match.port)) ?? 0
        command     = String(match.command).replacingOccurrences(of: "\\x20", with: " ")
        pid         = Int(String(match.pid)) ?? 0
        fd          = String(match.fd)
        user        = String(match.user)
        node        = String(match.node)
        inaddr      = String(match.inaddr)
        action      = String(match.action)
        fullCommand = ""
    }
    
    var description: String {
        return String(format:"%7d %@ %7d %@ %@ %@ %@ %@",
                      port,
                      command.padding(toLength: 25, withPad:" ", startingAt:0),
                      pid,
                      fd.padding     (toLength: 7,  withPad:" ", startingAt:0),
                      user.padding   (toLength: 7,  withPad:" ", startingAt:0),
                      node.padding   (toLength: 5,  withPad:" ", startingAt:0),
                      inaddr.padding (toLength: 17, withPad:" ", startingAt:0),
                      action.padding (toLength: 8,  withPad:" ", startingAt:0))
    }
}


/// Convenience type to automatically run the lsof command and parse the output
/// into an array of Listener structs
/// Allows you to just do:   for listener in Listeners() { ... }
typealias Listeners = [Listener]
extension Listeners {
    init() {
        // Get the stdout, stderr and status of the lsof shell command
        // Note that the definition of the Listener struct initializers assumes the
        // exact output of this hardcoded command.
        // Be careful if you change the args, or if you upgrade your OS (!)
        let (stdout, _, _) = shell("/usr/sbin/lsof", ["-nP","+c", "0", "-i4"])
        
        // TODO:   Should really check the status code and print the stderr output
        //          e.g. if lsof isn't in /usr/sbin, this will fail and return an empty array
        
        // Try to instantiate a listener from each line
        // if Listener.init can't parse a line it returns nil and is tossed out by compactMap
        if let lines = stdout?.components(separatedBy: "\n") {
            self.init(lines.compactMap { Listener($0) })
        } else {
            self.init()
        }
    }
}
