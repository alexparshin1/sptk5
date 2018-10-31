/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       JWT.cpp - description                                  ║
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
    if (alg == nullptr) {
        return JWT_ALG_INVAL;
    }

	String algUC = upperCase(alg);
    if (algUC == "NONE")
        return JWT_ALG_NONE;
    else if (algUC == "HS256")
        return JWT_ALG_HS256;
    else if (algUC == "HS384")
        return JWT_ALG_HS384;
    else if (algUC == "HS512")
        return JWT_ALG_HS512;
    else if (algUC == "RS256")
        return JWT_ALG_RS256;
    else if (algUC == "RS384")
        return JWT_ALG_RS384;
    else if (algUC == "RS512")
        return JWT_ALG_RS512;
    else if (algUC == "ES256")
        return JWT_ALG_ES256;
    else if (algUC == "ES384")
        return JWT_ALG_ES384;
    else if (algUC == "ES512")
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
        return (long) element->getNumber();
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

void JWT::write_head(std::ostream& output, bool pretty) const
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

void JWT::write_body(std::ostream& output, bool pretty) const
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

void JWT::exportTo(ostream& output, bool pretty) const
{
    write_head(output, pretty);
    output << ".";
    write_body(output, pretty);
}

void sptk::jwt_b64_decode(Buffer& destination, const char* src)
{
    char *newData;
    size_t len, i, z;

    /* Decode based on RFC-4648 URI safe encoding. */
    len = strlen(src);
    Buffer newData_buffer(len + 4);
    newData = newData_buffer.data();

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
}


static void jwt_b64_decode_json(json::Document &dest, const Buffer &src)
{
    Buffer decodedData(1024);
    Base64::decode(decodedData, src);

    dest.load(decodedData.c_str());
}

void sptk::jwt_base64uri_encode(Buffer& buffer)
{
    char* str = buffer.data();
    size_t len = strlen(str);
    size_t i, t;

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

#if USE_GTEST
#include <gtest/gtest.h>

TEST(SPTK_JWT, dup)
{
    time_t now;
    int valint;

    JWT jwt;

    jwt["iss"] = "test";
    String val = jwt["iss"];
    EXPECT_FALSE(val.empty()) << "Can't get grant for first JWT";

    JWT newJWT(jwt);
    val = newJWT["iss"];
    EXPECT_FALSE(val.empty()) << "Can't get grant for second JWT";

    EXPECT_STREQ("test", val.c_str()) << "Got incorrect grant";
    EXPECT_EQ(JWT::JWT_ALG_NONE, jwt.get_alg()) << "Got incorrect alogorithm";

    now = time(nullptr);
    jwt["iat"] = (int) now;

    valint = jwt["iat"];
    EXPECT_EQ((long)now, valint) << "Failed jwt_get_grant_int()";
}

TEST(SPTK_JWT, dup_signed)
{
    String key256("012345678901234567890123456789XY");

    JWT jwt;
    jwt["iss"] = "test";
    jwt.set_alg(JWT::JWT_ALG_HS256, key256);

    JWT newJWT(jwt);
    String val = newJWT["iss"];
    EXPECT_STREQ("test", val.c_str()) << "Failed jwt_get_grant_int()";
    EXPECT_EQ(JWT::JWT_ALG_HS256, jwt.get_alg()) << "Failed jwt_get_alg()";
}


TEST(SPTK_JWT, decode)
{
    const char token[] =
            "eyJhbGciOiJub25lIn0.eyJpc3MiOiJmaWxlcy5jeXBo"
            "cmUuY29tIiwic3ViIjoidXNlcjAifQ.";
    JWT::jwt_alg_t alg;

    auto jwt = new JWT;

    EXPECT_NO_THROW(jwt->decode(token)) << "Failed jwt_decode()";
    alg = jwt->get_alg();
    EXPECT_EQ(JWT::JWT_ALG_NONE, alg) << "Failed jwt_get_alg()";

    delete jwt;
}


TEST(SPTK_JWT, decode_invalid_final_dot)
{
    const char token[] = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzM4NCJ9."
                         "eyJpc3MiOiJmaWxlcy5jeXBocmUuY29tIiwic"
                         "3ViIjoidXNlcjAifQ";

    auto jwt = new JWT;
    EXPECT_NO_THROW(jwt->decode(token)) << "Not failed jwt_decode()";

    delete jwt;
}


TEST(SPTK_JWT, decode_invalid_alg)
{
    const char token[] = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIQUhBSCJ9."
                         "eyJpc3MiOiJmaWxlcy5jeXBocmUuY29tIiwic"
                         "3ViIjoidXNlcjAifQ.";

    auto jwt = new JWT;
    EXPECT_THROW(jwt->decode(token), Exception) << "Not failed jwt_decode()";

    delete jwt;
}


TEST(SPTK_JWT, decode_invalid_typ)
{
    const char token[] = "eyJ0eXAiOiJBTEwiLCJhbGciOiJIUzI1NiJ9."
                         "eyJpc3MiOiJmaWxlcy5jeXBocmUuY29tIiwic"
                         "3ViIjoidXNlcjAifQ.";

    auto jwt = new JWT;
    EXPECT_THROW(jwt->decode(token), Exception) << "Not failed jwt_decode()";

    delete jwt;
}


TEST(SPTK_JWT, decode_invalid_head)
{
    const char token[] =
            "yJ0eXAiOiJKV1QiLCJhbGciOiJIUzM4NCJ9."
            "eyJpc3MiOiJmaWxlcy5jeXBocmUuY29tIiwic"
            "3ViIjoidXNlcjAifQ.";

    auto jwt = new JWT;
    EXPECT_THROW(jwt->decode(token), Exception) << "Not failed jwt_decode()";

    delete jwt;
}


TEST(SPTK_JWT, decode_alg_none_with_key)
{
    const char token[] =
            "eyJhbGciOiJub25lIn0."
            "eyJpc3MiOiJmaWxlcy5jeXBocmUuY29tIiwic"
            "3ViIjoidXNlcjAifQ.";

    auto jwt = new JWT;
    EXPECT_NO_THROW(jwt->decode(token)) << "Not failed jwt_decode()";

    delete jwt;
}


TEST(SPTK_JWT, decode_invalid_body)
{
    const char token[] =
            "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzM4NCJ9."
            "eyJpc3MiOiJmaWxlcy5jeBocmUuY29tIiwic"
            "3ViIjoidXNlcjAifQ.";

    auto jwt = new JWT;
    EXPECT_THROW(jwt->decode(token), Exception) << "Not failed jwt_decode()";

    delete jwt;
}


TEST(SPTK_JWT, decode_hs256)
{
    const char token[] =
            "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJpc3Mi"
            "OiJmaWxlcy5jeXBocmUuY29tIiwic3ViIjoidXNlcjAif"
            "Q.dLFbrHVViu1e3VD1yeCd9aaLNed-bfXhSsF0Gh56fBg";
    String key256("012345678901234567890123456789XY");

    auto jwt = new JWT;
    EXPECT_NO_THROW(jwt->decode(token, key256)) << "Failed jwt_decode()";

    delete jwt;
}


TEST(SPTK_JWT, decode_hs384)
{
    const char token[] =
            "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzM4NCJ9."
            "eyJpc3MiOiJmaWxlcy5jeXBocmUuY29tIiwic"
            "3ViIjoidXNlcjAifQ.xqea3OVgPEMxsCgyikr"
            "R3gGv4H2yqMyXMm7xhOlQWpA-NpT6n2a1d7TD"
            "GgU6LOe4";
    String key384(
            "aaaabbbbccccddddeeeeffffg"
            "ggghhhhiiiijjjjkkkkllll");

    auto jwt = new JWT;
    EXPECT_NO_THROW(jwt->decode(token, key384)) << "Failed jwt_decode()";

    delete jwt;
}


TEST(SPTK_JWT, decode_hs512)
{
    const char token[] =
            "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzUxMiJ9.eyJpc3Mi"
            "OiJmaWxlcy5jeXBocmUuY29tIiwic3ViIjoidXNlcjAif"
            "Q.u-4XQB1xlYV8SgAnKBof8fOWOtfyNtc1ytTlc_vHo0U"
            "lh5uGT238te6kSacnVzBbC6qwzVMT1806oa1Y8_8EOg";
    String key512(
            "012345678901234567890123456789XY"
            "012345678901234567890123456789XY");

    auto jwt = new JWT;
    EXPECT_NO_THROW(jwt->decode(token, key512)) << "Failed jwt_decode()";

    delete jwt;
}

TEST(SPTK_JWT, encode_hs256_decode)
{
    String key256("012345678901234567890123456789XY");

    JWT jwt;
    jwt.set_alg(JWT::JWT_ALG_HS256, key256);

    jwt["iat"] = (int) time(nullptr);
    jwt["iss"] = "http://test.com";
    jwt["exp"] = (int) time(nullptr) + 86400;

    auto info = jwt.grants.root().set_object("info");
    info->set("company", "Linotex");
    info->set("city", "Melbourne");

    stringstream originalToken;
    jwt.encode(originalToken);

    stringstream originalJSON;
    jwt.exportTo(originalJSON, false);

    JWT jwt2;
    jwt2.decode(originalToken.str().c_str(), key256);

    stringstream copiedJSON;
    jwt2.exportTo(copiedJSON, false);

    json::Document original, copied;
    Buffer originalBuffer, copiedBuffer;

    original.load(originalJSON.str());
    original.exportTo(originalBuffer);
    copied.load(copiedJSON.str());
    copied.exportTo(copiedBuffer);
    EXPECT_STREQ(originalBuffer.c_str(), copiedBuffer.c_str()) << "Decoded JSON payload doesn't match the original";
}

#endif
