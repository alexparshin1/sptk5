#pragma once

#include <sptk5/String.h>

namespace sptk {

class SP_EXPORT TestSettings
{
public:
    void load(std::filesystem::path settingsFile);

private:
    std::map<String, String> m_keys;
};

} // namespace sptk
