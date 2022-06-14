//
//  main.swift
//  listening-swift
//
//  Created by Peter Richardson on 6/14/22.
//

import Foundation

for listener in Listeners().sorted(by: { $0.port < $1.port })  {
    print(listener)
}

