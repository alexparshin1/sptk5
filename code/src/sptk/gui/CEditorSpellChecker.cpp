/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CEditorSpellChecker.cpp - description                  ║
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

#include <FL/fl_ask.H>
#include <sptk5/gui/CEditorSpellChecker.h>
#include <aspell.h>

using namespace std;
using namespace sptk;

CSpellOption::CSpellOption(const String& name, const String& value)
{
    m_name = name;
    m_value = value;
}

CSpellOption& CSpellOption::operator=(const char* value)
{
    m_value = value;
    return *this;
}

CSpellOption& CSpellOption::operator=(const String& value)
{
    m_value = value;
    return *this;
}

void CSpellChecker::cb_learn(Fl_Widget* w, void*)
{
    auto spellChecker = (CSpellChecker*) w->window();
    spellChecker->learnAndClose();
}

void CSpellChecker::cb_ignore(Fl_Widget* w, void*)
{
    auto spellChecker = (CSpellChecker*) w->window();
    spellChecker->ignoreAndClose();
}

void CSpellChecker::cb_replaceword(Fl_Widget* w, void*)
{
    auto spellChecker = (CSpellChecker*) w->window();
    String word = trim(spellChecker->m_replaceToInput->data());
    if (word.length())
        spellChecker->m_okButton->activate();
    else spellChecker->m_okButton->deactivate();
}

void CSpellChecker::cb_suggest(Fl_Widget* lv, void*)
{
    auto listView = (CListView*) lv;
    if (listView->eventType() == CE_DATA_CHANGED) {
        auto spellChecker = (CSpellChecker*) listView->window();
        spellChecker->m_replaceToInput->data(listView->data());
        String word = trim(spellChecker->m_replaceToInput->data());
        if (word.length())
            spellChecker->m_okButton->activate();
        else spellChecker->m_okButton->deactivate();
    }
}

void CSpellChecker::learnAndClose()
{
    string word = m_wordInput->data();
    if (m_spellChecker) {
        aspell_speller_add_to_personal(m_spellChecker, word.c_str(), (int) word.length());
    }
    m_modalResult = DMR_USER;
}

void CSpellChecker::ignoreAndClose()
{
    string word = m_wordInput->data();
    if (m_spellChecker)
        aspell_speller_add_to_session(m_spellChecker, word.c_str(), (int) word.length());
    m_modalResult = DMR_USER;
}

CSpellChecker::CSpellChecker()
        : CDialog(340, 300, "Spell Check")
{
    m_spellChecker = nullptr;

    layoutSpacing(1);

    m_wordInput = new CInput("Word:", 10, SP_ALIGN_TOP);
    m_wordInput->labelWidth(80);
    m_wordInput->deactivate();

    m_replaceToInput = new CInput("Replace To:", 10, SP_ALIGN_TOP);
    m_replaceToInput->labelWidth(80);
    m_replaceToInput->callback(CSpellChecker::cb_replaceword);

    m_suggestionListView = new CListView("", 10, SP_ALIGN_CLIENT);
    m_suggestionListView->addColumn("Suggestions", VAR_STRING, 310);
    m_suggestionListView->callback(CSpellChecker::cb_suggest);

    addExtraButton(SP_ADD_BUTTON, "Add", CSpellChecker::cb_learn);
    addExtraButton(SP_NO_BUTTON, "Ignore", CSpellChecker::cb_ignore);

    m_okButton->label("Replace");
    m_cancelButton->label("Stop");

    CDialog::end();
#ifdef _WIN32
    (*this)["dict-dir"] = "C:\\Program Files\\Aspell\\dict";
    (*this)["data-dir"] = "C:\\Program Files\\Aspell\\data";

    if (m_personalDirectory == "")
       m_personalDirectory = "C:\\Program Files\\Aspell";

    (*this)["personal"] = m_personalDirectory + "\\personal.pws";
    (*this)["repl"]     = m_personalDirectory + "\\personal.prepl";
 /*
    (*this)["word-list-path"] = "C:\\Program Files\\Aspell\\dict";
    (*this)["master"] = "master.pwl";
 */
#endif
}

CSpellOption& CSpellChecker::operator[](const String& optionName)
{
    auto itor = CSpellOptions::find(optionName);
    if (itor == CSpellOptions::end()) {
        AspellConfig* aconfig = new_aspell_config();
        string val = aspell_config_retrieve(aconfig, optionName.c_str());
        delete_aspell_config(aconfig);
        CSpellOption newOption(optionName, val);
        CSpellOptions::insert(CSpellOptions::value_type(optionName, newOption));
        itor = CSpellOptions::find(optionName);
    }
    return itor->second;
}

void CSpellChecker::getConfigStrings(AspellConfig* aconfig, CSpellOptions& options)
{
    options.clear();
    AspellKeyInfoEnumeration* keyInfoElements = aspell_config_possible_elements(aconfig, false);
    do {
        const AspellKeyInfo* keyInfo = aspell_key_info_enumeration_next(keyInfoElements);
        if (!keyInfo) break;
        string val = aspell_config_retrieve(aconfig, keyInfo->name);
        CSpellOption option(keyInfo->name, val);
        options.insert(CSpellOptions::value_type(keyInfo->name, option));
    } while (!aspell_key_info_enumeration_at_end(keyInfoElements));
}

void CSpellChecker::getDictionaries(Strings& dictionaries)
{
    AspellConfig* aconfig = new_aspell_config();
    AspellDictInfoList* dictInfoList = get_aspell_dict_info_list(aconfig);
    setLocalOptions(aconfig);
    delete_aspell_config(aconfig);

    AspellDictInfoEnumeration* dictInfoElements = aspell_dict_info_list_elements(dictInfoList);
    while (!aspell_dict_info_enumeration_at_end(dictInfoElements)) {
        const AspellDictInfo* dictInfo = aspell_dict_info_enumeration_next(dictInfoElements);
        dictionaries.push_back(dictInfo->name);
    }
    delete_aspell_dict_info_enumeration(dictInfoElements);
}

void CSpellChecker::setLocalOptions(AspellConfig* aconfig)
{
    auto itor = CSpellOptions::begin();
    for (; itor != CSpellOptions::end(); ++itor) {
        CSpellOption& option = itor->second;
        aspell_config_replace(aconfig, itor->first.c_str(), option.m_value.c_str());
    }
}

void CSpellChecker::getOptions(CSpellOptions& options)
{
    AspellConfig* aconfig = new_aspell_config();

    setLocalOptions(aconfig);
    getConfigStrings(aconfig, options);

    delete_aspell_config(aconfig);
}

void CSpellChecker::checkForError()
{
    if (!m_spellChecker) return;
    if (aspell_speller_error(m_spellChecker) != nullptr) {
        throw Exception(aspell_speller_error_message(m_spellChecker));
    }
}

bool CSpellChecker::spellCheck()
{

    if (m_spellChecker) {
        delete_aspell_speller(m_spellChecker);
        m_spellChecker = nullptr;
    }

    AspellConfig* aconfig = new_aspell_config();

    setLocalOptions(aconfig);

    AspellCanHaveError* possibleError = new_aspell_speller(aconfig);

    if (aspell_error_number(possibleError)) {
        delete_aspell_config(aconfig);
        return false;
    }

    m_spellChecker = to_aspell_speller(possibleError);

    delete_aspell_config(aconfig);

    textStart();
    String word;

    bool rc = true;
    int wordStart, wordEnd;
    while (getNextWord(word, wordStart, wordEnd)) {
        if (strpbrk(word.c_str(), "0123456789")) continue;
        int result = aspell_speller_check(m_spellChecker, word.c_str(), (int) word.length());
        if (result != 1) {
            m_wordInput->data(word);
            m_suggestionListView->clear();
            const AspellWordList* suggestions = aspell_speller_suggest(m_spellChecker, word.c_str(), (int) word.length());
            AspellStringEnumeration* elements = aspell_word_list_elements(suggestions);
            string best;
            bool bestAssigned = false;
            while (!aspell_string_enumeration_at_end(elements)) {
                const char* nextWord = aspell_string_enumeration_next(elements);
                if (nextWord)
                    m_suggestionListView->addRow(0, Strings(nextWord, "|"));
                else break;
                if (!bestAssigned) {
                    best = nextWord;
                    bestAssigned = true;
                }
            }
            m_replaceToInput->data(best);
            m_okButton->deactivate();
            if (!showModal()) {
                rc = false;
                break;
            }
            if (modalResult() == DMR_OK)
                replaceWord(m_replaceToInput->data(), wordStart, wordEnd);
        }
    }

    aspell_speller_save_all_word_lists(m_spellChecker);
    checkForError();

    delete_aspell_speller(m_spellChecker);
    m_spellChecker = nullptr;
    return rc;
}

void CEditorSpellChecker::textStart()
{
    m_bufferPosition = 0;
}

bool CEditorSpellChecker::getNextWord(String& w, int& wordStart, int& wordEnd)
{
    Fl_Text_Buffer* buffer = m_editor->textBuffer();
    for (;;) {
        auto ch = (char) buffer->char_at(m_bufferPosition);
        if (ch == 0)
            return false;
        if (isalnum(ch)) {
            wordStart = m_bufferPosition;
            wordEnd = buffer->word_end(m_bufferPosition);
            w = buffer->text_range(m_bufferPosition, wordEnd);
            m_bufferPosition = uint32_t(wordEnd + 1);
            return true;
        }
        m_bufferPosition++;
    }
}

bool CEditorSpellChecker::replaceWord(const String& word, int wordStart, int wordEnd)
{
    Fl_Text_Buffer* buffer = m_editor->textBuffer();
    buffer->replace(wordStart, wordEnd, word.c_str());
    return true;
}
