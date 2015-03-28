/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CCGI.cpp  -  description
                             -------------------
    begin                : Thu May 25 2000
    copyright            : (C) 1999-2014 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sptk5/sptk.h>
#include <sptk5/CBuffer.h>
#include <sptk5/CException.h>
#include <sptk5/net/CCGI.h>
#include <sptk5/net/CHttpParams.h>

using namespace sptk;

static const char *tags[] = {
    "HTML",
    "TITLE",
    "HEADER",
    "BODY",
    "FORM",
    "TABLE",
    "TH",
    "TR",
    "TD",
    "FONT",
    "P",
    "STRONG",
    "BLOCKQUOTE",
    "SCRIPT",
    "IMAGE"
};

uint32_t CCGIApplication::objectCount;

CCGIApplication::CCGIApplication() {
    m_htmlPrint = stdout;

    if (objectCount)
        throw CException("Only one instance of CCGIApplication is allowed");

    objectCount = 1;
    m_htmlFile = NULL;
    m_showPreview = false;

    const char *p;

    // Get Query string
    p = getenv("QUERY_STRING");

    sprintf(m_debugInputFileName, "_input%05i.txt", (int) time(NULL));

    if (p)
        parseInput(p);

    // Get Content
    p = getenv("CONTENT_LENGTH");

    char* buffer = NULL;
    uint32_t bytes = 0;

    if (p) {
        int contentSize = atoi(p) + 1;

        buffer = new char[contentSize + 1];

        bytes = (uint32_t) fread(buffer, 1, size_t(contentSize), stdin);

        //copy input parameters into file
        sprintf(m_debugInputFileName, "debug/input%05i.txt", (int) time(NULL));
        FILE *f = fopen(m_debugInputFileName, "wb");
        if (f) {
            bytes = (uint32_t) fwrite(buffer, 1, bytes, f);
            fclose(f);
        } else {
            strcpy(m_debugInputFileName, "not created");
        }
    } else {
        p = "65535"; // Emulation for debugging from file

        int contentSize = atoi(p) + 1;

        buffer = new char[contentSize + 1];

        FILE *f = fopen("input.txt", "rb");
        if (f) {
            bytes = (uint32_t) fread(buffer, 1, size_t(contentSize), f);
            buffer[bytes] = 0;
            fclose(f);
        }
    }

    if (buffer) {
        if (bytes) parseInput(buffer);
        delete [] buffer;
    }
    try {
        m_showPreview = contentData["reportgenerator_reportmode"] == "display";
    } catch (...) {
        m_showPreview = true;
    }
}

CCGIApplication::~CCGIApplication() {
    if (m_htmlFile)
        fclose(m_htmlFile);
    objectCount = 0;
}

void CCGIApplication::parseInput(const char * str) {
    CBuffer buffer(str, (uint32_t) strlen(str));
    contentData.decode(buffer, true);
    sendHeader();
}

void CCGIApplication::sendHeader() {
#ifdef _WIN32
    //printf("HTTP/1.0 200 OK\nContent-type: text/html\n\n");
    fprintf(m_htmlPrint, "Content-type: text/html\n\n");
#endif
}

bool CCGIApplication::begin(HTML_TAGS tag, const char *params) {
    if ((m_showPreview && m_htmlPrint) || m_htmlFile) {
        uint32_t sz = 16;
        if (params)
            sz += uint32_t(strlen(params) + 1);
        char *buffer = new char[sz];
        if (params)
            sprintf(buffer, "<%s %s>", tags[tag], params);
        else sprintf(buffer, "<%s>", tags[tag]);

        printString(buffer);

        delete [] buffer;

        return true;
    }
    return false;
}

bool CCGIApplication::end(HTML_TAGS tag) {
    bool rc = false;
    if (m_showPreview && m_htmlPrint) {
        fprintf(m_htmlPrint, "</%s>", tags[tag]);
        rc = true;
    }

    if (m_htmlFile) {
        fprintf(m_htmlFile, "</%s>", tags[tag]);
        rc = true;
    }

    return rc;
}
//---------------------------------------------------------------------------

bool CCGIApplication::insertCRLF() {
    return printString("\n\r");
}
//---------------------------------------------------------------------------

bool CCGIApplication::printString(const char *s) {
    bool rc = false;
    if (m_showPreview && m_htmlPrint) {
        fputs(s, m_htmlPrint);
        rc = true;
    }
    if (m_htmlFile) {
        fputs(s, m_htmlFile);
        rc = true;
    }
    return rc;
}
//---------------------------------------------------------------------------

void CCGIApplication::startTable(const char *params) {
    if (insertCRLF())
        begin(TABLE_TAG, params);
}

void CCGIApplication::endTable() {
    if (end(TABLE_TAG))
        insertCRLF();
}

void CCGIApplication::startTableRow(const char *params) {
    if (insertCRLF())
        begin(TABLEROW_TAG, params);
}

void CCGIApplication::endTableRow() {
    if (end(TABLEROW_TAG))
        insertCRLF();
}

void CCGIApplication::tableHeaderCell(const char *cellData, const char *params) {
    if (begin(TABLEHEADER_TAG, params)) {
        printString(cellData);
        end(TABLEHEADER_TAG);
    }
}

void CCGIApplication::tableCell(const char *cellData, const char *params) {
    if (begin(TABLECELL_TAG, params)) {
        printString(cellData);
        end(TABLECELL_TAG);
    }
}

void CCGIApplication::printParameters() {
    CHttpParams::iterator itor = contentData.begin();
    for (; itor != contentData.end(); itor++) {
        begin(PARAGRAPH_TAG);
        begin(STRONG_TAG);
        if (m_htmlPrint) fprintf(m_htmlPrint, "%s :", itor->first.c_str());
        if (m_htmlFile) fprintf(m_htmlFile, "%s :", itor->first.c_str());
        end(STRONG_TAG);
        if (m_htmlPrint) fprintf(m_htmlPrint, "%s\n\r", itor->second.c_str());
        if (m_htmlFile) fprintf(m_htmlFile, "%s\n\r", itor->second.c_str());
    }
}

void CCGIApplication::beginPage(const char *title, const char *bodyParams) {
    if (begin(HTML_TAG)) {
        insertCRLF();
        printString("<HEAD>");
        begin(TITLE_TAG);
        printString(title);
        end(TITLE_TAG);
        printString("</HEAD>");
        insertCRLF();
        begin(BODY_TAG, bodyParams);
        insertCRLF();
    }
}

void CCGIApplication::endPage() {
    if (insertCRLF()) {
        end(BODY_TAG);
        end(HTML_TAG);
        insertCRLF();
    }
}

void CCGIApplication::addTextInput(const char *name, const char *value, const char *attr) {
    if (m_htmlPrint)
        fprintf(m_htmlPrint, "\n<INPUT NAME=%s TYPE=TEXT VALUE=\"%s\" %s>", name, value, attr);
    if (m_htmlFile)
        fprintf(m_htmlFile, "\n<INPUT NAME=%s TYPE=TEXT VALUE=\"%s\" %s>", name, value, attr);
}

void CCGIApplication::addHiddenInput(const char *name, const char *value, const char *attr) {
    if (m_htmlPrint)
        fprintf(m_htmlPrint, "\n<INPUT NAME=%s TYPE=HIDDEN VALUE=\"%s\" %s>", name, value, attr);
    if (m_htmlFile)
        fprintf(m_htmlFile, "\n<INPUT NAME=%s TYPE=HIDDEN VALUE=\"%s\" %s>", name, value, attr);
}

FILE *CCGIApplication::createHtmlCopyFile(const char *fname) {
    if (!m_htmlFile)
        m_htmlFile = fopen(fname, "wt");
    return m_htmlFile;
}

void CCGIApplication::closeHtmlCopyFile() {
    if (m_htmlFile)
        fclose(m_htmlFile);
    m_htmlFile = 0;
}

void CCGIApplication::showPreviewMode(bool sp) {
    m_showPreview = sp;
}

bool CCGIApplication::showPreviewMode() {
    return m_showPreview;
}
