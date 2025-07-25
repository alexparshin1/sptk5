/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       zlib_test.cpp - description                            ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Sunday June 24 2018                                    ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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

// This example shows how to compress and uncompress data using ZLib library.

#include <sptk5/cutils>
#include <sptk5/ZLib.h>

using namespace std;
using namespace sptk;

int main()
{
    try {
        Buffer testData("============================ 1234567890 1234567890 test data 1234567890 1234567890 ============================");
        COUT("Test data:              " << testData.bytes() << " bytes." << endl);

        Buffer compressedData;
        ZLib::compress(compressedData, testData);

        testData.reset(); // Decompressed data will be appended to destination
        ZLib::decompress(testData, compressedData);

        COUT("Compressed test data:   " << compressedData.bytes() << " bytes." << endl);
        COUT("Decompressed test data: " << testData.bytes() << " bytes." << endl);

        COUT(testData << endl);
    }
    catch (const Exception& e) {
        CERR(e.what() << endl);
        return 1;
    }
    return 0;
}
