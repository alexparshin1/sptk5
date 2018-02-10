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
    JWT* clone() const;

    String get_grant(const String& grant) const;
    long   get_grant_int(const String& grant) const;
    bool   get_grant_bool(const String& grant) const;
    String get_grants_json(const String& grant) const;

    void add_grant(const String& grant, const String& val);
    void add_grant_int(const String& grant, long val);
    void add_grant_bool(const String& grant, bool val);
    void add_grants_json(const char *json);

    void del_grant(const String& grant);

    jwt_alg_t get_alg() const;
    void set_alg(jwt_alg_t alg, const String &key);
    static const char * alg_str(jwt_alg_t alg);
    static jwt_alg_t str_alg(const char *alg);

    int sign(char **out, unsigned int *len, const char *str);
    int encode(std::ostream& out);

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
};

/* Helper routines. */
void jwt_base64uri_encode(Buffer& buffer);

void * jwt_b64_decode(const char *src, int *ret_len);

/* These routines are implemented by the crypto backend. */
int jwt_sign_sha_hmac(JWT* jwt, char** out, unsigned int* len, const char* str);
int jwt_verify_sha_hmac(JWT* jwt, const char* head, const char* sig);
int jwt_sign_sha_pem(JWT* jwt, char** out, unsigned int* len, const char* str);
int jwt_verify_sha_pem(JWT* jwt, const char* head, const char* sig_b64);

}

#endif /* JWT_PRIVATE_H */
