/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CCommandLine.cpp  -  description
                             -------------------
    begin                : Jun 20 2015
    copyright            : (C) 1999-2015 by Alexey Parshin. All rights reserved.
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

#include <sptk5/CCommandLine.h>
#include <iomanip>

using namespace std;
using namespace sptk;

map<string, CArgumentDefinition*> CArgumentDefinition::argumentNamesAndShortcuts;

CArgumentDefinition::CArgumentDefinition(string name, string valueType, char shortcut, string help, Type type, string defaultValue, string validateRegexp)
: m_name(name), m_valueType(valueType), m_shortcut(shortcut), m_help(help), m_type(type), m_defaultValue(defaultValue)
{
    if (!name.empty())
        argumentNamesAndShortcuts[name] = this;
    if (shortcut != char(0))
        argumentNamesAndShortcuts[string(1,shortcut)] = this;
    if (validateRegexp.empty())
        m_validateRegexp = NULL;
    else
        m_validateRegexp = new sptk::CRegExp(validateRegexp);
}

void CArgumentDefinition::validate(string value)
{
    if (!m_validateRegexp)
        return;
    if (m_validateRegexp && *m_validateRegexp == value)
        return;
    throw sptk::CException("Value '" + value + "' is invalid for parameter --" + m_name);
}

CArgumentDefinition* CArgumentDefinition::get(string nameOrShortcut)
{
    map<string, CArgumentDefinition*>::iterator itor = argumentNamesAndShortcuts.find(nameOrShortcut);
    if (itor == argumentNamesAndShortcuts.end())
        return NULL;
    return itor->second;
}

void CCommandLine::init(int argc, const char* argv[]) throw (exception)
{
    m_progname = argv[0];

    // Convert combined options into singles
    CStrings arguments;
    for (int i = 1; i < argc; i++) {
        const char* arg = argv[i];

        if (arg[0] != '-') {
            // Argument isn't an option
            arguments.push_back(arg);
            continue;
        }

        if (arg[1] == '-') {
            // Argument is a parameter
            arguments.push_back(arg);
            continue;
        }

        // Argument is on or more options
        for (size_t j = 1; j < strlen(arg); j++) {
            string option(1, arg[j]);
            CArgumentDefinition* argdef = CArgumentDefinition::get(option);
            if (!argdef)
                throw CException("Option -" + option + " is not defined");
            arguments.push_back("--" + argdef->name());
        }
    }

    m_names.clear();
    for (size_t i = 0; i < arguments.size(); i++) {
        const string& argument = arguments[i];

        if (argument.substr(0,2) != "--") {
            if (!m_allowNames)
                throw CException("Argument " + argument + " is not an option or parameter name");
            m_names.push_back(argument);
        }

        string name = argument.substr(2);
        CArgumentDefinition* argdef = CArgumentDefinition::get(name);
        if (!argdef)
            throw CException("Option --" + name + " is unknown");

        if (argdef->type() == CArgumentDefinition::Type::OPTION) {
            m_options.insert(argdef->name());
            continue;
        }

        i++;
        if (i > arguments.size())
            throw CException("Parameter " + argument + " doesn't have a value");

        const string& value = arguments[i];
        argdef->validate(value);
        m_parameters[name] = value;
    }
}

bool CCommandLine::hasOption(string name) const throw (exception)
{
    if (m_options.find(name) != m_options.end())
        return true;

    CArgumentDefinition* argdef = CArgumentDefinition::get(name);
    if (!argdef || argdef->type() != CArgumentDefinition::Type::OPTION)
        throw CException("Option --" + name + " is invalid");

    return false;
}

string CCommandLine::parameterValue(string name) const throw (exception)
{
    map<string,string>::const_iterator itor = m_parameters.find(name);
    if (itor != m_parameters.end())
        return itor->second;

    CArgumentDefinition* argdef = CArgumentDefinition::get(name);
    if (!argdef || argdef->type() != CArgumentDefinition::Type::PARAMETER)
        throw CException("Parameter --" + name + " is invalid");

    return argdef->defaultValue();
}

void formatText(CStrings& text, string fromText, size_t width)
{
    CStrings words(fromText, " \n\r", CStrings::SM_ANYCHAR);
    string   row;

    for (string word: words) {
        if (row.length() + word.length() >= width) {
            text.push_back(row);
            row = "";
        }
        if (!row.empty())
            row += " ";
        row += word;
    }

    if (!row.empty())
        text.push_back(row);
}

void CCommandLine::printTypeHelp(CArgumentDefinition::Type type, unsigned screenColumns, unsigned commandColumns) const
{
    switch (type) {
        case CArgumentDefinition::Type::PARAMETER:
            cout << "Parameters:" << endl << endl;
            break;
        case CArgumentDefinition::Type::OPTION:
            cout << "Options:" << endl << endl;
            break;
    }

    if (screenColumns < commandColumns + 20)
        screenColumns = commandColumns + 20;

    for (auto& itor: CArgumentDefinition::definitions()) {
        string name = itor.first;
        const CArgumentDefinition* argdef = itor.second;

        if (argdef->type() == type && name == argdef->name()) {
            if (argdef->shortcut() != char(0))
                name += "|-" + string(1,argdef->shortcut());

            if (!argdef->valueType().empty())
                name += " <" + argdef->valueType() + ">";

            CStrings helpText;
            string description = replaceAll(argdef->help(), "${DEFAULT}", argdef->defaultValue());
            formatText(helpText, description, screenColumns - commandColumns);
            int rowNumber = 0;
            for (string& helpRow: helpText) {
                if (rowNumber == 0)
                    cout << setw(commandColumns) << left << "--" + name;
                else
                    cout << setw(commandColumns) << " ";
                cout << helpRow << endl;
                rowNumber++;
            }
        }
    }
    cout << endl;
}

void CCommandLine::printHelp(string argumentType) const
{
    unsigned screenColumns = 80;
    const char* envcols = getenv("COLS");
    if (envcols)
        screenColumns = atoi(envcols);
#ifndef WIN32
    else {
        const char* envterm = getenv("TERM");
        if (envterm) {
            FILE* tput = popen("tput cols", "r");
            if (tput) {
                fscanf(tput, "%u", &screenColumns);
                fclose(tput);
            }
        }
    }
#endif

    size_t maxNameLength = 0;
    bool hasOptions = false, hasParameters = false;
    for (auto& itor: CArgumentDefinition::definitions()) {
        CArgumentDefinition* argdef = itor.second;
        switch (argdef->type()) {
            case CArgumentDefinition::PARAMETER:
                hasParameters = true;
                break;
            case CArgumentDefinition::OPTION:
                hasOptions = true;
                break;
        }
        size_t nameLength = argdef->name().length() + 2;
        if (!argdef->valueType().empty())
            nameLength += argdef->valueType().length() + 3;
        if (argdef->shortcut() != char(0))
            nameLength += 3;
        if (maxNameLength < nameLength)
            maxNameLength = nameLength;
    }

    cout << "Syntax:\n\n";
    cout << " " << m_progname << " ";
    if (hasOptions)
        cout << "[options] ";
    if (hasParameters)
        cout << "[parameters] ";
    if (m_allowNames)
        cout << argumentType << "(s)";
    cout << endl;

    cout << "" << endl;

    if (hasParameters)
        printTypeHelp(CArgumentDefinition::Type::PARAMETER, screenColumns, maxNameLength + 2);

    if (hasOptions)
        printTypeHelp(CArgumentDefinition::Type::OPTION, screenColumns, maxNameLength + 2);
}