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

#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <sptk5/sptk.h>
#include <sptk5/Exception.h>
#include <sptk5/JWT.h>

#include <iomanip>

using namespace std;
using namespace sptk;

void test_dup()
{
	time_t now;
	int valint;

	JWT jwt;

    jwt["iss"] = "test";
    String val = jwt["iss"];
    if (val.empty()) {
        throw Exception("Can't get grant");
    }

	JWT newJWT(jwt);
	val = newJWT["iss"];
	if (val.empty()) {
        throw Exception("Can't get grant");
    }

    if (val != "test") {
        throw Exception("Got incorrect grant");
    }

	if (jwt.get_alg() != JWT::JWT_ALG_NONE) {
        throw Exception("Got incorrect alogorithm");
    }

	now = time(nullptr);
	jwt["iat"] = (int) now;

	valint = jwt["iat"];
    if (((long)now) != valint) {
        throw Exception("Failed jwt_get_grant_int()");
    }
}


void test_dup_signed()
{
	String key256("012345678901234567890123456789XY");

	JWT jwt;
	jwt["iss"] = "test";
	jwt.set_alg(JWT::JWT_ALG_HS256, key256);

	JWT newJWT(jwt);
	String val = newJWT["iss"];
    if (val != "test") {
        throw Exception("Failed jwt_get_grant_int()");
    }

    if (jwt.get_alg() != JWT::JWT_ALG_HS256) {
        throw Exception("Failed jwt_get_alg()");
    }
}


void test_decode()
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
        throw Exception("Failed jwt_decode(): " + string(e.what()));
    }

	alg = jwt->get_alg();
	if (alg != JWT::JWT_ALG_NONE) {
        throw Exception("Failed jwt_get_alg()");
    }

	delete jwt;
}


void test_decode_invalid_final_dot()
{
	const char token[] = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzM4NCJ9."
			     "eyJpc3MiOiJmaWxlcy5jeXBocmUuY29tIiwic"
			     "3ViIjoidXNlcjAifQ";

    auto jwt = new JWT;
    try {
        jwt->decode(token);
        throw Exception("Not failed jwt_decode()");
    }
    catch (...) {
    }

	delete jwt;
}


void test_decode_invalid_alg()
{
	const char token[] = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIQUhBSCJ9."
			     "eyJpc3MiOiJmaWxlcy5jeXBocmUuY29tIiwic"
			     "3ViIjoidXNlcjAifQ.";

    auto jwt = new JWT;
    try {
        jwt->decode(token);
        throw Exception("Not failed jwt_decode()");
    }
    catch (...) {
    }

	delete jwt;
}


void test_decode_invalid_typ()
{
	const char token[] = "eyJ0eXAiOiJBTEwiLCJhbGciOiJIUzI1NiJ9."
			     "eyJpc3MiOiJmaWxlcy5jeXBocmUuY29tIiwic"
			     "3ViIjoidXNlcjAifQ.";

    auto jwt = new JWT;
    try {
        jwt->decode(token);
        throw Exception("Not failed jwt_decode()");
    }
    catch (...) {
    }

	delete jwt;
}


void test_decode_invalid_head()
{
	const char token[] = 
		"yJ0eXAiOiJKV1QiLCJhbGciOiJIUzM4NCJ9."
		"eyJpc3MiOiJmaWxlcy5jeXBocmUuY29tIiwic"
		"3ViIjoidXNlcjAifQ.";

    auto jwt = new JWT;
    try {
        jwt->decode(token);
        throw Exception("Not failed jwt_decode()");
    }
    catch (...) {
    }

	delete jwt;
}


void test_decode_alg_none_with_key()
{
	const char token[] = 
		"eyJhbGciOiJub25lIn0."
		"eyJpc3MiOiJmaWxlcy5jeXBocmUuY29tIiwic"
		"3ViIjoidXNlcjAifQ.";

    auto jwt = new JWT;
    try {
        jwt->decode(token);
        throw Exception("Not failed jwt_decode()");
    }
    catch (...) {
    }

	delete jwt;
}


void test_decode_invalid_body()
{
	const char token[] = 
		"eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzM4NCJ9."
		"eyJpc3MiOiJmaWxlcy5jeBocmUuY29tIiwic"
		"3ViIjoidXNlcjAifQ.";

    auto jwt = new JWT;
    try {
        jwt->decode(token);
        throw Exception("Not failed jwt_decode()");
    }
    catch (...) {
    }

	delete jwt;
}


void test_decode_hs256()
{
	const char token[] = 
		"eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJpc3Mi"
		"OiJmaWxlcy5jeXBocmUuY29tIiwic3ViIjoidXNlcjAif"
		"Q.dLFbrHVViu1e3VD1yeCd9aaLNed-bfXhSsF0Gh56fBg";
	String key256("012345678901234567890123456789XY");

    auto jwt = new JWT;
    try {
        jwt->decode(token, key256);
    }
    catch (const exception& e) {
        throw Exception("Failed jwt_decode(): " + string(e.what()));
    }

	delete jwt;
}


void test_decode_hs384()
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
        throw Exception("Failed jwt_decode(): " + string(e.what()));
    }

	delete jwt;
}


void test_decode_hs512()
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
        throw Exception("Failed jwt_decode(): " + string(e.what()));
    }

	delete jwt;
}

void test_encode_hs256_decode()
{
    String key256("012345678901234567890123456789XY");

    JWT jwt;
    jwt.set_alg(JWT::JWT_ALG_HS256, key256);

    jwt["iat"] = (int) time(nullptr);
    jwt["iss"] = "http://test.com";
    jwt["exp"] = (int) time(nullptr) + 86400;

	auto info = jwt.grants.root().set_object("info");
    info->set("company", "Linotex");
    info->set("city", "Melbourne");

    stringstream originalToken;
    jwt.encode(originalToken);

    stringstream originalJSON;
    jwt.exportTo(originalJSON, false);

    JWT jwt2;
    jwt2.decode(originalToken.str().c_str(), key256);

    stringstream copiedJSON;
    jwt2.exportTo(copiedJSON, false);

    stringstream copiedToken;
    jwt2.encode(copiedToken);

    if (originalJSON.str() != copiedJSON.str())
        throw Exception("Decoded JSON payload doesn't match the original");

    if (originalToken.str() != copiedToken.str())
        throw Exception("Decoded JWT data doesn't not match the original");
}

#define run_test(test_name) { cout << setw(40) << left << string(#test_name) + ": "; try { test_name(); cout << "Ok" << endl; } catch (const exception& e) { cout << e.what() << endl; } }

int main(int argc, char *argv[])
{
    /*
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
        throw Exception("Failed jwt_decode(): " + string(e.what()));
    }
    */

    try {
        run_test(test_dup);
        run_test(test_dup_signed);
        run_test(test_decode);
        run_test(test_decode_invalid_alg);
        run_test(test_decode_invalid_typ);
        run_test(test_decode_invalid_head);
        run_test(test_decode_alg_none_with_key);
        run_test(test_decode_invalid_body);
        run_test(test_decode_invalid_final_dot);
        run_test(test_decode_hs256);
        run_test(test_decode_hs384);
        run_test(test_decode_hs512);
        run_test(test_encode_hs256_decode);
    }
    catch (const exception& e) {
        cerr << "ERROR:" << endl;
        cerr << e.what() << endl;
        return 1;
    }

    cout << "All tests passed." << endl;
    return 0;
}
