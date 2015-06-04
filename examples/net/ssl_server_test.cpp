#include <errno.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <resolv.h>
#include "openssl/ssl.h"
#include "openssl/err.h"

#include <sptk5/net/CSSLContext.h>
#include <sptk5/net/CSSLSocket.h>

using namespace sptk;

#define FAIL    -1

int OpenListener(int port)
{   int sd;
    struct sockaddr_in addr;

    sd = socket(PF_INET, SOCK_STREAM, 0);
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    if ( bind(sd, (struct sockaddr*)&addr, sizeof(addr)) != 0 )
    {
        perror("can't bind port");
        abort();
    }
    if ( listen(sd, 10) != 0 )
    {
        perror("Can't configure listening port");
        abort();
    }
    return sd;
}

void ShowCerts(SSL* ssl)
{   X509 *cert;
    char *line;

    cert = SSL_get_peer_certificate(ssl); /* Get certificates (if available) */
    if ( cert != NULL )
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

void Servlet(CSSLSocket& socket) /* Serve the connection -- threadable */
{   char buf[1024];
    char reply[1024];
    int sd, bytes;
    const char* HTMLecho="<html><body><pre>%s</pre></body></html>\n\n";

    ShowCerts(socket.handle());        /* get any certificates */
    bytes = SSL_read(socket.handle(), buf, sizeof(buf)); /* get request */
    if ( bytes > 0 )
    {
        buf[bytes] = 0;
        printf("Client msg: \"%s\"\n", buf);
        sprintf(reply, HTMLecho, buf);   /* construct reply */
        SSL_write(socket.handle(), reply, strlen(reply)); /* send reply */
    }
    else
        ERR_print_errors_fp(stderr);
}

int main(int count, char *strings[])
{   SSL_CTX *ctx;
    int server;
    char *portnum;

    if ( count != 2 )
    {
        printf("Usage: %s <portnum>\n", strings[0]);
        exit(0);
    }

    CSSLContext sslContext;

    portnum = strings[1];
    ctx = sslContext.handle();        /* initialize SSL */
    sslContext.loadKeys("key.pem", "cert.pem", "");
    server = OpenListener(atoi(portnum));    /* create server socket */
    while (1)
    {   struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        SSL *ssl;

        int client = accept(server, (struct sockaddr*)&addr, &len);  /* accept connection as usual */
        printf("Connection: %s:%d\n",inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

        //ssl = SSL_new(ctx);           // get new SSL state with context
        CSSLSocket socket(sslContext);
        ssl = socket.handle();

        //SSL_set_fd(ssl, client);      // set connection socket to SSL state
        socket.attach(client);

        Servlet(socket);         /* service connection */
    }
    close(server);          /* close server socket */
    SSL_CTX_free(ctx);         /* release context */
}