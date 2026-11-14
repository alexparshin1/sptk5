/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            (C) 1999-2026 Alexey Parshin                           ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
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
#include <sstream> // libc++
#include <string_view>

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

void JWT::sign(Buffer& token, const char* str) const
{
    using enum Algorithm;
    switch (alg)
    {
        /* HMAC */
        case HS256:
        case HS384:
        case HS512:
            sign_sha_hmac(token, str);
            break;

        /* RSA */
        case RS256:
        case RS384:
        case RS512:

        /* ECC */
        case ES256:
        case ES384:
        case ES512:
            sign_sha_pem(token, str);
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
    if (src == nullptr)
        throw Exception("Invalid base64url data");

    const std::string_view encoded(src);
    if (encoded.size() % 4 == 1)
        throw Exception("Invalid base64url data");

    const auto sextet = [](const char value) -> int
    {
        if (value >= 'A' && value <= 'Z') return value - 'A';
        if (value >= 'a' && value <= 'z') return value - 'a' + 26;
        if (value >= '0' && value <= '9') return value - '0' + 52;
        if (value == '-') return 62;
        if (value == '_') return 63;
        return -1;
    };

    std::string standard;
    standard.reserve(encoded.size() + 3);
    for (const char value : encoded)
    {
        const auto decoded = sextet(value);
        if (decoded < 0)
            throw Exception("Invalid base64url data");
        standard.push_back(value == '-' ? '+' : value == '_' ? '/' : value);
    }

    // RFC 4648 requires unused low bits in the last sextet to be zero. Reject alternate encodings
    // of the same bytes rather than inheriting the permissive SPTK Base64 decoder's behavior.
    if (encoded.size() % 4 == 2 && (sextet(encoded.back()) & 0x0f) != 0)
        throw Exception("Invalid base64url data");
    if (encoded.size() % 4 == 3 && (sextet(encoded.back()) & 0x03) != 0)
        throw Exception("Invalid base64url data");

    while (standard.size() % 4 != 0)
        standard.push_back('=');
    const Buffer standardBuffer(reinterpret_cast<const uint8_t*>(standard.data()), standard.size());
    Base64::decode(destination, standardBuffer);
}


static void jwt_b64_decode_json(xdoc::Document& dest, const Buffer& src)
{
    Buffer decodedData;
    jwt_b64_decode(decodedData, src.c_str());
    dest.clear();
    dest.load(decodedData);
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

static void jwt_verify_head(JWT* jwt, xdoc::Document& header, const Buffer& head,
                            const JWT::Algorithm expectedAlgorithm)
{
    jwt_b64_decode_json(header, head);
    const auto& node = header.root();

    const auto algNode = node->findFirst("alg", xdoc::SearchMode::ImmediateChild);
    String val = algNode && algNode->type() == xdoc::Node::Type::Text ? algNode->getString() : String{};
    jwt->alg = JWT::str_alg(val.c_str());
    if (jwt->alg == JWT::Algorithm::INVAL)
    {
        throw Exception("Invalid algorithm.");
    }
    if (expectedAlgorithm != JWT::Algorithm::INVAL && jwt->alg != expectedAlgorithm)
    {
        throw Exception("JWT algorithm does not match the expected algorithm");
    }

    if (jwt->alg != JWT::Algorithm::NONE)
    {
        /* If alg is not NONE, there may be a typ. */
        const auto typNode = node->findFirst("typ", xdoc::SearchMode::ImmediateChild);
        val = typNode && typNode->type() == xdoc::Node::Type::Text ? typNode->getString() : String{};
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
    decode(token, _key, Algorithm::INVAL);
}

void JWT::decode(const char* token, const String& _key, const Algorithm expectedAlgorithm)
{
    decode(token, _key, expectedAlgorithm, nullptr);
}

void JWT::decode(const char* token, const String& _key, xdoc::Document* decodedHeader)
{
    decode(token, _key, Algorithm::INVAL, decodedHeader);
}

void JWT::decode(const char* token, const String& _key, const Algorithm expectedAlgorithm,
                 xdoc::Document* decodedHeader)
{
    if (token == nullptr)
        throw Exception("Invalid JWT data");
    const char* firstDot = strchr(token, '.');
    const char* secondDot = firstDot ? strchr(firstDot + 1, '.') : nullptr;
    if (firstDot == nullptr || secondDot == nullptr || firstDot == token ||
        secondDot == firstDot + 1 || strchr(secondDot + 1, '.') != nullptr)
        throw Exception("Invalid JWT data");

    const auto headLength = static_cast<size_t>(firstDot - token);
    const auto bodyLength = static_cast<size_t>(secondDot - firstDot - 1);
    const auto signatureLength = strlen(secondDot + 1);
    Buffer head(bit_cast<const uint8_t*>(token), headLength);
    const Buffer body(bit_cast<const uint8_t*>(firstDot + 1), bodyLength);
    const Buffer sig(bit_cast<const uint8_t*>(secondDot + 1), signatureLength);

    // Now that we have everything split up, let's check out the header.

    // Copy the key over for verify_head.
    if (!_key.empty())
    {
        this->key = _key;
    }

    xdoc::Document localHeader;
    jwt_verify_head(this, decodedHeader ? *decodedHeader : localHeader, head, expectedAlgorithm);
    if ((alg == Algorithm::NONE) != (sig.bytes() == 0))
        throw Exception("JWT signature does not match its algorithm");
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
