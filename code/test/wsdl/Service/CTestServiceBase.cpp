#include "CTestServiceBase.h"
#include <sptk5/wsdl/WSParser.h>
#include <sptk5/wsdl/WSMessageIndex.h>
#include <functional>
#include <set>

using namespace std;
using namespace placeholders;
using namespace sptk;
using namespace test_service;

CTestServiceBase::CTestServiceBase(LogEngine* logEngine)
: WSRequest(logEngine)
{
    map<String, RequestMethod> requestMethods {
        {"AccountBalance", bind(&CTestServiceBase::process_AccountBalance, this, _1, _2, _3, _4)},
        {"Hello", bind(&CTestServiceBase::process_Hello, this, _1, _2, _3, _4)},
        {"Login", bind(&CTestServiceBase::process_Login, this, _1, _2, _3, _4)},
    };
    setRequestMethods(move(requestMethods));
}


template <class InputData, class OutputData>
void processAnyRequest(xdoc::SNode& requestNode, HttpAuthentication* authentication, const WSNameSpace& requestNameSpace, function<void(const InputData& input, OutputData& output, HttpAuthentication* authentication)>& method)
{
   const String requestName = wsTypeIdToName(typeid(InputData).name());
   const String responseName = wsTypeIdToName(typeid(OutputData).name());
   String ns(requestNameSpace.getAlias());
   InputData inputData((ns + ":" + requestName).c_str());
   OutputData outputData((ns + ":" + responseName).c_str());
   try {
      inputData.load(requestNode);
   }   catch (const Exception& e) {
      // Can't parse input data
      throw HTTPException(400, e.what());
   }
   auto& soapBody = requestNode->parent();
   soapBody->clearChildren();
   method(inputData, outputData, authentication);
   auto& response = soapBody->pushNode(ns + ":" + responseName);
   response->setAttribute("xmlns:" + ns, requestNameSpace.getLocation());
   outputData.unload(response);
}

template <class InputData, class OutputData>
void processAnyRequest(xdoc::SNode& request, HttpAuthentication* authentication,
                       const function<void(const InputData&, OutputData&, HttpAuthentication*)>& method)
{
   InputData inputData;
   OutputData outputData;
   try {
      inputData.load(request);
   }
   catch (const Exception& e) {
      // Can't parse input data
      throw HTTPException(400, e.what());
   }
   method(inputData, outputData, authentication);
   request->clear();
   outputData.unload(request);
}


void CTestServiceBase::process_AccountBalance(xdoc::SNode& xmlNode, xdoc::SNode& jsonNode, HttpAuthentication* authentication, const WSNameSpace& requestNameSpace)
{
    function<void(const CAccountBalance&, CAccountBalanceResponse&, HttpAuthentication*)> method = bind(&CTestServiceBase::AccountBalance, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    if (xmlNode)
        processAnyRequest<CAccountBalance,CAccountBalanceResponse>(xmlNode, authentication, requestNameSpace, method);
    else
        processAnyRequest<CAccountBalance,CAccountBalanceResponse>(jsonNode, authentication, method);
}

void CTestServiceBase::process_Hello(xdoc::SNode& xmlNode, xdoc::SNode& jsonNode, HttpAuthentication* authentication, const WSNameSpace& requestNameSpace)
{
    function<void(const CHello&, CHelloResponse&, HttpAuthentication*)> method = bind(&CTestServiceBase::Hello, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    if (xmlNode)
        processAnyRequest<CHello,CHelloResponse>(xmlNode, authentication, requestNameSpace, method);
    else
        processAnyRequest<CHello,CHelloResponse>(jsonNode, authentication, method);
}

void CTestServiceBase::process_Login(xdoc::SNode& xmlNode, xdoc::SNode& jsonNode, HttpAuthentication* authentication, const WSNameSpace& requestNameSpace)
{
    function<void(const CLogin&, CLoginResponse&, HttpAuthentication*)> method = bind(&CTestServiceBase::Login, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    if (xmlNode)
        processAnyRequest<CLogin,CLoginResponse>(xmlNode, authentication, requestNameSpace, method);
    else
        processAnyRequest<CLogin,CLoginResponse>(jsonNode, authentication, method);
}

String CTestServiceBase::wsdl() const
{
    stringstream output;
    for (auto& row: Test_wsdl)
        output << row << endl;
    return output.str();
}
