/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       ceditor_test.cpp - description                         ║
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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cerrno>

#ifdef __MWERKS__
#    define FL_DLL
#endif

#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Text_Buffer.H>

#include <sptk5/gui/CGroup.h>
#include <sptk5/gui/CWindow.h>
#include <sptk5/gui/CMenuBar.h>
#include <sptk5/gui/CDataControl.h>
#include <sptk5/gui/CButton.h>
#include <sptk5/gui/CEditor.h>
#include <sptk5/gui/CFileOpenDialog.h>
#include <sptk5/gui/CFileSaveDialog.h>

using namespace std;
using namespace sptk;

int changed = 0;
char filename[256] = "";
char title[256];
Fl_Text_Buffer *textbuf = nullptr;

// Syntax highlighting stuff...
Fl_Text_Buffer *stylebuf = nullptr;

Fl_Text_Display::Style_Table_Entry
styletable[] = {// Style table
    { FL_BLACK, FL_COURIER, 14}, // A - Plain
    { FL_DARK_GREEN, FL_COURIER_ITALIC, 14}, // B - Line comments
    { FL_DARK_GREEN, FL_COURIER_ITALIC, 14}, // C - Block comments
    { FL_BLUE, FL_COURIER, 14}, // D - Strings
    { FL_DARK_RED, FL_COURIER, 14}, // E - Directives
    { FL_DARK_RED, FL_COURIER_BOLD, 14}, // F - Types
    { FL_BLUE, FL_COURIER_BOLD, 14} // G - Keywords
};

const char *code_keywords[] = {// List of known C/C++ keywords...
    "and",
    "and_eq",
    "asm",
    "bitand",
    "bitor",
    "break",
    "case",
    "catch",
    "compl",
    "continue",
    "default",
    "delete",
    "do",
    "else",
    "false",
    "for",
    "goto",
    "if",
    "new",
    "not",
    "not_eq",
    "operator",
    "or",
    "or_eq",
    "return",
    "switch",
    "template",
    "this",
    "throw",
    "true",
    "try",
    "while",
    "xor",
    "xor_eq"
};
const char *code_types[] = {// List of known C/C++ types...
    "auto",
    "bool",
    "char",
    "class",
    "const",
    "const_cast",
    "double",
    "dynamic_cast",
    "enum",
    "explicit",
    "extern",
    "float",
    "friend",
    "inline",
    "int",
    "long",
    "mutable",
    "namespace",
    "private",
    "protected",
    "public",
    "register",
    "short",
    "signed",
    "sizeof",
    "static",
    "static_cast",
    "struct",
    "template",
    "typedef",
    "typename",
    "union",
    "unsigned",
    "virtual",
    "void",
    "volatile"
};


//
// 'compare_keywords()' - Compare two keywords...
//

int compare_keywords(const void *a, const void *b)
{
    return (strcmp(*((const char **) a), *((const char **) b)));
}


//
// 'style_parse()' - Parse text and produce style data.
//

void style_parse(const char *text, char *style, int length)
{
    char current;
    int col;
    int last;
    char buf[255], *bufptr;
    const char *temp;

    for (current = *style, col = 0, last = 0; length > 0; length--, text++) {
        if (current == 'B') current = 'A';
        if (current == 'A') {
            // Check for directives, comments, strings, and keywords...
            if (col == 0 && *text == '#') {
                // Set style to directive
                current = 'E';
            }
            else if (strncmp(text, "//", 2) == 0) {
                current = 'B';
                for (; length > 0 && *text != '\n'; length--, text++) *style++ = 'B';
                if (length == 0) break;
            }
            else if (strncmp(text, "/*", 2) == 0) {
                current = 'C';
            }
            else if (strncmp(text, "\\\"", 2) == 0) {
                // Quoted quote...
                *style++ = current;
                *style++ = current;
                text++;
                length--;
                col += 2;
                continue;
            }
            else if (*text == '\"') {
                current = 'D';
            }
            else if (!last && islower(*text)) {
                // Might be a keyword...
                for (temp = text, bufptr = buf;
                    islower(*temp) && bufptr < (buf + sizeof (buf) - 1);
                    *bufptr++ = *temp++);

                if (!islower(*temp)) {
                    *bufptr = '\0';

                    bufptr = buf;

                    if (bsearch(&bufptr, code_types,
                        sizeof (code_types) / sizeof (code_types[0]),
                        sizeof (code_types[0]), compare_keywords)) {
                        while (text < temp) {
                            *style++ = 'F';
                            text++;
                            length--;
                            col++;
                        }

                        text--;
                        length++;
                        last = 1;
                        continue;
                    }
                    else if (bsearch(&bufptr, code_keywords,
         sizeof (code_keywords) / sizeof (code_keywords[0]),
         sizeof (code_keywords[0]), compare_keywords)) {
                        while (text < temp) {
                            *style++ = 'G';
                            text++;
                            length--;
                            col++;
                        }

                        text--;
                        length++;
                        last = 1;
                        continue;
                    }
                }
            }
        }
        else if (current == 'C' && strncmp(text, "*/", 2) == 0) {
            // Close a C comment...
            *style++ = current;
            *style++ = current;
            text++;
            length--;
            current = 'A';
            col += 2;
            continue;
        }
        else if (current == 'D') {
            // Continuing in string...
            if (strncmp(text, "\\\"", 2) == 0) {
                // Quoted end quote...
                *style++ = current;
                *style++ = current;
                text++;
                length--;
                col += 2;
                continue;
            }
            else if (*text == '\"') {
                // End quote...
                *style++ = current;
                col++;
                current = 'A';
                continue;
            }
        }

        // Copy style info...
        if (current == 'A' && (*text == '{' || *text == '}')) *style++ = 'G';
        else *style++ = current;
        col++;

        last = isalnum(*text) || *text == '.';

        if (*text == '\n') {
            // Reset column and possibly reset the style
            col = 0;
            if (current == 'B' || current == 'E') current = 'A';
        }
    }
}


//
// 'style_init()' - Initialize the style buffer...
//

void style_init()
{
    auto style = new char[textbuf->length() + 1];
    char *text = textbuf->text();

    memset(style, 'A', (size_t) textbuf->length());
    style[textbuf->length()] = '\0';

    if (!stylebuf) stylebuf = new Fl_Text_Buffer(textbuf->length());

    style_parse(text, style, textbuf->length());

    stylebuf->text(style);
    delete[] style;
    free(text);
}


//
// 'style_unfinished_cb()' - Update unfinished styles.
//

void style_unfinished_cb(int, void*) { }


//
// 'style_update()' - Update the style buffer...
//

void style_update(
    int         pos, // I - Position of update
    int         nInserted, // I - Number of inserted chars
    int         nDeleted, // I - Number of deleted chars
    int         /*nRestyled*/, // I - Number of restyled chars
    const char* /*deletedText*/, // I - Text that was deleted
    void*       cbArg
)
{ // I - Callback data
    int start, // Start of text
        end; // End of text
    char last, // Last style on line
        *style, // Style data
        *text; // Text data


    // If this is just a selection change, just unselect the style buffer...
    if (nInserted == 0 && nDeleted == 0) {
        stylebuf->unselect();
        return;
    }

    // Track changes in the text buffer...
    if (nInserted > 0) {
        // Insert characters into the style buffer...
        style = new char[nInserted + 1];
        memset(style, 'A', (size_t) nInserted);
        style[nInserted] = '\0';

        stylebuf->replace(pos, pos + nDeleted, style);
        delete[] style;
    }
    else {
        // Just delete characters in the style buffer...
        stylebuf->remove(pos, pos + nDeleted);
    }

    // Select the area that was just updated to avoid unnecessary
    // callbacks...
    stylebuf->select(pos, pos + nInserted - nDeleted);

    // Re-parse the changed region; we do this by parsing from the
    // beginning of the line of the changed region to the end of
    // the line of the changed region...  Then we check the last
    // style character and keep updating if we have a multi-line
    // comment character...
    start = textbuf->line_start(pos);
    end = textbuf->line_end(pos + nInserted);
    text = textbuf->text_range(start, end);
    style = stylebuf->text_range(start, end);
    last = style[end - start - 1];

    //  printf("start = %d, end = %d, text = \"%s\", style = \"%s\"...\n",
    //         start, end, text, style);

    style_parse(text, style, end - start);

    //  printf("new style = \"%s\"...\n", style);

    stylebuf->replace(start, end, style);
    ((Fl_Text_Editor *) cbArg)->redisplay_range(start, end);

    if (last != style[end - start - 1]) {
        // The last character on the line changed styles, so reparse the
        // remainder of the buffer...
        free(text);
        free(style);

        end = textbuf->length();
        text = textbuf->text_range(start, end);
        style = stylebuf->text_range(start, end);

        style_parse(text, style, end - start);

        stylebuf->replace(start, end, style);
        ((Fl_Text_Editor *) cbArg)->redisplay_range(start, end);
    }

    free(text);
    free(style);
}


// Editor window functions and class...
void save_cb();
void saveas_cb();
void find2_cb(Fl_Widget*, void*);
void replall_cb(Fl_Widget*, void*);
void replace2_cb(Fl_Widget*, void*);
void replcan_cb(Fl_Widget*, void*);

class EditorWindow : public CWindow
{
public:
    EditorWindow(int w, int h, const char* t);
    ~EditorWindow() override;

    CWindow *replace_dlg;
    CInput *replace_find;
    CInput *replace_with;
    CButton *replace_all;
    CButton *replace_next;
    CButton *replace_cancel;

    CBox *cursor_position {nullptr};

    CEditor *editor;
    string search;
};

EditorWindow::EditorWindow(int w, int h, const char* t) : CWindow(w, h, t)
{
    replace_dlg = new CWindow(350, 105, "Replace");
    replace_find = new CInput("Find:");
    replace_with = new CInput("Replace:");

    auto grp = new CGroup("", 10, SP_ALIGN_BOTTOM);
    replace_all = new CButton("Replace All");
    replace_all->callback((Fl_Callback *) replall_cb, this);

    replace_next = new CButton("Replace Next");
    replace_next->callback((Fl_Callback *) replace2_cb, this);

    replace_cancel = new CButton("Cancel");
    replace_cancel->callback((Fl_Callback *) replcan_cb, this);
    grp->end();

    replace_dlg->end();
    replace_dlg->set_non_modal();
    editor = nullptr;
    search = "";
}

EditorWindow::~EditorWindow()
{
    delete replace_dlg;
}

int check_save()
{
    if (!changed) return 1;

    int r = fl_choice("The current file has not been saved.\n"
        "Would you like to save it now?",
        "Cancel", "Save", "Discard");

    if (r == 1) {
        save_cb(); // Save the file...
        return !changed;
    }

    return (r == 2) ? 1 : 0;
}

int loading = 0;

void load_file(const char *newfile, int ipos)
{
    loading = 1;
    int insert = (ipos != -1);
    changed = insert;
    if (!insert) strncpy(filename, "", sizeof(filename));
    int r;
    if (!insert) r = textbuf->loadfile(newfile);
    else r = textbuf->insertfile(newfile, ipos);
    if (r)
        fl_alert("Error reading from file \'%s\':\n%s.", newfile, strerror(errno));
    else
        if (!insert)
            strncpy(filename, newfile, sizeof(filename) - 1);
    loading = 0;
    textbuf->call_modify_callbacks();
}

void save_file(const char *newfile)
{
    if (textbuf->savefile(newfile))
        fl_alert("Error writing to file \'%s\':\n%s.", newfile, strerror(errno));
    else
        strncpy(filename, newfile, sizeof(filename) - 1);
    changed = 0;
    textbuf->call_modify_callbacks();
}

void copy_cb(Fl_Widget*, void* v)
{
    auto e = (EditorWindow*) v;
    Fl_Text_Editor::kf_copy(0, e->editor);
}

void cut_cb(Fl_Widget*, void* v)
{
    auto e = (EditorWindow*) v;
    Fl_Text_Editor::kf_cut(0, e->editor);
}

void delete_cb(Fl_Widget*, void*)
{
    textbuf->remove_selection();
}

void find_cb(Fl_Widget* w, void* v)
{
    auto e = (EditorWindow*) v;
    const char *val;

    val = fl_input("Search String:", e->search.c_str());
    if (val != nullptr) {
        // User entered a string - go find it!
        e->search = val;
        find2_cb(w, v);
    }
}

void find2_cb(Fl_Widget* w, void* v)
{
    auto e = (EditorWindow*) v;
    if (e->search[0] == '\0') {
        // Search string is blank; get a new one...
        find_cb(w, v);
        return;
    }

    int pos = e->editor->insert_position();
    int found = textbuf->search_forward(pos, e->search.c_str(), &pos);
    if (found) {
        // Found a match; select and update the position...
        textbuf->select(pos, pos + (int) strlen(e->search.c_str()));
        e->editor->insert_position(pos + (int) strlen(e->search.c_str()));
        e->editor->show_insert_position();
    }
    else fl_alert("No occurrences of \'%s\' found!", e->search.c_str());
}

void cursor_cb(Fl_Widget* w, void*)
{
    auto e = (EditorWindow*) w->window();
    int row, col;
    e->editor->cursorRowCol(row, col);
    e->cursor_position->data("Row " + int2string(row) + ", Col " + int2string(col));
    e->relayout();
}

void set_title(Fl_Window* w)
{
    if (filename[0] == '\0') strncpy(title, "Untitled", sizeof(title));
    else {
        char *slash;
        slash = strrchr(filename, '/');
#ifdef WIN32
        if (slash == NULL) slash = strrchr(filename, '\\');
#endif
        if (slash != nullptr)
            strncpy(title, slash + 1, sizeof(title) - 1);
        else
            strncpy(title, filename, sizeof(title) - 1);
    }

    if (changed)
        strncat(title, " (modified)", sizeof(title) - 1);

    w->label(title);
}

void changed_cb(int, int nInserted, int nDeleted, int, const char*, void* v)
{
    if ((nInserted || nDeleted) && !loading) changed = 1;
    auto w = (EditorWindow *) v;
    set_title(w);
    if (loading) w->editor->show_insert_position();
}

void new_cb(Fl_Widget*, void*)
{
    if (!check_save()) return;

    filename[0] = '\0';
    textbuf->select(0, textbuf->length());
    textbuf->remove_selection();
    changed = 0;
    textbuf->call_modify_callbacks();
}

void open_cb(Fl_Widget*, void*)
{
    if (!check_save()) return;

    CFileOpenDialog dialog;
    if (dialog.execute()) {
        String newfile = dialog.fullFileName();
        if (newfile.length()) load_file(newfile.c_str(), -1);
    }
}

void insert_cb(Fl_Widget*, void *v)
{
    auto w = (EditorWindow *) v;

    CFileOpenDialog dialog("Insert File");
    if (dialog.execute()) {
        String newfile = dialog.fullFileName();
        if (newfile.length()) load_file(newfile.c_str(), w->editor->insert_position());
    }
}

void paste_cb(Fl_Widget*, void* v)
{
    auto e = (EditorWindow*) v;
    Fl_Text_Editor::kf_paste(0, e->editor);
}

int num_windows = 0;

void close_cb(Fl_Widget*, void* v)
{
    auto w = (Fl_Window*) v;
    if (num_windows == 1 && !check_save()) {
        return;
    }

    w->hide();
    textbuf->remove_modify_callback(changed_cb, w);
    delete w;
    num_windows--;
    if (!num_windows) exit(0);
}

void quit_cb(Fl_Widget*, void*)
{
    if (changed && !check_save())
        return;

    exit(0);
}

void replace_cb(Fl_Widget*, void* v)
{
    auto e = (EditorWindow*) v;
    e->replace_dlg->show();
}

void replace2_cb(Fl_Widget*, void* v)
{
    auto e = (EditorWindow*) v;
    string find = e->replace_find->data();
    string replace = e->replace_with->data();

    if (find[0] == '\0') {
        // Search string is blank; get a new one...
        e->replace_dlg->show();
        return;
    }

    e->replace_dlg->hide();

    int pos = e->editor->insert_position();
    int found = textbuf->search_forward(pos, find.c_str(), &pos);

    if (found) {
        // Found a match; update the position and replace text...
        textbuf->select(pos, pos + (int) strlen(find.c_str()));
        textbuf->remove_selection();
        textbuf->insert(pos, replace.c_str());
        textbuf->select(pos, pos + (int) strlen(replace.c_str()));
        e->editor->insert_position(pos + (int) strlen(replace.c_str()));
        e->editor->show_insert_position();
    }
    else fl_alert("No occurrences of \'%s\' found!", find.c_str());
}

void replall_cb(Fl_Widget*, void* v)
{
    auto e = (EditorWindow*) v;
    string find = e->replace_find->data();
    string replace = e->replace_with->data();

    if (find[0] == '\0') {
        // Search string is blank; get a new one...
        e->replace_dlg->show();
        return;
    }

    e->replace_dlg->hide();

    e->editor->insert_position(0);
    int times = 0;

    // Loop through the whole string
    for (int found = 1; found;) {
        int pos = e->editor->insert_position();
        found = textbuf->search_forward(pos, find.c_str(), &pos);

        if (found) {
            // Found a match; update the position and replace text...
            textbuf->select(pos, pos + (int) strlen(find.c_str()));
            textbuf->remove_selection();
            textbuf->insert(pos, replace.c_str());
            e->editor->insert_position(pos + (int) strlen(replace.c_str()));
            e->editor->show_insert_position();
            times++;
        }
    }

    if (times) fl_message("Replaced %d occurrences.", times);
    else fl_alert("No occurrences of \'%s\' found!", find.c_str());
}

void replcan_cb(Fl_Widget*, void* v)
{
    auto e = (EditorWindow*) v;
    e->replace_dlg->hide();
}

void save_cb()
{
    if (filename[0] == '\0') {
        // No filename - get one!
        saveas_cb();
        return;
    }
    else save_file(filename);
}

void saveas_cb()
{
    CFileSaveDialog dialog;
    if (dialog.execute()) {
        String newfile = dialog.fullFileName();
        if (newfile.length()) save_file(newfile.c_str());
    }
}

CWindow* new_view();

void view_cb(Fl_Widget*, void*)
{
    Fl_Window* w = new_view();
    w->show();
}

Fl_Menu_Item menuitems[] = {
    { "&File", 0, nullptr, nullptr, FL_SUBMENU},
    { "&New File", 0, (Fl_Callback *) new_cb},
    { "&Open File...", FL_CTRL + 'o', (Fl_Callback *) open_cb},
    { "&Insert File...", FL_CTRL + 'i', (Fl_Callback *) insert_cb, nullptr, FL_MENU_DIVIDER},
    { "&Save File", FL_CTRL + 's', (Fl_Callback *) save_cb},
    { "Save File &As...", FL_CTRL + FL_SHIFT + 's', (Fl_Callback *) saveas_cb, nullptr, FL_MENU_DIVIDER},
    { "New &View", FL_ALT + 'v', (Fl_Callback *) view_cb, nullptr},
    { "&Close View", FL_CTRL + 'w', (Fl_Callback *) close_cb, nullptr, FL_MENU_DIVIDER},
    { "E&xit", FL_CTRL + 'q', (Fl_Callback *) quit_cb, nullptr},
    { nullptr },

    { "&Edit", 0, nullptr, nullptr, FL_SUBMENU},
    { "Cu&t", FL_CTRL + 'x', (Fl_Callback *) cut_cb},
    { "&Copy", FL_CTRL + 'c', (Fl_Callback *) copy_cb},
    { "&Paste", FL_CTRL + 'v', (Fl_Callback *) paste_cb},
    { "&Delete", 0, (Fl_Callback *) delete_cb},
    { nullptr },

    { "&Search", 0, nullptr, nullptr, FL_SUBMENU},
    { "&Find...", FL_CTRL + 'f', (Fl_Callback *) find_cb},
    { "F&ind Again", FL_CTRL + 'g', find2_cb},
    { "&Replace...", FL_CTRL + 'r', replace_cb},
    { "Re&place Again", FL_CTRL + 't', replace2_cb},
    { nullptr },

    { nullptr }
};

CWindow* new_view()
{
    auto w = new EditorWindow(660, 400, title);
    w->begin();

    auto m = new CMenuBar;
    m->copy(menuitems, w);

    w->editor = new CEditor(10, SP_ALIGN_CLIENT);
    w->editor->buffer(textbuf);
    w->editor->highlight_data(stylebuf, styletable,
        sizeof (styletable) / sizeof (styletable[0]),
        'A', style_unfinished_cb, nullptr);
    w->editor->textfont(FL_COURIER);
    w->editor->callback(cursor_cb);


    auto grp = new CGroup("", 20, SP_ALIGN_BOTTOM);
    grp->layoutSpacing(1);
    w->cursor_position = new CBox("", 150, SP_ALIGN_RIGHT);
    w->cursor_position->box(FL_THIN_DOWN_BOX);
    w->cursor_position->textSize(12);
    grp->end();

    w->end();
    w->resizable(w->editor);
    w->callback((Fl_Callback *) close_cb, w);
    w->relayout();

    textbuf->add_modify_callback(style_update, w->editor);
    textbuf->add_modify_callback(changed_cb, w);
    textbuf->call_modify_callbacks();
    num_windows++;
    return w;
}

int main(int argc, char *argv[])
{
    try {
        // Initialize themes
        CThemes themes;

        int fontCount;
#ifdef _WIN32
        fontCount = Fl::set_fonts("*");
#elif __APPLE__
        fontCount = Fl::set_fonts("*");
#else
        // Load the systems available fonts - ask for everything that claims to be iso10646 compatible
        fontCount = Fl::set_fonts("-*-*-*-*-*-*-*-*-*-*-*-*-iso10646-1");
#endif

        // If any unicode font found, set the first found for two default fonts.
        // This is only good for the simple example.
        if (fontCount) {
            Fl::set_font(FL_HELVETICA, FL_FREE_FONT);
            Fl::set_font(FL_COURIER, FL_FREE_FONT);
        }

        textbuf = new Fl_Text_Buffer;
        style_init();

        CWindow* window = new_view();

        window->show(1, argv);

        if (argc > 1) load_file(argv[1], -1);

        CThemes::set("Keramic");

        return Fl::run();
    }
    catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }
}
