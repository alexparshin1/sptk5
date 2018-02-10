/* Copyright (C) 2015-2018 Ben Collins <ben@cyphre.com>
   This file is part of the JWT C Library

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/**
 * @file jwt.h, jwt->key_len);
        newJWT->key_len = jwt->key_len
 * @brief JWT C Library
 */

#ifndef JWT_H
#define JWT_H

#include <cstdio>
#include <fstream>
#include <sptk5/Strings.h>
#include <sptk5/JWT.h>

namespace sptk {

/**
 * @defgroup jwt_new JWT Object Creation
 * Functions used to create and destroy JWT objects.
 *
 * Generally, one would use the jwt_new() function to create an object
 * from scratch and jwt_decode() to create and verify and object from an
 * existing token.
 *
 * Note, when using RSA keys (e.g. with RS256), the key is expected to be
 * a private key in PEM format. If the RSA private key requires a passphrase,
 * the default is to request it on the command line from stdin. However,
 * you can override this using OpenSSL's default_passwd routines. For
 * example, using SSL_CTX_set_default_passwd_cb().
 * @{
 */

/**
 * Verify an existing JWT and allocate a new JWT object from it.
 *
 * Decodes a JWT string and verifies the signature (if one is supplied).
 * If no signature is used (JWS, alg="none") or key is NULL, then no
 * validation is done other than formatting. It is not suggested to use
 * this on a string that has a signature without passing the key to
 * verify it. If the JWT is encrypted and no key is supplied, an error
 * is returned.
 *
 * @param jwt Pointer to a JWT object pointer. Will be allocated on
 *     success.
 * @param token Pointer to a valid JWT string, nul terminated.
 * @param key Pointer to the key for validating the JWT signature or for
 *     decrypting the token or empty string if no validation is to be performed.
 * @return 0 on success, valid errno otherwise.
 *
 * @remark If a key is supplied, the token must pass sig check or decrypt
 *     for it to be parsed without error. If no key is supplied, then a
 *     non-encrypted token will be parsed without any checks for a valid
 *     signature, however, standard validation of the token is still
 *     performed.
 */
void jwt_decode(JWT **jwt, const char *token, const String& key="");

/** @} */

/**
 * Return the value of a grant as JSON encoded object string.
 *
 * Returns the JSON encoded string value for a grant (e.g. "iss"). If it
 * does not exist, NULL will be returned.
 *
 * @param jwt Pointer to a JWT object.
 * @param grant String containing the name of the grant to return a value
 *     for. If this is NULL, all grants will be returned as a JSON encoded
 *     hash.
 * @return Returns a string for the value, or NULL when not found. The
 *     returned string must be freed by the caller.
 */
String jwt_get_grants_json(JWT *jwt, const char *grant);

/**
 * Add a new string grant to this JWT object.
 *
 * Creates a new grant for this object. The string for grant and val
 * are copied internally, so do not require that the pointer or string
 * remain valid for the lifetime of this object. It is an error if you
 * try to add a grant that already exists.
 *
 * @param jwt Pointer to a JWT object.
 * @param grant String containing the name of the grant to add.
 * @param val String containing the value to be saved for grant. Can be
 *     an empty string, but cannot be NULL.
 * @return Returns 0 on success, valid errno otherwise.
 *
 * Note, this only allows for string based grants. If you wish to add
 * integer grants, then use jwt_add_grant_int(). If you wish to add more
 * complex grants (e.g. an array), then use jwt_add_grants_json().
 */
int jwt_add_grant(JWT *jwt, const char *grant, const char *val);

/**
 * Add a new integer grant to this JWT object.
 *
 * Creates a new grant for this object. The string for grant
 * is copied internally, so do not require that the pointer or string
 * remain valid for the lifetime of this object. It is an error if you
 * try to add a grant that already exists.
 *
 * @param jwt Pointer to a JWT object.
 * @param grant String containing the name of the grant to add.
 * @param val int containing the value to be saved for grant.
 * @return Returns 0 on success, valid errno otherwise.
 *
 * Note, this only allows for integer based grants. If you wish to add
 * string grants, then use jwt_add_grant(). If you wish to add more
 * complex grants (e.g. an array), then use jwt_add_grants_json().
 */
int jwt_add_grant_int(JWT *jwt, const char *grant, long val);

/**
 * Add a new boolean grant to this JWT object.
 *
 * Creates a new grant for this object. The string for grant
 * is copied internally, so do not require that the pointer or string
 * remain valid for the lifetime of this object. It is an error if you
 * try to add a grant that already exists.
 *
 * @param jwt Pointer to a JWT object.
 * @param grant String containing the name of the grant to add.
 * @param val boolean containing the value to be saved for grant.
 * @return Returns 0 on success, valid errno otherwise.
 *
 * Note, this only allows for boolean based grants. If you wish to add
 * string grants, then use jwt_add_grant(). If you wish to add more
 * complex grants (e.g. an array), then use jwt_add_grants_json().
 */
int jwt_add_grant_bool(JWT *jwt, const char *grant, int val);

/**
 * Add grants from a JSON encoded object string.
 *
 * Loads a grant from an existing JSON encoded object string. Overwrites
 * existing grant. If grant is NULL, then the JSON encoded string is
 * assumed to be a JSON hash of all grants being added and will be merged
 * into the grant listing.
 *
 * @param jwt Pointer to a JWT object.
 * @param json String containing a JSON encoded object of grants.
 * @return Returns 0 on success, valid errno otherwise.
 */
int jwt_add_grants_json(JWT *jwt, const char *json);

/** @} */

}

#endif /* JWT_H */
