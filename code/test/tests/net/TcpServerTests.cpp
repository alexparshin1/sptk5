#include <chrono>
#include <gtest/gtest.h>
#include <thread>

#include <sptk5/net/SSLKeys.h>
#include <sptk5/net/TCPServer.h>
#include <sptk5/net/TCPServerListener.h>

using namespace sptk;
using namespace std::chrono_literals;

TEST(TCPServerTests, AddAndRemoveTcpListener)
{
    TCPServer  server("test-listener", 2);
    const Host listenerHost("127.0.0.1", 0); // port 0 -> ephemeral

    // Add a TCP listener
    ASSERT_NO_THROW(server.addListener(ServerConnection::Type::TCP, listenerHost, 1));

    // Server should be active and have at least one listener
    ASSERT_TRUE(server.active());

    std::this_thread::sleep_for(100ms);

    // Remove the listener and ensure the server becomes inactive
    server.removeListener(listenerHost);
    std::this_thread::sleep_for(50ms); // small delay to allow threads to stop
    ASSERT_FALSE(server.active());
}

TEST(TCPServerTests, AddSslListenerWithoutKeysThrows)
{
    TCPServer  server("ssl-listener", 2);
    const Host listenerHost("127.0.0.1", 0);

    // Adding SSL listener without setting SSL keys should throw
    EXPECT_THROW(server.addListener(ServerConnection::Type::SSL, listenerHost, 1), Exception);
}

TEST(TCPServerTests, AddSslListenerWithKeys)
{
    TCPServer  server("ssl-listener-keys", 2);
    const Host listenerHost("127.0.0.1", 0);

    // Create minimal SSLKeys with dummy file paths. We don't perform connections in this test,
    // so it's enough to set the certificate file path to an existing file (use /dev/null as placeholder on Unix).
#ifdef _WIN32
    // On Windows, skip this test because /dev/null is not available
    GTEST_SKIP();
#else
    auto keys = std::make_shared<SSLKeys>("/dev/null", "/dev/null");

    server.setSSLKeys(keys);

    // Now adding SSL listener should not throw (we won't accept connections here)
    ASSERT_NO_THROW(server.addListener(ServerConnection::Type::SSL, listenerHost, 1));

    // Cleanup
    server.removeListener(listenerHost);
#endif
}
