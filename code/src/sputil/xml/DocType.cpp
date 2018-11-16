/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       DocType.cpp - description                              ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
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

xml::DocType::DocType(const char *name, const char *public_id, const char *system_id)
{
    m_name = name;
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

typedef map<std::string, struct entity *> CEntityMap;

static struct entity builtin_ent_xml[] = {
        { "amp", 1, "&" },
        { "lt", 1, "<" },
        { "gt", 1, ">" },
        { "apos", 1, "'" },
        { "quot", 1, "\"" },
        {nullptr, 1, "\"" }
};

const char xml_shortcut[] = "&<>'\"";

class CEntityCache
{
    CEntityMap              m_hash;
    map<int, CEntityMap>    m_replacementMaps;
public:

    explicit CEntityCache(struct entity entities[]) noexcept
    {
        struct entity *ent = entities;
        for (; ent->name != nullptr; ent++) {
            m_hash[ent->name] = ent;
            m_replacementMaps[ent->replacement_len][ent->replacement] = ent;
        }
    }

    const struct entity *find(const std::string &ent) const
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

void xml::DocType::decodeEntities(const char* str, uint32_t sz, Buffer& ret)
{
    ret.bytes(0);

    const char* start = str;
    const char* ptr = str;
    while (*ptr != char(0)) {
        const char* ent_start = strchr(ptr, '&');
        if (ent_start != nullptr) {
            auto ent_end = (char*) strchr(ent_start + 1, ';');
            if (ent_end != nullptr) {
                char ch = *ent_end;
                *ent_end = 0;
                uint32_t replacementLength = 0;
                const char* rep = getReplacement(ent_start + 1, replacementLength);
                *ent_end = ch;
                if (rep != nullptr) {
                    auto len = uint32_t(ent_start - start);
                    if (len != 0)
                        ret.append(start, len);
                    ptr = ent_end + 1;
                    start = ptr;
                    ret.append(rep, replacementLength);
                } else
                    ptr++;
            } else {
                ptr++;
            }
        } else {
            sz = (uint32_t) strlen(start);
            break;
        }
    }
    ret.append(start, sz);
}

bool xml::DocType::encodeEntities(const char *str, Buffer& ret)
{
    entity* table = builtin_ent_xml;

    bool replaced = false;

    const char* ptr = str;
    Buffer* src = &m_encodeBuffers[0];
    Buffer* dst = &m_encodeBuffers[1];
    dst->bytes(0);
    for (;;) {
        const char* pos = strpbrk(ptr, xml_shortcut);
        if (pos != nullptr) {
            auto index = uint32_t(strchr(xml_shortcut, *pos) - xml_shortcut);
            entity* ent = table + index;
            auto tailBytes = uint32_t(pos - ptr);
            if (tailBytes != 0)
                dst->append(ptr, tailBytes);
            dst->append('&');
            dst->append(ent->name);
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
            std::string& val = it->second;
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
                if (pos == nullptr) {
                    dst->append(ptr);
                    ptr = dst->data();
                    Buffer* tmp = dst;
                    dst = src;
                    src = tmp;
                    dst->bytes(0);
                }
            }
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
    if (name[0] == '#') {
        if (isdigit(name[1]) != 0) {
            m_replacementBuffer[0] = (char) strtol(name + 1, nullptr, 10);
            m_replacementBuffer[1] = '\0';
            replacementLength = 1;
            return m_replacementBuffer;
        }
        if (name[1] == 'x' || name[1] == 'X') {
            m_replacementBuffer[0] = (char) strtol(name + 2, nullptr, 16);
            m_replacementBuffer[1] = '\0';
            replacementLength = 1;
            return m_replacementBuffer;
        }
    }

    // Find in built-ins, see entities.h
    const struct entity *entity = xml_entities.find(name);
    if (entity != nullptr) {
        replacementLength = uint32_t(entity->replacement_len);
        return entity->replacement;
    }

    // Find in custom attributes
    Entities::const_iterator itor = m_entities.find(name);
    if (itor != m_entities.end()) {
        const string& rep = itor->second;
        replacementLength = (uint32_t) rep.length();
        return rep.c_str();
    }

    return nullptr;
}

bool xml::DocType::hasEntity(const char *name)
{
    uint32_t len;
    const char* tmp = getReplacement(name, len);
    return tmp != nullptr;
}
