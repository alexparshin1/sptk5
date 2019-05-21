/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       MemoryBucket.cpp - description                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Sunday May 19 2019                                     ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
┌──────────────────────────────────────────────────────────────────────────────┐
│   This library is free software; you can redistribute it and/or modify it    │
│   under the terms of the GNU Library General Public License as published by  │
│   the Free Software Foundation; either version 2 of the License, or (at your │
│   option) any later version.                                                 │
│                                                                              │
│   This library is distributed in the hope that it will be useful, but        │
│   WITHOUT ANY WARRANTY; without even the implied warranty of                 │
│   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library   │
│   General Public License for more details.                                   │
│                                                                              │
│   You should have received a copy of the GNU Library General Public License  │
│   along with this library; if not, write to the Free Software Foundation,    │
│   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include "smq/persistent/MemoryBucket.h"

using namespace std;
using namespace sptk;
using namespace smq::persistent;
using namespace chrono;

String MemoryBucket::formatId(uint32_t bucketId)
{
    char formattedId[64];
    snprintf(formattedId, 32, "%010d", bucketId);
    return String(formattedId, 10);
}

MemoryBucket::MemoryBucket(const String& directoryName, const String& objectName, uint32_t id, size_t size)
: m_id(id),
  m_objectName(objectName),
  m_mappedFile(directoryName + "/" + objectName + "_" + formatId(id), size),
  m_freeBlocks((uint32_t)m_mappedFile.size())
{
    m_mappedFile.open();
    load(nullptr);
}

void MemoryBucket::load(std::vector<Handle>* handles)
{
    lock_guard<mutex> lock(m_mutex);

    m_freeBlocks.clear();

    char* memoryStart = (char*) m_mappedFile.data();
    char* memoryEnd = memoryStart + m_mappedFile.size();
    char* itemPtr = memoryStart;
    Item* item = (Item*) itemPtr;
    bool done = false;
    uint32_t offset = 0;
    while (!done) {
        size_t itemFullSize = sizeof(Item) + item->size;
        switch (item->signature) {
            case allocatedMark:
                if (handles)
                    handles->emplace_back(*this, offset);
                offset += itemFullSize;
                break;
            case releasedMark:
                m_freeBlocks.load(offset, item->size);
                offset += itemFullSize;
                break;
            default:
                done = true;
                break;
        }
        itemPtr = memoryStart + offset;
        if (itemPtr > memoryEnd)
            break;
        item = (Item*)itemPtr;
    }

    if (offset < m_mappedFile.size())
        m_freeBlocks.load(offset, uint32_t(m_mappedFile.size()) - offset);
}

Handle MemoryBucket::insert(const void* data, size_t bytes)
{
    lock_guard<mutex> lock(m_mutex);

    size_t   itemFullSize = sizeof(Item) + bytes;
    uint32_t offset = m_freeBlocks.alloc((uint32_t)itemFullSize);
    if (offset == UINT32_MAX)
        return Handle();

    Handle handle(*this, offset);

    char* itemPtr = (char*) m_mappedFile.data() + offset;
    char* dataPtr = itemPtr + sizeof(Item);
    Item* item = (Item*) itemPtr;

    item->signature = allocatedMark;
    item->size = (uint32_t) bytes;

    if (data != nullptr)
        memcpy(dataPtr, data, bytes);

    return handle;
}

void MemoryBucket::free(Handle& data)
{
    lock_guard<mutex> lock(m_mutex);

    auto* item = (Item*) data.m_record;
    if (item->signature != allocatedMark) {
        if (item->signature == releasedMark)
            return;
        throw invalid_argument("Invalid persistent address");
    }

    auto offset = uint32_t( (char*) data.m_record - (char*) m_mappedFile.data() );
    m_freeBlocks.free(offset, item->size);

    item->signature = releasedMark;
}

void MemoryBucket::clear()
{
    lock_guard<mutex> lock(m_mutex);
    auto* itemPtr = (char*)m_mappedFile.data();
    memset(itemPtr, 0, m_mappedFile.size());
    m_freeBlocks.clear();
    m_freeBlocks.load(0, (uint32_t) m_mappedFile.size());
}

bool MemoryBucket::empty() const
{
    return m_freeBlocks.available() == m_mappedFile.size();
}

size_t MemoryBucket::size() const
{
    lock_guard<mutex> lock(m_mutex);
    return m_mappedFile.size();
}

size_t MemoryBucket::available() const
{
    lock_guard<mutex> lock(m_mutex);
    uint32_t available = m_freeBlocks.available();
    if (available <= sizeof(Item))
        return 0;
    return available - sizeof(Item);
}

const uint32_t MemoryBucket::id() const
{
    return m_id;
}

const void* MemoryBucket::data() const
{
    return m_mappedFile.data();
}

static mutex                        bucketMapMutex;
static map<uint32_t,MemoryBucket*>  bucketMap;

MemoryBucket* MemoryBucket::find(uint32_t bucketId)
{
    lock_guard<mutex> lock(bucketMapMutex);
    auto itor = bucketMap.find(bucketId);
    if (itor == bucketMap.end())
        return nullptr;
    return itor->second;
}

MemoryBucket::FreeBlocks::FreeBlocks(uint32_t size)
: m_size(size)
{
}

static void replaceKeyAndValue(multimap<uint32_t,uint32_t>& map,
                               uint32_t oldKey, uint32_t oldValue,
                               uint32_t newKey, uint32_t newValue)
{
    auto itor = map.find(oldKey);
    while (itor != map.end()) {
        if (itor->second == oldValue) {
            map.erase(itor);
            if (newKey != 0)
                map.insert(pair<uint32_t, uint32_t>(newKey, newValue));
            return;
        }
        itor++;
    }
}

void MemoryBucket::FreeBlocks::free(uint32_t offset, uint32_t size)
{
    uint32_t fullItemSize = size + sizeof(Item);
    // Find nearest free block down
    auto nextBlockItor = m_offsetMap.upper_bound(offset);
    auto priorBlockItor = nextBlockItor;
    if (priorBlockItor != m_offsetMap.begin() && priorBlockItor != m_offsetMap.end())
        priorBlockItor--;
    else
        priorBlockItor = m_offsetMap.end(); // No prior block

    uint32_t priorBlockOffset {0};
    uint32_t priorBlockSize {0};

    // Can we join to prior free block?
    if (priorBlockItor != m_offsetMap.end()) {
        uint32_t endOfPriorBlock = priorBlockItor->first + priorBlockItor->second;
        if (offset != endOfPriorBlock)
            priorBlockItor = m_offsetMap.end();
        else {
            priorBlockOffset = priorBlockItor->first;
            priorBlockSize = priorBlockItor->second;
        }
    }

    if (nextBlockItor != m_offsetMap.end()) {
        // Can we join to the next free block?
        uint32_t endOfBlock = offset + fullItemSize;
        if (endOfBlock != nextBlockItor->first)
            nextBlockItor = m_offsetMap.end();
    }

    if (priorBlockItor != m_offsetMap.end()) {
        if (nextBlockItor == m_offsetMap.end()) {
            // Join new block to prior block
            priorBlockItor->second += fullItemSize;
            replaceKeyAndValue(m_sizeMap, priorBlockSize, priorBlockOffset, priorBlockItor->second, priorBlockItor->first);
            return;
        } else {
            // Join prior, new, and next free blocks together
            priorBlockItor->second += nextBlockItor->second + fullItemSize;
            replaceKeyAndValue(m_sizeMap, nextBlockItor->second, nextBlockItor->first, 0, 0);
            replaceKeyAndValue(m_sizeMap, priorBlockSize, priorBlockOffset, priorBlockItor->second, priorBlockItor->first);
            m_offsetMap.erase(nextBlockItor);
            return;
        }
    }
    else if (nextBlockItor != m_offsetMap.end()) {
        // Join new block with the next free block
        uint32_t nextBlockOffset = nextBlockItor->first;
        uint32_t nextBlockSize = nextBlockItor->second;
        uint32_t newBlockSize = nextBlockSize + fullItemSize;
        nextBlockItor->second = newBlockSize;
        replaceKeyAndValue(m_sizeMap, nextBlockSize, nextBlockOffset, newBlockSize, offset);
        m_offsetMap.erase(nextBlockItor);
        m_offsetMap[offset] = newBlockSize;
        return;
    }
    m_offsetMap[offset] = fullItemSize;
    m_sizeMap.insert(pair<uint32_t,uint32_t>(fullItemSize, offset));
}

uint32_t MemoryBucket::FreeBlocks::alloc(uint32_t size)
{
    // Find smallest memory block to use
    auto itor = m_sizeMap.lower_bound(size);
    if (itor == m_sizeMap.end())
        return UINT32_MAX;

    uint32_t blockSize = itor->first;
    uint32_t blockOffset = itor->second;

    uint32_t newBlockSize = blockSize - size;
    uint32_t newBlockOffset = blockOffset + size;

    m_sizeMap.erase(itor);
    m_offsetMap.erase(blockOffset);
    if (newBlockSize > 0) {
        m_sizeMap.insert(pair<uint32_t, uint32_t>(newBlockSize, newBlockOffset));
        m_offsetMap.insert(pair<uint32_t, uint32_t>(newBlockOffset, newBlockSize));
    }

    return blockOffset;
}

void MemoryBucket::FreeBlocks::load(uint32_t offset, uint32_t size)
{
    auto rc = m_offsetMap.insert(pair<uint32_t, uint32_t>(offset, size));
    if (rc.second)
        m_sizeMap.insert(pair<uint32_t, uint32_t>(size, offset));
}

void MemoryBucket::FreeBlocks::clear()
{
    m_offsetMap.clear();
    m_sizeMap.clear();
}

size_t MemoryBucket::FreeBlocks::count() const
{
    return m_offsetMap.size();
}

uint32_t MemoryBucket::FreeBlocks::available() const
{
    if (m_offsetMap.empty())
        return 0;

    uint32_t available = m_sizeMap.rbegin()->first;

    return available;
}

void MemoryBucket::FreeBlocks::print() const
{
    cout << "Size map:" << endl;
    for (auto itor: m_sizeMap)
        cout << left << setw(5) << itor.first << "| " << setw(8) << itor.second << endl;

    cout << endl << "Offset map:" << endl;
    for (auto itor: m_offsetMap)
        cout << left << setw(5) << itor.first << "| " << setw(8) << itor.second << endl;
}

#if USE_GTEST

constexpr size_t storageHandleSize = 2 * sizeof(uint32_t);
static const char* testData = "Test Data ";

#ifdef _WIN32
static const char* testBucketDirectory = "C:/Windows/temp/mmf_test";
#else
static const char* testBucketDirectory = "/tmp/mmf_test";
#endif

static void prepareTestDirectory()
{
    if (access(testBucketDirectory, 0) != 0)
        mkdir(testBucketDirectory, 0777);
}

static size_t populateBucket(MemoryBucket& bucket, size_t count, vector<Handle>& handles)
{
    size_t totalStoredSize = 0;
    for (size_t i = 0; i < count; i++) {
        String testStr(testData);
        testStr += to_string(i);
        Handle handle = bucket.insert((void*) testStr.c_str(), testStr.size());
        handles.push_back(handle);
        totalStoredSize += testStr.size() + storageHandleSize;
    }
    return totalStoredSize;
}

TEST(SMQ_MemoryBucket, allocAndClear)
{
    prepareTestDirectory();

    auto bucket = make_shared<MemoryBucket>(testBucketDirectory, "bucket", 1, 128 * 1024);
    bucket->clear();

    EXPECT_EQ(bucket->available(), bucket->size() - storageHandleSize);

    // Populate test data
    vector<Handle> handles;
    size_t totalStoredSize = populateBucket(*bucket, 10, handles);

    EXPECT_EQ(bucket->available(), bucket->size() - totalStoredSize - storageHandleSize);

    bucket->clear();

    EXPECT_EQ(bucket->available(), bucket->size() - storageHandleSize);
}

TEST(SMQ_MemoryBucket, allocAndRead)
{
    prepareTestDirectory();

    auto bucket = make_shared<MemoryBucket>(testBucketDirectory, "bucket", 1, 128 * 1024);
    bucket->clear();

    // Populate test data
    vector<Handle> handles;
    populateBucket(*bucket, 10, handles);

    bucket.reset(); // Destroy the bucket object: doesn't destroy file on disk

    // Re-open the bucket
    bucket = make_shared<MemoryBucket>(testBucketDirectory, "bucket", 1, 128 * 1024);

    // Check the stored data, using handles from prior open
    size_t i = 0;
    for (auto& handle: handles) {
        String testStr(testData);
        testStr += to_string(i);

        String storedStr(handle.c_str(), handle.size());
        EXPECT_STREQ(testStr.c_str(), storedStr.c_str());
        i++;
    }
}

TEST(SMQ_MemoryBucket, free)
{
    prepareTestDirectory();

    auto bucket = make_shared<MemoryBucket>(testBucketDirectory, "bucket", 1, 16 * 1024);
    bucket->clear();

    EXPECT_EQ(bucket->available(), bucket->size() - storageHandleSize);

    // Populate test data
    vector<Handle> handles;
    populateBucket(*bucket, 10, handles);

    // Free every allocated string
    for (int i = 9; i >= 0; i--) {
        auto& handle = handles[i];
        bucket->free(handle);
    }

    bucket->freeBlocks().print();

    EXPECT_EQ(bucket->available(), bucket->size() - storageHandleSize);
}

TEST(SMQ_MemoryBucket, performance)
{
    prepareTestDirectory();

    auto bucket = make_shared<MemoryBucket>(testBucketDirectory, "bucket", 1, 32 * 1024 * 1024);
    bucket->clear();

    EXPECT_EQ(bucket->available(), bucket->size() - storageHandleSize);

    vector<Handle>  handles;
    size_t          count = 256 * 1024;

    DateTime started("now");
    populateBucket(*bucket, count, handles);
    DateTime ended("now");

    double durationSec = duration_cast<microseconds>(ended - started).count() / 1E6;
    cout << "Allocated " << count << " items for " << durationSec << " sec, " << (int) count / durationSec / 1000 << "K op/sec" << endl;

    started = DateTime::Now();
    // Free every odd allocated string
    size_t i = 0;
    for (auto& handle: handles) {
        if (i % 2 == 1) {
            bucket->free(handle);
        }
        i++;
    }
    // Free every even allocated string
    for (auto& handle: handles) {
        if (i % 2 == 0) {
            bucket->free(handle);
        }
        i++;
    }
    ended = DateTime::Now();

    durationSec = duration_cast<microseconds>(ended - started).count() / 1E6;
    cout << "Deallocated " << count << " items for " << durationSec << " sec, " << (int) count / durationSec / 1000 << "K op/sec" << endl;
}

#endif
