/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CommandLine.h - description                            ║
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

#ifndef __SPTK_COMMANDLINE_H__
#define __SPTK_COMMANDLINE_H__

#include <map>
#include <set>
#include <list>
#include <sptk5/cutils>
#include <sptk5/RegularExpression.h>

namespace sptk {

/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * @brief Command line parser
 */
class CommandLine
{
public:
    /**
     * @brief Option visibility for an argument or command.
     *
     * Argument(s) are defined with regular expression,
     * that ether should match or should not match depending
     * on constructor mustMatch parameter value.
     */
    class Visibility
    {
        /**
         * If true then expression shoul not match
         */
        bool                m_inverted;

        /**
         * Regular expression for matching an argument(s)
         */
        RegularExpression*  m_regexp;

        /**
         * Regular expression pattern
         */
        String              m_pattern;


    public:

        /**
         * @brief Constructor
         * @param pattern       Regular expression to match or do not match command line argument. If empty then it matches any argument.
         * @param mustMatch     If true then regular expression should match, otherwiseit shouldn't match.
         */
        Visibility(const String& pattern, bool mustMatch = true);

        /**
         * @brief Copy constructor
         * @param other const Visibility&, The object to copy from
         */
        Visibility(const Visibility& other);

        /**
         * @brief Destructor
         */
        ~Visibility();

        /**
         * @brief Returns true if there is no regular expression to match, and matches any argument.
         */
        bool any() const;

        /**
         * @brief Returns true if matches given argument or command.
         */
        bool matches(const String& command) const;
    };

protected:
    /**
     * @brief Command line element that could be argument/command, option, or option with value
     */
    class CommandLineElement
    {
    protected:

        /**
         * @brief Command line element type
         */
        enum Type
        {
            /**
             * Type is unknown or undefined
             */
            IS_UNKNOWN,

            /**
             * Element is a command, not starting from '-' character
             */
            IS_COMMAND,

            /**
             * Element is an option that doesn't expect value
             */
            IS_OPTION,

            /**
             * Element is an option that expects value
             */
            IS_VALUE_OPTION

        };

        /**
         * Element name
         */
        String      m_name;

        /**
         * Short element name (single character, options only)
         */
        String      m_shortName;

        /**
         * Help (description) for the element
         */
        String m_help;


        /**
         * Element visibility for a command (options only)
         */
        Visibility m_useWithCommands;

    public:

        /**
         * @brief Constructor
         * @param name              Element name
         * @param shortName         Short element name (single character, options only)
         * @param help              Help (description) for the element
         * @param useWithCommands   Element visibility for a command (options only)
         */
        CommandLineElement(const String& name, const String& shortName, const String& help,
                           const Visibility& useWithCommands);

        /**
         * @brief Destructor
         */
        virtual ~CommandLineElement() = default;

        /**
         * @brief Returns element type
         */
        virtual Type type() const;

        /**
         * @brief Returns element name
         */
        virtual String name() const;

        /**
         * @brief Returns true if element expects value
         */
        virtual bool hasValue() const;

        /**
         * @brief Validates given value (for elements that provide validation)
         *
         * Throws an exception if the value is invalid
         * @param value             Value to validate
         */
        virtual void validate(const String& value) const;

        /**
         * @brief Returns element name in help print format
         */
        virtual String printableName() const;

        /**
         * @brief Returns true if element may be used with command (options only)
         */
        bool useWithCommand(const String& command) const;

        /**
         * @brief Formats element help for printout
         * @param textWidth         Help text width
         * @param formattedText     Formatted help text
         */
        void formatHelp(size_t textWidth, Strings& formattedText) const;

        /**
         * @brief Prints element help
         * @param nameWidth         Option name width
         * @param textWidth         Help text width
         * @param optionDefaultValue Option default value (if any)
         */
        void printHelp(size_t nameWidth, size_t textWidth, const String& optionDefaultValue) const;
    };

    /**
     * @brief Command line argument
     *
     * Command line argument that doesn't start from '-' character and doesn't expect a value,
     * AKA command.
     */
    class CommandLineArgument: public CommandLineElement
    {
    public:
        /**
         * @brief Constructor
         * @param name          Element name
         * @param help          Help (description) for the element
         */
        CommandLineArgument(const String& name, const String& help);

        /**
         * @brief Destructor
         */
        virtual ~CommandLineArgument() = default;
    };

    /**
     * @brief Command line option
     *
     * Command line argument that starts from '-' character and doesn't expect a value.
     * It may have a long name that starts from '--', and/or a short name, that starts from '-'.
     */
    class CommandLineOption: public CommandLineElement
    {
    public:
        /**
         * @brief Constructor
         * @param name              Element name
         * @param shortName         Short element name (single character, options only)
         * @param useWithCommands   Element visibility for a command (options only)
         * @param help              Help (description) for the element
         */
        CommandLineOption(const String& name, const String& shortName, const Visibility& useWithCommands,
                          const String& help);

        /**
         * @brief Destructor
         */
        virtual ~CommandLineOption() = default;

        /**
         * @brief Returns true if element expects value
         */
        virtual bool hasValue() const;

        /**
         * @brief Returns element type
         */
        virtual CommandLineElement::Type type() const;

        /**
         * @brief Returns element name in help print format
         */
        virtual String printableName() const;
    };

    /**
     * @brief Command line parameter
     *
     * Command line argument that starts from '-' character and expects a value.
     * It may have a long name that starts from '--', and/or a short name, that starts from '-'.
     * Value has human readable name, such as 'file name', 'text', 'number', etc.. and optional
     * validation pattern that can be regular expression or empty string.
     */
    class CommandLineParameter: public CommandLineElement
    {
        /**
         * Value name, for using in help
         */
        String                  m_valueInfo;

        /**
         * Value validation regular expression
         */
        RegularExpression*      m_validateValue;


    public:

        /**
         * @brief Constructor
         * @param name          Element name
         * @param shortName     Short element name (single character, options only)
         * @param valueName     Value name
         * @param validateValue Value validation regular expression
         * @param useWithCommands Element visibility for a command (options only)
         * @param help          Help (description) for the element
         */
        CommandLineParameter(const String& name, const String& shortName, const String& valueName,
                             const String& validateValue,
                             const Visibility& useWithCommands, const String& help);

        /**
         * @brief Destructor
         */
        virtual ~CommandLineParameter();

        /**
         * @brief Returns element name in help print format
         */
        virtual String printableName() const;

        /**
         * @brief Validates parameter value
         * @param value         Value to validate
         */
        virtual void validate(const String& value) const;

        /**
         * @brief Returns true if element expects value
         */
        virtual bool hasValue() const;

        /**
         * @brief Returns element type
         */
        virtual Type type() const;
    };

    /**
     * Program version and copyright message (forhelp only).
     */
    String                                      m_programVersion;

    /**
     * Program description (forhelp only).
     */
    String                                      m_description;

    /**
     * Command line prototype (forhelp only).
     */
    String                                      m_commandLinePrototype;

    /**
     * All the defined options.
     */
    std::map<String, CommandLineElement*>       m_optionTemplates;

    /**
     * All the defined arguments.
     */
    std::map<String, CommandLineArgument*>      m_argumentTemplates;

    /**
     * Received option values.
     */
    std::map<String, String>                    m_values;

    /**
     * Received arguments.
     */
    Strings                                     m_arguments;

    /**
     * All defined elements.
     */
    std::list<CommandLineElement*>              m_allElements;


    /**
     * @brief Returns true if string start matches the pattern
     * @param subject           String to check
     * @param pattern           String fragment to match
     */
    bool startsWith(const String& subject, const String& pattern);

    /**
     * @brief Returns true if string end matches the pattern
     * @param subject           String to check
     * @param pattern           String fragment to match
     */
    bool endsWith(const String& subject, const String& pattern);

    /**
     * @brief prints a line of characters
     * @param ch                Character to print
     * @param count             Number of characters to print
     */
    static void printLine(const String& ch, size_t count);

public:
    /**
     * @brief Constructor
     * @param programVersion        Program version and copyright message (forhelp only).
     * @param description           Program description (forhelp only).
     * @param commandLinePrototype  Command line prototype (forhelp only).
     */
    CommandLine(const String& programVersion, const String& description, const String& commandLinePrototype);

    /**
     * @brief destructor
     */
    virtual ~CommandLine();

    /**
     * @brief Defines command line option
     *
     * An option doesn't expect a value. If it is present in command line, it assumes value 'yes'.
     * Otherwise it has value 'no'.
     * @param fullName          Element name
     * @param shortName         Short element name (single character, options only)
     * @param useForCommands    Element visibility for a command (options only)
     * @param help              Help (description) for the element
     */
    void defineOption(const String& fullName, const String& shortName, Visibility useForCommands, const String& help);

    /**
     * @brief Defines command line parameter
     * @param fullName          Element name
     * @param shortName         Short element name (single character, options only)
     * @param valueName         Value name
     * @param validateValue     Value validation regular expression
     * @param useForCommands    Element visibility for a command (options only)
     * @param defaultValue      Option default value
     * @param help              Help (description) for the element
     */
    void defineParameter(const String& fullName, const String& shortName, const String& valueName,
                         const String& validateValue,
                         Visibility useForCommands, const String& defaultValue, const String& help);

    /**
     * @brief Defines command line argument/command.
     * @param fullName          Argument/command name
     * @param helpText          Help (description) for the element
     */
    void defineArgument(const String& fullName, const String& helpText);

    /**
     * @brief Parses actual command line arguments.
     *
     * Should be executed after any define* methods of this class.
     * @param argc              Number of command line arguments (from main(argc,argv)).
     * @param argv              Command line arguments (from main(argc,argv)).
     */
    void init(int argc, const char* argv[]);

    /**
     * @brief Returns actual option value
     * @param name              Option name
     */
    String getOptionValue(const String& name) const;

    /**
     * @brief Returns true if actual command line contains option
     * @param name              Option name
     */
    bool hasOption(const String& name) const;

    /**
     * @brief Re-defines actual option value
     * @param name              Option name
     * @param value             Option value
     */
    void setOptionValue(const String& name, const String& value = "yes");

    /**
     * @brief Returns list of command line arguments
     */
    const Strings& arguments() const;

    /**
     * @brief Prints full help
     * @param screenColumns     Screen width in columns
     */
    void printHelp(size_t screenColumns) const;

    /**
     * @brief Prints help for a given command/argument
     * @param command           Command to print help for
     * @param screenColumns     Screen width in columns
     */
    void printHelp(const String& command, size_t screenColumns) const;

    /**
     * @brief Prints program version
     */
    void printVersion() const;
};

/**
 * @}
 */
}

#endif
