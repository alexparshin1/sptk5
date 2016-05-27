#ifndef __CWSLISTENER_H__
#define __CWSLISTENER_H__

#include <sptk5/cutils>
#include <sptk5/cnet>
#include <sptk5/wsdl/CWSRequest.h>

namespace sptk {

/// @addtogroup wsdl WSDL-related Classes
/// @{

class CWSListener : public sptk::CTCPServer
{
protected:
    sptk::CWSRequest&   m_service;              ///< Web Service request processor
    sptk::Logger&     m_logger;               ///< Logger object
    const std::string   m_staticFilesDirectory; ///< Web Service static files directory

    /// @brief Creates connection thread derived from CTCPServerConnection
    ///
    /// Application should override this method to create concrete connection object.
    /// Created connection object is maintained by CTCPServer.
    /// @param connectionSocket SOCKET, Already accepted incoming connection socket
    /// @param peer sockaddr_in*, Incoming connection information
    virtual sptk::CServerConnection* createConnection(SOCKET connectionSocket, sockaddr_in* peer);
    
public:
    /// @brief Constructor
    /// @param service sptk::CWSRequest&, Web Service request processor
    /// @param logger sptk::Logger&, Logger
    /// @param staticFilesDirectory const std::string&, Web Service static files directory
    CWSListener(sptk::CWSRequest& service, sptk::Logger& logger, std::string staticFilesDirectory);

    /// @brief Destructor
    ~CWSListener();
 };
 
/// @}
 
}

#endif
