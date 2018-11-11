/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          wsdl2cxx.cpp  -  description
                             -------------------
    begin                : 03 Aug 2012
    copyright            : (C) 1999-2018 by Alexey Parshin. All rights reserved.
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

#include <sptk5/wsdl/WSParser.h>

using namespace std;
using namespace sptk;

void help()
{
    cout << "WSDL to C++ prototype parser. (C) 2012-2013 Alexey Parshin" << endl << endl;
    cout << "Generates Web Service C++ class that is used as a base class for actual Web Service implementation." << endl;
    cout << "Usage:" << endl << endl;
    cout << "  wsdl2cxx <WSDL file> [output directory] [header file]" << endl;
}

int main(int argc, const char* argv[])
{
    try {
        WSParser   wsParser;
        if (argc < 2) {
            help();
            return 1;
        }

        string outputDirectory;
        if (argc > 2)
            outputDirectory = argv[2];
        else
            outputDirectory = ".";

        string headerFile;
        if (argc > 3)
            headerFile = argv[3];

        int rc = system(("mkdir -p " + outputDirectory).c_str());
        if (rc != 0) {
            cerr << "Can't open or create output directory '" << outputDirectory << "'." << endl;
            return 1;
        }

        wsParser.parse(argv[1]);
        wsParser.generate(outputDirectory, headerFile);

        return 0;
    }
    catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }
}
