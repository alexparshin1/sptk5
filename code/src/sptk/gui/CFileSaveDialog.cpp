/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CFileSaveDialog.cpp - description                      ║
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

#ifdef _WIN32
#include <io.h>
#endif
#include <sys/stat.h>
#include <fcntl.h>

#include <sptk5/gui/CFileSaveDialog.h>
#include <sptk5/gui/CMessageDialog.h>
#include <sptk5/SystemException.h>

using namespace std;
using namespace sptk;

bool CFileSaveDialog::okPressed() {
   string fname;
   try {
      fname = fileName();
      if (!fname.length()) 
         throw Exception("Please, select or type in the filename.");
      fname = removeTrailingSlash(directory()) + CFileDialog::slashStr + fname;

      int fh = open(fname.c_str(),O_RDONLY);
      if (fh >= 0) {
         close(fh);
         if (!spAsk("File exists, overwrite it?"))
            return false;
         fh = open(fname.c_str(),O_RDWR);
         if (fh < 0)
            throw SystemException("File is write-protected.");
         close(fh);
      } else {
         fh = creat(fname.c_str(),S_IWRITE);
         if (fh < 0)
            throw SystemException("Can't create the file.");
         close(fh);
         if (::remove(fname.c_str()) != 0)
            throw SystemException("Can't create temporary file file.");
      }
      return true;
   }
   catch(exception& e) {
      spError("Can't save file "+fname +".\n"+string(e.what()));
   }
   return false;
}

CFileSaveDialog::CFileSaveDialog(const string& caption)
: CFileDialog(caption,true)
{
   m_okButton->label("Save");
   m_okButton->buttonImage(SP_SAVE_BUTTON);
   m_directoryView->multiSelect(false);
}
