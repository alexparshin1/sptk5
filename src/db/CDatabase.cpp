/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CDatabase.cpp  -  description
                             -------------------
    begin                : Wed Dec 15 1999
    copyright            : (C) 1999-2008 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#include <sptk5/db/CDatabase.h>
#include <sptk5/db/CQuery.h>
#include <sptk5/CException.h>

#include <algorithm>

using namespace std;
using namespace sptk;

CDatabase::CDatabase(string connectionString) :
    m_connString(connectionString)
{
    m_inTransaction = false;
    m_log = 0;
}

CDatabase::~CDatabase() {
   // To prevent the exceptions, if the database connection
   // is terminated already
   try {
      while (m_queryList.size()) {
         CQuery *query = (CQuery *)m_queryList[0];
         query->disconnect();
      }
   }
   catch (...) {
   }
}

bool CDatabase::linkQuery(CQuery *q) {
   m_queryList.push_back(q);
   return true;
}

bool CDatabase::unlinkQuery(CQuery *q) {
   CQueryVector::iterator itor = find(m_queryList.begin(),m_queryList.end(),q);
   m_queryList.erase(itor);
   return true;
}

void CDatabase::openDatabase(string newConnectionString) throw (CException) {
   notImplemented("openDatabase");
}

void CDatabase::open(string newConnectionString) throw (CException)  {
   clearStatistics();
   openDatabase(newConnectionString);
   if (m_log) *m_log << "Opened database: " << m_connString << endl;
}

void CDatabase::closeDatabase() throw(CException) {
   notImplemented("closeDatabase");
}

void CDatabase::close() throw (CException) {
   if (active()) {
      if (m_inTransaction) {
         rollbackTransaction();
         m_inTransaction = false;
      }
      for (uint32_t i = 0; i < m_queryList.size(); i++) {
         CQuery& query = *m_queryList[i];
         query.storeStatistics();
         query.closeQuery(true);
      }
      closeDatabase();
      if (m_log) *m_log << "Closed database: " << m_connString << endl;
   }
}

void* CDatabase::handle() const {
   notImplemented("handle");
   return 0;
}

bool CDatabase::active() const {
   notImplemented("active");
   return true;
}

void CDatabase::beginTransaction() throw(CException) {
   if (m_log) *m_log << "Begin transaction" << endl;
   driverBeginTransaction();
}

void CDatabase::commitTransaction() throw(CException) {
   if (m_log) *m_log << "Commit transaction" << endl;
   driverEndTransaction(true);
}

void CDatabase::rollbackTransaction() throw(CException) {
   if (m_log) *m_log << "Rollback transaction" << endl;
   driverEndTransaction(false);
}

//-----------------------------------------------------------------------------------------------

string CDatabase::queryError(const CQuery *query) const {
   notImplemented("queryError");
   return "";
}

void CDatabase::querySetAutoPrep(CQuery *q,bool pf) {
   q->m_autoPrepare = pf;
}

void CDatabase::querySetStmt(CQuery *q,void *stmt)  {
   q->m_statement = stmt;
}

void CDatabase::querySetConn(CQuery *q,void *conn)  {
   q->m_connection = conn;
}

void CDatabase::querySetPrepared(CQuery *q,bool pf) {
   q->m_prepared = pf;
}

void CDatabase::querySetActive(CQuery *q,bool af)   {
   q->m_active = af;
}

void CDatabase::querySetEof(CQuery *q,bool eof)     {
   q->m_eof = eof;
}

void CDatabase::queryAllocStmt(CQuery *query) {
   notImplemented("queryAllocStmt");
}

void CDatabase::queryFreeStmt(CQuery *query) {
   notImplemented("queryFreeStmt");
}

void CDatabase::queryCloseStmt(CQuery *query) {
   notImplemented("queryCloseStmt");
}

void CDatabase::queryPrepare(CQuery *query) {
   notImplemented("queryPrepare");
}

void CDatabase::queryUnprepare(CQuery *query) {
   queryFreeStmt(query);
}

void CDatabase::queryExecute(CQuery *query) {
   notImplemented("queryExecute");
}

int CDatabase::queryColCount(CQuery *query) {
   notImplemented("queryColCount");
   return 0;
}

void CDatabase::queryColAttributes(CQuery *query,int16_t column,int16_t descType,int32_t& value) {
   notImplemented("queryColAttributes");
}

void CDatabase::queryColAttributes(CQuery *query,int16_t column,int16_t descType,char *buff,int32_t len) {
   notImplemented("queryColAttributes");
}

void CDatabase::queryBindParameters(CQuery *query) {
   notImplemented("queryBindParameters");
}

void CDatabase::queryOpen(CQuery *query) {
   notImplemented("queryOpen");
}

void CDatabase::queryFetch(CQuery *query) {
   notImplemented("queryFetch");
}

void CDatabase::notImplemented(const char *methodName) const {
   throw CException("Method '"+string(methodName)+"' is not supported by this database driver.");
}

void *CDatabase::queryHandle(CQuery *query) const {
   return query->m_statement;
}

void CDatabase::queryHandle(CQuery *query,void *handle) {
   query->m_statement = handle;
}

void CDatabase::logAndThrow(string method,string error) throw(CException) {
   string errorText("Exception in " + method + ": " + error);
   if (m_log) *m_log << "errorText" << endl;
   throw CException(errorText);
}

void CDatabase::clearStatistics() {
   m_queryStatisticMap.clear();
}

void CDatabase::addStatistics(const std::string& location, double totalDuration, unsigned totalCalls,const std::string& sql) {
   CCallStatisticMap::iterator itor = m_queryStatisticMap.find(location);
   if (itor != m_queryStatisticMap.end()) {
      CCallStatistic& stat = itor->second;
      stat.m_duration += totalDuration;
      stat.m_calls += totalCalls;
      stat.m_sql = sql;
   } else {
      CCallStatistic stat(totalDuration, totalCalls, sql);
      m_queryStatisticMap[location] = stat;
   }
}
