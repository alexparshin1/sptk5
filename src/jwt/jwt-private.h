/* Copyright (C) 2015-2017 Ben Collins <ben@cyphre.com>
   This file is part of the JWT C Library

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef JWT_PRIVATE_H
#define JWT_PRIVATE_H

#include <sptk5/cutils>

namespace sptk {

struct jwt
{
	jwt_alg_t       alg;
	String          key;
	json::Document  grants;

    jwt() : alg(JWT_ALG_NONE), grants(true)
    {
    }

    jwt_t* clone()
    {
        auto newJWT = new jwt_t;

        if (!key.empty()) {
            newJWT->alg = alg;
            newJWT->key = key;
        }

        Buffer tempBuffer;
        grants.exportTo(tempBuffer, false);
        newJWT->grants.load(tempBuffer.c_str());

        return newJWT;
    }

    String get_grant(const String& grant) const
    {
        return get_js_string(&grants.root(), grant);
    }

    long get_grant_int(const String& grant) const
    {
        return jwt::get_js_int(&grants.root(), grant);
    }

    bool get_grant_bool(const String& grant)
    {
        return jwt::get_js_bool(&grants.root(), grant);
    }

    void add_grant(const String& grant, const String& val)
    {
        bool exists;
        get_js_string(&grants.root(), grant, &exists);
        if (exists)
            throw Exception("Grant already exists");

        grants.root().add(grant, val);
    }

    void add_grant_int(const String& grant, long val)
    {
        bool exists;
        get_js_int(&grants.root(), grant, &exists);
        if (exists)
            throw Exception("Grant already exists");

        grants.root().add(grant, double(val));
    }

    void add_grant_bool(const String& grant, bool val)
    {
        bool exists;
        get_js_int(&grants.root(), grant, &exists);
        if (exists)
            throw Exception("Grant already exists");

        grants.root().add(grant, bool(val));
    }

    void del_grant(const String& grant)
    {
        grants.root().remove(grant);
    }


    jwt_alg_t get_alg() const
    {
        return alg;
    }

    void set_alg(jwt_alg_t alg, const String &key)
    {
        switch (alg) {
            case JWT_ALG_NONE:
                if (!key.empty())
                    throw Exception("Key is not expected here");
                break;

            default:
                if (key.empty())
                    throw Exception("Empty key is not expected here");
        }

        this->key = key;
        this->alg = alg;
    }

    static const char * alg_str(jwt_alg_t alg)
    {
        switch (alg) {
            case JWT_ALG_NONE:
                return "none";
            case JWT_ALG_HS256:
                return "HS256";
            case JWT_ALG_HS384:
                return "HS384";
            case JWT_ALG_HS512:
                return "HS512";
            case JWT_ALG_RS256:
                return "RS256";
            case JWT_ALG_RS384:
                return "RS384";
            case JWT_ALG_RS512:
                return "RS512";
            case JWT_ALG_ES256:
                return "ES256";
            case JWT_ALG_ES384:
                return "ES384";
            case JWT_ALG_ES512:
                return "ES512";
            default:
                return nullptr;
        }
    }

    static jwt_alg_t str_alg(const char *alg)
    {
        if (alg == nullptr)
            return JWT_ALG_INVAL;

        if (!strcasecmp(alg, "none"))
            return JWT_ALG_NONE;
        else if (!strcasecmp(alg, "HS256"))
            return JWT_ALG_HS256;
        else if (!strcasecmp(alg, "HS384"))
            return JWT_ALG_HS384;
        else if (!strcasecmp(alg, "HS512"))
            return JWT_ALG_HS512;
        else if (!strcasecmp(alg, "RS256"))
            return JWT_ALG_RS256;
        else if (!strcasecmp(alg, "RS384"))
            return JWT_ALG_RS384;
        else if (!strcasecmp(alg, "RS512"))
            return JWT_ALG_RS512;
        else if (!strcasecmp(alg, "ES256"))
            return JWT_ALG_ES256;
        else if (!strcasecmp(alg, "ES384"))
            return JWT_ALG_ES384;
        else if (!strcasecmp(alg, "ES512"))
            return JWT_ALG_ES512;

        return JWT_ALG_INVAL;
    }


private:

    void jwt_scrub_key()
    {
        key = "";
        alg = JWT_ALG_NONE;
    }

    static const json::Element* find_grant(const json::Element *js, const String& key)
    {
        if (js->isObject()) {
            auto element = js->find(key);
            return element;
        }
        return nullptr;
    }

public:

    static String get_js_string(const json::Element *js, const String& key, bool* found=nullptr)
    {
        if (found)
            *found = false;
        const json::Element *element = find_grant(js, key);
        if (element != nullptr && element->isString()) {
            if (found)
                *found = true;
            return element->getString();
        }
        return String();
    }

    static long get_js_int(const json::Element *js, const String& key, bool* found=nullptr)
    {
        if (found)
            *found = false;
        const json::Element *element = find_grant(js, key);
        if (element != nullptr && element->isNumber()) {
            if (found)
                *found = true;
            return element->getNumber();
        }
        return 0;
    }

    static bool get_js_bool(const json::Element *js, const String& key, bool* found=nullptr)
    {
        if (found)
            *found = false;
        const json::Element *element = find_grant(js, key);
        if (element != nullptr && element->isBoolean()) {
            if (found)
                *found = true;
            return element->isBoolean() ? 1 : 0;
        }
        return false;
    }

};

/* Helper routines. */
void jwt_base64uri_encode(Buffer& buffer);

void* jwt_b64_decode(const char* src, int* ret_len);

/* These routines are implemented by the crypto backend. */
int jwt_sign_sha_hmac(jwt_t* jwt, char** out, unsigned int* len, const char* str);

int jwt_verify_sha_hmac(jwt_t* jwt, const char* head, const char* sig);

int jwt_sign_sha_pem(jwt_t* jwt, char** out, unsigned int* len, const char* str);

int jwt_verify_sha_pem(jwt_t* jwt, const char* head, const char* sig_b64);

}

#endif /* JWT_PRIVATE_H */
