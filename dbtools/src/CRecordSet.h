/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CRecordSet.h  -  description
                             -------------------
    begin                : Wed Feb 22, 2006
    copyright            : (C) 2006-2008 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
  Redistribution and use in source and binary forms, with or without 
  modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, 
    this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, 
    this list of conditions and the following disclaimer in the documentation 
    and/or other materials provided with the distribution.
  * Neither the name of the <ORGANIZATION> nor the names of its contributors 
    may be used to endorse or promote products derived from this software 
    without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER 
  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ***************************************************************************/
#ifndef __CRECORDSET_H__
#define __CRECORDSET_H__

#include <sptk3/database>

namespace sptk {
/// @brief Minimal class to return a record-specific query
template <class T>
class CRecordSet {
   /// Managed query object
   CQuery& m_query;
public:
   /// @brief Constructor
   ///
   /// @param query CQuery&, query object 
   CRecordSet(CQuery& query) : m_query(query) {}

   /// @brief Copy constructor
   ///
   /// @param rs const CRecorSet&, record set to copy
   CRecordSet(const CRecordSet& rs) : m_query(rs.m_query) {}
   
   /// @brief Defines internal query parameter
   ///
   /// @param paramName std::string, parameter name
   CParam& param(std::string paramName);
};

};

#endif
