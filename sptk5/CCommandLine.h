/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CCommandLine.h - description                           ║
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

#ifndef __CCOMMANDLINE_H__
#define __CCOMMANDLINE_H__

/***************************************************************************
                     SIMPLY POWERFUL TOOLKIT (SPTK)
                     CCommandLine.h  -  description
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

#include <map>
#include <set>
#include <list>
#include <sptk5/cutils>
#include <sptk5/CRegExp.h>

namespace sptk {

/// @addtogroup utility Utility Classes
/// @{

/// @brief Command line parser
class CCommandLine
{
public:
    /// @brief Option visibility for an argument or command.
    ///
    /// Argument(s) are defined with regular expression,
    /// that ether should match or should not match depending
    /// on constructor mustMatch parameter value.
    class Visibility
    {
        bool        m_inverted;     ///< If true then expression shoul not match
        CRegExp*    m_regexp;       ///< Regular expression for matching an argument(s)
        std::string m_pattern;      ///< Regular expression pattern

    public:

        /// @brief Constructor
        /// @param pattern std::string, Regular expression to match or do not match command line argument. If empty then it matches any argument.
        /// @param mustMatch bool, If true then regular expression should match, otherwiseit shouldn't match.
        Visibility(std::string pattern, bool mustMatch = true);

        /// @brief Copy constructor
        /// @param other const Visibility&, The object to copy from
        Visibility(const Visibility& other);

        /// @brief Destructor
        ~Visibility();

        /// @brief Returns true if there is no regular expression to match, and matches any argument.
        bool any() const;

        /// @brief Returns true if matches given argument or command.
        bool matches(std::string command) const;
    };

protected:
    /// @brief Command line element that could be argument/command, option, or option with value
    class CommandLineElement
    {
    protected:

        /// @brief Command line element type
        enum Type
        {
            IS_UNKNOWN,         ///< Type is unknown or undefined
            IS_COMMAND,         ///< Element is a command, not starting from '-' character
            IS_OPTION,          ///< Element is an option that doesn't expect value
            IS_VALUE_OPTION     ///< Element is an option that expects value
        };

        std::string m_name;             ///< Element name
        std::string m_shortName;        ///< Short element name (single character, options only)
        std::string m_help;             ///< Help (description) for the element

        Visibility m_useWithCommands;   ///< Element visibility for a command (options only)
    public:

        /// @brief Constructor
        /// @param name std::string name, Element name
        /// @param shortName std::string, Short element name (single character, options only)
        /// @param help std::string, Help (description) for the element
        /// @param useWithCommands const Visibility&, Element visibility for a command (options only)
        CommandLineElement(std::string name, std::string shortName, std::string help, const Visibility& useWithCommands);

        /// @brief Destructor
        virtual ~CommandLineElement();

        /// @brief Returns element type
        virtual Type type() const;

        /// @brief Returns element name
        virtual std::string name() const;

        /// @brief Returns true if element expects value
        virtual bool hasValue() const;

        /// @brief Validates given value (for elements that provide validation)
        ///
        /// Throws an exception if the value is invalid
        /// @param value std::string, Value to validate
        virtual void validate(std::string value) const;

        /// @brief Returns element name in help print format
        virtual std::string printableName() const;

        /// @brief Returns true if element may be used with command (options only)
        bool useWithCommand(std::string command) const;

        /// @brief Formats element help for printout
        /// @param textWidth size_t, Help text width
        /// @param formattedText CStrings&, Formatted help text
        void formatHelp(size_t textWidth, CStrings& formattedText) const;

        /// @brief Prints element help
        /// @param nameWidth size_t, Option name width
        /// @param textWidth size_t, Help text width
        /// @param optionDefaultValue std::string, Option default value (if any)
        void printHelp(size_t nameWidth, size_t textWidth, std::string optionDefaultValue) const;
    };

    /// @brief Command line argument
    ///
    /// Command line argument that doesn't start from '-' character and doesn't expect a value,
    /// AKA command.
    class CommandLineArgument: public CommandLineElement
    {
    public:
        /// @brief Constructor
        /// @param name std::string name, Element name
        /// @param help std::string, Help (description) for the element
        CommandLineArgument(std::string name, std::string help);

        /// @brief Destructor
        virtual ~CommandLineArgument();
    };

    /// @brief Command line option
    ///
    /// Command line argument that starts from '-' character and doesn't expect a value.
    /// It may have a long name that starts from '--', and/or a short name, that starts from '-'.
    class CommandLineOption: public CommandLineElement
    {
    public:
        /// @brief Constructor
        /// @param name std::string name, Element name
        /// @param shortName std::string, Short element name (single character, options only)
        /// @param useWithCommands const Visibility&, Element visibility for a command (options only)
        /// @param help std::string, Help (description) for the element
        CommandLineOption(std::string name, std::string shortName, const Visibility& useWithCommands, std::string help);

        /// @brief Destructor
        virtual ~CommandLineOption();

        /// @brief Returns true if element expects value
        virtual bool hasValue() const;

        /// @brief Returns element type
        virtual CommandLineElement::Type type() const;

        /// @brief Returns element name in help print format
        virtual std::string printableName() const;
    };

    /// @brief Command line parameter
    ///
    /// Command line argument that starts from '-' character and expects a value.
    /// It may have a long name that starts from '--', and/or a short name, that starts from '-'.
    /// Value has human readable name, such as 'file name', 'text', 'number', etc.. and optional
    /// validation pattern that can be regular expression or empty string.
    class CommandLineParameter: public CommandLineElement
    {
        std::string     m_valueInfo;        ///< Value name, for using in help
        CRegExp*        m_validateValue;    ///< Value validation regular expression

    public:

        /// @brief Constructor
        /// @param name std::string name, Element name
        /// @param shortName std::string, Short element name (single character, options only)
        /// @param valueName std::string, Value name
        /// @param validateValue std::string, Value validation regular expression
        /// @param useWithCommands const Visibility&, Element visibility for a command (options only)
        /// @param help std::string, Help (description) for the element
        CommandLineParameter(std::string name, std::string shortName, std::string valueName, std::string validateValue,
                const Visibility& useWithCommands, std::string help);

        /// @brief Destructor
        virtual ~CommandLineParameter();

        /// @brief Returns element name in help print format
        virtual std::string printableName() const;

        /// @brief Validates parameter value
        /// @param value std::string, Value to validate
        virtual void validate(std::string value) const;

        /// @brief Returns true if element expects value
        virtual bool hasValue() const;

        /// @brief Returns element type
        virtual Type type() const;
    };

    std::string                                 m_programVersion;       ///< Program version and copyright message (forhelp only).
    std::string                                 m_description;          ///< Program description (forhelp only).
    std::string                                 m_commandLinePrototype; ///< Command line prototype (forhelp only).
    std::map<std::string, CommandLineElement*>  m_optionTemplates;      ///< All the defined options.
    std::map<std::string, CommandLineArgument*> m_argumentTemplates;    ///< All the defined arguments.
    std::map<std::string, std::string>          m_values;               ///< Recevied option values.
    CStrings                                    m_arguments;            ///< Received arguments.
    std::list<CommandLineElement*>              m_allElements;          ///< All defined elements.

    /// @brief Returns true if string start matches the pattern
    /// @param subject std::string, String to check
    /// @param pattern std::string, String fragment to match
    bool startsWith(std::string subject, std::string pattern);

    /// @brief Returns true if string end matches the pattern
    /// @param subject std::string, String to check
    /// @param pattern std::string, String fragment to match
    bool endsWith(std::string subject, std::string pattern);

    /// @brief prints a line of characters
    /// @param ch std::string, Character to print
    /// @param count size_t, Number of characters to print
    static void printLine(std::string ch, size_t count);

public:
    /// @brief Constructor
    /// @param programVersion std::string, Program version and copyright message (forhelp only).
    /// @param description std::string, Program description (forhelp only).
    /// @param commandLinePrototype std::string, Command line prototype (forhelp only).
    CCommandLine(std::string programVersion, std::string description, std::string commandLinePrototype);

    /// @brief destructor
    virtual ~CCommandLine();

    /// @brief Defines command line option
    ///
    /// An option doesn't expect a value. If it is present in command line, it assumes value 'yes'.
    /// Otherwise it has value 'no'.
    /// @param fullName std::string name, Element name
    /// @param shortName std::string, Short element name (single character, options only)
    /// @param useForCommands const Visibility&, Element visibility for a command (options only)
    /// @param help std::string, Help (description) for the element
    void defineOption(std::string fullName, std::string shortName, Visibility useForCommands, std::string help);

    /// @brief Defines command line parameter
    /// @param fullName std::string name, Element name
    /// @param shortName std::string, Short element name (single character, options only)
    /// @param valueName std::string, Value name
    /// @param validateValue std::string, Value validation regular expression
    /// @param useForCommands const Visibility&, Element visibility for a command (options only)
    /// @param defaultValue std::string, Option default value
    /// @param help std::string, Help (description) for the element
    void defineParameter(std::string fullName, std::string shortName, std::string valueName, std::string validateValue,
            Visibility useForCommands, std::string defaultValue, std::string help);

    /// @brief Defines command line argument/command.
    /// @param fullName std::string, Argument/command name
    /// @param helpText std::string, Help (description) for the element
    void defineArgument(std::string fullName, std::string helpText);

    /// @brief Parses actual command line arguments.
    ///
    /// Should be executed after any define* methods of this class.
    /// @param argc int, Number of command line arguments (from main(argc,argv)).
    /// @param argv const char*[], Command line arguments (from main(argc,argv)).
    void init(int argc, const char* argv[]);

    /// @brief Returns actual option value
    /// @param name std::string, Option name
    std::string getOptionValue(std::string name) const;

    /// @brief Returns true if actual command line contains option
    /// @param name std::string, Option name
    bool hasOption(std::string name) const;

    /// @brief Re-defines actual option value
    /// @param name std::string, Option name
    /// @param value std::string, Option value
    void setOptionValue(std::string name, std::string value="yes");

    /// @brief Returns list of command line arguments
    const CStrings& arguments() const;

    /// @brief Prints full help
    /// @param screenColumns size_t, Screen width in columns
    void printHelp(size_t screenColumns) const;

    /// @brief Prints help for a given command/argument
    /// @param command std::string, Command to print help for
    /// @param screenColumns size_t, Screen width in columns
    void printHelp(std::string command, size_t screenColumns) const;

    /// @brief Prints program version
    void printVersion() const;
};

/// @}
}

#endif
