#include "CTestServiceBase.h"
#include <sptk5/wsdl/WSParser.h>
#include <sptk5/wsdl/WSMessageIndex.h>
#include <functional>
#include <set>

using namespace std;
using namespace sptk;
using namespace test_service;

void CTestServiceBase::requestBroker(const String& requestName, xml::Element* xmlContent, json::Element* jsonContent, HttpAuthentication* authentication, const WSNameSpace& requestNameSpace)
{
    static const WSMessageIndex messageNames(Strings("AccountBalance|Hello|Login", "|"));

    int messageIndex = messageNames.indexOf(requestName);
    try {
        switch (messageIndex) {
        case 0:
            process_AccountBalance(xmlContent, jsonContent, authentication, requestNameSpace);
            break;
        case 1:
            process_Hello(xmlContent, jsonContent, authentication, requestNameSpace);
            break;
        case 2:
            process_Login(xmlContent, jsonContent, authentication, requestNameSpace);
            break;
        default:
            throw SOAPException("Request '" + requestName + "' is not defined in this service");
        }
    }
    catch (const SOAPException& e) {
        logError(requestName, e.what(), 0);
        handleError(xmlContent, jsonContent, e.what(), 0);
    }
    catch (const HTTPException& e) {
        logError(requestName, e.what(), (int) e.statusCode());
        handleError(xmlContent, jsonContent, e.what(), (int) e.statusCode());
    }
    catch (const Exception& e) {
        logError(requestName, e.what(), 0);
        handleError(xmlContent, jsonContent, e.what(), 0);
    }
}

void CTestServiceBase::logError(const String& requestName, const String& error, int errorCode) const
{
    if (m_logEngine) {
        Logger logger(*m_logEngine);
        if (errorCode != 0)
            logger.error(requestName + ": " + to_string(errorCode) + " " + error);
        else
            logger.error(requestName + ": " + error);
    }
}

void CTestServiceBase::handleError(xml::Element* xmlContent, json::Element* jsonContent, const String& error, int errorCode) const
{
    // Error handling
    if (xmlContent) {
        auto* soapBody = (xml::Element*) xmlContent->parent();
        soapBody->clearChildren();
        String soap_namespace = WSParser::get_namespace(soapBody->name());
        if (!soap_namespace.empty())
            soap_namespace += ":";
        auto* faultNode = new xml::Element(soapBody, (soap_namespace + "Fault").c_str());
        auto* faultCodeNode = new xml::Element(faultNode, "faultcode");
        faultCodeNode->text(soap_namespace + "Client");
        auto* faultStringNode = new xml::Element(faultNode, "faultstring");
        faultStringNode->text(error);
        new xml::Element(faultNode, "detail");
    }
    else {
        jsonContent->clear();
        if (errorCode != 0)
            jsonContent->set("error_code", errorCode);
        jsonContent->set("error_description", error);
    }
}


template <class InputData, class OutputData>
void processAnyRequest(xml::Element* requestNode, HttpAuthentication* authentication, const WSNameSpace& requestNameSpace, function<void(const InputData& input, OutputData& output, HttpAuthentication* authentication)>& method)
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
   auto* soapBody = (xml::Element*) requestNode->parent();
   soapBody->clearChildren();
   method(inputData, outputData, authentication);
   auto* response = new xml::Element(soapBody, (ns + ":" + responseName).c_str());
   response->setAttribute("xmlns:" + ns, requestNameSpace.getLocation());
   outputData.unload(response);
}

template <class InputData, class OutputData>
void processAnyRequest(json::Element* request, HttpAuthentication* authentication,
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


void CTestServiceBase::process_AccountBalance(xml::Element* xmlNode, json::Element* jsonNode, HttpAuthentication* authentication, const WSNameSpace& requestNameSpace)
{
  function<void(const CAccountBalance&, CAccountBalanceResponse&, HttpAuthentication*)> method = bind(&CTestServiceBase::AccountBalance, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
  if (xmlNode)
     processAnyRequest<CAccountBalance,CAccountBalanceResponse>(xmlNode, authentication, requestNameSpace, method);
  else
     processAnyRequest<CAccountBalance,CAccountBalanceResponse>(jsonNode, authentication, method);
}

void CTestServiceBase::process_Hello(xml::Element* xmlNode, json::Element* jsonNode, HttpAuthentication* authentication, const WSNameSpace& requestNameSpace)
{
  function<void(const CHello&, CHelloResponse&, HttpAuthentication*)> method = bind(&CTestServiceBase::Hello, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
  if (xmlNode)
     processAnyRequest<CHello,CHelloResponse>(xmlNode, authentication, requestNameSpace, method);
  else
     processAnyRequest<CHello,CHelloResponse>(jsonNode, authentication, method);
}

void CTestServiceBase::process_Login(xml::Element* xmlNode, json::Element* jsonNode, HttpAuthentication* authentication, const WSNameSpace& requestNameSpace)
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
