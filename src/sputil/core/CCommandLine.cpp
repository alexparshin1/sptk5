/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CCommandLine.cpp - description                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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

#include <sptk5/CCommandLine.h>

using namespace std;
using namespace sptk;

CCommandLine::Visibility::Visibility(string pattern, bool _mustMatch)
: m_inverted(!_mustMatch), m_pattern(pattern)
{
    if (m_pattern.empty())
        m_regexp = NULL;
    else
        m_regexp = new CRegExp(m_pattern);
}

CCommandLine::Visibility::Visibility(const Visibility& other)
: m_inverted(other.m_inverted), m_pattern(other.m_pattern)
{
    if (m_pattern.empty())
        m_regexp = NULL;
    else
        m_regexp = new CRegExp(m_pattern);
}

CCommandLine::Visibility::~Visibility()
{
    if (m_regexp)
        delete m_regexp;
}

bool CCommandLine::Visibility::any() const
{
    return m_regexp == NULL;
}

bool CCommandLine::Visibility::matches(string command) const
{
    CStrings matches;
    if (m_inverted)
        return !m_regexp->m(command, matches);
    return m_regexp->m(command, matches);
}
//=============================================================================

CCommandLine::CommandLineElement::CommandLineElement(string name, string shortName, string help,
    const Visibility& useWithCommands)
: m_name(name), m_shortName(shortName), m_help(help), m_useWithCommands(useWithCommands)
{
    if (m_name.empty())
        throw CException("Command line elements must have a name");
}

CCommandLine::CommandLineElement::~CommandLineElement() { }

CCommandLine::CommandLineElement::Type CCommandLine::CommandLineElement::type() const
{
    return IS_UNKNOWN;
}

string CCommandLine::CommandLineElement::name() const
{
    return m_name;
}

bool CCommandLine::CommandLineElement::hasValue() const
{
    return false;
}

void CCommandLine::CommandLineElement::validate(string value) const { }

string CCommandLine::CommandLineElement::printableName() const
{
    return m_name;
}

bool CCommandLine::CommandLineElement::useWithCommand(string command) const
{
    if (command.empty())
        return true;
    if (m_useWithCommands.any())
        return true;
    return m_useWithCommands.matches(command);
}

void CCommandLine::CommandLineElement::formatHelp(size_t textWidth, CStrings& formattedText) const
{
    CStrings words(m_help, "\\s+", CStrings::SM_REGEXP);

    formattedText.clear();

    string row = "";
    for (string word : words) {
        if (row.empty()) {
            row = word;
            continue;
        }
        if ((row.length() + word.length() + 1) > textWidth) {
            formattedText.push_back(row);
            row = word;
            continue;
        }
        row += " " + word;
    }
    if (!row.empty())
        formattedText.push_back(row);
}

void CCommandLine::CommandLineElement::printHelp(size_t nameWidth, size_t textWidth, string optionDefaultValue) const
{
    static const CRegExp doesntNeedQuotes("[\\d\\.\\-\\+:,_]+");

    CStrings helpText;
    formatHelp(textWidth, helpText);
    bool firstRow = true;
    string printFormat = "%-" + int2string(nameWidth) + "s  %s";
    char rowBuffer[1024];
    for (string helpRow : helpText) {
        if (firstRow) {
            sprintf(rowBuffer, printFormat.c_str(), printableName().c_str(), helpRow.c_str());
            cout << rowBuffer << endl;
            firstRow = false;
        }
        else {
            sprintf(rowBuffer, printFormat.c_str(), "", helpRow.c_str());
            cout << rowBuffer << endl;
        }
    }

    if (!optionDefaultValue.empty()) {
        CStrings matches;
        if (!doesntNeedQuotes.m(optionDefaultValue, matches))
            optionDefaultValue = "'" + optionDefaultValue + "'";
        string defaultValueStr = "The default value is " + optionDefaultValue + ".";
        sprintf(rowBuffer, printFormat.c_str(), "", defaultValueStr.c_str());
        cout << rowBuffer << endl;
    }
}
//=============================================================================

CCommandLine::CommandLineArgument::CommandLineArgument(string name, string help)
: CommandLineElement(name, "", help, Visibility("")) { }

CCommandLine::CommandLineArgument::~CommandLineArgument() { }
//=============================================================================

CCommandLine::CommandLineOption::CommandLineOption(string name, string shortName,
    const Visibility& useWithCommands, string help)
: CommandLineElement(name, shortName, help, useWithCommands) { }

CCommandLine::CommandLineOption::~CommandLineOption() { }

bool CCommandLine::CommandLineOption::hasValue() const
{
    return false;
}

CCommandLine::CommandLineElement::Type CCommandLine::CommandLineOption::type() const
{
    return CommandLineElement::IS_OPTION;
}

string CCommandLine::CommandLineOption::printableName() const
{
    string result = "";

    result += "--" + m_name;

    if (!result.empty())
        result += ", ";

    if (!m_shortName.empty())
        result += "-" + m_shortName;

    return result;
}
//=============================================================================

CCommandLine::CommandLineParameter::CommandLineParameter(string name, string shortName, string valueInfo,
    string validateValue, const Visibility& useWithCommands, string help)
: CommandLineElement(name, shortName, help, useWithCommands), m_valueInfo(valueInfo)
{
    if (validateValue.empty())
        m_validateValue = NULL;
    else
        m_validateValue = new CRegExp(validateValue);
    if (m_valueInfo.empty())
        throw CException("Command line parameters must have a value info");
}

CCommandLine::CommandLineParameter::~CommandLineParameter()
{
    if (m_validateValue)
        delete m_validateValue;
}

string CCommandLine::CommandLineParameter::printableName() const
{
    string result = "";

    result += "--" + m_name;

    if (!result.empty())
        result += ", ";

    if (!m_shortName.empty())
        result += "-" + m_shortName;

    result += " <" + m_valueInfo + ">";

    return result;
}

void CCommandLine::CommandLineParameter::validate(string value) const
{
    if (!m_validateValue)
        return;
    CStrings matches;
    if (!m_validateValue->m(value, matches))
        throw CException("Parameter " + m_name + " has invalid value");
}

bool CCommandLine::CommandLineParameter::hasValue() const
{
    return true;
}

CCommandLine::CommandLineElement::Type CCommandLine::CommandLineParameter::type() const
{
    return IS_VALUE_OPTION;
}
//=============================================================================

bool CCommandLine::startsWith(string str, string pattern)
{
    return str.substr(0, pattern.length()) == pattern;
}

bool CCommandLine::endsWith(string str, string pattern)
{
    size_t pos = str.length() - pattern.length() - 1;
    if (int(pos) < 0)
        return false;
    return str.substr(pos) == pattern;
}

CCommandLine::CCommandLine(string programVersion, string description, string commandLinePrototype)
: m_programVersion(programVersion), m_description(description), m_commandLinePrototype(commandLinePrototype) { }

CCommandLine::~CCommandLine()
{
    for (CommandLineElement* element : m_allElements)
        delete element;
}

void CCommandLine::defineOption(string fullName, string shortName, Visibility useForCommands, string help)
{
    if (fullName.empty() && shortName.empty())
        return;

    CommandLineOption* optionTemplate = new CommandLineOption(fullName, shortName, useForCommands, help);
    m_allElements.push_back(optionTemplate);
    if (!fullName.empty())
        m_optionTemplates[fullName] = optionTemplate;
    if (!shortName.empty())
        m_optionTemplates[shortName] = optionTemplate;
}

void CCommandLine::defineParameter(string fullName, string shortName, string valueName,
    string validateValue, Visibility useForCommands, string defaultValue, string help)
{
    if (fullName.empty() && shortName.empty())
        return;

    CommandLineParameter* argumentTemplate = new CommandLineParameter(fullName, shortName, valueName, validateValue,
        useForCommands, help);
    m_allElements.push_back(argumentTemplate);

    string name;
    if (!shortName.empty()) {
        m_optionTemplates[shortName] = argumentTemplate;
        name = shortName;
    }

    if (!fullName.empty()) {
        m_optionTemplates[fullName] = argumentTemplate;
        name = fullName;
    }

    if (!defaultValue.empty()) {
        argumentTemplate->validate(defaultValue);
        m_values[name] = defaultValue;
    }
}

void CCommandLine::defineArgument(string fullName, string helpText)
{
    if (!fullName.empty()) {
        CommandLineArgument* argumentTemplate = new CommandLineArgument(fullName, helpText);
        m_allElements.push_back(argumentTemplate);
        m_argumentTemplates[fullName] = argumentTemplate;
    }
}

void CCommandLine::init(int argc, const char* argv[])
{
    CStrings args;
    for (int i = 1; i < argc; i++)
        args.push_back(string(argv[i]));

    // Pre-process command line arguments
    CStrings arguments;
    string quote = "";
    string quotedString = "";
    for (string arg : args) {
        if (quote.empty()) {
            if (startsWith(arg, "'")) {
                quote = arg.substr(0, 1);
                quotedString = arg.substr(1);
                if (endsWith(quotedString, quote)) {
                    quotedString = quotedString.substr(0, arg.length() - 1);
                    arguments.push_back(quotedString);
                    quote = "";
                    quotedString = "";
                }
            }
            else {
                arguments.push_back(arg);
                continue;
            }
        }
        else {
            if (endsWith(arg, quote)) {
                arg = arg.substr(0, arg.length() - 1);
                quote = "";
                quotedString += " " + arg;
                arguments.push_back(quotedString);
            }
            else
                quotedString += " " + arg;
        }
    }

    // Re-write arguments
    CStrings digestedArgs;
    for (string arg : arguments) {
        if (startsWith(arg, "--")) {
            // Full option name
            digestedArgs.push_back(arg);
            continue;
        }

        if (startsWith(arg, "-")) {
            // Short option name(s)
            for (unsigned j = 1; j < arg.length(); j++) {
                string opt = "-" + arg.substr(j, j + 1);
                digestedArgs.push_back(opt);
            }
            continue;
        }

        digestedArgs.push_back(arg);
    }

    for (unsigned i = 0; i < digestedArgs.size(); i++) {
        string arg = digestedArgs[i];
        string value;

        if (startsWith(arg, "-")) {
            string optionName;
            if (startsWith(arg, "--")) {
                // Full option name
                optionName = arg.substr(2);
            }
            else {
                // Short option name
                optionName = arg.substr(1);
            }
            CommandLineElement* element = m_optionTemplates[optionName];
            if (element == NULL)
                throw CException("Command line option or parameter " + arg + " is not supported");
            if (element->hasValue()) {
                i++;
                if (i >= digestedArgs.size())
                    throw CException("Command line parameter " + arg + " should have value");
                value = digestedArgs[i];
                element->validate(value);
                m_values[element->name()] = value;
            }
            else
                m_values[element->name()] = "true";
            continue;
        }

        m_arguments.push_back(arg);
    }
}

string CCommandLine::getOptionValue(string name) const
{
    map<string,string>::const_iterator itor = m_values.find(name);
    if (itor == m_values.end())
        return "";
    return itor->second;
}

bool CCommandLine::hasOption(string name) const
{
    return m_values.find(name) != m_values.end();
}

void CCommandLine::setOptionValue(string name, string value)
{
    CommandLineElement* element = m_optionTemplates[name];
    if (!element)
        throw CException("Invalid option or parameter name: " + name);
    element->validate(value);
    m_values[name] = value;
}

const CStrings& CCommandLine::arguments() const
{
    return m_arguments;
}

void CCommandLine::printLine(string ch, size_t count)
{
    for (size_t i = 0; i < count; i++)
        cout << ch;
    cout << endl;
}

void CCommandLine::printHelp(size_t screenColumns) const
{
    printHelp("", screenColumns);
}

void CCommandLine::printHelp(string onlyForCommand, size_t screenColumns) const
{
    if (!onlyForCommand.empty() && m_argumentTemplates.find(onlyForCommand) == m_argumentTemplates.end()) {
        cerr << "Command '" + onlyForCommand + "' is not defined" << endl;
        return;
    }

    cout << m_programVersion << endl;
    printLine("═", screenColumns);
    cout << m_description << endl;

    cout << "\nSyntax:" << endl;
    printLine("─", screenColumns);

    string commandLinePrototype = m_commandLinePrototype;
    if (!onlyForCommand.empty())
        commandLinePrototype = replaceAll(m_commandLinePrototype, "<command>", onlyForCommand);
    cout << commandLinePrototype << endl;

    // Find out space needed for command and option names
    size_t nameColumns = 10;
    CStrings sortedCommands;
    for (auto& itor : m_argumentTemplates)
        sortedCommands.push_back(itor.first);

    for (string commandName : sortedCommands) {
        if (!onlyForCommand.empty() && commandName != onlyForCommand)
            continue;
        if (nameColumns < commandName.length())
            nameColumns = commandName.length();
    }

    CStrings sortedOptions;
    for (auto& itor : m_optionTemplates) {
        string optionName = itor.first;
        if (optionName.length() > 1)
            sortedOptions.push_back(optionName);
    }

    map<string,CommandLineElement*>::const_iterator itor;

    for (string optionName : sortedOptions) {
        itor = m_optionTemplates.find(optionName);
        const CommandLineElement* optionTemplate = itor->second;
        if (!optionTemplate->useWithCommand(onlyForCommand))
            continue;
        size_t width = optionTemplate->printableName().length();
        if (nameColumns < width)
            nameColumns = width;
    }

    size_t helpTextColumns = screenColumns - (nameColumns + 2);
    if ((int)helpTextColumns < 10) {
        cerr << "Can't print help information - the screen width is too small" << endl;
        return;
    }

    if (onlyForCommand.empty() && !m_argumentTemplates.empty()) {
        cout << "\nCommands:" << endl;
        printLine("─", screenColumns);
        for (string commandName : sortedCommands) {
            map<string,CommandLineArgument*>::const_iterator itor = m_argumentTemplates.find(commandName);
            const CommandLineArgument* commandTemplate = itor->second;
            if (!onlyForCommand.empty() && commandName != onlyForCommand)
                continue;
            commandTemplate->printHelp(nameColumns, helpTextColumns, "");
        }
    }

    if (!m_optionTemplates.empty()) {
        cout << "\nOptions:" << endl;
        printLine("─", screenColumns);
        for (string optionName : sortedOptions) {
            itor = m_optionTemplates.find(optionName);
            const CommandLineElement* optionTemplate = itor->second;
            if (!optionTemplate->useWithCommand(onlyForCommand))
                continue;
            string defaultValue;
            map<string,string>::const_iterator vtor = m_values.find(optionTemplate->name());
            if (vtor != m_values.end())
                defaultValue = vtor->second;
            optionTemplate->printHelp(nameColumns, helpTextColumns, defaultValue);
        }
    }
}

void CCommandLine::printVersion() const
{
    cout << m_programVersion << endl;
}
