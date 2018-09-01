/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       NodeList.cpp - description                             ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
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

#include <sptk5/sptk.h>
#include <sptk5/cxml>

using namespace std;
using namespace sptk;
using namespace sptk::xml;

void NodeList::clear()
{
    for (auto item: *this)
        delete item;
    NodeVector::clear();
}

NodeList::iterator NodeList::findFirst(const char* nodeName)
{
    auto _begin = begin();
    auto _end = end();
    if (_begin == _end)
        return _end;

    Node* anode = *_begin;
    const string* sharedName = &anode->document()->shareString(nodeName);

    iterator itor;
    for (itor = _begin; itor != _end; ++itor) {
        anode = *itor;
        if (anode->nameIs(sharedName))
            break;
    }
    return itor;
}

NodeList::iterator NodeList::findFirst(const string& nodeName)
{
    return findFirst(nodeName.c_str());
}

NodeList::const_iterator NodeList::findFirst(const char* nodeName) const
{
    auto _begin = begin();
    auto _end = end();
    if (_begin == _end)
        return _end;

    const Node* anode = *_begin;
    const string* sharedName = anode->document()->findString(nodeName);

    const_iterator itor;
    for (itor = _begin; itor != _end; ++itor) {
        anode = *itor;
        if (anode->nameIs(sharedName))
            break;
    }
    return itor;
}

NodeList::const_iterator NodeList::findFirst(const string& nodeName) const
{
    return findFirst(nodeName.c_str());
}
