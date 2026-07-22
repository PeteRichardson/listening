//
//  shell.swift
//  listening-swift
//
//  Created by Peter Richardson on 6/14/22.
//

import Foundation

/// wrapper function for shell commands
/// must provide full path to executable
///
/// example: let (stdout, stderr, status) = shell("/usr/sbin/lsof", ["-nP","+c", "0", "-i4"])
///
/// source: taken from from angusc's StackOverflow answer at
/// https://stackoverflow.com/questions/26971240/how-do-i-run-a-terminal-command-in-a-swift-script-e-g-xcodebuild
/// and trivially adapted to return stderr as well
///
func shell(_ launchPath: String, _ arguments: [String] = []) -> (String?, String?, Int32) {
    let task = Process()
    task.executableURL = URL(fileURLWithPath: launchPath)
    task.arguments = arguments
    
    let outpipe = Pipe()
    let errpipe = Pipe()
    task.standardOutput = outpipe
    task.standardError = errpipe
    
    do {
        try task.run()
    } catch {
        // handle errors
        print("Error: \(error.localizedDescription)")
    }
    
    let outdata = outpipe.fileHandleForReading.readDataToEndOfFile()
    let stdout = String(data: outdata, encoding: .utf8)
    let errdata = errpipe.fileHandleForReading.readDataToEndOfFile()
    let stderr = String(data: errdata, encoding: .utf8)
    
    task.waitUntilExit()
    return (stdout, stderr, task.terminationStatus)
}
