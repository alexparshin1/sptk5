/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                     encrypt_decrypt.cpp - description                        ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Tuesday July 18 2017                                   ║
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

#include <sptk5/cutils>
#include <sptk5/Crypt.h>

using namespace std;
using namespace sptk;

int main()
{
    string text("The quick brown fox jumps over the lazy dog.ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    string key("01234567890123456789012345678901");
    string iv("0123456789012345");

    Buffer intext(text), outtext;
    cout << "Encrypt text (" << text.length() << " bytes)." << endl;
    try {
        Crypt::encrypt(outtext, intext, key, iv);
        cout << outtext << endl;
        Crypt::decrypt(intext, outtext, key, iv);
        cout << intext << endl;
    }
    catch (exception& e) {
        puts(e.what());
    }

    return 0;
}
