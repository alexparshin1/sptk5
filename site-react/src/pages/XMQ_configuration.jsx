import React from "react";
import "../css/Documentation.css";
import Menu from "../components/Menu";

export default class XMQ_configuration extends React.Component
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
        return <div key="configuration-menu" className="XMQ" style={{textAlign: "left", padding: 8}}>
            <Menu menu={this.menuItems}/>
            <div id="fadeout"/>

            <p>
                XMQ has configuration interface running on port 18883, as soon as XMQ server is started.
                Open the interface in the browser as http://xmq_host:18883, and provide an administrative
                user credentials.
            </p>

            <p>
                The default administrative user name is admin and its password is also admin.
                It is recommended to change the admin user password at the first possibility.
                If you need several different users to administrate the server, simply set the
                administrator flag for these users.
            </p>

            <p>
                Most of the configuration parameter changes take effect immediately, unless the configuration
                interfaces tells you otherwise, in which case applying changes requires XMQ server restart.
            </p>

        </div>;
    }
}
