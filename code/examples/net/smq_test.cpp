#include <sptk5/mq/SMQServer.h>
#include <sptk5/mq/SMQClient.h>
#include <sptk5/cutils>
#include "MQClient.h"

using namespace std;
using namespace sptk;

static size_t messageCount {50000};

int main()
{
    try {
        Buffer          buffer;
        FileLogEngine   logEngine("SMQServer.log");

        SMQServer smqServer("user", "secret", logEngine);
        smqServer.listen(4000);

        SMQClient smqClient;
        smqClient.connect(Host("localhost:4000"), "test-client1", "user", "secret");

        smqClient.subscribe("test-queue", std::chrono::milliseconds());

        DateTime started("now");
        Message msg(Message::MESSAGE, Buffer("This is SMQ test"), "test-queue");
        for (size_t m = 0; m < messageCount; m++) {
            //if (m%5 == 0)
                //this_thread::sleep_for(chrono::microseconds(10));
            msg.set("This is SMQ message " + to_string(m));
            smqClient.sendMessage(msg);
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
        size_t durationMS = chrono::duration_cast<chrono::milliseconds>(ended - started).count();
        COUT("Done for " << durationMS << " ms, " << double(messageCount) / durationMS * 1000 << " msg/sec" << endl);

        smqClient.disconnect();
        smqServer.stop();
    }
    catch (const Exception& e) {
        CERR(e.what() << endl);
    }

    return 0;
}
