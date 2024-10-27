import React from "react";
import "../css/Documentation.css";
import Menu from "../components/Menu";

export default class SPTK_documentation extends React.Component
{
    state = {
        fileName: ""
    };

    constructor() {
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
        return <div className="Documentation">
            <Menu key="about-menu" menu={this.menuItems}/>
            <div id="fadeout"/>
            <p>Currently, the following documentation is available:</p>
            <table className="DocumentationItem">
                <tbody>
                <tr>
                    <td><a href="http://www.sptk.net/classes/hierarchy.html"
                           className="DocumentationItem">SPTK 3.0 classes
                        hieararchy</a> by Doxygen
                    </td>
                </tr>
                <tr>
                    <td><a href="http://www.sptk.net/docs/dialog.html">Happy Life With CDialog</a> by Alexey Parshin
                    </td>
                </tr>
                <tr>
                    <td><a href="http://www.sptk.net/docs/layout.html">New Layouts Initiative</a> by Alexey Parshin</td>
                </tr>
                <tr>
                    <td><a href="http://www.sptk.net/docs/devcpp/index.htm">HowTo use SPTK/SPDB Dev C++</a> by Andreas
                        Bresser
                    </td>
                </tr>
                <tr>
                    <td><a href="http://www.sptk.net/docs/HowTo-SPTK-Win32.html">HowTo compile SPTK with Visual
                        C++</a> by
                        Alexey Parshin
                    </td>
                </tr>
                <tr>
                    <td><a href="http://www.sptk.net/docs/HowTo-SPTK-XML.html">HowTo create and use an XML document in
                        SPTK</a> by Alexey Parshin
                    </td>
                </tr>
                </tbody>
            </table>

            <p className="notify">Please, consider the documentation unfinished. </p>
        </div>;
    }
}
