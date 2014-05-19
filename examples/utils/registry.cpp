/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                               DEMO PROGRAMS SET
                          registry.cpp  -  description
                             -------------------
    begin                : January 3, 2003
    copyright            : (C) 1999-2014 by Alexey S.Parshin
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef __BORLANDC__
#include <vcl.h>
#pragma hdrstop
#endif

#include <sptk5/CVariant.h>
#include <sptk5/CRegistry.h>
#include <sptk5/CException.h>

#include <iostream>

using namespace std;
using namespace sptk;

void printRegistry(CRegistryMode mode) {
   CStrings strings;
   // Open user settings, file is located in user home directory
   CRegistry   mySettings("mySettings.ini", "sptk_test", mode);
   try {
      mySettings.load();
      cout << "---> Reading " << mySettings.fileName() << endl;

      CXmlNode* windowNode = mySettings.findFirst("window");
      if (windowNode) {
         CXmlNode::iterator itor = windowNode->begin();
         // Processing the subnodes of <window> node
         for (; itor != windowNode->end(); itor++) {
            CXmlNode*        node = *itor;
            if (node->name() == "position")
               cout << "Window position: "
               << (int) node->getAttribute("x") << ":"
               << (int) node->getAttribute("y") << endl;
            else if (node->name() == "colors") {
               // Processing the subnodes of <colors>
               CXmlNode::iterator itor = node->begin();
               cout << "Window colors:" << endl;
               for (; itor != node->end(); itor++) {
                  CXmlNode* colorNode = *itor;
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

void updateRegistry(CRegistryMode mode) {
   // Open user settings, file is located in user home directory
   CRegistry   mySettings("mySettings.ini", "sptk_test", mode);
   try {
      cout << "<--- Updating " << mySettings.fileName() << endl;

      CXmlNode* windowNode = mySettings.findOrCreate("window");
      windowNode->clear();

      CXmlNode* positionNode = new CXmlElement(*windowNode, "position");
      positionNode->setAttribute("x", 100);
      positionNode->setAttribute("y", 150);

      CXmlNode* colorsNode = new CXmlElement(*windowNode, "colors");
      CXmlNode* colorNode;

      colorNode = new CXmlElement(*colorsNode, "color");
      colorNode->setAttribute("name", "Header");
      colorNode->setAttribute("foreground", "WHITE");
      colorNode->setAttribute("background", "BLUE");

      colorNode = new CXmlElement(*colorsNode, "color");
      colorNode->setAttribute("name", "Text");
      colorNode->setAttribute("foreground", "0x000000");
      colorNode->setAttribute("background", "0xFF80FF");

      mySettings.save();
   }
   catch (exception& e) {
      cerr << e.what() << endl;
   }
}

int main() {
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
//---------------------------------------------------------------------------
