/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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

using namespace std;
using namespace sptk;

constexpr int TEXT_BLOCK = 16384;

void Crypt::encrypt(Buffer& dest, const Buffer& src, const String& key, const String& iv)
{
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (ctx == nullptr)
    {
        throw Exception("Error calling EVP_CIPHER_CTX_new()");
    }

    /* Initialise the encryption operation. IMPORTANT - ensure you use a key
     * and IV size appropriate for your cipher
     * In this example we are using 256-bit AES (i.e. a 256-bit key). The
     * IV size for *most* modes is the same as the block size. For AES this
     * is 128 bits */
    if (constexpr int minimalKeyLength = 32;
        key.length() < minimalKeyLength)
    {
        throw Exception("Please use 256 bit key");
    }

    if (constexpr int minimalIvLength = 16;
        iv.length() < minimalIvLength)
    {
        throw Exception("Please use 128 bit initialization vector");
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, bit_cast<const unsigned char*>(key.c_str()),
                           bit_cast<const unsigned char*>(iv.c_str())) != 1)
    {
        throw Exception("Error calling EVP_EncryptInit_ex()");
    }

    int len = 0;
    dest.bytes(0);
    dest.checkSize(src.bytes());
    for (size_t position = 0; position < src.bytes(); position += TEXT_BLOCK)
    {
        const auto* intext = src.data() + position;
        size_t      inlen = src.bytes() - position;
        if (inlen > TEXT_BLOCK)
        {
            inlen = TEXT_BLOCK;
        }
        dest.checkSize(position + TEXT_BLOCK);
        if (EVP_EncryptUpdate(ctx, dest.data() + dest.bytes(), &len, intext, static_cast<int>(inlen)) != 1)
        {
            throw Exception("Error calling EVP_EncryptUpdate()");
        }
        dest.bytes(dest.bytes() + len);
    }

    dest.checkSize(dest.bytes() + TEXT_BLOCK);
    if (EVP_EncryptFinal_ex(ctx, dest.data() + dest.bytes(), &len) != 1)
    {
        throw Exception("Error calling EVP_EncryptFinal_ex()");
    }
    dest.bytes(dest.bytes() + len);

    // Clean up
    EVP_CIPHER_CTX_free(ctx);
}

void Crypt::decrypt(Buffer& dest, const Buffer& src, const String& key, const String& iv)
{
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (ctx == nullptr)
    {
        throw Exception("Error calling EVP_CIPHER_CTX_new()");
    }

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, bit_cast<const unsigned char*>(key.c_str()),
                           bit_cast<const unsigned char*>(iv.c_str())) != 1)
    {
        throw Exception("Error calling EVP_DecryptInit_ex()");
    }

    int len = 0;
    dest.bytes(0);
    dest.checkSize(src.bytes());
    for (size_t position = 0; position < src.bytes(); position += TEXT_BLOCK)
    {
        const auto* intext = src.data() + position;
        size_t      inlen = src.bytes() - position;
        if (inlen > TEXT_BLOCK)
        {
            inlen = TEXT_BLOCK;
        }
        dest.checkSize(position + TEXT_BLOCK);
        if (EVP_DecryptUpdate(ctx, dest.data() + dest.bytes(), &len, intext, static_cast<int>(inlen)) != 1)
        {
            throw Exception("Error calling EVP_DecryptUpdate()");
        }
        dest.bytes(dest.bytes() + len);
        if (len < TEXT_BLOCK - 16)
        {
            break;
        }
    }

    dest.checkSize(dest.bytes() + TEXT_BLOCK);
    if (EVP_DecryptFinal_ex(ctx, dest.data() + dest.bytes(), &len) != 1)
    {
        throw Exception("Error calling EVP_DecryptFinal_ex()");
    }
    dest.bytes(dest.bytes() + len);
    dest[dest.bytes()] = 0;

    // Clean up
    EVP_CIPHER_CTX_free(ctx);
}
