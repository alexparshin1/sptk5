/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

#include "TestWebService.h"
#include <sptk5/db/DatabaseConnectionPool.h>
#include <sptk5/db/Query.h>
#include <sptk5/wsdl/WSConnection.h>
#include <sptk5/wsdl/WSListener.h>

#ifdef USE_GTEST
#include <gtest/gtest.h>
#endif

using namespace std;
using namespace sptk;
using namespace test_service;
using namespace xdoc;

shared_ptr<HttpConnect::Authorization> TestWebService::jwtAuthorization;

void TestWebService::Hello(const CHello& input, CHelloResponse& output, HttpAuthentication*)
{
    if (input.m_action.asString() != "view")
    {
        throw Exception("Invalid action: expecting 'view'");
    }

    if (input.m_first_name.asString() != "John" || input.m_last_name.asString() != "Doe")
    {
        throw Exception("Invalid first or last name: expecting John Doe");
    }

    output.m_date_of_birth = DateTime("1981-02-01");
    output.m_height = 6.5;
    output.m_hour_rate = 15.6;
    output.m_retired = false;
    output.m_vacation_days = 21;
    output.m_verified = DateTime("2020-01-02 10:00:00+10");
}

// JWT encryption key
static const String jwtEncryptionKey256("012345678901234567890123456789XY");

/**
 * WS method that takes username and password, and returns Java Web Token (JWT).
 * After calling this method, we use JWT token *instead* of user name and password.
 */
void TestWebService::Login(const CLogin& input, CLoginResponse& output, sptk::HttpAuthentication*)
{
    constexpr int secondsInDay = 86400;

    // First, we verify credentials. Usually, we check the username and password against the password database
    if (input.m_username.asString() != "johnd" || input.m_password.asString() != "secret")
    {
        throw Exception("Invalid username or password");
    }

    JWT jwt;
    jwt.set_alg(JWT::Algorithm::HS256, jwtEncryptionKey256);

    jwt.set("iat", (int) time(nullptr));                // JWT issue time
    jwt.set("iss", "http://test.com");                  // JWT issuer
    jwt.set("exp", (int) time(nullptr) + secondsInDay); // JWT expiration time

    // Add some description information that we may use later
    const auto& info = jwt.grants.root()->pushNode("info");
    info->set("username", "johnd");
    info->set("company", "My Company");
    info->set("city", "My City");

    // Convert JWT token to string
    stringstream token;
    jwt.encode(token);

    // Return JWT token to client
    output.m_jwt = token.str();
}

void TestWebService::AccountBalance(const CAccountBalance& input, CAccountBalanceResponse& output,
                                    sptk::HttpAuthentication* authentication)
{
    static constexpr double testAmount = 12345.67;
    if (authentication == nullptr)
    {
        throw Exception("Not authenticated");
    }

    const auto& token = authentication->getData();
    const auto info = token->findFirst("info");
    auto username = info->getString("username");

    output.m_account_balance = testAmount;
}

#ifdef USE_GTEST

static constexpr int int123 = 123;

/**
 * Test Hello WS method input and output
 */
TEST(SPTK_TestWebService, Hello)
{
    TestWebService service;

    CHello hello;
    hello.m_action = "view";
    hello.m_first_name = "John";
    hello.m_last_name = "Doe";

    CHelloResponse response;
    service.Hello(hello, response, nullptr);

    if (response.m_date_of_birth.asDate() != DateTime("1981-02-01").date())
        FAIL() << "m_date_of_birth has invalid value";
    if (response.m_verified.asDateTime() != DateTime("2020-01-02 10:00:00+10"))
        FAIL() << "m_verified has invalid value";
    EXPECT_DOUBLE_EQ(response.m_height, 6.5);
    EXPECT_DOUBLE_EQ(response.m_hour_rate, 15.6);
    EXPECT_EQ(response.m_retired.asBool(), false);
    EXPECT_EQ(response.m_vacation_days.asInteger(), 21);
}

static const String soapWrapper(
    R"(<?xml version="1.0" encoding="UTF-8"?>)"
    R"(<soap:Envelope xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/">)"
    "<soap:Body>"
    "{REQUEST_DATA}"
    "</soap:Body>"
    "</soap:Envelope>");


static Document make_send_request(const String& methodName, DataFormat dataFormat)
{
    Document sendRequest;

    auto requestNode = sendRequest.root();

    if (dataFormat == DataFormat::XML)
    {
        auto wrapper = soapWrapper.replace("{REQUEST_DATA}", "<ns1:" + methodName + "/>");
        sendRequest.load(wrapper);
        requestNode = sendRequest.root()->findFirst("ns1:" + methodName);
    }

    if (methodName == "Hello")
    {
        requestNode->set("first_name", "John");
        requestNode->set("last_name", "Doe");
        TestWebService::jwtAuthorization.reset();
    }
    else if (methodName == "Login")
    {
        requestNode->set("username", "johnd");
        requestNode->set("password", "secret");
        TestWebService::jwtAuthorization.reset();
    }
    else if (methodName == "AccountBalance")
    {
        requestNode->set("account_number", "000-123456-7890");
    }

    return sendRequest;
}

static SNode get_response_node(const Document& response, DataFormat dataFormat)
{
    SNode responseNode = response.root();
    if (dataFormat == DataFormat::XML)
    {
        auto bodyNode = responseNode->findFirst("soap:Body");
        responseNode = *bodyNode->nodes().begin();
    }
    return responseNode;
}

class TestListener : public WSListener
{
public:
    uint16_t servicePort {11000};
    shared_ptr<SSLKeys> sslKeys;
    SysLogEngine logEngine {"TestWebService"};

    TestListener(const WSServices& services, const WSConnection::Options& options, bool encrypted)
        : WSListener(services, logEngine, "localhost", 16, options)
    {
        if (encrypted)
        {
            servicePort = 11001;
            sslKeys = make_shared<SSLKeys>("keys/test.key", "keys/test.cert");
            setSSLKeys(sslKeys);
        }
    }
};

shared_ptr<TestListener> createTestListener(bool encrypted)
{
    auto service = make_shared<TestWebService>();

    // Define Web Service listener
    const WSConnection::Paths paths("index.html", "/test", ".");
    WSConnection::Options options(paths);
    options.encrypted = encrypted;
    const WSServices services(service);
    auto testListener = make_shared<TestListener>(services, options, encrypted);

    // Start Web Service listener
    testListener->listen(ServerConnection::Type::TCP, testListener->servicePort);

    return testListener;
}

/**
 * Test execution of { Hello, Login, AccountBalance } methods.
 * Calling AccountBalance method requires calling Login method first.
 * If gzip-encoding is allowed, it is used for messages bigger than 255 bytes.
 * @param methodNames           WS methods to be executed
 */
static void request_listener_test(const Strings& methodNames, DataFormat dataFormat, bool encrypted = false)
{
    static shared_ptr<TestListener> tcpTestListener;
    static shared_ptr<TestListener> sslTestListener;

    if (!tcpTestListener)
    {
        tcpTestListener = createTestListener(false);
        sslTestListener = createTestListener(true);
    }

    auto testListener = encrypted ? sslTestListener : tcpTestListener;

    try
    {
        const String serviceType = dataFormat == DataFormat::XML ? "xml" : "json";

        for (const auto& methodName: methodNames)
        {
            Buffer sendRequestBuffer;
            const Document sendRequest = make_send_request(methodName, dataFormat);
            sendRequest.exportTo(dataFormat, sendRequestBuffer, true);

            shared_ptr<TCPSocket> client;
            if (encrypted)
            {
                auto sslClient = make_shared<SSLSocket>();
                sslClient->loadKeys(*testListener->sslKeys);
                client = sslClient;
            }
            else
            {
                client = make_shared<TCPSocket>();
            }
            client->host(Host("localhost", testListener->servicePort));
            client->open();

            HttpConnect httpClient(*client);
            const HttpParams httpParams {{"action", "view"}};
            Buffer requestResponse;
            httpClient.requestHeaders()["Content-Type"] = "application/" + serviceType;
            const int statusCode = httpClient.cmd_post("/" + methodName, httpParams, sendRequestBuffer,
                                                       requestResponse, {"gzip"},
                                                       TestWebService::jwtAuthorization.get());
            client->close();

            if (statusCode >= 400)
                FAIL() << requestResponse.c_str();
            else
            {
                Document response;
                response.load(requestResponse.c_str());

                auto responseNode = get_response_node(response, dataFormat);

                if (methodName == "Hello")
                {
                    // Just check some fields
                    EXPECT_DOUBLE_EQ(responseNode->getNumber("height"), 6.5);
                    EXPECT_DOUBLE_EQ(responseNode->getNumber("vacation_days"), 21);
                }
                else if (methodName == "Login")
                {
                    // Set JWT authorization for future operations
                    TestWebService::jwtAuthorization = make_shared<HttpConnect::BearerAuthorization>(
                        responseNode->getString("jwt"));

                    // Decode JWT content
                    JWT jwt;
                    jwt.decode(TestWebService::jwtAuthorization->value().c_str(), jwtEncryptionKey256);

                    // Get username from "info" node
                    auto info = jwt.grants.root()->findFirst("info");
                    auto username = info->getString("username");

                    EXPECT_STREQ(username.c_str(), "johnd");
                }
                else if (methodName == "AccountBalance")
                {
                    EXPECT_DOUBLE_EQ(responseNode->getNumber("account_balance"), 12345.67);
                }
            }
        }
    }
    catch (const Exception& e)
    {
        FAIL() << e.what();
    }
}

/**
 * Test Hello method working through the service in JSON and XML modes
 */
TEST(SPTK_TestWebService, Hello_HTTP)
{
    for (auto dataType: {DataFormat::JSON, DataFormat::XML})
    {
        request_listener_test({"Hello"}, dataType, false);
    }
}

/**
 * Test Login method input and output
 */
TEST(SPTK_TestWebService, Login)
{
    TestWebService service;

    CLogin hello;
    hello.m_username = "johnd";
    hello.m_password = "secret";

    CLoginResponse response;
    service.Login(hello, response, nullptr);

    JWT jwt;
    jwt.decode(response.m_jwt.getString(), jwtEncryptionKey256);

    auto info = jwt.grants.root()->findFirst("info");
    auto username = info->getString("username");

    EXPECT_STREQ(username.c_str(), "johnd");
}

/**
 * Test Login and AccountBalance methods working through the service
 */
TEST(SPTK_TestWebService, LoginAndAccountBalance_HTTP)
{
    for (auto dataType: {DataFormat::JSON, DataFormat::XML})
    {
        request_listener_test(Strings("Login|AccountBalance", "|"), dataType, false);
    }
}

/**
 * Test Login and AccountBalance methods working through the service
 */
TEST(SPTK_TestWebService, LoginAndAccountBalance_HTTPS)
{
    for (auto dataType: {DataFormat::JSON, DataFormat::XML})
    {
        request_listener_test(Strings("Login|AccountBalance", "|"), dataType, true);
    }
}

static String exportToString(const WSComplexType& object)
{
    Buffer buffer;
    xdoc::Document document;
    object.exportTo(document.root());
    document.root()->exportTo(xdoc::DataFormat::JSON, buffer, true);
    return String(buffer);
}

TEST(SPTK_WSGeneratedClasses, CopyConstructor)
{
    CLogin login;
    login.m_username = "johnd";
    login.m_password = "secret";
    auto str = exportToString(login);

    const CLogin login2(login);
    EXPECT_EQ(login.m_username.asString(), login2.m_username.asString());
    EXPECT_EQ(login.m_password.asString(), login2.m_password.asString());
    auto str2 = exportToString(login2);

    EXPECT_STREQ(str.c_str(), str2.c_str());
}

TEST(SPTK_WSGeneratedClasses, MoveConstructor)
{
    CLogin login;
    login.m_username = "johnd";
    login.m_password = "secret";
    auto str = exportToString(login);

    const CLogin login2(std::move(login));
    EXPECT_STREQ("johnd", login2.m_username.asString().c_str());
    EXPECT_STREQ("secret", login2.m_password.asString().c_str());
    auto str2 = exportToString(login2);

    EXPECT_STREQ(str.c_str(), str2.c_str());
}

TEST(SPTK_WSGeneratedClasses, CopyAssignment)
{
    CLogin login;
    login.m_username = "johnd";
    login.m_password = "secret";
    auto str = exportToString(login);

    CLogin login2;
    login2 = login;
    EXPECT_EQ(login.m_username.asString(), login2.m_username.asString());
    EXPECT_EQ(login.m_password.asString(), login2.m_password.asString());
    auto str2 = exportToString(login2);

    EXPECT_STREQ(str.c_str(), str2.c_str());
}

TEST(SPTK_WSGeneratedClasses, MoveAssignment)
{
    CLogin login;
    login.m_username = "johnd";
    login.m_password = "secret";
    auto str = exportToString(login);

    CLogin login2;
    login2 = std::move(login);
    EXPECT_STREQ("johnd", login2.m_username.asString().c_str());
    EXPECT_STREQ("secret", login2.m_password.asString().c_str());
    auto str2 = exportToString(login2);

    EXPECT_STREQ(str.c_str(), str2.c_str());
}

TEST(SPTK_WSGeneratedClasses, Clear)
{
    CLogin login;
    login.m_username = "johnd";
    login.m_password = "secret";

    login.clear();

    EXPECT_TRUE(login.m_username.isNull());
    EXPECT_TRUE(login.m_password.isNull());
    EXPECT_TRUE(login.isNull());
}

static const String testXML(
    R"(<?xml version="1.0" encoding="UTF-8"?>)"
    "<login server_count=\"2\" type=\"abstract\">"
    "<username>johnd</username>"
    "<password>secret</password>"
    "<servers><item>x1</item><item>x2</item></servers>"
    "<project><id>123</id><expiration>2020-10-01</expiration></project>"
    "</login>");
static const String testJSON(
    R"({"attributes":{"server_count":2,"type":"abstract"},"username":"johnd","password":"secret","servers":["x1","x2"],"project":{"id":123,"expiration":"2020-10-01"}})");

TEST(SPTK_WSGeneratedClasses, LoadXML)
{
    Document input;
    input.load(testXML);
    const auto loginNode = input.root()->findFirst("login");

    CLogin login;
    login.load(loginNode);

    EXPECT_STREQ("johnd", login.m_username.asString().c_str());
    EXPECT_STREQ("secret", login.m_password.asString().c_str());
    EXPECT_STREQ("123", login.m_project.m_id.asString().c_str());
    EXPECT_STREQ("2020-10-01", login.m_project.m_expiration.asString().c_str());
}

TEST(SPTK_WSGeneratedClasses, LoadJSON)
{
    Document input;
    input.load(testXML);
    const auto loginNode = input.root()->findFirst("login");

    CLogin login;
    login.load(loginNode);

    EXPECT_STREQ("johnd", login.m_username.asString().c_str());
    EXPECT_STREQ("secret", login.m_password.asString().c_str());
}

TEST(SPTK_WSGeneratedClasses, LoadFields)
{
    FieldList fields(false);
    fields.push_back(make_shared<Field>("username"));
    fields.push_back(make_shared<Field>("password"));
    fields.push_back(make_shared<Field>("servers"));
    fields.push_back(make_shared<Field>("server_count"));

    fields["username"] = "johnd";
    fields["password"] = "secret";
    fields["server_count"] = 2;
    fields["servers"] = R"(["x1","x2"])";

    CLogin login;
    login.load(fields);

    EXPECT_STREQ("johnd", login.m_username.asString().c_str());
    EXPECT_STREQ("secret", login.m_password.asString().c_str());
    EXPECT_EQ(2, login.m_server_count.asInteger());
    EXPECT_STREQ("x1", login.m_servers[0].asString().c_str());
}

TEST(SPTK_WSGeneratedClasses, UnloadXML)
{
    CLogin login;
    login.m_username = "johnd";
    login.m_password = "secret";
    login.m_servers.push_back(WSString("x1"));
    login.m_servers.push_back(WSString("x2"));
    login.m_project.m_id = int123;
    login.m_project.m_expiration = "2020-10-01";
    login.m_server_count = 2;
    login.m_type = "abstract";

    Document xml;
    auto loginNode = xml.root()->findOrCreate("login");
    login.unload(loginNode);

    Buffer buffer;
    xml.root()->exportTo(DataFormat::XML, buffer, false);

    // Exclude <?xml..?> from test
    auto pos = testXML.find("?>") + 2;
    EXPECT_STREQ(buffer.c_str(), testXML.substr(pos).c_str());
}

TEST(SPTK_WSGeneratedClasses, UnloadJSON)
{
    CLogin login;

    EXPECT_THROW(login.throwIfNull(""), SOAPException);

    login.m_username = "johnd";
    login.m_password = "secret";
    login.m_servers.push_back(WSString("x1"));
    login.m_servers.push_back(WSString("x2"));
    login.m_project.m_id = int123;
    login.m_project.m_expiration = "2020-10-01";
    login.m_server_count = 2;
    login.m_type = "abstract";

    Document json;
    login.unload(json.root());

    Buffer buffer;
    json.exportTo(DataFormat::JSON, buffer, false);

    EXPECT_STREQ(buffer.c_str(), testJSON.c_str());

    auto str = login.toString();
    EXPECT_STREQ(str.c_str(), testJSON.c_str());
}

TEST(SPTK_WSGeneratedClasses, UnloadQueryParameters)
{
    CLogin login;
    login.m_username = "johnd";
    login.m_password = "secret";
    login.m_servers.push_back(WSString("x1"));
    login.m_servers.push_back(WSString("x2"));
    login.m_project.m_id = int123;
    login.m_project.m_expiration = "2020-10-01";
    login.m_server_count = 2;
    login.m_type = "abstract";

    DatabaseConnectionPool pool("postgresql://localhost/test");
    auto connection = pool.getConnection();
    Query query(connection,
                "SELECT * FROM test WHERE username = :username AND password = :password AND servers = :servers");

    login.unload(query.params());

    EXPECT_STREQ(query.param("username").asString().c_str(), "johnd");
    EXPECT_STREQ(query.param("password").asString().c_str(), "secret");
    EXPECT_STREQ(query.param("servers").asString().c_str(), R"(["x1","x2"])");
}

#endif
