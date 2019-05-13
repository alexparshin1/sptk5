#include "sptk5/persist/PersistentMemoryBlock.h"
#include <vector>

using namespace std;
using namespace sptk;

size_t PersistentMemoryBlock::m_allocationUnit {0};

PersistentMemoryBlock::PersistentMemoryBlock(const std::string &fileName, size_t fileSize)
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

PersistentMemoryBlock::~PersistentMemoryBlock()
{
    close();
}

void PersistentMemoryBlock::open()
{
    createOrOpenFile();
    createFileMapping();
}

void PersistentMemoryBlock::close()
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
#endif    
}

void PersistentMemoryBlock::createOrOpenFile()
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

    if (m_file == INVALID_HANDLE_VALUE)
        throw runtime_error("Can't create or open file " + m_fileName);

    uint64_t fileSize = GetFileSize(m_file,  NULL);
    if (fileSize != 0) {
        m_fileSize = fileSize;  // Using size of existing file
    } else {
        vector<char> buffer(m_allocationUnit, 1);
        DWORD dBytesWritten;
        while (fileSize < m_fileSize) {
            if (WriteFile(m_file, buffer.data(), (DWORD) m_allocationUnit, &dBytesWritten, NULL) == 0)
                throw runtime_error("Can't write to file " + m_fileName);
            fileSize += m_allocationUnit;
        }
        m_fileSize = fileSize;
    }
#endif
}

void PersistentMemoryBlock::createFileMapping()
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
#endif
}
