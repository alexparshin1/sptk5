/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CImapConnect.cpp  -  description
                             -------------------
    begin                : July 19 2003
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

#include <sptk5/string_ext.h>
#include <sptk5/CImapConnect.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

using namespace std;
using namespace sptk;

// Implementation is based on
// http://www.course.molina.com.br/RFC/Orig/rfc2060.txt

CImapConnect::CImapConnect() {
   m_port = 143;
   m_ident = 1;
}

CImapConnect::~CImapConnect() {
   close();
}

#define RSP_BLOCK_SIZE 1024
bool CImapConnect::getResponse(string ident) {
   char    readBuffer[RSP_BLOCK_SIZE+1];

   for (;;) {
      uint32_t len = readLine(readBuffer,RSP_BLOCK_SIZE);
      string longLine = readBuffer;
      if (len == RSP_BLOCK_SIZE && readBuffer[RSP_BLOCK_SIZE]!='\n') {
         do {
            len = readLine(readBuffer,RSP_BLOCK_SIZE);
            longLine += readBuffer;
         } while(len == RSP_BLOCK_SIZE);
      }
      m_response.push_back(longLine);
      if (ident[0] == 0) return true;

      if (longLine[0] == '*')
         continue;
      if (longLine[0] == '+')
         return true;
      if (longLine.find(ident)==0) {
         uint32_t p = (uint32_t) ident.length();
         while (longLine[p] == ' ') p++;
         switch (longLine[p]) {
            case 'O': // OK
               return true;
            case 'N': // NO
               throw CException(longLine.c_str()+8);
            case 'B': // BAD
               throw CException(longLine.c_str()+9);
         }
      }
   }
   return false;
}

const string CImapConnect::empty_quotes;

static string quotes(string st) {
   return "\"" + st + "\"";
}

string CImapConnect::sendCommand(string cmd) {
   char id_str[10];
   sprintf(id_str,"a%03i ",m_ident++);
   string ident(id_str);
   cmd = ident + cmd + "\n";
   if (!active())
      throw CException("Socket isn't open");
   write(cmd.c_str(),(uint32_t)cmd.length());
   return ident;
}

void CImapConnect::command(string cmd,const std::string& arg1,const std::string& arg2) {
   if (arg1.length() || &arg1 == &empty_quotes)
      cmd += " " + quotes(arg1);
   if (arg2.length() || &arg2 == &empty_quotes)
      cmd += " " + quotes(arg2);
   m_response.clear();
   string ident = sendCommand(cmd);
   getResponse(ident);
}

void CImapConnect::cmd_login(string user,string password) {
   close();
   open();
   m_response.clear();
   getResponse("");
   command("login "+user+" "+password);
}

// RFC 2060 test message :)
/*
CBuffer testMsg(
   "Date: Mon, 7 Feb 1994 21:52:25 -0800 (PST)\n\r"
   "From: Fred Foobar <foobar@Blurdybloop.COM\n\r"
   "Subject: afternoon meeting\n\r"
   "To: mooch@owatagu.siam.edu\n\r"
   "Message-Id: <B27397-0100000@Blurdybloop.COM>\n\r"
   "MIME-Version: 1.0\n\r"
   "Content-Type: TEXT/PLAIN; CHARSET=US-ASCII\n\r\n\r"
   "Hello Joe, do you think we can meet at 3:30 tomorrow?\n\r");
*/
void CImapConnect::cmd_append(string mail_box,const CBuffer& message) {
   string cmd = "APPEND \"" + mail_box + "\" (\\Seen) {"+ int2string((uint32_t)message.bytes()) +"}";
   string ident = sendCommand(cmd);
   getResponse(ident);
   write(message.data(),message.bytes());
   write("\n",1);
   getResponse(ident);
}

void CImapConnect::cmd_select(string mail_box,int32_t& total_msgs) {
   command("select",mail_box);
   for (unsigned i = 0; i < m_response.size(); i++) {
      std::string& st = m_response[i];
      if (st[0] == '*') {
         size_t p = st.find("EXISTS");
	 if (p != string::npos) {
            total_msgs = atoi(st.substr(2,p - 2).c_str());
            break;
         }
      }
   }
}

void CImapConnect::parseSearch(std::string& result) {
   result = "";
   for (unsigned i = 0; i < m_response.size(); i++) {
      std::string& st = m_response[i];
      if (st.find("* SEARCH") == 0)
         result += st.substr(8,st.length());
   }
}

void CImapConnect::cmd_search_all(std::string& result) {
   command("search all");
   parseSearch(result);
}

void CImapConnect::cmd_search_new(std::string& result) {
   command("search unseen");
   parseSearch(result);
}

static const char *required_headers[] = {
   "Date",
   "From",
   "Subject",
   "To",
   "CC",
   "Content-Type",
   "Reply-To",
   "Return-Path",
   NULL
};

static void parse_header(const string& header,string& header_name,string& header_value) {
   if (header[0] == ' ')
      return;

   size_t p = header.find(" ");
   if (p == string::npos)
      return;
   if (header[p-1] == ':') {
      header_name = lowerCase(header.substr(0,p-1));
      header_value = header.substr(p+1,header.length());
   }
}

static CDateTime decodeDate(const std::string& dt) {
   char    temp[40];
   strcpy(temp,dt.c_str()+5);
    // 1. get the day of the month
   char *p1 = temp;
   char *p2 = strchr(p1,' ');
   if (!p2) return CDateTime(0.0);
   *p2 = 0;
   int mday = atoi(p1);
    // 2. get the month
   p1 = p2 + 1;
   int month = 1;
   switch (*p1) {
      case 'A':
         if (*(p1+1) == 'p') {
            month = 4; // Apr
            break;
         }
         month = 8; // Aug
         break;
      case 'D':
         month = 12; // Dec
         break;
      case 'F':
         month = 2; // Feb
         break;
      case 'J':   
         if (*(p1+1) == 'a') {
            month = 1; // Jan
            break;
         } 
         if (*(p1+2) == 'n') {
            month = 6; // Jun
            break;
         }
         month = 7; // Jul
         break;
      case 'M':
         if (*(p1+2) == 'r') {
            month = 3; // Mar
            break;
         }
         month = 5; // May
         break;
      case 'N':
         month = 11; // Oct
         break;
      case 'O':
         month = 10; // Oct
         break;
      case 'S':
         month = 9; // Sep
         break;
   }
    // 2. get the year
   p1 += 4;
   p2 = p1 + 4;
   *p2 = 0;
   int year = atoi(p1);
   p1 = p2 + 1;
   p2 = strchr(p1,' ');
   if (p2) *p2 = 0;
   CDateTime    time(p1);
   CDateTime    date(year,month,mday);
   return double(date) + double(time);
}

void CImapConnect::parseMessage(CFieldList& results,bool headers_only) {
   results.clear();
   unsigned i;
   for (i = 0; required_headers[i]; i++) {
      string headerName = required_headers[i];
      CField *fld = new CField(lowerCase(headerName).c_str());
      switch (i) {
         case 0:  fld->width = 16; break;
         default: fld->width = 32; break;
      }
      results.push_back(fld);
   }

   // parse headers
   i = 1;
   for (; i < m_response.size() - 1; i++) {
      std::string& st = m_response[i];
      if (!st.length())
         break;
      string header_name, header_value;
      parse_header(st,header_name,header_value);
      if (header_name.length()) {
         try {
            CField& field = results[header_name];
            if (header_name == "date")
               field.setDate(decodeDate(header_value));
            else
               field = header_value;
         }
         catch (...) {
         }
      }
   }

   for (i = 0; i < results.size(); i++) {
      CField& field  = results[i];
      if (field.dataType() == VAR_NONE)
         field.setString("");
   }

   if (headers_only) return;

   string   body;
   for (; i < m_response.size() - 1; i++) {
      body += m_response[i] + "\n";
   }

   CField& bodyField = results.push_back(new CField("body"));
   bodyField.setString(body);
}

void CImapConnect::cmd_fetch_headers(int32_t msg_id,CFieldList& result) {
   command("FETCH "+int2string(msg_id)+" (BODY[HEADER])");
   parseMessage(result,true);
}

void CImapConnect::cmd_fetch_message(int32_t msg_id,CFieldList& result) {
   command("FETCH "+int2string(msg_id)+" (BODY[])");
   parseMessage(result,false);
}

string CImapConnect::cmd_fetch_flags(int32_t msg_id) {
   string result;
   command("FETCH "+int2string(msg_id)+" (FLAGS)");
   for (unsigned i = 0; i < m_response.size() - 1; i++) {
      std::string& st = m_response[i];
      const char *fpos = strstr(st.c_str(),"(\\");
      if (!fpos)
         return "";
      string flags(fpos+1);
      size_t pos = flags.find("))");
      if (pos != string::npos)
         flags[pos] = 0;
      return flags;
   }
   return result;
}

void CImapConnect::cmd_store_flags(int32_t msg_id,const char *flags) {
   command("STORE "+int2string(msg_id)+" FLAGS "+std::string(flags));
}

static string strip_framing_quotes(string st) {
   if (st[0] == '\"')
      return st.substr(1,st.length()-2);
   else return st;
}

void CImapConnect::parseFolderList() {
   CStrings folder_names;
   string prefix = "* LIST ";
   for (unsigned i = 0; i < m_response.size(); i++) {
      std::string& st = m_response[i];
      if (st.find(prefix) == 0) {
            // passing the attribute(s)
         const char *p = strstr(st.c_str() + prefix.length(),") ");
         if (!p) continue;
            // passing the reference
         p = strchr(p + 2,' ');
         if (!p) continue;
         p++;
         // Ok, we found the path
         folder_names.push_back(strip_framing_quotes(p));
      }
   }
   m_response = folder_names;
}

void CImapConnect::cmd_list(string mail_box_mask,bool decode)  { 
   command("list",empty_quotes,mail_box_mask);
   if (decode)
      parseFolderList();
}
