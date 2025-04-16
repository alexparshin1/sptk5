#include <iostream>
#include <sptk5/cutils>

using namespace std;
using namespace sptk;

namespace {
void printHelp(const String& progName, const String& error = "")
{
    COUT("Convertor of XML to JSON and back\n\n"
         << "Syntax:\n"
         << progName << " <filename>\n\n");
    if (!error.empty())
    {
        CERR("Error: "
             << error << "\n\n");
    }
}
}

int main(int argc, const char* argv[])
{
    if (argc != 2)
    {
        printHelp(argv[0], "Invalid number of arguments");
        return 1;
    }

    const RegularExpression matchExtension(R"(^(.*)\.(xml|json)$)", "i");
    const auto matches = matchExtension.m(argv[1]);
    if (!matches)
    {
        printHelp(argv[0], "Invalid file extension");
        return 1;
    }

    const auto extension = matches[1].value.toLowerCase();

    try
    {
        const xdoc::Document document;
        Buffer documentData;
        documentData.loadFromFile(argv[1]);
        document.load(documentData);
        String outputFileName;
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
        documentData.saveToFile(outputFileName.c_str());
    }
    catch (const Exception& e)
    {
        printHelp(argv[0], e.what());
        return 1;
    }

    return 0;
}
