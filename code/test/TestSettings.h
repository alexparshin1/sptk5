#pragma once

#include <sptk5/String.h>

namespace sptk {
class SP_EXPORT TestSettings
{
public:
    void load(const std::filesystem::path& settingsFile);

private:
    std::map<std::string, std::string> m_keys;
};

} // namespace sptk
