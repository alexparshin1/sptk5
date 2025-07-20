#pragma once

#include <sptk5/Buffer.h>
#include <sptk5/String.h>
#include <sptk5/xdoc/Document.h>
#include <sptk5/xdoc/Node.h>
#include <sptk5/xdoc/XMLDocType.h>

namespace sptk::xdoc {

class SP_EXPORT ExportXML
{
public:
    explicit ExportXML(int indentSpaces = 2)
        : m_indentSpaces(indentSpaces)
    {
    }

    void saveElement(const Node* node, const String& nodeName, Buffer& buffer, bool formatted, int indent);

    void appendSubNodes(const Node* node, Buffer& buffer, bool formatted, int indent);

    static void appendClosingTag(const Node* node, Buffer& buffer, bool formatted, int indent);

    void saveAttributes(const Node* node, Buffer& buffer);

private:
    int        m_indentSpaces {2};
    XMLDocType m_docType;

    Buffer& appendNodeContent(const Node* node, Buffer& buffer);

    void appendNodeNameAndAttributes(const Node* node, const String& nodeName, Buffer& buffer);

    void appendNodeEnd(const Node* node, const String& nodeName, Buffer& buffer, bool isNode);
};

} // namespace sptk::xdoc
