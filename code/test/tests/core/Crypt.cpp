/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2022 Alexey Parshin. All rights reserved.       ║
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

#include <openssl/evp.h>
#include <sptk5/Crypt.h>
#include <sptk5/Exception.h>

#include <gtest/gtest.h>
#include <sptk5/Base64.h>


using namespace std;
using namespace sptk;

static const String testText("The quick brown fox jumps over the lazy dog.ABCDEFGHIJKLMNOPQRSTUVWXYZ");
static const String testKey("01234567890123456789012345678901");
static const String testIV("0123456789012345");
static const String encryptedB64(
    "4G9jpxHot6qflEAQfUaAoReZQ4DqMdKimblTAtQ5uXDTSIEjcUAiDF1QrdMc1bFLyizf6AIDArct48AnL8KBENhT/jBS8kVz7tPBysfHBKE=");

TEST(SPTK_Crypt, encrypt)
{
    Buffer encrypted;
    String encryptedStr;

    EXPECT_THROW(Crypt::encrypt(encrypted, Buffer(testText), "xxx", testIV), Exception);
    EXPECT_THROW(Crypt::encrypt(encrypted, Buffer(testText), testKey, "xxx"), Exception);

    Crypt::encrypt(encrypted, Buffer(testText), testKey, testIV);
    Base64::encode(encryptedStr, encrypted);

    EXPECT_STREQ(encryptedB64.c_str(), encryptedStr.c_str());
}

TEST(SPTK_Crypt, decrypt)
{
    Buffer encrypted;
    Buffer decrypted;

    Base64::decode(encrypted, encryptedB64);
    Crypt::decrypt(decrypted, encrypted, testKey, testIV);

    EXPECT_STREQ(testText.c_str(), decrypted.c_str());
}
