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

#include <sptk5/wsdl/WSConnection.h>
#include <sptk5/wsdl/WSListener.h>
#include <sptk5/StopWatch.h>
#include <sptk5/db/Query.h>
#include <sptk5/db/DatabaseConnectionPool.h>
#include "TestWebService.h"

using namespace std;
using namespace sptk;
using namespace test_service;

shared_ptr<HttpConnect::Authorization> TestWebService::jwtAuthorization;

void TestWebService::Hello(const CHello& input, CHelloResponse& output, HttpAuthentication*)
{
    if (input.m_action.asString() != "view")
        throw Exception("Invalid action: expecting 'view'");

    if (input.m_first_name.asString() != "John" || input.m_last_name.asString() != "Doe")
        throw Exception("Invalid first or last name: expecting John Doe");

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
    // First, we verify credentials. Usually, we check the username and password against the password database
    String username = input.m_username;
    String password = input.m_password;
    if (username != "johnd" || password != "secret")
        throw Exception("Invalid username or password");

    JWT jwt;
    jwt.set_alg(JWT::JWT_ALG_HS256, jwtEncryptionKey256);

    jwt["iat"] = (int) time(nullptr);           // JWT issue time
    jwt["iss"] = "http://test.com";                   // JWT issuer
    jwt["exp"] = (int) time(nullptr) + 86400;   // JWT expiration time

    // Add some description information that we may use later
    auto* info = jwt.grants.root().add_object("info");
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
    if (authentication == nullptr)
        throw Exception("Not authenticated");

    auto& token = authentication->getData();
    auto& info = token.getObject("info");
    auto  username = info["username"].getString();

    output.m_account_balance = 12345.67;
}

#if USE_GTEST

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

/**
 * Test execution of { Hello, Login, AccountBalance } methods.
 * Calling AccountBalance method requires calling Login method first.
 * If gzip-encoding is allowed, it is used for messages bigger than 255 bytes.
 * @param methodNames           WS methods to be executed
 */
static void request_listener_test(const Strings& methodNames, bool encrypted = false)
{
    SysLogEngine    logEngine("TestWebService");
    auto            service = make_shared<TestWebService>();

    // Define Web Service listener
    WSConnection::Paths     paths("index.html","/test",".");
    WSConnection::Options   options(paths);
    options.encrypted = encrypted;
    WSServices services(service);
    WSListener listener(services, logEngine, "localhost", 16, options);

    const uint16_t      servicePort = 11000;
    shared_ptr<SSLKeys> sslKeys;
    try {

        if (encrypted) {
            sslKeys = make_shared<SSLKeys>("keys/test.key", "keys/test.cert");
            listener.setSSLKeys(sslKeys);
        }

        // Start Web Service listener
        listener.listen(servicePort);

        for (auto& methodName: methodNames) {
            Buffer sendRequestBuffer;
            json::Document sendRequestJson;

            if (methodName == "Hello") {
                sendRequestJson.root()["first_name"] = "John";
                sendRequestJson.root()["last_name"] = "Doe";
                TestWebService::jwtAuthorization.reset();
            } else if (methodName == "Login") {
                sendRequestJson.root()["username"] = "johnd";
                sendRequestJson.root()["password"] = "secret";
                TestWebService::jwtAuthorization.reset();
            } else if (methodName == "AccountBalance") {
                sendRequestJson.root()["account_number"] = "000-123456-7890";
            }

            sendRequestJson.exportTo(sendRequestBuffer);

            shared_ptr<TCPSocket> client;
            if (encrypted) {
                auto sslClient = make_shared<SSLSocket>();
                sslClient->loadKeys(*sslKeys);
                client = sslClient;
            } else
                client = make_shared<TCPSocket>();
            client->host(Host("localhost", servicePort));
            client->open();

            HttpConnect httpClient(*client);
            HttpParams httpParams { {"action", "view"} };
            Buffer requestResponse;
            httpClient.requestHeaders()["Content-Type"] = "application/json";
            int statusCode = httpClient.cmd_post("/" + methodName, httpParams, sendRequestBuffer, requestResponse, {"gzip"}, TestWebService::jwtAuthorization.get());
            client->close();

            if (statusCode >= 400)
                FAIL() << requestResponse.c_str();
            else {
                json::Document response;
                response.load(requestResponse.c_str());

                if (methodName == "Hello") {
                    // Just check some fields
                    EXPECT_DOUBLE_EQ(response.root().getNumber("height"), 6.5);
                    EXPECT_DOUBLE_EQ(response.root().getNumber("vacation_days"), 21);
                } else if (methodName == "Login") {
                    // Set JWT authorization for future operations
                    TestWebService::jwtAuthorization = make_shared<HttpConnect::BearerAuthorization>(response.root().getString("jwt"));

                    // Decode JWT content
                    JWT jwt;
                    jwt.decode(TestWebService::jwtAuthorization->value().c_str(), jwtEncryptionKey256);

                    // Get username from "info" node
                    auto& info = jwt.grants.root().getObject("info");
                    auto username = info["username"].getString();

                    EXPECT_STREQ(username.c_str(), "johnd");
                } else if (methodName == "AccountBalance") {
                    EXPECT_DOUBLE_EQ(response.root().getNumber("account_balance"), 12345.67);
                }
            }
        }

        StopWatch stopwatch;
        stopwatch.start();
        listener.stop();
        stopwatch.stop();
    }
    catch (const Exception& e) {
        FAIL() << e.what();
    }
}

/**
 * Test Hello method working through the service
 */
TEST(SPTK_TestWebService, Hello_HTTP)
{
    request_listener_test(Strings("Hello", ","), false);
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

    auto& info = jwt.grants.root().getObject("info");
    auto username = info["username"].getString();

    EXPECT_STREQ(username.c_str(), "johnd");
}

/**
 * Test Login and AccountBalance methods working through the service
 */
TEST(SPTK_TestWebService, LoginAndAccountBalance_HTTP)
{
    request_listener_test(Strings("Login|AccountBalance", "|"), false);
}

/**
 * Test Login and AccountBalance methods working through the service
 */
TEST(SPTK_TestWebService, LoginAndAccountBalance_HTTPS)
{
    request_listener_test(Strings("Login|AccountBalance", "|"), true);
}

TEST(SPTK_WSGeneratedClasses, CopyConstructor)
{
    CLogin login;
    login.m_username = "johnd";
    login.m_password = "secret";

    CLogin login2(login);
    EXPECT_EQ(login.m_username.asString(), login2.m_username.asString());
    EXPECT_EQ(login.m_password.asString(), login2.m_password.asString());
}

TEST(SPTK_WSGeneratedClasses, MoveConstructor)
{
    CLogin login;
    login.m_username = "johnd";
    login.m_password = "secret";

    CLogin login2(move(login));
    EXPECT_STREQ("johnd", login2.m_username.asString().c_str());
    EXPECT_STREQ("secret", login2.m_password.asString().c_str());
    EXPECT_TRUE(login.m_username.isNull());
    EXPECT_TRUE(login.m_password.isNull());
}

TEST(SPTK_WSGeneratedClasses, CopyAssignment)
{
    CLogin login;
    login.m_username = "johnd";
    login.m_password = "secret";

    CLogin login2;
    login2 = login;
    EXPECT_EQ(login.m_username.asString(), login2.m_username.asString());
    EXPECT_EQ(login.m_password.asString(), login2.m_password.asString());
}

TEST(SPTK_WSGeneratedClasses, MoveAssignment)
{
    CLogin login;
    login.m_username = "johnd";
    login.m_password = "secret";

    CLogin login2;
    login2 = move(login);
    EXPECT_STREQ("johnd", login2.m_username.asString().c_str());
    EXPECT_STREQ("secret", login2.m_password.asString().c_str());
    EXPECT_TRUE(login.m_username.isNull());
    EXPECT_TRUE(login.m_password.isNull());
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

static const String testXML(R"(<?xml version="1.0" encoding="UTF-8"?><login><username>johnd</username><password>secret</password></login>)");
static const String testJSON(R"({"username":"johnd","password":"secret"})");

TEST(SPTK_WSGeneratedClasses, LoadXML)
{
    xml::Document input;
    input.load(testXML);
    const auto* loginNode = input.findFirst("login");

    CLogin login;
    login.load(loginNode);

    EXPECT_STREQ("johnd", login.m_username.asString().c_str());
    EXPECT_STREQ("secret", login.m_password.asString().c_str());
}

TEST(SPTK_WSGeneratedClasses, LoadJSON)
{
    json::Document input;
    input.load(testJSON);
    const auto& loginNode = input.root();

    CLogin login;
    login.load(&loginNode);

    EXPECT_STREQ("johnd", login.m_username.asString().c_str());
    EXPECT_STREQ("secret", login.m_password.asString().c_str());
}

TEST(SPTK_WSGeneratedClasses, UnloadXML)
{
    CLogin login;
    login.m_username = "johnd";
    login.m_password = "secret";

    xml::Document xml;
    auto* loginNode = xml.findOrCreate("login");
    login.unload(loginNode);

    Buffer buffer;
    xml.save(buffer, 0);

    EXPECT_STREQ(buffer.c_str(), testXML.c_str());
}

TEST(SPTK_WSGeneratedClasses, UnloadJSON)
{
    CLogin login;
    login.m_username = "johnd";
    login.m_password = "secret";

    json::Document json;
    login.unload(&json.root());

    Buffer buffer;
    json.exportTo(buffer, false);

    EXPECT_STREQ(buffer.c_str(), testJSON.c_str());
}

TEST(SPTK_WSGeneratedClasses, UnloadQueryParameters)
{
    CLogin login;
    login.m_username = "johnd";
    login.m_password = "secret";

    DatabaseConnectionPool pool("postgresql://localhost/test");
    auto connection = pool.getConnection();
    Query query(connection, "SELECT * FROM test WHERE username = :username and password = :password");

    login.unload(query.params());

    EXPECT_STREQ(query.param("username").asString().c_str(), "johnd");
    EXPECT_STREQ(query.param("password").asString().c_str(), "secret");
}

#endif
