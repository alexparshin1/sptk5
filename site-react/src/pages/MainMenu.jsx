import React from "react";
import {NavLink} from "react-router-dom";
import ControlAPI from "../ControlAPI";
import "../css/MainMenu.css";

export default class MainMenu extends React.Component
{
    state = {
        activePage: "/",
        counter: 0
    };

    constructor()
    {
        super();
        this.state.counter = this.getCounter("/");
    }

    getCounter(activePage)
    {
        switch (activePage)
        {
            case "/":
            case "/home":
                activePage = "/index";
                break;
            default:
                break;
        }
        let sptkCounterName = "alexeyp" + activePage + ".php";
        let counter = ControlAPI.getRequest("counter.php", {countername: sptkCounterName});
        return counter.visitors;
    }

    setActivePage(activePage)
    {
        this.setState({activePage: activePage, counter: this.getCounter(activePage)});
    }

    renderMenuItem(link, text)
    {
        return <td key={"menu-item-" + link} className="MainMenuItem">
            <NavLink to={link} onClick={() => this.setActivePage(link)}>{text}</NavLink>
        </td>;
    }

    render()
    {
        let mainTabsData = {
            "/": "Home",
            "/screenshots": "Screenshots",
            "/themes": "Themes",
            "/documentation": "Documentation",
            "/dbtools": "DB Tools",
            "/support": "Support",
            "/xmq": "XMQ",
            "/download": "Contact"
        };

        let mainTabs = [];
        for (let link in mainTabsData)
        {
            mainTabs.push(this.renderMenuItem(link, mainTabsData[link]));
        }

        return <div style={{align: 'left'}}>
            <table className="MainMenu" style={{width: "100%"}}>
                <tbody>
                <tr>
                    {mainTabs}
                    <td align="right" style={{width: "100%"}}>There were {this.state.counter} unique visitors to this
                        page.
                    </td>
                </tr>
                </tbody>
            </table>
            <div id="fadeout"/>
        </div>;
    }
}
