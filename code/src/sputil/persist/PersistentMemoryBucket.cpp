#include "sptk5/persist/PersistentMemoryBucket.h"
#include <iostream>

using namespace std;
using namespace sptk;

PersistentMemoryBucket::PersistentMemoryBucket(const std::string& fileName, size_t size)
: m_memory(fileName, size)
{
    m_memory.open();
    load();
}

void PersistentMemoryBucket::load()
{
    lock_guard<mutex> lock(m_mutex);

    m_index.clear();

    char* memoryStart = (char*) m_memory.data();
    char* memoryEnd = memoryStart + m_memory.size();
    char* itemPtr = memoryStart;
    Item* item = (Item*) itemPtr;
    size_t itemsLoaded {0};
    while (item->signature == itemSignature) {
        uint64_t id = item->id;
        if (id != 0) {
            // item isn't deleted
            auto ret = m_index.insert(pair<uint64_t, Item *>(id, item));
            if (ret.second == false) {
                // Duplicate id
                item->id = 0;
            } else
                itemsLoaded++;
        }
        size_t itemFullSize = sizeof(Item) + item->size;
        itemPtr += itemFullSize;
        if (itemPtr > memoryEnd)
            break;
        item = (Item*)itemPtr;
    }
    cout << "loaded " << itemsLoaded << endl;
}

void * PersistentMemoryBucket::insert(uint64_t id, void *data, size_t bytes)
{
    lock_guard<mutex> lock(m_mutex);

    size_t itemFullSize = sizeof(Item) + bytes;
    if (m_memory.size() < m_allocated + itemFullSize)
        throw runtime_error("Bucket is full");

    char* itemPtr = (char*) m_memory.data() + m_allocated;
    char* dataPtr = itemPtr + sizeof(Item);
    Item* item = (Item*) itemPtr;

    if (id == 0)
        throw invalid_argument("Invalid id");

    auto ret = m_index.insert(pair<uint64_t ,Item*>(id, item));
    if (ret.second == false)
        throw invalid_argument("Duplicate id");

    item->signature = itemSignature;
    item->id = id;
    item->size = bytes;

    memcpy(dataPtr, data, bytes);

    m_allocated += itemFullSize;

    return dataPtr;
}

void PersistentMemoryBucket::free(void *data)
{
    lock_guard<mutex> lock(m_mutex);

    auto* itemPtr = (char*) data - sizeof(Item);
    auto* item = (Item*) itemPtr;
    if (item->signature != itemSignature)
        throw invalid_argument("Invalid persistent address");

    m_index.erase(item->id);
    item->id = 0;
}

void PersistentMemoryBucket::clear()
{
    lock_guard<mutex> lock(m_mutex);
    auto* itemPtr = (char*)m_memory.data();
    memset(itemPtr, 0, m_memory.size());
    m_index.clear();
    m_allocated = 0;
}

bool PersistentMemoryBucket::empty() const
{
    return m_index.empty();
}

size_t PersistentMemoryBucket::size() const
{
    lock_guard<mutex> lock(m_mutex);
    return m_memory.size();
}

size_t PersistentMemoryBucket::available() const
{
    lock_guard<mutex> lock(m_mutex);
    int availableBytes = (int) m_memory.size() - int(m_allocated + sizeof(Item));
    if (availableBytes < 0)
        availableBytes = 0;
    return availableBytes;
}

void* PersistentMemoryBucket::find(uint64_t id, size_t& size) const
{
    lock_guard<mutex> lock(m_mutex);

    auto itor = m_index.find(id);
    if (itor == m_index.end())
        return nullptr;

    size = itor->second->size;
    return (char*) itor->second + sizeof(Item);
}
