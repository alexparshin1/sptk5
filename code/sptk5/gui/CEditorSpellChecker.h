/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CEditorSpellChecker.h - description                    ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Wednesday November 2 2005                              ║
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

#ifndef __CEDITORSPELLCHECKER_H__
#define __CEDITORSPELLCHECKER_H__

#include <FL/Fl.H>

#include <sptk5/sptk.h>
#include <sptk5/cgui>

#include <map>

struct AspellSpeller;
struct AspellConfig;

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * CSpellOption represents an option of Aspell config.
 */
    class CSpellOption
    {
    public:
        /**
         * Option name
         */
        String m_name;

        /**
         * Option value
         */
        String m_value;

    public:
        /**
         * Constructor
         */
        CSpellOption(const String& name, const String& value);

        /**
         * Copy constructor
         */
        CSpellOption(const CSpellOption& co)
        {
            m_name = co.m_name;
            m_value = co.m_value;
        }

        /**
         * Assigns a new value to the option
         */
        CSpellOption& operator=(const char* value);

        /**
         * Assigns a new value to the option
         */
        CSpellOption& operator=(const String& value);
    };

/**
 * CSpellOptions is a map of CSpellOption.
 */
    class CSpellOptions : public std::map<String, CSpellOption>
    {

    public:
        /**
         * Default constructor
         */
        CSpellOptions()
        {}
    };

/**
 * CSpellChecker is a base class for Aspell interface.
 */
    class CSpellChecker : public CSpellOptions, public CDialog
    {
        /**
         * Input widget to show the misspelled word
         */
        CInput* m_wordInput;

        /**
         * Input widget to show the replacement word
         */
        CInput* m_replaceToInput;

        /**
         * List view widget to show the list of suggestions
         */
        CListView* m_suggestionListView;

        /**
         * Pointer to the Aspell spell checker
         */
        AspellSpeller* m_spellChecker;

#ifdef _WIN32

        /**
         * Full path to personal directory on Windows
         */
        std::string    m_personalDirectory;

#endif

        /**
         * Loads all Aspell config options into list of options
         * @param aconfig AspellConfig *, Aspell config structure
         * @param options CSpellOptions &, config options (output)
         */
        void getConfigStrings(AspellConfig* aconfig, CSpellOptions& options);

        /**
         * Saves changed (local) Aspell config options from internal list of options
         * @param aconfig AspellConfig *, Aspell config structure
         */
        void setLocalOptions(AspellConfig* aconfig);

    protected:
        /**
         * Callback for the add word button
         */
        static void cb_learn(Fl_Widget*, void*);

        /**
         * Callback for the ignore word button
         */
        static void cb_ignore(Fl_Widget*, void*);

        /**
         * Callback for the suggestion list click
         */
        static void cb_suggest(Fl_Widget*, void*);

        /**
         * Callback for the replace word button
         */
        static void cb_replaceword(Fl_Widget*, void*);


        /**
         * Adds a word to personal dictionary and hides window to continue spelling
         */
        void learnAndClose();

        /**
         * Adds a word to ignore list and hides window to continue spelling
         */
        void ignoreAndClose();

        /**
         * Checks for error after the operation and throws CException if error is detected
         */
        void checkForError();

    public:

        /**
         * Default constructor
         */
        CSpellChecker();

        /**
         * Destructor
         */
        virtual ~CSpellChecker()
        {}

        /**
         * Sets the dictionary for the spell checking
         * @see getDictionaries for obtaining the list of available dictionaries
         */
        void selectDictionary(std::string dictionary);

        /**
         * Defines the directory where Aspell data (language) files are stored
         */
        void languageDataDirectory(std::string path);

        /**
         * Defines the directory where Aspell dictionary files are stored
         */
        void dictionaryDirectory(std::string path);

        /**
         * Downloads the list of installed dictionaries
         */
        void getDictionaries(Strings& dictionaries);

        /**
         * Downloads the Aspell options
         */
        void getOptions(CSpellOptions& options);

        /**
         * Resets the spell check position. Purely virtual, should be implemented in the derived class.
         */
        virtual void textStart()=0;

        /**
         * Gets the next word and advances the spell check position. Purely virtual, should be implemented in the derived class.
         */
        virtual bool getNextWord(String& w, int& wordStart, int& wordEnd)
        {
            return false;
        }

        /**
         * Replaces the word and advances the spell check position. Purely virtual, should be implemented in the derived class.
         */
        virtual bool replaceWord(const String& w, int wordStart, int wordEnd)
        {
            return false;
        }

        /**
         * Spell checks the text. Uses textStart(), getNextWord(), and replaceWord() to go through the text.
         */
        bool spellCheck();

        /**
         * Access (read and write) to the particular Aspell option.
         */
        CSpellOption& operator[](const String& optionName);

#ifdef _WIN32
        /**
         * Sets full path to personal directory on Windows
         */
        void personalDirectory(const String& dir)
        {
            m_personalDirectory = dir;
        }

        /**
         * Returns full path to personal directory on Windows
         */
        String personalDirectory() const
        {
            return  m_personalDirectory;
        }
#endif

    };

/**
 * Specialized spell checker to use with CEditor class.
 */
    class CEditorSpellChecker : public CSpellChecker
    {
        /**
         * CEditor widget containing the spelling text
         */
        CEditor* m_editor;

        /**
         * Current spell checking word buffer position
         */
        uint32_t m_bufferPosition;

    public:
        /**
         * Constructor
         * @param editor CEditor *, widget to work with
         */
        CEditorSpellChecker(CEditor* editor)
        {
            m_editor = editor;
            m_bufferPosition = 0;
        }

        /**
         * Resets the spell check position
         */
        virtual void textStart();

        /**
         * Gets the next word and advances the spell check position
         */
        virtual bool getNextWord(String& w, int& wordStart, int& wordEnd);

        /**
         * Replaces the word and advances the spell check position
         */
        virtual bool replaceWord(const String& w, int wordStart, int wordEnd);
    };
/**
 * @}
 */
}
#endif
