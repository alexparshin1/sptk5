import React from "react";
import "../css/Documentation.css";
import Menu from "./Menu";
import connectionsTest from "../images/tests/connections.png"

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
            <Menu menu={this.menuItems}/>
            <div id="fadeout"/>

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
                They can be used with any MQTT server, not just XMQ.
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
                Connect 100K clients using 100 threads.
                Verify that there is no performance degradation with growing number of connected clients.
                The used test utility is xmq_con.
            </p>
            <img src={connectionsTest} alt="Connection performance chart"/>

        </div>;
    }
}
