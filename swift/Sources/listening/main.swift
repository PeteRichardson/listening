//
//  main.swift
//  listening-swift
//
//  Created by Peter Richardson on 6/14/22.
//

import Foundation

func usageAndQuit() -> Never {
    let exeName = (CommandLine.arguments.first as NSString?)?.lastPathComponent ?? "listening"
    FileHandle.standardError.write("# usage: \(exeName) [-c]    # Show IPv4 TCP ports open for listening\n".data(using: .utf8)!)
    FileHandle.standardError.write("#          -c     # list full invocation command lines after table\n".data(using: .utf8)!)
    exit(EXIT_FAILURE)
}

let args = CommandLine.arguments.dropFirst()
var showFullCommands = false
if args.count > 1 {
    usageAndQuit()
}
if let onlyArg = args.first {
    if onlyArg == "-c" {
        showFullCommands = true
    } else {
        usageAndQuit()
    }
}

let listeners = Listeners().sorted(by: { $0.port < $1.port })
for listener in listeners {
    print(listener)
}

if showFullCommands {
    for listener in listeners {
        print(String(format: "%7d  %@", listener.port, listener.fullCommand))
    }
}

