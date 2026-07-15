/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
║  email                alexeyp@gmail.com                                      ║
║  code review          2026-05-10                                             ║
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

#include "sptk5/Base64.h"
#include "sptk5/net/RedisConnect.h"

using namespace std;
using namespace sptk;

namespace {
void bufferAppendCount(Buffer& buffer, size_t value)
{
    buffer.reserve(buffer.size() + 24);
    char* start = reinterpret_cast<char*>(buffer.data()) + buffer.size();
    auto* end = std::to_chars(start, start + 24, value).ptr;
    *end = 0;
    buffer.bytes(reinterpret_cast<uint8_t*>(end) - buffer.data());
}
} // namespace

RedisCommand::RedisCommand(string_view command, string_view mode)
    : Buffer(32)
{
    append('$');
    bufferAppendCount(*this, command.size());
    append("\r\n", 2);

    append(command.data(), command.size());
    append("\r\n", 2);
    ++m_count;

    if (!mode.empty())
    {
        append('$');
        bufferAppendCount(*this, mode.size());
        append("\r\n", 2);
        append(mode.data(), mode.size());
        append("\r\n", 2);
        ++m_count;
    }
}

void RedisCommand::emplace_back(string_view argument)
{
    append('$');
    bufferAppendCount(*this, argument.size());
    append("\r\n", 2);

    append(argument.data(), argument.size());
    append("\r\n", 2);
    ++m_count;
}

void RedisCommand::emplace_back(const std::vector<std::string>& arguments)
{
    for (const auto& argument: arguments)
    {
        emplace_back(argument);
    }
}

void RedisCommand::emplace_back(const Variant& argument)
{
    switch (argument.dataType())
    {
        using enum VariantDataType;

        case VAR_STRING:
        case VAR_TEXT:
        case VAR_BUFFER: {
            const string_view value {argument.getString(), argument.dataSize()};
            emplace_back(value);
            return;
        }

        case VAR_BOOL: {
            const string_view value = argument.asBool() ? "true" : "false";
            emplace_back(value);
            return;
        }

        case VAR_INT:
        case VAR_INT64: {
            array<char, 24>   stage {};
            const auto        end = to_chars(stage.data(), stage.data() + sizeof(stage), argument.asInt64()).ptr;
            const auto        size = static_cast<size_t>(end - stage.data());
            const string_view value {stage.data(), size};
            emplace_back(value);
            return;
        }

        case VAR_FLOAT: {
            array<char, 32>   stage {};
            const auto        end = to_chars(stage.data(), stage.data() + sizeof(stage), argument.asFloat()).ptr;
            const auto        size = static_cast<size_t>(end - stage.data());
            const string_view value {stage.data(), size};
            emplace_back(value);
            return;
        }

        case VAR_DATE: {
            array<char, 32>   stage {};
            const auto        result = format_to_n(stage.data(), sizeof(stage), "{:%F}", argument.asDate().timePoint());
            const auto        size = static_cast<size_t>(result.out - stage.data());
            const string_view value {stage.data(), size};
            emplace_back(value);
            return;
        }

        case VAR_DATE_TIME: {
            array<char, 40>   stage {};
            const auto        result = format_to_n(stage.data(), sizeof(stage), "{:%F %T}", argument.asDateTime().timePoint());
            const auto        size = static_cast<size_t>(result.out - stage.data());
            const string_view value {stage.data(), size};
            emplace_back(value);
            return;
        }

        case VAR_NONE:
            append("$1\r\n_\r\n", 7);
            ++m_count;
            return;

        default:
            throw Exception("Unsupported variant type");
    }
}

size_t RedisCommand::count() const
{
    return m_count;
}
