/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            (C) 1999-2026 Alexey Parshin. All rights reserved.     ║
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

void JWT::set_alg(const Algorithm _alg, const String& _key)
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

const char* JWT::alg_str(const Algorithm _alg)
{
    switch (_alg)
    {
        using enum Algorithm;
        case NONE:
            return "none";
        case HS256:
            return "HS256";
        case HS384:
            return "HS384";
        case HS512:
            return "HS512";
        case RS256:
            return "RS256";
        case RS384:
            return "RS384";
        case RS512:
            return "RS512";
        case ES256:
            return "ES256";
        case ES384:
            return "ES384";
        case ES512:
            return "ES512";
        default:
            return nullptr;
    }
}

JWT::Algorithm JWT::str_alg(const char* alg)
{
    using enum Algorithm;
    static const map<String, Algorithm> algorithmInfo = {
        {"NONE", NONE},
        {"HS256", HS256},
        {"HS384", HS384},
        {"HS512", HS512},
        {"RS256", RS256},
        {"RS384", RS384},
        {"RS512", RS512},
        {"ES256", ES256},
        {"ES384", ES384},
        {"ES512", ES512}};

    if (alg == nullptr)
    {
        return INVAL;
    }

    const auto itor = algorithmInfo.find(upperCase(alg));
    if (itor == algorithmInfo.end())
    {
        return INVAL;
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
        return static_cast<long>(element->getNumber());
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

void JWT::write_head(std::ostream& output, const bool pretty) const
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
    output << "\"" << alg_str(alg) << "\"";

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

void JWT::write_body(std::ostream& output, const bool pretty) const
{
    grants.root()->exportTo(xdoc::DataFormat::JSON, output, pretty);
}

void JWT::sign(Buffer& out, const char* str) const
{
    using enum Algorithm;
    switch (alg)
    {
        /* HMAC */
        case HS256:
        case HS384:
        case HS512:
            sign_sha_hmac(out, str);
            break;

        /* RSA */
        case RS256:
        case RS384:
        case RS512:

        /* ECC */
        case ES256:
        case ES384:
        case ES512:
            sign_sha_pem(out, str);
            break;

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

    if (alg == Algorithm::NONE)
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

void JWT::exportTo(ostream& output, const bool pretty) const
{
    write_head(output, pretty);
    output << ".";
    write_body(output, pretty);
}

void sptk::jwt_b64_decode(Buffer& destination, const char* src)
{
    /* Decode based on RFC-4648 URI safe encoding. */
    const auto len = strlen(src);
    Buffer     newData_buffer(len + 4);
    auto*      newData = bit_cast<char*>(newData_buffer.data());

    size_t index = 0;
    for (; index < len; ++index)
    {
        switch (src[index])
        {
            case '-':
                newData[index] = '+';
                break;
            case '_':
                newData[index] = '/';
                break;
            default:
                newData[index] = src[index];
        }
    }

    if (auto trailingIndex = 4 - (index % 4);
        trailingIndex < 4)
    {
        while (--trailingIndex)
        {
            newData[index] = '=';
            ++index;
        }
    }
    newData[index] = '\0';

    Base64::decode(destination, newData);
}


static void jwt_b64_decode_json(xdoc::Document& dest, const Buffer& src)
{
    constexpr size_t bufferSize {1024};
    Buffer           decodedData(bufferSize);
    Base64::decode(decodedData, src);

    dest.load(decodedData.c_str());
}

void sptk::jwt_base64uri_encode(Buffer& buffer)
{
    auto*        str = bit_cast<char*>(buffer.data());
    const size_t len = strlen(str);
    size_t       outputIndex = 0;

    for (size_t i = 0; i < len; ++i)
    {
        switch (str[i])
        {
            case '+':
                str[outputIndex] = '-';
                ++outputIndex;
                break;
            case '/':
                str[outputIndex] = '_';
                ++outputIndex;
                break;
            case '=':
                break;
            default:
                str[outputIndex] = str[i];
                ++outputIndex;
                break;
        }
    }

    buffer[outputIndex] = static_cast<char>(0);
    buffer.bytes(outputIndex);
}

void JWT::verify(const Buffer& head, const Buffer& sig) const
{
    using enum Algorithm;
    switch (alg)
    {
        /* HMAC */
        case HS256:
        case HS384:
        case HS512:
            verify_sha_hmac(head.c_str(), sig.c_str());
            break;

        /* RSA */
        case RS256:
        case RS384:
        case RS512:

        /* ECC */
        case ES256:
        case ES384:
        case ES512:
            verify_sha_pem(head.c_str(), sig.c_str());
            break;

        default:
            throw Exception("Unknown encryption algorithm");
    }
}

static void jwt_parse_body(JWT* jwt, const Buffer& body)
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
        throw Exception("Invalid algorithm.");
    }

    if (jwt->alg != JWT::Algorithm::NONE)
    {
        /* If alg is not NONE, there may be a typ. */
        val = JWT::get_js_string(node, "typ");
        if (!val.empty() && val != "JWT")
        {
            throw Exception("Invalid algorithm name");
        }
    }
    else
    {
        /* If alg is NONE, there should not be a key */
        if (!jwt->key.empty())
        {
            throw Exception("Unexpected key.");
        }
    }
}

void JWT::decode(const char* token, const String& _key)
{
    struct Part
    {
        const char* data;
        size_t      length;
    };

    vector<Part> parts(3);

    size_t      index = 0;
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

    if (index < 2)
    {
        throw Exception("Invalid JWT data");
    }

    Buffer       head(bit_cast<const uint8_t*>(parts[0].data), parts[0].length);
    const Buffer body(bit_cast<const uint8_t*>(parts[1].data), parts[1].length);
    const Buffer sig(bit_cast<const uint8_t*>(parts[2].data), parts[2].length);

    // Now that we have everything split up, let's check out the header.

    // Copy the key over for verify_head.
    if (!_key.empty())
    {
        this->key = _key;
    }

    jwt_verify_head(this, head);
    jwt_parse_body(this, body);

    // Check the signature if the key is provided.
    if (this->alg != Algorithm::NONE && !key.empty())
    {
        // Re-add this since it's part of the verified data.
        head.append('.');
        head.append(body);
        verify(head, sig);
    }
}
