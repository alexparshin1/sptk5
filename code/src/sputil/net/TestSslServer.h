#pragma once

#include "openssl/err.h"
#include "openssl/ssl.h"

#include <sptk5/cutils>

namespace sptk {

class TestSslServer
{
public:
    /**
     * @brief Constructor
     */
    TestSslServer();
    /**
     * @brief Destructor
     */
    virtual ~TestSslServer() = default;
    int start(uint16_t port);

public:
    int OpenListener(int port);
    int isRoot();
    SSL_CTX* InitServerCTX(void);
    void LoadCertificates(SSL_CTX* ctx, const char* CertFile, const char* KeyFile);
    void ShowCerts(SSL* ssl);
    void Servlet(SSL* ssl);
};

} // namespace sptk
