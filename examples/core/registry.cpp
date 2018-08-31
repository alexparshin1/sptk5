/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       registry.cpp - description                             ║
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

#ifdef __BORLANDC__
#include <vcl.h>
#pragma hdrstop
#endif

#include <sptk5/Variant.h>
#include <sptk5/Registry.h>

using namespace std;
using namespace sptk;

void printRegistry(RegistryMode mode)
{
    // Open user settings, file is located in user home directory
    Registry mySettings("mySettings.ini", "sptk_test", mode);
    try {
        mySettings.load();
        cout << "---> Reading " << mySettings.fileName() << endl;

        xml::Node* windowNode = mySettings.findFirst("window");
        if (windowNode != nullptr) {
            // Processing the subnodes of <window> node
            for (auto node: *windowNode) {
                if (node->name() == "position")
                    cout << "Window position: "
                         << (int) node->getAttribute("x") << ":"
                         << (int) node->getAttribute("y") << endl;
                else if (node->name() == "colors") {
                    // Processing the subnodes of <colors>
                    cout << "Window colors:" << endl;
                    for (auto colorNode: *node) {
                        cout << "  " << (string) colorNode->getAttribute("name")
                             << ": fg " << (string) colorNode->getAttribute("foreground")
                             << ", bg " << (string) colorNode->getAttribute("background")
                             << endl;
                    }
                }
            }
        } else {
            cout << "The registry doesn't contain window information" << endl;
        }
    }
    catch (exception& e) {
        cerr << e.what() << endl;
    }
}

void updateRegistry(RegistryMode mode)
{
    // Open user settings, file is located in user home directory
    Registry mySettings("mySettings.ini", "sptk_test", mode);
    try {
        cout << "<--- Updating " << mySettings.fileName() << endl;

        xml::Node* windowNode = mySettings.findOrCreate("window");
        windowNode->clear();

        xml::Node* positionNode = new xml::Element(*windowNode, "position");
        positionNode->setAttribute("x", 100);
        positionNode->setAttribute("y", 150);

        xml::Node* colorsNode = new xml::Element(*windowNode, "colors");
        xml::Node* colorNode;

        colorNode = new xml::Element(*colorsNode, "color");
        colorNode->setAttribute("name", "Header");
        colorNode->setAttribute("foreground", "WHITE");
        colorNode->setAttribute("background", "BLUE");

        colorNode = new xml::Element(*colorsNode, "color");
        colorNode->setAttribute("name", "Text");
        colorNode->setAttribute("foreground", "0x000000");
        colorNode->setAttribute("background", "0xFF80FF");

        mySettings.save();
    }
    catch (exception& e) {
        cerr << e.what() << endl;
    }
}

int main()
{
    try {
        cout << "-------- Test for the USER (stored in homedir)  registry. -----------" << endl;

        // Print the original registry
        // The registry file is located in the user home directory
        printRegistry(USER_REGISTRY);

        // Define the registry values and save the registry
        updateRegistry(USER_REGISTRY);

        // Print the changed registry
        printRegistry(USER_REGISTRY);

        return 0;
    }
    catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }
}
//---------------------------------------------------------------------------
