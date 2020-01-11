// Web Service Test definition

#ifndef __CTESTSERVICEBASE__
#define __CTESTSERVICEBASE__

#include "CTestWSDL.h"
#include <sptk5/wsdl/WSRequest.h>
#include <sptk5/net/HttpAuthentication.h>

// This Web Service types
#include "CAccountBalance.h"
#include "CAccountBalanceResponse.h"
#include "CHello.h"
#include "CHelloResponse.h"
#include "CLogin.h"
#include "CLoginResponse.h"

/**
 * Base class for service method.
 *
 * Web Service application derives its service class from this class
 * by overriding abstract methods
 */
class CTestServiceBase : public sptk::WSRequest
{
    sptk::LogEngine*  m_logEngine;    ///< Optional logger, or nullptr
    /**
     * Internal Web Service AccountBalance processing
     * @param requestNode      Operation input/output XML data
     * @param authentication   Optional HTTP authentication
     * @param requestNameSpace Request SOAP element namespace
     */
    void process_AccountBalance(sptk::xml::Element* requestNode, sptk::HttpAuthentication* authentication, const sptk::WSNameSpace& requestNameSpace);

    /**
     * Internal Web Service Hello processing
     * @param requestNode      Operation input/output XML data
     * @param authentication   Optional HTTP authentication
     * @param requestNameSpace Request SOAP element namespace
     */
    void process_Hello(sptk::xml::Element* requestNode, sptk::HttpAuthentication* authentication, const sptk::WSNameSpace& requestNameSpace);

    /**
     * Internal Web Service Login processing
     * @param requestNode      Operation input/output XML data
     * @param authentication   Optional HTTP authentication
     * @param requestNameSpace Request SOAP element namespace
     */
    void process_Login(sptk::xml::Element* requestNode, sptk::HttpAuthentication* authentication, const sptk::WSNameSpace& requestNameSpace);

protected:
    /**
     * Internal SOAP body processor
     *
     * Receive incoming SOAP body of Web Service requests, and returns
     * application response.
     * @param requestNode      Incoming and outgoing SOAP element
     * @param requestNameSpace Request SOAP element namespace
     */
    void requestBroker(sptk::xml::Element* requestNode, sptk::HttpAuthentication* authentication, const sptk::WSNameSpace& requestNameSpace) override;

public:
    /**
     * Constructor
     * @param logEngine        Optional log engine for error messages
     */
    explicit CTestServiceBase(sptk::LogEngine* logEngine=nullptr)
     : m_logEngine(logEngine)
     {}

    /**
     * Destructor
     */
    ~CTestServiceBase() override = default;

    // Abstract methods below correspond to WSDL-defined operations. 
    // Application must overwrite these methods with processing of corresponding
    // requests, reading data from input and writing data to output structures.

    /**
     * Web Service AccountBalance operation
     *
     * This method is abstract and must be overwritten by derived Web Service implementation class.
     * @param input            Operation input data
     * @param output           Operation response data
     */
    virtual void AccountBalance(const CAccountBalance& input, CAccountBalanceResponse& output, sptk::HttpAuthentication* authentication) = 0;

    /**
     * Web Service Hello operation
     *
     * This method is abstract and must be overwritten by derived Web Service implementation class.
     * @param input            Operation input data
     * @param output           Operation response data
     */
    virtual void Hello(const CHello& input, CHelloResponse& output, sptk::HttpAuthentication* authentication) = 0;

    /**
     * Web Service Login operation
     *
     * This method is abstract and must be overwritten by derived Web Service implementation class.
     * @param input            Operation input data
     * @param output           Operation response data
     */
    virtual void Login(const CLogin& input, CLoginResponse& output, sptk::HttpAuthentication* authentication) = 0;

    /**
     * @return original WSDL file content
     */
    sptk::String wsdl() const override;
};

#endif
