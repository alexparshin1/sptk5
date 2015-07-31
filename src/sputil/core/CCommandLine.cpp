#include <sptk5/CCommandLine.h>
#include <iomanip>

using namespace std;
using namespace sptk;

map<string, CArgumentDefinition*> CArgumentDefinition::argumentNamesAndShortcuts;

CArgumentDefinition::CArgumentDefinition(string name, char shortcut, string help, Type type, string defaultValue, string validateRegexp)
: m_name(name), m_help(help), m_type(type), m_defaultValue(defaultValue)
    {
        if (!name.empty())
            argumentNamesAndShortcuts[name] = this;
        if (shortcut != char(0))
            argumentNamesAndShortcuts[string(1,shortcut)] = this;
        if (validateRegexp.empty())
            m_validateRegexp = NULL;
        else
            m_validateRegexp = new sptk::CRegExp(validateRegexp);
    }

    void CArgumentDefinition::validate(string value)
    {
        if (m_validateRegexp && *m_validateRegexp == value)
            throw sptk::CException("Value '" + value + "' is invalid for parameter --" + m_name);
    }

    CArgumentDefinition* CArgumentDefinition::get(string nameOrShortcut)
    {
        map<string, CArgumentDefinition*>::iterator itor = argumentNamesAndShortcuts.find(nameOrShortcut);
        if (itor == argumentNamesAndShortcuts.end())
            return NULL;
        return itor->second;
    }

void CCommandLine::init(int argc, const char* argv[]) throw (exception)
{
    m_progname = argv[0];

    // Convert combined options into singles
    CStrings arguments;
    for (int i = 1; i < argc; i++) {
        const char* arg = argv[i];

        if (arg[0] != '-') {
            // Argument isn't an option
            arguments.push_back(arg);
            continue;
        }

        if (arg[1] == '-') {
            // Argument is a parameter
            arguments.push_back(arg);
            continue;
        }

        // Argument is on or more options
        for (size_t j = 1; j < strlen(arg); j++) {
            string option(1, arg[j]);
            CArgumentDefinition* argdef = CArgumentDefinition::get(option);
            if (!argdef)
                throw CException("Option -" + option + " is not defined");
            arguments.push_back("--" + argdef->name());
        }
    }

    m_names.clear();
    for (size_t i = 0; i < arguments.size(); i++) {
        const string& argument = arguments[i];

        if (argument.substr(0,2) != "--") {
            if (!m_allowNames)
                throw CException("Argument " + argument + " is not an option or parameter name");
            m_names.push_back(argument);
        }

        string name = argument.substr(2);
        CArgumentDefinition* argdef = CArgumentDefinition::get(name);
        if (!argdef)
            throw CException("Option --" + name + " is unknown");

        if (argdef->type() == CArgumentDefinition::Type::OPTION) {
            m_options.insert(argdef->name());
            continue;
        }

        i++;
        if (i > arguments.size())
            throw CException("Parameter " + argument + " doesn't have a value");

        const string& value = arguments[i];
        argdef->validate(value);
        m_parameters[name] = value;
    }
}

bool CCommandLine::hasOption(string name) const throw (exception)
{
    if (m_options.find(name) != m_options.end())
        return true;

    CArgumentDefinition* argdef = CArgumentDefinition::get(name);
    if (!argdef || argdef->type() != CArgumentDefinition::Type::OPTION)
        throw CException("Option --" + name + " is invalid");

    return false;
}

string CCommandLine::parameterValue(string name) const throw (exception)
{
    map<string,string>::const_iterator itor = m_parameters.find(name);
    if (itor != m_parameters.end())
        return itor->second;

    CArgumentDefinition* argdef = CArgumentDefinition::get(name);
    if (!argdef || argdef->type() != CArgumentDefinition::Type::PARAMETER)
        throw CException("Parameter --" + name + " is invalid");

    return argdef->defaultValue();
}

void formatText(CStrings& text, string fromText, size_t width)
{
    CStrings words(fromText, " \n\r", CStrings::SM_ANYCHAR);
    string   row;

    for (string word: words) {
        if (row.length() + word.length() >= width) {
            text.push_back(row);
            row = "";
        }
        if (!row.empty())
            row += " ";
        row += word;
    }

    if (!row.empty())
        text.push_back(row);
}

void CCommandLine::printTypeHelp(CArgumentDefinition::Type type, unsigned screenColumns, unsigned commandColumns) const
{
    switch (type) {
        case CArgumentDefinition::Type::PARAMETER:
            cout << "Parameters:" << endl << endl;
            break;
        case CArgumentDefinition::Type::OPTION:
            cout << "Options:" << endl << endl;
            break;
    }

    if (screenColumns < commandColumns + 20)
        screenColumns = commandColumns + 20;

    for (auto& itor: CArgumentDefinition::definitions()) {
        string name = itor.first;
        const CArgumentDefinition* argdef = itor.second;

        if (argdef->type() == type && name == argdef->name()) {
            CStrings helpText;
            string description = replaceAll(argdef->help(), "${DEFAULT}", argdef->defaultValue());
            formatText(helpText, description, screenColumns - commandColumns);
            int rowNumber = 0;
            for (string& helpRow: helpText) {
                if (rowNumber == 0)
                    cout << setw(commandColumns) << left << argdef->name();
                else
                    cout << setw(commandColumns) << " ";
                cout << helpRow << endl;
                rowNumber++;
            }
        }
    }
    cout << endl;
}

void CCommandLine::printHelp(string argumentType) const
{
    size_t screenColumns = 80;
    const char* envterm = getenv("TERM");
    if (envterm) {
        FILE* tput = popen("tput cols", "r");
        if (tput) {
            fscanf(tput, "%lu", &screenColumns);
            fclose(tput);
        }
    }

    size_t maxNameLength = 0;
    bool hasOptions = false, hasParameters = false;
    for (auto& itor: CArgumentDefinition::definitions()) {
        CArgumentDefinition* argdef = itor.second;
        switch (argdef->type()) {
            case CArgumentDefinition::PARAMETER:
                hasParameters = true;
                break;
            case CArgumentDefinition::OPTION:
                hasOptions = true;
                break;
        }
        if (maxNameLength < argdef->name().length())
            maxNameLength = argdef->name().length();
    }

    cout << "Syntax:\n\n";
    cout << " " << m_progname << " ";
    if (hasOptions)
        cout << "[options] ";
    if (hasParameters)
        cout << "[parameters] ";
    if (m_allowNames)
        cout << argumentType << "(s)";
    cout << endl;

    cout << "" << endl;

    if (hasParameters)
        printTypeHelp(CArgumentDefinition::Type::PARAMETER, screenColumns, maxNameLength + 2);

    if (hasOptions)
        printTypeHelp(CArgumentDefinition::Type::OPTION, screenColumns, maxNameLength + 2);
}