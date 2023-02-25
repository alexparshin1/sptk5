/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            (C) 1999-2023 Alexey Parshin. All rights reserved.     ║
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

#include <sptk5/Base64.h>
#include <sptk5/JWT.h>

using namespace std;
using namespace sptk;

JWT::Algorithm JWT::get_alg() const
{
    return alg;
}

void JWT::set_alg(Algorithm _alg, const String& _key)
{
    if (_alg == Algorithm::NONE)
    {
        if (!_key.empty())
        {
            throw Exception("Key is not expected here");
        }
    }
    else
    {
        if (_key.empty())
        {
            throw Exception("Empty key is not expected here");
        }
    }

    key = _key;
    alg = _alg;
}

const char* JWT::alg_str(Algorithm _alg)
{
    switch (_alg)
    {
        case Algorithm::NONE:
            return "none";
        case Algorithm::HS256:
            return "HS256";
        case Algorithm::HS384:
            return "HS384";
        case Algorithm::HS512:
            return "HS512";
        case Algorithm::RS256:
            return "RS256";
        case Algorithm::RS384:
            return "RS384";
        case Algorithm::RS512:
            return "RS512";
        case Algorithm::ES256:
            return "ES256";
        case Algorithm::ES384:
            return "ES384";
        case Algorithm::ES512:
            return "ES512";
        default:
            return nullptr;
    }
}

JWT::Algorithm JWT::str_alg(const char* alg)
{
    static const map<String, Algorithm> algorithmInfo = {
        {"NONE", Algorithm::NONE},
        {"HS256", Algorithm::HS256},
        {"HS384", Algorithm::HS384},
        {"HS512", Algorithm::HS512},
        {"RS256", Algorithm::RS256},
        {"RS384", Algorithm::RS384},
        {"RS512", Algorithm::RS512},
        {"ES256", Algorithm::ES256},
        {"ES384", Algorithm::ES384},
        {"ES512", Algorithm::ES512}};

    if (alg == nullptr)
    {
        return Algorithm::INVAL;
    }

    auto itor = algorithmInfo.find(upperCase(alg));
    if (itor == algorithmInfo.end())
    {
        return Algorithm::INVAL;
    }
    return itor->second;
}

xdoc::SNode JWT::find_grant(const xdoc::SNode& node, const String& key)
{
    if (node->type() == xdoc::Node::Type::Object)
    {
        return node->findFirst(key);
    }
    return nullptr;
}

String JWT::get_js_string(const xdoc::SNode& node, const String& key, bool* found)
{
    if (found)
    {
        *found = false;
    }

    if (const auto& element = find_grant(node, key);
        element != nullptr && element->type() == xdoc::Node::Type::Text)
    {
        if (found)
        {
            *found = true;
        }
        return element->getString();
    }
    return {};
}

long JWT::get_js_int(const xdoc::SNode& node, const String& key, bool* found)
{
    if (found)
    {
        *found = false;
    }

    if (const auto& element = find_grant(node, key);
        element != nullptr && element->type() == xdoc::Node::Type::Number)
    {
        if (found)
        {
            *found = true;
        }
        return (long) element->getNumber();
    }
    return 0;
}

bool JWT::get_js_bool(const xdoc::SNode& node, const String& key, bool* found)
{
    if (found)
    {
        *found = false;
    }

    if (const auto& element = find_grant(node, key);
        element != nullptr && element->type() == xdoc::Node::Type::Boolean)
    {
        if (found)
        {
            *found = true;
        }
        return element->getBoolean();
    }
    return false;
}

void JWT::write_head(std::ostream& output, bool pretty) const
{
    output << "{";

    if (pretty)
    {
        output << std::endl;
    }

    /* An unsecured JWT is a JWS and provides no "typ".
     * -- draft-ietf-oauth-json-web-token-32 #6. */
    if (alg != Algorithm::NONE)
    {
        if (pretty)
        {
            output << "    ";
        }

        output << "\"typ\":";
        if (pretty)
        {
            output << " ";
        }
        output << "\"JWT\",";

        if (pretty)
        {
            output << std::endl;
        }
    }

    if (pretty)
    {
        output << "    ";
    }

    output << "\"alg\":";
    if (pretty)
    {
        output << " ";
    }
    output << "\"" << JWT::alg_str(alg) << "\"";

    if (pretty)
    {
        output << std::endl;
    }

    output << "}";

    if (pretty)
    {
        output << std::endl;
    }
}

void JWT::write_body(std::ostream& output, bool pretty) const
{
    grants.root()->exportTo(xdoc::DataFormat::JSON, output, pretty);
}

void JWT::sign(Buffer& out, const char* str) const
{
    switch (alg)
    {
        /* HMAC */
        case JWT::Algorithm::HS256:
        case JWT::Algorithm::HS384:
        case JWT::Algorithm::HS512:
            sign_sha_hmac(out, str);
            break;

            /* RSA */
        case JWT::Algorithm::RS256:
        case JWT::Algorithm::RS384:
        case JWT::Algorithm::RS512:

            /* ECC */
        case JWT::Algorithm::ES256:
        case JWT::Algorithm::ES384:
        case JWT::Algorithm::ES512:
            sign_sha_pem(out, str);
            break;

            /* You wut, mate? */
        default:
            throw Exception("Invalid algorithm");
    }
}

void JWT::encode(ostream& out) const
{
    /* First the header. */
    stringstream header;
    write_head(header, false);

    Buffer data(header.str());
    Buffer encodedHead;
    Base64::encode(encodedHead, data.data(), data.bytes());

    /* Now the body. */
    stringstream body;
    write_body(body, false);

    data = body.str();
    Buffer encodedBody;
    Base64::encode(encodedBody, data.data(), data.bytes());

    jwt_base64uri_encode(encodedHead);
    jwt_base64uri_encode(encodedBody);

    Buffer output(encodedHead);
    output.append('.');
    output.append(encodedBody);

    if (alg == JWT::Algorithm::NONE)
    {
        out << output.c_str() << '.';
        return;
    }

    /* Now the signature. */
    Buffer sig;
    sign(sig, output.c_str());

    Buffer signature;
    Base64::encode(signature, sig);
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
    /* Decode based on RFC-4648 URI safe encoding. */
    auto len = strlen(src);
    Buffer newData_buffer(len + 4);
    auto* newData = reinterpret_cast<char*>(newData_buffer.data());

    size_t i = 0;
    for (; i < len; ++i)
    {
        switch (src[i])
        {
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
    auto z = 4 - (i % 4);
    if (z < 4)
    {
        while (--z)
        {
            newData[i] = '=';
            ++i;
        }
    }
    newData[i] = '\0';

    Base64::decode(destination, newData);
}


static void jwt_b64_decode_json(const xdoc::Document& dest, const Buffer& src)
{
    constexpr size_t bufferSize {1024};
    Buffer decodedData(bufferSize);
    Base64::decode(decodedData, src);

    dest.load(decodedData.c_str());
}

void sptk::jwt_base64uri_encode(Buffer& buffer)
{
    auto* str = reinterpret_cast<char*>(buffer.data());
    size_t len = strlen(str);
    size_t t = 0;

    for (size_t i = 0; i < len; ++i)
    {
        switch (str[i])
        {
            case '+':
                str[t] = '-';
                ++t;
                break;
            case '/':
                str[t] = '_';
                ++t;
                break;
            case '=':
                break;
            default:
                str[t] = str[i];
                ++t;
                break;
        }
    }

    buffer[t] = char(0);
    buffer.bytes(t);
}

void JWT::verify(const Buffer& head, const Buffer& sig) const
{
    switch (alg)
    {
        /* HMAC */
        case JWT::Algorithm::HS256:
        case JWT::Algorithm::HS384:
        case JWT::Algorithm::HS512:
            verify_sha_hmac(head.c_str(), sig.c_str());
            break;

            /* RSA */
        case JWT::Algorithm::RS256:
        case JWT::Algorithm::RS384:
        case JWT::Algorithm::RS512:

            /* ECC */
        case JWT::Algorithm::ES256:
        case JWT::Algorithm::ES384:
        case JWT::Algorithm::ES512:
            verify_sha_pem(head.c_str(), sig.c_str());
            break;

            /* You wut, mate? */
        default:
            throw Exception("Unknown encryption algorithm");
    }
}

static void jwt_parse_body(const JWT* jwt, const Buffer& body)
{
    jwt_b64_decode_json(jwt->grants, body);
}

static void jwt_verify_head(JWT* jwt, const Buffer& head)
{
    xdoc::Document jsdoc;
    jwt_b64_decode_json(jsdoc, head);
    const auto& node = jsdoc.root();

    String val = JWT::get_js_string(node, "alg");
    jwt->alg = JWT::str_alg(val.c_str());
    if (jwt->alg == JWT::Algorithm::INVAL)
    {
        throw Exception("Invalid algorithm");
    }

    if (jwt->alg != JWT::Algorithm::NONE)
    {
        /* If alg is not NONE, there may be a typ. */
        val = JWT::get_js_string(node, "typ");
        if (val != "JWT")
        {
            throw Exception("Invalid algorithm name");
        }

        if (jwt->key.empty())
        {
            jwt->alg = JWT::Algorithm::NONE;
        }
    }
    else
    {
        /* If alg is NONE, there should not be a key */
        if (!jwt->key.empty())
        {
            throw Exception("Unexpected key");
        }
    }
}

void JWT::decode(const char* token, const String& _key)
{
    struct Part {
        const char* data;
        size_t length;
    };

    vector<Part> parts(3);

    size_t index = 0;
    const char* data = token;
    while (data != nullptr && index < 3)
    {
        parts[index].data = data;
        const char* end = strchr(data, '.');
        if (end == nullptr)
        {
            parts[index].length = strlen(data);
            break;
        }
        parts[index].length = end - data;
        data = end + 1;
        ++index;
    }

    if (parts[1].data == nullptr)
    {
        throw Exception("Invalid JWT data");
    }

    Buffer head(reinterpret_cast<const uint8_t*>(parts[0].data), parts[0].length);
    Buffer body(reinterpret_cast<const uint8_t*>(parts[1].data), parts[1].length);
    Buffer sig(reinterpret_cast<const uint8_t*>(parts[2].data), parts[2].length);

    // Now that we have everything split up, let's check out the header.

    // Copy the key over for verify_head.
    if (!_key.empty())
    {
        this->key = _key;
    }

    jwt_verify_head(this, head);
    jwt_parse_body(this, body);

    // Check the signature, if needed.
    if (this->alg != JWT::Algorithm::NONE)
    {
        // Re-add this since it's part of the verified data.
        head.append('.');
        head.append(body);
        verify(head, sig);
    }
}
