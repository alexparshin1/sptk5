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
#include "config.h"

using namespace std;
using namespace sptk;

String JWT::jwt_alg_str(JWT::jwt_alg_t alg)
{
    switch (alg) {
        case JWT::JWT_ALG_NONE:
            return "none";
        case JWT::JWT_ALG_HS256:
            return "HS256";
        case JWT::JWT_ALG_HS384:
            return "HS384";
        case JWT::JWT_ALG_HS512:
            return "HS512";
        case JWT::JWT_ALG_RS256:
            return "RS256";
        case JWT::JWT_ALG_RS384:
            return "RS384";
        case JWT::JWT_ALG_RS512:
            return "RS512";
        case JWT::JWT_ALG_ES256:
            return "ES256";
        case JWT::JWT_ALG_ES384:
            return "ES384";
        case JWT::JWT_ALG_ES512:
            return "ES512";
        default:
            return nullptr;
    }
}

JWT::jwt_alg_t JWT::jwt_str_alg(const char *alg)
{
    if (alg == nullptr)
        return JWT::JWT_ALG_INVAL;

    if (!strcasecmp(alg, "none"))
        return JWT::JWT_ALG_NONE;
    else if (!strcasecmp(alg, "HS256"))
        return JWT::JWT_ALG_HS256;
    else if (!strcasecmp(alg, "HS384"))
        return JWT::JWT_ALG_HS384;
    else if (!strcasecmp(alg, "HS512"))
        return JWT::JWT_ALG_HS512;
    else if (!strcasecmp(alg, "RS256"))
        return JWT::JWT_ALG_RS256;
    else if (!strcasecmp(alg, "RS384"))
        return JWT::JWT_ALG_RS384;
    else if (!strcasecmp(alg, "RS512"))
        return JWT::JWT_ALG_RS512;
    else if (!strcasecmp(alg, "ES256"))
        return JWT::JWT_ALG_ES256;
    else if (!strcasecmp(alg, "ES384"))
        return JWT::JWT_ALG_ES384;
    else if (!strcasecmp(alg, "ES512"))
        return JWT::JWT_ALG_ES512;

    return JWT_ALG_INVAL;
}

static void jwt_scrub_key(JWT::jwt_t *jwt)
{
    if (!jwt->key.empty())
        jwt->key = "";

    jwt->alg = JWT::JWT_ALG_NONE;
}

void JWT::jwt_set_alg(jwt_alg_t alg, const String& key)
{
    m_jwt.key = alg == JWT::JWT_ALG_NONE? "" : key;
    m_jwt.alg = alg;
}

JWT::jwt_alg_t JWT::jwt_get_alg() const
{
    return m_jwt.alg;
}

JWT::JWT()
{
    m_jwt.grants = new json::Document(true);
}

JWT::JWT(const JWT& other)
{
    m_jwt.alg = other.m_jwt.alg;
    m_jwt.key = other.m_jwt.key;

    Buffer tempBuffer;
    other.m_jwt.grants->exportTo(tempBuffer, false);
    m_jwt.grants = new json::Document(true);
    m_jwt.grants->load(tempBuffer.c_str());
}

JWT::~JWT()
{
    jwt_scrub_key(&m_jwt);
    delete m_jwt.grants;
}

static String get_js_string(json::Element* js, const String& key, bool* found=nullptr)
{
    if (found != nullptr)
        *found = false;
    json::Element *element = js;
    if (js->isObject())
        element = js->find(key);

    if (element != nullptr && element->isString()) {
        if (found != nullptr)
            *found = true;
        return element->getString();
    }

    return "";
}

static long get_js_int(json::Element* js, const String& key, bool* found= nullptr)
{
    if (found != nullptr)
        *found = false;

    json::Element *element = js;
    if (js->isObject())
        element = js->find(key);

    if (element != nullptr && element->isNumber()) {
        if (found != nullptr)
            *found = true;
        return element->getNumber();
    }

    return 0;
}

static int get_js_bool(json::Element* js, const String& key, bool* found=nullptr)
{
    if (found != nullptr)
        *found = false;

    json::Element* element = js;
    if (js->isObject())
        element = js->find(key);

    if (element != nullptr && element->isBoolean()) {
        if (found != nullptr)
            *found = true;
        return element->isBoolean() ? 1 : 0;
    }
    return false;
}

void JWT::jwt_b64_decode(Buffer& dest, const Buffer& source)
{
    Buffer buffer(source);
    int i;

    // Decode based on RFC-4648 URI safe encoding.
    int len = buffer.bytes();

    for (i = 0; i < len; i++) {
        switch (buffer[i]) {
            case '-':
                buffer[i] = '+';
                break;
            case '_':
                buffer[i] = '/';
                break;
        }
    }
    while (buffer.bytes() % 4 != 0)
        buffer.append('=');

    Base64::decode(dest, buffer);
}


static sptk::json::Document *jwt_b64_decode_json(const Buffer& src)
{
    Buffer decodedData;
    Base64::decode(decodedData, src);

    auto js = new json::Document(true);
    js->load(decodedData.c_str());

    return js;
}

void JWT::jwt_base64uri_encode(Buffer& buffer)
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

void JWT::jwt_sign(char **out, unsigned int *len, const char *str)
{
    switch (m_jwt.alg) {
        // HMAC
        case JWT_ALG_HS256:
        case JWT_ALG_HS384:
        case JWT_ALG_HS512:
            jwt_sign_sha_hmac(out, len, str);
            break;

        // RSA
        case JWT_ALG_RS256:
        case JWT_ALG_RS384:
        case JWT_ALG_RS512:

        // ECC
        case JWT_ALG_ES256:
        case JWT_ALG_ES384:
        case JWT_ALG_ES512:
            jwt_sign_sha_pem(out, len, str);
            break;

        default:
            throw Exception("Invalid algorithm");
    }
}

void JWT::jwt_verify(const Buffer& head, const Buffer& sig)
{
    switch (m_jwt.alg) {
        /* HMAC */
        case JWT_ALG_HS256:
        case JWT_ALG_HS384:
        case JWT_ALG_HS512:
            jwt_verify_sha_hmac(head.c_str(), sig.c_str());
            break;

            /* RSA */
        case JWT_ALG_RS256:
        case JWT_ALG_RS384:
        case JWT_ALG_RS512:

            /* ECC */
        case JWT_ALG_ES256:
        case JWT_ALG_ES384:
        case JWT_ALG_ES512:
            jwt_verify_sha_pem(head.c_str(), sig.c_str());
            break;

            /* You wut, mate? */
        default:
            throw Exception("Invalid algorithm");
    }
}

void JWT::jwt_parse_body(const Buffer& body)
{
    if (m_jwt.grants) {
        delete m_jwt.grants;
        m_jwt.grants = nullptr;
    }

    m_jwt.grants = jwt_b64_decode_json(body);
}

void JWT::jwt_verify_head(const Buffer& head)
{
    jwt_t *jwt = &m_jwt;
    json::Document* jsdoc = jwt_b64_decode_json(head);
    json::Element* js = &jsdoc->root();

    try {
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

            if (jwt->key.empty()) {
                jwt_scrub_key(jwt);
            }
        } else {
            /* If alg is NONE, there should not be a key */
            if (!jwt->key.empty()) {
                throw Exception("Unexpected key");
            }
        }
    }
    catch (...) {
        delete jsdoc;
        throw;
    }
}

void JWT::jwt_decode(const char *token, const String& key)
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

    // Now that we have everything split up, let's check out the header.

    // Copy the key over for verify_head.
    if (!key.empty()) {
        // We don't know alg yet, so using any and set it correctly in verify_head() call
        jwt_set_alg(JWT_ALG_HS256, key);
    }

    jwt_verify_head(head);
    jwt_parse_body(body);

    // Check the signature, if needed.
    if (jwt_get_alg() != JWT_ALG_NONE) {
        // Re-add this since it's part of the verified data.
        //body[-1] = '.';
        jwt_verify(head, sig);
    }
}

String JWT::jwt_get_grant(const String& grant) const
{
    if (grant.empty())
        throw Exception("Invalid grant");

    return get_js_string(&m_jwt.grants->root(), grant);
}

long JWT::jwt_get_grant_int(const String& grant) const
{
    if (grant.empty())
        throw Exception("Invalid grant");

    return get_js_int(&m_jwt.grants->root(), grant);
}

bool JWT::jwt_get_grant_bool(const String& grant) const
{
    if (grant.empty())
        throw Exception("Invalid grant");

    return get_js_bool(&m_jwt.grants->root(), grant);
}

String JWT::jwt_get_grants_json(jwt_t *jwt, const char *grant)
{
    json::Element *js_val;

    if (!jwt)
        return "";

    if (grant && strlen(grant))
        js_val = jwt->grants->root().find(grant);
    else
        js_val = &jwt->grants->root();

    if (js_val == nullptr)
        return "";

    stringstream output;
    js_val->exportTo(output, false);
    return output.str();
}

void JWT::jwt_add_grant(const String& grant, const String& val)
{
    if (grant.empty())
        throw Exception("Invalid grant");

    json::Element* grants = &m_jwt.grants->root();

    if (!get_js_string(grants, grant).empty())
        throw Exception("Grant already exist");

    grants->add(grant, val);
}

void JWT::jwt_add_grant_int(const String& grant, long val)
{
    if (grant.empty())
        throw Exception("Invalid grant");

    json::Element* grants = &m_jwt.grants->root();

    bool exists;
    get_js_int(grants, grant, &exists);
    if (exists)
        throw Exception("Grant already exists");

    grants->add(grant, double(val));
}

void JWT::jwt_add_grant_bool(const String& grant, int val)
{
    if (grant.empty())
        throw Exception("Invalid grant");

    json::Element* grants = &m_jwt.grants->root();

    bool exists;
    get_js_int(grants, grant, &exists);
    if (exists)
        throw Exception("Grant already exists");

    grants->add(grant, bool(val));
}

int JWT::jwt_add_grants_json(jwt_t *jwt, const char *json)
{
    if (!jwt)
        return EINVAL;

    json::Document newGrantsDoc(true);
    newGrantsDoc.load(json);
    json::ObjectData& newGrantsObject = newGrantsDoc.root().getObject();
    for (auto itor: newGrantsObject) {
        json::Element* existingGrant = jwt->grants->root().find(itor.first);
        if (existingGrant)
            newGrantsObject.remove(itor.first);
        jwt->grants->root().add(itor.first, itor.second);
    }

    return 0;
}

int JWT::jwt_del_grants(jwt_t *jwt, const char *grant)
{
    if (!jwt)
        return EINVAL;

    if (grant == nullptr || !strlen(grant))
        jwt->grants->clear();
    else
        jwt->grants->root().remove(grant);

    return 0;
}

int JWT::jwt_del_grant(jwt_t *jwt, const char *grant)
{
    return jwt_del_grants(jwt, grant);
}

void JWT::jwt_write_head(ostream& output, int pretty)
{
    output << "{";

    if (pretty)
        output << endl;

    /* An unsecured JWT is a JWS and provides no "typ".
     * -- draft-ietf-oauth-json-web-token-32 #6. */
    if (m_jwt.alg != JWT_ALG_NONE) {
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
    output << "\"" << jwt_alg_str(m_jwt.alg) << "\"";

    if (pretty)
        output << endl;

    output << "}";

    if (pretty)
        output << endl;
}

void JWT::jwt_write_body(ostream& output, int pretty)
{
    m_jwt.grants->exportTo(output, pretty);
}

void JWT::jwt_dump(ostream& output, int pretty)
{
    jwt_write_head(output, pretty);
    output << ".";
    jwt_write_body(output, pretty);
}

void JWT::jwt_encode(ostream& out)
{
    unsigned int sig_len;

    /* First the header. */
    stringstream header;
    jwt_write_head(header, false);

    string data(header.str());
    Buffer encodedHead;
    Base64::encode(encodedHead, data.c_str(), data.length());

    /* Now the body. */
    stringstream body;
    jwt_write_body(body, false);

    data = body.str();
    Buffer encodedBody;
    Base64::encode(encodedBody, data.c_str(), data.length());

    jwt_base64uri_encode(encodedHead);
    jwt_base64uri_encode(encodedBody);

    Buffer output(move(encodedHead));
    output.append('.');
    output.append(encodedBody);

    if (m_jwt.alg == JWT_ALG_NONE) {
        output.append('.');
        return;
    }

    /* Now the signature. */
    char* sig;
    jwt_sign(&sig, &sig_len, output.data());

    Buffer signature;
    Base64::encode(signature, sig, sig_len);
    jwt_base64uri_encode(signature);

    output.append('.');
    output.append(signature);
}
