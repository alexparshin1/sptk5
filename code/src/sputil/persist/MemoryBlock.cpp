#include "sptk5/persist/MemoryBlock.h"
#include <vector>

#ifndef _WIN32
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#endif

using namespace std;
using namespace sptk;
using namespace persist;

size_t MemoryBlock::m_allocationUnit {0};

MemoryBlock::MemoryBlock(const std::string &fileName, size_t fileSize)
: m_fileName(fileName),
  m_fileSize(fileSize)
{
#ifdef _WIN32
    if (m_allocationUnit == 0) {
        SYSTEM_INFO SysInfo;
        GetSystemInfo(&SysInfo);
        m_allocationUnit = SysInfo.dwAllocationGranularity;
    }
#else
    if (m_allocationUnit == 0)
        m_allocationUnit = 65536;
#endif
}

MemoryBlock::~MemoryBlock()
{
    close();
}

void MemoryBlock::open()
{
    createOrOpenFile();
    createFileMapping();
}

void MemoryBlock::close()
{
#ifdef _WIN32    
    if (m_data != nullptr) {
        UnmapViewOfFile(m_data);
        m_data = nullptr;
    }

    if (m_mapFile != INVALID_HANDLE_VALUE) {
        CloseHandle(m_mapFile);
        m_mapFile = INVALID_HANDLE_VALUE;
    }

    if (m_file != INVALID_HANDLE_VALUE) {
        CloseHandle(m_file);
        m_file = INVALID_HANDLE_VALUE;
    }
#else
    if (m_data != nullptr)
        munmap(m_data, m_fileSize);
    ::close(m_file);
#endif
}

void MemoryBlock::createOrOpenFile()
{
#ifdef _WIN32
    // Create or open the file
    m_file = CreateFile(m_fileName.c_str(),
                       GENERIC_READ | GENERIC_WRITE,
                       0,
                       NULL,
                       OPEN_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
#else
    m_file = ::open(m_fileName.c_str(), O_RDWR | O_CREAT, (mode_t)0600);
#endif

    if (m_file == INVALID_HANDLE_VALUE)
        throw runtime_error("Can't create or open file " + m_fileName);

#ifdef _WIN32
    uint64_t fileSize = GetFileSize(m_file,  NULL);
    if (fileSize != 0) {
        m_fileSize = fileSize;  // Using size of existing file
    } else {
        vector<char> buffer(m_allocationUnit);
        DWORD dBytesWritten;
        while (fileSize < m_fileSize) {
            if (WriteFile(m_file, buffer.data(), (DWORD) m_allocationUnit, &dBytesWritten, NULL) == 0)
                throw runtime_error("Can't write to file " + m_fileName);
            fileSize += m_allocationUnit;
        }
        m_fileSize = fileSize;
    }
#else
    struct stat st {};
    fstat(m_file, &st);
    uint64_t fileSize = st.st_size;
    if (fileSize != 0) {
        m_fileSize = fileSize;  // Using size of existing file
    } else {
        vector<char> buffer(m_allocationUnit);
        while (fileSize < m_fileSize) {
            if (write(m_file, buffer.data(), m_allocationUnit) != (int) m_allocationUnit)
                throw runtime_error("Can't write to file " + m_fileName);
            fileSize += m_allocationUnit;
        }
        m_fileSize = fileSize;
    }
#endif
}

void MemoryBlock::createFileMapping()
{
#ifdef _WIN32
    // Create a file mapping object for the file
    // Note that it is a good idea to ensure the file size is not zero
    m_mapFile = CreateFileMapping(m_file,             // current file handle
                                  NULL,               // default security
                                  PAGE_READWRITE,     // read/write permission
                                  0,                  // size of mapping object, high
                                  (DWORD) m_fileSize, // size of mapping object, low
                                  NULL);              // name of mapping object

    if (m_mapFile == nullptr)
        throw runtime_error("Can't create file mapping");

    m_data = MapViewOfFile(m_mapFile,                 // handle to mapping object
                           FILE_MAP_ALL_ACCESS,       // read/write
                           0,                         // high-order 32 bits of file offset
                           0,                         // low-order 32 bits of file offset
                           (DWORD) m_fileSize);       // number of bytes to map
    if (m_data == nullptr)
        throw runtime_error("Can't create file mapping");
#else
    /* Now the file is ready to be mmapped.
     */
    m_data = mmap(0, m_fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, m_file, 0);
    if (m_data == MAP_FAILED) {
        m_data = nullptr;
        throw runtime_error("Can't create file mapping");
    }
#endif
}
