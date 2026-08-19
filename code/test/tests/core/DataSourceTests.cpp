/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <gtest/gtest.h>
#include <sptk5/DataSource.h>
#include <sptk5/FieldList.h>
#include <sptk5/xdoc/Document.h>

using namespace std;
using namespace sptk;
using namespace xdoc;

class TestDataSource final : public DataSource
{
public:
    TestDataSource()
    {
        FieldList row1(false);
        row1.push_back("name", false) = "John";
        row1.push_back("age", false) = 30;
        m_rows.push_back(std::move(row1));

        FieldList row2(false);
        row2.push_back("name", false) = "Jane";
        row2.push_back("age", false) = 28;
        m_rows.push_back(std::move(row2));
    }

    Field& operator[](size_t fieldIndex) override
    {
        return currentRow()[static_cast<int>(fieldIndex)];
    }

    Field& operator[](const String& fieldName) override
    {
        return currentRow()[fieldName];
    }

    size_t fieldCount() const override
    {
        if (m_rows.empty() || m_current >= m_rows.size())
        {
            return 0U;
        }
        return m_rows[m_current].size();
    }

    size_t recordCount() const override
    {
        return m_rows.size();
    }

    bool readField(const char* fieldName, Variant& fieldValue) override
    {
        auto field = currentRow().findField(fieldName);
        if (!field)
        {
            return false;
        }
        fieldValue = *field;
        return true;
    }

    bool writeField(const char* fieldName, const Variant& fieldValue) override
    {
        auto field = currentRow().findField(fieldName);
        if (!field)
        {
            return false;
        }
        *field = fieldValue;
        return true;
    }

    bool open() override
    {
        ++open_calls;
        m_open = true;
        m_current = 0;
        return true;
    }

    bool close() override
    {
        ++close_calls;
        m_open = false;
        return true;
    }

    bool first() override
    {
        if (!m_open || m_rows.empty())
        {
            m_current = m_rows.size();
            return false;
        }
        m_current = 0;
        return true;
    }

    bool next() override
    {
        ++next_calls;
        if (!m_open)
        {
            return false;
        }
        if (m_current < m_rows.size())
        {
            ++m_current;
        }
        return m_current < m_rows.size();
    }

    bool prior() override
    {
        if (!m_open || m_rows.empty() || m_current == 0)
        {
            return false;
        }
        --m_current;
        return true;
    }

    bool last() override
    {
        if (!m_open || m_rows.empty())
        {
            m_current = m_rows.size();
            return false;
        }
        m_current = m_rows.size() - 1;
        return true;
    }

    bool eof() const override
    {
        return !m_open || m_current >= m_rows.size();
    }

protected:
    bool loadData() override
    {
        ++load_calls;
        return true;
    }

    bool saveData() override
    {
        ++save_calls;
        return true;
    }

public:
    int load_calls {0};
    int save_calls {0};
    int open_calls {0};
    int close_calls {0};
    int next_calls {0};

private:
    FieldList& currentRow()
    {
        return m_rows.at(m_current);
    }

    const FieldList& currentRow() const
    {
        return m_rows.at(m_current);
    }

    vector<FieldList> m_rows;
    size_t            m_current {0};
    bool              m_open {false};
};
namespace sptk {

TEST(DataSourceTests,loadSaveDelegates)
{
    TestDataSource ds;

    EXPECT_TRUE(ds.load());
    EXPECT_TRUE(ds.save());

    EXPECT_EQ(ds.load_calls, 1);
    EXPECT_EQ(ds.save_calls, 1);
}

TEST(DataSourceTests,exportRowToCompactXml)
{
    TestDataSource ds;
    ds.open();

    Document document;
    auto     row = document.root()->pushNode("row", Node::Type::Object);
    ds.exportRowTo(row, true);

    EXPECT_TRUE(row->attributes().have("name"));
    EXPECT_STREQ("John", row->attributes().get("name").c_str());
    EXPECT_STREQ("30", row->attributes().get("age").c_str());
    EXPECT_EQ(row->nodes().size(), static_cast<size_t>(0));
}

TEST(DataSourceTests,exportRowToFullXml)
{
    TestDataSource ds;
    ds.open();

    Document document;
    auto     row = document.root()->pushNode("row", Node::Type::Object);
    ds.exportRowTo(row, false);

    EXPECT_FALSE(row->attributes().have("name"));
    EXPECT_STREQ("John", row->getString("name").c_str());
    EXPECT_EQ(static_cast<int>(row->getNumber("age")), 30);
    EXPECT_EQ(row->nodes().size(), static_cast<size_t>(2));
}

TEST(DataSourceTests,exportToIteratesAllRows)
{
    TestDataSource ds;

    Document document;
    ds.exportTo(*document.root(), "row", true);

    EXPECT_EQ(ds.open_calls, 1);
    EXPECT_EQ(ds.close_calls, 1);
    EXPECT_EQ(ds.next_calls, 2);
}

} // namespace sptk_test
