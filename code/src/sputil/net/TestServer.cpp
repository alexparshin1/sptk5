#include "TestServer.h"
#include "sptk5/StopWatch.h"

#include <netinet/tcp.h>
#include <sptk5/Printer.h>

using namespace std;
using namespace sptk;

void EchoConnection::connectionFunction(Runable& task, TCPSocket& socket, const String& address)
{
    Buffer data;
    while (!task.terminated())
    {
        try
        {
            if (socket.readyToRead(chrono::seconds(1)))
            {
                if (socket.readLine(data) == 0)
                {
                    return;
                }
                string str(data.c_str());
                str += "\n";
                socket.write(str);
            }
            else
            {
                break;
            }
        }
        catch (const Exception& e)
        {
            CERR(e.what() << endl)
        }
    }
    socket.close();
}

void PushConnection::connectionFunction(Runable& task, TCPSocket& socket, const String& address)
{
    const size_t packetCount = 100000;
    const size_t packetSize = 50;
    Buffer data(packetSize);

    for (size_t i = 0; i < packetSize; ++i)
    {
        data[i] = uint8_t(i % 255);
    }
    data.bytes(packetSize);

    StopWatch stopWatch;
    stopWatch.start();

    constexpr chrono::seconds timeout(10);

    for (size_t packetNumber = 0; packetNumber < packetCount; ++packetNumber)
    {
        try
        {
            auto res = socket.send(data.data(), packetSize);
            if (res < 0)
            {
                throwSocketError("Error writing to socket", __FILE__, __LINE__);
            }
        }
        catch (const Exception& e)
        {
            CERR(e.what() << endl)
        }
    }
    stopWatch.stop();

    COUT("Sent " << packetCount << " packets at the rate " << fixed << setprecision(2) << packetCount / stopWatch.seconds() << "/s, or "
                 << packetCount * packetSize / stopWatch.seconds() / 1024 / 1024 << " Mb/s" << endl)

    socket.close();
}

void EchoSslConnection::connectionFunction(Runable& task, TCPSocket& socket, const String& address)
{
    Buffer data;
    while (!task.terminated())
    {
        try
        {
            if (socket.readyToRead(chrono::seconds(1)))
            {
                if (socket.readLine(data) == 0)
                {
                    return;
                }
                string str(data.c_str());
                str += "\n";
                socket.write(str);
            }
            else
            {
                break;
            }
        }
        catch (const Exception& e)
        {
            CERR(e.what() << endl)
        }
    }
    socket.close();
}

void PushSslConnection::connectionFunction(Runable& task, TCPSocket& socket, const String& address)
{
    const size_t packetCount = 100000;
    const size_t packetSize = 50;
    Buffer data(packetSize);

    for (size_t i = 0; i < packetSize; ++i)
    {
        data[i] = uint8_t(i % 255);
    }
    data.bytes(packetSize);

    StopWatch stopWatch;
    stopWatch.start();

    constexpr chrono::seconds timeout(10);

    for (size_t packetNumber = 0; packetNumber < packetCount; ++packetNumber)
    {
        try
        {
            auto res = socket.send(data.data(), packetSize);
            if (res < 0)
            {
                throwSocketError("Error writing to socket", __FILE__, __LINE__);
            }
        }
        catch (const Exception& e)
        {
            CERR(e.what() << endl)
        }
    }
    stopWatch.stop();

    COUT("Sent " << packetCount << " packets at the rate " << fixed << setprecision(2) << packetCount / stopWatch.seconds() << "/s, or "
                 << packetCount * packetSize / stopWatch.seconds() / 1024 / 1024 << " Mb/s" << endl)

    socket.close();
}
