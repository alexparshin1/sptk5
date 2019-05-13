#ifndef MEMORYMAPPING_MEMORYMAPPEDFILE_H
#define MEMORYMAPPING_MEMORYMAPPEDFILE_H

#include <sptk5/cutils>

namespace sptk {

class PersistentMemoryBlock
{
public:
    PersistentMemoryBlock(const std::string& fileName, size_t fileSize);

    virtual ~PersistentMemoryBlock();

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
    HANDLE              m_mapFile {INVALID_HANDLE_VALUE};      // handle for the file's memory-mapped region
    HANDLE              m_file {INVALID_HANDLE_VALUE};         // the file handle
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

#endif
