#include <sptk5/persist/MemoryPool.h>

using namespace std;
using namespace sptk;
using namespace sptk::persist;

MemoryPool::MemoryPool(const String& directory, const String& objectName)
: m_directory(directory)
{
}
