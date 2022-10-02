#pragma once

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <sptk5/cutils>

namespace sptk {

class TestSslClient
{
public:
    int connect(const char* addr, uint16_t port);

private:
    SSL_CTX* InitCTX(void);
    int OpenConnection(const char* hostname, int port);
    void ShowCerts(SSL* ssl);
};

} // namespace sptk
