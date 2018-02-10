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

#include "jwt-private.h"
#include "base64.h"

using namespace std;
using namespace sptk;


const char * sptk::jwt_alg_str(jwt_alg_t alg)
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

jwt_alg_t sptk::jwt_str_alg(const char *alg)
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

static void jwt_scrub_key(jwt_t *jwt)
{
    jwt->key = "";
    jwt->alg = JWT_ALG_NONE;
}

static string get_js_string(json::Element *js, const char *key)
{
    json::Element *element = js;
    if (js->isObject())
        element = js->find(key);

    if (element != nullptr && element->isString())
        return element->getString();

    errno = ENOENT;
    return string();
}

static long get_js_int(json::Element *js, const char *key)
{
    json::Element *element = js;
    if (js->isObject())
        element = js->find(key);

    if (element != nullptr && element->isNumber())
        return element->getNumber();

    errno = ENOENT;
    return -1;
}

static int get_js_bool(json::Element *js, const char *key)
{
    json::Element *element = js;
    if (js->isObject())
        element = js->find(key);

    if (element != nullptr && element->isBoolean())
        return element->isBoolean() ? 1 : 0;

    errno = ENOENT;
    return -1;
}

void * sptk::jwt_b64_decode(const char *src, int *ret_len)
{
    void *buf;
    char *newData;
    int len, i, z;

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

static int jwt_sign(jwt_t *jwt, char **out, unsigned int *len, const char *str)
{
    switch (jwt->alg) {
        /* HMAC */
        case JWT_ALG_HS256:
        case JWT_ALG_HS384:
        case JWT_ALG_HS512:
            return jwt_sign_sha_hmac(jwt, out, len, str);

            /* RSA */
        case JWT_ALG_RS256:
        case JWT_ALG_RS384:
        case JWT_ALG_RS512:

            /* ECC */
        case JWT_ALG_ES256:
        case JWT_ALG_ES384:
        case JWT_ALG_ES512:
            return jwt_sign_sha_pem(jwt, out, len, str);

            /* You wut, mate? */
        default:
            return EINVAL;
    }
}

static int jwt_verify(jwt_t *jwt, const Buffer& head, const Buffer& sig)
{
    switch (jwt->alg) {
        /* HMAC */
        case JWT_ALG_HS256:
        case JWT_ALG_HS384:
        case JWT_ALG_HS512:
            return jwt_verify_sha_hmac(jwt, head.c_str(), sig.c_str());

            /* RSA */
        case JWT_ALG_RS256:
        case JWT_ALG_RS384:
        case JWT_ALG_RS512:

            /* ECC */
        case JWT_ALG_ES256:
        case JWT_ALG_ES384:
        case JWT_ALG_ES512:
            return jwt_verify_sha_pem(jwt, head.c_str(), sig.c_str());

            /* You wut, mate? */
        default:
            return EINVAL;
    }
}

static void jwt_parse_body(jwt_t *jwt, const Buffer& body)
{
    jwt_b64_decode_json(jwt->grants, body);
}

static void jwt_verify_head(jwt_t *jwt, const Buffer& head)
{
    json::Document jsdoc;
    jwt_b64_decode_json(jsdoc, head);
    json::Element* js = &jsdoc.root();

    string val = get_js_string(js, "alg");
    jwt->alg = jwt_str_alg(val.c_str());
    if (jwt->alg == JWT_ALG_INVAL) {
        throw Exception("Invalid algorithm");
    }

    if (jwt->alg != JWT_ALG_NONE) {
        /* If alg is not NONE, there may be a typ. */
        val = get_js_string(js, "typ");
        if (val != "JWT")
            throw Exception("Invalid algorithm name");

        if (jwt->key.empty())
            jwt_scrub_key(jwt);
    } else {
        /* If alg is NONE, there should not be a key */
        if (!jwt->key.empty()) {
            throw Exception("Unexpected key");
        }
    }
}

void sptk::jwt_decode(jwt_t **jwt, const char *token, const String& key)
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

    jwt_t *newData = nullptr;

    *jwt = nullptr;

    // Now that we have everything split up, let's check out the header.
    newData = new jwt_t;

    // Copy the key over for verify_head.
    if (!key.empty())
        newData->key = key;

    jwt_verify_head(newData, head);
    jwt_parse_body(newData, body);

    // Check the signature, if needed.
    if (newData->alg != JWT_ALG_NONE) {
        // Re-add this since it's part of the verified data.
        //body[-1] = '.';
        jwt_verify(newData, head, sig);
    }

    *jwt = newData;
}

string sptk::jwt_get_grant(jwt_t *jwt, const char *grant)
{
    if (!jwt || !grant || !strlen(grant)) {
        errno = EINVAL;
        return nullptr;
    }

    errno = 0;

    return get_js_string(&jwt->grants.root(), grant);
}

long sptk::jwt_get_grant_int(jwt_t *jwt, const char *grant)
{
    if (!jwt || !grant || !strlen(grant)) {
        errno = EINVAL;
        return 0;
    }

    errno = 0;

    return get_js_int(&jwt->grants.root(), grant);
}

int sptk::jwt_get_grant_bool(jwt_t *jwt, const char *grant)
{
    if (!jwt || !grant || !strlen(grant)) {
        errno = EINVAL;
        return 0;
    }

    errno = 0;

    return get_js_bool(&jwt->grants.root(), grant);
}

String jwt_get_grants_json(jwt_t *jwt, const char *grant)
{
    json::Element *js_val = nullptr;

    errno = EINVAL;

    if (!jwt)
        return nullptr;

    if (grant && strlen(grant))
        js_val = jwt->grants.root().find(grant);
    else
        js_val = &jwt->grants.root();

    if (js_val == nullptr)
        return nullptr;

    errno = 0;

    stringstream output;
    js_val->exportTo(output, false);
    return output.str();
}

int sptk::jwt_add_grant(jwt_t *jwt, const char *grant, const char *val)
{
    if (!jwt || !grant || !strlen(grant) || !val)
        return EINVAL;

    json::Element* grants = &jwt->grants.root();

    if (!get_js_string(grants, grant).empty())
        return EEXIST;

    grants->add(grant, val);

    return 0;
}

int sptk::jwt_add_grant_int(jwt_t *jwt, const char *grant, long val)
{
    if (!jwt || !grant || !strlen(grant))
        return EINVAL;

    json::Element* grants = &jwt->grants.root();

    if (get_js_int(grants, grant) != -1)
        return EEXIST;

    grants->add(grant, double(val));

    return 0;
}

int sptk::jwt_add_grant_bool(jwt_t *jwt, const char *grant, int val)
{
    if (!jwt || !grant || !strlen(grant))
        return EINVAL;

    json::Element* grants = &jwt->grants.root();

    if (get_js_int(grants, grant) != -1)
        return EEXIST;

    grants->add(grant, bool(val));

    return 0;
}

int sptk::jwt_add_grants_json(jwt_t *jwt, const char *json)
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

int sptk::jwt_del_grants(jwt_t *jwt, const char *grant)
{
    if (!jwt)
        return EINVAL;

    if (grant == nullptr || !strlen(grant))
        jwt->grants.clear();
    else
        jwt->grants.root().remove(grant);

    return 0;
}

int sptk::jwt_del_grant(jwt_t *jwt, const char *grant)
{
    return jwt_del_grants(jwt, grant);
}

static void jwt_write_head(jwt_t *jwt, ostream& output, int pretty)
{
    output << "{";

    if (pretty)
        output << endl;

    /* An unsecured JWT is a JWS and provides no "typ".
     * -- draft-ietf-oauth-json-web-token-32 #6. */
    if (jwt->alg != JWT_ALG_NONE) {
        if (pretty)
            output << "    ";

        output << "\"typ\":";
        if (pretty)
            output << " ";
        output << "\"JWT\",";

        if (pretty)
            output << endl;
    }

    if (pretty)
        output << "    ";

    output << "\"alg\":";
    if (pretty)
        output << " ";
    output << "\"" << jwt_alg_str(jwt->alg) << "\"";

    if (pretty)
        output << endl;

    output << "}";

    if (pretty)
        output << endl;
}

static void jwt_write_body(jwt_t *jwt, ostream& output, int pretty)
{
    jwt->grants.exportTo(output, pretty);
}

static void jwt_dump(jwt_t *jwt, ostream& output, int pretty)
{
    jwt_write_head(jwt, output, pretty);
    output << ".";
    jwt_write_body(jwt, output, pretty);
}

static int jwt_encode(jwt_t *jwt, ostream& out)
{
    int ret;
    unsigned int sig_len;

    /* First the header. */
    stringstream header;
    jwt_write_head(jwt, header, false);

    string data(header.str());
    Buffer encodedHead;
    Base64::encode(encodedHead, data.c_str(), data.length());

    /* Now the body. */
    stringstream body;
    jwt_write_body(jwt, body, false);

    data = body.str();
    Buffer encodedBody;
    Base64::encode(encodedBody, data.c_str(), data.length());

    jwt_base64uri_encode(encodedHead);
    jwt_base64uri_encode(encodedBody);

    Buffer output(move(encodedHead));
    output.append('.');
    output.append(encodedBody);

    if (jwt->alg == JWT_ALG_NONE) {
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
