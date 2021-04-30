/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
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
*/

#include <sptk5/cxml>

using namespace std;
using namespace sptk;
using namespace xml;

void Entity::parse(const String& entityTag)
{
    static const RegularExpression matchEntity(R"(^<!ENTITY\s*(?<percent>\%)? (?<name>\S+)(?<type> SYSTEM| PUBLIC \w+)? (?<resource>[\w\._]+|".*"))");

    name = "";
    type = SYSTEM;
    id = "";
    resource = "";

    auto matches = matchEntity.m(entityTag);
    if (matches) {
        auto pc = matches["percent"].value;
        name = matches["name"].value;
        if (!pc.empty())
            name = "%" + name;

        auto typeAndId = matches["type"].value;
        if (typeAndId == " SYSTEM") {
            type = SYSTEM;
            id = "";
        }
        else if (typeAndId.startsWith(" PUBLIC ")) {
            type = PUBLIC;
            id = typeAndId.substr(8);
        }

        resource = matches["resource"].value;
        if (resource[0] == '"')
            resource = resource.substr(1, resource.length() - 2);
    }
}

xml::DocType::DocType(const char *name, const char *public_id, const char *system_id)
: m_name(name)
{
    if (public_id != nullptr)
        m_public_id = public_id;
    if (system_id != nullptr)
        m_system_id = system_id;
}

struct entity
{
    const char *name;
    int replacement_len;
    const char *replacement;
};

typedef map<std::string, const struct entity *> CEntityMap;

static const vector<entity> builtin_ent_xml = {
    { "amp", 1, "&" },
    { "lt", 1, "<" },
    { "gt", 1, ">" },
    { "apos", 1, "'" },
    { "quot", 1, "\"" },
    {nullptr, 1, "\"" }
};

static const char* xml_shortcut = "&<>'\"";

class CEntityCache
{
    CEntityMap              m_hash;
    map<int, CEntityMap>    m_replacementMaps;
public:

    explicit CEntityCache(const std::vector<entity>& entities) noexcept
    {
        for (const auto& ent: entities) {
            m_hash[ent.name] = &ent;
            m_replacementMaps[ent.replacement_len][ent.replacement] = &ent;
        }
    }

    [[nodiscard]] const struct entity *find(const std::string &ent) const
    {
        auto itor = m_hash.find(ent);
        if (itor != m_hash.end())
            return itor->second;
        return nullptr;
    }

    const struct entity *encode(const char* str) const;
};

const struct entity *CEntityCache::encode(const char* str) const
{
    auto maps = m_replacementMaps.begin();
    for (; maps != m_replacementMaps.end(); ++maps) {
        int len = maps->first;
        string fragment(str, size_t(len));
        const CEntityMap& replacements = maps->second;
        auto itor = replacements.find(fragment);
        if (itor != replacements.end())
            return itor->second;
    }
    return nullptr;
}

static const CEntityCache xml_entities(builtin_ent_xml);

void xml::DocType::decodeEntities(const char* str, size_t sz, Buffer& ret)
{
    Buffer buffer(str, sz);
    ret.bytes(0);

    char* start = buffer.data();
    char* ptr = start;
    while (*ptr != char(0)) {
        char* ent_start = strchr(ptr, '&');
        if (ent_start == nullptr)
            break;

        auto* ent_end = strchr(ent_start + 1, ';');
        if (ent_end != nullptr) {
            auto len = uint32_t(ent_start - start);
            if (len != 0)
                ret.append(start, len);
            ptr = appendDecodedEntity(ret, ent_start, ent_end);
            start = ptr;
        } else
            ++ptr;
    }
    ret.append(start, strlen(start));
}

char* xml::DocType::appendDecodedEntity(Buffer& ret, const char* ent_start, char* ent_end)
{
    char ch = *ent_end;
    *ent_end = 0;
    uint32_t replacementLength = 0;
    const char* rep = this->getReplacement(ent_start + 1, replacementLength);
    *ent_end = ch;
    if (rep != nullptr)
        ret.append(rep, replacementLength);
    else
        ret.append(ent_start, ent_end - ent_start);
    return ent_end + 1;
}

bool xml::DocType::encodeEntities(const char *str, Buffer& ret)
{
    bool replaced = false;

    const char* ptr = str;
    Buffer* src = &m_encodeBuffers[0];
    Buffer* dst = &m_encodeBuffers[1];
    dst->bytes(0);
    for (;;) {
        const char* pos = strpbrk(ptr, xml_shortcut);
        if (pos != nullptr) {
            auto index = uint32_t(strchr(xml_shortcut, *pos) - xml_shortcut);
            const auto& ent = builtin_ent_xml[index];
            auto tailBytes = uint32_t(pos - ptr);
            if (tailBytes != 0)
                dst->append(ptr, tailBytes);
            dst->append('&');
            dst->append(ent.name);
            dst->append(';');
            replaced = true;
            ptr = pos + 1;
        } else {
            if (ptr != str) {
                dst->append(ptr);
                ptr = dst->data();
                Buffer* tmp = dst;
                dst = src;
                src = tmp;
                dst->bytes(0);
            }
            break;
        }
    }

    if (!m_entities.empty()) {
        auto it = m_entities.begin();
        for (; it != m_entities.end(); ++it) {
            const string& val = it->second;
            auto len = (uint32_t) val.length();
            const char* pos = strstr(ptr, val.c_str());
            while (pos != nullptr) {
                dst->append(ptr, uint32_t(pos - ptr));
                dst->append('&');
                dst->append(it->first);
                dst->append(';');
                replaced = true;
                ptr = pos + len;
                pos = strstr(ptr, val.c_str());
            }
            dst->append(ptr);
            ptr = dst->data();
            Buffer* tmp = dst;
            dst = src;
            src = tmp;
            dst->bytes(0);
        }
    }

    if (replaced)
        ret.append(src->data(), src->bytes());
    else
        ret.append(ptr);

    return replaced;
}

const char* xml::DocType::getReplacement(const char *name, uint32_t& replacementLength)
{
    // &#123; style entity..
    if (*name == '#') {
        const char *ptr = name;
        ++ptr;

        if (*ptr == 'x' || *ptr == 'X')
            ++ptr;

        if (isdigit(*ptr) != 0) {
            constexpr int decimal = 10;
            m_replacementBuffer[0] = (char) strtol(ptr, nullptr, decimal);
            m_replacementBuffer[1] = '\0';
            replacementLength = 1;
            return m_replacementBuffer;
        }
    }

    const char* result {nullptr};

    // Find in built-ins, see entities.h
    const struct entity *entity = xml_entities.find(name);
    if (entity != nullptr) {
        replacementLength = uint32_t(entity->replacement_len);
        result = entity->replacement;
    } else {
        // Find in custom attributes
        auto itor = m_entities.find(name);
        if (itor != m_entities.end()) {
            const string& rep = itor->second;
            replacementLength = (uint32_t) rep.length();
            result = rep.c_str();
        }
    }

    return result;
}

bool xml::DocType::hasEntity(const char *name)
{
    uint32_t len = 0;
    const char* tmp = getReplacement(name, len);
    return tmp != nullptr;
}

#if USE_GTEST

TEST(SPTK_XmlDocType, parseEntity)
{
    Entity entity;

    entity.parse(R"(<!ENTITY file_pic SYSTEM "file.jpg" NDATA jpg>)");
    EXPECT_STREQ(entity.name.c_str(), "file_pic");
    EXPECT_EQ(entity.type, Entity::SYSTEM);
    EXPECT_STREQ(entity.resource.c_str(), "file.jpg");

    entity.parse(R"(<!ENTITY % lists "ul | ol")");
    EXPECT_STREQ(entity.name.c_str(), "%lists");
    EXPECT_EQ(entity.type, Entity::SYSTEM);
    EXPECT_STREQ(entity.resource.c_str(), "ul | ol");

    entity.parse(R"(<!ENTITY % lists PUBLIC list_id "ul | ol")");
    EXPECT_STREQ(entity.name.c_str(), "%lists");
    EXPECT_EQ(entity.type, Entity::PUBLIC);
    EXPECT_STREQ(entity.id.c_str(), "list_id");
    EXPECT_STREQ(entity.resource.c_str(), "ul | ol");
}

TEST(SPTK_XmlDocType, decodeEncodeEntities)
{
    String testString1("<'test1'> value");
    String testString2(R"(<v a='test1'>value</v>)");

    Buffer encoded;
    Buffer decoded;
    xml::DocType docType("x");

    docType.encodeEntities(testString1.c_str(), encoded);
    docType.decodeEntities(encoded.c_str(), (uint32_t) encoded.length(), decoded);
    EXPECT_STREQ(testString1.c_str(), decoded.c_str());

    encoded.reset();
    decoded.reset();

    docType.encodeEntities(testString2.c_str(), encoded);
    docType.decodeEntities(encoded.c_str(), (uint32_t) encoded.length(), decoded);
    EXPECT_STREQ(testString2.c_str(), decoded.c_str());
}

#endif
