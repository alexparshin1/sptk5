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
        return <div className="XMQ">
            <Menu key="about-menu" menu={this.menuItems}/>
            <div id="fadeout"/>
            about
        </div>;
    }
}
