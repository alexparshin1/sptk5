#ifndef __PERSIST_MEMORY_MANAGER_H__
#define __PERSIST_MEMORY_MANAGER_H__

#include <sptk5/persist/MemoryBucket.h>

namespace sptk {
namespace persist {

class MemoryManager
{
    String                  m_directory;
    std::set<MemoryBucket>  m_buckets;
public:

};

}
}

#endif
