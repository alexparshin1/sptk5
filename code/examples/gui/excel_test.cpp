/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       excel_test.cpp - description                           ║
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

#include <iostream>
#include <iomanip>
#include <sptk5/excel/CExcelBook.h>
#include <sptk5/cexcel>

int main(int argc, char* argv[])
{
  sptk::CExcelBook book("org");
  book.init();
  book.newSheet("debug");
  book.sheets[0]->resize(4,4);
  // write row 1

  book.sheets[0]->rows[0][0].setFloat( 0.00000020 );
  book.sheets[0]->rows[0][1].setFloat(-0.22222220 );
  book.sheets[0]->rows[0][2].setFloat( 2.10000000 );        // 2.1
  book.sheets[0]->rows[0][3].setFloat( 9999999.999900 );    // this works!
  // row 2
  book.sheets[0]->rows[1][0].setFloat( 0.00000030 ); 
  book.sheets[0]->rows[1][1].setFloat(-0.33333330 );
  book.sheets[0]->rows[1][2].setFloat( 3.00000000 );        // 3.0
  book.sheets[0]->rows[1][3].setFloat( 10000000.999900 );   // not working

  // write row 3
  book.sheets[0]->rows[2][0].setFloat( 0.00000080 );
  book.sheets[0]->rows[2][1].setFloat(-0.88888880 );
  book.sheets[0]->rows[2][2].setFloat( 8.00000000 );        // 8.0
  book.sheets[0]->rows[2][3].setFloat( 10000005.99990000 ); // not working
  // write row 4 (only one cell)
  book.sheets[0]->rows[3][0].setFloat( 10000005.99990000 ); // this works!

  // uncomment following and suprisingly everything works!
  // could't reproduce with sptk2.x
  // following line can be at anywhere you like
  //sptk::CExcelCell dumb;

  book.writeExcelFile("excel_test.xls");

  return 0;
}

