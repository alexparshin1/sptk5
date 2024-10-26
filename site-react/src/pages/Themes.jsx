import React from "react";
import "../css/Screenshots.css"

import theme_blue from "../screenshots/theme_blue.png";
import theme_dark from "../screenshots/theme_dark.png";
import theme_default from "../screenshots/theme_default.png";
import theme_e17 from "../screenshots/theme_e17.png";
import theme_flat from "../screenshots/theme_flat.png";
import theme_keramic from "../screenshots/theme_keramic.png";
import theme_opera from "../screenshots/theme_opera.png";
import theme_osx from "../screenshots/theme_osx.png";

export default class Themes extends React.Component
{
    render()
    {
        return <div style={{align: "left", background: "#ddd"}}>
            <div id="fadeout"/>
            <div style={{textAlign: "left", padding: 8}}>
                Several examples of the SPTK themes. They are made as copies of the popular themes, developed by
                some other people.
                The screenshots below belong to the examples/cgroup_test.
            </div>
            <table className="screenshot-table" style={{width: "100%"}}>
                <tbody>
                <tr>
                    <td>
                        <img src={theme_blue} alt="" title=""/>
                    </td>
                    <td>
                        <img src={theme_dark} alt="" title=""/>
                    </td>
                </tr>

                <tr>
                    <td>
                        <img src={theme_default} alt="" title=""/>
                    </td>
                    <td>
                        <img src={theme_e17} alt="" title=""/>
                    </td>
                </tr>

                <tr>
                    <td>
                        <img src={theme_flat} alt="" title=""/>
                    </td>
                    <td>
                        <img src={theme_keramic} alt="" title=""/>
                    </td>
                </tr>

                <tr>
                    <td>
                        <img src={theme_opera} alt="" title=""/>
                    </td>
                    <td>
                        <img src={theme_osx} alt="" title=""/>
                    </td>
                </tr>
                </tbody>
            </table>

        </div>
            ;
    }
}
