import React from "react";
import {NavLink} from "react-router-dom";
import ControlAPI from "../ControlAPI";
import "../css/Menu.css";

export default class Menu extends React.Component
{
    state = {
        activePage: "/",
        counter: 0
    };

    constructor(props)
    {
        super();
        this.state.counter = this.getCounter("/");
        if (props.menu === undefined)
        {
            this.menuItems = {
                "/": "Home",
                "/screenshots": "Screenshots",
                "/themes": "Themes",
                "/documentation": "Documentation",
                "/support": "Support",
                "/xmq_about": "XMQ",
                "/contact": "Contact",
                "/downloads": "Downloads"
            };
            this.menuType = "main";
        } else
        {
            this.menuItems = props.menu;
            this.menuType = "submenu";
        }
        this.onChange = props.onChange;
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
        if (this.state.activePage !== activePage && this.onChange !== undefined)
        {
            this.onChange(activePage);
        }
        this.setState({activePage: activePage, counter: this.getCounter(activePage)});
    }

    renderMenuItem(link, text)
    {
        if (link === this.state.activePage)
        {
            return <td key={"menu-item-" + link} className="MenuItemSelected">
                <NavLink to={link} onClick={() => this.setActivePage(link)}>{text}</NavLink>
            </td>;
        }
        return <td key={"menu-item-" + link} className="MenuItem">
            <NavLink to={link} onClick={() => this.setActivePage(link)}>{text}</NavLink>
        </td>;
    }

    render()
    {
        let mainTabs = [];
        for (let link in this.menuItems)
        {
            mainTabs.push(this.renderMenuItem(link, this.menuItems[link]));
        }

        return <div style={{align: 'left'}}>
            <table className="Menu" style={{width: "100%"}}>
                <tbody>
                <tr>
                    {mainTabs}
                    <td align="right" style={{width: "100%"}}>There were {this.state.counter} unique visitors to this
                        page.
                    </td>
                </tr>
                </tbody>
            </table>
        </div>;
    }
}
