#include "TestSslServer.h"
#include "TestSslClient.h"

#include <arpa/inet.h>
#include <future>
#include <malloc.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define FAIL -1

#include <gtest/gtest.h>
#include <sptk5/cnet>

using namespace std;
using namespace sptk;
using namespace sptk;

TestSslServer::TestSslServer()
{
}

// Create the SSL socket and intialize the socket address structure
int TestSslServer::OpenListener(int port)
{
    int sd;
    struct sockaddr_in addr;
    sd = socket(PF_INET, SOCK_STREAM, 0);
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sd, (struct sockaddr*) &addr, sizeof(addr)) != 0)
    {
        perror("can't bind port");
        abort();
    }
    if (listen(sd, 10) != 0)
    {
        perror("Can't configure listening port");
        abort();
    }
    return sd;
}

int TestSslServer::isRoot()
{
    if (getuid() != 0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

SSL_CTX* TestSslServer::InitServerCTX(void)
{
    const SSL_METHOD* method;
    SSL_CTX* ctx;
    OpenSSL_add_all_algorithms();     /* load & register all cryptos, etc. */
    SSL_load_error_strings();         /* load all error messages */
    method = TLSv1_2_server_method(); /* create new server-method instance */
    ctx = SSL_CTX_new(method);        /* create new context from method */
    if (ctx == NULL)
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    return ctx;
}

void TestSslServer::LoadCertificates(SSL_CTX* ctx, const char* CertFile, const char* KeyFile)
{
    /* set the local certificate from CertFile */
    if (SSL_CTX_use_certificate_file(ctx, CertFile, SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    /* set the private key from KeyFile (may be the same as CertFile) */
    if (SSL_CTX_use_PrivateKey_file(ctx, KeyFile, SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    /* verify private key */
    if (!SSL_CTX_check_private_key(ctx))
    {
        fprintf(stderr, "Private key does not match the public certificate\n");
        abort();
    }
}

void TestSslServer::ShowCerts(SSL* ssl)
{
    X509* cert;
    char* line;
    cert = SSL_get_peer_certificate(ssl); /* Get certificates (if available) */
    if (cert != NULL)
    {
        printf("Server certificates:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("Subject: %s\n", line);
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("Issuer: %s\n", line);
        free(line);
        X509_free(cert);
    }
    else
        printf("No certificates.\n");
}

void TestSslServer::Servlet(SSL* ssl) /* Serve the connection -- threadable */
{
    char buf[1024] = {0};
    int sd, bytes;

    const char* ServerResponse =
        R"(<Body>
    <Name>aticleworld.com</Name>
    <year>1.5</year>
    <BlogType>Embeded and c/c++</BlogType>
    <Author>amlendra</Author>
</Body>)";

    const char* cpValidMessage =
        R"(<Body>
<UserName>aticle</UserName>\
<Password>123</Password>\
</Body>)";

    if (SSL_accept(ssl) == -1) /* do SSL-protocol accept */
        ERR_print_errors_fp(stderr);
    else
    {
        ShowCerts(ssl);                          /* get any certificates */
        bytes = SSL_read(ssl, buf, sizeof(buf)); /* get request */
        buf[bytes] = '\0';
        printf("Client msg: \"%s\"\n", buf);
        if (bytes > 0)
        {
            if (strcmp(cpValidMessage, buf) == 0)
            {
                SSL_write(ssl, ServerResponse, strlen(ServerResponse)); /* send reply */
            }
            else
            {
                SSL_write(ssl, "Invalid Message", strlen("Invalid Message")); /* send reply */
            }
        }
        else
        {
            ERR_print_errors_fp(stderr);
        }
    }
    sd = SSL_get_fd(ssl); /* get socket connection */
    SSL_free(ssl);        /* release SSL state */
    close(sd);            /* close connection */
}

sptk::Semaphore g_serverStarted;

int TestSslServer::start(uint16_t port)
{
    SSL_CTX* ctx;
    int server;

    string pem = string(TEST_DIRECTORY) + "/keys/mycert.pem";

    // Initialize the SSL library
    SSL_library_init();
    ctx = InitServerCTX();                           /* initialize SSL */
    LoadCertificates(ctx, pem.c_str(), pem.c_str()); /* load certs */
    server = OpenListener(port);                     /* create server socket */

    g_serverStarted.post();

    while (1)
    {
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        SSL* ssl;
        int client = accept(server, (struct sockaddr*) &addr, &len); /* accept connection as usual */
        printf("Connection: %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        ssl = SSL_new(ctx);      /* get new SSL state with context */
        SSL_set_fd(ssl, client); /* set connection socket to SSL state */
        Servlet(ssl);            /* service connection */
    }
    close(server);     /* close server socket */
    SSL_CTX_free(ctx); /* release context */
}

TEST(SSL, Server)
{
    auto server = async(launch::async, []() {
        COUT("### Server started" << endl)
        TestSslServer sslServer;
        sslServer.start(3000);
        COUT("### Server exited" << endl)
    });

    g_serverStarted.sleep_for(chrono::seconds(9999));

    /*
    TestSslClient client;
    client.connect("127.0.0.1", 3000);
    */
    SSLSocket socket;
    socket.open(Host("127.0.0.1:3000"), BaseSocket::OpenMode::CONNECT, 3000);

    server.wait();
}
