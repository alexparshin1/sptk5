#ifndef __PERSIST_MEMORYBUCKET_H__
#define __PERSIST_MEMORYBUCKET_H__

#include "MemoryMappedFile.h"

namespace sptk {
namespace persist {

class MemoryBucket;

static constexpr uint32_t allocatedMark = 0x5F7F9FAF;
static constexpr uint32_t releasedMark = 0x5E7E9EAE;

/**
 * Abstract interface that defines methods
 * needed to store/restore class in/from persistent memory
 */
class SP_EXPORT Record
{
protected:
    virtual size_t packSize() const = 0;
    virtual void pack(void* destination) const = 0;
    virtual void unpack(const void* destination) = 0;
};

class SP_EXPORT Handle : public Record
{
    friend class MemoryBucket;
    MemoryBucket*   m_bucket {nullptr};

protected:
    void*           m_record {nullptr};

    size_t packSize() const override;
    void pack(void* destination) const override;
    void unpack(const void* destination) override;
public:
    Handle(MemoryBucket& bucket, size_t m_bucketOffset);
    Handle(size_t bucketId, size_t m_bucketOffset);
    Handle(const Handle& other) = default;
    Handle& operator = (const Handle& other) = default;
    explicit operator void* () const;
    void* data() const;
    const char* c_str() const;
    size_t size() const;
};

class SP_EXPORT MemoryBucket
{
    friend class Handle;
    struct FreeBlocks
    {
        uint32_t                            m_size;
        std::map<uint32_t,uint32_t>         m_offsetMap;
        std::multimap<uint32_t,uint32_t>    m_sizeMap;

        FreeBlocks(uint32_t size);
        void load(uint32_t offset, uint32_t size);
        void free(uint32_t offset, uint32_t size);
        uint32_t alloc(uint32_t size);
        void clear();
        size_t count() const;
        uint32_t available() const;

        void print() const;
    };

public:

    MemoryBucket(const String& directoryName, uint32_t id, size_t size);

    std::vector<Handle> load();

    const uint32_t id() const;
    const void* data() const;

    Handle alloc(void* data, size_t bytes);

    void free(Handle& data);

    void clear();

    bool empty() const;

    size_t size() const;

    size_t available() const;

    static MemoryBucket* find(uint32_t bucketId);

    const FreeBlocks& freeBlocks() { return m_freeBlocks; }

protected:

    struct Item
    {
        uint32_t signature {0};
        uint32_t size {0};
    };

private:

    mutable std::mutex          m_mutex;
    uint32_t                    m_id;
    MemoryMappedFile            m_mappedFile;
    FreeBlocks                  m_freeBlocks;

    static String formatId(uint32_t bucketId);
};

}
}

#endif