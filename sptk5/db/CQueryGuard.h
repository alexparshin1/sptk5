/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CQueryGuard.h  -  description
                             -------------------
    begin                : Fir Jun 9 2006
    copyright            : (C) 1999-2014 by Alexey Parshin. All rights reserved.
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

#ifndef __CQUERYGARD_H__
#define __CQUERYGARD_H__

#include <sptk5/cdatabase>

namespace sptk {

    /// @addtogroup Database Database Support
    /// @{

    /// @brief Service class to use with CQuery
    ///
    /// Manages the external CQuery object to make sure that it's automatically closed when it leaves the scope.

    class CQueryGuard {
        sptk::CQuery& m_query; ///< Wrapped query object
    public:
        /// @brief Constructor
        /// @param query sptk::CQuery, query to manage

        CQueryGuard(sptk::CQuery& query) : m_query(query) {
        }

        /// @brief Destructor

        ~CQueryGuard() {
            m_query.close();
        }

        /// @brief Opens managed query object

        void open() {
            m_query.open();
        }

        /// @brief Executes managed query object

        void exec() {
            m_query.exec();
        }

        /// @brief Fetches next data row from the managed query object

        void fetch() {
            m_query.fetch();
        }

        /// @brief Returns true if there is no more data rows to read

        bool eof() const {
            return m_query.eof();
        }

        /// @brief Allows to set query parameter by name
        /// @param paramName const char*, parameter name

        sptk::CParam& param(const char* paramName) {
            return m_query.param(paramName);
        }

        /// @brief Allows to set query parameter by parameter index
        /// @param paramIndex uint32_t, parameter index

        sptk::CParam& param(uint32_t paramIndex) {
            return m_query.param(paramIndex);
        }

        /// @brief Returns query field by the field name
        /// @param fieldName const char*, field name

        sptk::CField & operator [] (const char* fieldName) {
            return m_query[fieldName];
        }

        /// @brief Returns query field by field index
        /// @param fieldIndex uint32_t, field index

        sptk::CField & operator [] (uint32_t fieldIndex) {
            return m_query[fieldIndex];
        }

        /// @brief Returns managed query object

        operator sptk::CQuery& () {
            return m_query;
        }
    };

    /// @}
}

#endif
