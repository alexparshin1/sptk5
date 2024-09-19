/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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

#include <future>
#include <gtest/gtest.h>
#include <sptk5/cnet>

using namespace std;
using namespace sptk;
using namespace chrono;

TEST(SPTK_SSLSocket, connect)
{
    const SSLKeys keys(String(TEST_DIRECTORY) + "/keys/test.key", String(TEST_DIRECTORY) + "/keys/test.cert");
    if (!filesystem::exists(keys.certificateFileName()))
    {
        GTEST_SKIP() << "Certificate file " << keys.certificateFileName() << " does not exist.";
    }

    SSLSocket sslSocket;

    try
    {
        sslSocket.loadKeys(keys); // Optional step - not required for Google connect
        sslSocket.open(Host("www.msn.com:443"));
        sslSocket.close();
    }
    catch (const Exception& e)
    {
        FAIL() << e.what();
    }
}

TEST(SPTK_SSLSocket, httpConnect)
{
    GTEST_SKIP();

    constexpr uint16_t sslPort {443};
    const Host         yahoo("www.yahoo.com", sslPort);
    sockaddr_in        address {};
    yahoo.getAddress(address);

    vector<future<tuple<String, String>>> tasks;
    for (int i = 0; i < 2; i++)
    {
        auto task = async(launch::async, [&]()
                          {
                              Buffer output;
                              String result {"OK"};
                              try
                              {
                                  const auto socket = make_shared<SSLSocket>();

                                  socket->open(yahoo);
                                  HttpConnect http(*socket);

                                  const auto statusCode = http.cmd_get("/", HttpParams(), output);
                                  if (statusCode != 200)
                                  {
                                      result = http.statusText().c_str();
                                  }
                              }
                              catch (const Exception& e)
                              {
                                  result = e.what();
                              }
                              return make_tuple(result, static_cast<String>(output));
                          });

        tasks.push_back(std::move(task));
    }

    for (auto& task: tasks)
    {
        if (task.wait_for(chrono::seconds(3)) == future_status::timeout)
        {
            FAIL() << "Timeout";
        }
        else
        {
            auto result = task.get();
            EXPECT_STREQ("OK", get<0>(result).c_str());
            EXPECT_TRUE(get<1>(result).toLowerCase().find("</html>") != string::npos);
        }
    }
}
