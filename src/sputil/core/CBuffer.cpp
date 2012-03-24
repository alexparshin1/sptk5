/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CBuffer.cpp  -  description
                             -------------------
    begin                : January 3, 2003
    copyright            : (C) 2003-2012 by Alexey Parshin. All rights reserved.
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

#include <sptk5/CBuffer.h>
#include <sptk5/CException.h>
#include <sptk5/filedefs.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;
using namespace sptk;

CBuffer::CBuffer(uint32_t sz) {
   m_buffer = (char *)calloc(1,sz);
   if (m_buffer)
      m_size = sz;
   else  m_size = 0;
   m_bytes = 0;
}

CBuffer::CBuffer(const void *data,uint32_t sz) {
   m_buffer = (char *)malloc(sz+1);
   if (m_buffer) {
      memcpy(m_buffer,data,sz);
      m_size = sz;
      m_bytes = sz;
	  m_buffer[sz] = 0;
   } else {
      m_size = m_bytes = 0;
   }
}

CBuffer::CBuffer(const char *str) {
   uint32_t sz = (uint32_t) strlen(str) + 1;
   m_buffer = (char *)malloc(sz);
   if (m_buffer) {
      memcpy(m_buffer,str,sz);
      m_size = sz;
      m_bytes = sz-1;
   } else {
      m_size = 0;
      m_bytes = 0;
   }
}

CBuffer::CBuffer(const string& str) {
   uint32_t sz = (uint32_t) str.length() + 1;
   m_buffer = (char *)malloc(sz);
   if (m_buffer) {
      if (sz > 1)
         memcpy(m_buffer,str.c_str(),sz);
      else m_buffer[0] = 0;
      m_size = sz;
      m_bytes = sz-1;
   } else {
      m_size = 0;
      m_bytes = 0;
   }
}

CBuffer::CBuffer(const CBuffer& buffer) {
   uint32_t sz = buffer.bytes() + 1;
   m_buffer = (char *)malloc(sz);
   if (m_buffer) {
      memcpy(m_buffer,buffer.data(),sz);
      m_size = sz;
      m_bytes = sz-1;
   } else {
      m_size = 0;
      m_bytes = 0;
   }
}

void CBuffer::adjustSize(uint32_t sz) {
    uint32_t newSize = sz / 3 * 4 + 16;
    char *p = (char *)realloc(m_buffer,newSize + 1);
    if (!p)
        throw CException("Can't reallocate a buffer");
    m_buffer = p;
    m_size = newSize;
}

void CBuffer::set(const char *data,uint32_t sz) {
   checkSize(sz+1);
   if (data) {
      memcpy(m_buffer,data,sz);
      m_buffer[sz] = 0;
   }
   m_bytes = sz;
}

void CBuffer::append(char ch) {
   checkSize(m_bytes + 1);
   m_buffer[m_bytes] = ch;
   m_bytes++;
}

void CBuffer::append(const char *data,uint32_t sz) {
   if (!sz)
       sz = (uint32_t) strlen(data);
   checkSize(m_bytes + sz + 1);
   memcpy(m_buffer+m_bytes,data,sz);
   m_bytes += sz;
   m_buffer[m_bytes] = 0;
}

void CBuffer::fill(char c) {
   memset(m_buffer,c,m_size);
}

void CBuffer::reset(uint32_t sz) {
   if (sz) {
      char *p = (char *)realloc(m_buffer,sz + 1);
      if (!p)
         throw CException("Can't reallocate a buffer");
      m_buffer = p;
      m_size = sz;
   }
   m_bytes = 0;
}

void CBuffer::loadFromFile(string fileName) {
   FILE *f = fopen(fileName.c_str(),"rb");
   if (!f)
      throw CException("Can't open file " + fileName + " for reading");

   struct stat st;
   fstat(fileno(f),&st);
   int size = st.st_size;

   reset(size+1);
   m_buffer[size] = 0;
   m_bytes = (uint32_t) fread(m_buffer,1,size,f);
   fclose(f);
}

void CBuffer::saveToFile(string fileName) const {
   FILE *f = fopen(fileName.c_str(),"wb");
   if (!f)
      throw CException("Can't open file " + fileName + " for writing");
   fwrite(m_buffer,bytes(),1,f);
   fclose(f);
}

CBuffer& CBuffer::operator = (const CBuffer& b) {
   checkSize(b.m_bytes + 1);
   if (b.m_buffer)
	   memcpy(m_buffer,b.m_buffer,b.m_bytes);
   m_bytes = b.m_bytes;
   m_buffer[m_bytes] = 0;
   return *this;
}

CBuffer& CBuffer::operator = (const std::string& str) {
   uint32_t sz = (uint32_t) str.length(); 
   checkSize(sz + 1);
   if (sz)
	   memcpy(m_buffer,str.c_str(),sz + 1);
   m_bytes = sz;
   return *this;
}

CBuffer& CBuffer::operator = (const char *str) {
   uint32_t sz = (uint32_t) strlen(str); 
   checkSize(sz + 1);
   if (sz)
	   memcpy(m_buffer,str,sz + 1);
   m_bytes = sz;
   return *this;
}
