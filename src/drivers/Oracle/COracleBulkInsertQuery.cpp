#include "COracleBulkInsertQuery.h"

using namespace std;
using namespace sptk;

COracleBulkInsertQuery::COracleBulkInsertQuery(CDatabaseConnection *db, std::string sql, size_t recordCount, const CColumnTypeSizeMap& columnTypeSizes)
: CQuery(db, sql), m_recordCount(recordCount), m_recordNumber(0), m_batchSize(2), m_lastIteration(false), m_columnTypeSizes(columnTypeSizes)
{
    m_bulkMode = true;
}

COracleBulkInsertQuery::~COracleBulkInsertQuery()
{
}

void COracleBulkInsertQuery::execNext() THROWS_EXCEPTIONS
{
    m_recordNumber++;
    m_lastIteration = (m_recordNumber == m_recordCount || (m_recordNumber % m_batchSize) == 0);
    exec();
}
