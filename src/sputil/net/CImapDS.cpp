/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CImapDS.cpp - description
                             -------------------
    begin                : Mar 19 2003
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

#include <sptk5/net/CImapDS.h>
#include <sptk5/CException.h>

#include <stdlib.h>

#ifndef _WIN32
# include <unistd.h>
#endif

using namespace sptk;

// read the folder() and move item into the first entry
bool CImapDS::open() {
   clear();

    // Connect to the server
   m_imap.cmd_login(m_user,m_password);

    // Select the mail box
   int32_t total_messages, first_message = 1;
   m_imap.cmd_select(m_folder,total_messages);

   if (m_msgid) {
      first_message = m_msgid;
      total_messages = m_msgid;
   }
   if (total_messages) {
      if (m_callback)
         m_callback(total_messages,0);
      for (long msg_id = first_message; msg_id <= total_messages; msg_id++) {
         CFieldList   *df = new CFieldList(false);

         df->user_data((void *)(size_t)msg_id);

         if (m_fetchbody)
            m_imap.cmd_fetch_message(msg_id,*df);
         else m_imap.cmd_fetch_headers(msg_id,*df);

         CField *fld = new CField("msg_id");
         fld->width = 0;
         fld->setInteger(msg_id);
         df->push_back(fld);

         m_list.push_back(df);

         if (m_callback)
            m_callback(total_messages,msg_id);
      }
      if (m_callback)
         m_callback(total_messages,total_messages);
   } else {
      if (m_callback)
         m_callback(100,100);
   }

   first();

   m_imap.cmd_logout();
   m_imap.close();

   m_eof = m_list.size() == 0;

   return !m_eof;
}
