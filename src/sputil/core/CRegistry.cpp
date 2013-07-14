/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CRegistry.cpp  -  description
                             -------------------
    begin                : Tue Mar 26 2002
    copyright            : (C) 1999-2013 by Alexey Parshin. All rights reserved.
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
#include <sys/stat.h>

#include <sptk5/CRegistry.h>
#include <sptk5/CException.h>
#include <sptk5/xml/CXml.h>
#include <sptk5/string_ext.h>

using namespace std;
using namespace sptk;

#ifndef _WIN32
    #include <unistd.h>
#else
    #include <winsock2.h>
    #include <windows.h>
    #include <io.h>
    #include <direct.h>

    #define S_ISLNK(m)      (false)
    #define S_ISEXEC(m)     (((m) & _S_IEXEC) == _S_IEXEC)
    #if !defined(__UNIX_COMPILER__) && !defined(__BORLANDC__)
        #define S_ISREG(m)      (((m) & _S_IFREG) == _S_IFREG)
        #define S_ISDIR(m)      (((m) & _S_IFDIR) == _S_IFDIR)
        #define S_ISBLK(m)      (((m) & _S_IFBLK) == _S_IFBLK)
    #endif
#endif
//----------------------------------------------------------------------------
string CRegistry::homeDirectory() {
#ifndef _WIN32
    string homeDir = trim(getenv("HOME"));
    if (homeDir.empty())
        homeDir = ".";
    homeDir += "/";
#else

    char *hdrive = getenv("HOMEDRIVE");
    char *hdir   = getenv("HOMEPATH");
    if (!hdir && !hdrive)
        return getenv("WINDIR") + string("\\");

    string homeDrive;
    string homeDir;
    if (hdrive)
        homeDrive = hdrive;
    if (hdir)
        homeDir = hdir;
    if (homeDir == "\\")
        homeDir = homeDrive + "\\";
    else
        homeDir = homeDrive + "\\" + homeDir;
#endif

    return homeDir;
}

CRegistry::CRegistry(string fileName,string programGroupName,CRegistryMode mode)
        : CXmlDoc("Configuration"), m_fileName(fileName) {
    if (m_fileName.length()) {
        string directory;
        if (mode == USER_REGISTRY)
            directory = homeDirectory();
        directory += "/";
        if (programGroupName != "") {
            while (programGroupName[0] == '.')
                programGroupName = programGroupName.substr(1);
            programGroupName = replaceAll(programGroupName,"\\","_");
            programGroupName = replaceAll(programGroupName,"/","_");
        }
        if (programGroupName != "")
            directory += "." + programGroupName + "/";
        m_fileName = directory + m_fileName;
    }
    m_fileName = replaceAll(m_fileName,"\\","/");
    m_fileName = replaceAll(m_fileName,"//","/");
}

CRegistry::~CRegistry() {
    clear();
}

void CRegistry::prepareDirectory() {
    struct stat st;
    size_t pos = m_fileName.rfind("/");
    if (pos == STRING_NPOS)
        return;
    string directory = m_fileName.substr(0,pos);
    if (stat(directory.c_str(),&st) == 0) {
        if (!S_ISDIR(st.st_mode))
            throw CException("Can't create directory '"+directory+"'");
    } else {
#ifdef _WIN32
        mkdir(directory.c_str());
#else

        mkdir(directory.c_str(),0770);
#endif

    }
}

void CRegistry::load(const CStrings& inputData) {
    clear();
    CBuffer buffer = inputData.asString("\n");
    CXmlDoc::load(buffer);
}

void CRegistry::load(const char* fileName) {
    if (fileName)
        m_fileName = fileName;
    CBuffer inputData;
    inputData.loadFromFile(m_fileName);
    CXmlDoc::load(inputData);
}

void CRegistry::save(CStrings& outputData) {
    CBuffer buffer;
    prepareDirectory();
    outputData.clear();
    CXmlDoc::save(buffer);
    outputData.fromString(buffer.data(),"\n");
}

void CRegistry::save() {
    CBuffer outputData;
    prepareDirectory();
    CXmlDoc::save(outputData);
    outputData.saveToFile(m_fileName);
}

void CRegistry::clean(CXmlNode* node) {
    CXmlNode::iterator itor = node->begin();
    CXmlNode::iterator iend = node->end();
    CXmlNodeVector     toDelete;
    for (; itor != iend; itor++ ) {
        CXmlNode* anode = *itor;
        if (anode->type() != DOM_ELEMENT) {
            toDelete.push_back(anode);
            continue;
        }
        if (anode->size())
            clean(anode);
    }
    CXmlNodeVector::iterator it = toDelete.begin();
    for ( ; it != toDelete.end(); it++)
        node->remove
        (*it);
}

void CRegistry::load(const CXmlDoc& data) {
    clear();
    copy(data);
    clean(this);
}

void CRegistry::save(CXmlDoc& data) const {
    data.copy(*this);
}
