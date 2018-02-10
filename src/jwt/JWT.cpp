#include <sptk5/JWT.h>

using namespace std;
using namespace sptk;

JWT::JWT()
: alg(JWT_ALG_NONE), grants(true)
{
}

JWT* JWT::clone() const
{
    auto newJWT = new JWT;

    if (!key.empty()) {
        newJWT->alg = alg;
        newJWT->key = key;
    }

    Buffer tempBuffer;
    grants.exportTo(tempBuffer, false);
    newJWT->grants.load(tempBuffer.c_str());

    return newJWT;
}

String JWT::get_grant(const String& grant) const
{
    return get_js_string(&grants.root(), grant);
}

long JWT::get_grant_int(const String& grant) const
{
    return JWT::get_js_int(&grants.root(), grant);
}

bool JWT::get_grant_bool(const String& grant) const
{
    return JWT::get_js_bool(&grants.root(), grant);
}

String JWT::get_grants_json(const String& grant) const
{
    const json::Element *js_val = nullptr;

    if (!grant.empty())
        js_val = grants.root().find(grant);
    else
        js_val = &grants.root();

    if (js_val == nullptr)
        return "";

    std::stringstream output;
    js_val->exportTo(output, false);
    return output.str();
}

void JWT::add_grant(const String& grant, const String& val)
{
    bool exists;
    get_js_string(&grants.root(), grant, &exists);
    if (exists)
        throw Exception("Grant already exists");

    grants.root().add(grant, val);
}

void JWT::add_grant_int(const String& grant, long val)
{
    bool exists;
    get_js_int(&grants.root(), grant, &exists);
    if (exists)
        throw Exception("Grant already exists");

    grants.root().add(grant, double(val));
}

void JWT::add_grant_bool(const String& grant, bool val)
{
    bool exists;
    get_js_int(&grants.root(), grant, &exists);
    if (exists)
        throw Exception("Grant already exists");

    grants.root().add(grant, bool(val));
}

void JWT::del_grant(const String& grant)
{
    grants.root().remove(grant);
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
        return element->isBoolean() ? 1 : 0;
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

