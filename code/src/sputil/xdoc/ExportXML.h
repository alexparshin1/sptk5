#pragma once

#include <sptk5/String.h>
#include <sptk5/Buffer.h>
#include "Node.h"
#include "XMLDocType.h"
#include "Document.h"

namespace sptk::xdoc {

class ExportXML
{
public:
    ExportXML(int indentSPaces = 2)
        : m_indentSpaces(indentSPaces)
    {
    }

    void saveElement(const Node& node, const String& nodeName, Buffer& buffer, int indent);

    void appendSubNodes(const Node& node, Buffer& buffer, int indent, bool only_cdata);

    void appendClosingTag(const Node& node, Buffer& buffer, int indent, bool only_cdata);

    void saveAttributes(const Node& node, Buffer& buffer);

    void save(const Node& node, Buffer& buffer, int indent);

private:
    int m_indentSpaces;
    XMLDocType m_docType;
};

}
