/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          wsdl2cxx.cpp  -  description
                             -------------------
    begin                : 03 Aug 2012
    copyright            : © 1999-2020 by Alexey Parshin. All rights reserved.
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

#include <sptk5/cutils>
#include <sptk5/wsdl/WSParser.h>
#ifdef _WIN32
#include <io.h>
#endif

using namespace std;
using namespace sptk;

void help()
{
    COUT("WSDL to C++ prototype parser. (C) 2012-2019 Alexey Parshin" << endl << endl)
    COUT("Generates Web Service C++ class that is used as a base class for actual Web Service implementation." << endl)
    COUT("Usage:" << endl << endl)
    COUT("  wsdl2cxx <WSDL file> [output directory [header file]]" << endl << endl)
    COUT("Parameters:" << endl)
    COUT("WSDL file         WSDL file that defines Web Service" << endl)
    COUT("output directory  Directory where generated files will be stored" << endl)
    COUT("header file       File that contains text too be added at the start of gerated files" << endl)
}

#ifdef _WIN32
#define access _access
#endif

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

        if (access(outputDirectory.c_str(), 0) < 0) {
            int rc = system(("mkdir " + outputDirectory).c_str());
            if (rc != 0) {
                CERR("Can't open or create output directory '" << outputDirectory << "'." << endl)
                return 1;
            }
        }

        wsParser.parse(argv[1]);
        wsParser.generate(outputDirectory, headerFile);
        wsParser.generateWsdlCxx(outputDirectory, headerFile, argv[1]);

        return 0;
    }
    catch (const Exception& e) {
        CERR(e.what() << endl)
        return 1;
    }
}
