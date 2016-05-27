#include <sptk5/wsdl/CWSListener.h>

using namespace std;
using namespace sptk;

class CWSConnection : public CTCPServerConnection
{
protected:
    CWSRequest&     m_service;
    Logger&       m_logger;
    const string&   m_staticFilesDirectory;
public:

    CWSConnection(SOCKET connectionSocket, sockaddr_in*, CWSRequest& service, Logger& logger, const string& staticFilesDirectory)
    : CTCPServerConnection(connectionSocket), m_service(service), m_logger(logger), m_staticFilesDirectory(staticFilesDirectory)
    {
    }

    virtual ~CWSConnection()
    {
    }

    virtual void threadFunction();
};

void CWSConnection::threadFunction()
{
    static const CRegExp parseProtocol("^(GET|POST) (\\S+)", "i");
    static const CRegExp parseHeader("^([^:]+): \"{0,1}(.*)\"{0,1}$", "i");

    const char* startOfMessage = NULL;
    const char* endOfMessage = NULL;
    const char* endOfMessageMark = ":Envelope>";

    CBuffer data;

    // Read request data
    string      row;
    CStrings    matches;
    string      protocol, url, requestType;
    int         contentLength = 0;

    try {
        if (!m_socket->readyToRead(30000)) {
            m_socket->close();
            m_logger << LP_DEBUG << "Client closed connection" << endl;
            return;
        }

        try {
            while (!terminated()) {
                if (!m_socket->readLine(data))
                    return;
                row = trim(data.c_str());
                if (protocol.empty()) {
                    if (strstr(row.c_str(), "<?xml")) {
                        protocol = "xml";
                        break;
                    }
                    if (parseProtocol.m(row, matches)) {
                        protocol = "http";
                        requestType = matches[0];
                        url = matches[1];
                        continue;
                    }
                }
                if (parseHeader.m(row, matches)) {
                    string header = matches[0];
                    string value = matches[1];
                    if (lowerCase(header) == "content-length")
                        contentLength = string2int(value);
                    continue;
                }
                if (row.empty()) {
                    data.reset();
                    break;
                }
            }
        }
        catch (CException& e) {
            m_logger << LP_ERROR << e.message() << endl;
            return;
        }
        catch (exception& e) {
            m_logger << LP_ERROR << e.what() << endl;
            return;
        }

        if (protocol == "http" && !url.empty() && url != "/service.html") {
            CBuffer page;
            try {
                page.loadFromFile(m_staticFilesDirectory + url);
                m_socket->write("HTTP/1.1 200 OK\n");
                m_socket->write("Content-Type: text/html; charset=utf-8\n");
                m_socket->write("Content-Length: " + int2string(page.bytes()) + "\n\n");
                m_socket->write(page);
            }
            catch (...) {
                string text("<html><head><title>Not Found</title></head><body>Sorry, the page you requested was not found.</body></html>\n");
                m_socket->write("HTTP/1.1 404 Not Found\n");
                m_socket->write("Content-Type: text/html; charset=utf-8\n");
                m_socket->write("Content-length: " + int2string(text.length()) + "\n\n");
                m_socket->write(text);
            }
            m_socket->close();
            return;
        }

        if (protocol == "xml")
            m_socket->write("<?xml version='1.0' encoding='UTF-8'?><server name='" + m_service.title() + "' version='1.0'/>\n");
        uint32_t offset = 0;
        while (!terminated()) {
            if (contentLength) {
                m_socket->read(data, contentLength);
                startOfMessage = data.c_str();
                endOfMessage = startOfMessage + data.bytes();
            } else {
                uint32_t socketBytes = m_socket->socketBytes();
                if (!socketBytes) {
                    if (!m_socket->readyToRead(30000)) {
                        m_logger <<"Client disconnected" << endl;
                        break;
                    }
                    socketBytes = m_socket->socketBytes();
                }
                // If socket is signaled but empty - then other side closed connection
                if (socketBytes == 0) {
                    m_logger <<"Client disconnected" << endl;
                    break;
                }
                do {
                    // Read all available data (appending to data buffer)
                    data.checkSize(offset + socketBytes);
                    socketBytes = (uint32_t) m_socket->read(data.data() + offset, (uint32_t) socketBytes);
                    data.bytes(offset + socketBytes);
                    //cout << data.c_str() << endl;
                    if (!startOfMessage) {
                        startOfMessage = strstr(data.c_str(), "<?xml");
                        if (!startOfMessage) {
                            startOfMessage = strstr(data.c_str(), "Envelope");
                            if (startOfMessage)
                                while (*startOfMessage != '<' && startOfMessage > data.c_str())
                                    startOfMessage--;
                        }
                        if (!startOfMessage)
                            throwException("Message start <?xml> not found");
                    }
                    endOfMessage = strstr(startOfMessage, endOfMessageMark);
                } while (!endOfMessage && !terminated());

                if (terminated())
                    break;

                // Message received, processing it
                endOfMessage += strlen(endOfMessageMark);
            }

            CXmlDoc message;
            if (endOfMessage)
                *(char *) endOfMessage = 0;
            message.load(startOfMessage);

            //cout << startOfMessage << endl << endl;

            CBuffer output;
            m_service.processRequest(&message);
            message.save(output);
            
            //cout << output.c_str() << endl;

            if (protocol == "http") {
                m_socket->write("HTTP/1.1 200 OK\n");
                m_socket->write("Content-Type: text/xml; charset=utf-8\n");
                m_socket->write("Content-Length: " + int2string(output.bytes()) + "\n\n");
            }
            m_socket->write(output);
            m_socket->close();
            break;
        }
    }
    catch (exception& e) {
        if (!terminated())
            m_logger << LP_ERROR << "Error in thread " << name() << ": " << e.what() << endl;
    }
    catch (...) {
        if (!terminated())
            m_logger << LP_ERROR << "Unknown error in thread " << name() << endl;
    }
}

CWSListener::CWSListener(CWSRequest& service, Logger& logger, string staticFilesDirectory)
: CTCPServer(), m_service(service), m_logger(logger), m_staticFilesDirectory(staticFilesDirectory)
{
}

CWSListener::~CWSListener()
{
}

CServerConnection* CWSListener::createConnection(SOCKET connectionSocket, sockaddr_in* peer)
{
    return new CWSConnection(connectionSocket, peer, m_service, m_logger, m_staticFilesDirectory);
}
