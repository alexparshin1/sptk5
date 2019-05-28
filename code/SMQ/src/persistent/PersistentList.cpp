/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       PersistentList.cpp - description                       ║
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

#include <smq/persistent/PersistentList.h>
#include <sptk5/StopWatch.h>

using namespace std;
using namespace sptk;
using namespace smq;
using namespace persistent;
using namespace chrono;

PersistentList::ItemHandle::operator void*() const
{
    auto* item = (ItemStorage*) header();
    if (item->signature != allocatedMark)
        return nullptr;
    return (void*)((const char*) item->data);
}

void* PersistentList::ItemHandle::data() const
{
    auto* item = (ItemStorage*) header();
    if (item->signature != allocatedMark)
        return nullptr;
    return (void*)((const char*) item->data);
}

const char* PersistentList::ItemHandle::c_str() const
{
    auto* item = (ItemStorage*) header();
    if (item->signature != allocatedMark)
        return nullptr;
    return (const char*) item->data;
}

size_t PersistentList::ItemHandle::size() const
{
    auto* item = (ItemStorage*) header();
    return item->dataLength;
}

PersistentList::PersistentList(MemoryPool& pool, const String& name)
: m_pool(pool), m_name(name)
{
    size_t headerLength = sizeof(ItemStorage) + name.length();
    m_header = m_pool.insert(nullptr, headerLength);

    auto* headerPtr = header();
    headerPtr->type = HT_LIST_HEADER;
    strcpy((char*)headerPtr->data, name.c_str());
    headerPtr->dataLength = name.length();
}

PersistentList::PersistentList(MemoryPool& pool, Handle& header)
: m_pool(pool), m_header(header)
{
    auto* headerPtr = this->header();
    if (headerPtr->type != HT_LIST_HEADER)
        throw Exception("Handle is not a list header");
    m_name = String((const char*)headerPtr->data, headerPtr->dataLength);
}

Handle PersistentList::push_front(const void* data, size_t size)
{
    sptk::UniqueLock(m_mutex);

    size_t itemLength = sizeof(ItemStorage) + size - 1;
    auto itemHandle = ItemHandle(m_pool.insert(nullptr, itemLength));

    auto* item = this->item(itemHandle);

    auto* header = this->header();
    header->next = itemHandle.location();

    if (!m_handles.empty()) {
        Handle& front = m_handles.front();
        item->next = front.location();
    }

    item->dataLength = size;
    memcpy(item->data, data, size);

    m_handles.push_front(itemHandle);

    return m_handles.front();
}

Handle PersistentList::push_back(const void* data, size_t size)
{
    sptk::UniqueLock(m_mutex);

    size_t itemLength = sizeof(ItemStorage) + size - 1;
    auto itemHandle = ItemHandle(m_pool.insert(nullptr, itemLength));

    auto* item = this->item(itemHandle);

    item->type = HT_LIST_ITEM;
    item->next = Location();
    if (m_handles.empty()) {
        auto* header = this->header();
        header->next = itemHandle.location();
    } else {
        Handle& back = m_handles.back();
        ItemStorage* backItem = this->item(back);
        backItem->next = itemHandle.location();
    }

    item->dataLength = size;
    memcpy(item->data, data, size);

    m_handles.push_back(itemHandle);

    return m_handles.back();
}


void PersistentList::erase(iterator from, iterator to)
{
    sptk::UniqueLock(m_mutex);
    for(auto& itor = from; itor != to; itor++) {
        auto& handle = *itor;
        m_pool.free(handle);
    }

    auto& prior = from;
    ItemStorage* priorItem;
    if (from != m_handles.begin()) {
        prior--;
        priorItem = item(*prior);
    } else {
        prior = m_handles.end();
        priorItem = nullptr;
    }

    m_handles.erase(from, to);
    auto& next = to;
    ItemStorage* nextItem;
    if (next != m_handles.end())
        nextItem = item(*next);
    else
        nextItem = nullptr;

    if (priorItem != nullptr) {
        if (nextItem != nullptr)
            priorItem->next = next->location();
        else
            priorItem->next = Location();
    }
}

void PersistentList::clear()
{
    sptk::UniqueLock(m_mutex);
    for (auto& handle: m_handles)
        m_pool.free(handle);
    m_handles.clear();

    header()->next = Location();
}

void PersistentList::load()
{
    sptk::UniqueLock(m_mutex);

    m_handles.clear();

    auto* headerPtr = header();
    if (headerPtr->next.empty())
        return;

    ItemStorage* itemPtr = nullptr;
    for (Location location = headerPtr->next; !location.empty(); location = itemPtr->next) {
        ItemHandle itemHandle(location);
        m_handles.push_back(itemHandle);
        itemPtr = item(itemHandle);
    }
}


#if USE_GTEST

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

class TestListThread : public Thread
{
    SPersistentList m_list;
    size_t          m_maxItems {1024};

protected:
    void threadFunction() override
    {
        String listItem = m_list->name() + ": long text item long text item long text item long text item";
        for (size_t i = 0; i < m_maxItems; i++)
            m_list->push_back(listItem.c_str(), listItem.length());
    }

public:
    TestListThread(SPersistentList list, size_t maxItems)
    : Thread("Test"), m_list(list), m_maxItems(maxItems)
    {}
};

TEST(SMQ_PersistentList, alloc)
{
    prepareTestDirectory();

    MemoryPool pool(testPoolDirectory, "lists", 16 * 1024 * 1024);
    SHandles listHandles = make_shared<Handles>();
    pool.load(listHandles, HT_LIST_HEADER);

    StopWatch stopWatch;

    stopWatch.start();
    size_t totalItems = 0;
    for (auto& handle: *listHandles) {
        PersistentList list(pool, handle);
        list.load();
        totalItems += list.size();
    }
    stopWatch.stop();
    COUT("Total " << fixed << setprecision(3) << totalItems << " items loaded for "
         << stopWatch.seconds() << " sec, " << totalItems / stopWatch.seconds() / 1E3 << "K/sec" << endl);

    pool.clear();

    vector<shared_ptr<TestListThread>> threads;
    size_t maxItems = 16 * 1024;
    size_t maxThreads = 16;
    maxItems = maxItems / maxThreads * maxThreads;
    for (size_t i = 0; i < maxThreads; i++) {
        auto list = make_shared<PersistentList>(pool, "List " + to_string(i));
        auto thread = make_shared<TestListThread>(list, maxItems / maxThreads);
        threads.push_back(thread);
    }

    stopWatch.start();

    for (auto& thread: threads)
        thread->run();

    for (auto& thread: threads)
        thread->join();

    stopWatch.stop();

    COUT("Total " << fixed << setprecision(3) << totalItems << " items created for "
         << stopWatch.seconds() << " sec, " << totalItems / stopWatch.seconds() / 1E3 << "K/sec" << endl);
}

#endif
