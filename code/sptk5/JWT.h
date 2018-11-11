/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       JWT.h - description                                    ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Monday Feb 12 2017                                     ║
║  copyright            (C) 1999-2018 by Alexey Parshin.                       ║
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
┌──────────────────────────────────────────────────────────────────────────────┐
│   The code in this module is based JWT C Library, developed by Ben Collins.  │
│   Please see http://github.com/benmcollins/libjwt for more information.      │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#ifndef __JWT_H__
#define __JWT_H__

#include <sptk5/cutils>
#include <sstream>

namespace sptk {

/**
 * Java Web Token encoding and decoding
 */
class JWT
{
public:
    /** JWT algorithm types. */
    enum jwt_alg_t {
        JWT_ALG_NONE = 0,
        JWT_ALG_HS256,
        JWT_ALG_HS384,
        JWT_ALG_HS512,
        JWT_ALG_RS256,
        JWT_ALG_RS384,
        JWT_ALG_RS512,
        JWT_ALG_ES256,
        JWT_ALG_ES384,
        JWT_ALG_ES512,
        JWT_ALG_TERM
    };

#define JWT_ALG_INVAL JWT_ALG_TERM

public:
    jwt_alg_t       alg;        ///< Signature encryption algorithm
    String          key;        ///< Signature encryption key
    json::Document  grants;     ///< Token content

    /**
     * Constructor
     */
    JWT();

    /**
     * Copy constructor
     * @param other             Other JWT object
     */
    JWT(const JWT& other);

    /**
     * Get JSON element in JSON object element by name.
     * If element doesn't exist in JSON object yet, it's created as JSON null element.
     * If this element is not JSON object, an exception is thrown.
     * @param name              Name of the element in the object element
     * @returns Element for the name, or NULL if not found
     */
    json::Element& operator[](const std::string& name)
    {
        return grants.root()[name];
    }

    /**
     * Get JSON element in JSON object element by name.
     * If element doesn't exist in JSON object yet, then reference to static const JSON null element is returned.
     * If this element is not JSON object, an exception is thrown.
     * @param name              Name of the element in the object element
     * @returns Element for the name, or NULL if not found
     */
    const json::Element& operator[](const std::string& name) const
    {
        return grants.root()[name];
    }

    /*
     * Get signature encryption algorithm
     * @return signature encryption algorithm
     */
    jwt_alg_t get_alg() const;

    /**
     * Set signature encryption algorithm
     * @param alg               Signature encryption algorithm
     * @param key               Signature encryption key
     */
    void set_alg(jwt_alg_t alg, const String &key);

    /**
     * Get signature encryption algorithm name
     * @param alg               Signature encryption algorithm
     * @return
     */
    static const char * alg_str(jwt_alg_t alg);

    /**
     * Get signature encryption algorithm from name
     * @param alg               Signature encryption algorithm name
     * @return
     */
    static jwt_alg_t str_alg(const char *alg);

    /**
     * Sign token
     * @param out               Output token data
     * @param len               Output token length
     * @param str               Data to sign
     */
    void sign(char** out, unsigned int* len, const char* str);

    /**
     * Encode token to stream
     * @param out               Output stream
     */
    void encode(std::ostream& out);

    /**
     * Decode token
     * @param token             Input token data
     * @param key               Optional signature encryption key
     */
    void decode(const char *token, const String& key="");

    /**
     * Export token to stream
     * @param output            Output stream
     * @param pretty            If true, produce formatted output
     */
    void exportTo(std::ostream& output, bool pretty) const;

private:

    /**
     * Find grant in token content
     * @param js                Parent JSON element
     * @param key               Grant name
     * @return JSON element, or nullptr if not found
     */
    static const json::Element* find_grant(const json::Element *js, const String& key);

public:

    /**
     * Find string grant value by name
     * @param js                Parent JSON element
     * @param key               Grant name
     * @param found             Optional (output) flag, true is found
     * @return grant value
     */
    static String get_js_string(const json::Element *js, const String& key, bool* found=nullptr);

    /**
     * Find integer grant value by name
     * @param js                Parent JSON element
     * @param key               Grant name
     * @param found             Optional (output) flag, true is found
     * @return grant value
     */
    static long get_js_int(const json::Element *js, const String& key, bool* found=nullptr);

    /**
     * Find boolean grant value by name
     * @param js                Parent JSON element
     * @param key               Grant name
     * @param found             Optional (output) flag, true is found
     * @return grant value
     */
    static bool get_js_bool(const json::Element *js, const String& key, bool* found=nullptr);

    /**
     * Write token head to output stream
     * @param output            Output stream
     * @param pretty            If true then produce formatted output
     */
    void write_head(std::ostream& output, bool pretty) const;

    /**
     * Write token body to output stream
     * @param output            Output stream
     * @param pretty            If true then produce formatted output
     */
    void write_body(std::ostream& output, bool pretty) const;

    /**
     * Verify token
     * @param head              Token head
     * @param sig               Signature
     */
    void verify(const Buffer& head, const Buffer& sig);

    /**
     * Sign using SHA algorithm to HMAC format
     * @param out               Output data
     * @param len               Output data length
     * @param str               Input data
     */
    void sign_sha_hmac(char** out, unsigned int* len, const char* str);

    /**
     * Verify using SHA algorithm in HMAC format
     * @param head              Token head
     * @param sig               Signature
     */
    void verify_sha_hmac(const char* head, const char* sig);

    /**
     * Sign using SHA algorithm to PEM format
     * @param out               Output data
     * @param len               Output data length
     * @param str               Input data
     */
    void sign_sha_pem(char** out, unsigned int* len, const char* str);

    /**
     * Verify using SHA algorithm in PEM format
     * @param head              Token head
     * @param sig_b64           Signature
     */
    void verify_sha_pem(const char* head, const char* sig_b64);
};

/**
 * Encode token into Base64 URI format
 * @param buffer                Token data (input and output)
 */
void jwt_base64uri_encode(Buffer& buffer);

/**
 * Decode token from Base64 URI format
 * @param destination           Token data (output)
 * @param src                   Token data (input)
 */
void jwt_b64_decode(Buffer& destination, const char* src);

} // namespace sptk

#endif
