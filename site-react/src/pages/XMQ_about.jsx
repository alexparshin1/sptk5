import React from "react";
import Seo from "../components/Seo";
import "../css/Documentation.css";
import {Link} from "react-router-dom";

export default class XMQ_about extends React.Component
{
    render()
    {
        return <div className="XMQ" style={{textAlign: "left", padding: 8}}>
            <Seo title="About XMQ — Fast Free MQTT Server for Linux and Windows"
                 description="XMQ is a fast, free MQTT server written in C++. It supports MQTT 3.1, 3.1.1 and 5.0, QoS 0/1/2, TLS encryption, message persistence and MQTT bridging, on Linux and Windows."
                 keywords="XMQ, fast MQTT server, free MQTT server, MQTT bridge, Linux MQTT server, Windows MQTT server"
                 path="/xmq_about"/>
            <h1>XMQ MQTT Server</h1>

            <h3>Project Goals</h3>
            <p>
                XMQ is a project aiming to create a fast and powerful MQTT server, free to use
                under the GPL/LGPL License Agreement v2.0.
                It supports persistent and non-persistent modes with several database backends.
                The supported MQTT protocol versions are 3.1, 3.1.1, and 5.0, with QoS levels
                0, 1 and 2, and TLS encryption.
                Bridging to other MQTT servers is supported; clustering is still in progress.
            </p>

            <h3>Implementation</h3>
            <p>
                XMQ server is written in C++ and uses the <Link to="/sptk_about">SPTK class
                library</Link> for cross-platform functionality — sockets and TLS, thread pools,
                database access and logging.
                It is accompanied by the test utilities that allow implementing various test scenarios.
                The <Link to="/xmq_mqtt_test_suite">MQTT test suite</Link> uses standard MQTT
                protocols, which allows comparing XMQ with other MQTT servers; the
                resulting <Link to="/xmq_tests_environment">MQTT performance tests</Link> are
                published on this site and can be reproduced.
            </p>

            <h3>Supported Operating Systems</h3>
            <p>
                As a Linux MQTT server, XMQ ships in .deb and .rpm flavours.
                As a Windows MQTT server it is supported, though the installer isn't yet available.
                BSD port is in the works, but the ETA is not available.
                The binary packages are available on the <Link to="/downloads">Downloads</Link> page.
            </p>

            <h3>The project progress</h3>
            <p>
                Here are the list of XMQ features that are already implemented or
                scheduled for implementation.
            </p>
            <table cellPadding="4" cellSpacing="4">
                <thead>
                <tr>
                    <th>feature</th>
                    <th>status</th>
                    <th>release (expected)</th>
                </tr>
                </thead>
                <tbody>
                <tr>
                    <td>Queues</td>
                    <td>Implemented</td>
                    <td>0.9.5</td>
                </tr>
                <tr>
                    <td>Topics</td>
                    <td>Implemented</td>
                    <td>0.9.5</td>
                </tr>
                <tr>
                    <td>10,000 clients</td>
                    <td>Implemented</td>
                    <td>0.9.5</td>
                </tr>
                <tr>
                    <td>200,000 clients</td>
                    <td>Implemented</td>
                    <td>0.9.6</td>
                </tr>
                <tr>
                    <td>1,000,000 clients</td>
                    <td>Implemented</td>
                    <td>0.9.13</td>
                </tr>
                <tr>
                    <td>MQTT protocol (3/4/5)</td>
                    <td>Implemented</td>
                    <td>0.9.5</td>
                </tr>
                <tr>
                    <td>MQTT protocol (QOS0, QOS1, QOS2)</td>
                    <td>Implemented</td>
                    <td>0.9.5</td>
                </tr>
                <tr>
                    <td>Linux packages (.deb,.rpm)</td>
                    <td>Implemented</td>
                    <td>0.9.5</td>
                </tr>
                <tr>
                    <td>Windows installer</td>
                    <td>Implemented</td>
                    <td>0.9.13</td>
                </tr>
                <tr>
                    <td>SSL encryption</td>
                    <td>Implemented</td>
                    <td>0.9.5</td>
                </tr>
                <tr>
                    <td>Message persistence</td>
                    <td>Implemented</td>
                    <td>0.9.5</td>
                </tr>
                <tr>
                    <td>Performance tests and compare</td>
                    <td>Implemented</td>
                    <td>0.9.7</td>
                </tr>
                <tr>
                    <td>Bridging nodes</td>
                    <td>Implemented</td>
                    <td>0.9.9</td>
                </tr>
                <tr>
                    <td>Clustering</td>
                    <td>In progress</td>
                    <td>(0.9.12)</td>
                </tr>
                <tr>
                    <td>MQTT Load Test Suite</td>
                    <td>Implemented</td>
                    <td>0.9.13</td>
                </tr>
                </tbody>
            </table>

        </div>;
    }
}
