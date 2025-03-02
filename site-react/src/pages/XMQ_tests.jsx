import React from "react";
import "../css/Documentation.css";
import connectionsTest from "../images/tests/connections.png"
import sendTest1 from "../images/tests/send-1M-messages-to-1K-clients.png"

export default class XMQ_tests extends React.Component
{
    constructor()
    {
        super();
        this.menuItems = {
            "/xmq_about": "About",
            "/xmq_tests": "Tests",
            "/xmq_configuration": "Configuration"
        };
    }

    render()
    {
        return <div key="test-menu" className="XMQ" style={{textAlign: "left", padding: 8}}>
            <p>
                The XMQ server provides several utilities that allow performing load testing of the following,
                for N simultaneously running clients:
            </p>
            <ul>
                <li>How fast clients can connect to the server?</li>
                <li>How fast clients can subscribe to the same topic?</li>
                <li>How fast clients can subscribe to the topic per client?</li>
                <li>How fast clients can subscribe to the topic per client?</li>
                <li>How fast clients can pass messages to same number of subscribers?
                </li>
            </ul>
            <p>
                The testing utilities xmq_con, xmq_pub, and xmq_sub, are using MQTT protocols 3.1 to 5.0.
                They can be used with any MQTT server, not just XMQ, when one or more MQTT clients simulation is required.
                The testing is comparing XMQ to recognised leaders in the are, such as eMQX and Mosquitto.
            </p>
            <p>
                For the each test, the testing utilities produce the statistics in 20 intervals.
                The results are shown in the charts.
                The maximum number of clients is limited by the testing system:
                <ul>
                    <li>XMQ server is running on Intel NUC 7 with 8 cores and 32GB memory.</li>
                    <li>The test utilities are running on a VM configured to have 4 network interfaces.
                        The VM is hosted on Intel NUC 7 with 8 cores and 32GB memory.
                    </li>
                </ul>
            </p>

            <h4>Connection Performance</h4>
            <p>
                Connect 1K clients using 100 threads. The protocol is MQTT 3.1, not encrypted.
                The test checks how fast connections can be performed.
                The used test utility is xmq_con.
            </p>
            <img src={connectionsTest} alt="Connection performance chart"/>

            <h4>Message Send Performance</h4>
            <p>
                Connect 1K publishers using a topic per a pair publisher, then send 1M messages through 1K topics. The protocol is MQTT 3.1, not encrypted.
                The test checks how fast the server can receive messages.
                The used test utilities are xmq_pub and xmq_sub.
            </p>
            <img src={sendTest1} alt="Send performance chart"/>

        </div>;
    }
}
