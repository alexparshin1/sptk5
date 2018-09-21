/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CommandLine.cpp - description                          ║
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

#include <sptk5/CommandLine.h>

using namespace std;
using namespace sptk;

CommandLine::Visibility::Visibility(const String& pattern, bool _mustMatch)
: m_inverted(!_mustMatch), m_pattern(pattern)
{
    if (m_pattern.empty())
        m_regexp = nullptr;
    else
        m_regexp = new RegularExpression(m_pattern);
}

CommandLine::Visibility::Visibility(const Visibility& other)
: m_inverted(other.m_inverted), m_pattern(other.m_pattern)
{
    if (m_pattern.empty())
        m_regexp = nullptr;
    else
        m_regexp = new RegularExpression(m_pattern);
}

CommandLine::Visibility::~Visibility()
{
    delete m_regexp;
}

bool CommandLine::Visibility::any() const
{
    return m_regexp == nullptr;
}

bool CommandLine::Visibility::matches(const String& command) const
{
    Strings matches;
    return m_inverted == !m_regexp->m(command, matches);
}
//=============================================================================

CommandLine::CommandLineElement::CommandLineElement(const String& name, const String& shortName, const String& help,
                                                    const Visibility& useWithCommands)
: m_name(name), m_shortName(shortName), m_help(help), m_useWithCommands(useWithCommands)
{
    if (m_name.empty())
        throw Exception("Command line elements must have a name");
}

CommandLine::CommandLineElement::Type CommandLine::CommandLineElement::type() const
{
    return IS_UNKNOWN;
}

String CommandLine::CommandLineElement::name() const
{
    return m_name;
}

bool CommandLine::CommandLineElement::hasValue() const
{
    return false;
}

void CommandLine::CommandLineElement::validate(const String& value) const { }

String CommandLine::CommandLineElement::printableName() const
{
    return m_name;
}

bool CommandLine::CommandLineElement::useWithCommand(const String& command) const
{
    if (command.empty())
        return true;
    return m_useWithCommands.any() || m_useWithCommands.matches(command);
}

void CommandLine::CommandLineElement::formatHelp(size_t textWidth, Strings& formattedText) const
{
    Strings words(m_help, "\\s+", Strings::SM_REGEXP);

    formattedText.clear();

    String row;
    for (const String& word : words) {
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

void CommandLine::CommandLineElement::printHelp(size_t nameWidth, size_t textWidth, const String& optionDefaultValue) const
{
    static const RegularExpression doesntNeedQuotes("[\\d\\.\\-\\+:,_]+");

    Strings helpText;
    formatHelp(textWidth, helpText);
    bool firstRow = true;
    string printFormat = "%-" + int2string(nameWidth) + "s  %s";
    char rowBuffer[1024];
    for (const string& helpRow : helpText) {
        if (firstRow) {
            snprintf(rowBuffer, sizeof(rowBuffer), printFormat.c_str(), printableName().c_str(), helpRow.c_str());
            cout << rowBuffer << endl;
            firstRow = false;
        }
        else {
            snprintf(rowBuffer, sizeof(rowBuffer), printFormat.c_str(), "", helpRow.c_str());
            cout << rowBuffer << endl;
        }
    }

    if (!optionDefaultValue.empty()) {
        Strings matches;
        String printDefaultValue = optionDefaultValue;
        if (!doesntNeedQuotes.m(printDefaultValue, matches))
            printDefaultValue = "'" + optionDefaultValue + "'";
        string defaultValueStr = "The default value is " + printDefaultValue + ".";
        snprintf(rowBuffer, sizeof(rowBuffer), printFormat.c_str(), "", defaultValueStr.c_str());
        cout << rowBuffer << endl;
    }
}
//=============================================================================

CommandLine::CommandLineArgument::CommandLineArgument(const String& name, const String& help)
: CommandLineElement(name, "", help, Visibility("")) { }

//=============================================================================

CommandLine::CommandLineOption::CommandLineOption(const String& name, const String& shortName,
                                                  const Visibility& useWithCommands, const String& help)
: CommandLineElement(name, shortName, help, useWithCommands) { }

bool CommandLine::CommandLineOption::hasValue() const
{
    return false;
}

CommandLine::CommandLineElement::Type CommandLine::CommandLineOption::type() const
{
    return CommandLineElement::IS_OPTION;
}

String CommandLine::CommandLineOption::printableName() const
{
    String result;

    result += "--" + m_name;

    if (!result.empty())
        result += ", ";

    if (!m_shortName.empty())
        result += "-" + m_shortName;

    return result;
}
//=============================================================================

CommandLine::CommandLineParameter::CommandLineParameter(const String& name, const String& shortName,
                                                        const String& valueInfo,
                                                        const String& validateValue, const Visibility& useWithCommands,
                                                        const String& help)
: CommandLineElement(name, shortName, help, useWithCommands), m_valueInfo(valueInfo)
{
    if (validateValue.empty())
        m_validateValue = nullptr;
    else
        m_validateValue = new RegularExpression(validateValue);
    if (m_valueInfo.empty())
        throw Exception("Command line parameters must have a value info");
}

CommandLine::CommandLineParameter::~CommandLineParameter()
{
    delete m_validateValue;
}

String CommandLine::CommandLineParameter::printableName() const
{
    string result;

    result += "--" + m_name;

    if (!result.empty())
        result += ", ";

    if (!m_shortName.empty())
        result += "-" + m_shortName;

    result += " <" + m_valueInfo + ">";

    return result;
}

void CommandLine::CommandLineParameter::validate(const String& value) const
{
    if (m_validateValue == nullptr)
        return;
    Strings matches;
    if (!m_validateValue->m(value, matches))
        throw Exception("Parameter " + m_name + " has invalid value");
}

bool CommandLine::CommandLineParameter::hasValue() const
{
    return true;
}

CommandLine::CommandLineElement::Type CommandLine::CommandLineParameter::type() const
{
    return IS_VALUE_OPTION;
}
//=============================================================================

bool CommandLine::startsWith(const String& str, const String& pattern)
{
    return str.startsWith(pattern);
}

bool CommandLine::endsWith(const String& str, const String& pattern)
{
    return str.endsWith(pattern);
}

CommandLine::CommandLine(const String& programVersion, const String& description, const String& commandLinePrototype)
: m_programVersion(programVersion), m_description(description), m_commandLinePrototype(commandLinePrototype) { }

CommandLine::~CommandLine()
{
    for (CommandLineElement* element : m_allElements)
        delete element;
}

void CommandLine::defineOption(const String& fullName, const String& shortName, Visibility useForCommands,
                               const String& help)
{
    if (fullName.empty() && shortName.empty())
        return;

    auto optionTemplate = new CommandLineOption(fullName, shortName, useForCommands, help);
    m_allElements.push_back(optionTemplate);
    if (!fullName.empty())
        m_optionTemplates[fullName] = optionTemplate;
    if (!shortName.empty())
        m_optionTemplates[shortName] = optionTemplate;
}

void CommandLine::defineParameter(const String& fullName, const String& shortName, const String& valueName,
                                  const String& validateValue, Visibility useForCommands, const String& defaultValue,
                                  const String& help)
{
    if (fullName.empty() && shortName.empty())
        return;

    auto argumentTemplate = new CommandLineParameter(fullName, shortName, valueName, validateValue,
        useForCommands, help);
    m_allElements.push_back(argumentTemplate);

    String name;
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

void CommandLine::defineArgument(const String& fullName, const String& helpText)
{
    if (!fullName.empty()) {
        auto argumentTemplate = new CommandLineArgument(fullName, helpText);
        m_allElements.push_back(argumentTemplate);
        m_argumentTemplates[fullName] = argumentTemplate;
    }
}

void CommandLine::init(int argc, const char* argv[])
{
    Strings args;
    for (int i = 1; i < argc; i++)
        args.push_back(string(argv[i]));

    // Pre-process command line arguments
    Strings arguments;
    string quote, quotedString;
    for (auto arg : args) {
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
    Strings digestedArgs;
    for (const String& arg : arguments) {
        if (startsWith(arg, "--")) {
            // Full option name
            if (arg.startsWith("--gtest_"))
                continue; // Ignore googletest arguments
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
        String arg = digestedArgs[i];
        String value;

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
            if (element == nullptr)
                throw Exception("Command line option or parameter " + arg + " is not supported");
            if (element->hasValue()) {
                i++;
                if (i >= digestedArgs.size())
                    throw Exception("Command line parameter " + arg + " should have value");
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

String CommandLine::getOptionValue(const String& name) const
{
    auto itor = m_values.find(name);
    if (itor == m_values.end())
        return "";
    return itor->second;
}

bool CommandLine::hasOption(const String& name) const
{
    return m_values.find(name) != m_values.end();
}

void CommandLine::setOptionValue(const String& name, const String& value)
{
    CommandLineElement* element = m_optionTemplates[name];
    if (element == nullptr)
        throw Exception("Invalid option or parameter name: " + name);
    element->validate(value);
    m_values[name] = value;
}

const Strings& CommandLine::arguments() const
{
    return m_arguments;
}

void CommandLine::printLine(const String& ch, size_t count)
{
    for (size_t i = 0; i < count; i++)
        cout << ch;
    cout << endl;
}

void CommandLine::printHelp(size_t screenColumns) const
{
    printHelp("", screenColumns);
}

void CommandLine::printHelp(const String& onlyForCommand, size_t screenColumns) const
{
    if (!onlyForCommand.empty() && m_argumentTemplates.find(onlyForCommand) == m_argumentTemplates.end()) {
        cerr << "Command '" << onlyForCommand << "' is not defined" << endl;
        return;
    }

    cout << m_programVersion << endl;
    printLine("═", screenColumns);
    cout << m_description << endl;

    cout << "\nSyntax:" << endl;
    printLine("─", screenColumns);

    String commandLinePrototype = m_commandLinePrototype;
    if (!onlyForCommand.empty())
        commandLinePrototype = commandLinePrototype.replace("<command>", onlyForCommand);
    cout << commandLinePrototype << endl;

    // Find out space needed for command and option names
    size_t nameColumns = 10;
    Strings sortedCommands;
    for (auto& itor : m_argumentTemplates)
        sortedCommands.push_back(itor.first);

    for (const String& commandName : sortedCommands) {
        if (!onlyForCommand.empty() && commandName != onlyForCommand)
            continue;
        if (nameColumns < commandName.length())
            nameColumns = commandName.length();
    }

    Strings sortedOptions;
    for (auto& itor : m_optionTemplates) {
        String optionName = itor.first;
        if (optionName.length() > 1)
            sortedOptions.push_back(optionName);
    }

    for (const String& optionName : sortedOptions) {
        auto itor = m_optionTemplates.find(optionName);
        if (itor == m_optionTemplates.end())
            continue;
        const CommandLineElement* optionTemplate = itor->second;
        if (optionTemplate == nullptr)
            continue;
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
        for (const String& commandName : sortedCommands) {
            auto ator = m_argumentTemplates.find(commandName);
            const CommandLineArgument* commandTemplate = ator->second;
            if (!onlyForCommand.empty() && commandName != onlyForCommand)
                continue;
            commandTemplate->printHelp(nameColumns, helpTextColumns, "");
        }
    }

    if (!m_optionTemplates.empty()) {
        cout << "\nOptions:" << endl;
        printLine("─", screenColumns);
        for (const String& optionName : sortedOptions) {
            auto itor = m_optionTemplates.find(optionName);
            const CommandLineElement* optionTemplate = itor->second;
            if (optionTemplate == nullptr)
                continue;
            if (!optionTemplate->useWithCommand(onlyForCommand))
                continue;
            String defaultValue;
            auto vtor = m_values.find(optionTemplate->name());
            if (vtor != m_values.end())
                defaultValue = vtor->second;
            optionTemplate->printHelp(nameColumns, helpTextColumns, defaultValue);
        }
    }
}

void CommandLine::printVersion() const
{
    cout << m_programVersion << endl;
}

#if USE_GTEST
#include <gtest/gtest.h>

static const char* testCommandLineArgs[] = { "testapp", "connect", "--host", "ahostname", "-p", "12345", "--verbose",
                                             nullptr };

static const char* testCommandLineArgs2[] = { "testapp", "connect", "--host", "host name", "-p", "12345", "--verbose",
                                             nullptr };

static const char* testCommandLineArgs3[] = { "testapp", "connect", "--host", "ahostname", "-p", "12345", "--verbotten",
                                              nullptr };

CommandLine* createTestCommandLine()
{
    CommandLine* commandLine = new CommandLine("test 1.00", "", "testapp <action> [options]");
    commandLine->defineArgument("action", "Action to perform");
    commandLine->defineParameter("host", "h", "hostname", "^[\\S]+$", CommandLine::Visibility(""), "", "Hostname to connect");
    commandLine->defineParameter("port", "p", "port #", "^\\d{2,5}$", CommandLine::Visibility(""), "", "Port to connect");
    commandLine->defineOption("verbose", "v", CommandLine::Visibility(""), "Verbose messages");
    return commandLine;
}

TEST(SPTK_CommandLine, ctor)
{
    unique_ptr<CommandLine> commandLine(createTestCommandLine());
    commandLine->init(7, testCommandLineArgs);

    EXPECT_STREQ("ahostname", commandLine->getOptionValue("host").c_str());
    EXPECT_STREQ("12345", commandLine->getOptionValue("port").c_str());
    EXPECT_STREQ("true", commandLine->getOptionValue("verbose").c_str());
}

TEST(SPTK_CommandLine, wrongArgumentValue)
{
    unique_ptr<CommandLine> commandLine(createTestCommandLine());

    EXPECT_THROW(
        commandLine->init(7, testCommandLineArgs2),
        Exception
    );
}

TEST(SPTK_CommandLine, wrongOption)
{
    unique_ptr<CommandLine> commandLine(createTestCommandLine());

    EXPECT_THROW(
            commandLine->init(7, testCommandLineArgs3),
            Exception
    );
}

#endif