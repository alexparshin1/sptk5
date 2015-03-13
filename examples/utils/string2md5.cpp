/***************************************************************************
                          unique_test.cpp  -  description
                             -------------------
    begin                : June 6 2002
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

// This example shows how to create "unique instance" application.
// Such application may only have one process running simultaneously on the same computer.

#include <iostream>
#include <sptk5/md5.h>

using namespace std;
using namespace sptk;

int main(int argc, const char* argv[])
{
    if (argc != 2) {
        cerr << "Please provide a phrase to md5 as a single parameter!" << endl;
        return 1;
    }
    
    cout << md5(argv[1]) << endl;
   
    return 0;
}
