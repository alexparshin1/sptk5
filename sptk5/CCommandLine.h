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

class CCommandLine
{
public:
    class Visibility
    {
        bool m_inverted;
        CRegExp* m_regexp;
        std::string m_pattern;

    public:

        Visibility(std::string pattern, bool _mustMatch = true);
        Visibility(const Visibility& other);
        ~Visibility();
        bool any();
        bool matches(std::string command);
    };

private:
    class CommandLineElement
    {
    protected:

        enum Type
        {
            IS_UNKNOWN, IS_COMMAND, IS_OPTION, IS_VALUE_OPTION
        };

        std::string m_name;
        std::string m_shortName;
        std::string m_help;

        Visibility m_useWithCommands;
    public:

        CommandLineElement(std::string name, std::string shortName, std::string help,
                const Visibility& useWithCommands);
        virtual ~CommandLineElement();
        virtual Type type();
        virtual std::string name();
        virtual bool hasValue();
        virtual void validate(std::string value);
        virtual std::string printableName();
        bool useWithCommand(std::string command);
        void formatHelp(int textWidth, CStrings& formattedText);
        void printHelp(int nameWidth, int textWidth, std::string optionDefaultValue);

    };

    class CommandLineArgument: public CommandLineElement
    {
    public:
        CommandLineArgument(std::string name, std::string help);
        virtual ~CommandLineArgument();
    };

    class CommandLineOption: public CommandLineElement
    {
    public:
        CommandLineOption(std::string name, std::string shortName, const Visibility& useWithCommands, std::string help);
        virtual ~CommandLineOption();
        virtual bool hasValue();
        virtual CommandLineElement::Type type();
        virtual std::string printableName();
    };

    class CommandLineParameter: public CommandLineElement
    {
        std::string m_valueInfo;
        CRegExp* m_validateValue;

    public:

        CommandLineParameter(std::string name, std::string shortName, std::string valueInfo, std::string validateValue,
                const Visibility& useWithCommands, std::string help);
        virtual ~CommandLineParameter();
        virtual std::string printableName();
        virtual void validate(std::string value);
        virtual bool hasValue();
        virtual Type type();
    };

    std::string m_programVersion;
    std::string m_description;
    std::string m_commandLinePrototype;
    std::map<std::string, CommandLineElement*> m_optionTemplates;
    std::map<std::string, CommandLineArgument*> m_argumentTemplates;
    std::map<std::string, std::string> m_values;
    CStrings m_arguments;
    std::list<CommandLineElement*> m_allElements;

    bool startsWith(std::string str, std::string pattern);
    bool endsWith(std::string str, std::string pattern);

    static void printLine(std::string ch, int count);

public:
    CCommandLine(std::string programVersion, std::string description, std::string commandLinePrototype);
    virtual ~CCommandLine();

    void defineOption(std::string fullName, std::string shortName, Visibility useForCommands, std::string help);
    void defineParameter(std::string fullName, std::string shortName, std::string valueName, std::string validateValue,
            Visibility useForCommands, std::string defaultValue, std::string help);
    void defineArgument(std::string fullName, std::string helpText);

    void init(int argc, const char* argv[]);

    std::string getOptionValue(std::string name);
    bool hasOption(std::string name);
    void setOptionValue(std::string name, std::string value);

    void setOption(std::string name);
    void clearOption(std::string name);

    const CStrings& arguments();

    void printHelp(int screenColumns);
    void printHelp(std::string onlyForCommand, int screenColumns);
    void printVersion();
};

/// @}
}

#endif
