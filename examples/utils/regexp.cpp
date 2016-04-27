/***************************************************************************
 SIMPLY POWERFUL TOOLKIT (SPTK)
 DEMO PROGRAMS SET
 regexp.cpp  -  description
 -------------------
 begin                : Jul 8, 2013
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

#include <sptk5/cutils>
#include <sptk5/RegularExpression.h>

#include <iostream>

using namespace std;
using namespace sptk;

int main()
{
    string text;

    text = "This text contains number: ABCDEF";
    cout << "Test: does '" << text << "' contain number? ";
    RegularExpression regexp("[\\d]+");
    if (text == regexp) {
        cout << "yes" << endl;
    }
    else {
        cout << "no" << endl;
    }

    text = "This text contains number: 12345";
    cout << "Test: does '" << text << "' contain number? ";
    if (text == regexp) {
        cout << "yes" << endl;
    }
    else {
        cout << "no" << endl;
    }

    text = "This text contains phone number: (415)-123-4567";
    cout << "Test: does '" << text << "' contain valid phone number? ";
    RegularExpression phoneRegexp("\\(\\d{3}\\)-\\d{3}-\\d{4}");
    if (text == phoneRegexp) {
        cout << "yes" << endl;
    }
    else {
        cout << "no" << endl;
    }
    text = "This text contains phone number: 415/123/4567";
    cout << "Test: does '" << text << "' contain valid phone number? ";
    if (text != RegularExpression("\\(\\d{3}\\)-\\d{3}-\\d{4}"))
        cout << "no" << endl;
    else
        cout << "yes" << endl;

    text = "user='horse' noice='some' password='haystack' host='localhost'";
    cout << "\nParsing the text: " << text << endl;
    RegularExpression connectionParser("(user|password|host)='([\\S]+)'", "g");
    RegularExpression parameterParser("(\\S+)=['\"]([\\S]+)['\"]");
    CStrings matches;
    connectionParser.m(text, matches);
    for (unsigned i = 0; i < matches.size(); i++) {
        cout << matches[i] << " : ";
        i++;
        cout << matches[i] << endl;
    }
    cout << endl;

    text = "Area code: 415 Phone: 123-4567";
    cout << "\nParsing the text: " << text << endl;
    RegularExpression phoneStringParser("^Area code: (\\d{3}) Phone: (\\d{3})-(\\d{4})$");
    string phoneNumber = phoneStringParser.s(text, "(\\1)-\\2-\\3");
    cout << "Reformatted phone number: " << phoneNumber << endl << endl;

    CDateTime started = CDateTime::Now();

    unsigned counter = 0;
    unsigned tests = 1000000;
    for (unsigned i = 0; i < tests; i++) {
        if (text == RegularExpression("\\(\\d{3}\\)-\\d{3}-\\d{4}"))
            counter++;
    }
    CDateTime finished = CDateTime::Now();
    cout << "Executed " << tests << " regexp tests (compiled on the fly) for " << (finished - started) * 86400 << " seconds." << endl;
    cout.precision(2);
    cout << "That is " << fixed << tests / ((finished - started) * 86400) / 1000000 << "M tests/sec" << endl;

    started = CDateTime::Now();
    counter = 0;
    for (unsigned i = 0; i < tests; i++) {
        if (text == phoneRegexp)
            counter++;
    }
    finished = CDateTime::Now();
    cout << "Executed " << tests << " regexp tests (precompiled) for " << (finished - started) * 86400 << " seconds." << endl;
    cout.precision(2);
    cout << "That is " << fixed << tests / ((finished - started) * 86400) / 1000000 << "M tests/sec" << endl;

    return 0;
}
//---------------------------------------------------------------------------
