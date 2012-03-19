/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                               DEMO PROGRAMS SET
                          xml_test3.cpp  -  description
                             -------------------
    begin                : August 20, 2007
    copyright            : (C) 2003-2012 by Alexey S.Parshin
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

#ifdef WIN32
#include <direct.h>
#endif

#include <sptk5/sptk.h>
#include <sptk5/cxml>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <iomanip>

using namespace std;
using namespace sptk;

void testXPath(string fileName, string xpath, int expectedNodeCount = -1)
{
    CXmlDoc doc;
    CBuffer buf;

    cout << "Test file: " << fileName << endl;
    cout << "XPath is : " << xpath << endl;
    cout << "Selected Nodes are:" << endl;

    buf.loadFromFile(fileName);
    doc.load(buf);

    CXmlNodeVector selectedNodes;
    doc.select(selectedNodes, xpath);
    for (CXmlNode::iterator itor = selectedNodes.begin(); itor != selectedNodes.end(); itor++) {
        CXmlNode* node = *itor;
        cout << node->name();
        if (node->hasAttribute("N"))
            cout << ", N=" << node->getAttribute("N").str();
        if (!node->value().empty())
            cout << ", value=" << node->value();
        cout << endl;
    }
    cout << endl;
    if (expectedNodeCount != -1 && expectedNodeCount != (int) selectedNodes.size()) {
        stringstream st;
        st << "ERROR: expected " << expectedNodeCount << " node(s) in selection, got " << selectedNodes.size() << " node(s)" << endl;
        throw runtime_error(st.str());
    }
}

int main(int argc, char *argv[])
{
    try {
        cout << "The XPath selection test started." << endl << endl;

        string fullPath(argv[0]);
        unsigned pos = fullPath.rfind("/");
        if (pos == STRING_NPOS)
            pos = fullPath.rfind("\\");
        if (pos == STRING_NPOS)
            throw CException("Can't determine work directory");

        string workDirectory(fullPath.substr(0, pos));
        chdir(workDirectory.c_str());

        // http://www.zvon.org/xxl/XPathTutorial/Output/example1.html
        testXPath("xpath_test1.xml", "/AAA", 1);          // Select the root element AAA
        testXPath("xpath_test1.xml", "/AAA/CCC", 2);      // Select all elements CCC which are children of the root element AAA
        testXPath("xpath_test1.xml", "/AAA/DDD/BBB", 1);  // Select all elements BBB which are children of DDD which are children of the root element AAA

        // http://www.zvon.org/xxl/XPathTutorial/Output/example2.html
        testXPath("xpath_test2.xml", "//BBB", 5);
        testXPath("xpath_test2.xml", "//DDD/BBB", 3);

        // http://www.zvon.org/xxl/XPathTutorial/Output/example3.html
        testXPath("xpath_test3.xml", "/AAA/CCC/DDD/*", 4);
        testXPath("xpath_test3.xml", "/*/*/*/BBB", 5);
        testXPath("xpath_test3.xml", "//*", 17);

        // http://www.zvon.org/xxl/XPathTutorial/Output/example4.html
        testXPath("xpath_test4.xml", "/AAA/BBB[1]", 1);
        testXPath("xpath_test4.xml", "/AAA/BBB[last()]", 1);

        // http://www.zvon.org/xxl/XPathTutorial/Output/example5.html
        testXPath("xpath_test5.xml", "//@id", 2);
        testXPath("xpath_test5.xml", "//BBB[@id]", 2);
        testXPath("xpath_test5.xml", "//BBB[@name]", 1);
        testXPath("xpath_test5.xml", "//BBB[@*]", 3);

        // http://www.zvon.org/xxl/XPathTutorial/Output/example6.html
        testXPath("xpath_test6.xml", "//BBB[@id='b1']", 1);
        testXPath("xpath_test6.xml", "//BBB[@name='bbb']", 1);

        cout << "The XPath selection test completed" << endl;

    }
    catch (std::exception& e) {
        cout << e.what() << endl;
        return 1;
    }
    return 0;
}
