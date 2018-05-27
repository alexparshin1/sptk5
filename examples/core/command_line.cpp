/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       command_line.cpp - description                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
┌──────────────────────────────────────────────────────────────────────────────┐
│   This library is free software; you can redistribute it and/or modify it    │
│   under the terms of the GNU Library General Public License as published by  │
│   the Free Software Foundation; either version 2 of the License, or (at your │
│   option) any later version.                                                 │
│                                                                              │
│   This library is distributed in the hope that it will be useful, but        │
│   WITHOUT ANY WARRANTY; without even the implied warranty of                 │
│   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library   │
│   General Public License for more details.                                   │
│                                                                              │
│   You should have received a copy of the GNU Library General Public License  │
│   along with this library; if not, write to the Free Software Foundation,    │
│   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <iostream>
#include <sptk5/CommandLine.h>

using namespace std;
using namespace sptk;

int main(int argc, const char* argv[])
{
    try {
        CommandLine commandLine(
                "Command Line Arguments demo v.1.00",
                "Demonstrates basic command line support.",
                "command_line <command> [options]");

        commandLine.defineOption("help", "h", CommandLine::Visibility(""), "Prints this help.");
        commandLine.defineParameter("archive-mode", "a", "mode", "^(copy|zip|bzip2|xz)$", CommandLine::Visibility("archive"), "copy",  "Archive mode may be one of {copy,zip,bzip2,xz}.");
        commandLine.defineParameter("archive-date", "d", "date", "^\\d{4}-\\d\\d-\\d\\d$", CommandLine::Visibility(""), "", "Date in the format 'YYYY-MM-DD'.");
        commandLine.defineArgument("archive", "Archive data (does nothing)");
        commandLine.defineArgument("restore", "Restore data (does nothing)");
        try {
            commandLine.init(argc, argv);
        }
        catch (const exception& e) {
            cerr << "Error in command line arguments:" << endl;
            cerr << e.what() << endl;
            cout << endl;
            commandLine.printHelp(80);
            return 1;
        }

        String command;
        if (!commandLine.arguments().empty())
            command = commandLine.arguments()[0];

        if (command == "help") {
            // Print full help
            commandLine.printHelp(80);
            return 0;
        }

        if (commandLine.hasOption("help")) {
            // Print help on command (if any) or full help
            commandLine.printHelp(command, 80);
        } else {
            cout << "Archive mode: " << commandLine.getOptionValue("archive-mode") << endl;
            cout << "Archive date: " << commandLine.getOptionValue("archive-date") << endl;
        }

        return 0;
    }
    catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }
}
