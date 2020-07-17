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
#include <sptk5/CommandLine.h>

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

class AppCommandLine : public CommandLine
{
public:
    AppCommandLine()
    : CommandLine("wsdl2cxx v 1.1",
                  "Generate C++ skeleton classes from WSDL file",
                  "wsdl2cxx <wsdl file> [options]")
    {
        defineArgument("wsdl", "Input WSDL file to process");

        defineParameter("cxx-directory", "d", "directory",
                        ".*",
                        CommandLine::Visibility(""), "Service",
                        "Directory where C++ files are generated");

        defineParameter("cxx-header", "h", "filename",
                        ".*",
                        CommandLine::Visibility(""), "",
                        "Header file the content of which is added to every generated C++ file");

        defineParameter("openapi-json", "j", "filename", ".*",
                        CommandLine::Visibility(""), "",
                        "Create openapi service description file. The default file name is the same as WSDL file, only with .json extention.");

        defineParameter("openapi-default-auth", "a", "auth method",
                        "^(none|basic|bearer)$",
                        CommandLine::Visibility(""), "none",
                        "Default auth method for OpenAPI operations, one of {none, basic, bearer}");

        defineParameter("openapi-operation-auth", "o", "operation:auth list",
                        R"(([\w_]+:(none|basic|bearer),?)+$)",
                        CommandLine::Visibility(""), "",
                        "Comma-separated list of operation:auth_method for OpenAPI operations, like <operation>:{none, basic, bearer}. "
                        "Defined for operations that have auth method different from default.");

        defineOption("help", "h", CommandLine::Visibility(""), "Block all messages but errors");
        defineOption("quiet", "q", CommandLine::Visibility(""), "Block all messages but errors");
        defineOption("verbose", "v", CommandLine::Visibility(""), "Verbose messages");
    }
};

auto parseOperationsAuth(const String& operationsAuth)
{
    map<String, OpenApiGenerator::AuthMethod> output;
    for (auto& operationAuthStr: Strings(operationsAuth, "[,; ]+", Strings::SM_REGEXP)) {
        operationAuthStr = operationAuthStr.trim();
        if (operationAuthStr.empty())
            continue;
        Strings parts(operationAuthStr, ":");
        if (parts.size() != 2)
            throw Exception("Invalid operation auth definition: '" + operationAuthStr + "'");
        output[parts[0]] = OpenApiGenerator::authMethod(parts[1]);
    }
    return output;
}

int main(int argc, const char* argv[])
{
    AppCommandLine commandLine;

    try {
        size_t screenColumns = 80;
        const auto* colsStr = getenv("COLS");
        if (colsStr != nullptr)
            screenColumns = string2int(colsStr);

        commandLine.init(argc, argv);

        if (argc == 1 || commandLine.hasOption("help")) {
            commandLine.printHelp(screenColumns);
            return 1;
        }

        auto quiet = commandLine.hasOption("quiet");
        auto verbose = commandLine.hasOption("verbose");

        if (commandLine.arguments().size() != 1) {
            CERR("Please provide single WSDL file name");
            commandLine.printHelp(screenColumns);
            return 1;
        }
        auto wsdlFile = commandLine.arguments().front();

        WSParser   wsParser;
        if (argc < 2) {
            help();
            return 1;
        }

        string outputDirectory = commandLine.getOptionValue("cxx-directory").trim();
        if (outputDirectory.empty())
            outputDirectory = ".";

        string headerFile = commandLine.getOptionValue("cxx-directory").trim();

        if (outputDirectory != "." && access(outputDirectory.c_str(), 0) < 0) {
            int rc = system(("mkdir " + outputDirectory).c_str());
            if (rc != 0) {
                CERR("Can't open or create output directory '" << outputDirectory << "'." << endl)
                return 1;
            }
        }

        if (!quiet && verbose) {
            COUT("Input WSDL file:             " << wsdlFile << endl);
            COUT("Generate files to directory: " << wsdlFile << endl);
            if (!headerFile.empty())
                COUT("Using C++ header file:       " << headerFile << endl);
        }

        OpenApiGenerator::Options options;
        options.defaultAuthMethod = OpenApiGenerator::authMethod(commandLine.getOptionValue("openapi-default-auth"));
        options.operationsAuth = parseOperationsAuth(commandLine.getOptionValue("openapi-operation-auth"));
        options.openApiFile = commandLine.getOptionValue("openapi-json");

        wsParser.parse(wsdlFile);
        wsParser.generate(outputDirectory, headerFile, options, verbose);
        wsParser.generateWsdlCxx(outputDirectory, headerFile, wsdlFile);

        return 0;
    }
    catch (const Exception& e) {
        CERR(e.what() << endl)
        return 1;
    }
}
