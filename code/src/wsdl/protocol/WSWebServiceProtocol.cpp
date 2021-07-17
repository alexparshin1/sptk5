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

#include <sptk5/Brotli.h>
#include <sptk5/ZLib.h>
#include <sptk5/wsdl/protocol/WSWebServiceProtocol.h>
#include <sptk5/xdoc/Document.h>

using namespace std;
using namespace sptk;

WSWebServiceProtocol::WSWebServiceProtocol(HttpReader& httpReader, const URL& url, WSServices& services,
                                           const Host& host, bool allowCORS, bool keepAlive,
                                           bool suppressHttpStatus)
    : BaseWebServiceProtocol(&httpReader.socket(), httpReader.getHttpHeaders(), services, url),
      m_httpReader(httpReader),
      m_host(host),
      m_allowCORS(allowCORS),
      m_keepAlive(keepAlive),
      m_suppressHttpStatus(suppressHttpStatus)
{
}

void WSWebServiceProtocol::generateFault(Buffer& output, HttpResponseStatus& httpStatus, String& contentType,
                                         const HTTPException& e, bool jsonOutput) const
{
    httpStatus.code = e.statusCode();
    httpStatus.description = e.statusText();

    String errorText(e.what());

    // Remove possibly injected scripts
    errorText = errorText.replace("<script.*</script>", "");

    if (jsonOutput)
    {
        contentType = "application/json";

        json::Document error;
        error.root().set("error", errorText);
        error.root().set("status_code", (int) e.statusCode());
        error.root().set("status_text", e.statusText());
        error.exportTo(output, true);
    }
    else
    {
        contentType = "application/xml";

        xml::Document error;
        auto* xmlEnvelope = new xml::Element(&error, "soap:Envelope");
        xmlEnvelope->setAttribute("xmlns:soap", "http://schemas.xmlsoap.org/soap/envelope/");

        auto* xmlBody = new xml::Element(xmlEnvelope, "soap:Body");
        auto* faultNode = new xml::Element(xmlBody, "soap:Fault");

        auto* faultcodeNode = new xml::Element(faultNode, "faultCode");
        faultcodeNode->text("soap:Client");

        auto* faultstringNode = new xml::Element(faultNode, "faultString");
        faultstringNode->text(e.what());

        error.save(output, 2);
    }
}

static void substituteHostname(Buffer& page, const Host& host)
{
    xml::Document wsdl;
    wsdl.load(page);
    xml::Node* node = wsdl.findFirst("soap:address");
    if (node == nullptr)
    {
        throw Exception("Can't find <soap:address> in WSDL file");
    }
    auto location = (String) node->getAttribute("location", "");
    if (location.empty())
    {
        throw Exception("Can't find location attribute of <soap:address> in WSDL file");
    }
    stringstream listener;
    listener << "http://" << host.toString() << "/";
    location = location.replace("http://([^\\/]+)/", listener.str());
    node->setAttribute("location", location);
    wsdl.save(page, 2);
}

RequestInfo WSWebServiceProtocol::process()
{
    constexpr int okResponseCode(200);
    constexpr int httpErrorResponseCode(400);
    constexpr int onlyPostResponseCode(405);
    constexpr int invalidContentResponseCode(406);
    constexpr int serverErrorResponseCode(500);
    constexpr chrono::seconds thirtySeconds(30);

    HttpResponseStatus httpStatus {okResponseCode, "OK"};
    bool returnWSDL = false;

    if (m_httpReader.getRequestType() != "POST")
    {
        throw HTTPException(onlyPostResponseCode, "Only POST method is supported");
    }

    const Buffer& contentBuffer = m_httpReader.output();
    m_httpReader.readAll(thirtySeconds);

    RequestInfo requestInfo;

    String contentEncoding;
    String contentType(m_httpReader.httpHeader("Content-Type"));
    bool urlEncoded = contentType.find("x-www-form-urlencoded") != string::npos;
    bool requestIsJSON = true;
    if (urlEncoded)
    {
        contentEncoding = "x-www-form-urlencoded";
    }
    else
    {
        contentEncoding = m_httpReader.httpHeader("Content-Encoding");
    }
    requestInfo.request.input(contentBuffer, contentEncoding);

    auto authentication = getAuthentication();

    auto* startOfMessage = requestInfo.request.content().data();
    auto* endOfMessage = startOfMessage + requestInfo.request.content().bytes();

    while (startOfMessage != endOfMessage && *startOfMessage < 33)
    {
        ++startOfMessage;
    }

    xdoc::Document xmlContent;
    xdoc::Document jsonContent;

    if (startOfMessage != endOfMessage)
    {
        *endOfMessage = 0;
        if (*startOfMessage == '<')
        {
            contentType = "application/xml; charset=utf-8";
            requestIsJSON = false;
            processXmlContent((const char*) startOfMessage, xmlContent.root(), jsonContent.root());
        }
        else if (*startOfMessage == '{' || *startOfMessage == '[')
        {
            contentType = "application/json; charset=utf-8";
            processJsonContent((const char*) startOfMessage, jsonContent.root(), requestInfo, httpStatus,
                               contentType);
        }
        else
        {
            generateFault(requestInfo.response.content(), httpStatus, contentType,
                          HTTPException(invalidContentResponseCode, "Expect JSON or XML content"),
                          requestIsJSON);
        }
    }
    else
    {
        // Empty request content
        if (m_url.params().has("wsdl"))
        {
            // Requested WSDL content
            returnWSDL = true;
            const auto& service = m_services.get(m_url.location());
            requestInfo.response.content().set(service.wsdl());
            substituteHostname(requestInfo.response.content(), m_host);
            requestInfo.name = "wsdl";
        }
        else
        {
            // Regular request w/o content
            RESTtoSOAP(m_url, "", xmlContent.root());
        }
    }

    if (!returnWSDL && httpStatus.code < httpErrorResponseCode)
    {
        requestInfo.name = processMessage(requestInfo.response.content(), xmlContent.root(), jsonContent.root(),
                                          authentication, requestIsJSON, httpStatus, contentType);
    }

    Strings clientAcceptEncoding(header("accept-encoding"), "[,\\s]+", Strings::SplitMode::REGEXP);

    // For errors, always return uncompressed content
    if (httpStatus.code >= httpErrorResponseCode)
    {
        clientAcceptEncoding.clear();
    }

    if (httpStatus.code == serverErrorResponseCode)
    {
        clientAcceptEncoding.clear();
    }

    Buffer outputData = requestInfo.response.output(clientAcceptEncoding);
    contentEncoding = requestInfo.response.contentEncoding();

    Buffer response;
    response.append("HTTP/1.1 ");
    if (m_suppressHttpStatus && httpStatus.code >= httpErrorResponseCode)
    {
        response.append("202 Accepted\r\n");
    }
    else
    {
        response.append(to_string(httpStatus.code) + " " + httpStatus.description + "\r\n");
    }
    response.append("Content-Type: " + contentType + "\r\n");
    response.append("Content-Length: " + to_string(outputData.bytes()) + "\r\n");
    if (m_keepAlive)
    {
        response.append("Connection: keep-alive\r\n");
    }
    if (m_allowCORS)
    {
        response.append("Access-Control-Allow-Origin: *\r\n");
    }
    if (!contentEncoding.empty())
    {
        response.append("Content-Encoding: " + contentEncoding + "\r\n");
    }

    // OWASP-suggested headers
    response.append("X-Content-Type-Options: nosniff\r\n");

    response.append("\r\n", 2);
    response.append(outputData);
    socket().write(response);

    return requestInfo;
}

shared_ptr<HttpAuthentication> WSWebServiceProtocol::getAuthentication()
{
    shared_ptr<HttpAuthentication> authentication;

    if (auto itor = headers().find("authorization");
        itor != headers().end())
    {
        String value(itor->second);
        authentication = make_shared<HttpAuthentication>(value);
    }

    return authentication;
}

int WSWebServiceProtocol::getContentLength()
{
    int contentLength = -1; // Undefined
    if (m_url.params().has("wsdl"))
    {
        contentLength = 0;
    }

    if (auto itor = headers().find("Content-Length");
        itor != headers().end())
    {
        contentLength = string2int(itor->second);
    }
    return contentLength;
}
