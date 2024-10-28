import React from "react";
import "../css/Documentation.css";
import XMQ_interface from "../images/XMQ-interface.png";

export default class XMQ_configuration extends React.Component
{
    render()
    {
        return <div key="configuration-menu" className="XMQ" style={{textAlign: "left", padding: 8}}>
            <p>
                XMQ has configuration interface running on port 18883, as soon as XMQ server is started.
                Open the interface in the browser as http://xmq_host:18883, and provide an administrative
                user credentials.
            </p>
            <img src={XMQ_interface} alt="XMQ interface screenshot" title="XMQ interface" width="60%"/>

            <p>
                The default administrative user name is admin and its password is also admin.
                It is recommended to change the admin user password at the first possibility.
                If you need several different users to administrate the server, simply set the
                administrator flag for these users.
            </p>

            <p>
                Most of the configuration changes take effect immediately, unless the configuration
                interfaces tells you otherwise, in which case applying changes requires XMQ server restart.
            </p>

        </div>;
    }
}
