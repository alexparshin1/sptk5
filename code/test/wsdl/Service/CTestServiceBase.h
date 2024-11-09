// Web Service Test definition

#pragma once
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
#include "CProjectInfo.h"

namespace test_service {

/**
 * Base class for service method.
 *
 * Web Service application derives its service class from this class
 * by overriding abstract methods
 */
class WS_EXPORT CTestServiceBase : public sptk::WSRequest
{
public:
    /**
     * Constructor
     * @param logEngine        Optional log engine for error messages
     */
    explicit CTestServiceBase(sptk::LogEngine* logEngine=nullptr);

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
    virtual void AccountBalance(const CAccountBalance& input, CAccountBalanceResponse& output, sptk::HttpAuthentication* auth) = 0;

    /**
     * Web Service Hello operation
     *
     * This method is abstract and must be overwritten by derived Web Service implementation class.
     * @param input            Operation input data
     * @param output           Operation response data
     */
    virtual void Hello(const CHello& input, CHelloResponse& output, sptk::HttpAuthentication* auth) = 0;

    /**
     * Web Service Login operation
     *
     * This method is abstract and must be overwritten by derived Web Service implementation class.
     * @param input            Operation input data
     * @param output           Operation response data
     */
    virtual void Login(const CLogin& input, CLoginResponse& output, sptk::HttpAuthentication* auth) = 0;

    /**
     * @return original WSDL specifications
     */
    sptk::String wsdl() const override;

    /**
     * @return OpenAPI specifications
     */
    sptk::String openapi() const override;

    /**
     * @return SOAP WebService targetNamespace
     */
    sptk::String targetNamespace() const
    {
        return "http://www.example.org/Test/";
    }

private:

    /**
     * Internal Web Service AccountBalance processing
     * @param requestNode      Operation input/output XML data
     * @param authentication   Optional HTTP authentication
     * @param requestNameSpace Request SOAP element namespace
     */
    void process_AccountBalance(const sptk::xdoc::SNode& xmlContent, const sptk::xdoc::SNode& jsonContent, sptk::HttpAuthentication* authentication, const sptk::WSNameSpace& requestNameSpace);

    /**
     * Internal Web Service Hello processing
     * @param requestNode      Operation input/output XML data
     * @param authentication   Optional HTTP authentication
     * @param requestNameSpace Request SOAP element namespace
     */
    void process_Hello(const sptk::xdoc::SNode& xmlContent, const sptk::xdoc::SNode& jsonContent, sptk::HttpAuthentication* authentication, const sptk::WSNameSpace& requestNameSpace);

    /**
     * Internal Web Service Login processing
     * @param requestNode      Operation input/output XML data
     * @param authentication   Optional HTTP authentication
     * @param requestNameSpace Request SOAP element namespace
     */
    void process_Login(const sptk::xdoc::SNode& xmlContent, const sptk::xdoc::SNode& jsonContent, sptk::HttpAuthentication* authentication, const sptk::WSNameSpace& requestNameSpace);

};

using STestServiceBase = std::shared_ptr<CTestServiceBase>;

}
