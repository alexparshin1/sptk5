#ifndef __PERSIST_MEMORY_BLOCK_H__
#define __PERSIST_MEMORY_BLOCK_H__

#include <sptk5/cutils>

namespace sptk {
namespace persist {

class MemoryBlock
{
#ifndef _WIN32
    static constexpr int INVALID_HANDLE_VALUE{-1};
#endif

public:
    MemoryBlock(const std::string& fileName, size_t fileSize);

    virtual ~MemoryBlock();

    void open();

    void close();

    uint64_t size() const
    {
        return m_fileSize;
    }

private:

    static size_t m_allocationUnit;

    std::string m_fileName;
    uint64_t m_fileSize{0};

#ifdef _WIN32
    HANDLE              m_file {INVALID_HANDLE_VALUE};          // the file handle
    HANDLE              m_mapFile {INVALID_HANDLE_VALUE};       // handle for the file's memory-mapped region
#else
    int m_file{INVALID_HANDLE_VALUE};          // the file handle
#endif

    void* m_data{nullptr};

    void createOrOpenFile();

    void createFileMapping();

public:
    void* data()
    {
        return m_data;
    }

    const void* data() const
    {
        return m_data;
    }
};

}
}

#endif
