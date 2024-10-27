import React from "react";
import ControlAPI from "../ControlAPI";
import {Link} from "react-router-dom";

export default class Home extends React.Component
{
    state = {
        fileName: ""
    };

    renderNewsItem(item)
    {
        return <div>
            <hr/>
            <h3>{item.version_date}: {item.version}</h3>
            <div dangerouslySetInnerHTML={{__html: item.news}}/>
        </div>;
    }

    render()
    {
        let historyData = ControlAPI.getRequest("site_host_news_list.php");
        let history = [];
        for (let i = 0; i < historyData.length; i++)
        {
            history.push(this.renderNewsItem(historyData[i]));
        }

        return <div key="home-page" className="Page">
            <div key="home-page-fadeout" id="fadeout"/>
            <p>
                This website contains two main products:
                <ul>
                    <li><Link to="/xmq">XMQ</Link> cross-platform MQTT server.</li>
                    <li>SPTK class library that is used for cross-platform development.</li>
                </ul>
                Both, <Link to="/xmq">XMQ</Link> and SPTK, use GPL/LGPL License Agreement v2.0 that can be found <a
                href="https://opensource.org/license/gpl-2-0">here.</a>
                SPTK is available in source code and binary packages from <Link to="/download">Download</Link> page.
                XMQ is only available as binary packages.
            </p>
            <p>
                Below is the history of development for the last 12 months, a kind of changelog and new version
                announcements.
            </p>
            {history}
        </div>;
    }
}
