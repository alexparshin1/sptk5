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
        if (grant.empty())
            throw Exception("Invalid grant name");

        return get_js_string(&grants.root(), grant);
    }

    void add_grant(const String& grant, const String& val)
    {
        if (grant.empty())
            throw Exception("Invalid grant name");

        if (!get_js_string(&grants.root(), grant).empty())
            throw Exception("Grant already exists");

        grants.root().add(grant, val);
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

private:

    void jwt_scrub_key()
    {
        key = "";
        alg = JWT_ALG_NONE;
    }

    static String get_js_string(const json::Element *js, const String& key)
    {
        const json::Element *element = js;
        if (js->isObject())
            element = js->find(key);

        if (element != nullptr && element->isString())
            return element->getString();

        return String();
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
