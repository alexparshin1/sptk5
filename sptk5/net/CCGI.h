/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          ccgi.h  -  description
                             -------------------
    begin                : Thu May 25 2000
    copyright            : (C) 1999-2014 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
 **************************************************************************/

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

#ifndef ccgiH
#define ccgiH

#include <stdio.h>

#include <sptk5/sptk.h>
#include <sptk5/net/CHttpParams.h>

#include <string>

namespace sptk {

/// @addtogroup utility Utility Classes
/// @{

/// HTML standard tags to use in begin() and end() methods
enum HTML_TAGS {
    HTML_TAG, TITLE_TAG, HEADER_TAG, BODY_TAG, FORM_TAG,
    TABLE_TAG, TABLEHEADER_TAG, TABLEROW_TAG, TABLECELL_TAG,
    FONT_TAG, PARAGRAPH_TAG, STRONG_TAG, BLOCKQUOTE_TAG,
    SCRIPT_TAG, IMAGE_TAG };

/// Supports a classic CGI application, decodes input data stream etc
class SP_EXPORT CCGIApplication {
    /// Global counter of the CCGIApplication objects. It should be 0 or 1
    static uint32_t objectCount;
    /// Parses the input data stream
    void   parseInput(const char *str);
    /// Prints the required HTTP header
    void   sendHeader();
protected:
    /// Flag, true if the application should actually print something and not just create
    /// the HTML file for future references
    bool   m_showPreview;
    FILE  *m_htmlPrint;                 ///< File descriptor for the normal HTML output
    FILE  *m_htmlFile;                  ///< File descriptor for the external file HTML output
    char   m_debugInputFileName[64];    ///< File name for the debug file to store the input data

    /// Prints CRLF into output
    /// @returns true if anything was printed
    bool   insertCRLF();
public:

    /// The map of input parameter names and values. Contains the information
    /// sent to the application from another page form through GET or POST metods.
    CHttpParams contentData;

    /// Reports the debug file name that contains the input data as it is received by
    /// the CGI application.
    const char *debugInputFileName() const {
        return m_debugInputFileName;
    }

public:

    /// Default constructor
    CCGIApplication();

    /// Destructor
    ~CCGIApplication();

public:

    /// Begins the standard HTML tag. See HTML_TAGS for the tag list.
    /// @returns true if anything was printed
    /// @param tag HTML_TAGS, HTML tag
    /// @param params const char *, extra parameters
    bool begin(HTML_TAGS tag,const char *params=NULL);

    /// Closes the standard HTML tag. See HTML_TAGS for the tag list.
    /// @param tag HTML_TAGS, extra parameters
    /// @returns true if anything was printed
    bool end(HTML_TAGS tag);

    /// Prints the htmlCode to the output files
    /// @param htmlCode const char *, the HTML code to print
    /// @returns true if anything was printed
    bool printString(const char *htmlCode);
public:

    /// Begins the TABLE HTML tag.
    /// @param params const char *, optional table attributes
    void startTable(const char *params=NULL);

    /// Ends the TABLE HTML tag.
    void endTable();

    /// Begins the table row HTML tag.
    void startTableRow(const char *params=NULL);

    /// Ends the table row HTML tag.
    void endTableRow();

    /// Defines the table header cell HTML tag.
    /// @param headerCellData const char *, header cell text
    /// @param params const char *, optional cells attributes
    void tableHeaderCell(const char *headerCellData,const char *params=NULL);

    /// Defines the table cell.
    /// @param cellData const char *, cell text
    /// @param params const char *, optional cells attributes
    void tableCell(const char *cellData,const char *params=NULL);

    /// Begins HTML page.
    /// @param title const char *, page title
    /// @param bodyParams const char *, optional body attributes
    void beginPage(const char *title,const char *bodyParams="");

    /// Ends HTML page.
    void endPage();

    /// Inserts text input on the page.
    /// @param name const char *, input name
    /// @param value const char *, optional input value
    /// @param attr const char *, optional input attributes
    void addTextInput(const char *name,const char *value="",const char *attr="");

    /// Inserts hidden input on the page.
    /// @param name const char *, hidden input name
    /// @param value const char *, optional hidden input value
    /// @param attr const char *, optional hidden input attributes
    void addHiddenInput(const char *name,const char *value="",const char *attr="");

    /// Starts duplicating the HTML output to the file.
    /// @param fname const char *, output file name
    /// @returns the output file descriptor
    FILE *createHtmlCopyFile(const char *fname);

    /// Stops duplicating the HTML output to the file.
    void closeHtmlCopyFile();

    /// Produce or not the HTML output to stdout
    void showPreviewMode(bool);

    /// Reports ether the the HTML output to stdout is produced or not
    bool showPreviewMode();

public:

    /// Prints input information for debug purposes
    void printParameters();
};
/// @}
}
#endif
