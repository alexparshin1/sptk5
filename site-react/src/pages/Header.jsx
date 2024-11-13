import React from "react";
import logo from "../images/logo.png";
import linotex from "../images/linotex_white_1.png";
import advancedInstaller from "../images/AdvancedInstallerLogo.png";
import "../css/Header.css";

export default class Header extends React.Component
{
    state = {
        fileName: ""
    };

    render()
    {
        return <div className="Header">
            <table style={{width: "100%"}}>
                <tbody>
                <tr>
                    <td align="left" colSpan={2}><img src={logo} alt=""/></td>
                    <td align="right">
                        <img src={linotex} width="205" height="59" alt=""/>
                    </td>
                    <td align="right" width="250">
                        <img src={advancedInstaller} width="205" height="59" alt=""/>
                    </td>
                    <td width="20"></td>
                </tr>
                <tr className="Credits">
                    <td>Credits:</td>
                    <td>
                        The SPTK web site is provided by
                        Linotex.
                    </td>
                    <td colSpan="2">
                        The SPTK Windows installer is created with Advanced
                        Installer free Open-Source license.
                    </td>
                </tr>
                </tbody>
            </table>
        </div>;
    }
}
