#ifndef __PERSIST_MEMORY_MANAGER_H__
#define __PERSIST_MEMORY_MANAGER_H__

#include <sptk5/persist/MemoryBucket.h>
#include <sptk5/Loop.h>

namespace sptk {
namespace persist {

class SP_EXPORT MemoryPool
{
public:

    MemoryPool(const String& directory, const String& objectName, uint32_t bucketSize);

    void load(std::vector<Handle>* handles);

    Handle insert(const void* data, size_t bytes);
    void free(Handle handle);

private:

    std::mutex                          m_mutex;

    String                              m_directory;
    String                              m_objectName;
    uint32_t                            m_bucketSize;
    Loop<SMemoryBucket>                 m_bucketLoop;
    std::map<uint32_t,SMemoryBucket>    m_bucketMap;

    uint32_t      makeBucketIdUnlocked();
    SMemoryBucket createBucketUnlocked(uint32_t id);

    SMemoryBucket createBucket(uint32_t id);
};

}
}

#endif
