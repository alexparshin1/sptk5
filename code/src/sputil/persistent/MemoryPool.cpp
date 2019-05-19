/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       MemoryPool.cpp - description                           ║
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

#include <sptk5/persistent/MemoryPool.h>
#include <sptk5/DirectoryDS.h>

using namespace std;
using namespace sptk;
using namespace sptk::persistent;

MemoryPool::MemoryPool(const String& directory, const String& objectName, uint32_t bucketSize)
: m_directory(directory), m_bucketSize(bucketSize)
{
    if (access(directory.c_str(), 0) != 0) {
#ifdef _WIN32
        int rc = mkdir(directory.c_str());
#else
        int rc = mkdir(directory.c_str(), 0777);
#endif
        if (rc != 0)
            throw SystemException("Can't create directory '" + directory + "'");
    }
    m_objectName = objectName.replace(R"([\/\\\s~*]+)", "_");
}

SMemoryBucket MemoryPool::createBucketUnlocked(uint32_t id)
{
    if (id == 0)
        id = makeBucketIdUnlocked();

    auto bucket = make_shared<MemoryBucket>(m_directory, m_objectName, id, m_bucketSize);
    m_bucketLoop.add(bucket);
    m_bucketMap[id] = bucket;
    return bucket;
}

SMemoryBucket MemoryPool::createBucket(uint32_t id)
{
    lock_guard<mutex> lock(m_mutex);
    return createBucketUnlocked(id);
}

void MemoryPool::load(std::vector<Handle>* handles)
{
    DirectoryDS         bucketFiles(m_directory, m_objectName + "_*", DDS_HIDE_DIRECTORIES|DDS_HIDE_DOT_FILES);
    RegularExpression   matchFileName(m_objectName + "_(\\d+)$");
    Strings             matches;

    bucketFiles.open();
    while (!bucketFiles.eof()) {
        String fileName(bucketFiles["Name"].asString());
        if (matchFileName.m(fileName,matches)) {
            auto id = (uint32_t) string2int64(matches[0]);
            auto bucket = createBucket(id);
            bucket->load(handles);
        }
        bucketFiles.next();
    }
}

Handle MemoryPool::insert(const void* data, size_t bytes)
{
    lock_guard<mutex> lock(m_mutex);

    for (size_t i = 0; i < m_bucketLoop.size(); i++) {
        auto bucket = m_bucketLoop.get();
        Handle handle = bucket->insert(data, bytes);
        if (!handle.isNull())
            return handle;
    }

    auto bucket = createBucketUnlocked(0);
    return bucket->insert(data, bytes);
}

uint32_t MemoryPool::makeBucketIdUnlocked()
{
    uint32_t id = 1;
    for (auto& itor: m_bucketMap) {
        if (itor.first != id)
            break;
        id++;
    }
    return id;
}

#if USE_GTEST

constexpr size_t storageHandleSize = 2 * sizeof(uint32_t);
static const char* testData = "Test Data ";

#ifdef _WIN32
static const char* testPoolDirectory = "C:/Windows/temp/mmf_test";
#else
static const char* testPoolDirectory = "/tmp/mmf_test";
#endif

static void prepareTestDirectory()
{
    if (access(testPoolDirectory, 0) != 0)
        mkdir(testPoolDirectory, 0777);
}

static size_t populatePool(MemoryPool& pool, size_t count, vector<Handle>& handles)
{
    size_t totalStoredSize = 0;
    for (size_t i = 0; i < count; i++) {
        String testStr(testData);
        testStr += to_string(i);
        Handle handle = pool.insert((void*) testStr.c_str(), testStr.size());
        handles.push_back(handle);
        totalStoredSize += testStr.size() + storageHandleSize;
    }
    return totalStoredSize;
}

TEST(SPTK_MemoryPool, alloc)
{
    prepareTestDirectory();

    MemoryPool pool(testPoolDirectory, "bucket", 128 * 1024);
    vector<Handle> oldHandles;
    pool.load(&oldHandles);

    // Populate test data
    vector<Handle> handles;
    size_t totalStoredSize = populatePool(pool, 10, handles);
}

#endif
