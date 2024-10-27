import React from "react";
import "../css/Documentation.css";
import Menu from "../components/Menu";
import {Link} from "react-router-dom";

export default class XMQ_about extends React.Component
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
        return <div className="XMQ" style={{textAlign: "left", padding: 8}}>
            <Menu key="about-menu" menu={this.menuItems}/>
            <div id="fadeout"/>

            <h3>Project Goals</h3>
            <p>
                XMQ is a project aiming to create a fast and powerful MQTT server.
                It supports persistent and non-persistent modes with several database backends.
                The supported MQTT protocol versions are 3.1, 3.11, and 5.0.
                It doesn't support clustering in the current version.
            </p>

            <h3>Implementation</h3>
            <p>
                XMQ server is written in C++ and uses SPTK class library for cross-platform
                functionality.
                It is accompanied by the test utilities that allow implementing various test scenarious.
                The <Link to="/xmq_tests">test utilities</Link> are using standard MQTT protocols that allows comparing
                XMQ with
                other MQTT server.
            </p>

            <h3>Supported Operating Systems</h3>
            <p>
                The server supports Linux (.deb and .rpm flavours) and MS Windows (installer isn't yet
                available).
                BSD port is in the works, but the ETA is not available.
                The binary files are available on Download page, along with SPTK downloads.
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
                    <td>Scheduled</td>
                    <td>(0.9.8)</td>
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
                    <td>Scheduled</td>
                    <td>(0.9.7)</td>
                </tr>
                </tbody>
            </table>

        </div>;
    }
}
