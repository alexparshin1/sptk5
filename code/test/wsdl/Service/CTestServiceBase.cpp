#include "CTestServiceBase.h"
#include <sptk5/wsdl/WSParser.h>
#include <sptk5/wsdl/WSMessageIndex.h>
#include <set>

using namespace std;
using namespace sptk;

void CTestServiceBase::requestBroker(xml::Element* requestNode, HttpAuthentication* authentication, const WSNameSpace& requestNameSpace)
{
    static const WSMessageIndex messageNames(Strings("Hello", "|"));

    string requestName = WSParser::strip_namespace(requestNode->name());
    int messageIndex = messageNames.indexOf(requestName);
    try {
        switch (messageIndex) {
        case 0:
            process_Hello(requestNode, authentication, requestNameSpace);
            break;
        default:
            throwSOAPException("Request node '" + requestNode->name() + "' is not defined in this service")
        }
    }
    catch (const SOAPException& e) {
        auto* soapBody = (xml::Element*) requestNode->parent();
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
    catch (const Exception& e) {
        if (m_logEngine != nullptr) {
            Logger logger(*m_logEngine);
            logger.error(String("WS request error: ") + e.what());
        }
    }
}

void CTestServiceBase::process_Hello(xml::Element* requestNode, HttpAuthentication* authentication, const WSNameSpace& requestNameSpace)
{
    String ns(requestNameSpace.getAlias());
    CHello inputData((ns + ":Hello").c_str());
    CHelloResponse outputData((ns + ":HelloResponse").c_str());
    inputData.load(requestNode);
    auto* soapBody = (xml::Element*) requestNode->parent();
    soapBody->clearChildren();
    Hello(inputData, outputData, authentication);
    auto* response = new xml::Element(soapBody, (ns + ":HelloResponse").c_str());
    response->setAttribute("xmlns:" + ns, requestNameSpace.getLocation());
    outputData.unload(response);
}

    String CTestServiceBase::wsdl() const
    {
        stringstream output;
        for (int i = 0; Test_wsdl[i] != nullptr; i++)
            output << Test_wsdl[i] << endl;
        return output.str();
    }
