#pragma once

#include <sptk5/String.h>

namespace sptk {

class TestSettings
{
public:
    TestSettings(std::filesystem::path settingsFile);

private:
    std::map<String, String> m_keys;
};

} // namespace sptk
