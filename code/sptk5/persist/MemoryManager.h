#ifndef __PERSIST_MEMORY_MANAGER_H__
#define __PERSIST_MEMORY_MANAGER_H__

#include <sptk5/persist/MemoryBucket.h>

namespace sptk {
namespace persist {

class MemoryManager
{
public:
    MemoryManager(const String& directory);
private:
    String                          m_directory;
    std::map<void*,MemoryBucket>    m_buckets;
};

}
}

#endif
