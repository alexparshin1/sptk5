/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
*/

#include <gtest/gtest.h>
#include <sptk5/xdoc/JsonSchema.h>
#include <stdexcept>

namespace sptk::xdoc {
namespace {

TEST(JsonSchemaTests, ValidatesOriginalJsonAndReportsViolation)
{
    if (!JsonSchema::available())
        GTEST_SKIP() << "SPTK_ENABLE_JSON_SCHEMA is disabled";

    const auto schema = JsonSchema::compile(R"({
        "type": "object",
        "properties": {
            "device_id": {"type": "string"},
            "temperature": {"type": "number"}
        },
        "required": ["device_id", "temperature"]
    })");
    EXPECT_TRUE(schema.validate(R"({"device_id":"s17","temperature":21.5})"));
    const auto invalid = schema.validate(R"({"device_id":"s17","temperature":"21.5"})");
    EXPECT_EQ(invalid.status, JsonSchema::Status::SchemaViolation);
    EXPECT_EQ(invalid.instancePath, "/temperature");
    EXPECT_EQ(invalid.keyword, "type");
    EXPECT_EQ(schema.validate(R"({"device_id":)").status, JsonSchema::Status::InvalidJson);
    EXPECT_EQ(schema.validate("/* comment */ {}").status, JsonSchema::Status::InvalidJson);
}

TEST(JsonSchemaTests, PreservesLargeIntegersAndResolvesLocalReferences)
{
    if (!JsonSchema::available())
        GTEST_SKIP() << "SPTK_ENABLE_JSON_SCHEMA is disabled";

    const auto schema = JsonSchema::compile(R"({
        "$defs": {"device": {"const": 9007199254740993}},
        "$ref": "#/$defs/device"
    })");
    EXPECT_TRUE(schema.validate("9007199254740993"));
    EXPECT_FALSE(schema.validate("9007199254740992"));
}

TEST(JsonSchemaTests, RejectsInvalidSchemasAndUnresolvedExternalReferences)
{
    if (!JsonSchema::available())
    {
        EXPECT_THROW(static_cast<void>(JsonSchema::compile("true")), std::logic_error);
        return;
    }

    EXPECT_THROW(static_cast<void>(JsonSchema::compile(R"({"required":1})")), std::invalid_argument);
    EXPECT_THROW(static_cast<void>(JsonSchema::compile(
        R"({"$schema":"http://json-schema.org/draft-07/schema#"})")), std::invalid_argument);
    EXPECT_THROW(static_cast<void>(JsonSchema::compile(
        R"({"$ref":"https://example.invalid/device.json"})")), std::invalid_argument);
}

} // namespace
} // namespace sptk::xdoc
