/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       JWT.h - description                                    ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Monday Feb 12 2017                                     ║
║  copyright            (C) 1999-2017 by Alexey Parshin.                       ║
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
	jwt_alg_t       alg;
	String          key;
	json::Document  grants;

    JWT();
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

    jwt_alg_t get_alg() const;
    void set_alg(jwt_alg_t alg, const String &key);
    static const char * alg_str(jwt_alg_t alg);
    static jwt_alg_t str_alg(const char *alg);

    void sign(char** out, unsigned int* len, const char* str);
    void encode(std::ostream& out);

    void decode(const char *token, const String& key="");

    void exportTo(std::ostream& output, int pretty) const;

private:

    static const json::Element* find_grant(const json::Element *js, const String& key);

public:

    static String get_js_string(const json::Element *js, const String& key, bool* found=nullptr);
    static long get_js_int(const json::Element *js, const String& key, bool* found=nullptr);
    static bool get_js_bool(const json::Element *js, const String& key, bool* found=nullptr);

    void write_head(std::ostream& output, int pretty) const;
    void write_body(std::ostream& output, int pretty) const;

    void verify(const Buffer& head, const Buffer& sig);

    void sign_sha_hmac(char** out, unsigned int* len, const char* str);
    void verify_sha_hmac(const char* head, const char* sig);
    void sign_sha_pem(char** out, unsigned int* len, const char* str);
    void verify_sha_pem(const char* head, const char* sig_b64);
};

/* Helper routines. */
void jwt_base64uri_encode(Buffer& buffer);

void jwt_b64_decode(Buffer& destination, const char* src, int* ret_len);

}

#endif /* JWT_PRIVATE_H */
