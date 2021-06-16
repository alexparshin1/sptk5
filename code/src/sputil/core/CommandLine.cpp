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

#include <sptk5/CommandLine.h>
#include <sptk5/Printer.h>

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
        m_regexp = make_shared<RegularExpression>(pattern);
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

CommandLine::CommandLineElement::CommandLineElement(const String& name, const String& shortName, const String& help,
                                                    const Visibility& useWithCommands)
: m_name(name), m_shortName(shortName), m_help(help), m_useWithCommands(useWithCommands)
{
    if (m_name.empty())
        throw Exception("Command line elements must have a name");
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
        return true;
    return m_useWithCommands.any() || m_useWithCommands.matches(command);
}

void CommandLine::CommandLineElement::formatHelp(size_t textWidth, Strings& formattedText) const
{
    Strings words(m_help, "\\s+", Strings::SplitMode::REGEXP);

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
    for (const string& helpRow : helpText) {
        if (firstRow) {
            COUT(left << setw((int)nameWidth) << printableName())
            firstRow = false;
        }
        else {
            COUT(left << setw((int)nameWidth) << "")
        }
        COUT("  " << helpRow << endl)
    }

    if (!optionDefaultValue.empty()) {
        String printDefaultValue = optionDefaultValue;
        if (!doesntNeedQuotes.matches(printDefaultValue))
            printDefaultValue = "'" + optionDefaultValue + "'";
        COUT(left << setw((int)nameWidth) << "" << "  The default value is " + printDefaultValue + "." << endl)
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
    return CommandLineElement::Type::IS_OPTION;
}

String CommandLine::CommandLineOption::printableName() const
{
    String result;

    result += "--" + name();

    if (!result.empty())
        result += ", ";

    if (!shortName().empty())
        result += "-" + shortName();

    return result;
}
//=============================================================================

CommandLine::CommandLineParameter::CommandLineParameter(const String& name, const String& shortName,
                                                        const String& valueInfo,
                                                        const String& validateValue, const Visibility& useWithCommands,
                                                        const String& help)
: CommandLineElement(name, shortName, help, useWithCommands), m_valueInfo(valueInfo)
{
    if (!validateValue.empty())
        m_validateValue = make_shared<RegularExpression>(validateValue);
    if (m_valueInfo.empty())
        throw Exception("Command line parameters must have a value info");
}

String CommandLine::CommandLineParameter::printableName() const
{
    string result;

    result += "--" + name();

    if (!result.empty())
        result += ", ";

    if (!shortName().empty())
        result += "-" + shortName();

    result += " <" + m_valueInfo + ">";

    return result;
}

void CommandLine::CommandLineParameter::validate(const String& value) const
{
    if (m_validateValue == nullptr)
        return;
    if (!m_validateValue->matches(value))
        throw Exception("Parameter " + name() + " has invalid value");
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

CommandLine::CommandLine(const String& programVersion, const String& description, const String& commandLinePrototype)
: m_programVersion(programVersion), m_description(description), m_commandLinePrototype(commandLinePrototype) { }

void CommandLine::defineOption(const String& fullName, const String& shortName, Visibility useForCommands,
                               const String& help)
{
    if (fullName.empty() && shortName.empty())
        return;

    auto optionTemplate = make_shared<CommandLineOption>(fullName, shortName, useForCommands, help);
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

    auto argumentTemplate = make_shared<CommandLineParameter>(fullName, shortName, valueName, validateValue, useForCommands, help);
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
        auto argumentTemplate = make_shared<CommandLineArgument>(fullName, helpText);
        m_allElements.push_back(argumentTemplate);
        m_argumentTemplates[fullName] = argumentTemplate;
    }
}

Strings CommandLine::preprocessArguments(int argc, const char* const* argv)
{
    Strings args;
    for (int i = 1; i < argc && argv[i] != nullptr; ++i)
        args.push_back(string(argv[i]));

    // Pre-process command line arguments
    Strings arguments;
    String quote;
    String quotedString;
    for (auto& arg : args) {
        String digestedArg = preprocessArgument(arg, quote, quotedString);
        if (!digestedArg.empty())
            arguments.push_back(digestedArg);
    }
    return arguments;
}

String CommandLine::preprocessArgument(String& arg, String& quote, String& quotedString)
{
    String output;
    if (quote.empty()) {
        if (arg.startsWith("'") || arg.startsWith("\"")) {
            quote = arg.substr(0, 1);
            quotedString = arg.substr(1);
            if (arg.length() > 1 && arg.endsWith(quote)) {
                quote.clear();
                quotedString.resize(quotedString.length() - 1);
                output = quotedString;
            }
        }
        else
            output = arg;
    }
    else {
        if (arg.endsWith(quote)) {
            arg = arg.substr(0, arg.length() - 1);
            quote = "";
            quotedString += " " + arg;
            output = quotedString;
        }
        else
            quotedString += " " + arg;
    }
    return output;
}

Strings CommandLine::rewriteArguments(const Strings& arguments)
{
    Strings digestedArgs;
    for (auto& arg : arguments) {
        if (arg.startsWith("--")) {
            // Full option name
            if (arg.startsWith("--gtest_"))
                continue; // Ignore googletest arguments
            digestedArgs.push_back(arg);
            continue;
        }

        if (arg.startsWith("-")) {
            // Short option name(s)
            for (unsigned j = 1; j < arg.length(); ++j) {
                string opt = "-" + arg.substr(j, j + 1);
                digestedArgs.push_back(opt);
            }
            continue;
        }

        digestedArgs.push_back(arg);
    }
    return digestedArgs;
}

void CommandLine::readOption(const Strings& digestedArgs, size_t& i)
{
    String arg = digestedArgs[i];
    String value;
    if (arg.startsWith("-")) {
        String optionName;
        if (arg.startsWith("--")) {
            // Full option name
            optionName = arg.substr(2);
        }
        else {
            // Short option name
            optionName = arg.substr(1);
        }
        auto element = m_optionTemplates[optionName];
        if (!element)
            throw Exception("Command line option or parameter " + arg + " is not supported");
        if (element->hasValue()) {
            ++i;
            if (i >= digestedArgs.size())
                throw Exception("Command line parameter " + arg + " should have value");
            value = digestedArgs[i];
            element->validate(value);
            m_values[element->name()] = value;
        }
        else
            m_values[element->name()] = "true";
    } else
        m_arguments.push_back(arg);
}

void CommandLine::init(int argc, const char* argv[])
{
    Strings arguments = preprocessArguments(argc, argv);
    Strings digestedArgs = rewriteArguments(arguments);

    for (size_t i = 0; i < digestedArgs.size(); ++i) {
        readOption(digestedArgs, i);
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
    auto element = m_optionTemplates[name];
    if (!element)
        throw Exception("Invalid option or parameter name: " + name);
    element->validate(value);
    m_values[name] = value;
}

const Strings& CommandLine::arguments() const
{
    return m_arguments;
}

void CommandLine::printLine(const String& ch, size_t count) const
{
    stringstream temp;
    for (size_t i = 0; i < count; ++i)
        temp << ch;
    COUT(temp.str() << endl)
}

void CommandLine::printHelp(size_t screenColumns) const
{
    printHelp("", screenColumns);
}

void CommandLine::printHelp(const String& onlyForCommand, size_t screenColumns) const
{
    if (!onlyForCommand.empty() && m_argumentTemplates.find(onlyForCommand) == m_argumentTemplates.end()) {
        CERR("Command '" << onlyForCommand << "' is not defined" << endl)
        return;
    }

    printVersion();
    printLine(doubleLine, screenColumns);
    COUT(m_description << endl)

    COUT(endl << "Syntax:" << endl)
    printLine(singleLine, screenColumns);

    String commandLinePrototype = m_commandLinePrototype;
    if (!onlyForCommand.empty())
        commandLinePrototype = commandLinePrototype.replace("<command>", onlyForCommand);
    COUT(commandLinePrototype << endl)

    // Find out space needed for command and option names
    size_t nameColumns = 10;
    Strings sortedCommands;

    for (auto& [argumentName, value]: m_argumentTemplates)
        sortedCommands.push_back(argumentName);

    for (const String& commandName : sortedCommands) {
        if (!onlyForCommand.empty() && commandName != onlyForCommand)
            continue;
        if (nameColumns < commandName.length())
            nameColumns = commandName.length();
    }

    Strings sortedOptions;
    for (auto& [optionName, value] : m_optionTemplates) {
        if (optionName.length() > 1)
            sortedOptions.push_back(optionName);
    }

    for (const String& optionName : sortedOptions) {
        auto itor = m_optionTemplates.find(optionName);
        if (itor == m_optionTemplates.end())
            continue;
        const auto optionTemplate = itor->second;
        if (!optionTemplate || !optionTemplate->useWithCommand(onlyForCommand))
            continue;
        size_t width = optionTemplate->printableName().length();
        if (nameColumns < width)
            nameColumns = width;
    }

    size_t helpTextColumns = screenColumns - (nameColumns + 2);
    if ((int)helpTextColumns < 10) {
        CERR("Can't print help information - the screen width is too small" << endl)
        return;
    }

    printCommands(onlyForCommand, screenColumns, nameColumns, sortedCommands, helpTextColumns);
    printOptions(onlyForCommand, screenColumns, nameColumns, sortedOptions, helpTextColumns);
}

void CommandLine::printOptions(const String& onlyForCommand, size_t screenColumns, size_t nameColumns,
                               const Strings& sortedOptions, size_t helpTextColumns) const
{
    if (!m_optionTemplates.empty()) {
        COUT(endl << "Options:" << endl)
        printLine(singleLine, screenColumns);
        for (const String& optionName : sortedOptions) {
            auto itor = m_optionTemplates.find(optionName);
            const auto optionTemplate = itor->second;
            if (!optionTemplate || !optionTemplate->useWithCommand(onlyForCommand))
                continue;
            String defaultValue;
            if (auto vtor = m_values.find(optionTemplate->name()); vtor != m_values.end())
                defaultValue = vtor->second;
            optionTemplate->printHelp(nameColumns, helpTextColumns, defaultValue);
        }
    }
}

void CommandLine::printCommands(const String& onlyForCommand, size_t screenColumns, size_t nameColumns,
                                const Strings& sortedCommands, size_t helpTextColumns) const
{
    if (onlyForCommand.empty() && !m_argumentTemplates.empty()) {
        COUT(endl << "Commands:" << endl)
        printLine(singleLine, screenColumns);
        for (const String& commandName : sortedCommands) {
            auto ator = m_argumentTemplates.find(commandName);
            if (!onlyForCommand.empty() && commandName != onlyForCommand) {
                continue;
            }
            const auto commandTemplate = ator->second;
            commandTemplate->printHelp(nameColumns, helpTextColumns, "");
        }
    }
}

void CommandLine::printVersion() const
{
    COUT(m_programVersion << endl)
}

#if USE_GTEST

class CommandLineTestData
{
public:
    static const char* testCommandLineArgs[14];

    static const char* testCommandLineArgs2[8];

    static const char* testCommandLineArgs3[9];
};

const char* CommandLineTestData::testCommandLineArgs[14] = {"testapp", "connect", "--host", "'ahostname'", "-p", "12345",
                                              "--verbose",
                                              "--description", "'This", "is", "a", "quoted", "argument'", nullptr};

const char* CommandLineTestData::testCommandLineArgs2[8] = {"testapp", "connect", "--host", "host name", "-p", "12345",
                                              "--verbose",
                                              nullptr};

const char* CommandLineTestData::testCommandLineArgs3[9] = {"testapp", "connect", "--host", "ahostname", "-p", "12345",
                                              "--verbotten", "--gtest_test",
                                              nullptr};

shared_ptr<CommandLine> createTestCommandLine()
{
    auto commandLine = make_shared<CommandLine>("test 1.00", "This is test command line description.", "testapp <action> [options]");
    commandLine->defineArgument("action", "Action to perform");
    commandLine->defineParameter("host", "h", "hostname", "^[\\S]+$", CommandLine::Visibility(""), "", "Hostname to connect");
    commandLine->defineParameter("port", "p", "port #", "^\\d{2,5}$", CommandLine::Visibility(""), "80", "Port to connect");
    commandLine->defineParameter("port2", "r", "port #", "^\\d{2,5}$", CommandLine::Visibility(""), "80", "Port to connect");
    commandLine->defineParameter("description", "d", "text", "", CommandLine::Visibility(""), "", "Operation description");
    commandLine->defineOption("verbose", "v", CommandLine::Visibility(""), "Verbose messages");
    return commandLine;
}

TEST(SPTK_CommandLine, Visibility)
{
    CommandLine::Visibility visibility1("");
    CommandLine::Visibility visibility2("^\\d+$");

    EXPECT_TRUE(visibility1.any());
    EXPECT_TRUE(visibility2.matches("123"));
}

TEST(SPTK_CommandLine, CommandLineElement)
{
    CommandLine::CommandLineElement testElement("test", "t", "short help", CommandLine::Visibility("^test|not-test$"));

    EXPECT_FALSE(testElement.hasValue());
    EXPECT_TRUE(testElement.useWithCommand("not-test"));
}

TEST(SPTK_CommandLine, ctor)
{
    auto commandLine = createTestCommandLine();
    commandLine->init(13, CommandLineTestData::testCommandLineArgs);

    EXPECT_EQ(size_t(1), commandLine->arguments().size());
    EXPECT_STREQ("ahostname", commandLine->getOptionValue("host").c_str());
    EXPECT_STREQ("12345", commandLine->getOptionValue("port").c_str());
    EXPECT_STREQ("80", commandLine->getOptionValue("port2").c_str());
    EXPECT_STREQ("true", commandLine->getOptionValue("verbose").c_str());
    EXPECT_STREQ("", commandLine->getOptionValue("bad").c_str());
    EXPECT_STREQ("This is a quoted argument", commandLine->getOptionValue("description").c_str());
    EXPECT_STREQ("connect", commandLine->arguments()[0].c_str());
    EXPECT_TRUE(commandLine->hasOption("verbose"));

    EXPECT_THROW(commandLine->setOptionValue("something", "xxx"), Exception);
}

TEST(SPTK_CommandLine, wrongArgumentValue)
{
    auto commandLine = createTestCommandLine();

    EXPECT_THROW(
        commandLine->init(7, CommandLineTestData::testCommandLineArgs2),
        Exception
    );
}

TEST(SPTK_CommandLine, wrongOption)
{
    auto commandLine = createTestCommandLine();

    EXPECT_THROW(
            commandLine->init(8, CommandLineTestData::testCommandLineArgs3),
            Exception
    );
}

TEST(SPTK_CommandLine, setOption)
{
    auto commandLine = createTestCommandLine();

    commandLine->init(7, CommandLineTestData::testCommandLineArgs);
    EXPECT_STREQ(commandLine->getOptionValue("host").c_str(), "ahostname");
    commandLine->setOptionValue("host", "www.x.com");
    EXPECT_STREQ(commandLine->getOptionValue("host").c_str(), "www.x.com");
}

TEST(SPTK_CommandLine, printHelp)
{
    auto commandLine = createTestCommandLine();

    stringstream output;
    commandLine->init(7, CommandLineTestData::testCommandLineArgs);
    commandLine->printHelp(80);
}

#endif
