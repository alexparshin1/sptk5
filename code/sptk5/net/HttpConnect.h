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

#pragma once

#include <memory>
#include <sptk5/Strings.h>
#include <sptk5/net/HttpParams.h>
#include <sptk5/net/HttpReader.h>
#include <sptk5/net/TCPSocket.h>

namespace sptk {
/**
 * @addtogroup network Network Classes.
 * @{
 */

/**
 * @brief HTTP socket.
 *
 * Implements the GET, POST, PUT, and DELETE methods of HTTP protocol.
 * Allows defining the host, port, submit information, and then sends the HTML request to the server.
 */
class SP_EXPORT HttpConnect
{
public:
    /**
     * @brief HTTP authorization.
     */
    class SP_EXPORT Authorization
    {
    public:
        /**
         * @brief Copy constructor.
         * @param other         Another object.
         */
        Authorization(const Authorization& other) = default;

        /**
         * @brief Constructor.
         */
        Authorization() = delete;

        /**
         * @brief Copy assignment.
         * @param other         Other object.
         */
        Authorization& operator=(const Authorization& other) = default;

        /**
         * @brief Get authorization method name.
         * @return authorization method name.
         */
        [[nodiscard]] String method() const
        {
            return m_method;
        }

        /**
         * @brief Get authorization method value.
         * @return authorization method value.
         */
        [[nodiscard]] String value() const
        {
            return m_value;
        }

    protected:
        /**
         * @brief Basic or Bearer authorization.
         * @param method        Auth method, 'basic' or 'bearer'.
         * @param username      Username, basic authorization only.
         * @param password      Password, basic authorization only.
         * @param jwtToken      JWT token, bearer authorization only.
         */
        explicit Authorization(const String& method, const String& username, const String& password, const String& jwtToken);

    private:
        String m_method; ///< Authorization method name.
        String m_value;  ///< Authorization data.
    };

    /**
     * @brief Basic authorization.
     */
    class SP_EXPORT BasicAuthorization : public Authorization
    {
    public:
        /**
         * @brief Basic authorization.
         * @param username      Username.
         * @param password      Password.
         */
        BasicAuthorization(const String& username, const String& password)
            : Authorization("basic", username, password, "")
        {
        }
    };

    /**
     * @brief Bearer authorization.
     */
    class SP_EXPORT BearerAuthorization : public Authorization
    {
    public:
        /**
         * @brief Bearer authorization.
         * @param jwtToken      JWT token.
         */
        explicit BearerAuthorization(const String& jwtToken)
            : Authorization("bearer", "", "", jwtToken)
        {
        }
    };

    /**
     * @brief Constructor.
     *
     * External socket has to be active before HTTP operations.
     * For HTTPS protocol, use COpenSSLSocket.
     * @param socket            external socket.
     */
    explicit HttpConnect(std::shared_ptr<TCPSocket> socket);

    /**
     * @brief Returns the HTTP request headers.
     *
     * The HTTP headers request should be set before sending a command to the server.
     * @returns internal http request headers reference.
     */
    HttpHeaders& requestHeaders()
    {
        return m_requestHeaders;
    }

    /**
     * @brief Returns the HTTP headers.
     *
     * The HTTP response headers make sense only after sending a command to the server (if that command calls
     * the getResponse() method internally).
     * @returns internal http headers reference.
     */
    [[nodiscard]] const HttpHeaders& responseHeaders() const;

    /**
     * @brief Sends the GET command to the server.
     *
     * Retrieves the server response into the internal read buffer.
     * @param pageName          Page URL without the server name.
     * @param parameters        HTTP request parameters.
     * @param output            Output data.
     * @param authorization     Optional authorization.
     * @param timeout           Response timeout.
     * @return HTTP result code.
     */
    [[nodiscard]] int cmd_get(const String& pageName, const HttpParams& parameters, Buffer& output,
                              const Authorization*             authorization = nullptr,
                              const std::chrono::milliseconds& timeout = std::chrono::seconds(60));

    /**
     * @brief Sends the POST command to the server.
     *
     * Retrieves the server response into the internal read buffer.
     * @param pageName          Page URL without the server name.
     * @param parameters        HTTP request parameters.
     * @param content           The data to post to the server.
     * @param output            Output data.
     * @param possibleContentEncodings Possible content encodings.
     * @param authorization     Optional authorization.
     * @param timeout           Response timeout.
     * @return HTTP result code.
     */
    [[nodiscard]] int cmd_post(const String& pageName, const HttpParams& parameters, const Buffer& content, Buffer& output,
                               const Strings& possibleContentEncodings, const Authorization* authorization = nullptr,
                               const std::chrono::milliseconds& timeout = std::chrono::seconds(60));

    /**
     * @brief Sends the PUT command to the server.
     *
     * Retrieves the server response into the internal read buffer.
     * @param pageName          Page URL without the server name.
     * @param parameters        HTTP request parameters.
     * @param content           The data to post to the server.
     * @param output            Output data.
     * @param authorization     Optional authorization.
     * @param timeout           Optional response timeout.
     * @return HTTP result code.
     */
    [[nodiscard]] int cmd_put(const String& pageName, const HttpParams& parameters, const Buffer& content, Buffer& output,
                              const Authorization*             authorization = nullptr,
                              const std::chrono::milliseconds& timeout = std::chrono::seconds(60));

    /**
      * @brief Sends the DELETE command to the server.
      *
      * Retrieves the server response into the internal read buffer.
      * @param pageName          Page URL without the server name.
      * @param parameters        HTTP request parameters.
      * @param output            Output data.
      * @param authorization     Optional authorization.
      * @param timeout           Request timeout.
      * @return HTTP result code.
      */
    [[nodiscard]] int cmd_delete(const String& pageName, const HttpParams& parameters, Buffer& output,
                                 const Authorization*             authorization = nullptr,
                                 const std::chrono::milliseconds& timeout = std::chrono::seconds(60));

    /**
     * @brief Get value of the response header.
     * @param headerName        Response header name.
     * @return header value, or empty string if the header is not a part of the response.
     */
    [[nodiscard]] String responseHeader(const String& headerName) const;

    /**
     * @brief Get the request execution status code.
     * @return request execution status code.
     */
    [[nodiscard]] int statusCode() const;

    /**
     * @brief Get the request execution status text.
     * @return request execution status text.
     */
    [[nodiscard]] String statusText() const;

protected:
    /**
     * @brief Create default headers for HTTP request.
     */
    Strings makeHeaders(const String& httpCommand, const String& pageName, const HttpParams& requestParameters,
                        const Authorization* authorization) const;

    /**
     * @brief Sends a single command to the HTTP server.
     *
     * CRLF characters are automatically appended to the command.
     * @param cmd               HTTP command.
     */
    void sendCommand(const String& cmd) const;

    /**
     * @brief Sends a single command to the HTTP server.
     *
     * CRLF characters are automatically appended to the command.
     * @param cmd               HTTP command.
     */
    void sendCommand(const Buffer& cmd) const;

    /**
     * @brief Retrieves the server response on the command.
     *
     * Stops when the HTTP server closes the connection. The server response can then be
     * accessed through the htmlData() method.
     * @param output            Output buffer to store the server response.
     * @param timeout           Response timeout.
     * @return HTTP result code.
     */
    int getResponse(Buffer& output, const std::chrono::milliseconds& timeout);

private:
    std::shared_ptr<HttpReader> m_reader;         ///< HTTP reader.
    std::shared_ptr<TCPSocket>  m_socket;         ///< External socket.
    HttpHeaders                 m_requestHeaders; ///< HTTP request headers.
};

/**
 * @}
 */

} // namespace sptk
