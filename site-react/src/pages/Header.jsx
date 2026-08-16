import React from "react";
import logo from "../images/xmq_logo_wide.svg";
import linotex from "../images/linotex_white_1.png";
import advancedInstaller from "../images/AdvancedInstallerLogo.png";
import LoginDialog from "../components/LoginDialog";
import LocalAuth from "../LocalAuth";
import "../css/Header.css";

export default class Header extends React.Component
{
    state = {
        showLogin: false,
        loggedInUser: LocalAuth.username
    };

    logout()
    {
        LocalAuth.logout();
        this.setState({loggedInUser: ""});
    }

    renderLoginArea()
    {
        if (this.state.loggedInUser) {
            return <span>
                Logged in as <b>{this.state.loggedInUser}</b>
                <button className="LoginLink" onClick={() => this.logout()}>Logout</button>
            </span>;
        }
        return <button className="LoginLink" onClick={() => this.setState({showLogin: true})}>Login</button>;
    }

    render()
    {
        return <div className="Header">
            <table style={{width: "100%"}}>
                <tbody>
                <tr className="LoginArea">
                    <td colSpan={5} align="right">
                        {this.renderLoginArea()}
                    </td>
                </tr>
                <tr>
                    <td align="left" colSpan={2}><img src={logo} width="300" height="72" alt="XMQ"/></td>
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
                        The XMQ web site is provided by
                        Linotex.
                    </td>
                    <td colSpan="2">
                        The XMQ and SPTK Windows installers are created with Advanced
                        Installer free Open-Source license.
                    </td>
                </tr>
                </tbody>
            </table>
            {this.state.showLogin &&
                <LoginDialog onClose={(success) => {
                    this.setState({showLogin: false, loggedInUser: LocalAuth.username});
                }}/>}
        </div>;
    }
}
