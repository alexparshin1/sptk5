#ifndef __PERSIST_MEMORYBUCKET_H__
#define __PERSIST_MEMORYBUCKET_H__

#include "MemoryBlock.h"

namespace sptk {
namespace persist {

class MemoryBucket
{
public:
    MemoryBucket(const std::string& fileName, size_t size);

    void load();

    void* insert(uint64_t id, void* data, size_t bytes);

    void* find(uint64_t id, size_t& bytes) const;

    void free(void* data);

    void clear();

    bool empty() const;

    size_t size() const;

    size_t available() const;

private:

    static constexpr uint32_t itemSignature = 0x5F7F9FAF;

    struct Item
    {
        uint32_t signature{0};
        size_t size{0};
        uint64_t id{0};
    };

    mutable std::mutex          m_mutex;
    MemoryBlock                 m_memory;
    std::map<uint64_t, Item*>   m_index;
    size_t                      m_allocated{0};
};

}
}

#endif