import React from "react";
import "../css/Documentation.css";
import Menu from "./Menu";

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

            <p>
                XMQ is a project aiming to create a fast and powerful MQTT server.
                It supports persistent and non-persistent modes with several database backends.
                It doesn't support clustering in the current version.
            </p>
            <p>
                XMQ is currently approaching version 1.0.
                That means that server and test utilities are already working, and you can try them on your system.
            </p>
            <p>
                The server supports Linux (.deb and .rpm flavours) and MS Windows (installer isn't yet
                available).
                BSD port is in the works, but the ETA is not available.
                The binary files are available on Download page, along with SPTK downloads.
            </p>
            <p>
                At the current stage of development, XMQ server uses 300+ unit tests.
                If you decide to give it a try, please email any bugs or shortcomings that you discover,
                and I will do my best to resolve them.
                For every noticeable bug, corresponding unit test will be added, to prevent it from happening in the
                future.
            </p>
        </div>;
    }
}
