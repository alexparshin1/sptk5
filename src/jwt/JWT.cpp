/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       JWT.cpp - description                                  ║
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

#include <sptk5/JWT.h>
#include <sptk5/Base64.h>

using namespace std;
using namespace sptk;

JWT::JWT()
: alg(JWT_ALG_NONE), grants(true)
{
}

JWT::JWT(const JWT& other)
: alg(other.alg), key(other.key), grants(true)
{
    Buffer tempBuffer;
    other.grants.exportTo(tempBuffer, false);
    grants.load(tempBuffer.c_str());
}

JWT::jwt_alg_t JWT::get_alg() const
{
    return alg;
}

void JWT::set_alg(jwt_alg_t alg, const String &key)
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

const char * JWT::alg_str(jwt_alg_t alg)
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

JWT::jwt_alg_t JWT::str_alg(const char *alg)
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

const json::Element* JWT::find_grant(const json::Element *js, const String& key)
{
    if (js->isObject()) {
        auto element = js->find(key);
        return element;
    }
    return nullptr;
}

String JWT::get_js_string(const json::Element *js, const String& key, bool* found)
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

long JWT::get_js_int(const json::Element *js, const String& key, bool* found)
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

bool JWT::get_js_bool(const json::Element *js, const String& key, bool* found)
{
    if (found)
        *found = false;
    const json::Element *element = find_grant(js, key);
    if (element != nullptr && element->isBoolean()) {
        if (found)
            *found = true;
        return element->getBoolean();
    }
    return false;
}

void JWT::write_head(std::ostream& output, int pretty) const
{
    output << "{";

    if (pretty)
        output << std::endl;

    /* An unsecured JWT is a JWS and provides no "typ".
     * -- draft-ietf-oauth-json-web-token-32 #6. */
    if (alg != JWT_ALG_NONE) {
        if (pretty)
            output << "    ";

        output << "\"typ\":";
        if (pretty)
            output << " ";
        output << "\"JWT\",";

        if (pretty)
            output << std::endl;
    }

    if (pretty)
        output << "    ";

    output << "\"alg\":";
    if (pretty)
        output << " ";
    output << "\"" << JWT::alg_str(alg) << "\"";

    if (pretty)
        output << std::endl;

    output << "}";

    if (pretty)
        output << std::endl;
}

void JWT::write_body(std::ostream& output, int pretty) const
{
    grants.exportTo(output, pretty);
}

void JWT::sign(char** out, unsigned int* len, const char* str)
{
    switch (alg) {
        /* HMAC */
        case JWT::JWT_ALG_HS256:
        case JWT::JWT_ALG_HS384:
        case JWT::JWT_ALG_HS512:
            sign_sha_hmac(out, len, str);
            break;

            /* RSA */
        case JWT::JWT_ALG_RS256:
        case JWT::JWT_ALG_RS384:
        case JWT::JWT_ALG_RS512:

            /* ECC */
        case JWT::JWT_ALG_ES256:
        case JWT::JWT_ALG_ES384:
        case JWT::JWT_ALG_ES512:
            sign_sha_pem(out, len, str);
            break;

            /* You wut, mate? */
        default:
            throw Exception("Invalid algorithm");
    }
}

void JWT::encode(ostream& out)
{
    unsigned int sig_len;

    /* First the header. */
    stringstream header;
    write_head(header, false);

    string data(header.str());
    Buffer encodedHead;
    Base64::encode(encodedHead, data.c_str(), data.length());

    /* Now the body. */
    stringstream body;
    write_body(body, false);

    data = body.str();
    Buffer encodedBody;
    Base64::encode(encodedBody, data.c_str(), data.length());

    jwt_base64uri_encode(encodedHead);
    jwt_base64uri_encode(encodedBody);

    Buffer output(encodedHead);
    output.append('.');
    output.append(encodedBody);

    if (alg == JWT::JWT_ALG_NONE) {
        out << output.c_str() << '.';
        return;
    }

    /* Now the signature. */
    char* sig;
    sign(&sig, &sig_len, output.data());

    Buffer signature;
    Base64::encode(signature, sig, sig_len);
    jwt_base64uri_encode(signature);

    out << output.c_str() << '.' << signature.c_str();
}

void JWT::exportTo(ostream& output, int pretty) const
{
    write_head(output, pretty);
    output << ".";
    write_body(output, pretty);
}

void sptk::jwt_b64_decode(Buffer& destination, const char* src, int* ret_len)
{
    char *newData;
    size_t len, i, z;

    /* Decode based on RFC-4648 URI safe encoding. */
    len = strlen(src);
    newData = (char*) alloca(len + 4);

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

    Base64::decode(destination, newData);
    *ret_len = destination.bytes();
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

    buffer[t] = char(0);
    buffer.bytes(t);
}

void JWT::verify(const Buffer& head, const Buffer& sig)
{
    switch (alg) {
        /* HMAC */
        case JWT::JWT_ALG_HS256:
        case JWT::JWT_ALG_HS384:
        case JWT::JWT_ALG_HS512:
            verify_sha_hmac(head.c_str(), sig.c_str());
            break;

            /* RSA */
        case JWT::JWT_ALG_RS256:
        case JWT::JWT_ALG_RS384:
        case JWT::JWT_ALG_RS512:

            /* ECC */
        case JWT::JWT_ALG_ES256:
        case JWT::JWT_ALG_ES384:
        case JWT::JWT_ALG_ES512:
            verify_sha_pem(head.c_str(), sig.c_str());
            break;

            /* You wut, mate? */
        default:
            throw Exception("Unknown encryption algorithm");
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

    String val = JWT::get_js_string(js, "alg");
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

void JWT::decode(const char *token, const String& key)
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
    if (!key.empty())
        this->key = key;

    jwt_verify_head(this, head);
    jwt_parse_body(this, body);

    // Check the signature, if needed.
    if (this->alg != JWT::JWT_ALG_NONE) {
        // Re-add this since it's part of the verified data.
        //body[-1] = '.';
        head.append('.');
        head.append(body);
        verify(head, sig);
    }
}
