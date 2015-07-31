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

#ifndef __CCOMMANDLINE_H__
#define __CCOMMANDLINE_H__

#include <map>
#include <set>
#include <list>
#include <sptk5/cutils>
#include <sptk5/CRegExp.h>

namespace sptk {

/// @addtogroup utility Utility Classes
/// @{

/// @brief Base class for command line argument definition
///
/// Each argument must have full name that is used in command line with '--' prefix.
/// Each argument may have a single char shortcut that is used in command line with '-' prefix.
/// Command line arguments are checked to match definitions.
/// The definitions may be declared as static variables anywhere
/// in program code - they are still global.
class CArgumentDefinition
{
    /// @brief Map of full option/parameter names and shortcuts to argument definitions
    static std::map<std::string, CArgumentDefinition*> argumentNamesAndShortcuts;

public:
    /// @brief Argument definition type
    enum Type {
        PARAMETER,  ///< Argument definition is for command line parameter
        OPTION      ///< Argument definition is for command line option
    };

private:
    std::string     m_name;             ///< Full name of the parameter or option
    std::string     m_help;             ///< Description of the parameter or option
    Type            m_type;             ///< Argument definition type
    std::string     m_defaultValue;     ///< Optional default value (parameter only)
    sptk::CRegExp*  m_validateRegexp;   ///< Optional value validation regexp (parameter only)

protected:
    /// @brief Protected constructor (is used only by derived classes)
    /// @param name std::string, Full name of the parameter or option
    /// @param shortcut char, Single character shortcut
    /// @param help std::string, Description of the parameter or option
    /// @param type CArgumentDefinition::Type, Argument definition type
    /// @param defaultValue std::string, Optional default value (parameter only)
    /// @param validateRegexp std::string, Optional value validation regexp (parameter only)
    CArgumentDefinition(std::string name, char shortcut, std::string help, Type type, std::string defaultValue="", std::string validateRegexp="");

public:
    /// @brief Validate value using optional parameter validation regexep
    void validate(std::string value);

    /// @brief Get parameter or option definition by full name or shortcut
    /// @return Parameter or option definition, or NULL if not found
    static CArgumentDefinition* get(std::string nameOrShortcut);

    /// @brief Get parameter or option definition full name
    const std::string& name() const { return m_name; }

    /// @brief Get parameter or option definition description
    const std::string& help() const { return m_help; }

    /// @brief Get parameter or option definition type (parameter or option)
    Type               type() const { return m_type; }

    /// @brief Get parameter definition default value
    const std::string& defaultValue() const { return m_defaultValue; }

    /// @brief Get global list of parameter/option definitions
    static const std::map<std::string, CArgumentDefinition*>& definitions()
    {
        return argumentNamesAndShortcuts;
    }
};

/// @brief Definition of command line option
///
/// A command line option has a value true when it is included in command line,
/// or false otherwise.
/// Command line arguments are checked to match definitions.
/// The definitions may be declared as static variables anywhere
/// in program code - they are still global.
class CCommandLineOption : public CArgumentDefinition
{
public:
    /// @brief Constructor
    /// @param name std::string, Full name of the parameter or option
    /// @param shortcut char, Single character shortcut
    /// @param help std::string, Description of the parameter or option
    CCommandLineOption(std::string name, char shortcut=0, std::string help="")
    : CArgumentDefinition(name, shortcut, help, OPTION) {}
};

/// @brief Definition of command line parameter
///
/// A command line parameter has a value provided in command line,
/// or uses its default value otherwise. If a validation regexp is
/// provided, command line value is verified with that regexp.
/// Command line arguments are checked to match definitions.
/// The definitions may be declared as static variables anywhere
/// in program code - they are still global.
class CCommandLineParameter : public CArgumentDefinition
{
public:
    /// @brief Constructor
    /// @param name std::string, Full name of the parameter or option
    /// @param shortcut char, Single character shortcut
    /// @param help std::string, Description of the parameter or option
    /// @param defaultValue std::string, Optional default value (parameter only)
    /// @param validateRegexp std::string, Optional value validation regexp (parameter only)
    CCommandLineParameter(std::string name, char shortcut=0, std::string help="", std::string defaultValue="", std::string validateRegexp="")
    : CArgumentDefinition(name, shortcut, help, PARAMETER, defaultValue, validateRegexp) {}
};

/// @brief Command line arguments parser
///
/// Checks argument types and optionally verifies argument values
/// using regular expressions.
class CCommandLine
{
private:

    std::string                             m_progname;     ///< Program executable name
    bool                                    m_allowNames;   ///< If true then allow names (arguments not started with '-')
    std::list<std::string>                  m_names;        ///< Names (object names, file names) listed in command line
    std::map<std::string, std::string>      m_parameters;   ///< Parameters (options with values) listed in command line
    std::set<std::string>                   m_options;      ///< Options (options without values) listed in command line

protected:

    /// @brief Prints help on arguments of a given type
    /// @param type CArgumentDefinition::Type, Argument type
    /// @param screenColumns unsigned, Screen width
    /// @param commandColumns unsigned, Command name column width
    void printTypeHelp(CArgumentDefinition::Type type, unsigned screenColumns, unsigned commandColumns) const;
public:

    /// @brief Constructor
    /// @param allowNames bool, If true then allow names (arguments not started with '-')
    CCommandLine(bool allowNames=true)
    : m_allowNames(allowNames)
    {}

    /// @brief Initializer
    ///
    /// Must be called before accessing any methods of this class
    /// @param argc int, Number of program command line arguments as it's passed to main()
    /// @param argv const char* argv[], Program command line arguments as it's passed to main()
    void init(int argc, const char* argv[]) throw (std::exception);

    /// @brief Returns true if an option was set in program command line
    /// @param name std::string, Option name
    bool hasOption(std::string name) const throw (std::exception);

    /// @brief Returns command line parameter value
    ///
    /// If corresponding parameter was not in command line arguments, but was defined with
    /// a default value, returns that default value
    /// @param name std::string, Parameter name
    std::string parameterValue(std::string name) const throw (std::exception);

    /// @brief Returns list of names (arguments not started with '-'
    const std::list<std::string>& names() const { return m_names; }

    /// @brief Returns program executable name, as defined in command line argument 0
    std::string progname() const { return m_progname; }

    /// @brief Prints help on parameters and options
    /// @param argumentType std::string, a type of an argument that is printed in help
    void printHelp(std::string argumentType="filename") const;
};

/// @}

}

#endif
