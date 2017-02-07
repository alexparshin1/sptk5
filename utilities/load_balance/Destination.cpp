#include "Destination.h"
#include <sptk5/net/BaseSocket.h>

using namespace std;
using namespace sptk;

Destination::Destination(String host, uint16_t port)
: m_host(host), m_port(port)
{
    BaseSocket::getHostAddress(m_host, m_address);
    m_address.sin_port = htons(m_port);
    m_address.sin_family = AF_INET;
}

Destination::Destination(const Destination& other)
: m_host(other.m_host), m_port(other.m_port)
{
    BaseSocket::getHostAddress(m_host, m_address);
    m_address.sin_port = htons(m_port);
    m_address.sin_family = AF_INET;
}
