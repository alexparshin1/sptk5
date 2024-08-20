#include "CTestServiceBase.h"
#include <functional>
#include <set>
#include <sptk5/wsdl/WSParser.h>

using namespace std;
using namespace placeholders;
using namespace sptk;
using namespace test_service;

CTestServiceBase::CTestServiceBase(LogEngine* logEngine)
    : WSRequest(logEngine)
{
    map<String, RequestMethod> requestMethods {

        {"AccountBalance",
         [this](const xdoc::SNode& xmlNode, const xdoc::SNode& jsonNode, HttpAuthentication* authentication, const WSNameSpace& requestNameSpace)
         {
             process_AccountBalance(xmlNode, jsonNode, authentication, requestNameSpace);
         }},

        {"Hello",
         [this](const xdoc::SNode& xmlNode, const xdoc::SNode& jsonNode, HttpAuthentication* authentication, const WSNameSpace& requestNameSpace)
         {
             process_Hello(xmlNode, jsonNode, authentication, requestNameSpace);
         }},

        {"Login",
         [this](const xdoc::SNode& xmlNode, const xdoc::SNode& jsonNode, HttpAuthentication* authentication, const WSNameSpace& requestNameSpace)
         {
             process_Login(xmlNode, jsonNode, authentication, requestNameSpace);
         }}

    };
    setRequestMethods(std::move(requestMethods));
}


template<class InputData, class OutputData>
void processAnyRequest(const xdoc::SNode& requestNode, HttpAuthentication* authentication, const WSNameSpace& requestNameSpace, function<void(const InputData& input, OutputData& output, HttpAuthentication* authentication)>& method)
{
    const String requestName = InputData::classId();
    const String responseName = OutputData::classId();
    const String       ns(requestNameSpace.getAlias());
    InputData    inputData((ns + ":" + requestName).c_str());
    OutputData   outputData((ns + ":" + responseName).c_str());
    try
    {
        inputData.load(requestNode);
    }
    catch (const Exception& e)
    {
        // Can't parse input data
        throw HTTPException(400, e.what());
    }
    const auto& soapBody = requestNode->parent();
    soapBody->clearChildren();
    method(inputData, outputData, authentication);
    auto response = soapBody->pushNode(ns + ":" + responseName);
    response->attributes().set("xmlns:" + ns, requestNameSpace.getLocation());
    outputData.unload(response);
}

template<class InputData, class OutputData>
void processAnyRequest(const xdoc::SNode& request, HttpAuthentication* authentication,
                       const function<void(const InputData&, OutputData&, HttpAuthentication*)>& method)
{
    InputData  inputData;
    OutputData outputData;
    try
    {
        inputData.load(request);
    }
    catch (const Exception& e)
    {
        // Can't parse input data
        throw HTTPException(400, e.what());
    }
    method(inputData, outputData, authentication);
    request->clear();
    outputData.unload(request);
}


void CTestServiceBase::process_AccountBalance(const xdoc::SNode& xmlNode, const xdoc::SNode& jsonNode, HttpAuthentication* authentication, const WSNameSpace& requestNameSpace)
{
    function<void(const CAccountBalance&, CAccountBalanceResponse&, HttpAuthentication*)> method =
        [this](const CAccountBalance& request, CAccountBalanceResponse& response, HttpAuthentication* auth)
    {
        AccountBalance(request, response, auth);
    };

    if (xmlNode)
        processAnyRequest<CAccountBalance, CAccountBalanceResponse>(xmlNode, authentication, requestNameSpace, method);
    else
        processAnyRequest<CAccountBalance, CAccountBalanceResponse>(jsonNode, authentication, method);
}

void CTestServiceBase::process_Hello(const xdoc::SNode& xmlNode, const xdoc::SNode& jsonNode, HttpAuthentication* authentication, const WSNameSpace& requestNameSpace)
{
    function<void(const CHello&, CHelloResponse&, HttpAuthentication*)> method =
        [this](const CHello& request, CHelloResponse& response, HttpAuthentication* auth)
    {
        Hello(request, response, auth);
    };

    if (xmlNode)
        processAnyRequest<CHello, CHelloResponse>(xmlNode, authentication, requestNameSpace, method);
    else
        processAnyRequest<CHello, CHelloResponse>(jsonNode, authentication, method);
}

void CTestServiceBase::process_Login(const xdoc::SNode& xmlNode, const xdoc::SNode& jsonNode, HttpAuthentication* authentication, const WSNameSpace& requestNameSpace)
{
    function<void(const CLogin&, CLoginResponse&, HttpAuthentication*)> method =
        [this](const CLogin& request, CLoginResponse& response, HttpAuthentication* auth)
    {
        Login(request, response, auth);
    };

    if (xmlNode)
        processAnyRequest<CLogin, CLoginResponse>(xmlNode, authentication, requestNameSpace, method);
    else
        processAnyRequest<CLogin, CLoginResponse>(jsonNode, authentication, method);
}

String CTestServiceBase::wsdl() const
{
    stringstream output;
    for (auto& row: Test_wsdl)
        output << row << endl;
    return output.str();
}
