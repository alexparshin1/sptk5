/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                               DEMO PROGRAMS SET
                          driver_load_test.cpp  -  description
                             -------------------
    begin                : Mar 24, 2012
    copyright            : (C) 2000-2012 by Alexey S.Parshin
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

#include <iostream>
#include <iomanip>

#include <sptk5/db/CODBCDatabase.h>
#include <sptk5/db/CDatabaseDriverLoader.h>
#include <sptk5/cdatabase>
#include <sptk5/cutils>

using namespace std;
using namespace sptk;

int main()
{
    CDatabaseDriverLoader driverLoader;
    try {
        driverLoader.load("odbc");
        CDatabaseDriver* driverInstance = driverLoader.createDriverInstance("connstring");
        cout << driverInstance->connectionString() << endl;
    }
    catch (exception& e) {
        cerr << e.what() << endl;
    }
    return 0;
}
