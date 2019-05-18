#ifndef __PERSIST_MEMORY_MANAGER_H__
#define __PERSIST_MEMORY_MANAGER_H__

#include <sptk5/persist/MemoryBucket.h>

namespace sptk {
namespace persist {

class SP_EXPORT MemoryPool
{
public:
    MemoryPool(const String& directory, const String& objectName);
private:
    String                          m_directory;
    std::map<void*,MemoryBucket*>   m_buckets;
};

}
}

#endif
