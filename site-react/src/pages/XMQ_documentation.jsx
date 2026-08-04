import React from "react";
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
            <UserManualScreen/>
        </div>;
    }
}
