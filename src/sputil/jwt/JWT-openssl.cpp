/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       jwt_test.cpp - description                             ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Monday Feb 12 2017                                     ║
║  copyright            (C) 1999-2018 by Alexey Parshin.                       ║
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
┌──────────────────────────────────────────────────────────────────────────────┐
│   The code in this module is based JWT C Library, developed by Ben Collins.  │
│   Please see http://github.com/benmcollins/libjwt for more information.      │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <cstdlib>
#include <cstring>
#include <cerrno>

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/pem.h>

#include <sptk5/JWT.h>

using namespace std;
using namespace sptk;

/* Functions to make libjwt backward compatible with OpenSSL version < 1.1.0
 * See https://wiki.openssl.org/index.php/1.1_API_Changes
 */
#if OPENSSL_VERSION_NUMBER < 0x10100000L

static void ECDSA_SIG_get0(const ECDSA_SIG *sig, const BIGNUM **pr, const BIGNUM **ps)
{
    if (pr != NULL)
        *pr = sig->r;
    if (ps != NULL)
        *ps = sig->s;
}

static int ECDSA_SIG_set0(ECDSA_SIG *sig, BIGNUM *r, BIGNUM *s)
{
    if (r == NULL || s == NULL)
        return 0;

    BN_clear_free(sig->r);
    BN_clear_free(sig->s);
    sig->r = r;
    sig->s = s;

    return 1;
}

#endif

namespace sptk {

void JWT::sign_sha_hmac(char** out, unsigned int* len, const char* str)
{
    const EVP_MD* alg;

    switch (this->alg) {
        /* HMAC */
        case JWT::JWT_ALG_HS256:
            alg = EVP_sha256();
            break;
        case JWT::JWT_ALG_HS384:
            alg = EVP_sha384();
            break;
        case JWT::JWT_ALG_HS512:
            alg = EVP_sha512();
            break;
        default:
            throw Exception("Invalid sign algorithm");
    }

    *out = (char*) malloc(EVP_MAX_MD_SIZE);
    if (*out == nullptr)
        throw Exception("Can't allocate memory");

    HMAC(alg, key.c_str(), (int) key.length(),
         (const unsigned char*) str, (int) strlen(str), (unsigned char*) *out,
         len);
}

void JWT::verify_sha_hmac(const char* head, const char* sig)
{
    unsigned char res[EVP_MAX_MD_SIZE];
    BIO* bmem = nullptr, * b64 = nullptr;
    unsigned int res_len;
    const EVP_MD* alg;
    int len;
    Buffer readBuf;

    switch (this->alg) {
        case JWT::JWT_ALG_HS256:
            alg = EVP_sha256();
            break;
        case JWT::JWT_ALG_HS384:
            alg = EVP_sha384();
            break;
        case JWT::JWT_ALG_HS512:
            alg = EVP_sha512();
            break;
        default:
            throw Exception("Invalid verify algorithm");
    }

    bool matches = false;

    b64 = BIO_new(BIO_f_base64());
    if (b64 == nullptr)
        throw Exception("Can't allocate memory");

    bmem = BIO_new(BIO_s_mem());
    if (bmem == nullptr) {
        BIO_free(b64);
        throw Exception("Can't allocate memory");
    }

    BIO_push(b64, bmem);
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    HMAC(alg, key.c_str(), (int) key.length(),
         (const unsigned char*) head, (int) strlen(head), res, &res_len);

    BIO_write(b64, res, res_len);

    (void) BIO_flush(b64);

    len = BIO_pending(bmem);
    if (len < 0)
        goto jwt_verify_hmac_done;

    readBuf.checkSize(size_t(len) + 1);
    len = BIO_read(bmem, readBuf.data(), len);
    readBuf.bytes(size_t(len));
	readBuf[len] = 0;

    jwt_base64uri_encode(readBuf);

    /* And now... */
    matches = strcmp(readBuf.c_str(), sig) == 0;

    jwt_verify_hmac_done:
    BIO_free_all(b64);

    if (!matches)
        throw Exception("Signature doesn't match");
}

#define SIGN_ERROR(__err) { if ((__err) == EINVAL) throw Exception("Invalid value"); else throw Exception("Can't allocate memory"); }

void JWT::sign_sha_pem(char** out, unsigned int* len, const char* str)
{
    EVP_MD_CTX* mdctx = nullptr;
    ECDSA_SIG* ec_sig = nullptr;
    const BIGNUM* ec_sig_r = nullptr;
    const BIGNUM* ec_sig_s = nullptr;
    BIO* bufkey = nullptr;
    const EVP_MD* alg;
    int type;
    EVP_PKEY* pkey = nullptr;
    unsigned char* sig;
    size_t slen;

    switch (this->alg) {
        /* RSA */
        case JWT::JWT_ALG_RS256:
            alg = EVP_sha256();
            type = EVP_PKEY_RSA;
            break;
        case JWT::JWT_ALG_RS384:
            alg = EVP_sha384();
            type = EVP_PKEY_RSA;
            break;
        case JWT::JWT_ALG_RS512:
            alg = EVP_sha512();
            type = EVP_PKEY_RSA;
            break;

            /* ECC */
        case JWT::JWT_ALG_ES256:
            alg = EVP_sha256();
            type = EVP_PKEY_EC;
            break;
        case JWT::JWT_ALG_ES384:
            alg = EVP_sha384();
            type = EVP_PKEY_EC;
            break;
        case JWT::JWT_ALG_ES512:
            alg = EVP_sha512();
            type = EVP_PKEY_EC;
            break;

        default:
            throw Exception("Invalid sign algorithm");
    }

    Buffer sig_buffer;
    string error;

    try {
        bufkey = BIO_new_mem_buf((void*) key.c_str(), (int) key.length());
        if (bufkey == nullptr) SIGN_ERROR(ENOMEM);

        /* This uses OpenSSL's default passphrase callback if needed. The
         * library caller can override this in many ways, all of which are
         * outside of the scope of LibJWT and this is documented in jwt.h. */
        pkey = PEM_read_bio_PrivateKey(bufkey, nullptr, nullptr, nullptr);
        if (pkey == nullptr) SIGN_ERROR(EINVAL);

        int pkey_type = EVP_PKEY_id(pkey);
        if (pkey_type != type) SIGN_ERROR(EINVAL);

        mdctx = EVP_MD_CTX_create();
        if (mdctx == nullptr) SIGN_ERROR(ENOMEM);

        /* Initialize the DigestSign operation using alg */
        if (EVP_DigestSignInit(mdctx, nullptr, alg, nullptr, pkey) != 1) SIGN_ERROR(EINVAL);

        /* Call update with the message */
        if (EVP_DigestSignUpdate(mdctx, str, strlen(str)) != 1) SIGN_ERROR(EINVAL);

        /* First, call EVP_DigestSignFinal with a nullptr sig parameter to get length
         * of sig. Length is returned in slen */
        if (EVP_DigestSignFinal(mdctx, nullptr, &slen) != 1) SIGN_ERROR(EINVAL);

        /* Allocate memory for signature based on returned size */
        sig_buffer.checkSize(slen);
        sig = (unsigned char*) sig_buffer.data();

        /* Get the signature */
        if (EVP_DigestSignFinal(mdctx, sig, &slen) != 1) SIGN_ERROR(EINVAL);

        if (pkey_type != EVP_PKEY_EC) {
            *out = (char*) malloc(slen);
            if (*out == nullptr) SIGN_ERROR(ENOMEM);
            memcpy(*out, sig, slen);
            *len = (unsigned) slen;
        } else {
            unsigned int degree, bn_len, r_len, s_len, buf_len;
            unsigned char* raw_buf;
            EC_KEY* ec_key;

            /* For EC we need to convert to a raw format of R/S. */

            /* Get the actual ec_key */
            ec_key = EVP_PKEY_get1_EC_KEY(pkey);
            if (ec_key == nullptr) SIGN_ERROR(ENOMEM);

            degree = (unsigned) EC_GROUP_get_degree(EC_KEY_get0_group(ec_key));

            EC_KEY_free(ec_key);

            /* Get the sig from the DER encoded version. */
            ec_sig = d2i_ECDSA_SIG(nullptr, (const unsigned char**) &sig, (long) slen);
            if (ec_sig == nullptr) SIGN_ERROR(ENOMEM);

            ECDSA_SIG_get0(ec_sig, &ec_sig_r, &ec_sig_s);
            r_len = (unsigned) BN_num_bytes(ec_sig_r);
            s_len = (unsigned) BN_num_bytes(ec_sig_s);
            bn_len = (degree + 7) / 8;
            if ((r_len > bn_len) || (s_len > bn_len)) SIGN_ERROR(EINVAL);

            buf_len = 2 * bn_len;
            Buffer raw_buf_buffer(buf_len);
            raw_buf = (unsigned char*) raw_buf_buffer.data();
            if (raw_buf == nullptr) SIGN_ERROR(ENOMEM);

            /* Pad the bignums with leading zeroes. */
            memset(raw_buf, 0, buf_len);
            BN_bn2bin(ec_sig_r, raw_buf + bn_len - r_len);
            BN_bn2bin(ec_sig_s, raw_buf + buf_len - s_len);

            *out = (char*) malloc(buf_len);
            if (*out == nullptr) SIGN_ERROR(ENOMEM);
            memcpy(*out, raw_buf, buf_len);
            *len = buf_len;
        }
    }
    catch (const exception& e) {
        error = e.what();
    }

    if (bufkey)
        BIO_free(bufkey);
    if (pkey)
        EVP_PKEY_free(pkey);
    if (mdctx)
        EVP_MD_CTX_destroy(mdctx);
    if (ec_sig)
        ECDSA_SIG_free(ec_sig);

    if (!error.empty())
        throw Exception("Sign error: " + string(error));
}

#define VERIFY_ERROR(__err) { if ((__err) == EINVAL) throw Exception("Invalid value"); else throw Exception("Can't allocate memory"); }

void JWT::verify_sha_pem(const char* head, const char* sig_b64)
{
    unsigned char* sig = nullptr;
    EVP_MD_CTX* mdctx = nullptr;
    ECDSA_SIG* ec_sig = nullptr;
    BIGNUM* ec_sig_r = nullptr;
    BIGNUM* ec_sig_s = nullptr;
    EVP_PKEY* pkey = nullptr;
    const EVP_MD* alg;
    int type;
    BIO* bufkey = nullptr;
    int slen;

    switch (this->alg) {
        /* RSA */
        case JWT::JWT_ALG_RS256:
            alg = EVP_sha256();
            type = EVP_PKEY_RSA;
            break;
        case JWT::JWT_ALG_RS384:
            alg = EVP_sha384();
            type = EVP_PKEY_RSA;
            break;
        case JWT::JWT_ALG_RS512:
            alg = EVP_sha512();
            type = EVP_PKEY_RSA;
            break;

            /* ECC */
        case JWT::JWT_ALG_ES256:
            alg = EVP_sha256();
            type = EVP_PKEY_EC;
            break;
        case JWT::JWT_ALG_ES384:
            alg = EVP_sha384();
            type = EVP_PKEY_EC;
            break;
        case JWT::JWT_ALG_ES512:
            alg = EVP_sha512();
            type = EVP_PKEY_EC;
            break;

        default:
            throw Exception("Invalid verify algorythm");
    }

    Buffer sig_buffer;
    jwt_b64_decode(sig_buffer, sig_b64);
    sig = (unsigned char*) sig_buffer.data();
    slen = (int) sig_buffer.bytes();

    string error;
    try {
        bufkey = BIO_new_mem_buf((void*) key.c_str(), (int)key.length());
        if (bufkey == nullptr) VERIFY_ERROR(ENOMEM);

        /* This uses OpenSSL's default passphrase callback if needed. The
         * library caller can override this in many ways, all of which are
         * outside of the scope of LibJWT and this is documented in jwt.h. */
        pkey = PEM_read_bio_PUBKEY(bufkey, nullptr, nullptr, nullptr);
        if (pkey == nullptr) VERIFY_ERROR(EINVAL);

        int pkey_type = EVP_PKEY_id(pkey);
        if (pkey_type != type) VERIFY_ERROR(EINVAL);

        /* Convert EC sigs back to ASN1. */
        if (pkey_type == EVP_PKEY_EC) {
            unsigned int degree, bn_len;
            unsigned char* p;
            EC_KEY* ec_key;

            ec_sig = ECDSA_SIG_new();
            if (ec_sig == nullptr) VERIFY_ERROR(ENOMEM);

            /* Get the actual ec_key */
            ec_key = EVP_PKEY_get1_EC_KEY(pkey);
            if (ec_key == nullptr) VERIFY_ERROR(ENOMEM);

            degree = (unsigned) EC_GROUP_get_degree(EC_KEY_get0_group(ec_key));

            EC_KEY_free(ec_key);

            bn_len = (degree + 7) / 8;
            if ((bn_len * 2) != (unsigned) slen) VERIFY_ERROR(EINVAL);

            ec_sig_r = BN_bin2bn(sig, bn_len, nullptr);
            ec_sig_s = BN_bin2bn(sig + bn_len, bn_len, nullptr);
            if (ec_sig_r == nullptr || ec_sig_s == nullptr) VERIFY_ERROR(EINVAL);

            ECDSA_SIG_set0(ec_sig, ec_sig_r, ec_sig_s);
            free(sig);

            slen = i2d_ECDSA_SIG(ec_sig, nullptr);
            sig = (unsigned char*) malloc((unsigned) slen);
            if (sig == nullptr) VERIFY_ERROR(ENOMEM);

            p = sig;
            slen = i2d_ECDSA_SIG(ec_sig, &p);

            if (slen == 0) VERIFY_ERROR(EINVAL);
        }

        mdctx = EVP_MD_CTX_create();
        if (mdctx == nullptr) VERIFY_ERROR(ENOMEM);

        /* Initialize the DigestVerify operation using alg */
        if (EVP_DigestVerifyInit(mdctx, nullptr, alg, nullptr, pkey) != 1) VERIFY_ERROR(EINVAL);

        /* Call update with the message */
        if (EVP_DigestVerifyUpdate(mdctx, head, strlen(head)) != 1) VERIFY_ERROR(EINVAL);

        /* Now check the sig for validity. */
        if (EVP_DigestVerifyFinal(mdctx, sig, (unsigned) slen) != 1) VERIFY_ERROR(EINVAL);
    }
    catch (const exception& e) {
        error = e.what();
    }

    if (bufkey)
        BIO_free(bufkey);
    if (pkey)
        EVP_PKEY_free(pkey);
    if (mdctx)
        EVP_MD_CTX_destroy(mdctx);
    if (sig)
        free(sig);
    if (ec_sig)
        ECDSA_SIG_free(ec_sig);

    if (!error.empty())
        throw Exception("Verify error:" + error);
}

}