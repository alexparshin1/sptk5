#include <smq/server/SMQServer.h>
#include <smq/clients/SMQClient.h>

using namespace std;
using namespace sptk;
using namespace chrono;

static size_t messageCount {1000};

int main()
{
    try {
        Buffer          buffer;
        FileLogEngine   logEngine("SMQServer.log");
        seconds         connectTimeout(10);
        seconds         sendTimeout(5);

        MQProtocolType protocolType = MP_MQTT;

        SMQServer smqServer(protocolType, "user", "secret", logEngine);
        smqServer.listen(1883);

        this_thread::sleep_for(seconds(86400));

        SMQClient smqClient(protocolType, "test-client1");
        smqClient.connect(Host("localhost:1883"), "user", "secret", false, connectTimeout);

        smqClient.subscribe("test-queue", connectTimeout);

        DateTime started("now");
        auto msg = make_shared<Message>(Message::MESSAGE, Buffer("This is SMQ test"));
        for (size_t m = 0; m < messageCount; m++) {
            msg->set("This is SMQ message " + to_string(m));
            smqClient.send("test-queue", msg, sendTimeout);
            if (m % 100 == 0)
                COUT("Sent " << m << ", received " << smqClient.hasMessages() << endl);
        }

        size_t maxWait = 20000;
        while (smqClient.hasMessages() < messageCount) {
            this_thread::sleep_for(chrono::milliseconds(10));
            maxWait -= 10;
            if (maxWait == 0)
                break;
            COUT("Received " << smqClient.hasMessages() << endl);
        }

        DateTime ended("now");
        long durationMS = duration_cast<milliseconds>(ended - started).count();
        COUT("Done for " << durationMS << " ms, " << double(messageCount) / durationMS * 1000 << " msg/sec" << endl);

        smqClient.disconnect(true);
        smqServer.stop();
    }
    catch (const Exception& e) {
        CERR(e.what() << endl);
    }

    return 0;
}
