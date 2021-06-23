/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/cutils>
#include <sptk5/db/FirebirdConnection.h>
#include <sptk5/db/FirebirdStatement.h>
#include <sptk5/db/Query.h>

using namespace std;
using namespace sptk;

FirebirdConnection::FirebirdConnection(const String& connectionString)
    : PoolDatabaseConnection(connectionString, DatabaseConnectionType::FIREBIRD)
{
}

FirebirdConnection::~FirebirdConnection()
{
    try
    {
        if (getInTransaction() && FirebirdConnection::active())
        {
            rollbackTransaction();
        }
        disconnectAllQueries();
        close();
    }
    catch (const Exception& e)
    {
        CERR(e.what() << endl)
    }
}

void FirebirdConnection::checkStatus(const ISC_STATUS* status_vector, const char* file, int line)
{
    if (status_vector[0] == 1 && status_vector[1])
    {
        string errors;
        array<char, 256> error;
        const ISC_STATUS* pvector = status_vector;
        while (fb_interpret(error.data(), sizeof(error), &pvector))
        {
            errors += string(error.data()) + " ";
        }
        m_lastStatus = errors;
        throw DatabaseException(errors, file, line);
    }
    else
    {
        m_lastStatus.clear();
    }
}

void FirebirdConnection::_openDatabase(const String& newConnectionString)
{
    array<ISC_STATUS, 20> status_vector;

    if (!active())
    {
        setInTransaction(false);
        if (newConnectionString.length())
        {
            connectionString(DatabaseConnectionString(newConnectionString));
        }

        array<char, 256> dpb_buffer;
        char* dpb;
        short dpb_length;

        dpb = dpb_buffer.data();
        *dpb++ = isc_dpb_version1;
        *dpb++ = isc_dpb_num_buffers;
        *dpb++ = 1;
        *dpb++ = 90;
        dpb_length = short(dpb - dpb_buffer.data());

        dpb = dpb_buffer.data();

        const DatabaseConnectionString& connString = connectionString();

        if (const string& username = connString.userName(); !username.empty())
        {
            isc_modify_dpb(&dpb, &dpb_length, isc_dpb_user_name, username.c_str(), (short) username.length());
        }

        if (const string& password = connString.password(); !password.empty())
        {
            isc_modify_dpb(&dpb, &dpb_length, isc_dpb_password, password.c_str(), (short) password.length());
        }

        m_connection = 0;
        string fullDatabaseName = connString.hostName() + ":/" + connString.databaseName();
        isc_attach_database(status_vector.data(), (short) fullDatabaseName.length(), fullDatabaseName.c_str(),
                            &m_connection, dpb_length, dpb);
        checkStatus(status_vector.data(), __FILE__, __LINE__);
    }
}

void FirebirdConnection::closeDatabase()
{
    array<ISC_STATUS, 20> status_vector;

    if (m_transaction)
    {
        driverEndTransaction(false);
    }

    disconnectAllQueries();

    if (m_connection)
    {
        isc_detach_database(status_vector.data(), &m_connection);
        m_connection = 0;
    }
}

PoolDatabaseConnection::DBHandle FirebirdConnection::handle() const
{
    union
    {
        isc_db_handle connection;
        void* handle;
    } convert = {};
    convert.connection = m_connection;
    return (PoolDatabaseConnection::DBHandle) convert.handle;
}

bool FirebirdConnection::active() const
{
    return m_connection != 0L;
}

String FirebirdConnection::nativeConnectionString() const
{
    // Connection string in format: host[:port][/instance]
    stringstream connectionString;
    const DatabaseConnectionString& connString = PoolDatabaseConnection::connectionString();
    connectionString << connString.hostName();
    if (connString.portNumber())
    {
        connectionString << ":" << int2string(connString.portNumber());
    }
    if (!connString.databaseName().empty())
    {
        connectionString << "/" << connString.databaseName();
    }
    return connectionString.str();
}

void FirebirdConnection::driverBeginTransaction()
{
    array<ISC_STATUS, 20> status_vector;

    if (!m_connection)
    {
        open();
    }

    if (getInTransaction())
    {
        driverEndTransaction(true);
    }

    m_transaction = 0L;
    static array<char, 5> isc_tpb = {isc_tpb_version3, isc_tpb_write, isc_tpb_read_committed, isc_tpb_no_rec_version,
                             isc_tpb_wait};
    isc_start_transaction(status_vector.data(), &m_transaction, 1, &m_connection, sizeof(isc_tpb), isc_tpb);
    checkStatus(status_vector.data(), __FILE__, __LINE__);

    setInTransaction(true);
}

void FirebirdConnection::driverEndTransaction(bool commit)
{
    array<ISC_STATUS, 20> status_vector;

    if (!getInTransaction())
    throwDatabaseException("Transaction isn't started.")

    if (commit)
    {
        isc_commit_transaction(status_vector.data(), &m_transaction);
    }
    else
    {
        isc_rollback_transaction(status_vector.data(), &m_transaction);
    }

    checkStatus(status_vector.data(), __FILE__, __LINE__);
    m_transaction = 0L;
    setInTransaction(false);
}

//-----------------------------------------------------------------------------------------------
String FirebirdConnection::queryError(const Query*) const
{
    return m_lastStatus;
}

void FirebirdConnection::queryAllocStmt(Query* query)
{
    queryFreeStmt(query);
    querySetStmt(query, new FirebirdStatement(this, query->sql()));
}

void FirebirdConnection::queryFreeStmt(Query* query)
{
    scoped_lock lock(m_mutex);
    auto* statement = (FirebirdStatement*) query->statement();
    if (statement)
    {
        delete statement;
        querySetStmt(query, nullptr);
        querySetPrepared(query, false);
    }
}

void FirebirdConnection::queryCloseStmt(Query* query)
{
    scoped_lock lock(m_mutex);
    try
    {
        auto* statement = (FirebirdStatement*) query->statement();
        if (statement)
        {
            statement->close();
        }
    }
    catch (const Exception& e)
    {
        throwDatabaseException(e.what())
    }
}

void FirebirdConnection::queryPrepare(Query* query)
{
    scoped_lock lock(m_mutex);

    if (!query->prepared())
    {
        auto* statement = (FirebirdStatement*) query->statement();
        try
        {
            statement->prepare(query->sql());
            statement->enumerateParams(query->params());
        }
        catch (const Exception& e)
        {
            throwDatabaseException(e.what())
        }
        querySetPrepared(query, true);
    }
}

void FirebirdConnection::queryUnprepare(Query* query)
{
    queryFreeStmt(query);
}

int FirebirdConnection::queryColCount(Query* query)
{
    int colCount = 0;
    const auto* statement = (FirebirdStatement*) query->statement();
    try
    {
        colCount = (int) statement->colCount();
    }
    catch (const Exception& e)
    {
        throwDatabaseException(e.what())
    }
    return colCount;
}

void FirebirdConnection::queryBindParameters(Query* query)
{
    scoped_lock lock(m_mutex);

    auto* statement = (FirebirdStatement*) query->statement();
    try
    {
        if (!statement)
        throwDatabaseException("Query not prepared")
        statement->setParameterValues();
    }
    catch (const Exception& e)
    {
        throwDatabaseException(e.what())
    }
}

void FirebirdConnection::queryExecute(Query* query)
{
    auto* statement = (FirebirdStatement*) query->statement();
    try
    {
        if (!statement)
        throwDatabaseException("Query is not prepared")
        statement->execute(getInTransaction());
    }
    catch (const Exception& e)
    {
        throwDatabaseException(e.what())
    }
}

void FirebirdConnection::queryOpen(Query* query)
{
    if (!active())
    {
        open();
    }

    if (query->active())
    {
        return;
    }

    if (!query->statement())
    {
        queryAllocStmt(query);
    }

    if (!query->prepared())
    {
        queryPrepare(query);
    }

    // Bind parameters also executes a query
    queryBindParameters(query);

    auto* statement = (FirebirdStatement*) query->statement();

    queryExecute(query);
    if (auto fieldCount = (short) queryColCount(query); fieldCount < 1)
    {
        return;
    }
    else
    {
        querySetActive(query, true);
        if (query->fieldCount() == 0)
        {
            scoped_lock lock(m_mutex);
            statement->bindResult(query->fields());
        }
    }

    querySetEof(query, statement->eof());

    queryFetch(query);
}

void FirebirdConnection::queryFetch(Query* query)
{
    if (!query->active())
    throwDatabaseException("Dataset isn't open")

    scoped_lock lock(m_mutex);

    try
    {
        auto* statement = (FirebirdStatement*) query->statement();

        statement->fetch();

        if (statement->eof())
        {
            querySetEof(query, true);
            return;
        }

        statement->fetchResult(query->fields());
    }
    catch (const Exception& e)
    {
        throwDatabaseException(e.what())
    }
}

void FirebirdConnection::objectList(DatabaseObjectType objectType, Strings& objects)
{
    string objectsSQL;
    objects.clear();
    switch (objectType)
    {
        case DatabaseObjectType::PROCEDURES:
            objectsSQL =
                "SELECT rdb$procedure_name object_name "
                "FROM rdb$procedures "
                "ORDER BY 1";
            break;
        case DatabaseObjectType::FUNCTIONS:
            objectsSQL =
                "SELECT rdb$function_name object_name "
                "FROM rdb$functions "
                "ORDER BY 1";
            break;
        case DatabaseObjectType::TABLES:
            objectsSQL =
                "SELECT rdb$relation_name object_name "
                "FROM rdb$relations "
                "WHERE rdb$system_flag = 0 AND rdb$view_source IS NULL "
                "ORDER BY 1";
            break;
        case DatabaseObjectType::VIEWS:
            objectsSQL =
                "SELECT rdb$relation_name object_name "
                "FROM rdb$relations "
                "WHERE rdb$system_flag = 0 AND rdb$view_source IS NOT NULL "
                "ORDER BY 1";
            break;
        default:
        throwDatabaseException("Not supported")
    }
    Query query(this, objectsSQL);
    query.open();
    while (!query.eof())
    {
        objects.push_back(query[uint32_t(0)].asString());
        query.next();
    }
    query.close();
}

String FirebirdConnection::driverDescription() const
{
    return "Firebird";
}

String FirebirdConnection::paramMark(unsigned)
{
    return "?";
}

void* firebird_create_connection(const char* connectionString)
{
    auto* connection = new FirebirdConnection(connectionString);
    return connection;
}

void firebird_destroy_connection(void* connection)
{
    delete (FirebirdConnection*) connection;
}
