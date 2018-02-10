/* Public domain, no copyright. Use at your own risk. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <sptk5/sptk.h>
#include <sptk5/Exception.h>
#include <sptk5/jwt.h>
#include <iostream>
#include <src/jwt/jwt-private.h>

using namespace std;
using namespace sptk;

void test_jwt_new()
{
	jwt_t *newJWT = NULL;
	jwt_new(&newJWT);
    if (newJWT == NULL) {
        throw Exception(string(__PRETTY_FUNCTION__) + " Can't create JWT object");
    }
	jwt_free(newJWT);
}

void test_jwt_dup()
{
	jwt_t *jwt = NULL, *newJWT = NULL;
	int ret = 0;
	time_t now;
	long valint;

	ret = jwt_new(&jwt);
    if (jwt == NULL) {
        throw Exception(string(__PRETTY_FUNCTION__) + " Can't create JWT object");
    }

	ret = jwt_add_grant(jwt, "iss", "test");
    if (ret != 0) {
        throw Exception(string(__PRETTY_FUNCTION__) + " Can't add grant");
    }

    string val = jwt_get_grant(jwt, "iss");
    if (val.empty()) {
        throw Exception(string(__PRETTY_FUNCTION__) + " Can't get grant");
    }

	newJWT = jwt_dup(jwt);
    if (newJWT == NULL) {
        throw Exception(string(__PRETTY_FUNCTION__) + " Can't duplicate JWT object");
    }

	val = jwt_get_grant(newJWT, "iss");
	if (val.empty()) {
        throw Exception(string(__PRETTY_FUNCTION__) + " Can't get grant");
    }

    if (val != "test") {
        throw Exception(string(__PRETTY_FUNCTION__) + " Got incorrect grant");
    }

	if (jwt_get_alg(newJWT) != JWT_ALG_NONE) {
        throw Exception(string(__PRETTY_FUNCTION__) + " Got incorrect alogorithm");
    }

	now = time(NULL);
	ret = jwt_add_grant_int(jwt, "iat", (long)now);
    if (ret != 0) {
        throw Exception(string(__PRETTY_FUNCTION__) + " Failed jwt_add_grant_int()");
    }

	valint = jwt_get_grant_int(jwt, "iat");
    if (((long)now) != valint) {
        throw Exception(string(__PRETTY_FUNCTION__) + " Failed jwt_get_grant_int()");
    }

	jwt_free(newJWT);
	jwt_free(jwt);
}


void test_jwt_dup_signed()
{
	String key256("012345678901234567890123456789XY");
	jwt_t *jwt = NULL, *newJWT = NULL;
	int ret = 0;

	ret = jwt_new(&jwt);
    if (jwt == NULL) {
        throw Exception(string(__PRETTY_FUNCTION__) + " Can't create JWT object");
    }

	ret = jwt_add_grant(jwt, "iss", "test");
    if (ret != 0) {
        throw Exception(string(__PRETTY_FUNCTION__) + " Failed jwt_add_grant_int()");
    }

	ret = jwt_set_alg(jwt, JWT_ALG_HS256, key256);
    if (ret != 0) {
        throw Exception(string(__PRETTY_FUNCTION__) + " Failed jwt_set_alg()");
    }

	newJWT = jwt_dup(jwt);
    if (newJWT == NULL) {
        throw Exception(string(__PRETTY_FUNCTION__) + " Can't create JWT object");
    }

	string val = jwt_get_grant(newJWT, "iss");
    if (val != "test") {
        throw Exception(string(__PRETTY_FUNCTION__) + " Failed jwt_get_grant_int()");
    }

    if (jwt_get_alg(newJWT) != JWT_ALG_HS256) {
        throw Exception(string(__PRETTY_FUNCTION__) + " Failed jwt_get_alg()");
    }

	jwt_free(newJWT);
	jwt_free(jwt);
}


void test_jwt_decode()
{
	const char token[] =
            "eyJhbGciOiJub25lIn0.eyJpc3MiOiJmaWxlcy5jeXBo"
            "cmUuY29tIiwic3ViIjoidXNlcjAifQ.";
	jwt_alg_t alg;
	jwt_t *jwt;

    try {
        jwt_decode(&jwt, token);
    }
    catch (const exception& e) {
        throw Exception(string(__PRETTY_FUNCTION__) + " Failed jwt_decode(): " + string(e.what()));
    }

	alg = jwt_get_alg(jwt);
	if (alg != JWT_ALG_NONE) {
        throw Exception(string(__PRETTY_FUNCTION__) + " Failed jwt_get_alg()");
    }

	jwt_free(jwt);
}


void test_jwt_decode_invalid_final_dot()
{
	const char token[] = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzM4NCJ9."
			     "eyJpc3MiOiJmaWxlcy5jeXBocmUuY29tIiwic"
			     "3ViIjoidXNlcjAifQ";
	jwt_t *jwt;

    try {
        jwt_decode(&jwt, token);
        throw Exception(string(__PRETTY_FUNCTION__) + " Not failed jwt_decode()");
    }
    catch (...) {
    }

	jwt_free(jwt);
}


void test_jwt_decode_invalid_alg()
{
	const char token[] = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIQUhBSCJ9."
			     "eyJpc3MiOiJmaWxlcy5jeXBocmUuY29tIiwic"
			     "3ViIjoidXNlcjAifQ.";
	jwt_t *jwt;

    try {
        jwt_decode(&jwt, token);
        throw Exception(string(__PRETTY_FUNCTION__) + " Not failed jwt_decode()");
    }
    catch (...) {
    }

	jwt_free(jwt);
}


void test_jwt_decode_invalid_typ()
{
	const char token[] = "eyJ0eXAiOiJBTEwiLCJhbGciOiJIUzI1NiJ9."
			     "eyJpc3MiOiJmaWxlcy5jeXBocmUuY29tIiwic"
			     "3ViIjoidXNlcjAifQ.";
	jwt_t *jwt;

    try {
        jwt_decode(&jwt, token);
        throw Exception(string(__PRETTY_FUNCTION__) + " Not failed jwt_decode()");
    }
    catch (...) {
    }

	jwt_free(jwt);
}


void test_jwt_decode_invalid_head()
{
	const char token[] = "yJ0eXAiOiJKV1QiLCJhbGciOiJIUzM4NCJ9."
			     "eyJpc3MiOiJmaWxlcy5jeXBocmUuY29tIiwic"
			     "3ViIjoidXNlcjAifQ.";
	jwt_t *jwt;

    try {
        jwt_decode(&jwt, token);
        throw Exception(string(__PRETTY_FUNCTION__) + " Not failed jwt_decode()");
    }
    catch (...) {
    }

	jwt_free(jwt);
}


void test_jwt_decode_alg_none_with_key()
{
	const char token[] = "eyJhbGciOiJub25lIn0."
			     "eyJpc3MiOiJmaWxlcy5jeXBocmUuY29tIiwic"
			     "3ViIjoidXNlcjAifQ.";
	jwt_t *jwt;

    try {
        jwt_decode(&jwt, token);
        throw Exception(string(__PRETTY_FUNCTION__) + " Not failed jwt_decode()");
    }
    catch (...) {
    }

	jwt_free(jwt);
}


void test_jwt_decode_invalid_body()
{
	const char token[] = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzM4NCJ9."
			     "eyJpc3MiOiJmaWxlcy5jeBocmUuY29tIiwic"
			     "3ViIjoidXNlcjAifQ.";
	jwt_t *jwt;

    try {
        jwt_decode(&jwt, token);
        throw Exception(string(__PRETTY_FUNCTION__) + " Not failed jwt_decode()");
    }
    catch (...) {
    }

	jwt_free(jwt);
}


void test_jwt_decode_hs256()
{
	const char token[] = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJpc3Mi"
			     "OiJmaWxlcy5jeXBocmUuY29tIiwic3ViIjoidXNlcjAif"
			     "Q.dLFbrHVViu1e3VD1yeCd9aaLNed-bfXhSsF0Gh56fBg";
	String key256("012345678901234567890123456789XY");
	jwt_t *jwt;

    try {
        jwt_decode(&jwt, token, key256);
    }
    catch (const exception& e) {
        throw Exception(string(__PRETTY_FUNCTION__) + " Failed jwt_decode(): " + string(e.what()));
    }

	jwt_free(jwt);
}


void test_jwt_decode_hs384()
{
	const char token[] =
            "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzM4NCJ9."
            "eyJpc3MiOiJmaWxlcy5jeXBocmUuY29tIiwic"
            "3ViIjoidXNlcjAifQ.xqea3OVgPEMxsCgyikr"
            "R3gGv4H2yqMyXMm7xhOlQWpA-NpT6n2a1d7TD"
            "GgU6LOe4";
	String key384(
            "aaaabbbbccccddddeeeeffffg"
			"ggghhhhiiiijjjjkkkkllll");
	jwt_t *jwt;

    try {
        jwt_decode(&jwt, token, key384);
    }
    catch (const exception& e) {
        throw Exception(string(__PRETTY_FUNCTION__) + " Failed jwt_decode(): " + string(e.what()));
    }

	jwt_free(jwt);
}


void test_jwt_decode_hs512()
{
    const char token[] =
            "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzUxMiJ9.eyJpc3Mi"
            "OiJmaWxlcy5jeXBocmUuY29tIiwic3ViIjoidXNlcjAif"
            "Q.u-4XQB1xlYV8SgAnKBof8fOWOtfyNtc1ytTlc_vHo0U"
            "lh5uGT238te6kSacnVzBbC6qwzVMT1806oa1Y8_8EOg";
	String key512(
            "012345678901234567890123456789XY"
			"012345678901234567890123456789XY");
	jwt_t *jwt;

    try {
	    jwt_decode(&jwt, token, key512);
    }
    catch (const exception& e) {
        throw Exception(string(__PRETTY_FUNCTION__) + " Failed jwt_decode(): " + string(e.what()));
    }

	jwt_free(jwt);
}


int main(int argc, char *argv[])
{
/*
    jwt_t *jwt;
    const char* token = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJpc3MiOiJodHRwOi8va2Fycm9zdGVjaC5pbyIsInN1YiI6ImM0Y2ZjODIyLWM5ZjAtNGExMS05ZmFkLTJhNzJlYWVlMzM1NyIsImF1ZCI6Ijc1NzNjMjZiMTY5OTNlZjUzNmRjOTM5N2Y1YzA4OTExYTU1MWM3OWEiLCJpYXQiOjE1MTgxNTQxODIsImV4cCI6MTUxODE1Nzc4Miwic2NvcGUiOlsiRURQOkVkdWxvZzpBZG1pbiJdfQ.6DDnBtXAcYalH0NRnUnSRMNLFmjugId6RI9IyHCOQ_8";

    DateTime dt = DateTime::Now();
    chrono::seconds sec = chrono::duration_cast<chrono::seconds>(dt.sinceEpoch());
    cout << "now: " << sec.count() << endl;
    try {
        jwt_decode(&jwt, token, nullptr, 0);
        jwt->grants->exportTo(cout);
    }
    catch (const exception& e) {
        throw Exception(string(__PRETTY_FUNCTION__) + " Failed jwt_decode(): " + string(e.what()));
    }
*/
    try {
        test_jwt_new();
        test_jwt_dup();
        test_jwt_dup_signed();
        test_jwt_decode();
        test_jwt_decode_invalid_alg();
        test_jwt_decode_invalid_typ();
        test_jwt_decode_invalid_head();
        test_jwt_decode_alg_none_with_key();
        test_jwt_decode_invalid_body();
        test_jwt_decode_invalid_final_dot();
        test_jwt_decode_hs256();
        test_jwt_decode_hs384();
        test_jwt_decode_hs512();
    }
    catch (const exception& e) {
        cerr << "ERROR:" << endl;
        cerr << e.what() << endl;
        return 1;
    }

    cout << "All tests passed." << endl;
    return 0;
}
