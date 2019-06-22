#include <sptk5/sptk.h>
#include <sptk5/net/TCPSocket.h>

#include <string.h>

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>

#include <openssl/applink.c>

using namespace sptk;
using namespace std;

/* ---------------------------------------------------------- *
 * First we need to make a standard TCP socket connection.    *
 * create_socket() creates a socket & TCP-connects to server. *
 * ---------------------------------------------------------- */
int create_socket(const String& hostname);

int main() {

  String hostname("bai-labs-rds.azurewebsites.net");
  BIO               *outbio = NULL;
  const SSL_METHOD *method;
  SSL_CTX *ctx;
  SSL *ssl;
  int server = 0;
  int ret, i;

  /* ---------------------------------------------------------- *
   * These function calls initialize openssl for correct work.  *
   * ---------------------------------------------------------- */
  OpenSSL_add_all_algorithms();
  ERR_load_BIO_strings();
  ERR_load_crypto_strings();
  SSL_load_error_strings();

  /* ---------------------------------------------------------- *
   * initialize SSL library and register algorithms             *
   * ---------------------------------------------------------- */
  if(SSL_library_init() < 0)
    cerr << "Could not initialize the OpenSSL library !" << endl;

  /* ---------------------------------------------------------- *
   * Set SSLv2 client hello, also announce SSLv3 and TLSv1      *
   * ---------------------------------------------------------- */
  method = SSLv23_client_method();

  /* ---------------------------------------------------------- *
   * Try to create a new SSL context                            *
   * ---------------------------------------------------------- */
  if ( (ctx = SSL_CTX_new(method)) == NULL)
    cerr << "Unable to create a new SSL context structure." << endl;

  /* ---------------------------------------------------------- *
   * Disabling SSLv2 will leave v3 and TSLv1 for negotiation    *
   * ---------------------------------------------------------- */
  SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);

  /* ---------------------------------------------------------- *
   * Create new SSL connection state object                     *
   * ---------------------------------------------------------- */
  ssl = SSL_new(ctx);

  /* ---------------------------------------------------------- *
   * Make the underlying TCP socket connection                  *
   * ---------------------------------------------------------- */
  sptk::TCPSocket tcpSocket;
  Host tcpHost(hostname, 443);
  tcpSocket.open(tcpHost);
  server = tcpSocket.handle();
  if (server != 0)
    cout << "Successfully made the TCP connection to " << hostname << endl;

  /* ---------------------------------------------------------- *
   * Attach the SSL session to the socket descriptor            *
   * ---------------------------------------------------------- */
  SSL_set_fd(ssl, server);

  /* ---------------------------------------------------------- *
   * Try to SSL-connect here, returns 1 for success             *
   * ---------------------------------------------------------- */
  if ( SSL_connect(ssl) != 1 )
    cerr << "Error: Could not build a SSL session to " << hostname << endl;
  else
    cout << "Successfully enabled SSL/TLS session to: " << hostname << endl;

  /* ---------------------------------------------------------- *
   * Free the structures we don't need anymore                  *
   * -----------------------------------------------------------*/
  SSL_free(ssl);
  tcpSocket.close();
  SSL_CTX_free(ctx);
  cout << "Finished SSL/TLS connection with server " << hostname << endl;
  return(0);
}


sptk::TCPSocket tcpSocket;

int create_socket(const String& hostname)
{
  Host tcpHost(hostname, 443);
  tcpSocket.open(tcpHost);
  return tcpSocket.handle();
}
