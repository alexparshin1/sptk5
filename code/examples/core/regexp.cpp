/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       regexp.cpp - description                               ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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
*/

#include <sptk5/cutils>

using namespace std;
using namespace sptk;
using namespace chrono;

int main()
{
    try
    {
        String text;

        text = "This text contains number: ABCDEF";

        COUT("Test: does '" << text << "' contain number? ");
        RegularExpression regexp("\\d+");
        if (regexp.matches(text))
        {
            COUT("yes" << endl);
        }
        else
        {
            COUT("no" << endl);
        }

        text = "This text contains number: 12345";
        COUT("Test: does '" << text << "' contain number? ");
        if (regexp.matches(text))
        {
            COUT("yes" << endl);
        }
        else
        {
            COUT("no" << endl);
        }

        text = "This text contains phone number: (415)-123-4567";
        COUT("Test: does '" << text << "' contain valid phone number? ");
        RegularExpression phoneRegexp("\\(\\d{3}\\)-\\d{3}-\\d{4}");
        if (phoneRegexp.matches(text))
        {
            COUT("yes" << endl);
        }
        else
        {
            COUT("no" << endl);
        }

        text = "This text contains phone number: 415/123/4567";
        COUT("Test: does '" << text << "' contain valid phone number? ");
        if (RegularExpression("\\(\\d{3}\\)-\\d{3}-\\d{4}").matches(text))
        {
            COUT("no" << endl);
        }
        else
        {
            COUT("yes" << endl);
        }

        text = "user='horse' noice='some' password='haystack' host='localhost'";
        COUT("\nParsing the text: " << text << endl);
        RegularExpression connectionParser("(user|password|host)='([\\S]+)'", "g");
        RegularExpression parameterParser("(\\S+)=['\"]([\\S]+)['\"]");
        auto matches = connectionParser.m(text);
        if (matches)
        {
            for (unsigned i = 0; i < matches.groups().size(); i++)
            {
                COUT(matches[i].value << " : ");
                i++;
                COUT(matches[i].value << endl);
            }
            COUT(endl);
        }
        else
            CERR("ERROR: Didn't match connectionParser" << endl);

        text = "Area code: 415 Phone: 123-4567";
        COUT("\nParsing the text: " << text << endl);
        RegularExpression phoneStringParser("^Area code: (\\d{3}) Phone: (\\d{3})-(\\d{4})$");
        String phoneNumber = phoneStringParser.s(text, "(\\1)-\\2-\\3");
        COUT("Reformatted phone number: " << phoneNumber << endl
                                          << endl)

        DateTime started = DateTime::Now();

        text = "{one}-{four}{one}{five}-{One}{TwO}{Three}-{one}{two}{two}{one}";
        map<String, String> substitutions;
        substitutions["{one}"] = "1";
        substitutions["{Two}"] = "2";
        substitutions["{three}"] = "3";
        substitutions["{Four}"] = "4";
        substitutions["{five}"] = "5";
        RegularExpression phoneTranslate("{[a-z]+}", "gi");
        bool replaced;
        phoneNumber = phoneTranslate.replaceAll(text, substitutions, replaced);
        COUT("\nSubstituting text '" << text << "' to digits." << endl);
        COUT("The result is '" << phoneNumber << "'." << endl
                               << endl)

        text = "This text contains phone number: (415)-123-4567";

        unsigned counter = 0;
        unsigned tests = 1000000;
        for (unsigned i = 0; i < tests; i++)
        {
            if (RegularExpression(R"(\(\d{3}\)-\d{3}-\d{4})").matches(text))
                counter++;
        }
        DateTime finished = DateTime::Now();
        double duration = duration_cast<milliseconds>(finished - started).count() / 1000.0;
        COUT("Executed " << tests << " regexp tests (compiled on the fly) for " << duration << " seconds." << endl);
        COUT("That is " << fixed << tests / duration / 1000000 << "M tests/sec" << endl);

        started = DateTime::Now();
        counter = 0;
        for (unsigned i = 0; i < tests; i++)
        {
            if (phoneRegexp.matches(text))
                counter++;
        }
        finished = DateTime::Now();
        duration = duration_cast<milliseconds>(finished - started).count() / 1000.0;
        COUT("Executed " << counter << " regexp tests (precompiled) for " << duration << " seconds." << endl);
        COUT("That is " << fixed << tests / duration / 1000000 << "M tests/sec" << endl);

        return 0;
    }
    catch (const Exception& e)
    {
        CERR(e.what() << endl);
        return 1;
    }
}
//---------------------------------------------------------------------------
