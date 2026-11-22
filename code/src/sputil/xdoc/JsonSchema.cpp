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

#include <sptk5/xdoc/JsonSchema.h>

#include <stdexcept>
#include <utility>

#ifdef SPTK_HAVE_JSONCONS
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>
#endif

namespace sptk::xdoc {

#ifdef SPTK_HAVE_JSONCONS
namespace {
namespace schema = jsoncons::jsonschema;
using Json = jsoncons::json;
using CompiledSchema = schema::json_schema<Json>;

Json parse(std::string_view text)
{
    return Json::parse(text, jsoncons::json_options{}.allow_comments(false).max_nesting_depth(128));
}

JsonSchema::Result validateInstance(const CompiledSchema& compiled, const Json& instance)
{
    JsonSchema::Result result;
    compiled.validate(instance, [&result](const schema::validation_message& violation)
    {
        result = {JsonSchema::Status::SchemaViolation, violation.instance_location().string(),
                  violation.keyword(), violation.message()};
        return schema::walk_result::abort;
    });
    return result;
}
} // namespace

struct JsonSchema::Impl
{
    CompiledSchema schema;
};
#endif

JsonSchema::JsonSchema(std::shared_ptr<const Impl> impl)
    : m_impl(std::move(impl))
{
}

bool JsonSchema::available() noexcept
{
#ifdef SPTK_HAVE_JSONCONS
    return true;
#else
    return false;
#endif
}

JsonSchema JsonSchema::compile(std::string_view schemaJson)
{
#ifdef SPTK_HAVE_JSONCONS
    try
    {
        auto document = parse(schemaJson);
        const auto dialect = schema::schema_version::draft202012();
        if (document.is_object() && document.contains("$schema") &&
            document.at("$schema") != Json(dialect))
            throw std::invalid_argument("Only JSON Schema Draft 2020-12 is supported");

        static const auto metaSchema = schema::make_json_schema(
            schema::meta_resolver<Json>(jsoncons::uri(dialect)));
        if (const auto result = validateInstance(metaSchema, document); !result)
            throw std::invalid_argument("Invalid JSON Schema at " + result.instancePath + ": " + result.message);

        return JsonSchema(std::make_shared<Impl>(schema::make_json_schema(std::move(document))));
    }
    catch (const jsoncons::json_exception& error)
    {
        throw std::invalid_argument(error.what());
    }
#else
    (void)schemaJson;
    throw std::logic_error("SPTK was built without JSON Schema support");
#endif
}

JsonSchema::Result JsonSchema::validate(std::string_view payloadJson) const
{
#ifdef SPTK_HAVE_JSONCONS
    if (!m_impl)
        throw std::logic_error("JSON Schema has no compiled state");

    Json document;
    try
    {
        document = parse(payloadJson);
    }
    catch (const jsoncons::ser_error& error)
    {
        return {Status::InvalidJson, {}, {}, error.what()};
    }
    return validateInstance(m_impl->schema, document);
#else
    (void)payloadJson;
    throw std::logic_error("SPTK was built without JSON Schema support");
#endif
}

} // namespace sptk::xdoc
