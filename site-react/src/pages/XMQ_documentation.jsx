import React from "react";
import Seo from "../components/Seo";
import "../css/Documentation.css";
import UserManualScreen from "./UserManualScreen";

/**
 * XMQ documentation page.
 *
 * The manual itself is UserManualScreen, copied verbatim from the XMQ repository
 * (gui-react/src/screen/), where it is also served by the server's own configuration
 * interface. Only this wrapper is site-specific, so refreshing the documentation is a
 * copy of UserManualScreen.jsx and UserManual.css and nothing else.
 */
export default class XMQ_documentation extends React.Component
{
    render()
    {
        return <div key="xmq-documentation" className="XMQ" style={{textAlign: "left", padding: 8}}>
            <Seo title="XMQ MQTT Server Documentation"
                 description="User manual for the XMQ MQTT server: installation on Linux, FreeBSD and Windows, configuration, MQTT bridge setup, security and administration."
                 path="/xmq_documentation"/>
            <h1>XMQ MQTT Server Documentation</h1>
            <UserManualScreen/>
        </div>;
    }
}
