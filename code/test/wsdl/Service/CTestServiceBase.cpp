#include "CTestServiceBase.h"
#include <sptk5/wsdl/WSParser.h>
#include <sptk5/wsdl/WSMessageIndex.h>
#include <functional>
#include <set>

using namespace std;
using namespace sptk;

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
            faultStringNode->text(e.what());
            new xml::Element(faultNode, "detail");
        }
        else throw;
    }
    catch (const Exception& e) {
        if (m_logEngine != nullptr) {
            Logger logger(*m_logEngine);
            logger.error(String("WS request error: ") + e.what());
        }
    }
}

template <class InputData, class OutputData>
void processAnyRequest(xml::Element* requestNode, const String& requestName, const String& responseName, HttpAuthentication* authentication, const WSNameSpace& requestNameSpace, function<void(const InputData& input, OutputData& output, HttpAuthentication* authentication)>& method)
{
   String ns(requestNameSpace.getAlias());
   InputData inputData((ns + ":" + requestName).c_str());
   OutputData outputData((ns + ":" + responseName).c_str());
   inputData.load(requestNode);
   auto* soapBody = (xml::Element*) requestNode->parent();
   soapBody->clearChildren();
   method(inputData, outputData, authentication);
   auto* response = new xml::Element(soapBody, (ns + ":" + responseName).c_str());
   response->setAttribute("xmlns:" + ns, requestNameSpace.getLocation());
   outputData.unload(response);
}

template <class InputData, class OutputData>
void processAnyRequest(json::Element* request, HttpAuthentication* authentication,
                       function<void(const InputData&, OutputData&, HttpAuthentication*)>& method)
{
   InputData inputData;
   OutputData outputData;
   inputData.load(request);
   method(inputData, outputData, authentication);
   request->clear();
   outputData.unload(request);
}


void CTestServiceBase::process_AccountBalance(xml::Element* xmlNode, json::Element* jsonNode, HttpAuthentication* authentication, const WSNameSpace& requestNameSpace)
{
  function<void(const CAccountBalance&, CAccountBalanceResponse&, HttpAuthentication*)> method = bind(&CTestServiceBase::AccountBalance, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
  if (xmlNode)
     processAnyRequest<CAccountBalance,CAccountBalanceResponse>(xmlNode, "AccountBalance", "AccountBalanceResponse", authentication, requestNameSpace, method);
  else
     processAnyRequest<CAccountBalance,CAccountBalanceResponse>(jsonNode, authentication, method);
}

void CTestServiceBase::process_Hello(xml::Element* xmlNode, json::Element* jsonNode, HttpAuthentication* authentication, const WSNameSpace& requestNameSpace)
{
  function<void(const CHello&, CHelloResponse&, HttpAuthentication*)> method = bind(&CTestServiceBase::Hello, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
  if (xmlNode)
     processAnyRequest<CHello,CHelloResponse>(xmlNode, "Hello", "HelloResponse", authentication, requestNameSpace, method);
  else
     processAnyRequest<CHello,CHelloResponse>(jsonNode, authentication, method);
}

void CTestServiceBase::process_Login(xml::Element* xmlNode, json::Element* jsonNode, HttpAuthentication* authentication, const WSNameSpace& requestNameSpace)
{
  function<void(const CLogin&, CLoginResponse&, HttpAuthentication*)> method = bind(&CTestServiceBase::Login, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
  if (xmlNode)
     processAnyRequest<CLogin,CLoginResponse>(xmlNode, "Login", "LoginResponse", authentication, requestNameSpace, method);
  else
     processAnyRequest<CLogin,CLoginResponse>(jsonNode, authentication, method);
}

    String CTestServiceBase::wsdl() const
    {
        stringstream output;
        for (int i = 0; Test_wsdl[i] != nullptr; i++)
            output << Test_wsdl[i] << endl;
        return output.str();
    }
