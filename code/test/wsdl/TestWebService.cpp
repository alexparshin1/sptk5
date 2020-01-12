/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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
#include "TestWebService.h"

using namespace std;
using namespace sptk;

void TestWebService::Hello(const CHello& input, CHelloResponse& output, HttpAuthentication*)
{
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
    auto* info = jwt.grants.root().set_object("info");
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
TEST(SPTK_TestWebService, hello_method)
{
    TestWebService service;

    CHello hello;
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

static String jwtString;

/**
 * Test execution of { Hello, Login, AccountBalance } methods.
 * Calling AccountBalance method requires calling Login method first.
 * If gzip-encoding is allowed, it is used for messages bigger than 255 bytes.
 * @param methodNames           WS methods to be executed
 * @param allowGzipEncoding     Allow content optionally use gzip-encoding?
 */
static void request_listener_test(const Strings& methodNames, bool allowGzipEncoding)
{
    SysLogEngine        logEngine("TestWebService");
    TestWebService      service;

    WSConnection::Paths paths("index.html","/test",".");
    WSListener          listener(service, logEngine, paths);

    const uint16_t      servicePort = 10000;
    try {

        listener.listen(servicePort);

        for (auto& methodName: methodNames) {
            Buffer sendRequestBuffer;
            json::Document sendRequestJson;

            bool useJWT = false;
            if (methodName == "Hello") {
                sendRequestJson.root()["first_name"] = "John";
                sendRequestJson.root()["last_name"] = "Doe";
            } else if (methodName == "Login") {
                sendRequestJson.root()["username"] = "johnd";
                sendRequestJson.root()["password"] = "secret";
            } else if (methodName == "AccountBalance") {
                sendRequestJson.root()["jwt"] = jwtString;
                sendRequestJson.root()["account_number"] = "000-123456-7890";
                useJWT = true;
            }

            sendRequestJson.exportTo(sendRequestBuffer);

            TCPSocket client;
            client.host(Host("localhost", servicePort));
            client.open();

            HttpConnect httpClient(client);
            Buffer requestResponse;
            httpClient.requestHeaders()["Content-Type"] = "application/json";
            if (useJWT)
                httpClient.requestHeaders()["Authorization"] = "bearer " + jwtString;
            httpClient.cmd_post("/" + methodName, HttpParams(), sendRequestBuffer, allowGzipEncoding, requestResponse);
            client.close();

            if (httpClient.statusCode() >= 400)
                FAIL() << requestResponse.c_str();
            else {
                json::Document response;
                response.load(requestResponse.c_str());

                if (methodName == "Hello") {
                    // Just check some fields
                    EXPECT_DOUBLE_EQ(response.root().getNumber("height"), 6.5);
                    EXPECT_DOUBLE_EQ(response.root().getNumber("vacation_days"), 21);
                } else if (methodName == "Login") {
                    // Remember JWT for future operations
                    jwtString = response.root().getString("jwt");

                    // Decode JWT content
                    JWT jwt;
                    jwt.decode(jwtString.c_str(), jwtEncryptionKey256);

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
TEST(SPTK_TestWebService, hello_service)
{
    request_listener_test(Strings("Hello",","), true);
}

/**
 * Test Login method input and output
 */
TEST(SPTK_TestWebService, login_method)
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
TEST(SPTK_TestWebService, LoginAndAccountBalance)
{
    request_listener_test(Strings("Login|AccountBalance", "|"), true);
}

#endif
