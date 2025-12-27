/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY Message QUEUE                                   ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 by Alexey Parshin. All rights reserved.    ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
*/

#pragma once

#include "sptk5/db/QueryParameter.h"
#include <ocilib.hpp>

namespace sptk {

class OracleOciParameterBuffer
{
public:
    static constexpr unsigned MaxStringLength = 2048;

    explicit OracleOciParameterBuffer(VariantDataType type, const std::shared_ptr<ocilib::Connection>& connection);
    OracleOciParameterBuffer(const OracleOciParameterBuffer&) = delete;
    OracleOciParameterBuffer(OracleOciParameterBuffer&&) = delete;
    OracleOciParameterBuffer& operator=(const OracleOciParameterBuffer&) = delete;
    OracleOciParameterBuffer& operator=(OracleOciParameterBuffer&&) = delete;
    ~OracleOciParameterBuffer();

    void bind(ocilib::Statement statement, const ocilib::ostring& parameterMark, ocilib::BindInfo::BindDirectionValues bindDirection);
    void bindOutput(ocilib::Statement statement, const ocilib::ostring& parameterMark) const;

    template<typename T>
    T& getValue()
    {
        return *std::bit_cast<T*>(m_bindBuffer);
    }

    void setValue(const QueryParameter& value);
    template<typename T>
    void setValue(const T& value)
    {
        T& bindValue = *std::bit_cast<T*>(m_bindBuffer);
        bindValue = value;
    }

private:
    VariantDataType m_bindType;
    uint8_t*        m_bindBuffer {nullptr};

    template<typename T>
    [[nodiscard]] uint8_t* makeBuffer()
    {
        return std::bit_cast<uint8_t*>(new T);
    }

    template<typename T, typename P>
    [[nodiscard]] uint8_t* makeBuffer(const P& parameter)
    {
        return std::bit_cast<uint8_t*>(new T(parameter));
    }

    template<typename T>
    void deleteBuffer(uint8_t* buffer)
    {
        delete std::bit_cast<T*>(buffer);
    }
};

} // namespace sptk
