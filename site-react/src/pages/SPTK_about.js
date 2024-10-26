import React from "react";
import "../css/Documentation.css";
import Menu from "./Menu";

export default class SPTK_about extends React.Component
{
    constructor()
    {
        super();
        this.menuItems = {
            "/sptk_about": "About",
            "/sptk_screenshots": "Screenshots",
            "/sptk_themes": "Themes",
            "/sptk_documentation": "Documentation"
        };
    }

    render()
    {
        return <div className="SPTK" style={{textAlign: "left", padding: 8}}>
            <Menu key="about-menu" menu={this.menuItems}/>
            <div id="fadeout"/>

            <h3>Project Goals</h3>
            <p>
                SPTK is a cross-platform class library written in C++.
                It is implementing classes that are usually needed when a C++ application
                should work in more than one operating system.
                The range of classes includes GUI, threads, databases access, networking, etc.
            </p>

            <h3>Supported Operating Systems</h3>
            <p>
                The library supports Linux (.deb and .rpm flavours), BSD, and MS Windows.
                The binary files are available on Download page, along with source code downloads.
            </p>

        </div>;
    }
}
