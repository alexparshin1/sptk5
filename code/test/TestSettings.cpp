#include "TestSettings.h"
#include <sptk5/xdoc/Document.h>

using namespace std;
using namespace sptk;

void TestSettings::load(std::filesystem::path settingsFile)
{
    Buffer settings;
    settings.loadFromFile(settingsFile);

    xdoc::Document document;
    document.load(settings);

    for (const auto& node: document.root()->nodes())
    {
        m_keys[node->getName()] = node->getString();
    }
}
