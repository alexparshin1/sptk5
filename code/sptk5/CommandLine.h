/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

#pragma once

#include <iostream>
#include <list>
#include <map>
#include <set>
#include <sptk5/RegularExpression.h>
#include <sptk5/cutils>

namespace sptk {

/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * Command line parser
 */
class SP_EXPORT CommandLine
{
public:
    /**
     * Option visibility for an argument or command.
     *
     * Argument(s) are defined with regular expression,
     * that ether should match or should not match depending
     * on constructor mustMatch parameter value.
     */
    class SP_EXPORT Visibility
    {
    public:
        /**
         * Constructor
         * @param pattern       Regular expression to match or do not match command line argument. If empty then it matches any argument.
         * @param mustMatch     If true then regular expression should match, otherwise it shouldn't match.
         */
        Visibility(const String& pattern, bool mustMatch = true);

        /**
         * Returns true if there is no regular expression to match, and matches any argument.
         */
        [[nodiscard]] bool any() const;

        /**
         * Returns true if matches given argument or command.
         */
        [[nodiscard]] bool matches(const String& command) const;

    private:
        bool m_inverted;                             ///< If true then expression should not match
        std::shared_ptr<RegularExpression> m_regexp; ///< Regular expression for matching an argument(s)
    };

    /**
     * prints a line of characters
     * @param ch                Character to print
     * @param count             Number of characters to print
     */
    static void printLine(const String& ch, size_t count);

    /**
     * Print help on commands
     * @param onlyForCommand    Optional: if not empty then print only this command help
     * @param screenColumns     Screen width in columns
     * @param nameColumns       Number of columns for command names
     * @param sortedCommands    Sorted command list
     * @param helpTextColumns   Number of columns for help text
     */
    void printCommands(const String& onlyForCommand, size_t screenColumns, size_t nameColumns,
                       const Strings& sortedCommands, size_t helpTextColumns) const;

    /**
     * Print help on options
     * @param onlyForCommand    Optional: if not empty then print only this command help
     * @param screenColumns     Screen width in columns
     * @param nameColumns       Number of columns for option names
     * @param sortedOptions     Sorted option list
     * @param helpTextColumns   Number of columns for help text
     */
    void printOptions(const String& onlyForCommand, size_t screenColumns, size_t nameColumns,
                      const Strings& sortedOptions, size_t helpTextColumns) const;

    /**
     * @brief Print notes
     */
    void printNotes(size_t screenColumns) const;

    /**
     * Constructor
     * @param programVersion        Program version and copyright message (for help only).
     * @param description           Program description (for help only).
     * @param commandLinePrototype  Command line prototype (for help only).
     */
    CommandLine(String programVersion, String description, String commandLinePrototype);

    /**
     * Defines command line option
     *
     * An option doesn't expect a value. If it is present in command line, it assumes value 'yes'.
     * Otherwise it has value 'no'.
     * @param fullName          Element name
     * @param shortName         Short element name (single character, options only)
     * @param useForCommands    Element visibility for a command (options only)
     * @param help              Help (description) for the element
     */
    void defineOption(const String& fullName, const String& shortName, const Visibility& useForCommands, const String& help);

    /**
     * Defines command line parameter
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
                         const Visibility& useForCommands, const String& defaultValue, const String& help);

    /**
     * Defines command line argument/command.
     * @param fullName          Argument/command name
     * @param helpText          Help (description) for the element
     */
    void defineArgument(const String& fullName, const String& helpText);

    /**
     * @brief Add a note.
     * @param title             Argument/command name
     * @param text              Help (description) for the element
     */
    void addNote(const String& title, const String& text);

    /**
     * Parses actual command line arguments.
     *
     * Should be executed after any define* methods of this class.
     * @param argc              Number of command line arguments (from main(argc,argv)).
     * @param argv              Command line arguments (from main(argc,argv)).
     */
    void init(size_t argc, const char** argv);

    /**
     * Returns actual option value
     * @param name              Option name
     */
    String getOptionValue(const String& name) const;

    /**
     * Returns true if actual command line contains option
     * @param name              Option name
     */
    bool hasOption(const String& name) const;

    /**
     * Re-defines actual option value
     * @param name              Option name
     * @param value             Option value
     */
    void setOptionValue(const String& name, const String& value = "yes");

    /**
     * Returns list of command line arguments
     */
    const Strings& arguments() const;

    /**
     * Prints full help
     * @param screenColumns     Screen width in columns
     */
    void printHelp(size_t screenColumns) const;

    /**
     * Prints help for a given command/argument
     * @param onlyForCommand           Command to print help for
     * @param screenColumns     Screen width in columns
     */
    void printHelp(const String& onlyForCommand, size_t screenColumns) const;

    /**
     * Prints program version
     */
    void printVersion() const;

    /**
     * Preprocess command line arguments
     * @param argc              Number of command line arguments
     * @param argv              Command line arguments
     * @return preprocessed command line arguments
     */
    static Strings preprocessArguments(const std::vector<const char*>& argv);

    /**
     * Re-write command line arguments
     * @param arguments         Command line arguments
     * @return re-written command line arguments
     */
    static Strings rewriteArguments(const Strings& arguments);

    /**
     * Command line element that could be argument/command, option, or option with value
     */
    class SP_EXPORT CommandLineElement
    {
    public:
        /**
         * Command line element type
         */
        enum class Type
        {
            IS_UNKNOWN,     ///< Type is unknown or undefined
            IS_OPTION,      ///< Element is an option that doesn't expect value
            IS_VALUE_OPTION ///< Element is an option that expects value
        };

        /**
         * Constructor
         * @param name              Element name
         * @param shortName         Short element name (single character, options only)
         * @param help              Help (description) for the element
         * @param useWithCommands   Element visibility for a command (options only)
         */
        CommandLineElement(String name, String shortName, String help,
                           Visibility useWithCommands);

        /**
         * Destructor
         */
        virtual ~CommandLineElement() = default;

        /**
         * Returns element type
         */
        virtual Type type() const;

        /**
         * Returns element name
         */
        virtual String name() const;

        /**
         * Returns element short name
         */
        virtual String shortName() const;

        /**
         * Returns true if element expects value
         */
        virtual bool hasValue() const;

        /**
         * Validates given value (for elements that provide validation)
         *
         * Throws an exception if the value is invalid
         * @param value             Value to validate
         */
        virtual void validate(const String& value) const;

        /**
         * Returns element name in help print format
         */
        virtual String printableName() const;

        /**
         * Returns true if element may be used with command (options only)
         */
        bool useWithCommand(const String& command) const;

        /**
         * Formats element help for printout
         * @param textWidth         Help text width
         * @param formattedText     Formatted help text
         */
        void formatHelp(size_t textWidth, Strings& formattedText) const;

        /**
         * Prints element help
         * @param nameWidth         Optional name width
         * @param textWidth         Help text width
         * @param optionDefaultValue Option default value (if any)
         * @param output            Optional output stream
         */
        void printHelp(size_t nameWidth, size_t textWidth, const String& optionDefaultValue) const;

    private:
        /**
         * Element name
         */
        String m_name;

        /**
         * Short element name (single character, options only)
         */
        String m_shortName;

        /**
         * Help (description) for the element
         */
        String m_help;

        /**
         * Element visibility for a command (options only)
         */
        Visibility m_useWithCommands;
    };

    /**
     * Command line argument
     *
     * Command line argument that doesn't start from '-' character and doesn't expect a value,
     * AKA command.
     */
    class SP_EXPORT CommandLineArgument
        : public CommandLineElement
    {
    public:
        /**
         * Constructor
         * @param name          Element name
         * @param help          Help (description) for the element
         */
        CommandLineArgument(const String& name, const String& help);

        /**
         * Destructor
         */
        ~CommandLineArgument() override = default;
    };

    /**
     * Command line option
     *
     * Command line argument that starts from '-' character and doesn't expect a value.
     * It may have a long name that starts from '--', and/or a short name, that starts from '-'.
     */
    class SP_EXPORT CommandLineOption
        : public CommandLineElement
    {
    public:
        /**
         * Constructor
         * @param name              Element name
         * @param shortName         Short element name (single character, options only)
         * @param useWithCommands   Element visibility for a command (options only)
         * @param help              Help (description) for the element
         */
        CommandLineOption(const String& name, const String& shortName, const Visibility& useWithCommands,
                          const String& help);

        /**
         * Destructor
         */
        ~CommandLineOption() override = default;

        /**
         * Returns true if element expects value
         */
        bool hasValue() const override;

        /**
         * Returns element type
         */
        CommandLineElement::Type type() const override;

        /**
         * Returns element name in help print format
         */
        String printableName() const override;
    };

    /**
     * Command line parameter
     *
     * Command line argument that starts from '-' character and expects a value.
     * It may have a long name that starts from '--', and/or a short name, that starts from '-'.
     * Value has human readable name, such as 'file name', 'text', 'number', etc.. and optional
     * validation pattern that can be regular expression or empty string.
     */
    class SP_EXPORT CommandLineParameter
        : public CommandLineElement
    {
    public:
        /**
         * Constructor
         * @param name          Element name
         * @param shortName     Short element name (single character, options only)
         * @param valueName     Value name
         * @param validateValue Value validation regular expression
         * @param useWithCommands Element visibility for a command (options only)
         * @param help          Help (description) for the element
         */
        CommandLineParameter(const String& name, const String& shortName, String valueName,
                             const String& validateValue,
                             const Visibility& useWithCommands, const String& help);

        /**
         * Returns element name in help print format
         */
        String printableName() const override;

        /**
         * Validates parameter value
         * @param value         Value to validate
         */
        void validate(const String& value) const override;

        /**
         * Returns true if element expects value
         */
        bool hasValue() const override;

        /**
         * Returns element type
         */
        Type type() const override;

    private:
        String m_valueInfo;                                 ///< Value name, for using in help
        std::shared_ptr<RegularExpression> m_validateValue; ///< Value validation regular expression
    };

private:
    String m_programVersion;                                                 ///< Program version and copyright message (for help only).
    String m_description;                                                    ///< Program description (for help only).
    String m_commandLinePrototype;                                           ///< Command line prototype (for help only).
    std::map<String, std::shared_ptr<CommandLineElement>> m_optionTemplates; ///< All the defined options.

    /**
     * All the defined arguments.
     */
    std::map<String, std::shared_ptr<CommandLineArgument>> m_argumentTemplates;
    std::map<String, String> m_values;                            ///< Received option values.
    Strings m_arguments;                                          ///< Received arguments.
    std::filesystem::path m_executablePath;                       ///< Executable path, from argv[0]
    std::list<std::shared_ptr<CommandLineElement>> m_allElements; ///< All defined elements.
    std::list<std::pair<String, String>> m_notes;                 ///< Notes.

    static String preprocessArgument(String& arg, String& quote, String& quotedString);

    void readOption(const Strings& digestedArgs, size_t& i);
};

/**
 * @}
 */
} // namespace sptk
