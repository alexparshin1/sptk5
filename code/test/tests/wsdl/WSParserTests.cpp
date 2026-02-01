/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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

#include <gtest/gtest.h>

#include <sptk5/Exception.h>
#include <sptk5/wsdl/WSParser.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>

using namespace std;
using namespace sptk;

namespace {

filesystem::path makeUniqueTempDir(const string& prefix)
{
    const auto base = filesystem::temp_directory_path();
    const auto ts = chrono::duration_cast<chrono::microseconds>(
                        chrono::system_clock::now().time_since_epoch())
                        .count();
    const auto tidHash = hash<thread::id> {}(this_thread::get_id());

    filesystem::path dir = base / format("{}_{}_{}", prefix, ts, tidHash);
    filesystem::create_directories(dir);
    return dir;
}

void writeTextFile(const filesystem::path& file, string_view content)
{
    ofstream out(file, ios::binary);
    ASSERT_TRUE(out.good()) << "Can't open file for write: " << file.string();
    out.write(content.data(), static_cast<std::streamsize>(content.size()));
    out.close();
    ASSERT_TRUE(out.good()) << "Write failed: " << file.string();
}

string readTextFile(const filesystem::path& file)
{
    ifstream in(file, ios::binary);
    if (!in.good())
    {
        return {};
    }
    stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

class ScopedCurrentPath
{
public:
    explicit ScopedCurrentPath(const filesystem::path& newPath)
        : m_old(filesystem::current_path())
    {
        filesystem::current_path(newPath);
    }

    ScopedCurrentPath(const ScopedCurrentPath&) = delete;
    ScopedCurrentPath& operator=(const ScopedCurrentPath&) = delete;

    ~ScopedCurrentPath()
    {
        try
        {
            filesystem::current_path(m_old);
        }
        catch (...)
        {
            // best-effort
        }
    }

private:
    filesystem::path m_old;
};

filesystem::path locateTestWsdlFile()
{
    // Look for test/wsdl/Test.wsdl relative to the current work dir (build dir / repo root variability).
    const vector candidates = {
        filesystem::path("test/wsdl/Test.wsdl"),
        filesystem::path("../test/wsdl/Test.wsdl"),
        filesystem::path("../../test/wsdl/Test.wsdl"),
        filesystem::path("wsdl/Test.wsdl"),
        filesystem::path("../wsdl/Test.wsdl"),
    };

    for (const auto& rel: candidates)
    {
        auto p = filesystem::absolute(rel);
        if (filesystem::exists(p))
        {
            return p;
        }
    }

    // Fallback: climb up a few directories and try again.
    auto cur = filesystem::current_path();
    for (int i = 0; i < 6; ++i)
    {
        if (auto p = cur / "test/wsdl/Test.wsdl";
            filesystem::exists(p))
        {
            return p;
        }
        if (!cur.has_parent_path())
        {
            break;
        }
        cur = cur.parent_path();
    }

    return {};
}

} // namespace

TEST(SPTK_WSParser, parse_and_generate_from_repo_test_wsdl)
{
    const auto wsdlFile = locateTestWsdlFile();
    ASSERT_FALSE(wsdlFile.empty()) << "Couldn't locate test/wsdl/Test.wsdl";

    const auto outDir = makeUniqueTempDir("sptk_wsparser_repo_wsdl");

    WSParser parser;
    ASSERT_NO_THROW(parser.parse(wsdlFile));
    ASSERT_NO_THROW(parser.generate(outDir.string().c_str(), "", OpenApiGenerator::Options(), false, "test_service_ns"));

    const auto header = outDir / "CTestServiceBase.h";
    ASSERT_TRUE(filesystem::exists(header)) << header.string();

    const auto headerText = readTextFile(header);
    ASSERT_FALSE(headerText.empty());

    // Operations exist
    EXPECT_NE(headerText.find("virtual void Hello("), string::npos);
    EXPECT_NE(headerText.find("virtual void Login("), string::npos);
    EXPECT_NE(headerText.find("virtual void AccountBalance("), string::npos);

    // Basic signature sanity (input/output types should be based on element names)
    EXPECT_NE(headerText.find("const CHello& input"), string::npos);
    EXPECT_NE(headerText.find("CHelloResponse& output"), string::npos);
}

TEST(SPTK_WSParser, generate_creates_expected_artifacts_and_is_repeatable)
{
    const auto wsdlFile = locateTestWsdlFile();
    ASSERT_FALSE(wsdlFile.empty()) << "Couldn't locate test/wsdl/Test.wsdl";

    const auto        sandbox = makeUniqueTempDir("sptk_wsparser_generate");
    ScopedCurrentPath cwd(sandbox);

    WSParser parser;
    ASSERT_NO_THROW(parser.parse(wsdlFile));

    OpenApiGenerator::Options options;
    options.openApiFile = (sandbox / "openapi_test.json");

    // Generate into CWD (sandbox) so we can check the generated <service>.inc here too.
    ASSERT_NO_THROW(parser.generate(".", "", options, false, "wsparser_test_ns"));

    // Service base class
    EXPECT_TRUE(filesystem::exists(sandbox / "CTestServiceBase.h"));
    EXPECT_TRUE(filesystem::exists(sandbox / "CTestServiceBase.cpp"));

    // Auto-generated shared header
    EXPECT_TRUE(filesystem::exists(sandbox / "CommonHeaders.h"));

    // Generated cmake include list and OpenAPI
    EXPECT_TRUE(filesystem::exists(sandbox / "Test.inc"));
    EXPECT_TRUE(filesystem::exists(options.openApiFile));

    // Repeat generation should be deterministic (important for CI + clean diffs).
    const auto incBefore = readTextFile(sandbox / "Test.inc");
    const auto hdrBefore = readTextFile(sandbox / "CTestServiceBase.h");
    ASSERT_FALSE(incBefore.empty());
    ASSERT_FALSE(hdrBefore.empty());

    ASSERT_NO_THROW(parser.generate(".", "", options, false, "wsparser_test_ns"));

    const auto incAfter = readTextFile(sandbox / "Test.inc");
    const auto hdrAfter = readTextFile(sandbox / "CTestServiceBase.h");

    EXPECT_EQ(incAfter, incBefore);
    EXPECT_EQ(hdrAfter, hdrBefore);
}

TEST(SPTK_WSParser, output_message_name_can_differ_from_output_element_name)
{
    // Validates that the output type is resolved from wsdl:message/wsdl:part/@element (FooResponse),
    // not from the message name (FooOutMsg).
    const string wsdl = R"(<?xml version="1.0" encoding="UTF-8"?>
        <wsdl:definitions
          xmlns:soap="http://schemas.xmlsoap.org/wsdl/soap/"
          xmlns:tns="http://example.org/Foo/"
          xmlns:wsdl="http://schemas.xmlsoap.org/wsdl/"
          xmlns:xsd="http://www.w3.org/2001/XMLSchema"
          name="Foo"
          targetNamespace="http://example.org/Foo/">

          <wsdl:types>
            <xsd:schema targetNamespace="http://example.org/Foo/">
              <xsd:element name="Foo">
                <xsd:complexType>
                  <xsd:sequence>
                    <xsd:element name="value" type="xsd:string"/>
                  </xsd:sequence>
                </xsd:complexType>
              </xsd:element>

              <xsd:element name="FooResponse">
                <xsd:complexType>
                  <xsd:sequence>
                    <xsd:element name="ok" type="xsd:boolean"/>
                  </xsd:sequence>
                </xsd:complexType>
              </xsd:element>
            </xsd:schema>
          </wsdl:types>

          <wsdl:message name="FooRequestMsg">
            <wsdl:part name="parameters" element="tns:Foo"/>
          </wsdl:message>

          <wsdl:message name="FooOutMsg">
            <wsdl:part name="parameters" element="tns:FooResponse"/>
          </wsdl:message>

          <wsdl:portType name="FooPort">
            <wsdl:operation name="FooOp">
              <wsdl:input message="tns:FooRequestMsg"/>
              <wsdl:output message="tns:FooOutMsg"/>
            </wsdl:operation>
          </wsdl:portType>

          <wsdl:service name="Foo">
            <wsdl:port binding="tns:FooSOAP" name="FooSOAP">
              <soap:address location="http://example.org/"/>
            </wsdl:port>
          </wsdl:service>
        </wsdl:definitions>
        )";

    const auto dir = makeUniqueTempDir("sptk_wsparser_msg_vs_element");
    const auto wsdlFile = dir / "Foo.wsdl";
    writeTextFile(wsdlFile, wsdl);

    const auto outDir = dir / "out";
    filesystem::create_directories(outDir);

    WSParser parser;
    ASSERT_NO_THROW(parser.parse(wsdlFile));
    ASSERT_NO_THROW(parser.generate(outDir.string().c_str(), "", OpenApiGenerator::Options(), false, "foo_service_ns"));

    const auto header = outDir / "CFooServiceBase.h";
    ASSERT_TRUE(filesystem::exists(header)) << header.string();

    const auto headerText = readTextFile(header);
    ASSERT_FALSE(headerText.empty());

    EXPECT_NE(headerText.find("virtual void FooOp("), string::npos);
    EXPECT_NE(headerText.find("const CFoo& input"), string::npos);
    EXPECT_NE(headerText.find("CFooResponse& output"), string::npos);

    // Must NOT accidentally use message name as type
    EXPECT_EQ(headerText.find("CFooOutMsg"), string::npos);
}

TEST(SPTK_WSParser, missing_schema_throws)
{
    const string wsdl = R"(<?xml version="1.0" encoding="UTF-8"?>
        <wsdl:definitions
          xmlns:wsdl="http://schemas.xmlsoap.org/wsdl/"
          name="NoSchema"
          targetNamespace="http://example.org/NoSchema/">
          <wsdl:service name="NoSchema"/>
        </wsdl:definitions>
        )";

    const auto dir = makeUniqueTempDir("sptk_wsparser_missing_schema");
    const auto wsdlFile = dir / "NoSchema.wsdl";
    writeTextFile(wsdlFile, wsdl);

    WSParser parser;
    EXPECT_THROW(parser.parse(wsdlFile), Exception);
}
