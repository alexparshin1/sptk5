#pragma once

#include <sptk5/String.h>
#include <sptk5/Buffer.h>
#include <sptk5/xdoc/Node.h>
#include <sptk5/xdoc/XMLDocType.h>
#include <sptk5/xdoc/Document.h>

namespace sptk::xdoc {

class ExportXML
{
public:
    explicit ExportXML(int indentSPaces = 2)
        : m_indentSpaces(indentSPaces)
    {
    }

    void saveElement(const Node* node, const String& nodeName, Buffer& buffer, int indent);

    void appendSubNodes(const Node* node, Buffer& buffer, int indent, bool only_cdata);

    void appendClosingTag(const Node* node, Buffer& buffer, int indent, bool only_cdata) const;

    void saveAttributes(const Node* node, Buffer& buffer);

    void save(const SNode& node, Buffer& buffer, int indent);

private:
    int m_indentSpaces;
    XMLDocType m_docType;

    Buffer& appendNodeContent(const Node* node, Buffer& buffer);

    void appendNodeNameAndAttributes(const Node* node, const String& nodeName, Buffer& buffer);

    void appendNodeEnd(const Node* node, const String& nodeName, Buffer& buffer, bool isNode);
};

}
