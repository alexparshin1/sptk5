/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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

#include <ranges>
#include <utility>

using namespace std;
using namespace sptk;

#ifdef _WIN32
static const String doubleLine("=");
static const String singleLine("-");
#else
static const String doubleLine("═");
static const String singleLine("─");
#endif

CommandLine::Visibility::Visibility(const String& pattern, bool _mustMatch)
    : m_inverted(!_mustMatch)
{
    if (!pattern.empty())
    {
        m_regexp = make_shared<RegularExpression>(pattern);
    }
}

bool CommandLine::Visibility::any() const
{
    return m_regexp == nullptr;
}

bool CommandLine::Visibility::matches(const String& command) const
{
    return m_inverted == !m_regexp->matches(command);
}
//=============================================================================

CommandLine::CommandLineElement::CommandLineElement(String name, String shortName, String help,
                                                    Visibility useWithCommands)
    : m_name(std::move(name))
    , m_shortName(std::move(shortName))
    , m_help(std::move(help))
    , m_useWithCommands(std::move(useWithCommands))
{
    if (m_name.empty())
    {
        throw Exception("Command line elements must have a name");
    }
}

CommandLine::CommandLineElement::Type CommandLine::CommandLineElement::type() const
{
    return Type::IS_UNKNOWN;
}

String CommandLine::CommandLineElement::name() const
{
    return m_name;
}

String CommandLine::CommandLineElement::shortName() const
{
    return m_shortName;
}

bool CommandLine::CommandLineElement::hasValue() const
{
    return false;
}

void CommandLine::CommandLineElement::validate(const String&) const
{
    // Abstract element
}

String CommandLine::CommandLineElement::printableName() const
{
    return m_name;
}

bool CommandLine::CommandLineElement::useWithCommand(const String& command) const
{
    if (command.empty())
    {
        return true;
    }
    return m_useWithCommands.any() || m_useWithCommands.matches(command);
}

void CommandLine::CommandLineElement::formatHelp(size_t textWidth, Strings& formattedText) const
{
    const Strings words(m_help, "\\s+", Strings::SplitMode::REGEXP);

    formattedText.clear();

    String row;
    for (const String& word: words)
    {
        if (row.empty())
        {
            row = word;
            continue;
        }
        if ((row.length() + word.length() + 1) > textWidth)
        {
            formattedText.push_back(row);
            row = word;
            continue;
        }
        row += " " + word;
    }
    if (!row.empty())
    {
        formattedText.push_back(row);
    }
}

void CommandLine::CommandLineElement::printHelp(size_t nameWidth, size_t textWidth,
                                                const String& optionDefaultValue) const
{
    static const RegularExpression doesntNeedQuotes(R"([\d\.\-\+:,_]+)");

    Strings helpText;
    formatHelp(textWidth, helpText);
    bool firstRow = true;
    for (const string& helpRow: helpText)
    {
        stringstream str;
        if (firstRow)
        {
            str << left << setw(static_cast<int>(nameWidth)) << printableName();
            firstRow = false;
        }
        else
        {
            str << left << setw(static_cast<int>(nameWidth)) << "";
        }
        COUT(str.str() << "  " << helpRow);
    }

    if (!optionDefaultValue.empty())
    {
        String printDefaultValue = optionDefaultValue;
        if (!doesntNeedQuotes.matches(printDefaultValue))
        {
            printDefaultValue = "'" + optionDefaultValue + "'";
        }
        COUT(left << setw(static_cast<int>(nameWidth)) << ""
                  << "  The default value is " + printDefaultValue + ".");
    }
}
//=============================================================================

CommandLine::CommandLineArgument::CommandLineArgument(const String& name, const String& help)
    : CommandLineElement(name, "", help, Visibility(""))
{
}

//=============================================================================

CommandLine::CommandLineOption::CommandLineOption(const String& name, const String& shortName,
                                                  const Visibility& useWithCommands, const String& help)
    : CommandLineElement(name, shortName, help, useWithCommands)
{
}

bool CommandLine::CommandLineOption::hasValue() const
{
    return false;
}

CommandLine::CommandLineElement::Type CommandLine::CommandLineOption::type() const
{
    return CommandLineElement::Type::IS_OPTION;
}

String CommandLine::CommandLineOption::printableName() const
{
    String result;

    result += "--" + name();

    if (!result.empty())
    {
        result += ", ";
    }

    if (!shortName().empty())
    {
        result += "-" + shortName();
    }

    return result;
}
//=============================================================================

CommandLine::CommandLineParameter::CommandLineParameter(const String& name, const String& shortName,
                                                        String        valueInfo,
                                                        const String& validateValue, const Visibility& useWithCommands,
                                                        const String& help)
    : CommandLineElement(name, shortName, help, useWithCommands)
    , m_valueInfo(std::move(valueInfo))
{
    if (!validateValue.empty())
    {
        m_validateValue = make_shared<RegularExpression>(validateValue);
    }
    if (m_valueInfo.empty())
    {
        throw Exception("Command line parameters must have a value info");
    }
}

String CommandLine::CommandLineParameter::printableName() const
{
    string result;

    result += "--" + name();

    if (!result.empty())
    {
        result += ", ";
    }

    if (!shortName().empty())
    {
        result += "-" + shortName();
    }

    result += " <" + m_valueInfo + ">";

    return result;
}

void CommandLine::CommandLineParameter::validate(const String& value) const
{
    if (m_validateValue == nullptr)
    {
        return;
    }
    if (!m_validateValue->matches(value))
    {
        throw Exception("Parameter " + name() + " has invalid value");
    }
}

bool CommandLine::CommandLineParameter::hasValue() const
{
    return true;
}

CommandLine::CommandLineElement::Type CommandLine::CommandLineParameter::type() const
{
    return Type::IS_VALUE_OPTION;
}
//=============================================================================

CommandLine::CommandLine(String programVersion, String description, String commandLinePrototype)
    : m_programVersion(std::move(programVersion))
    , m_description(std::move(description))
    , m_commandLinePrototype(std::move(commandLinePrototype))
{
}

void CommandLine::defineOption(const String& fullName, const String& shortName, const Visibility& useForCommands,
                               const String& help)
{
    if (fullName.empty() && shortName.empty())
    {
        return;
    }

    const auto optionTemplate = make_shared<CommandLineOption>(fullName, shortName, useForCommands, help);
    m_allElements.push_back(optionTemplate);
    if (!fullName.empty())
    {
        m_optionTemplates[fullName] = optionTemplate;
    }
    if (!shortName.empty())
    {
        m_optionTemplates[shortName] = optionTemplate;
    }
}

void CommandLine::defineParameter(const String& fullName, const String& shortName, const String& valueName,
                                  const String& validateValue, const Visibility& useForCommands, const String& defaultValue,
                                  const String& help)
{
    if (fullName.empty() && shortName.empty())
    {
        return;
    }

    const auto argumentTemplate = make_shared<CommandLineParameter>(fullName, shortName, valueName, validateValue,
                                                                    useForCommands, help);
    m_allElements.push_back(argumentTemplate);

    String name;
    if (!shortName.empty())
    {
        m_optionTemplates[shortName] = argumentTemplate;
        name = shortName;
    }

    if (!fullName.empty())
    {
        m_optionTemplates[fullName] = argumentTemplate;
        name = fullName;
    }

    if (!defaultValue.empty())
    {
        argumentTemplate->validate(defaultValue);
        m_values[name] = defaultValue;
    }
}

void CommandLine::defineArgument(const String& fullName, const String& helpText)
{
    if (!fullName.empty())
    {
        const auto argumentTemplate = make_shared<CommandLineArgument>(fullName, helpText);
        m_allElements.push_back(argumentTemplate);
        m_argumentTemplates[fullName] = argumentTemplate;
    }
}

void CommandLine::addNote(const String& title, const String& text)
{
    m_notes.emplace_back(title, text);
}

Strings CommandLine::preprocessArguments(const vector<const char*>& argv)
{
    Strings args;
    for (const auto* arg: argv)
    {
        if (arg != nullptr)
        {
            args.push_back(arg);
        }
    }

    // Pre-process command line arguments
    Strings arguments;
    String  quote;
    String  quotedString;
    for (auto& arg: args)
    {
        if (const String digestedArg = preprocessArgument(arg, quote, quotedString);
            !digestedArg.empty())
        {
            arguments.push_back(digestedArg);
        }
    }
    return arguments;
}

String CommandLine::preprocessArgument(String& arg, String& quote, String& quotedString)
{
    String output;
    if (quote.empty())
    {
        if (arg.startsWith("'") || arg.startsWith("\""))
        {
            quote = arg.substr(0, 1);
            quotedString = arg.substr(1);
            if (arg.length() > 1 && arg.endsWith(quote))
            {
                quote.clear();
                quotedString.resize(quotedString.length() - 1);
                output = quotedString;
            }
        }
        else
        {
            output = arg;
        }
    }
    else
    {
        if (arg.endsWith(quote))
        {
            arg = arg.substr(0, arg.length() - 1);
            quote = "";
            quotedString += " " + arg;
            output = quotedString;
        }
        else
        {
            quotedString += " " + arg;
        }
    }
    return output;
}

Strings CommandLine::rewriteArguments(const Strings& arguments)
{
    Strings digestedArgs;
    for (const auto& arg: arguments)
    {
        if (arg.startsWith("--"))
        {
            // Full option name
            if (arg.startsWith("--gtest_"))
            {
                continue;
            } // Ignore googletest arguments
            digestedArgs.push_back(arg);
            continue;
        }

        if (arg.startsWith("-"))
        {
            // Short option name(s)
            for (unsigned j = 1; j < arg.length(); ++j)
            {
                const string opt = "-" + arg.substr(j, j + 1);
                digestedArgs.push_back(opt);
            }
            continue;
        }

        digestedArgs.push_back(arg);
    }
    return digestedArgs;
}

void CommandLine::readOption(const Strings& digestedArgs, size_t& argumentIndex)
{
    const String& arg = digestedArgs[argumentIndex];
    if (arg.startsWith("-"))
    {
        String optionName;
        if (arg.startsWith("--"))
        {
            // Full option name
            optionName = arg.substr(2);
        }
        else
        {
            // Short option name
            optionName = arg.substr(1);
        }
        const auto element = m_optionTemplates[optionName];
        if (!element)
        {
            throw Exception("Command line option or parameter " + arg + " is not supported");
        }
        if (element->hasValue())
        {
            ++argumentIndex;
            if (argumentIndex >= digestedArgs.size())
            {
                throw Exception("Command line parameter " + arg + " should have value");
            }
            const auto value = digestedArgs[argumentIndex];
            element->validate(value);
            m_values[element->name()] = value;
        }
        else
        {
            m_values[element->name()] = "true";
        }
    }
    else
    {
        m_arguments.push_back(arg);
    }
}

void CommandLine::init(size_t argc, const char** argv)
{
    const vector<const char*> args(argv + 1, argv + argc);
    m_executablePath = argv[0];

    const Strings arguments = preprocessArguments(args);
    const Strings digestedArgs = rewriteArguments(arguments);

    size_t argumentIndex = 0;
    while (argumentIndex < digestedArgs.size())
    {
        readOption(digestedArgs, argumentIndex);
        ++argumentIndex;
    }
}

String CommandLine::getOptionValue(const String& name) const
{
    const auto itor = m_values.find(name);
    if (itor == m_values.end())
    {
        return "";
    }
    return itor->second;
}

bool CommandLine::hasOption(const String& name) const
{
    return m_values.contains(name);
}

void CommandLine::setOptionValue(const String& name, const String& value)
{
    const auto element = m_optionTemplates[name];
    if (!element)
    {
        throw Exception("Invalid option or parameter name: " + name);
    }
    element->validate(value);
    m_values[name] = value;
}

const Strings& CommandLine::arguments() const
{
    return m_arguments;
}

void CommandLine::printLine(const String& fillChar, size_t count)
{
    stringstream temp;
    for (size_t i = 0; i < count; ++i)
    {
        temp << fillChar;
    }
    COUT(temp.str());
}

void CommandLine::printHelp(size_t screenColumns) const
{
    printHelp("", screenColumns);
}

void CommandLine::printHelp(const String& onlyForCommand, size_t screenColumns) const
{
    if (!onlyForCommand.empty() && !m_argumentTemplates.contains(onlyForCommand))
    {
        CERR("Command '" << onlyForCommand << "' is not defined");
        return;
    }

    printVersion();
    printLine(doubleLine, screenColumns);
    COUT(m_description);

    COUT(endl
         << "Syntax:" << endl);
    printLine(singleLine, screenColumns);

    String commandLinePrototype = m_commandLinePrototype;
    if (!onlyForCommand.empty())
    {
        commandLinePrototype = commandLinePrototype.replace("<command>", onlyForCommand);
    }
    COUT(commandLinePrototype);

    // Find out space needed for command and option names
    constexpr size_t minimalWidth {10};
    size_t           nameColumns = minimalWidth;
    Strings          sortedCommands;

    for (const auto& argumentName: views::keys(m_argumentTemplates))
    {
        sortedCommands.push_back(argumentName);
    }

    for (const String& commandName: sortedCommands)
    {
        if (!onlyForCommand.empty() && commandName != onlyForCommand)
        {
            continue;
        }
        nameColumns = max(nameColumns, commandName.length());
    }

    Strings sortedOptions;
    for (const auto& optionName: views::keys(m_optionTemplates))
    {
        if (optionName.length() > 1)
        {
            sortedOptions.push_back(optionName);
        }
    }

    for (const String& optionName: sortedOptions)
    {
        auto itor = m_optionTemplates.find(optionName);
        if (itor == m_optionTemplates.end())
        {
            continue;
        }

        const auto optionTemplate = itor->second;
        if (!optionTemplate || !optionTemplate->useWithCommand(onlyForCommand))
        {
            continue;
        }

        const size_t width = optionTemplate->printableName().length();
        nameColumns = nameColumns < width ? width : nameColumns;
    }

    const size_t helpTextColumns = screenColumns - (nameColumns + 2);
    if (helpTextColumns < minimalWidth)
    {
        CERR("Can't print help information - the screen width is too small");
        return;
    }

    printCommands(onlyForCommand, screenColumns, nameColumns, sortedCommands, helpTextColumns);
    printOptions(onlyForCommand, screenColumns, nameColumns, sortedOptions, helpTextColumns);
    printNotes(screenColumns);
}

void CommandLine::printOptions(const String& onlyForCommand, size_t screenColumns, size_t nameColumns,
                               const Strings& sortedOptions, size_t helpTextColumns) const
{
    if (!m_optionTemplates.empty())
    {
        COUT(endl
             << "Options:" << endl);
        printLine(singleLine, screenColumns);
        for (const String& optionName: sortedOptions)
        {
            const auto itor = m_optionTemplates.find(optionName);
            const auto optionTemplate = itor->second;
            if (!optionTemplate || !optionTemplate->useWithCommand(onlyForCommand))
            {
                continue;
            }
            String defaultValue;
            if (auto valueIterator = m_values.find(optionTemplate->name()); valueIterator != m_values.end())
            {
                defaultValue = valueIterator->second;
            }
            optionTemplate->printHelp(nameColumns, helpTextColumns, defaultValue);
        }
    }
}

void CommandLine::printCommands(const String& onlyForCommand, size_t screenColumns, size_t nameColumns,
                                const Strings& sortedCommands, size_t helpTextColumns) const
{
    if (onlyForCommand.empty() && !m_argumentTemplates.empty())
    {
        COUT(endl
             << "Commands:" << endl);
        printLine(singleLine, screenColumns);
        for (const String& commandName: sortedCommands)
        {
            const auto argumentIterator = m_argumentTemplates.find(commandName);
            if (!onlyForCommand.empty() && commandName != onlyForCommand)
            {
                continue;
            }
            const auto commandTemplate = argumentIterator->second;
            commandTemplate->printHelp(nameColumns, helpTextColumns, "");
        }
    }
}

void CommandLine::printVersion() const
{
    COUT(m_programVersion);
}

void CommandLine::printNotes(size_t screenColumns) const
{
    if (!m_notes.empty())
    {
        COUT(endl
             << "Notes:" << endl);
        printLine(singleLine, screenColumns);
        for (const auto& [title, text]: m_notes)
        {
            COUT(endl
                 << upperCase(title) << endl
                 << text << endl);
        }
    }
}
