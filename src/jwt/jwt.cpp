/* Copyright (C) 2015-2018 Ben Collins <ben@cyphre.com>
   This file is part of the JWT C Library

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <cstdlib>
#include <cstring>
#include <cerrno>

#include <sptk5/jwt.h>
#include <sstream>
#include <sptk5/Base64.h>

#include "sptk5/JWT.h"
#include "base64.h"

using namespace std;
using namespace sptk;


void * sptk::jwt_b64_decode(const char *src, int *ret_len)
{
    void *buf;
    char *newData;
    size_t len, i, z;

    /* Decode based on RFC-4648 URI safe encoding. */
    len = strlen(src);
    newData = (char*) alloca(len + 4);
    if (!newData)
        return nullptr;

    for (i = 0; i < len; i++) {
        switch (src[i]) {
            case '-':
                newData[i] = '+';
                break;
            case '_':
                newData[i] = '/';
                break;
            default:
                newData[i] = src[i];
        }
    }
    z = 4 - (i % 4);
    if (z < 4) {
        while (z--)
            newData[i++] = '=';
    }
    newData[i] = '\0';

    buf = malloc(i);
    if (buf == nullptr)
        return nullptr;

    *ret_len = jwt_Base64decode((char *)buf, newData);

    return buf;
}


static void jwt_b64_decode_json(json::Document &dest, const Buffer &src)
{
    Buffer decodedData;
    Base64::decode(decodedData, src);

    dest.load(decodedData.c_str());
}

void sptk::jwt_base64uri_encode(Buffer& buffer)
{
    char* str = buffer.data();
    int len = strlen(str);
    int i, t;

    for (i = t = 0; i < len; i++) {
        switch (str[i]) {
            case '+':
                str[t++] = '-';
                break;
            case '/':
                str[t++] = '_';
                break;
            case '=':
                break;
            default:
                str[t++] = str[i];
        }
    }

    str[t] = '\0';
}

static int jwt_sign(JWT *jwt, char **out, unsigned int *len, const char *str)
{
    switch (jwt->alg) {
        /* HMAC */
        case JWT::JWT_ALG_HS256:
        case JWT::JWT_ALG_HS384:
        case JWT::JWT_ALG_HS512:
            return jwt_sign_sha_hmac(jwt, out, len, str);

            /* RSA */
        case JWT::JWT_ALG_RS256:
        case JWT::JWT_ALG_RS384:
        case JWT::JWT_ALG_RS512:

            /* ECC */
        case JWT::JWT_ALG_ES256:
        case JWT::JWT_ALG_ES384:
        case JWT::JWT_ALG_ES512:
            return jwt_sign_sha_pem(jwt, out, len, str);

            /* You wut, mate? */
        default:
            return EINVAL;
    }
}

static int jwt_verify(JWT *jwt, const Buffer& head, const Buffer& sig)
{
    switch (jwt->alg) {
        /* HMAC */
        case JWT::JWT_ALG_HS256:
        case JWT::JWT_ALG_HS384:
        case JWT::JWT_ALG_HS512:
            return jwt_verify_sha_hmac(jwt, head.c_str(), sig.c_str());

            /* RSA */
        case JWT::JWT_ALG_RS256:
        case JWT::JWT_ALG_RS384:
        case JWT::JWT_ALG_RS512:

            /* ECC */
        case JWT::JWT_ALG_ES256:
        case JWT::JWT_ALG_ES384:
        case JWT::JWT_ALG_ES512:
            return jwt_verify_sha_pem(jwt, head.c_str(), sig.c_str());

            /* You wut, mate? */
        default:
            return EINVAL;
    }
}

static void jwt_parse_body(JWT *jwt, const Buffer& body)
{
    jwt_b64_decode_json(jwt->grants, body);
}

static void jwt_verify_head(JWT *jwt, const Buffer& head)
{
    json::Document jsdoc;
    jwt_b64_decode_json(jsdoc, head);
    json::Element* js = &jsdoc.root();

    string val = JWT::get_js_string(js, "alg");
    jwt->alg = JWT::str_alg(val.c_str());
    if (jwt->alg == JWT::JWT_ALG_INVAL) {
        throw Exception("Invalid algorithm");
    }

    if (jwt->alg != JWT::JWT_ALG_NONE) {
        /* If alg is not NONE, there may be a typ. */
        val = JWT::get_js_string(js, "typ");
        if (val != "JWT")
            throw Exception("Invalid algorithm name");

        if (jwt->key.empty())
            jwt->alg = JWT::JWT_ALG_NONE;
    } else {
        /* If alg is NONE, there should not be a key */
        if (!jwt->key.empty()) {
            throw Exception("Unexpected key");
        }
    }
}

void sptk::jwt_decode(JWT **jwt, const char *token, const String& key)
{
    struct {
        const char* data;
        size_t      length;
    } parts[3] = {};

    size_t index = 0;
    for (const char* data = token; data != nullptr && index < 3; index++) {
        parts[index].data = data;
        const char* end = strchr(data, '.');
        if (end == nullptr) {
            parts[index].length = strlen(data);
            data = nullptr;
            break;
        }
        parts[index].length = end - data;
        data = end + 1;
    }

    if (parts[1].data == nullptr)
        throw Exception("Invalid number of token parts");

    Buffer head(parts[0].data, parts[0].length), body(parts[1].data, parts[1].length), sig(parts[2].data, parts[2].length);

    JWT *newData = nullptr;

    *jwt = nullptr;

    // Now that we have everything split up, let's check out the header.
    newData = new JWT;

    // Copy the key over for verify_head.
    if (!key.empty())
        newData->key = key;

    jwt_verify_head(newData, head);
    jwt_parse_body(newData, body);

    // Check the signature, if needed.
    if (newData->alg != JWT::JWT_ALG_NONE) {
        // Re-add this since it's part of the verified data.
        //body[-1] = '.';
        jwt_verify(newData, head, sig);
    }

    *jwt = newData;
}

int sptk::jwt_add_grants_json(JWT *jwt, const char *json)
{
    if (!jwt)
        return EINVAL;

    json::Document newGrantsDoc(true);
    newGrantsDoc.load(json);
    json::ObjectData& newGrantsObject = newGrantsDoc.root().getObject();
    for (auto itor: newGrantsObject) {
        json::Element* existingGrant = jwt->grants.root().find(itor.first);
        if (existingGrant)
            newGrantsObject.remove(itor.first);
        jwt->grants.root().add(itor.first, itor.second);
    }

    return 0;
}

static void jwt_dump(JWT *jwt, ostream& output, int pretty)
{
    jwt->write_head(output, pretty);
    output << ".";
    jwt->write_body(output, pretty);
}

static int jwt_encode(JWT *jwt, ostream& out)
{
    int ret;
    unsigned int sig_len;

    /* First the header. */
    stringstream header;
    jwt->write_head(header, false);

    string data(header.str());
    Buffer encodedHead;
    Base64::encode(encodedHead, data.c_str(), data.length());

    /* Now the body. */
    stringstream body;
    jwt->write_body(body, false);

    data = body.str();
    Buffer encodedBody;
    Base64::encode(encodedBody, data.c_str(), data.length());

    jwt_base64uri_encode(encodedHead);
    jwt_base64uri_encode(encodedBody);

    Buffer output(move(encodedHead));
    output.append('.');
    output.append(encodedBody);

    if (jwt->alg == JWT::JWT_ALG_NONE) {
        output.append('.');
        return 0;
    }

    /* Now the signature. */
    char* sig;
    ret = jwt_sign(jwt, &sig, &sig_len, output.data());
    if (ret)
        return ret;

    Buffer signature;
    Base64::encode(signature, sig, sig_len);
    jwt_base64uri_encode(signature);

    output.append('.');
    output.append(signature);

    return ret;
}
