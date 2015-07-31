/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          datetime.cpp  -  description
                             -------------------
    begin                : June 28, 2015
    copyright            : (C) 1999-2015 by Alexey Parshin
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#include <iostream>
#include <sptk5/CCommandLine.h>

using namespace std;
using namespace sptk;

CCommandLineOption
    help("help", 'h', "Prints this help.");

CCommandLineParameter
    archiveMode("archive-mode", 'a', "Archive mode may be one of {copy,zip,bzip2,xz}.", "copy", "^(copy|zip|bzip2|xz)$"),
    date("archive-date", 'd', "Date in the format 'YYYY-MM-DD'.", "", "^\\d{4}-\\d\\d-\\d\\d$");

int main(int argc, const char* argv[])
{
    CCommandLine    commandLine;
    try {
        commandLine.init(argc, argv);
    }
    catch (const exception& e) {
        cerr << "Error in command line arguments:" << endl;
        cerr << e.what() << endl;
        cout << endl;
        commandLine.printHelp();
        return 1;
    }

    if (commandLine.hasOption("help")) {
        commandLine.printHelp();
    } else {
        cout << "Archive mode: " << commandLine.parameterValue("archive-mode") << endl;
        cout << "Archive date: " << commandLine.parameterValue("archive-date") << endl;
    }

    return 0;
}

