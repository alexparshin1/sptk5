import React from "react";
import "../css/Documentation.css";
import Menu from "./Menu";

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
        return <div key="configuration-menu" className="XMQ">
            <Menu menu={this.menuItems}/>
            <div id="fadeout"/>
            configuration
        </div>;
    }
}
