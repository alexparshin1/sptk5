/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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

#if !defined(S_ISREG) && defined(S_IFMT) && defined(S_IFREG)
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif

#endif

#include <sys/stat.h>

#include <sptk5/gui/CDirOpenDialog.h>
#include <sptk5/gui/CMessageDialog.h>

using namespace std;
using namespace sptk;

bool CDirOpenDialog::okPressed()
{
    struct stat st = {};
    String dname;
    try
    {
        dname = fileName();
        if (dname.empty())
        {
            dname = directory();
            const CSelection& selection = m_directoryView->selection();
            if (selection.size())
            {
                CPackedStrings& row = selection[0];
                String fname(row[1]);
                dname += fname;
            }
        }
        dname = removeTrailingSlash(dname) + slashStr;

        memset(&st, 0, sizeof(struct stat));

#ifdef _WIN32
        if (stat((dname + String(".")).c_str(), &st) != 0)
            throw Exception("Can't access directory '" + dname + "'");
#else
        if (lstat((dname + String(".")).c_str(), &st) != 0)
        {
            throw Exception("Can't access directory '" + dname + "'");
        }
#endif
        if (!S_ISDIR(st.st_mode))
        {
            dname = directory();
        }

        directory(dname);

        return true;
    }
    catch (Exception& e)
    {
        spError(e.what());
        return false;
    }
}

CDirOpenDialog::CDirOpenDialog(const String& caption)
    : CFileDialog(caption, true)
{
    m_okButton->label("Use");
    m_okButton->buttonImage(CButtonKind::SAVE_BUTTON);
    m_directoryView->multiSelect(false);
    m_patternCombo->hide();
    m_fileNameInput->label("Directory:");
}
