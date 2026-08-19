/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       DirectoryPage.cpp - installation directory page        ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include "DirectoryPage.h"
#include "InstallerUtils.h"

#include <sptk5/gui/CButton.h>
#include <sptk5/gui/CDirOpenDialog.h>
#include <sptk5/gui/CGroup.h>
#include <sptk5/gui/CMessageDialog.h>

using namespace std;
using namespace sptk;

DirectoryPage::DirectoryPage(InstallerConfig& config)
    : WizardPage(config, "Directory")
{
}

void DirectoryPage::cb_browse(Fl_Widget*, void* data)
{
    static_cast<DirectoryPage*>(data)->doBrowse();
}

void DirectoryPage::cb_changed(Fl_Widget*, void* data)
{
    static_cast<DirectoryPage*>(data)->updateStatus();
}

void DirectoryPage::build()
{
    auto* label = new CHtmlBox("", 60, CLayoutAlign::TOP);
    label->data(Variant("<h3>Installation Directory</h3>"
                        "<p>Choose the directory where " +
                        m_config.application + " will be installed. "
                                               "Type the full path, or click <b>Browse</b> to pick it.</p>"));

    // Input and 'Browse' button share a single row
    auto* dirGroup = new CGroup("", 30, CLayoutAlign::TOP);

    auto* browseButton = new CButton(CButtonKind::BROWSE_BUTTON, CLayoutAlign::RIGHT, "Browse");
    browseButton->callback(cb_browse, this);

    m_dirInput = new CInput("Install to:", 30, CLayoutAlign::CLIENT);
    m_dirInput->data(m_config.installDirectory);
    m_dirInput->callback(cb_changed, this);

    dirGroup->end();

    m_statusHtml = new CHtmlBox("", 80, CLayoutAlign::TOP);

    updateStatus();
}

void DirectoryPage::onEnter()
{
    updateStatus();
}

void DirectoryPage::doBrowse()
{
    filesystem::path current = normalizeDirectory(m_dirInput->data().getString());
    filesystem::path startFrom = existingAncestor(current);

    CDirOpenDialog dialog("Select Installation Directory");
    dialog.directory(startFrom.empty() ? String(".") : String(startFrom.string()));

    if (!dialog.execute())
        return;

    filesystem::path selected = normalizeDirectory(dialog.directory());
    if (selected.empty())
        return;

    // Keep the leaf name typed by the user if it isn't created yet, e.g. /opt + MyApp
    if (!current.empty() && current.has_filename() && existingAncestor(current) != current &&
        selected != current.parent_path() && selected != current)
        selected /= current.filename();

    m_dirInput->data(Variant(String(selected.string())));
    updateStatus();
}

void DirectoryPage::updateStatus()
{
    if (m_statusHtml == nullptr)
        return;

    filesystem::path directory = normalizeDirectory(m_dirInput->data().getString());
    String           html;

    if (directory.empty())
    {
        html = "<p><b>No installation directory specified.</b></p>";
    }
    else if (!directory.is_absolute())
    {
        html = "<p><b>Please enter an absolute path</b>, for example " +
               htmlEscape(m_config.installDirectory) + ".</p>";
    }
    else
    {
        error_code       ec;
        filesystem::path ancestor = existingAncestor(directory);

        if (filesystem::exists(directory, ec) && !filesystem::is_directory(directory, ec))
        {
            html = "<p><b>" + htmlEscape(String(directory.string())) +
                   "</b> already exists and is not a directory.</p>";
        }
        else
        {
            if (ancestor == directory)
            {
                html = filesystem::is_empty(directory, ec)
                           ? "<p>The directory exists and is empty.</p>"
                           : "<p>The directory exists and is <b>not empty</b>. "
                             "Its contents may be overwritten.</p>";
            }
            else if (ancestor.empty())
            {
                html = "<p>None of the parent directories of this path exist.</p>";
            }
            else
            {
                html = "<p>The directory will be created.</p>";
            }

            if (!ancestor.empty())
            {
                if (auto space = filesystem::space(ancestor, ec); !ec)
                    html += "<p>Free space on " + htmlEscape(String(ancestor.string())) + ": <b>" +
                            formatByteSize(space.available) + "</b></p>";

                if (!isDirectoryWritable(ancestor))
                    html += "<p><b>Note:</b> " + htmlEscape(String(ancestor.string())) +
                            " is not writable by the current user, so the installation may "
                            "ask for administrator privileges.</p>";
            }
        }
    }

    m_statusHtml->data(Variant(html));
    m_statusHtml->redraw();
}

bool DirectoryPage::onLeave()
{
    filesystem::path directory = normalizeDirectory(m_dirInput->data().getString());

    if (directory.empty())
    {
        spError("Please specify the installation directory.");
        return false;
    }

    if (!directory.is_absolute())
    {
        spError("The installation directory must be an absolute path.");
        return false;
    }

    error_code ec;
    if (filesystem::exists(directory, ec) && !filesystem::is_directory(directory, ec))
    {
        spError("'" + String(directory.string()) + "' already exists and is not a directory.\n"
                                                   "Please choose another installation directory.");
        return false;
    }

    if (existingAncestor(directory).empty())
    {
        spError("The path '" + String(directory.string()) + "' can't be created because\n"
                                                            "none of its parent directories exist.");
        return false;
    }

    if (filesystem::is_directory(directory, ec) && !filesystem::is_empty(directory, ec) &&
        !spAsk("The directory '" + String(directory.string()) + "' is not empty.\n"
                                                                "Install into it anyway?"))
        return false;

    m_config.installDirectory = String(directory.string());
    m_dirInput->data(Variant(m_config.installDirectory));
    return true;
}
