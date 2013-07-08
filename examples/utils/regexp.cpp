/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                               DEMO PROGRAMS SET
                          regexp.cpp  -  description
                             -------------------
    begin                : Jul 8, 2013
    copyright            : (C) 2003-2013 by Alexey S.Parshin
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
#include <sptk5/CRegExp.h>

#include <iostream>

using namespace std;
using namespace sptk;

int main()
{
    string text;

    text = "This text contains number: ABCDEF";
    cout << "Test: does '" << text << "' actually contains number? ";
    CRegExp regexp("[\\d]+");
    if (text == regexp) {
        cout << "yes" << endl;
    } else {
        cout << "no" << endl;
    }

    text = "This text contains number: 12345";
    cout << "Test: does '" << text << "' actually contains number? ";
    if (text == regexp) {
        cout << "yes" << endl;
    } else {
        cout << "no" << endl;
    }

    text = "This text contains phone number: (415)-123-4567";
    cout << "Test: does '" << text << "' actually contains phone number? ";
    CRegExp phoneRegexp("\\(\\d{3}\\)-\\d{3}-\\d{4}");
    if (text == phoneRegexp) {
        cout << "yes" << endl;
    } else {
        cout << "no" << endl;
    }
    text = "This text contains phone number: 415/123/4567";
    cout << "Test: does '" << text << "' contains valid phone number? ";
    if (text != CRegExp("\\(\\d{3}\\)-\\d{3}-\\d{4}")) {
        cout << "no" << endl;
    } else {
        cout << "yes" << endl;
    }

    return 0;
}
//---------------------------------------------------------------------------
