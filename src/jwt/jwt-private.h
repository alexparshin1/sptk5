/* Copyright (C) 2015-2017 Ben Collins <ben@cyphre.com>
   This file is part of the JWT C Library

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef JWT_PRIVATE_H
#define JWT_PRIVATE_H

#include <sptk5/cutils>

namespace sptk {

struct jwt
{
	JWT::jwt_alg_t alg;
	unsigned char* key;
	int key_len;
	json::Document* grants;
};

int jwt_verify_sha_hmac(JWT::jwt_t* jwt, const char* head, const char* sig);

int jwt_sign_sha_pem(JWT::jwt_t* jwt, char** out, unsigned int* len,
					 const char* str);

int jwt_verify_sha_pem(JWT::jwt_t* jwt, const char* head, const char* sig_b64);

}

#endif /* JWT_PRIVATE_H */
