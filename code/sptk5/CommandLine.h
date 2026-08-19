/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#pragma once

#include <list>
#include <map>
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
     * Argument(s) are defined with a regular expression that either should match or should not match depending
     * on constructor mustMatch parameter value.
     */
    class SP_EXPORT Visibility
    {
    public:
        /**
         * Constructor
         * @param pattern       Regular expression to match or do not match command line argument. If empty, then it matches any argument.
         * @param mustMatch     If true then regular expression should match, otherwise it shouldn't match.
         */
        Visibility(const String& pattern, bool mustMatch = true);

        /**
         * Returns true if there is no regular expression to match and matches any argument.
         */
        [[nodiscard]] bool any() const;

        /**
         * Returns true if matches given argument or command.
         */
        [[nodiscard]] bool matches(const String& command) const;

    private:
        bool                               m_inverted; ///< If true then expression should not match
        std::shared_ptr<RegularExpression> m_regexp;   ///< Regular expression for matching an argument(s)
    };

    /**
     * prints a line of characters
     * @param fillChar                Character to print
     * @param count             Number of characters to print
     */
    static void printLine(const String& fillChar, size_t count);

    /**
     * Print help on commands
     * @param onlyForCommand    Optional: if not empty, then print only this command help
     * @param screenColumns     Screen width in columns
     * @param nameColumns       Number of columns for command names
     * @param sortedCommands    Sorted command list
     * @param helpTextColumns   Number of columns for help text
     */
    void printCommands(const String& onlyForCommand, size_t screenColumns, size_t nameColumns,
                       const Strings& sortedCommands, size_t helpTextColumns) const;

    /**
     * Print help on options
     * @param onlyForCommand    Optional: if not empty, then print only this command help
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
     * An option doesn't expect a value. If it is present in the command line, it assumes the value 'yes',
     * otherwise it has the value 'no'.
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
                         const String&     validateValue,
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
    [[maybe_unused]] void addNote(const String& title, const String& text);

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
    [[nodiscard]] String getOptionValue(const String& name) const;

    /**
     * @brief Returns true if the actual command line contains the option.
     * @param name              Option name.
     */
    [[nodiscard]] bool hasOption(const String& name) const;

    /**
     * @brief Re-defines actual option value.
     * @param name              Option name.
     * @param value             Option value.
     */
    void setOptionValue(const String& name, const String& value = "yes");

    /**
     * @brief Returns the list of command line arguments.
     */
    [[nodiscard]] const Strings& arguments() const;

    /**
     * @brief Prints full help.
     * @param screenColumns     Screen width in columns.
     */
    void printHelp(size_t screenColumns) const;

    /**
     * @brief Prints help for a given command/argument.
     * @param onlyForCommand    Command to print help for.
     * @param screenColumns     Screen width in columns.
     */
    void printHelp(const String& onlyForCommand, size_t screenColumns) const;

    /**
     * @brief Prints program version.
     */
    void printVersion() const;

    /**
     * @brief Preprocess command line arguments.
     * @param argv              Command line arguments.
     * @return preprocessed command line arguments.
     */
    static Strings preprocessArguments(const std::vector<const char*>& argv);

    /**
     * @brief Re-write command line arguments.
     * @param arguments         Command line arguments.
     * @return re-written command line arguments.
     */
    static Strings rewriteArguments(const Strings& arguments);

    /**
     * @brief Command line element that could be argument/command, option, or option with value.
     */
    class SP_EXPORT CommandLineElement
    {
    public:
        /**
         * @brief Command line element type.
         */
        enum class Type
        {
            IS_UNKNOWN,     ///< Type is unknown or undefined.
            IS_OPTION,      ///< Element is an option that doesn't expect value.
            IS_VALUE_OPTION ///< Element is an option that expects value.
        };

        /**
         * @brief Constructor.
         * @param name              Element name.
         * @param shortName         Short element name (single character).
         * @param help              Help (description) for the element.
         * @param useWithCommands   Element visibility for a command (options only).
         */
        CommandLineElement(String name, String shortName, String help,
                           Visibility useWithCommands);

        /**
         * @brief Destructor.
         */
        virtual ~CommandLineElement() = default;

        /**
         * @brief Returns element type.
         */
        [[nodiscard]] virtual Type type() const;

        /**
         * @brief Returns element name.
         */
        [[nodiscard]] virtual String name() const;

        /**
         * @brief Returns element short name.
         */
        [[nodiscard]] virtual String shortName() const;

        /**
         * @brief Returns true if the element expects value.
         */
        [[nodiscard]] virtual bool hasValue() const;

        /**
         * @brief Validates given value (for elements that provide validation).
         *
         * Throws an exception if the value is invalid.
         * @param value             Value to validate.
         */
        virtual void validate(const String& value) const;

        /**
         * @brief Returns element name in help print format.
         */
        [[nodiscard]] virtual String printableName() const;

        /**
         * @brief Returns true if the element may be used with command (options only).
         */
        [[nodiscard]] bool useWithCommand(const String& command) const;

        /**
         * @brief Formats element help for printout.
         * @param textWidth         Help text width.
         * @param formattedText     Formatted help text.
         */
        void formatHelp(size_t textWidth, Strings& formattedText) const;

        /**
         * @brief Prints element help.
         * @param nameWidth         Optional name width.
         * @param textWidth         Help text width.
         * @param optionDefaultValue Option default value (if any).
         */
        void printHelp(size_t nameWidth, size_t textWidth, const String& optionDefaultValue) const;

    private:
        String     m_name;            ///< Element name.
        String     m_shortName;       ///< Short element name (single character, options only).
        String     m_help;            ///< Help (description) for the element.
        Visibility m_useWithCommands; ///< Element visibility for a command (options only).
    };

    /**
     * @brief Command line argument.
     *
     * Command line argument that doesn't start from the '-' character and doesn't expect a value,
     * AKA command.
     */
    class SP_EXPORT CommandLineArgument
        : public CommandLineElement
    {
    public:
        /**
         * @brief Constructor.
         * @param name          Element name.
         * @param help          Help (description) for the element.
         */
        CommandLineArgument(const String& name, const String& help);

        /**
         * @brief Destructor.
         */
        ~CommandLineArgument() override = default;
    };

    /**
     * Command line option
     *
     * Command line argument that starts from the '-' character and doesn't expect a value.
     * It may have a long name that starts from '--', and/or a short name that starts from '-'.
     */
    class SP_EXPORT CommandLineOption
        : public CommandLineElement
    {
    public:
        /**
         * @brief Constructor.
         * @param name              Element name.
         * @param shortName         Short element name (single character, options only).
         * @param useWithCommands   Element visibility for a command (options only).
         * @param help              Help (description) for the element.
         */
        CommandLineOption(const String& name, const String& shortName, const Visibility& useWithCommands,
                          const String& help);

        /**
         * @brief Destructor.
         */
        ~CommandLineOption() override = default;

        /**
         * @brief Returns true if the element expects value.
         */
        [[nodiscard]] bool hasValue() const override;

        /**
         * @brief Returns element type.
         */
        [[nodiscard]] Type type() const override;

        /**
         * @brief Returns element name in help print format.
         */
        [[nodiscard]] String printableName() const override;
    };

    /**
     * @brief Command line parameter.
     *
     * Command line argument that starts from the '-' character and expects a value.
     * It may have a long name that starts from the '--', and/or a short name that starts from '-'.
     * Value has a human-readable name, such as 'file name', 'text', 'number', etc. and an optional
     * validation pattern that can be regular expression or empty string.
     */
    class SP_EXPORT CommandLineParameter
        : public CommandLineElement
    {
    public:
        /**
         * @brief Constructor.
         * @param name          Element name.
         * @param shortName     Short element name (single character, options only).
         * @param valueName     Value name.
         * @param validateValue Value validation regular expression.
         * @param useWithCommands Element visibility for a command (options only).
         * @param help          Help (description) for the element.
         */
        CommandLineParameter(const String& name, const String& shortName, String valueName,
                             const String&     validateValue,
                             const Visibility& useWithCommands, const String& help);

        /**
         * @brief Returns element name in help print format.
         */
        [[nodiscard]] String printableName() const override;

        /**
         * @brief Validates parameter value.
         * @param value         Value to validate.
         */
        void validate(const String& value) const override;

        /**
         * @brief Returns true if the element expects value.
         */
        [[nodiscard]] bool hasValue() const override;

        /**
         * @brief Returns element type.
         */
        [[nodiscard]] Type type() const override;

    private:
        String                             m_valueInfo;     ///< Value name, for using in help.
        std::shared_ptr<RegularExpression> m_validateValue; ///< Value validation regular expression.
    };

private:
    String                                                 m_programVersion;       ///< Program version and copyright message (for help only).
    String                                                 m_description;          ///< Program description (for help only).
    String                                                 m_commandLinePrototype; ///< Command line prototype (for help only).
    std::map<String, std::shared_ptr<CommandLineElement>>  m_optionTemplates;      ///< All the defined options.
    std::map<String, std::shared_ptr<CommandLineArgument>> m_argumentTemplates;    ///< All the defined arguments.
    std::map<String, String>                               m_values;               ///< Received option values.
    Strings                                                m_arguments;            ///< Received arguments.
    std::filesystem::path                                  m_executablePath;       ///< Executable path, from argv[0]
    std::list<std::shared_ptr<CommandLineElement>>         m_allElements;          ///< All defined elements.
    std::list<std::pair<String, String>>                   m_notes;                ///< Notes.

    static String preprocessArgument(String& arg, String& quote, String& quotedString);

    void readOption(const Strings& digestedArgs, size_t& argumentIndex);
};

/**
 * @}
 */
} // namespace sptk
