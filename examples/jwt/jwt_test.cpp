/* Public domain, no copyright. Use at your own risk. */

#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <sptk5/sptk.h>
#include <sptk5/Exception.h>
#include <sptk5/jwt.h>
#include <iostream>
#include <sptk5/JWT.h>

using namespace std;
using namespace sptk;

void test_jwt_dup()
{
	time_t now;
	long valint;

	auto jwt = new JWT;

	jwt->add_grant("iss", "test");
    String val = jwt->get_grant("iss");
    if (val.empty()) {
        throw Exception(string(__PRETTY_FUNCTION__) + " Can't get grant");
    }

	auto newJWT = jwt->clone();
    if (newJWT == nullptr) {
        throw Exception(string(__PRETTY_FUNCTION__) + " Can't duplicate JWT object");
    }

	val = newJWT->get_grant("iss");
	if (val.empty()) {
        throw Exception(string(__PRETTY_FUNCTION__) + " Can't get grant");
    }

    if (val != "test") {
        throw Exception(string(__PRETTY_FUNCTION__) + " Got incorrect grant");
    }

	if (jwt->get_alg() != JWT::JWT_ALG_NONE) {
        throw Exception(string(__PRETTY_FUNCTION__) + " Got incorrect alogorithm");
    }

	now = time(nullptr);
	jwt->add_grant_int("iat", (long)now);

	valint = jwt->get_grant_int("iat");
    if (((long)now) != valint) {
        throw Exception(string(__PRETTY_FUNCTION__) + " Failed jwt_get_grant_int()");
    }

	delete newJWT;
	delete jwt;
}


void test_jwt_dup_signed()
{
	String key256("012345678901234567890123456789XY");

	auto jwt = new JWT;
	jwt->add_grant("iss", "test");
	jwt->set_alg(JWT::JWT_ALG_HS256, key256);

	auto newJWT = jwt->clone();
	String val = newJWT->get_grant("iss");
    if (val != "test") {
        throw Exception(string(__PRETTY_FUNCTION__) + " Failed jwt_get_grant_int()");
    }

    if (jwt->get_alg() != JWT::JWT_ALG_HS256) {
        throw Exception(string(__PRETTY_FUNCTION__) + " Failed jwt_get_alg()");
    }

	delete newJWT;
    delete jwt;
}


void test_jwt_decode()
{
	const char token[] =
            "eyJhbGciOiJub25lIn0.eyJpc3MiOiJmaWxlcy5jeXBo"
            "cmUuY29tIiwic3ViIjoidXNlcjAifQ.";
	JWT::jwt_alg_t alg;

	auto jwt = new JWT;
    try {
        jwt->decode(token);
    }
    catch (const exception& e) {
        throw Exception(string(__PRETTY_FUNCTION__) + " Failed jwt_decode(): " + string(e.what()));
    }

	alg = jwt->get_alg();
	if (alg != JWT::JWT_ALG_NONE) {
        throw Exception(string(__PRETTY_FUNCTION__) + " Failed jwt_get_alg()");
    }

	delete jwt;
}


void test_jwt_decode_invalid_final_dot()
{
	const char token[] = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzM4NCJ9."
			     "eyJpc3MiOiJmaWxlcy5jeXBocmUuY29tIiwic"
			     "3ViIjoidXNlcjAifQ";

    auto jwt = new JWT;
    try {
        jwt->decode(token);
        throw Exception(string(__PRETTY_FUNCTION__) + " Not failed jwt_decode()");
    }
    catch (...) {
    }

	delete jwt;
}


void test_jwt_decode_invalid_alg()
{
	const char token[] = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIQUhBSCJ9."
			     "eyJpc3MiOiJmaWxlcy5jeXBocmUuY29tIiwic"
			     "3ViIjoidXNlcjAifQ.";

    auto jwt = new JWT;
    try {
        jwt->decode(token);
        throw Exception(string(__PRETTY_FUNCTION__) + " Not failed jwt_decode()");
    }
    catch (...) {
    }

	delete jwt;
}


void test_jwt_decode_invalid_typ()
{
	const char token[] = "eyJ0eXAiOiJBTEwiLCJhbGciOiJIUzI1NiJ9."
			     "eyJpc3MiOiJmaWxlcy5jeXBocmUuY29tIiwic"
			     "3ViIjoidXNlcjAifQ.";

    auto jwt = new JWT;
    try {
        jwt->decode(token);
        throw Exception(string(__PRETTY_FUNCTION__) + " Not failed jwt_decode()");
    }
    catch (...) {
    }

	delete jwt;
}


void test_jwt_decode_invalid_head()
{
	const char token[] = "yJ0eXAiOiJKV1QiLCJhbGciOiJIUzM4NCJ9."
			     "eyJpc3MiOiJmaWxlcy5jeXBocmUuY29tIiwic"
			     "3ViIjoidXNlcjAifQ.";

    auto jwt = new JWT;
    try {
        jwt->decode(token);
        throw Exception(string(__PRETTY_FUNCTION__) + " Not failed jwt_decode()");
    }
    catch (...) {
    }

	delete jwt;
}


void test_jwt_decode_alg_none_with_key()
{
	const char token[] = "eyJhbGciOiJub25lIn0."
			     "eyJpc3MiOiJmaWxlcy5jeXBocmUuY29tIiwic"
			     "3ViIjoidXNlcjAifQ.";

    auto jwt = new JWT;
    try {
        jwt->decode(token);
        throw Exception(string(__PRETTY_FUNCTION__) + " Not failed jwt_decode()");
    }
    catch (...) {
    }

	delete jwt;
}


void test_jwt_decode_invalid_body()
{
	const char token[] = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzM4NCJ9."
			     "eyJpc3MiOiJmaWxlcy5jeBocmUuY29tIiwic"
			     "3ViIjoidXNlcjAifQ.";

    auto jwt = new JWT;
    try {
        jwt->decode(token);
        throw Exception(string(__PRETTY_FUNCTION__) + " Not failed jwt_decode()");
    }
    catch (...) {
    }

	delete jwt;
}


void test_jwt_decode_hs256()
{
	const char token[] = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJpc3Mi"
			     "OiJmaWxlcy5jeXBocmUuY29tIiwic3ViIjoidXNlcjAif"
			     "Q.dLFbrHVViu1e3VD1yeCd9aaLNed-bfXhSsF0Gh56fBg";
	String key256("012345678901234567890123456789XY");

    auto jwt = new JWT;
    try {
        jwt->decode(token, key256);
    }
    catch (const exception& e) {
        throw Exception(string(__PRETTY_FUNCTION__) + " Failed jwt_decode(): " + string(e.what()));
    }

	delete jwt;
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

    auto jwt = new JWT;
    try {
        jwt->decode(token, key384);
    }
    catch (const exception& e) {
        throw Exception(string(__PRETTY_FUNCTION__) + " Failed jwt_decode(): " + string(e.what()));
    }

	delete jwt;
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

    auto jwt = new JWT;
    try {
        jwt->decode(token, key512);
    }
    catch (const exception& e) {
        throw Exception(string(__PRETTY_FUNCTION__) + " Failed jwt_decode(): " + string(e.what()));
    }

	delete jwt;
}


int main(int argc, char *argv[])
{

    const char* token = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJpc3MiOiJodHRwOi8va2Fycm9zdGVjaC5pbyIsInN1YiI6ImM0Y2ZjODIyLWM5ZjAtNGExMS05ZmFkLTJhNzJlYWVlMzM1NyIsImF1ZCI6Ijc1NzNjMjZiMTY5OTNlZjUzNmRjOTM5N2Y1YzA4OTExYTU1MWM3OWEiLCJpYXQiOjE1MTgxNTQxODIsImV4cCI6MTUxODE1Nzc4Miwic2NvcGUiOlsiRURQOkVkdWxvZzpBZG1pbiJdfQ.6DDnBtXAcYalH0NRnUnSRMNLFmjugId6RI9IyHCOQ_8";

    JWT jwt;
    try {
        jwt.decode(token);
        jwt.grants.exportTo(cout);
        cout << endl;
        stringstream temp;
        jwt.encode(temp);
        jwt.decode(temp.str().c_str());
        jwt.grants.exportTo(cout);
        cout << endl;
    }
    catch (const exception& e) {
        throw Exception(string(__PRETTY_FUNCTION__) + " Failed jwt_decode(): " + string(e.what()));
    }

    return 0;

    try {
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
