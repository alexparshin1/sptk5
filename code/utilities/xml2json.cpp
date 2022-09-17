#include <iostream>
#include <sptk5/cutils>

using namespace std;
using namespace sptk;

void printHelp(const String& progName, const String& error = "")
{
    cout << "Convertor of XML to JSON and back" << endl
         << endl;
    cout << "Syntax:" << endl;
    cout << progName << " <filename>" << endl
         << endl;
    if (!error.empty())
    {
        cerr << "Error: " << error << endl
             << endl;
    }
}

int main(int argc, const char* argv[])
{
    if (argc != 2)
    {
        printHelp(argv[0], "Invalid number of arguments");
        return 1;
    }

    RegularExpression matchExtension(R"(^(.*)\.(xml|json)$)", "i");
    auto matches = matchExtension.m(argv[1]);
    if (!matches)
    {
        printHelp(argv[0], "Invalid file extension");
        return 1;
    }

    auto extension = matches[1].value.toLowerCase();

    xdoc::Document document;
    Buffer documentData;
    try
    {
        documentData.loadFromFile(argv[1]);
        document.load(documentData);
        fs::path outputFileName;
        if (extension == "xml")
        {
            document.exportTo(xdoc::DataFormat::JSON, documentData, true);
            outputFileName = matchExtension.s(argv[1], R"(\1.json)");
        }
        else
        {
            document.exportTo(xdoc::DataFormat::XML, documentData, true);
            outputFileName = matchExtension.s(argv[1], R"(\1.xml)");
        }
        documentData.saveToFile(outputFileName);
    }
    catch (const Exception& e)
    {
        printHelp(argv[0], e.what());
        return 1;
    }

    return 0;
}
