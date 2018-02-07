/* Copyright (C) 2015-2018 Ben Collins <ben@cyphre.com>
   This file is part of the JWT C Library

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sptk5/jwt.h>
#include <sstream>

#include "jwt-private.h"
#include "base64.h"
#include "config.h"

using namespace std;
using namespace sptk;


const char * sptk::jwt_alg_str(jwt_alg_t alg)
{
	switch (alg) {
	case JWT_ALG_NONE:
		return "none";
	case JWT_ALG_HS256:
		return "HS256";
	case JWT_ALG_HS384:
		return "HS384";
	case JWT_ALG_HS512:
		return "HS512";
	case JWT_ALG_RS256:
		return "RS256";
	case JWT_ALG_RS384:
		return "RS384";
	case JWT_ALG_RS512:
		return "RS512";
	case JWT_ALG_ES256:
		return "ES256";
	case JWT_ALG_ES384:
		return "ES384";
	case JWT_ALG_ES512:
		return "ES512";
	default:
		return NULL;
	}
}

jwt_alg_t sptk::jwt_str_alg(const char *alg)
{
	if (alg == NULL)
		return JWT_ALG_INVAL;

	if (!strcasecmp(alg, "none"))
		return JWT_ALG_NONE;
	else if (!strcasecmp(alg, "HS256"))
		return JWT_ALG_HS256;
	else if (!strcasecmp(alg, "HS384"))
		return JWT_ALG_HS384;
	else if (!strcasecmp(alg, "HS512"))
		return JWT_ALG_HS512;
	else if (!strcasecmp(alg, "RS256"))
		return JWT_ALG_RS256;
	else if (!strcasecmp(alg, "RS384"))
		return JWT_ALG_RS384;
	else if (!strcasecmp(alg, "RS512"))
		return JWT_ALG_RS512;
	else if (!strcasecmp(alg, "ES256"))
		return JWT_ALG_ES256;
	else if (!strcasecmp(alg, "ES384"))
		return JWT_ALG_ES384;
	else if (!strcasecmp(alg, "ES512"))
		return JWT_ALG_ES512;

	return JWT_ALG_INVAL;
}

static void jwt_scrub_key(jwt_t *jwt)
{
	if (jwt->key) {
		/* Overwrite it so it's gone from memory. */
		memset(jwt->key, 0, jwt->key_len);

		free(jwt->key);
		jwt->key = NULL;
	}

	jwt->key_len = 0;
	jwt->alg = JWT_ALG_NONE;
}

int jwt_set_alg(jwt_t *jwt, jwt_alg_t alg, const unsigned char *key, int len)
{
	/* No matter what happens here, we do this. */
	jwt_scrub_key(jwt);

	if (alg < JWT_ALG_NONE || alg >= JWT_ALG_INVAL)
		return EINVAL;

	switch (alg) {
	case JWT_ALG_NONE:
		if (key || len)
			return EINVAL;
		break;

	default:
		if (!key || len <= 0)
			return EINVAL;

		jwt->key = (unsigned char*) malloc(len);
		if (!jwt->key)
			return ENOMEM;

		memcpy(jwt->key, key, len);
	}

	jwt->alg = alg;
	jwt->key_len = len;

	return 0;
}

jwt_alg_t jwt_get_alg(jwt_t *jwt)
{
	return jwt->alg;
}

int sptk::jwt_new(jwt_t **jwt)
{
	if (!jwt)
		return EINVAL;

	*jwt = (jwt_t*) malloc(sizeof(jwt_t));
	if (!*jwt)
		return ENOMEM;

	memset(*jwt, 0, sizeof(jwt_t));

	(*jwt)->grants = new json::Document(true);
	if (!(*jwt)->grants) {
		free(*jwt);
		*jwt = NULL;
		return ENOMEM;
	}

	return 0;
}

void sptk::jwt_free(jwt_t *jwt)
{
	if (!jwt)
		return;

	jwt_scrub_key(jwt);

	delete jwt->grants;

	free(jwt);
}

jwt_t *jwt_dup(jwt_t *jwt)
{
	jwt_t *newJWT = NULL;

	if (!jwt) {
		errno = EINVAL;
		return NULL;
	}

	errno = 0;

	newJWT = (jwt_t*) malloc(sizeof(jwt_t));
	if (!newJWT) {
		errno = ENOMEM;
		return NULL;
	}

	memset(newJWT, 0, sizeof(jwt_t));

	if (jwt->key_len) {
		newJWT->alg = jwt->alg;
		newJWT->key = (unsigned char*) malloc(jwt->key_len);
		if (!newJWT->key) {
			errno = ENOMEM;
            jwt_free(newJWT);
            return NULL;
		}
		memcpy(newJWT->key, jwt->key, jwt->key_len);
		newJWT->key_len = jwt->key_len;
	}

    Buffer tempBuffer;
    jwt->grants->exportTo(tempBuffer, false);
	newJWT->grants = new json::Document(true);
    if (!newJWT->grants) {
        errno = ENOMEM;
        jwt_free(newJWT);
        return NULL;
    }

    jwt->grants->load(tempBuffer.c_str());

	return newJWT;
}

static const char *get_js_string(json::Element *js, const char *key)
{
    json::Element *element = js;
    if (js->isObject())
        element = js->find(key);

    if (element != nullptr && js->isString())
        return js->getString().c_str();

    errno = ENOENT;
    return nullptr;
}

static long get_js_int(json::Element *js, const char *key)
{
    json::Element *element = js;
    if (js->isObject())
        element = js->find(key);

    if (element != nullptr && js->isString())
        return js->getNumber();

    errno = ENOENT;
    return -1;
}

static int get_js_bool(json::Element *js, const char *key)
{
    json::Element *element = js;
    if (js->isObject())
        element = js->find(key);

    if (element != nullptr && js->isBoolean())
        return js->isBoolean() ? 1 : 0;

    errno = ENOENT;
    return -1;
}

void * sptk::jwt_b64_decode(const char *src, int *ret_len)
{
	void *buf;
	char *newData;
	int len, i, z;

	/* Decode based on RFC-4648 URI safe encoding. */
	len = strlen(src);
	newData = (char*) alloca(len + 4);
	if (!newData)
		return NULL;

	for (i = 0; i < len; i++) {
		switch (src[i]) {
		case '-':
			newData[i] = '+';
			break;
		case '_':
			newData[i] = '/';
			break;
		default:
			newData[i] = src[i];
		}
	}
	z = 4 - (i % 4);
	if (z < 4) {
		while (z--)
			newData[i++] = '=';
	}
	newData[i] = '\0';

	buf = malloc(i);
	if (buf == NULL)
		return NULL;

	*ret_len = jwt_Base64decode((char *)buf, newData);

	return buf;
}


static sptk::json::Document *jwt_b64_decode_json(const char *src)
{
	char *buf;
	int len;

	buf = (char*) jwt_b64_decode(src, &len);

	if (buf == NULL)
		return NULL;

	buf[len] = '\0';

	json::Document *js = new json::Document(true);
    js->load(buf);

	free(buf);

	return js;
}

void sptk::jwt_base64uri_encode(char *str)
{
	int len = strlen(str);
	int i, t;

	for (i = t = 0; i < len; i++) {
		switch (str[i]) {
		case '+':
			str[t++] = '-';
			break;
		case '/':
			str[t++] = '_';
			break;
		case '=':
			break;
		default:
			str[t++] = str[i];
		}
	}

	str[t] = '\0';
}

static int jwt_sign(jwt_t *jwt, char **out, unsigned int *len, const char *str)
{
	switch (jwt->alg) {
	/* HMAC */
	case JWT_ALG_HS256:
	case JWT_ALG_HS384:
	case JWT_ALG_HS512:
		return jwt_sign_sha_hmac(jwt, out, len, str);

	/* RSA */
	case JWT_ALG_RS256:
	case JWT_ALG_RS384:
	case JWT_ALG_RS512:

	/* ECC */
	case JWT_ALG_ES256:
	case JWT_ALG_ES384:
	case JWT_ALG_ES512:
		return jwt_sign_sha_pem(jwt, out, len, str);

	/* You wut, mate? */
	default:
		return EINVAL;
	}
}

static int jwt_verify(jwt_t *jwt, const char *head, const char *sig)
{
	switch (jwt->alg) {
	/* HMAC */
	case JWT_ALG_HS256:
	case JWT_ALG_HS384:
	case JWT_ALG_HS512:
		return jwt_verify_sha_hmac(jwt, head, sig);

	/* RSA */
	case JWT_ALG_RS256:
	case JWT_ALG_RS384:
	case JWT_ALG_RS512:

	/* ECC */
	case JWT_ALG_ES256:
	case JWT_ALG_ES384:
	case JWT_ALG_ES512:
		return jwt_verify_sha_pem(jwt, head, sig);

	/* You wut, mate? */
	default:
		return EINVAL;
	}
}

static int jwt_parse_body(jwt_t *jwt, char *body)
{
	if (jwt->grants) {
		delete jwt->grants;
		jwt->grants = NULL;
	}

	jwt->grants = jwt_b64_decode_json(body);
	if (!jwt->grants)
		return EINVAL;

	return 0;
}

static int jwt_verify_head(jwt_t *jwt, char *head)
{
	const char *val;
	int ret = 0;

	json::Document* jsdoc = jwt_b64_decode_json(head);
    json::Element* js = &jsdoc->root();
	if (!js)
		return EINVAL;

	val = get_js_string(js, "alg");
	jwt->alg = jwt_str_alg(val);
	if (jwt->alg == JWT_ALG_INVAL) {
		ret = EINVAL;
		goto verify_head_done;
	}

	if (jwt->alg != JWT_ALG_NONE) {
		/* If alg is not NONE, there may be a typ. */
		val = get_js_string(js, "typ");
		if (val && strcasecmp(val, "JWT"))
			ret = EINVAL;

		if (jwt->key) {
			if (jwt->key_len <= 0)
				ret = EINVAL;
		} else {
			jwt_scrub_key(jwt);
		}
	} else {
		/* If alg is NONE, there should not be a key */
		if (jwt->key){
			ret = EINVAL;
		}
	}

verify_head_done:
    delete jsdoc;

	return ret;
}

int jwt_decode(jwt_t **jwt, const char *token, const unsigned char *key,
	       int key_len)
{
	char *head = strdup(token);
	jwt_t *newData = NULL;
	char *body, *sig;
	int ret = EINVAL;

	if (!jwt)
		return EINVAL;

	*jwt = NULL;

	if (!head)
		return ENOMEM;

	/* Find the components. */
	for (body = head; body[0] != '.'; body++) {
		if (body[0] == '\0')
			goto decode_done;
	}

	body[0] = '\0';
	body++;

	for (sig = body; sig[0] != '.'; sig++) {
		if (sig[0] == '\0')
			goto decode_done;
	}

	sig[0] = '\0';
	sig++;

	/* Now that we have everything split up, let's check out the
	 * header. */
	ret = jwt_new(&newData);
	if (ret) {
		goto decode_done;
	}

	/* Copy the key over for verify_head. */
	if (key_len) {
		newData->key = (unsigned char*) malloc(key_len);
		if (newData->key == NULL)
			goto decode_done;
		memcpy(newData->key, key, key_len);
		newData->key_len = key_len;
	}

	ret = jwt_verify_head(newData, head);
	if (ret)
		goto decode_done;

	ret = jwt_parse_body(newData, body);
	if (ret)
		goto decode_done;

	/* Check the signature, if needed. */
	if (newData->alg != JWT_ALG_NONE) {
		/* Re-add this since it's part of the verified data. */
		body[-1] = '.';
		ret = jwt_verify(newData, head, sig);
	} else {
		ret = 0;
	}

decode_done:
	if (ret)
		jwt_free(newData);
	else
		*jwt = newData;

	free(head);

	return ret;
}

const char *jwt_get_grant(jwt_t *jwt, const char *grant)
{
	if (!jwt || !grant || !strlen(grant)) {
		errno = EINVAL;
		return NULL;
	}

	errno = 0;

	return get_js_string(&jwt->grants->root(), grant);
}

long jwt_get_grant_int(jwt_t *jwt, const char *grant)
{
	if (!jwt || !grant || !strlen(grant)) {
		errno = EINVAL;
		return 0;
	}

	errno = 0;

	return get_js_int(&jwt->grants->root(), grant);
}

int jwt_get_grant_bool(jwt_t *jwt, const char *grant)
{
	if (!jwt || !grant || !strlen(grant)) {
		errno = EINVAL;
		return 0;
	}

	errno = 0;

	return get_js_bool(&jwt->grants->root(), grant);
}

string jwt_get_grants_json(jwt_t *jwt, const char *grant)
{
	json::Element *js_val = NULL;

	errno = EINVAL;

	if (!jwt)
		return NULL;

	if (grant && strlen(grant))
		js_val = jwt->grants->root().find(grant);
	else
		js_val = &jwt->grants->root();

	if (js_val == NULL)
		return NULL;

	errno = 0;

    stringstream output;
	js_val->exportTo(output, false);
    return output.str();
}

int jwt_add_grant(jwt_t *jwt, const char *grant, const char *val)
{
	if (!jwt || !grant || !strlen(grant) || !val)
		return EINVAL;

    json::Element* grants = &jwt->grants->root();

	if (get_js_string(grants, grant) != NULL)
		return EEXIST;

    grants->add(grant, val);

	return 0;
}

int jwt_add_grant_int(jwt_t *jwt, const char *grant, long val)
{
	if (!jwt || !grant || !strlen(grant))
		return EINVAL;

    json::Element* grants = &jwt->grants->root();

    if (get_js_int(grants, grant) != -1)
		return EEXIST;

    grants->add(grant, double(val));

	return 0;
}

int jwt_add_grant_bool(jwt_t *jwt, const char *grant, int val)
{
	if (!jwt || !grant || !strlen(grant))
		return EINVAL;

    json::Element* grants = &jwt->grants->root();

    if (get_js_int(grants, grant) != -1)
		return EEXIST;

    grants->add(grant, bool(val));

	return 0;
}

int jwt_add_grants_json(jwt_t *jwt, const char *json)
{
	if (!jwt)
		return EINVAL;

    json::Document newGrantsDoc(true);
    newGrantsDoc.load(json);
    json::ObjectData& newGrantsObject = newGrantsDoc.root().getObject();
    for (auto itor: newGrantsObject) {
        json::Element* existingGrant = jwt->grants->root().find(itor.first);
        if (existingGrant)
            newGrantsObject.remove(itor.first);
        jwt->grants->root().add(itor.first, itor.second);
    }

	return 0;
}

int sptk::jwt_del_grants(jwt_t *jwt, const char *grant)
{
	if (!jwt)
		return EINVAL;

	if (grant == NULL || !strlen(grant))
		jwt->grants->clear();
	else
		jwt->grants->root().remove(grant);

	return 0;
}

int sptk::jwt_del_grant(jwt_t *jwt, const char *grant)
{
	return jwt_del_grants(jwt, grant);
}

static int __append_str(char **buf, const char *str)
{
	char *newData;

	if (*buf == NULL) {
		newData = (char *) calloc(1, strlen(str) + 1);
	} else {
		newData = (char *) realloc(*buf, strlen(*buf) + strlen(str) + 1);
	}

	if (newData == NULL)
		return ENOMEM;

	strcat(newData, str);

	*buf = newData;

	return 0;
}

#define APPEND_STR(__buf, __str) do {		\
	int ret = __append_str(__buf, __str);	\
	if (ret)				\
		return ret;			\
} while(0)

static int jwt_write_head(jwt_t *jwt, char **buf, int pretty)
{
	APPEND_STR(buf, "{");

	if (pretty)
		APPEND_STR(buf, "\n");

	/* An unsecured JWT is a JWS and provides no "typ".
	 * -- draft-ietf-oauth-json-web-token-32 #6. */
	if (jwt->alg != JWT_ALG_NONE) {
		if (pretty)
			APPEND_STR(buf, "    ");

		APPEND_STR(buf, "\"typ\":");
		if (pretty)
			APPEND_STR(buf, " ");
		APPEND_STR(buf, "\"JWT\",");

		if (pretty)
			APPEND_STR(buf, "\n");
	}

	if (pretty)
		APPEND_STR(buf, "    ");

	APPEND_STR(buf, "\"alg\":");
	if (pretty)
		APPEND_STR(buf, " ");
	APPEND_STR(buf, "\"");
	APPEND_STR(buf, jwt_alg_str(jwt->alg));
	APPEND_STR(buf, "\"");

	if (pretty)
		APPEND_STR(buf, "\n");

	APPEND_STR(buf, "}");

	if (pretty)
		APPEND_STR(buf, "\n");

	return 0;
}

static int jwt_write_body(jwt_t *jwt, char **buf, int pretty)
{
    stringstream output;
	jwt->grants->exportTo(output, pretty);
	return 0;
}

static int jwt_dump(jwt_t *jwt, char **buf, int pretty)
{
	int ret;

	ret = jwt_write_head(jwt, buf, pretty);

	if (ret == 0)
		ret = __append_str(buf, ".");

	if (ret == 0)
		ret = jwt_write_body(jwt, buf, pretty);

	return ret;
}

int jwt_dump_fp(jwt_t *jwt, FILE *fp, int pretty)
{
	char *out = NULL;
	int ret = 0;

	ret = jwt_dump(jwt, &out, pretty);

	if (ret == 0)
		fputs(out, fp);

	if (out)
		free(out);

	return ret;
}

char *jwt_dump_str(jwt_t *jwt, int pretty)
{
	char *out = NULL;
	int err;

	err = jwt_dump(jwt, &out, pretty);

	if (err) {
		errno = err;
		if (out)
			free(out);
		out = NULL;
	} else {
		errno = 0;
	}

	return out;
}

static int jwt_encode(jwt_t *jwt, char **out)
{
	char *buf = NULL, *head, *body, *sig;
	int ret, head_len, body_len;
	unsigned int sig_len;

	/* First the header. */
	ret = jwt_write_head(jwt, &buf, 0);
	if (ret) {
		if (buf)
			free(buf);
		return ret;
	}

	head = (char *) alloca(strlen(buf) * 2);
	if (head == NULL) {
		free(buf);
		return ENOMEM;
	}
	jwt_Base64encode(head, buf, strlen(buf));
	head_len = strlen(head);

	free(buf);
	buf = NULL;

	/* Now the body. */
	ret = jwt_write_body(jwt, &buf, 0);
	if (ret) {
		if (buf)
			free(buf);
		return ret;
	}

	body = (char *) alloca(strlen(buf) * 2);
	if (body == NULL) {
		free(buf);
		return ENOMEM;
	}
	jwt_Base64encode(body, buf, strlen(buf));
	body_len = strlen(body);

	free(buf);
	buf = NULL;

	jwt_base64uri_encode(head);
	jwt_base64uri_encode(body);

	/* Allocate enough to reuse as b64 buffer. */
	buf = (char *) malloc(head_len + body_len + 2);
	if (buf == NULL)
		return ENOMEM;
	strcpy(buf, head);
	strcat(buf, ".");
	strcat(buf, body);

	ret = __append_str(out, buf);
	if (ret == 0)
		ret = __append_str(out, ".");
	if (ret) {
		if (buf)
			free(buf);
		return ret;
	}

	if (jwt->alg == JWT_ALG_NONE) {
		free(buf);
		return 0;
	}

	/* Now the signature. */
	ret = jwt_sign(jwt, &sig, &sig_len, buf);
	free(buf);

	if (ret)
		return ret;

	buf = (char *) malloc(sig_len * 2);
	if (buf == NULL) {
		free(sig);
		return ENOMEM;
	}

	jwt_Base64encode(buf, sig, sig_len);

	free(sig);

	jwt_base64uri_encode(buf);
	ret = __append_str(out, buf);
	free(buf);

	return ret;
}

int jwt_encode_fp(jwt_t *jwt, FILE *fp)
{
	char *str = NULL;
	int ret;

	ret = jwt_encode(jwt, &str);
	if (ret) {
		if (str)
			free(str);
		return ret;
	}

	fputs(str, fp);
	free(str);

	return 0;
}

char *jwt_encode_str(jwt_t *jwt)
{
	char *str = NULL;

	errno = jwt_encode(jwt, &str);
	if (errno) {
		if (str)
			free(str);
		str = NULL;
	}

	return str;
}
