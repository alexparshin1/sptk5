/* Copyright (C) 2015-2017 Ben Collins <ben@cyphre.com>
   This file is part of the JWT C Library

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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

    int sign_sha_hmac(char** out, unsigned int* len, const char* str);
    int verify_sha_hmac(const char* head, const char* sig);
    void sign_sha_pem(char** out, unsigned int* len, const char* str);
    int verify_sha_pem(const char* head, const char* sig_b64);
};

/* Helper routines. */
void jwt_base64uri_encode(Buffer& buffer);

void jwt_b64_decode(Buffer& destination, const char* src, int* ret_len);

}

#endif /* JWT_PRIVATE_H */
