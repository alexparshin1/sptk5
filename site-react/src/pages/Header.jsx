import React from "react";
import logo from "../images/logo.png";
import linotex from "../images/linotex_white_1.png";
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
                    <td align="left"><img src={logo} alt=""/></td>
                    <td align="right"><img src={linotex} width="205" height="59" alt=""/>
                    </td>
                    <td width="20"></td>
                </tr>
                </tbody>
            </table>
        </div>;
    }
}
