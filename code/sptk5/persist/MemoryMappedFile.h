#ifndef __PERSIST_MEMORY_BLOCK_H__
#define __PERSIST_MEMORY_BLOCK_H__

#include <sptk5/cutils>

namespace sptk {
namespace persist {

/**
 * Persistent memory block, associated with memory-mapped file
 */
class MemoryMappedFile
{
#ifndef _WIN32
    static constexpr int INVALID_HANDLE_VALUE {-1};  ///< Invalid handle value
#endif

public:
    /**
     * Constructor
     * @param fileName          Memory mapped file name
     * @param fileSize          Size of memory mapped file and block of memory
     */
    MemoryMappedFile(const std::string& fileName, size_t fileSize);

    /**
     * Destructor
     */
    virtual ~MemoryMappedFile();

    /**
     * Open existing memory mapped file, or create new one
     */
    void open();

    /**
     * Close memory mapped file
     */
    void close();

    /**
     * Size of memory block
     * @return memory block size in bytes
     */
    uint64_t size() const
    {
        return m_fileSize;
    }

private:

    static size_t       m_allocationUnit;                       ///< Allocation unit granularity

    String              m_fileName;                             ///< Memory mapped file name
    uint64_t            m_fileSize {0};                         ///< Memory mapped file size, same as memory block size

#ifdef _WIN32
    HANDLE              m_file {INVALID_HANDLE_VALUE};          ///< Memory mapped file handle
    HANDLE              m_mapFile {INVALID_HANDLE_VALUE};       ///< Handle for the file's memory-mapped region
#else
    int m_file{INVALID_HANDLE_VALUE};                           ///< Memory mapped file handle
#endif

    void*               m_data {nullptr};                       ///< Mapped memory

    /**
     * Open existing file, or create a new one
     */
    void createOrOpenFile();

    /**
     * Map file to memory
     */
    void createFileMapping();

public:

    /**
     * @return Mapped memory
     */
    void* data()
    {
        return m_data;
    }

    /**
     * @return Mapped memory
     */
    const void* data() const
    {
        return m_data;
    }
};

}
}

#endif
